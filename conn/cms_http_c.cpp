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
#include <conn/cms_http_c.h>
#include <log/cms_log.h>
#include <common/cms_utility.h>
#include <taskmgr/cms_task_mgr.h>
#include <enc/cms_sha1.h>
#include <protocol/cms_http_const.h>
#include <flvPool/cms_flv_pool.h>
#include <protocol/cms_amf0.h>
#include <common/cms_char_int.h>
#include <static/cms_static.h>
#include <worker/cms_master_callback.h>
#include <config/cms_config.h>
#include <conn/cms_conn_common.h>
#include <mem/cms_mf_mem.h>
#include <ts/cms_hls_mgr.h>

CHttpFlvClient::CHttpFlvClient(HASH &hash, CReaderWriter *rw, std::string pullUrl, std::string oriUrl,
	std::string refer, bool isTls)
{
	char remote[23] = { 0 };
	rw->remoteAddr(remote, sizeof(remote));
	mremoteAddr = remote;
	size_t pos = mremoteAddr.find(":");
	if (pos == string::npos)
	{
		mremoteIP = mremoteAddr;
	}
	else
	{
		mremoteIP = mremoteAddr.substr(0, pos);
	}
	mrdBuff = new CBufferReader(rw, DEFAULT_BUFFER_SIZE);
	assert(mrdBuff);
	mwrBuff = new CBufferWriter(rw, DEFAULT_BUFFER_SIZE);
	assert(mwrBuff);
	mrw = rw;
	mhttp = new CHttp(this, mrdBuff, mwrBuff, rw, mremoteAddr, true, isTls);
	mhttp->setUrl(pullUrl);
	mhttp->setOriUrl(oriUrl);
	mhttp->setRefer(refer);
	mhttp->httpRequest()->setRefer(refer);
	mhttp->httpRequest()->setUrl(pullUrl);
	mhttp->httpRequest()->setHeader(HTTP_HEADER_REQ_USER_AGENT, "cms");
	if (!refer.empty())
	{
		mhttp->httpRequest()->setHeader(HTTP_HEADER_REQ_REFERER, refer);
	}
	mhttp->httpRequest()->setHeader(HTTP_HEADER_REQ_ICY_METADATA, "1");

	misRequet = false;
	misDecodeHeader = false;

	murl = pullUrl;
	moriUrl = oriUrl;
	misChangeMediaInfo = false;
	mllIdx = 0;
	misPushFlv = false;
	misDown8upBytes = false;
	misStop = false;
	misRedirect = false;
	//flv
	misReadTagHeader = false;
	misReadTagBody = false;
	misReadTagFooler = false;
	miReadFlvHeader = 13;
	mtagFlv = NULL;;
	mtagLen = 0;
	mtagReadLen = 0;
	mspeedTick = 0;
	mtimeoutTick = getTimeUnix();

	//速度统计
	mxSecdownBytes = 0;
	mxSecUpBytes = 0;
	mxSecTick = 0;

	if (!pullUrl.empty())
	{
		makeHash();
		mHash = hash;
		LinkUrl linkUrl;
		if (parseUrl(pullUrl, linkUrl))
		{
			mHost = linkUrl.host;
		}
	}
	std::string modeName = "CHttpFlvClient";
	mflvPump = new CFlvPump(this, mHash, mHashIdx, mremoteAddr, modeName, murl);

#ifdef __CMS_CYCLE_MEM__
	mcycMem = xmallocCycleMem(CMS_CYCLE_MEM_NODE_SIZE);
#endif

	initMediaConfig();
}

