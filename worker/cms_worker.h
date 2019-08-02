#ifndef __CMS_WORKER_H__
#define __CMS_WORKER_H__
#include <worker/cms_common_var.h>
#include <core/cms_thread.h>
#include <core/cms_lock.h>
#include <interface/cms_interf_conn.h>
#include <libev/libev-4.25/ev.h>
#include <queue>
#include <map>
#include <set>

class CWorker
{
public:
	CWorker();
	virtual ~CWorker();

	bool run(int i);
	void stop();
	void thread();
	static void *routinue(void *p);

	void workerAliveCallBack(struct ev_loop *loop, struct ev_timer *watcher, int revents);
	void workerPipeCallBack(struct ev_loop *loop, struct ev_io *watcher, int revents);
	void connIOCallBack(struct ev_loop *loop, struct ev_io *watcher, int revents);
	void connTimerCallBack(struct ev_loop *loop, struct ev_timer *watcher, int revents);

	void addOneConn(int fd, Conn *conn);
	void delOneConn(int fd, Conn *conn);
private:
	bool		misRunning;
	int			midx;
	int			mfdPipe[2];		//通过管道添加连接
	char        mpipeBuff[CMS_PIPE_BUF_SIZE];

	EvCallBackParam		mreadPipeEcp;
	cms_thread_t		mtid;
	EvCallBackParam		mtimerEcp;
	struct ev_loop		*mevLoop;
	std::set<ev_timer *> msetEvTimer;
	std::set<ev_io *>    msetEvIO;

	std::queue<FdQueeu*> mfdConnQueue;
	CLock				 mlockFdQueue;

	std::map<int, Conn*> mfdConn;
};
#endif