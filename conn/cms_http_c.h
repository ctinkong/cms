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
/*
该http支持简单的http请求，普遍的http-flv，不支持gzip
*/
#ifndef __CMS_HTTP_C_H__
#define __CMS_HTTP_C_H__
#include <interface/cms_interf_conn.h>
#include <interface/cms_read_write.h>
#include <interface/cms_stream_info.h>
#include <protocol/cms_flv_pump.h>
#include <common/cms_type.h>
#include <protocol/cms_http.h>
#include <strategy/cms_jitter.h>
#include <mem/cms_mf_mem.h>
#ifdef __CMS_CYCLE_MEM__
#include <mem/cms_cycle_mem.h>
#endif
#include <string>

class ChttpClient :public Conn, public CStreamInfo
{
public:
	ChttpClient(HASH &hash, CReaderWriter *rw, std::string pullUrl, std::string oriUrl,
		std::string refer, bool isTls);
	~ChttpClient();
	//conn 接口
	int activateEV(void *base, struct ev_loop *evLoop);
	int doit();
	int handleEv(bool isRead);
	int handleTimeout();
	int stop(std::string reason);
	std::string getUrl();
	std::string getPushUrl();
	std::string getRemoteIP();
	void down8upBytes();

	CReaderWriter *rwConn();

	//stream info 接口
	int		firstPlaySkipMilSecond();
	bool	isResetStreamTimestamp();
	bool	isNoTimeout();
	int		liveStreamTimeout();
	int 	noHashTimeout();
	bool	isRealTimeStream();
	int64   cacheTT();
	//std::string getRemoteIP() = 0;
	std::string getHost();
	void    makeOneTask();

	int doDecode();
	int doReadData();
	int doTransmission();
	int sendBefore(const char *data, int len);
	bool isFinish() { return false; };
	bool isWebsocket() { return false; };
	void reset() {};

	int doRead();
	int doWrite();

#ifdef __CMS_CYCLE_MEM__
	CmsCycleMem *getCycMem() { return mcycMem; };
	char *allocCycMem(uint32 size, unsigned int msgType);
#endif

	OperatorNewDelete
private:
	void initMediaConfig();
	int  request();
	int  handle();
	int	 handleFlv(int &ret);
	void makeHash();
	void tryCreateTask();
	int  decodeMetaData(char *data, int len);
	int  decodeVideo(char *data, int len, uint32 timestamp);
	int  decodeAudio(char *data, int len, uint32 timestamp);

#ifdef __CMS_CYCLE_MEM__
	CmsCycleMem		*mcycMem; //可能由flvpool释放 连接断开后 数据还在flvpool里面
#endif

	bool			misRequet;
	bool			misDecodeHeader;
	bool			misRedirect;
	CReaderWriter	*mrw;
	std::string		murl;
	std::string		moriUrl;
	std::string		mredirectUrl;
	std::string		mremoteAddr;
	std::string		mremoteIP;
	std::string		mHost;
	HASH			mHash;
	uint32          mHashIdx;
	std::string		mstrHash;
	std::string		mstrRefer;

	int64           mllIdx;
	CHttp			*mhttp;
	CBufferReader	*mrdBuff;
	CBufferWriter	*mwrBuff;

	int   miFirstPlaySkipMilSecond;
	bool  misResetStreamTimestamp;
	bool  misNoTimeout;
	int   miLiveStreamTimeout;
	int   miNoHashTimeout;
	bool  misRealTimeStream;
	int64 mllCacheTT;
	bool  misOpenHls;
	int   mtsDuration;
	int   mtsNum;
	int   mtsSaveNum;

	bool  misChangeMediaInfo;
	bool  misPushFlv;			//是否往flvPool投递过数据
	bool  misDown8upBytes;		//是否统计过数据
	bool  misStop;

	//速度统计
	int32			mxSecdownBytes;
	int32			mxSecUpBytes;
	int32			mxSecTick;

	FlvPoolDataType miTagType;			//数据类型
	uint32			muiTimestamp;	    //该slice数据对应rtmp的时间戳
	int	  miReadFlvHeader;
	bool  misReadTagHeader;
	bool  misReadTagBody;
	bool  misReadTagFooler;
	char  *mtagFlv;
	int	  mtagLen;
	int   mtagReadLen;

	unsigned long  mspeedTick;

	CFlvPump		*mflvPump;
	int64			mcreateTT;

	int64			mtimeoutTick;
	bool			misCreateHls;
};

#endif
