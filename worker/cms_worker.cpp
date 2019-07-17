#include <worker/cms_worker.h>
#include <worker/cms_master_callback.h>
#include <mem/cms_mf_mem.h>
#include <log/cms_log.h>
#include <common/cms_utility.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

CWorker::CWorker()
{
	midx = 0;
	mtid = 0;
	mevLoop = NULL;
	misRunning = false;
	mfdPipe[0] = 0;
	mfdPipe[1] = 0;
	memset(&mreadPipeEcp, 0, sizeof(EvCallBackParam));
	memset(&mtimerEcp, 0, sizeof(EvCallBackParam));
}

CWorker::~CWorker()
{

}

bool CWorker::run(int i)
{
	logs->debug("[CWorker::run] worker begin enter");
	if (misRunning)
	{
		logs->warn("### [CWorker::run] has been run ###");
		return true;
	}
	if (pipe(mfdPipe) != 0)
	{
		logs->error("[CWorker::run] create pipe err,errno=%d, strerror=%s *****", errno, strerror(errno));
		return false;
	}
	nonblocking(mfdPipe[0]);//设置管道非阻塞
	midx = i;
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

	ev_timer *evTimer = (ev_timer *)malloc(sizeof(ev_timer));
	mtimerEcp.base = this;
	evTimer->data = (void *)&mtimerEcp;
	ev_timer_init(evTimer, ::workerAliveCallBack, 0., 0.1); //检测线程是否被外部停止
	ev_timer_again(mevLoop, evTimer); //需要重复
	msetEvTimer.insert(evTimer);

	ev_io *evIO = (ev_io*)malloc(sizeof(ev_io));
	mreadPipeEcp.base = this;
	evIO->data = (void*)&mreadPipeEcp;
	ev_io_init(evIO, ::workerPipeCallBack, mfdPipe[0], EV_READ);
	ev_io_start(mevLoop, evIO);
	msetEvIO.insert(evIO);

	int res = cmsCreateThread(&mtid, routinue, this, false);
	if (res == -1)
	{
		char date[128] = { 0 };
		getTimeStr(date);
		logs->error("[CWorker::run] %s ***** file=%s,line=%d cmsCreateThread error *****", date, __FILE__, __LINE__);
		return false;
	}	
	logs->debug("[CWorker::run] worker finish leave");

	return true;
}

void CWorker::stop()
{
	logs->debug("##### [CWorker::stop] worker begin #####");
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

	FdQueeu* fq = NULL;
	while (!mfdAddConnQueue.empty())
	{
		fq = mfdAddConnQueue.front();
		mfdAddConnQueue.pop();
		fq->conn->stop("being stop.");
		delete fq->conn;
		delete fq;
	}

	Conn *conn = NULL;
	std::map<int, Conn*>::iterator itFdConn = mfdConn.begin();
	for (; itFdConn != mfdConn.end(); itFdConn++)
	{
		conn = itFdConn->second;
		conn->stop("being stop.");
		delete conn; //句柄析构的时候会close
	}
	mfdConn.clear();

	if (mfdPipe[0])
	{
		::close(mfdPipe[0]);
	}
	if (mfdPipe[1])
	{
		::close(mfdPipe[1]);
	}
	logs->debug("##### [CWorker::stop] worker %d finish #####");
}

void CWorker::thread()
{
	misRunning = true;
	char szThreadName[128] = { 0 };
	snprintf(szThreadName, sizeof(szThreadName), "cms-worker-%d", midx);
	setThreadName(szThreadName);
	ev_run(mevLoop, 0);
}

void *CWorker::routinue(void *p)
{
	logs->debug("##### [CWorker::routinue] enter %d #####", gettid());
	CWorker *pnt = reinterpret_cast<CWorker*>(p);
	pnt->thread();
	logs->debug("##### [CWorker::routinue] leave #####");
	return NULL;
}