CHttpFlvClient::~CHttpFlvClient()
{
	logs->debug("######### %s [CHttpFlvClient::~CHttpFlvClient] http enter ",
		mremoteAddr.c_str());
	if (mevLoop)
	{
		ev_io_stop(mevLoop, &mevIO);
		ev_timer_stop(mevLoop, &mevTimer);
	}
	if (mtagFlv != NULL)
	{
#ifdef __CMS_CYCLE_MEM__
		if (mcycMem)
		{
			xfreeCycleBuf(mcycMem, mtagFlv);
		}	
#else
		xfree(mtagFlv);
#endif		
	}
	delete mhttp;
	delete mrdBuff;
	delete mwrBuff;
	delete mflvPump;
	mrw->close();
	if (mrw->netType() == NetTcp)//udp 不调用
	{
		//udp 连接由udp模块自身管理 不需要也不能由外部释放
		delete mrw;
	}
}

void CHttpFlvClient::initMediaConfig()
{
	misCreateHls = false;
	miFirstPlaySkipMilSecond = CConfig::instance()->media()->getFirstPlaySkipMilSecond();
	misResetStreamTimestamp = CConfig::instance()->media()->isResetStreamTimestamp();
	misNoTimeout = CConfig::instance()->media()->isNoTimeout();
	miLiveStreamTimeout = CConfig::instance()->media()->getStreamTimeout();
	miNoHashTimeout = CConfig::instance()->media()->getNoHashTimeout();
	misRealTimeStream = CConfig::instance()->media()->isRealTimeStream();
	mllCacheTT = CConfig::instance()->media()->getCacheTime();
	misOpenHls = CConfig::instance()->media()->isOpenHls();
	mtsDuration = CConfig::instance()->media()->getTsDuration();
	mtsNum = CConfig::instance()->media()->getTsNum();
	mtsSaveNum = CConfig::instance()->media()->getTsSaveNum();
}

int CHttpFlvClient::activateEV(void *base, struct ev_loop *evLoop)
{
	mbase = base;
	mevLoop = evLoop;
	//io
	mioECP.base = mbase;
	mioECP.fd = mrw->fd();
	mevIO.data = (void*)&mioECP;
	ev_io_init(&mevIO, ::connIOCallBack, mrw->fd(), EV_READ | EV_WRITE);
	ev_io_start(mevLoop, &mevIO);
	//timer
	mtimerECP.base = mbase;
	mtimerECP.fd = mrw->fd();
	mevTimer.data = (void *)&mtimerECP;
	ev_timer_init(&mevTimer, ::connTimerCallBack, 0., CMS_CONN_TIMEOUT_MILSECOND); //检测线程是否被外部停止
	ev_timer_again(mevLoop, &mevTimer); //需要重复
	return 0;
}

int CHttpFlvClient::doit()
{
	if (!mhttp->run())
	{
		return CMS_ERROR;
	}
	if (!CTaskMgr::instance()->pullTaskAdd(mHash, this))
	{
		logs->debug("***%s [CHttpFlvClient::doit] http %s task is exist ***",
			mremoteAddr.c_str(), murl.c_str());
		return CMS_ERROR;
	}
	return CMS_OK;
}

int CHttpFlvClient::handleEv(bool isRead)
{
	if (misStop)
	{
		return CMS_ERROR;
	}

	if (isRead)
	{
		return doRead();
	}
	else
	{
		return doWrite();

	}
	return CMS_OK;
}

int CHttpFlvClient::handleTimeout()
{
	{
		int64 tn = getTimeUnix();
		if (tn - mtimeoutTick > 30)
		{
			logs->error("%s [CHttpFlvClient::handleTimeout] http %s is timeout ***",
				mremoteAddr.c_str(), murl.c_str());
			return CMS_ERROR;
		}
	}
	// 	if (mhttp->want2Read() == CMS_ERROR)
	// 	{
	// 		return CMS_ERROR;
	// 	}
	if (mhttp->want2Write() == CMS_ERROR)
	{
		return CMS_ERROR;
	}
	return CMS_OK;
}

