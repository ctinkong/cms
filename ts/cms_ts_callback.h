#ifndef __CMS_TS_CALLBACK_H__
#define __CMS_TS_CALLBACK_H__
#include <libev/libev-4.25/ev.h>
#include <ts/cms_hls_mgr.h>

void tsAliveCallBack(struct ev_loop *loop, struct ev_timer *watcher, int revents);
void tsTimerCallBack(struct ev_loop *loop, struct ev_timer *watcher, int revents);
#endif
