#include <ts/cms_ts_callback.h>


void tsAliveCallBack(struct ev_loop *loop, struct ev_timer *watcher, int revents)
{
	CMissionMgr::instance()->tsAliveCallBack(loop, watcher, revents);
}

void tsTimerCallBack(struct ev_loop *loop, struct ev_timer *watcher, int revents)
{
	CMissionMgr::instance()->tsTimerCallBack(loop, watcher, revents);
}

void hlsMgrPipeCallBack(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
	CMissionMgr::instance()->hlsMgrPipeCallBack(loop, watcher, revents);
}