int CHttpFlvClient::stop(std::string reason)
{
	//可能会被调用两次,任务断开时,正常调用一次 reason 为空,
	//主动断开时,会调用,reason 是调用原因
	logs->debug("%s [CHttpFlvClient::stop] http %s has been stop, reason %s ",
		mremoteAddr.c_str(), murl.c_str(), reason.c_str());
	if (!misStop)
	{
		if (misPushFlv)
		{
			mflvPump->stop();
		}
#ifdef __CMS_CYCLE_MEM__
		else
		{
			if (mtagFlv != NULL)
			{
				xfreeCycleBuf(mcycMem, mtagFlv);
			}
			xfreeCycleMem(mcycMem);
			mcycMem = NULL;
		}
#endif
		if (misDown8upBytes)
		{
			down8upBytes();
			makeOneTaskDownload(mHash, 0, true, true);
		}

		CTaskMgr::instance()->pullTaskDel(mHash);
		if (misRedirect)
		{
			tryCreateTask();
		}
	}
	misStop = true;
	if (misCreateHls)
	{
		CMissionMgr::instance()->destroy(mHashIdx, mHash);
		logs->debug("%s [CHttpFlvClient::stop] destroy hls, url %s",
			mremoteAddr.c_str(),
			murl.c_str());
	}
	return CMS_OK;
}

std::string CHttpFlvClient::getUrl()
{
	return murl;
}

std::string CHttpFlvClient::getPushUrl()
{
	return "";
}

std::string &CHttpFlvClient::getRemoteIP()
{
	return mremoteIP;
}

int CHttpFlvClient::doDecode()
{
	int ret = CMS_OK;
	if (!misDecodeHeader)
	{
		misDecodeHeader = true;
		if (mhttp->httpResponse()->getStatusCode() == HTTP_CODE_200 ||
			mhttp->httpResponse()->getStatusCode() == HTTP_CODE_206)
		{
			//succ
			std::string transferEncoding = mhttp->httpResponse()->getHeader(HTTP_HEADER_RSP_TRANSFER_ENCODING_L);
			if (transferEncoding == HTTP_VALUE_CHUNKED)
			{
				mhttp->setChunked();
			}
		}
		else if (mhttp->httpResponse()->getStatusCode() == HTTP_CODE_301 ||
			mhttp->httpResponse()->getStatusCode() == HTTP_CODE_302 ||
			mhttp->httpResponse()->getStatusCode() == HTTP_CODE_303)
		{
			//redirect
			misRedirect = true;
			mredirectUrl = mhttp->httpResponse()->getHeader(HTTP_HEADER_LOCATION);
			logs->info("%s [CHttpFlvClient::doDecode] http %s code %d redirect %s ",
				mremoteAddr.c_str(), moriUrl.c_str(), mhttp->httpResponse()->getStatusCode(), mredirectUrl.c_str());
			ret = CMS_ERROR;
		}
		else
		{
			//error
			logs->error("%s [CHttpFlvClient::doDecode] http %s code %d rsp %s ***",
				mremoteAddr.c_str(), moriUrl.c_str(), mhttp->httpResponse()->getStatusCode(),
				mhttp->httpResponse()->getResponse().c_str());
			ret = CMS_ERROR;
		}
	}
	return ret;
}

