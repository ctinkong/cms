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
#ifndef __CMS_TASK_MGR_H__
#define __CMS_TASK_MGR_H__
#include <interface/cms_interf_conn.h>
#include <http/cms_http_client.h>
#include <core/cms_lock.h>
#include <common/cms_type.h>
#include <core/cms_thread.h>
#include <mem/cms_mf_mem.h>
#include <string>
#include <map>
#include <queue>

#define CREATE_ACT_PULL	0x01
#define CREATE_ACT_PUSH	0x02

struct CreateTaskPacket
{
	HASH				hash;
	std::string			pullUrl;
	std::string			pushUrl;
	std::string			oriUrl;
	std::string			refer;
	int					createAct;
	bool				isHotPush;
	bool				isPush2Cdn;
	int64				ID;              //创建过程ID

	OperatorNewDelete
};

struct HttpQueryPacket
{
	char *data;
	int  len;
	CHttpClient *client;
	void *custom;
};

class CTaskMgr
{
public:
	CTaskMgr();
	~CTaskMgr();
	static CTaskMgr *instance();
	static void freeInstance();

	static void *routinue(void *param);
	void thread();
	bool run();
	void stop();

	int httpQueryCB(const char *  data, int dataLen, CHttpClient *client, void *custom);

	//拉流任务接口或者被推流
	bool	pullTaskAdd(HASH &hash, Conn *conn);
	bool	pullTaskDel(HASH &hash);
	bool	pullTaskStop(HASH &hash);
	void	pullTaskStopAll();
	void    pullTaskStopAllByIP(std::string strIP);		//删除拉流ip
	bool	pullTaskIsExist(HASH &hash);
	//推流到其它CDN任务接口
	bool	pushTaskAdd(HASH &hash, Conn *conn);
	bool	pushTaskDel(HASH &hash);
	bool	pushTaskStop(HASH &hash);
	void	pushTaskStopAll();
	void    pushTaskStopAllByIP(std::string strIP);		//删除推流ip
	bool	pushTaskIsExist(HASH &hash);
	//异步创建任务
	void	createTask(HASH &hash, std::string pullUrl, std::string pushUrl, std::string oriUrl,
		std::string refer, int createAct, bool isHotPush, bool isPush2Cdn);
	void	push(CreateTaskPacket *ctp);
	void	push(HttpQueryPacket *packet);

	OperatorNewDelete
private:
	
	bool	pop(HttpQueryPacket **ctp);
	bool	pop(CreateTaskPacket **ctp);
	//创建任务
	void	searchPullUrl(CreateTaskPacket *ctp);  //查询对应的真实流名
	void	pullCreateTask(CreateTaskPacket *ctp); //创建拉流
	void	pushCreateTask(CreateTaskPacket *ctp); //创建推流

	static CTaskMgr *minstance;
	bool			misRun;
	cms_thread_t	mtid;

	std::queue<HttpQueryPacket *>	mqueueHttpPacket;
	std::queue<CreateTaskPacket *>	mqueueCTP;
	CLock							mlockQueue;
	//拉流任务
	CLock					mlockPullTaskConn;
	std::map<HASH, Conn *>	mmapPullTaskConn;
	//推流任务
	CLock					mlockPushTaskConn;
	std::map<HASH, Conn *>	mmapPushTaskConn;

};
#endif