void CWorker::addOneConn(int fd, Conn *conn)
{
	FdQueeu *fq = new FdQueeu;
	fq->fd = fd;
	fq->conn = conn;
	mlockFdQueue.Lock();
	mfdAddConnQueue.push(fq);
	mlockFdQueue.Unlock();
	write(mfdPipe[1], &fd, sizeof(fd));
	logs->debug("[CWorker::addOneConn] worker-%d add one", midx);
}

void CWorker::delOneConn(int fd, Conn *conn)
{
	FdQueeu *fq = new FdQueeu;
	fq->fd = fd;
	fq->conn = NULL;
	mlockFdQueue.Lock();
	mfdDelConnQueue.push(fq);
	mlockFdQueue.Unlock();
	write(mfdPipe[1], &fd, sizeof(fd));
}

void CWorker::workerAliveCallBack(struct ev_loop *loop, struct ev_timer *watcher, int revents)
{
	if (!misRunning)
	{
		logs->warn("@@@ [CWorker::workerAliveCallBack] worker has been stop @@@");
		ev_break(EV_A_ EVBREAK_ALL);
	}
	else
	{

	}
}

void CWorker::workerPipeCallBack(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
	do
	{
		int n = read(watcher->fd, mpipeBuff, CMS_PIPE_BUF_SIZE);
		if (n <= 0)
		{
			break;
		}
	} while (1);
	logs->debug("[CWorker::workerPipeCallBack] worker-%d handle one", midx);
	mlockFdQueue.Lock();
	while (!mfdDelConnQueue.empty())
	{
		FdQueeu *fq = mfdDelConnQueue.front();
		mfdDelConnQueue.pop();
		std::map<int, Conn*>::iterator itFdConn = mfdConn.find(fq->fd);
		if (itFdConn != mfdConn.end() &&
			fq->conn == itFdConn->second)
		{
			//fd 一样不代表conn一样 又可能是新创建的任务 重用了相同的fd
			itFdConn->second->stop("task been deleted.");
			delete itFdConn->second;
			mfdConn.erase(itFdConn);
		}
		delete fq;
	}
	while (!mfdAddConnQueue.empty())
	{
		FdQueeu *fq = mfdAddConnQueue.front();
		mfdAddConnQueue.pop();
		std::map<int, Conn*>::iterator itFdConn = mfdConn.find(fq->fd);
		assert(itFdConn == mfdConn.end());
		mfdConn.insert(make_pair(fq->fd, fq->conn));
		fq->conn->activateEV(this, mevLoop);
		fq->conn->doit();
		delete fq;
	}
	mlockFdQueue.Unlock();
}

void CWorker::connIOCallBack(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
	EvCallBackParam *ecp = (EvCallBackParam*)watcher->data;
	std::map<int, Conn*>::iterator itFdConn = mfdConn.find(ecp->fd);
	assert(itFdConn != mfdConn.end());
	Conn *conn = itFdConn->second;
	bool isError = false;
	do
	{
		if (revents & EV_WRITE)
		{
			if (conn->handleEv(false) == CMS_ERROR)
			{
				isError = true;
				break;
			}
		}
		if (revents & EV_READ)
		{
			if (conn->handleEv(true) == CMS_ERROR)
			{
				isError = true;
				break;
			}
		}
		if (revents & EV_ERROR)
		{
			isError = true;
		}
	} while (0);
	if (isError)
	{
		conn->stop("net error");
		delete conn;
		mfdConn.erase(itFdConn);
	}
}

void CWorker::connTimerCallBack(struct ev_loop *loop, struct ev_timer *watcher, int revents)
{
	EvCallBackParam *ecp = (EvCallBackParam*)watcher->data;
	std::map<int, Conn*>::iterator itFdConn = mfdConn.find(ecp->fd);
	assert(itFdConn != mfdConn.end());
	Conn* conn = itFdConn->second;
	if (conn->handleTimeout() == CMS_ERROR)
	{
		conn->stop("timeout");
		delete conn;
		mfdConn.erase(itFdConn);
	}
}



