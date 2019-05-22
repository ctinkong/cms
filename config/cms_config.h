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
#ifndef __CONFIG_H__
#define __CONFIG_H__
#include <log/cms_log.h>
#include <cJSON/cJSON.h>
#include <common/cms_type.h>
#include <s2n/s2n.h>
#include <vector>
#include <string>

class CAddr
{
public:
	CAddr(char *addr, int defaultPort);
	~CAddr();
	char *addr();
	char *host();
	int  port();
private:
	char	*mAddr;
	char    *mHost;
	int		mPort;
};

class Clog
{
public:
	Clog(char *path, char *level, bool console, int size);
	~Clog();
	char		*path();
	int			size();
	LogLevel	level();
	bool		console();
private:
	char		*mpath;
	int			msize;
	bool		mconsole;
	LogLevel	mlevel;
};

class CSSLCertKey
{
public:
	CSSLCertKey();
	~CSSLCertKey();

	void setCertKeyNum(int n);
	bool setCertKey(char *cert, char *key);
	void setdhparam(char *dhparam);
	void setCipherVersion(char *v);
	int  getCertKeyNum();
	bool getCertKey(int i, char **cert, char**key);
	char *getCipherVersion();
	char *getDHparam();
	bool isOpenSSL();
	void free();
	bool createS2NConfig();
	struct s2n_config * s2nConfig();

private:
	int  mcertKeyNum;
	int  msetingIdx;
	bool misOpen;
	char **mcert;
	char **mkey;
	char *mcipherVersion;
	char *mdhparam;
	struct s2n_config *ms2nConfig;
};

class CUpperAddr
{
public:
	CUpperAddr();
	~CUpperAddr();

	void		addPull(std::string addr);
	std::string getPull(unsigned int i);
	void		addPush(std::string addr);
	std::string getPush(unsigned int i);
private:
	std::vector<std::string> mvPullAddr;
	std::vector<std::string> mvPushAddr;

};

class CUdpFlag
{
public:
	CUdpFlag();
	~CUdpFlag();
	bool		isOpenUdpPull();
	bool		isOpenUdpPush();
	void		setUdp(bool isPull, bool isPush);
	void		setUdpConnNum(int n);
	int			udpConnNum();
private:
	bool	    misOpenUdpPull;
	bool	    misOpenUdpPush;
	int			miUdpMaxConnNum;
};

class CMedia
{
public:
	CMedia();
	~CMedia();

	void set(int iFirstPlaySkipMilSecond,
		bool  isResetStreamTimestamp,
		bool  isNoTimeout,
		int   iLiveStreamTimeout,
		int   iNoHashTimeout,
		bool  isRealTimeStream,
		int64 llCacheTT,
		bool  isOpenHls,
		int   tsDuration,
		int   tsNum,
		int   tsSaveNum);
	int   getFirstPlaySkipMilSecond();
	bool  isResetStreamTimestamp();
	bool  isNoTimeout();
	int   getStreamTimeout();
	int   getNoHashTimeout();
	bool  isRealTimeStream();
	int64 getCacheTime();
	bool  isOpenHls();
	int   getTsDuration();
	int   getTsNum();
	int   getTsSaveNum();
private:
	int   miFirstPlaySkipMilSecond;	//首播丢帧
	bool  misResetStreamTimestamp;	//首播重设时间戳
	bool  misNoTimeout;				//
	int   miLiveStreamTimeout;		//直播流超时时间 单位毫秒
	int   miNoHashTimeout;			//播放任务不存在超时时间 单位毫秒
	bool  misRealTimeStream;		//低延时直播
	int64 mllCacheTT;				//缓存时长 单位毫秒
	bool  misOpenHls;				//开启内存切片
	int   mtsDuration;				//每块ts时长 单位秒
	int   mtsNum;					//m3u8 ts片数
	int   mtsSaveNum;				//内存缓存ts片数
};

class CWorkerCfg
{
public:
	CWorkerCfg();
	~CWorkerCfg();

	void setCount(int c);
	int	 getCount();
private:
	int	mworkerCount;
};

class CConfig
{
public:
	CConfig();
	~CConfig();
	static CConfig *instance();
	static void freeInstance();
	bool		init(const char *configPath);
	CAddr		*addrHttp();
	CAddr		*addrHttps();
	CAddr		*addrRtmp();
	CAddr		*addrQuery();
	CSSLCertKey	*certKey();
	Clog		*clog();
	CUpperAddr	*upperAddr();
	CUdpFlag	*udpFlag();
	CWorkerCfg  *workerCfg();
	CMedia      *media();
private:
	bool parseListenJson(cJSON *root, const char *configPath);
	bool parseNextHopJson(cJSON *root, const char *configPath);
	bool parseUdpJson(cJSON *root, const char *configPath);
	bool parseTlsJson(cJSON *root, const char *configPath);
	bool parseWorkerJson(cJSON *root, const char *configPath);
	bool parseLogJson(cJSON *root, const char *configPath);
	bool parseMediaJson(cJSON *root, const char *configPath);

	static CConfig *minstance;

	CAddr	   *mHttp;
	CAddr	   *mHttps;
	CAddr	   *mRtmp;
	CAddr	   *mQuery;
	CUpperAddr *muaAddr;
	Clog	   *mlog;
	CSSLCertKey*mcertKey;
	CUdpFlag   *mUdpFlag;
	CWorkerCfg *mworker;
	CMedia	   *mmedia;
};
#endif
