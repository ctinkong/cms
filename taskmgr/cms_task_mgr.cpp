/*
The MIT License (MIT)

Copyright (c) 2017- cms(hsc)

Author: 天空没有乌云/kisslovecsh@foxmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <taskmgr/cms_task_mgr.h>
#include <worker/cms_master.h>
#include <net/cms_tcp_conn.h>
#include <log/cms_log.h>
#include <config/cms_config.h>
#include <app/cms_app_info.h>
#include <common/cms_utility.h>
#include <common/cms_url.h>
#include <unistd.h>
#include <assert.h>

#define MapTaskConnIteror std::map<HASH,Conn *>::iterator

CTaskMgr *CTaskMgr::minstance = NULL;
CTaskMgr::CTaskMgr()
{
	misRun = false;
	mtid = 0;
}

CTaskMgr::~CTaskMgr()
{

}

CTaskMgr *CTaskMgr::instance()
{
	if (minstance == NULL)
	{
		minstance = new CTaskMgr;
	}
	return minstance;
}

void  CTaskMgr::freeInstance()
{
	if (minstance)
	{
		delete minstance;
		minstance = NULL;
	}
}

void *CTaskMgr::routinue(void *param)
{
	CTaskMgr *pIns = (CTaskMgr*)param;
	pIns->thread();
	return NULL;
}

void CTaskMgr::thread()
{
	logs->info(">>>>> CTaskMgr thread pid=%d", gettid());
	setThreadName("cms-taskmgr");
	CreateTaskPacket *ctp;
	bool isPop;
	while (misRun)
	{
		isPop = pop(&ctp);
		if (isPop)
		{
			if (ctp->createAct == CREATE_ACT_PULL)
			{
				pullCreateTask(ctp);
			}
			else if (ctp->createAct == CREATE_ACT_PUSH)
			{
				pushCreateTask(ctp);
			}
			delete ctp;
		}
		else
		{
			cmsSleep(10);
		}
	}
	logs->info(">>>>> CTaskMgr thread leave pid=%d", gettid());
}

bool CTaskMgr::run()
{
	misRun = true;
	int res = cmsCreateThread(&mtid, routinue, this, false);
	if (res == -1)
	{
		char date[128] = { 0 };
		getTimeStr(date);
		logs->error("%s ***** file=%s,line=%d cmsCreateThread error *****", date, __FILE__, __LINE__);
		return false;
	}
	return true;
}

void CTaskMgr::stop()
{
	logs->debug("##### CTaskMgr::stop begin #####");
	misRun = false;
	cmsWaitForThread(mtid, NULL);
	mtid = 0;
	logs->debug("##### CTaskMgr::stop finish #####");
}

void CTaskMgr::createTask(HASH &hash, std::string pullUrl, std::string pushUrl, std::string oriUrl,
	std::string refer, int createAct, bool isHotPush, bool isPush2Cdn)
{
	CreateTaskPacket * ctp = new CreateTaskPacket;
	ctp->hash = hash;
	ctp->createAct = createAct;
	ctp->pullUrl = pullUrl;
	ctp->pushUrl = pushUrl;
	ctp->oriUrl = oriUrl;
	ctp->refer = refer;
	ctp->isHotPush = isHotPush;
	ctp->isPush2Cdn = isPush2Cdn;
	ctp->ID = 0;
	push(ctp);
}

void CTaskMgr::push(CreateTaskPacket *ctp)
{
	mlockQueue.Lock();
	mqueueCTP.push(ctp);
	mlockQueue.Unlock();
}

bool CTaskMgr::pop(CreateTaskPacket **ctp)
{
	bool isPop = false;
	mlockQueue.Lock();
	if (!mqueueCTP.empty())
	{
		isPop = true;
		*ctp = mqueueCTP.front();
		mqueueCTP.pop();
	}
	mlockQueue.Unlock();
	return isPop;
}

void CTaskMgr::pullCreateTask(CreateTaskPacket *ctp)
{
	assert(ctp != NULL);
	string sAddr = CConfig::instance()->upperAddr()->getPull(hash2Idx(ctp->hash));
	LinkUrl linUrl;
	if (parseUrl(ctp->pullUrl, linUrl))
	{
		if (g_isTestServer)
		{
			//测试
			CMaster::instance()->createConn(ctp->hash,
				(char *)linUrl.addr.c_str(),
				ctp->pullUrl,
				"",
				"",
				ctp->refer,
				TypeRtmp, RtmpClient2Play,
				!CConfig::instance()->udpFlag()->isOpenUdpPull());
		}
		else if (!sAddr.empty())
		{
			//配置指定从上层源下载数据
			logs->debug("[CTaskMgr::pullCreateTask] create pull task %s dial addr %s,is open udp %s.",
				ctp->pullUrl.c_str(),
				sAddr.c_str(),
				CConfig::instance()->udpFlag()->isOpenUdpPull() ? "true" : "false");

			CMaster::instance()->createConn(ctp->hash,
				sAddr.empty() ? (char *)linUrl.addr.c_str() : (char *)sAddr.c_str(),
				ctp->pullUrl,
				"",
				"",
				ctp->refer,
				TypeRtmp, RtmpClient2Play,
				!CConfig::instance()->udpFlag()->isOpenUdpPull());
		}
		else if (linUrl.protocol == PROTOCOL_RTMP)
		{
			CMaster::instance()->createConn(ctp->hash,
				(char *)linUrl.addr.c_str(),
				ctp->pullUrl,
				"",
				"",
				ctp->refer,
				TypeRtmp, RtmpClient2Play,
				!CConfig::instance()->udpFlag()->isOpenUdpPull());
		}
		else if (linUrl.protocol == PROTOCOL_HTTP)
		{
			CMaster::instance()->createConn(ctp->hash,
				(char *)linUrl.addr.c_str(),
				ctp->pullUrl,
				"",
				ctp->oriUrl,
				ctp->refer,
				TypeHttp,
				RtmpTypeNone);
		}
		else if (linUrl.protocol == PROTOCOL_HTTPS)
		{
			CMaster::instance()->createConn(ctp->hash,
				(char *)linUrl.addr.c_str(),
				ctp->pullUrl,
				"",
				ctp->oriUrl,
				ctp->refer,
				TypeHttps,
				RtmpTypeNone);
		}
	}
	else
	{
		logs->error("*** [CTaskMgr::pullCreateTask] %s parse pull url fail ***", ctp->pullUrl.c_str());
	}
}

void CTaskMgr::pushCreateTask(CreateTaskPacket *ctp)
{
	assert(ctp != NULL);
	LinkUrl linUrl;
	if (parseUrl(ctp->pushUrl, linUrl))
	{
		string sAddr = CConfig::instance()->upperAddr()->getPush(hash2Idx(ctp->hash));
		if (!sAddr.empty())
		{
			logs->debug("[CTaskMgr::pushCreateTask] create push task %s dial addr %s,is open udp %s.",
				ctp->pushUrl.c_str(),
				sAddr.c_str(),
				CConfig::instance()->udpFlag()->isOpenUdpPush() ? "true" : "false");

			CMaster::instance()->createConn(ctp->hash,
				sAddr.empty() ? (char *)linUrl.addr.c_str() : (char *)sAddr.c_str(),
				ctp->pullUrl,
				ctp->pushUrl,
				"",
				ctp->refer,
				TypeRtmp, RtmpClient2Publish,
				!CConfig::instance()->udpFlag()->isOpenUdpPush());
		}
	}
	else
	{
		logs->error("*** [CTaskMgr::pushCreateTask] %s parse push url fail ***", ctp->pushUrl.c_str());
	}
}

bool CTaskMgr::pullTaskAdd(HASH &hash, Conn *conn)
{
	mlockPullTaskConn.Lock();
	MapTaskConnIteror it = mmapPullTaskConn.find(hash);
	if (it != mmapPullTaskConn.end())
	{
		mlockPullTaskConn.Unlock();
		return false;
	}
	mmapPullTaskConn[hash] = conn;
	mlockPullTaskConn.Unlock();
	return true;
}

bool CTaskMgr::pullTaskDel(HASH &hash)
{
	mlockPullTaskConn.Lock();
	MapTaskConnIteror it = mmapPullTaskConn.find(hash);
	if (it == mmapPullTaskConn.end())
	{
		mlockPullTaskConn.Unlock();
		return false;
	}
	mmapPullTaskConn.erase(it);
	mlockPullTaskConn.Unlock();
	return true;
}

bool CTaskMgr::pullTaskStop(HASH &hash)
{
	int fd = 0;
	Conn *conn = NULL;
	mlockPullTaskConn.Lock();
	MapTaskConnIteror it = mmapPullTaskConn.find(hash);
	if (it != mmapPullTaskConn.end())
	{
		mlockPullTaskConn.Unlock();
		return false;
	}
	fd = it->second->rwConn()->fd();
	conn = it->second;
	mlockPullTaskConn.Unlock();
	CMaster::instance()->removeConn(fd, conn);
	return true;
}

void CTaskMgr::pullTaskStopAll()
{
	std::map<int, Conn *> pullTaskConn;
	mlockPullTaskConn.Lock();
	MapTaskConnIteror it;
	for (it = mmapPullTaskConn.begin(); it != mmapPullTaskConn.end(); it++)
	{
		pullTaskConn.insert(make_pair(it->second->rwConn()->fd(), it->second)); //防止死锁
	}
	mlockPullTaskConn.Unlock();

	int fd = 0;
	Conn *conn = NULL;
	for (std::map<int, Conn *>::iterator it = pullTaskConn.begin(); it != pullTaskConn.end(); it++)
	{
		//这一步的时候conn又可能已经释放掉了,此时千万不要调用conn任何函数
		fd = it->first;
		conn = it->second;
		CMaster::instance()->removeConn(fd, conn);
	}
}

void CTaskMgr::pullTaskStopAllByIP(std::string strIP)
{
	std::map<int, Conn *> pullTaskConn;
	mlockPullTaskConn.Lock();
	MapTaskConnIteror it;
	for (it = mmapPullTaskConn.begin(); it != mmapPullTaskConn.end(); it++)
	{
		if (it->second->getRemoteIP() == strIP)
		{
			pullTaskConn.insert(make_pair(it->second->rwConn()->fd(), it->second)); //防止死锁
		}
	}
	mlockPullTaskConn.Unlock();

	int fd = 0;
	Conn *conn = NULL;
	for (std::map<int, Conn *>::iterator it = pullTaskConn.begin(); it != pullTaskConn.end(); it++)
	{
		//这一步的时候conn又可能已经释放掉了,此时千万不要调用conn任何函数
		fd = it->first;
		conn = it->second;
		CMaster::instance()->removeConn(fd, conn);
	}
}

bool CTaskMgr::pullTaskIsExist(HASH &hash)
{
	bool isExist = false;
	mlockPullTaskConn.Lock();
	MapTaskConnIteror it = mmapPullTaskConn.find(hash);
	if (it != mmapPullTaskConn.end())
	{
		isExist = true;
	}
	mlockPullTaskConn.Unlock();
	return isExist;
}
//分水岭
bool CTaskMgr::pushTaskAdd(HASH &hash, Conn *conn)
{
	mlockPushTaskConn.Lock();
	MapTaskConnIteror it = mmapPushTaskConn.find(hash);
	if (it != mmapPushTaskConn.end())
	{
		mlockPushTaskConn.Unlock();
		return false;
	}
	mmapPushTaskConn[hash] = conn;
	mlockPushTaskConn.Unlock();
	return true;
}

bool CTaskMgr::pushTaskDel(HASH &hash)
{
	mlockPushTaskConn.Lock();
	MapTaskConnIteror it = mmapPushTaskConn.find(hash);
	if (it == mmapPushTaskConn.end())
	{
		mlockPushTaskConn.Unlock();
		return false;
	}
	mmapPushTaskConn.erase(it);
	mlockPushTaskConn.Unlock();
	return true;
}

bool CTaskMgr::pushTaskStop(HASH &hash)
{
	int fd = 0;
	Conn *conn = NULL;
	mlockPushTaskConn.Lock();
	MapTaskConnIteror it = mmapPushTaskConn.find(hash);
	if (it != mmapPushTaskConn.end())
	{
		mlockPushTaskConn.Unlock();
		return false;
	}
	fd = it->second->rwConn()->fd();
	conn = it->second;
	mlockPushTaskConn.Unlock();
	CMaster::instance()->removeConn(fd, conn);
	return true;
}

void CTaskMgr::pushTaskStopAll()
{
	std::map<int, Conn *> pushTaskConn;
	mlockPushTaskConn.Lock();
	MapTaskConnIteror it;
	for (it = mmapPushTaskConn.begin(); it != mmapPushTaskConn.end(); it++)
	{
		pushTaskConn.insert(make_pair(it->second->rwConn()->fd(), it->second)); //防止死锁
	}
	mlockPushTaskConn.Unlock();

	int fd = 0;
	Conn *conn = NULL;
	for (std::map<int, Conn *>::iterator it = pushTaskConn.begin(); it != pushTaskConn.end(); it++)
	{
		//这一步的时候conn又可能已经释放掉了,此时千万不要调用conn任何函数
		fd = it->first;
		conn = it->second;
		CMaster::instance()->removeConn(fd, conn);
	}
}

void CTaskMgr::pushTaskStopAllByIP(std::string strIP)
{
	std::map<int, Conn *> pushTaskConn;
	mlockPushTaskConn.Lock();
	MapTaskConnIteror it;
	for (it = mmapPushTaskConn.begin(); it != mmapPushTaskConn.end(); it++)
	{
		if (it->second->getRemoteIP() == strIP)
		{
			pushTaskConn.insert(make_pair(it->second->rwConn()->fd(), it->second)); //防止死锁
		}
	}
	mlockPushTaskConn.Unlock();

	int fd = 0;
	Conn *conn = NULL;
	for (std::map<int, Conn *>::iterator it = pushTaskConn.begin(); it != pushTaskConn.end(); it++)
	{
		//这一步的时候conn又可能已经释放掉了,此时千万不要调用conn任何函数
		fd = it->first;
		conn = it->second;
		CMaster::instance()->removeConn(fd, conn);
	}
}

bool CTaskMgr::pushTaskIsExist(HASH &hash)
{
	bool isExist = false;
	mlockPushTaskConn.Lock();
	MapTaskConnIteror it = mmapPushTaskConn.find(hash);
	if (it != mmapPushTaskConn.end())
	{
		isExist = true;
	}
	mlockPushTaskConn.Unlock();
	return isExist;
}
