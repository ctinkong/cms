#ifndef __CMS_MASTER_CALLBACK_H__
#define __CMS_MASTER_CALLBACK_H__
#include <worker/cms_master.h>
#include <libev/libev-4.25/ev.h>

void masterAliveCallBack(struct ev_loop *loop, struct ev_timer *watcher, int revents);//ºÏ≤‚ «∑Ò÷’÷π
void masterTcpAcceptCallBack(struct ev_loop *loop, struct ev_io *watcher, int revents);

void workerAliveCallBack(struct ev_loop *loop, struct ev_timer *watcher, int revents);//ºÏ≤‚ «∑Ò÷’÷π
void workerPipeCallBack(struct ev_loop *loop, struct ev_io *watcher, int revents);

void connIOCallBack(struct ev_loop *loop, struct ev_io *watcher, int revents);
void connTimerCallBack(struct ev_loop *loop, struct ev_timer *watcher, int revents);
#endif