int CHttpFlvClient::doReadData()
{
	char *p;
	int ret = 0;
	int len;
	do
	{
		//flv header
		while (miReadFlvHeader > 0)
		{
			len = miReadFlvHeader;
			ret = mhttp->readFull(&p, len);
			if (ret <= 0)
			{
				return ret;
			}
			miReadFlvHeader -= ret;
			//logs->debug("%s [CHttpFlvClient::doReadData] http %s read flv header len %d ",
			//	mremoteAddr.c_str(),moriUrl.c_str(),ret);
		}
		//flv tag
		if (!misReadTagHeader)
		{
			char *tagHeader;
			len = 11;
			ret = mhttp->readFull(&tagHeader, len); //肯定会读到11字节
			if (ret <= 0)
			{
				return ret;
			}

			//printf("%s [CHttpFlvClient::doReadData] http %s read flv tag header len %d \n",
			//	mremoteAddr.c_str(),moriUrl.c_str(),ret);

			miTagType = (FlvPoolDataType)tagHeader[0];
			mtagLen = (int)bigInt24(tagHeader + 1);
			if (mtagLen < 0 || mtagLen > 1024 * 1024 * 10)
			{
				logs->error("%s [CHttpFlvClient::doReadData] http %s read tag len %d unexpect,tag type=%d ***",
					mremoteAddr.c_str(), moriUrl.c_str(), mtagLen, miTagType);
				return CMS_ERROR;
			}

			//printf("%s [CHttpFlvClient::doReadData] http %s read flv tag type %d, tag len %d \n",
			//	mremoteAddr.c_str(),moriUrl.c_str(),miTagType,mtagLen);

			p = (char*)&muiTimestamp;
			p[2] = tagHeader[4];
			p[1] = tagHeader[5];
			p[0] = tagHeader[6];
			p[3] = tagHeader[7];
			misReadTagHeader = true;
#ifdef __CMS_CYCLE_MEM__
			mtagFlv = allocCycMem(mtagLen, (int)miTagType);
#else
			mtagFlv = (char*)xmalloc(mtagLen);
#endif
			mtagReadLen = 0;
		}

		if (misReadTagHeader)
		{
			if (!misReadTagBody)
			{
				while (mtagReadLen < mtagLen)
				{
					len = mtagLen - mtagReadLen > 1024 * 8 ? 1024 * 8 : mtagLen - mtagReadLen;
					ret = mhttp->readFull(&p, len);
					if (ret <= 0)
					{
						return ret;
					}
					memcpy(mtagFlv + mtagReadLen, p, len);
					mtagReadLen += len;
				}
				misReadTagBody = true;
				//printf("%s [CHttpFlvClient::doReadData] http %s read flv tag body=%d \n",
				//	mremoteAddr.c_str(),moriUrl.c_str(),mtagReadLen);
			}
			if (!misReadTagFooler)
			{
				char *tagHeaderFooler;
				len = 4;
				ret = mhttp->readFull(&tagHeaderFooler, len); //肯定会读到4字节
				if (ret <= 0)
				{
					return ret;
				}
				misReadTagFooler = true;
				int tagTotalLen = bigInt32(tagHeaderFooler);
				if (mtagLen + 11 != tagTotalLen)
				{
					//警告
					//printf("%s [CHttpFlvClient::doReadData] http %s handle tagTotalLen=%d,mtagLen+11=%d \n",
					//	mremoteAddr.c_str(),moriUrl.c_str(),tagTotalLen,mtagLen+11);
				}
				//printf("%s [CHttpFlvClient::doReadData] http %s handle tagTotalLen=%d,mtagLen=%d \n",
				//	mremoteAddr.c_str(),moriUrl.c_str(),tagTotalLen,mtagLen);
			}

			misReadTagHeader = false;
			misReadTagBody = false;
			misReadTagFooler = false;

			switch ((int)miTagType)
			{
			case FLV_TAG_AUDIO:
				//printf("%s [CHttpFlvClient::doReadData] http %s handle audio tag \n",
				//	mremoteAddr.c_str(),moriUrl.c_str());
				mtimeoutTick = getTimeUnix();

				decodeAudio(mtagFlv, mtagLen, muiTimestamp);
				mtagFlv = NULL;
				mtagLen = 0;
				break;
			case FLV_TAG_VIDEO:
				//printf("%s [CHttpFlvClient::doReadData] http %s handle video tag \n",
				//	mremoteAddr.c_str(),moriUrl.c_str());
				mtimeoutTick = getTimeUnix();

				decodeVideo(mtagFlv, mtagLen, muiTimestamp);
				mtagFlv = NULL;
				mtagLen = 0;
				break;
			case FLV_TAG_SCRIPT:
				//printf("%s [CHttpFlvClient::doReadData] http %s handle metaData tag \n",
				//	mremoteAddr.c_str(),moriUrl.c_str());
				mtimeoutTick = getTimeUnix();

				decodeMetaData(mtagFlv, mtagLen);
				mtagFlv = NULL;
				mtagLen = 0;
				break;
			default:
				logs->error("*** %s [CHttpFlvClient::doReadData] http %s read tag type %d unexpect ***",
					mremoteAddr.c_str(), moriUrl.c_str(), miTagType);
				return 0;
			}
		}
	} while (1);
	return ret;
}

