/*
The MIT License (MIT)

Copyright (c) 2017- cms(hsc)

Author: 天空没有乌云/kisslovecsh@foxmail.com

Permission is hereby granted, xfree of charge, to any person obtaining a copy of
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
#include <worker/cms_master.h>
#include <worker/cms_master_callback.h>
#include <conn/cms_conn_rtmp.h>
#include <conn/cms_http_c.h>
#include <conn/cms_http_s.h>
#include <log/cms_log.h>
#include <common/cms_utility.h>
#include <config/cms_config.h>
#include <protocol/cms_rtmp.h>
#include <mem/cms_mf_mem.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

CMaster *CMaster::minstance = NULL;
CMaster::CMaster()
{
	mHttp = new TCPListener();
	mHttps = new TCPListener();
	mRtmp = new TCPListener();
	mQuery = new TCPListener();
	misRunning = false;
	mtid = 0;
	mevLoop = NULL;
	mworker = NULL;
	midxWorker = 0;
	memset(&mtimerEcp, 0, sizeof(EvCallBackParam));
}

CMaster::~CMaster()
{
	delete mHttp;
	delete mHttps;
	delete mRtmp;
	delete mQuery;
}

CMaster *CMaster::instance()
{
	if (minstance == NULL)
	{
		minstance = new CMaster();
	}
	return minstance;
}

void CMaster::freeInstance()
{
	if (minstance)
	{
		delete minstance;
		minstance = NULL;
	}
}

bool CMaster::run()
{
	logs->debug("[CMaster::run] worker begin enter");
	if (misRunning)
	{
		logs->warn("### [CMaster::run] has been run ###");
		return true;
	}
	mevLoop = ev_loop_new(EVBACKEND_EPOLL | EVBACKEND_POLL | EVBACKEND_SELECT | EVFLAG_NOENV);

	if (ev_backend(mevLoop) & EVBACKEND_EPOLL)
	{
		printf("gsdn epoll mode\n");
	}
	else if (ev_backend(mevLoop) & EVBACKEND_POLL)
	{
		printf("gsdn poll mode\n");
	}
	else if (ev_backend(mevLoop) & EVBACKEND_SELECT)
	{
		printf("gsdn select mode\n");
	}
	if (!listenAll())
	{
		return false;
	}
	if (!runWorker())
	{
		return false;
	}
	ev_timer *watcher = (ev_timer *)xmalloc(sizeof(ev_timer));
	mtimerEcp.base = this;
	watcher->data = (void *)&mtimerEcp;
	ev_timer_init(watcher, ::workerAliveCallBack, 0., 0.1); //检测线程是否被外部停止
	ev_timer_again(mevLoop, watcher); //需要重复
	msetEvTimer.insert(watcher);

	int res = cmsCreateThread(&mtid, routinue, this, false);
	if (res == -1)
	{
		char date[128] = { 0 };
		getTimeStr(date);
		logs->error("[CMaster::run] %s ***** file=%s,line=%d cmsCreateThread error *****", date, __FILE__, __LINE__);
		return false;
	}
	logs->debug("[CMaster::run] worker finish leave");
	return true;
}

void CMaster::stop()
{
	logs->debug("##### [CMaster::stop] worker begin #####");
	misRunning = false;
	cmsWaitForThread(mtid, NULL);
	mtid = 0;

	ev_loop_destroy(mevLoop);
	mevLoop = NULL;
	std::set<ev_timer *>::iterator itVT = msetEvTimer.begin();
	for (; itVT != msetEvTimer.end(); itVT++)
	{
		xfree(*itVT);
	}
	msetEvTimer.clear();
	std::set<ev_io *>::iterator itIO = msetEvIO.begin();
	for (; itIO != msetEvIO.end(); itIO++)
	{
		xfree(*itIO);
	}
	msetEvIO.clear();
	stopWorker();
	logs->debug("##### [CMaster::stop] worker %d finish #####");
}

void CMaster::thread()
{
	misRunning = true;
	setThreadName("cms-worker-m");
	ev_run(mevLoop, 0);
}

void *CMaster::routinue(void *p)
{
	logs->debug("##### [CMaster::routinue] enter %d #####", gettid());
	CMaster *pnt = reinterpret_cast<CMaster*>(p);
	pnt->thread();
	logs->debug("##### [CMaster::routinue] leave #####");
	return NULL;
}

bool CMaster::runWorker()
{
	bool isSucc = true;
	mworker = new CWorker*[CConfig::instance()->workerCfg()->getCount()];
	for (int i = 0; i < CConfig::instance()->workerCfg()->getCount(); i++)
	{
		if (!isSucc)
		{
			mworker[i] = NULL;
			continue;
		}
		mworker[i] = new CWorker();
		if (!mworker[i]->run(i))
		{
			logs->error("[CMaster::runWorker] worker fun fail");
			isSucc = false;
		}
	}
	return isSucc;
}

void CMaster::stopWorker()
{
	if (mworker)
	{
		for (int i = 0; i < CConfig::instance()->workerCfg()->getCount(); i++)
		{
			if (mworker[i])
			{
				mworker[i]->stop();
				delete mworker[i];
			}
		}
		delete[] mworker;
	}
}

bool CMaster::listenAll()
{
	if (mHttp->listen(CConfig::instance()->addrHttp()->addr(), TypeHttp) == CMS_ERROR)
	{
		logs->error("***** [CMaster::listenAll] listen http fail *****");
		return false;
	}
	if (CConfig::instance()->certKey()->isOpenSSL() &&
		mHttps->listen(CConfig::instance()->addrHttps()->addr(), TypeHttps) == CMS_ERROR)
	{
		logs->error("***** [CMaster::listenAll] listen https fail *****");
		return false;
	}
	if (mRtmp->listen(CConfig::instance()->addrRtmp()->addr(), TypeRtmp) == CMS_ERROR)
	{
		logs->error("***** [CMaster::listenAll] listen tcp rtmp fail *****");
		return false;
	}
	if (mQuery->listen(CConfig::instance()->addrQuery()->addr(), TypeQuery) == CMS_ERROR)
	{
		logs->error("***** [CMaster::listenAll] listen query fail *****");
		return false;
	}
	fdAssociate2Ev(mHttp, ::masterTcpAcceptCallBack);
	if (CConfig::instance()->certKey()->isOpenSSL())
	{
		fdAssociate2Ev(mHttps, ::masterTcpAcceptCallBack);
	}
	fdAssociate2Ev(mRtmp, ::masterTcpAcceptCallBack);
	fdAssociate2Ev(mQuery, ::masterTcpAcceptCallBack);
	return true;
}

void CMaster::fdAssociate2Ev(CConnListener *cls, listenTcpCallBack cb)
{
	EvCallBackParam *ecb = new EvCallBackParam();
	memset(ecb, 0, sizeof(EvCallBackParam));
	//添加到libev网络库中
	ev_io *watcher = (ev_io*)xmalloc(sizeof(ev_io));
	ecb->base = this;
	ecb->cls = cls;
	ecb->isPassive = true;
	watcher->data = (void*)ecb;
	ev_io_init(watcher, cb, cls->fd(), EV_READ);
	ev_io_start(mevLoop, watcher);
	msetEvIO.insert(watcher);
}

Conn *CMaster::createConn(HASH &hash, char *addr, string pullUrl, std::string pushUrl, std::string oriUrl, std::string strReferer
	, ConnType connectType, RtmpType rtmpType, bool isTcp/* = true*/)
{
	Conn *conn = NULL;
	if (isTcp)
	{
		TCPConn *tcp = new TCPConn();
		if (tcp->dialTcp(addr, connectType) == CMS_ERROR)
		{
			delete tcp;
			return NULL;
		}
		if (tcp->connect() == CMS_ERROR)
		{
			delete tcp;
			return NULL;
		}
		logs->info("CMaster create conn hash=%s, "
			"addr=%s, "
			"pull url=%s, "
			"push url=%s, "
			"ori url=%s, "
			"referer=%s, "
			"connectType=%s, "
			"rtmpType=%s, "
			"tcp=%s",
			hash2Char(hash.data).c_str(),
			addr,
			pullUrl.c_str(),
			pushUrl.c_str(),
			oriUrl.c_str(),
			strReferer.c_str(),
			getConnType(connectType).c_str(),
			getRtmpTypeStr(rtmpType).c_str(),
			isTcp ? "true" : "false");

		if (isHttp(connectType) || isHttps(connectType))
		{
			ChttpClient *http = new ChttpClient(hash, tcp, pullUrl, oriUrl, strReferer, isHttps(connectType) ? true : false);
			uint32 i = midxWorker++ % CConfig::instance()->workerCfg()->getCount();
			mworker[i]->addOneConn(tcp->fd(), http);
		}
		else if (isRtmp(connectType))
		{
			CConnRtmp *rtmp = new CConnRtmp(hash, rtmpType, tcp, pullUrl, pushUrl);
			uint32 i = midxWorker++ % CConfig::instance()->workerCfg()->getCount();
			mworker[i]->addOneConn(tcp->fd(), rtmp);
		}
	}
	else
	{

	}
	return conn;
}

