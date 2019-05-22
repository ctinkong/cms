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
#include <protocol/cms_ssl.h>
#include <log/cms_log.h>
#include <common/cms_utility.h>
#include <config/cms_config.h>
#include <app/cms_app_info.h>
#include <assert.h>

CSSL::CSSL(int fd, std::string remoteAddr, bool isClient)
{
	ms2nConn = NULL;
	mconfig = NULL;
	misTlsHandShake = false;
	misClient = isClient;
	mfd = fd;
	mremoteAddr = remoteAddr;
	mrdBuff = NULL;
	mwrBuff = NULL;
	mchain8Key = NULL;
}

CSSL::~CSSL()
{
	if (ms2nConn)
	{
		//s2n_connection_wipe 可通过该函数擦除 s2n_connection 关联数据，用于重用 s2n_connection 变量
		s2n_blocked_status blocked;
		s2n_shutdown(ms2nConn, &blocked);
		s2n_connection_wipe(ms2nConn);
		s2n_connection_free(ms2nConn);
		ms2nConn = NULL;
	}
	if (mconfig)
	{
		assert(misClient);
		if (s2n_config_free(mconfig) < 0)
		{
			logs->error("***** %s [CSSL::CSSL] error freeing configuration: '%s' *****",
				mremoteAddr.c_str(), s2n_strerror(s2n_errno, "EN"));
		}
	}
	if (mchain8Key)
	{
		if (s2n_cert_chain_and_key_free(mchain8Key) < 0)
		{
			logs->error("***** %s [CSSL::CSSL] error s2n_cert_chain_and_key_free: '%s' *****",
				mremoteAddr.c_str(), s2n_strerror(s2n_errno, "EN"));
		}
	}
	if (mrdBuff)
	{
		delete mrdBuff;
	}
	if (mwrBuff)
	{
		delete mwrBuff;
	}
}

bool CSSL::run()
{
	if (misClient)
	{
		ms2nConn = s2n_connection_new(S2N_CLIENT);
		mconfig = s2n_config_new();
		if (mconfig == NULL) {
			logs->error("***** %s [CSSL::run] error getting new config: '%s' *****",
				mremoteAddr.c_str(), s2n_strerror(s2n_errno, "EN"));
			return false;
		}

		if (s2n_config_set_status_request_type(mconfig, S2N_STATUS_REQUEST_OCSP) < 0) {
			logs->error("***** %s [CSSL::run] error setting status request type: '%s' *****",
				mremoteAddr.c_str(), s2n_strerror(s2n_errno, "EN"));
			return false;
		}
		if (s2n_connection_set_config(ms2nConn, mconfig) < 0)
		{
			logs->error("***** %s [CSSL::run] error client setting configuration: '%s' *****",
				mremoteAddr.c_str(), s2n_strerror(s2n_errno, "EN"));
			return false;
		}
		if (s2n_set_server_name(ms2nConn, APP_NAME) < 0)
		{
			logs->error("***** %s [CSSL::run] error setting server name: '%s' *****",
				mremoteAddr.c_str(), s2n_strerror(s2n_errno, "EN"));
			return false;
		}
	}
	else
	{
		ms2nConn = s2n_connection_new(S2N_SERVER);
		s2n_connection_set_blinding(ms2nConn, S2N_SELF_SERVICE_BLINDING);//防止异常导致阻塞，请参考api		
		if (s2n_connection_set_config(ms2nConn, CConfig::instance()->certKey()->s2nConfig()) < 0)
		{
			logs->error("***** %s [CSSL::run] error server setting configuration: '%s' *****",
				mremoteAddr.c_str(), s2n_strerror(s2n_errno, "EN"));
			return false;
		}
	}
	//s2n_connection_prefer_throughput(ms2nConn);//优先考虑吞吐量的连接将使用大的记录大小来最小化开销
	if (s2n_connection_prefer_low_latency(ms2nConn))//优先考虑低延迟的连接将使用可以由接收者更快地解密的小记录大小进行加密
	{
		logs->error("***** %s [CSSL::run] error s2n_connection_prefer_low_latency fail: '%s' *****",
			mremoteAddr.c_str(), s2n_strerror(s2n_errno, "EN"));
		return false;
	}
	if (s2n_connection_set_fd(ms2nConn, mfd) < 0)
	{
		logs->error("***** %s [CSSL::run] error setting file descriptor: '%s' *****",
			mremoteAddr.c_str(), s2n_strerror(s2n_errno, "EN"));
		return false;
	}
	return true;
}