int CHttpFlvClient::doTransmission()
{
	return CMS_OK;
}

int CHttpFlvClient::sendBefore(const char *data, int len)
{
	return CMS_OK;
}

int CHttpFlvClient::doRead()
{
	return mhttp->want2Read();
}

int CHttpFlvClient::doWrite()
{
	return mhttp->want2Write();
}

int  CHttpFlvClient::handle()
{
	return CMS_OK;
}

int	 CHttpFlvClient::handleFlv(int &ret)
{
	return CMS_OK;
}

int CHttpFlvClient::request()
{
	return CMS_OK;
}

void CHttpFlvClient::makeHash()
{
	HASH tmpHash;
	if (mHash == tmpHash)
	{
		mHash = makeUrlHash(murl);
		mstrHash = hash2Char(mHash.data);
		mHashIdx = CFlvPool::instance()->hashIdx(mHash);
		logs->info("%s [CHttpFlvClient::makeHash] %s hash url %s,hash=%s",
			mremoteAddr.c_str(), murl.c_str(), readHashUrl(murl).c_str(), mstrHash.c_str());
	}
	else
	{
		mHashIdx = CFlvPool::instance()->hashIdx(mHash);
	}
}

void CHttpFlvClient::tryCreateTask()
{
	if (!CTaskMgr::instance()->pullTaskIsExist(mHash))
	{
		CTaskMgr::instance()->createTask(mHash, mredirectUrl, "", moriUrl, mstrRefer, CREATE_ACT_PULL, false, false);
	}
}

int CHttpFlvClient::decodeMetaData(char *data, int len)
{
	misChangeMediaInfo = false;
	int ret = mflvPump->decodeMetaData(data, len, misChangeMediaInfo);
	if (ret == 1)
	{
		misPushFlv = true;
	}
#ifdef __CMS_CYCLE_MEM__
	xumallocCycleBuf(mcycMem, data);
#else
	xfree(data);
#endif	
	return CMS_OK;
}

int  CHttpFlvClient::decodeVideo(char *data, int len, uint32 timestamp)
{
	misChangeMediaInfo = false;
	int ret = mflvPump->decodeVideo(data, len, timestamp, misChangeMediaInfo);
	if (ret == 1)
	{
		misPushFlv = true;
	}
	else
	{
#ifdef __CMS_CYCLE_MEM__
		xumallocCycleBuf(mcycMem, data);
#else
		xfree(data);
#endif	
	}
	if (misOpenHls && !misCreateHls)
	{
		misCreateHls = true;
		std::string hlsUrl = makeHlsUrl(murl);
		logs->debug("%s [CHttpFlvClient::decodeVideo] %s http m3u8 url %s",
			mremoteAddr.c_str(), murl.c_str(), hlsUrl.c_str());
		CMissionMgr::instance()->create(mHashIdx, mHash, hlsUrl, mtsDuration, mtsNum, mtsSaveNum);
	}
	return CMS_OK;
}

