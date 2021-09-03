#include <http/cms_http_client.h>
#include <log/cms_log.h>
#include <common/cms_utility.h>
#include <protocol/cms_http_const.h>
#include <common/cms_char_int.h>
#include <worker/cms_master_callback.h>

CHttpClient::CHttpClient(CReaderWriter *rw, std::string strUrl, std::string refer)
{
	mrdBuff = new CBufferReader(rw, DEFAULT_BUFFER_SIZE);
	assert(mrdBuff);
	mwrBuff = new CBufferWriter(rw, DEFAULT_BUFFER_SIZE);
	assert(mwrBuff);
	mrw = rw;
	char remote[23] = { 0 };
	rw->remoteAddr(remote, sizeof(remote));
	LinkUrl linkUrl;
	parseUrl(strUrl, linkUrl);
	mhttp = new CHttp(this, mrdBuff, mwrBuff, rw, remote, true, linkUrl.protocol == PROTOCOL_HTTPS);
	mhttp->setUrl(strUrl);
	mhttp->setOriUrl(strUrl);
	mhttp->setRefer(refer);
	mhttp->httpRequest()->setRefer(refer);
	mhttp->httpRequest()->setUrl(strUrl);
	mhttp->httpRequest()->setHeader(HTTP_HEADER_REQ_USER_AGENT, "cms");
	if (!refer.empty())
	{
		mhttp->httpRequest()->setHeader(HTTP_HEADER_REQ_REFERER, refer);
	}

	mcb = NULL;
	mcustomData = NULL;;

	misRequet = false;
	misDecodeHeader = false;

	misStop = false;
	misRedirect = false;

	//速度统计
	mxSecdownBytes = 0;
	mxSecUpBytes = 0;
	mxSecTick = 0;


	mtimeoutTick = getTimeUnix();
}

CHttpClient::~CHttpClient()
{
	if (mevLoop)
	{
		ev_io_stop(mevLoop, &mevIO);
		ev_timer_stop(mevLoop, &mevTimer);
	}
	delete mhttp;
	delete mrdBuff;
	delete mwrBuff;
	mrw->close();
	if (mrw->netType() == NetTcp)//udp 不调用
	{
		//udp 连接由udp模块自身管理 不需要也不能由外部释放
		delete mrw;
	}
}

Request	*CHttpClient::httpRequest()
{
	return mhttp->httpRequest();
}

Response*CHttpClient::httpResponse()
{
	return mhttp->httpResponse();
}

bool CHttpClient::start(std::string postData, HttpCallBackFn cb, void *custom)
{
	mpostData = postData;
	mcb = cb;
	mcustomData = custom;
	char szLen[128] = { 0 };
	snprintf(szLen, sizeof(szLen), "%lu", postData.length());
	mhttp->httpRequest()->setHeader(HTTP_HEADER_RSP_CONTENT_LENGTH, szLen);
	return true;
}

void CHttpClient::saveData(const char* data, int len)
{
	msaveData.append(data, len);
}

std::string &CHttpClient::getData()
{
	return msaveData;
}

int CHttpClient::activateEV(void *base, struct ev_loop *evLoop)
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

int CHttpClient::doit()
{
	if (!mhttp->run())
	{
		return CMS_ERROR;
	}
	return CMS_OK;
}

int CHttpClient::handleEv(bool isRead)
{
	if (misStop)
	{
		return CMS_ERROR;
	}

	if (isRead)
	{
		return mhttp->want2Read();
	}
	else
	{
		return mhttp->want2Write();

	}
	return CMS_OK;
}

int CHttpClient::handleTimeout()
{
	{
		int64 tn = getTimeUnix();
		if (tn - mtimeoutTick > 30)
		{
			logs->error("%s [CHttpClient::handleTimeout] http %s is timeout ***",
				mhttp->remoteAddr().c_str(), mhttp->getUrl().c_str());
			return CMS_ERROR;
		}
	}
	if (mhttp->want2Write() == CMS_ERROR)
	{
		return CMS_ERROR;
	}
	return CMS_OK;
}

int CHttpClient::stop(std::string reason)
{
	//可能会被调用两次,任务断开时,正常调用一次 reason 为空,
	//主动断开时,会调用,reason 是调用原因
	logs->debug("%s [CHttpClient::stop] http %s has been stop, reason %s ",
		mhttp->remoteAddr().c_str(), mhttp->getUrl().c_str(), reason.c_str());
	if (!misStop)
	{
	}
	misStop = true;
	return CMS_OK;
}

