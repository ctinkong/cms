#include <worker/cms_master_callback.h>
#include <worker/cms_common_var.h>
#include <worker/cms_worker.h>

void masterAliveCallBack(struct ev_loop *loop, struct ev_timer *watcher, int revents)
{
	EvCallBackParam *ecb = (EvCallBackParam *)watcher->data;
	CMaster *cm = (CMaster *)ecb->base;
	cm->masterAliveCallBack(loop, watcher, revents);
}

void masterTcpAcceptCallBack(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
	EvCallBackParam *ecb = (EvCallBackParam *)watcher->data;
	CMaster *cm = (CMaster *)ecb->base;
	cm->masterTcpAcceptCallBack(loop, watcher, revents);
}

void workerAliveCallBack(struct ev_loop *loop, struct ev_timer *watcher, int revents)
{
	EvCallBackParam *ecb = (EvCallBackParam *)watcher->data;
	CWorker *cm = (CWorker *)ecb->base;
	cm->workerAliveCallBack(loop, watcher, revents);
}

void workerPipeCallBack(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
	EvCallBackParam *ecb = (EvCallBackParam *)watcher->data;
	CWorker *cm = (CWorker *)ecb->base;
	cm->workerPipeCallBack(loop, watcher, revents);
}

void connIOCallBack(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
	EvCallBackParam *ecb = (EvCallBackParam *)watcher->data;
	CWorker *cm = (CWorker *)ecb->base;
	cm->connIOCallBack(loop, watcher, revents);
}

void connTimerCallBack(struct ev_loop *loop, struct ev_timer *watcher, int revents)
{
	EvCallBackParam *ecb = (EvCallBackParam *)watcher->data;
	CWorker *cm = (CWorker *)ecb->base;
	cm->connTimerCallBack(loop, watcher, revents);
}