int  CHttpFlvClient::decodeAudio(char *data, int len, uint32 timestamp)
{
	misChangeMediaInfo = false;
	int ret = mflvPump->decodeAudio(data, len, timestamp, misChangeMediaInfo);
	if (ret == 1)
	{
		misPushFlv = true;
	}
	else
	{
#ifdef __CMS_CYCLE_MEM__
		xumallocCycleBuf(mcycMem, data);
#else
		xfree(data);
#endif		
	}
	if (misOpenHls && !misCreateHls)
	{
		misCreateHls = true;
		std::string hlsUrl = makeHlsUrl(murl);
		logs->debug("%s [CHttpFlvClient::Audio] %s http m3u8 url %s",
			mremoteAddr.c_str(), murl.c_str(), hlsUrl.c_str());
		CMissionMgr::instance()->create(mHashIdx, mHash, hlsUrl, mtsDuration, mtsNum, mtsSaveNum);
	}
	return CMS_OK;
}

void CHttpFlvClient::down8upBytes()
{
	unsigned long tt = getTickCount();
	if (tt - mspeedTick > 1000)
	{
		mspeedTick = tt;
		int32 bytes = mrdBuff->readBytesNum();
		if (bytes > 0 && misPushFlv)
		{
			misDown8upBytes = true;
			makeOneTaskDownload(mHash, bytes, false, true);
		}

		mxSecdownBytes += bytes;

		bytes = mwrBuff->writeBytesNum();
		if (bytes > 0)
		{
			makeOneTaskupload(mHash, bytes, PACKET_CONN_DATA);
		}

		mxSecUpBytes += bytes;
		mxSecTick++;
		if (((mxSecTick + (0x0F - (CMS_SPEED_DURATION >= 0x0F ? 10 : CMS_SPEED_DURATION) + 1)) & 0x0F) == 0)
		{
			logs->info("%s [CHttpFlvClient::down8upBytes] %s download speed %s,upload speed %s",
				mremoteAddr.c_str(), murl.c_str(),
				parseSpeed8Mem(mxSecdownBytes / mxSecTick, true).c_str(),
				parseSpeed8Mem(mxSecUpBytes / mxSecTick, true).c_str());
			mxSecTick = 0;
			mxSecdownBytes = 0;
			mxSecUpBytes = 0;
		}
	}
}

int		CHttpFlvClient::firstPlaySkipMilSecond()
{
	return miFirstPlaySkipMilSecond;
}

bool	CHttpFlvClient::isResetStreamTimestamp()
{
	return misResetStreamTimestamp;
}

bool	CHttpFlvClient::isNoTimeout()
{
	return misNoTimeout;
}

int		CHttpFlvClient::liveStreamTimeout()
{
	return miLiveStreamTimeout;
}

int	CHttpFlvClient::noHashTimeout()
{
	return miNoHashTimeout;
}

bool	CHttpFlvClient::isRealTimeStream()
{
	return misRealTimeStream;
}

int64   CHttpFlvClient::cacheTT()
{
	return mllCacheTT;
}

std::string &CHttpFlvClient::getHost()
{
	return mHost;
}


void CHttpFlvClient::makeOneTask()
{
	makeOneTaskDownload(mHash, 0, false, true);
	makeOneTaskMedia(mHash, mflvPump->getVideoFrameRate(), mflvPump->getAudioFrameRate(), mflvPump->getWidth(), mflvPump->getHeight(),
		mflvPump->getAudioSampleRate(), mflvPump->getMediaRate(), getVideoType(mflvPump->getVideoType()),
		getAudioType(mflvPump->getAudioType()), murl, mremoteAddr, mrw->netType() == NetUdp);
}

CReaderWriter *CHttpFlvClient::rwConn()
{
	return mrw;
}

#ifdef __CMS_CYCLE_MEM__
char *CHttpFlvClient::allocCycMem(uint32 size, unsigned int msgType)
{
	if (msgType != MESSAGE_TYPE_VIDEO && msgType != MESSAGE_TYPE_AUDIO)
	{
		return (char *)xmallocCycleBuf(mcycMem, size, 1);
	}
	return (char *)xmallocCycleBuf(mcycMem, size, 0);
}
#endif




