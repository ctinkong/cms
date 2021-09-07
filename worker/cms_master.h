#ifndef __CMS_MASTER_H__
#define __CMS_MASTER_H__
#include <libev/libev-4.25/ev.h>
#include <core/cms_thread.h>
#include <interface/cms_conn_listener.h>
#include <net/cms_tcp_conn.h>
#include <worker/cms_worker.h>
#include <protocol/cms_rtmp_const.h>
#include <worker/cms_master_callback.h>
#include <http/cms_http_client.h>
#include <set>

//ÍøÂç»Øµ÷
typedef void(*listenTcpCallBack)(struct ev_loop *loop, struct ev_io *watcher, int revents);

class CMaster
{
public:
	CMaster();
	virtual ~CMaster();

	static CMaster *instance();
	static void freeInstance();

	bool run();
	void stop();
	void thread();
	static void *routinue(void *p);

	Conn *createHttp(std::string url, std::string method, std::string postData, HttpCallBackFn fn, void *custom);

	Conn *createConn(HASH &hash, char *addr, string pullUrl, std::string pushUrl, std::string oriUrl, std::string strReferer
		, ConnType connectType, RtmpType rtmpType, bool isTcp = true);
	void removeConn(int fd, Conn *conn);

	void masterAliveCallBack(struct ev_loop *loop, struct ev_timer *watcher, int revents);
	void masterTcpAcceptCallBack(struct ev_loop *loop, struct ev_io *watcher, int revents);
private:
	bool isTaskExist(HASH &hash, ConnType connectType, RtmpType rtmpType);
	bool runWorker();
	void stopWorker();
	bool listenAll();
	void fdAssociate2Ev(CConnListener *cls, listenTcpCallBack cb);

	static CMaster	*minstance;

	bool		misRunning;
	TCPListener	*mHttp;
	TCPListener *mHttps;
	TCPListener	*mRtmp;
	TCPListener	*mQuery;

	CWorker		**mworker;
	uint32		midxWorker;

	cms_thread_t		mtid;
	EvCallBackParam		mtimerEcp;
	struct ev_loop		*mevLoop;
	std::set<ev_timer *> msetEvTimer;
	std::set<ev_io *>    msetEvIO;
};


#endif