void CMaster::removeConn(int fd, Conn *conn)
{
	for (int i = 0; i < CConfig::instance()->workerCfg()->getCount(); i++)
	{
		mworker[i]->delOneConn(fd, conn); //try every worker
	}
}

void CMaster::masterAliveCallBack(struct ev_loop *loop, struct ev_timer *watcher, int revents)
{
	if (!misRunning)
	{
		logs->warn("@@@ [CMaster::workerAliveCallBack] worker has been stop @@@");
		ev_break(EV_A_ EVBREAK_ALL);
	}
	else
	{

	}
}

void CMaster::masterTcpAcceptCallBack(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
	EvCallBackParam *ecb = (EvCallBackParam*)watcher->data;
	ConnType listenType = ((CConnListener*)(ecb->cls))->listenType();
	CReaderWriter *rw;
	int cfd = -1;
	do
	{
		cfd = ((CConnListener*)(ecb->cls))->accept();
		if (cfd == CMS_ERROR)
		{
			break;
		}
		rw = new TCPConn(cfd);
		nonblocking(cfd);
		switch (listenType)
		{
		case TypeHttp:
		case TypeHttps:
		case TypeQuery:
		{
			{
				CHttpServer *hs = new CHttpServer(rw, isHttps(listenType));
				uint32 i = midxWorker++ % CConfig::instance()->workerCfg()->getCount();
				mworker[i]->addOneConn(rw->fd(), hs);
			}
		}
		break;
		case TypeRtmp:
		{
			HASH hash;
			CConnRtmp *rtmp = new CConnRtmp(hash, RtmpAsServerBPlayOrPublish, rw, "", "");
			uint32 i = midxWorker++ % CConfig::instance()->workerCfg()->getCount();
			mworker[i]->addOneConn(rw->fd(), rtmp);
		}
		break;
		default:
			assert(0);
		}
	} while (true);
}