bool CSSL::isHandShake()
{
	return misTlsHandShake;
}

int CSSL::read(char **data, int &len)
{
	if (/*!misClient && */!misTlsHandShake)
	{
		int ret = handShakeTLS();
		if (ret == -1)
		{
			return -1;
		}
		if (ret == 0)
		{
			return 0;
		}
	}
	if (mrdBuff->size() < len && mrdBuff->grow(len) == CMS_ERROR)
	{
		return -1;
	}
	if (mrdBuff->size() < len)
	{
		return 0;
	}
	*data = mrdBuff->readBytes(len);
	return len;
}

int	CSSL::peek(char **data, int &len)
{
	if (!misClient && !misTlsHandShake)
	{
		int ret = handShakeTLS();
		if (ret == -1)
		{
			return -1;
		}
		if (ret == 0)
		{
			return 0;
		}
	}
	if (mrdBuff->size() < len && mrdBuff->grow(len) == CMS_ERROR)
	{
		return -1;
	}
	if (mrdBuff->size() < len)
	{
		return 0;
	}
	*data = mrdBuff->peek(len);
	return len;
}

void CSSL::skip(int len)
{
	mrdBuff->skip(len);
}

int CSSL::write(const char *data, int &len)
{
	if (/*misClient && */!misTlsHandShake)
	{
		int ret = handShakeTLS();
		if (ret == -1)
		{
			return -1;
		}
		if (ret == 0)
		{
			return 0;
		}
		if (data == NULL)
		{
			return 1;
		}
	}
	else
	{
		if (mwrBuff->writeBytes(data, len) == CMS_ERROR)
		{
			logs->error("***** %s [CSSL::write] write fail: '%s' *****",
				mremoteAddr.c_str(), mwrBuff->errnoCode());
			return -1;
		}
	}
	return len;
}

int CSSL::flush()
{
	if (!misTlsHandShake)
	{
		return CMS_OK;
	}
	int ret = mwrBuff->flush();
	if (ret == CMS_ERROR)
	{
		logs->error("%s [CSSL::flush] flush fail,errno=%d,strerrno=%s ***",
			mremoteAddr.c_str(), mwrBuff->errnos(), mwrBuff->errnoCode());
		return CMS_ERROR;
	}
	return CMS_OK;
}

bool CSSL::isUsable()
{
	return mwrBuff->isUsable();
}

int CSSL::bufferWriteSize()
{
	return mwrBuff->size();
}

int CSSL::handShakeTLS()
{
	s2n_blocked_status blocked;
	if (s2n_negotiate(ms2nConn, &blocked) < 0)
	{
		switch (s2n_error_get_type(s2n_errno)) {
		case S2N_ERR_T_BLOCKED:
			/* Blocked, come back later */
			return 0;
		default:
			break;
		}
		logs->error("***** %s [CSSL::handShakeTLS] failed to negotiate: '%s'.'%s', alert type=%d. %d *****",
			mremoteAddr.c_str(),
			s2n_strerror(s2n_errno, "EN"),
			s2n_strerror_debug(s2n_errno, "EN"),
			s2n_error_get_type(s2n_errno),
			s2n_connection_get_alert(ms2nConn));
		return -1;
	}
	if (blocked == S2N_NOT_BLOCKED)
	{
		logs->debug("%s [CSSL::handShakeTLS] TLS handshake succ ",
			mremoteAddr.c_str());
		misTlsHandShake = true;

		mrdBuff = new CBufferReader(ms2nConn, DEFAULT_BUFFER_SIZE);
		mwrBuff = new CBufferWriter(ms2nConn);
		//nonblocking(mfd);
		return 1;
	}
	return 0;
}