std::string CHttpClient::getUrl()
{
	return mhttp->getUrl();
}

std::string CHttpClient::getPushUrl()
{
	return "";
}

std::string &CHttpClient::getRemoteIP()
{
	if (mremoteAddr.empty())
	{
		mremoteAddr = mhttp->remoteAddr();
	}
	return mremoteAddr;
}

void CHttpClient::down8upBytes()
{

}

void CHttpClient::reset()
{

}

CReaderWriter *CHttpClient::rwConn()
{
	return mrw;
}

int  CHttpClient::doDecode()
{
	int ret = CMS_OK;
	if (!misDecodeHeader)
	{
		misDecodeHeader = true;
		if (mhttp->httpResponse()->getStatusCode() == HTTP_CODE_200 ||
			mhttp->httpResponse()->getStatusCode() == HTTP_CODE_206)
		{
			logs->debug("%s [CHttpClient::doDecode] http %s success.",
				mhttp->remoteAddr().c_str(), mhttp->getUrl().c_str());
			//succ
			std::string transferEncoding = mhttp->httpResponse()->getHeader(HTTP_HEADER_RSP_TRANSFER_ENCODING_L);
			if (transferEncoding == HTTP_VALUE_CHUNKED)
			{
				mhttp->setChunked();
			}
			std::string contengLen = mhttp->httpResponse()->getHeader(HTTP_HEADER_RSP_CONTENT_LENGTH);
			if (!contengLen.empty())
			{
				int64 len = atoi(contengLen.c_str());
				logs->debug("%s [CHttpClient::doDecode] http %s content len %d ",
					mhttp->remoteAddr().c_str(), mhttp->getUrl().c_str(), len);
				mhttp->setContentLength(len);
			}
		}
		else if (mhttp->httpResponse()->getStatusCode() == HTTP_CODE_301 ||
			mhttp->httpResponse()->getStatusCode() == HTTP_CODE_302 ||
			mhttp->httpResponse()->getStatusCode() == HTTP_CODE_303)
		{
			//redirect
			misRedirect = true;
			mredirectUrl = mhttp->httpResponse()->getHeader(HTTP_HEADER_LOCATION);
			logs->info("%s [CHttpClient::doDecode] http %s code %d redirect %s ",
				mhttp->remoteAddr().c_str(), mhttp->getUrl().c_str(),
				mhttp->httpResponse()->getStatusCode(), mredirectUrl.c_str());
			ret = CMS_ERROR;
		}
		else
		{
			//error
			logs->error("%s [CHttpClient::doDecode] http %s code %d rsp %s ***",
				mhttp->remoteAddr().c_str(), mhttp->getUrl().c_str(),
				mhttp->httpResponse()->getStatusCode(),
				mhttp->httpResponse()->getResponse().c_str());
			ret = CMS_ERROR;
		}
	}
	return ret;
}

int  CHttpClient::doReadData()
{
	int ret = 0;
	int len = DEFAULT_BUFFER_SIZE;
	char data[DEFAULT_BUFFER_SIZE];
	char *p = data;
	do
	{
		ret = mhttp->read(&p, len - 1);
		if (ret <= 0)
		{
			if (mhttp->isFinish())
			{
				//http 完成
				logs->debug("%s [CHttpClient::doReadData] http %s is finish.",
					mhttp->remoteAddr().c_str(), mhttp->getUrl().c_str());
				mcb(NULL, 0, this, mcustomData);
			}
			break;
		}
 		p[ret] = 0;
// 		logs->debug("%s [CHttpClient::doReadData]: %s.",
// 			mhttp->getUrl().c_str(), p);
		if (mcb(p, ret, this, mcustomData) < 0)
		{
			break;
		}
	} while (true);
	return ret;

}

int  CHttpClient::doTransmission()
{
	if (mpostData.length() > 0)
	{
		int len = mpostData.length();
		mhttp->write(mpostData.c_str(), len);
		mpostData.clear();
	}
	return CMS_OK;
}

int  CHttpClient::sendBefore(const char *data, int len)
{
	return CMS_OK;
}

bool CHttpClient::isFinish()
{
	return false;
}

bool CHttpClient::isWebsocket()
{
	return false;
}
