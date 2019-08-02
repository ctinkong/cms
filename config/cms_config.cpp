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
#include <config/cms_config.h>
#include <cJSON/cJSON.h>
#include <common/cms_utility.h>
#include <mem/cms_mf_mem.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string>
#include <algorithm>
using namespace std;

bool g_s2nInit = false;

int s2nInit()
{
	if (g_s2nInit)
	{
		return CMS_OK;
	}
	if (s2n_init() < 0)
	{
		printf("*** TCPListener error running s2n_init(): '%s' ***\n", s2n_strerror(s2n_errno, "EN"));
		return CMS_ERROR;
	}
	printf("s2nInit succ.\n");
	return CMS_OK;
}

CAddr::CAddr(char *addr, int defaultPort)
{
	assert(strlen(addr) < 128);
	mAddr = (char*)xmalloc(128);
	memset(mAddr, 0, 128);
	mHost = (char*)xmalloc(128);
	memset(mHost, 0, 128);
	strcpy(mAddr, addr);
	char *p = strstr(addr, ":");
	if (p == NULL)
	{
		strcpy(mHost, addr);
		mPort = defaultPort;
	}
	else
	{
		memcpy(mHost, addr, p - addr);
		mPort = atoi(p + 1);
	}
	printf("### CAddr addr=%s,defaultPort=%d,host=%s,port=%d ###\n", mAddr, defaultPort, mHost, mPort);
}

CAddr::~CAddr()
{
	if (mAddr)
	{
		xfree(mAddr);
		mAddr = NULL;
	}
	if (mHost)
	{
		xfree(mHost);
		mAddr = NULL;
	}
}

char *CAddr::addr()
{
	printf("### CAddr get addr=%s ###\n", mAddr);
	return mAddr;
}

char *CAddr::host()
{
	return mHost;
}

int  CAddr::port()
{
	return mPort;
}

LogLevel getLevel(string level)
{
	transform(level.begin(), level.end(), level.begin(), (int(*)(int))tolower);
	if (level == "off")
	{
		return OFF;
	}
	else if (level == "debug")
	{
		return DEBUG;
	}
	else if (level == "info")
	{
		return INFO;
	}
	else if (level == "warn")
	{
		return WARN;
	}
	else if (level == "error")
	{
		return ERROR1;
	}
	else if (level == "fatal")
	{
		return FATAL;
	}
	else if (level == "all_level")
	{
		return ALL_LEVEL;
	}
	return DEBUG;
}

Clog::Clog(char *path, char *level, bool console, int size)
{
	assert(path != NULL);
	int len = strlen(path);
	mpath = (char*)xmalloc(len + 1);
	memcpy(mpath, path, len);
	mpath[len] = '\0';
	if (size < 500 * 1024 * 1024)
	{
		msize = 500 * 1024 * 1024;
	}
	msize = size;
	mlevel = getLevel(level);
	mconsole = console;
}

Clog::~Clog()
{
	if (mpath)
	{
		xfree(mpath);
		mpath = NULL;
	}
}

char *Clog::path()
{
	return mpath;
}

int	Clog::size()
{
	return msize;
}

LogLevel Clog::level()
{
	return mlevel;
}

bool Clog::console()
{
	return mconsole;
}

CSSLCertKey::CSSLCertKey()
{
	mcertKeyNum = 0;
	msetingIdx = 0;
	misOpen = false;
	mcert = NULL;
	mkey = NULL;
	mcipherVersion = NULL;
	mdhparam = NULL;
	ms2nConfig = NULL;
}

CSSLCertKey::~CSSLCertKey()
{
	free();
}

void CSSLCertKey::free()
{
	for (int i = 0; i < mcertKeyNum; i++)
	{
		xfree(mcert[i]);
		xfree(mkey[i]);
	}
	if (mcertKeyNum > 0)
	{
		xfree(mcert);
		xfree(mkey);
	}
	if (mcipherVersion)
	{
		xfree(mcipherVersion);
	}
	if (mdhparam)
	{
		xfree(mdhparam);
	}
	if (ms2nConfig)
	{
		if (s2n_config_free(ms2nConfig) < 0)
		{
			printf("***** [CSSLCertKey::s2nConfig] error freeing configuration: '%s' *****", s2n_strerror(s2n_errno, "EN"));
		}
	}
	mcertKeyNum = 0;
	msetingIdx = 0;
	misOpen = false;
	mcert = NULL;
	mkey = NULL;
	mcipherVersion = NULL;
	mdhparam = NULL;
	ms2nConfig = NULL;
}

void CSSLCertKey::setCertKeyNum(int n)
{
	if (n <= 0)
	{
		return;
	}
	misOpen = true;
	mcertKeyNum = n;
	mcert = (char**)xmalloc(sizeof(char*)*n);
	mkey = (char**)xmalloc(sizeof(char*)*n);
}

bool CSSLCertKey::setCertKey(char *cert, char *key)
{
	if (msetingIdx >= mcertKeyNum)
	{
		return false;
	}
	int len = strlen(cert);
	mcert[msetingIdx] = (char*)xmalloc(len + 1);
	memcpy(mcert[msetingIdx], cert, len);
	mcert[msetingIdx][len] = 0;
	len = strlen(key);
	mkey[msetingIdx] = (char*)xmalloc(len + 1);
	memcpy(mkey[msetingIdx], key, len);
	mkey[msetingIdx][len] = 0;
	msetingIdx++;
	return true;
}

void CSSLCertKey::setdhparam(char *dhparam)
{
	if (mdhparam)
	{
		xfree(mdhparam);
	}
	int len = strlen(dhparam);
	mdhparam = (char*)xmalloc(len + 1);
	memcpy(mdhparam, dhparam, len);
	mdhparam[len] = 0;
}

void CSSLCertKey::setCipherVersion(char *v)
{
	if (mcipherVersion)
	{
		xfree(mcipherVersion);
	}
	int len = strlen(v);
	mcipherVersion = (char*)xmalloc(len + 1);
	memcpy(mcipherVersion, v, len);
	mcipherVersion[len] = 0;
}

int CSSLCertKey::getCertKeyNum()
{
	return mcertKeyNum;
}

bool CSSLCertKey::getCertKey(int i, char **cert, char**key)
{
	if (i < 0 || i >= mcertKeyNum)
	{
		return false;
	}
	*cert = mcert[i];
	*key = mkey[i];
	return true;
}

char *CSSLCertKey::getCipherVersion()
{
	return mcipherVersion;
}

char *CSSLCertKey::getDHparam()
{
	return mdhparam;
}

bool CSSLCertKey::isOpenSSL()
{
	return misOpen;
}

bool CSSLCertKey::createS2NConfig()
{
	if (s2nInit() == CMS_ERROR)
	{
		printf("***** [CSSLCertKey::createS2NConfig] s2nInit fail *****\n");
		return false;
	}
	ms2nConfig = s2n_config_new();
	if (ms2nConfig == NULL) {
		printf("***** [CSSLCertKey::createS2NConfig] 2 error getting new config: '%s' *****\n", s2n_strerror(s2n_errno, "EN"));
		return false;
	}
	for (int i = 0; i < getCertKeyNum(); i++)
	{
		//关联所有证书
		char *cert, *key;
		if (getCertKey(i, &cert, &key))
		{
			struct s2n_cert_chain_and_key *chain8Key = s2n_cert_chain_and_key_new();
			if (s2n_cert_chain_and_key_load_pem(chain8Key, cert, key) < 0) {
				printf("***** [CSSLCertKey::createS2NConfig] s2n_cert_chain_and_key_load_pem fail: '%s' *****\n", s2n_strerror(s2n_errno, "EN"));
				return false;
			}

			if (s2n_config_add_cert_chain_and_key_to_store(ms2nConfig, chain8Key) < 0) {
				printf("***** [CSSLCertKey::createS2NConfig] s2n_config_add_cert_chain_and_key_to_store fail: '%s' *****\n", s2n_strerror(s2n_errno, "EN"));
				return false;
			}
#ifdef __CMS_DEBUG__
			printf(" [CSSLCertKey::createS2NConfig] load cert and key succ.\n");
#endif
		}
	}
	//设置SSL握手阶段是否需要加密
	if (getDHparam() != NULL &&
		s2n_config_add_dhparams(ms2nConfig, getDHparam()) < 0)
	{
		printf("***** [CSSLCertKey::createS2NConfig] adding DH parameters fail: '%s' *****\n", s2n_strerror(s2n_errno, "EN"));
		return false;
	}
#ifdef __CMS_DEBUG__
	else if (getDHparam() != NULL)
	{
		printf(" [CSSLCertKey::createS2NConfig] load dhparam succ.\n");
	}
#endif		
	//设置s2n版本
	if (getCipherVersion() != NULL &&
		s2n_config_set_cipher_preferences(ms2nConfig, getCipherVersion()) < 0)
	{
		printf("***** [CSSLCertKey::createS2NConfig] setting cipher prefs fail: '%s' *****\n", s2n_strerror(s2n_errno, "EN"));
		return false;
	}
#ifdef __CMS_DEBUG__
	else if (getCipherVersion() != NULL)
	{
		printf(" [CSSLCertKey::createS2NConfig] load cipher version succ.\n");
	}
#endif
	return true;
}

struct s2n_config *CSSLCertKey::s2nConfig()
{
	return ms2nConfig;
}


CUpperAddr::CUpperAddr()
{

}

CUpperAddr::~CUpperAddr()
{

}

void CUpperAddr::addPull(std::string addr)
{
	mvPullAddr.push_back(addr);
}

std::string CUpperAddr::getPull(unsigned int i)
{
	if (mvPullAddr.empty())
	{
		return "";
	}
	i = i % mvPullAddr.size();
	return mvPullAddr.at(i);
}

void CUpperAddr::addPush(std::string addr)
{
	mvPushAddr.push_back(addr);
}

std::string CUpperAddr::getPush(unsigned int i)
{
	if (mvPushAddr.empty())
	{
		return "";
	}
	i = i % mvPushAddr.size();
	return mvPushAddr.at(i);
}


CUdpFlag::CUdpFlag()
{
	misOpenUdpPull = false;
	misOpenUdpPush = false;
	miUdpMaxConnNum = 20000;
}

CUdpFlag::~CUdpFlag()
{

}

bool CUdpFlag::isOpenUdpPull()
{
	return misOpenUdpPull;
}

bool CUdpFlag::isOpenUdpPush()
{
	return misOpenUdpPush;
}

void CUdpFlag::setUdp(bool isPull, bool isPush)
{
	misOpenUdpPull = isPull;
	misOpenUdpPush = isPush;
}

void CUdpFlag::setUdpConnNum(int n)
{
	if (n < 1024)
	{
		n = 1024;
	}
	miUdpMaxConnNum = n;
}

int CUdpFlag::udpConnNum()
{
	return miUdpMaxConnNum;
}


CWorkerCfg::CWorkerCfg()
{
	mworkerCount = 8;
}

CWorkerCfg::~CWorkerCfg()
{

}

void CWorkerCfg::setCount(int c)
{
	mworkerCount = c;
}

int	CWorkerCfg::getCount()
{
	return mworkerCount;
}

CMedia::CMedia()
{
	miFirstPlaySkipMilSecond = 0;
	misResetStreamTimestamp = true;
	misNoTimeout = false;
	miLiveStreamTimeout = 1000 * 30;
	miNoHashTimeout = 1000 * 15;
	misRealTimeStream = false;
	mllCacheTT = 1000 * 10;
	misOpenHls = false;
	mtsDuration = 3;
	mtsNum = 3;
	mtsSaveNum = 5;
	misOpenCover = false;
}

CMedia::~CMedia()
{

}

void CMedia::set(int iFirstPlaySkipMilSecond,
	bool  isResetStreamTimestamp,
	bool  isNoTimeout,
	int   iLiveStreamTimeout,
	int   iNoHashTimeout,
	bool  isRealTimeStream,
	int64 llCacheTT,
	bool  isOpenHls,
	int   tsDuration,
	int   tsNum,
	int   tsSaveNum,
	bool  isOpenCover)
{
	miFirstPlaySkipMilSecond = iFirstPlaySkipMilSecond;
	misResetStreamTimestamp = isResetStreamTimestamp;
	misNoTimeout = isNoTimeout;
	miLiveStreamTimeout = iLiveStreamTimeout;
	miNoHashTimeout = iNoHashTimeout;
	misRealTimeStream = isRealTimeStream;
	mllCacheTT = llCacheTT;
	misOpenHls = isOpenHls;
	mtsDuration = tsDuration;
	mtsNum = tsNum;
	mtsSaveNum = tsSaveNum;
	misOpenCover = isOpenCover;
}

int CMedia::getFirstPlaySkipMilSecond()
{
	return miFirstPlaySkipMilSecond;
}

bool CMedia::isResetStreamTimestamp()
{
	return misResetStreamTimestamp;
}

bool CMedia::isNoTimeout()
{
	return misNoTimeout;
}

int CMedia::getStreamTimeout()
{
	return miLiveStreamTimeout;
}


int CMedia::getNoHashTimeout()
{
	return miNoHashTimeout;
}

bool CMedia::isRealTimeStream()
{
	return misRealTimeStream;
}

int64 CMedia::getCacheTime()
{
	return mllCacheTT;
}

bool CMedia::isOpenHls()
{
	return misOpenHls;
}

int CMedia::getTsDuration()
{
	return mtsDuration;
}

int CMedia::getTsNum()
{
	return mtsNum;
}

int CMedia::getTsSaveNum()
{
	return mtsSaveNum;
}

bool CMedia::getCover()
{
	return misOpenCover;
}

CConfig *CConfig::minstance = NULL;
CConfig *CConfig::instance()
{
	if (minstance == NULL)
	{
		minstance = new CConfig();
	}
	return minstance;
}

void CConfig::freeInstance()
{
	if (minstance)
	{
		delete minstance;
		minstance = NULL;
	}
}

CConfig::CConfig()
{
	mHttp = NULL;
	mHttps = NULL;
	mRtmp = NULL;
	mQuery = NULL;
	muaAddr = NULL;
	mlog = NULL;
	mcertKey = NULL;
	mUdpFlag = NULL;
	mworker = new CWorkerCfg();
	mmedia = NULL;
}

CConfig::~CConfig()
{
	if (mHttp)
	{
		delete mHttp;
	}
	if (mHttps)
	{
		delete mHttps;
	}
	if (mRtmp)
	{
		delete mRtmp;
	}
	if (mQuery)
	{
		delete mQuery;
	}
	if (mlog)
	{
		delete mlog;
	}
	if (mcertKey)
	{
		delete mcertKey;
	}
	if (muaAddr)
	{
		delete muaAddr;
	}
	if (mUdpFlag)
	{
		delete mUdpFlag;
	}
	if (mworker)
	{
		delete mworker;
	}
	if (mmedia)
	{
		delete mmedia;
	}
}

bool CConfig::init(const char *configPath)
{
	FILE *fp = fopen(configPath, "rb");
	if (fp == NULL)
	{
		printf("*** [CConfig::init] open file %s fail,errno=%d,strerrno=%s ***\n",
			configPath, errno, strerror(errno));
		return false;
	}
	fseek(fp, 0, SEEK_END);
	long len = ftell(fp);
	if (len <= 0)
	{
		printf("*** [CConfig::init] file %s is empty=%ld ***\n",
			configPath, len);
		fclose(fp);
		return false;
	}
	char *data = (char*)xmalloc(len + 1);
	fseek(fp, 0, SEEK_SET);
	int n = fread(data, 1, len, fp);
	if (n != len)
	{
		printf("*** [CConfig::init] fread file %s fail,errno=%d,strerrno=%s ***\n",
			configPath, errno, strerror(errno));
		fclose(fp);
		return false;
	}
	data[len] = '\0';
	fclose(fp);

	cJSON *root = cJSON_Parse(data);
	if (root == NULL)
	{
		printf("*** [CConfig::init] config file %s parse json fail %s ***\n",
			configPath, data);
		xfree(data);
		return false;
	}

	xfree(data);
	data = NULL;
	//监听端口
	if (!parseListenJson(root, configPath))
	{
		cJSON_Delete(root);
		return false;
	}
	if (!parseNextHopJson(root, configPath))
	{
		cJSON_Delete(root);
		return false;
	}
	//udp 属性
	if (!parseUdpJson(root, configPath))
	{
		cJSON_Delete(root);
		return false;
	}
	//证书
	if (!parseTlsJson(root, configPath))
	{
		cJSON_Delete(root);
		return false;
	}
	if (!parseWorkerJson(root, configPath))
	{
		cJSON_Delete(root);
		return false;
	}
	//日志
	if (!parseLogJson(root, configPath))
	{
		cJSON_Delete(root);
		return false;
	}
	if (!parseMediaJson(root, configPath))
	{
		cJSON_Delete(root);
		return false;
	}

	cJSON_Delete(root);
	return true;
}

bool CConfig::parseListenJson(cJSON *root, const char *configPath)
{
	cJSON *listenObject = cJSON_GetObjectItem(root, "listen");
	if (listenObject == NULL)
	{
		printf("*** [CConfig::init] config file %s do not have [listen] ***\n",
			configPath);
		return false;
	}
	std::string httpListen, httpsListen, rtmpListen, queryListen;
	cJSON *T = cJSON_GetObjectItem(listenObject, "http");
	if (T == NULL || T->type != cJSON_String)
	{
		printf("*** [CConfig::init] config file %s listen term do not have http ***\n",
			configPath);
		return false;
	}
	httpListen = T->valuestring;
	T = cJSON_GetObjectItem(listenObject, "https");
	if (T == NULL || T->type != cJSON_String)
	{
		printf("*** [CConfig::init] config file %s listen term do not have https ***\n",
			configPath);
		return false;
	}
	httpsListen = T->valuestring;
	T = cJSON_GetObjectItem(listenObject, "rtmp");
	if (T == NULL || T->type != cJSON_String)
	{
		printf("*** [CConfig::init] config file %s listen term do not have rtmp ***\n",
			configPath);
		return false;
	}
	rtmpListen = T->valuestring;
	T = cJSON_GetObjectItem(listenObject, "query");
	if (T == NULL || T->type != cJSON_String)
	{
		printf("*** [CConfig::init] config file %s listen term do not have query ***\n",
			configPath);
		return false;
	}
	queryListen = T->valuestring;

	mHttp = new CAddr((char *)httpListen.c_str(), 80);
	mHttps = new CAddr((char *)httpsListen.c_str(), 443);
	mRtmp = new CAddr((char *)rtmpListen.c_str(), 1935);
	mQuery = new CAddr((char *)queryListen.c_str(), 8931);

	printf("config [listen]::: \n\trtmp=%s \n\thttp=%s \n\thttps=%s \n\tquery=%s\n",
		mRtmp->addr(),
		mHttp->addr(),
		mHttps->addr(),
		mQuery->addr());
	return true;
}

bool CConfig::parseNextHopJson(cJSON *root, const char *configPath)
{
	muaAddr = new CUpperAddr;
	//上层节点
	cJSON *upper = cJSON_GetObjectItem(root, "next_hop");
	if (upper != NULL)
	{
		//pull addr
		cJSON *pull = cJSON_GetObjectItem(upper, "pull");
		if (pull != NULL)
		{
			if (pull->type == cJSON_Array)
			{
				int iSize = cJSON_GetArraySize(pull);
				for (int i = 0; i < iSize; i++)
				{
					cJSON *T = cJSON_GetArrayItem(pull, i);
					if (T->type != cJSON_String)
					{
						printf("***** upper pull addr is not string *****\n");
						return false;
					}
					muaAddr->addPull(T->valuestring);
				}
			}
			else
			{
				printf("***** upper pull config is not array *****\n");
				return false;
			}
		}

		//push addr
		cJSON *push = cJSON_GetObjectItem(upper, "push");
		if (push != NULL)
		{
			if (push->type == cJSON_Array)
			{
				int iSize = cJSON_GetArraySize(push);
				for (int i = 0; i < iSize; i++)
				{
					cJSON *T = cJSON_GetArrayItem(push, i);
					if (T->type != cJSON_String)
					{
						printf("***** upper push addr is not string *****\n");
						return false;
					}
					muaAddr->addPush(T->valuestring);
				}
			}
			else
			{
				printf("***** upper push config is not array *****\n");
				return false;
			}
		}
	}
	return true;
}

bool CConfig::parseUdpJson(cJSON *root, const char *configPath)
{
	mUdpFlag = new CUdpFlag;
	cJSON *udp = cJSON_GetObjectItem(root, "udp");
	if (udp != NULL &&
		udp->type == cJSON_Object)
	{
		bool isOpenPull = false;
		bool isOpenPush = false;
		//pull flag
		cJSON *T = cJSON_GetObjectItem(udp, "open_pull");
		if (T != NULL &&
			(T->type == cJSON_True || T->type == cJSON_False))
		{
			if (T->type == cJSON_True)
			{
				isOpenPull = true;
			}
		}
		else
		{
			printf("***** udp open_pull config is not bool *****\n");
			return false;
		}
		//push flag
		T = cJSON_GetObjectItem(udp, "open_push");
		if (T != NULL &&
			(T->type == cJSON_True || T->type == cJSON_False))
		{
			if (T->type == cJSON_True)
			{
				isOpenPush = true;
			}
		}
		else
		{
			printf("***** udp open_push config is not bool *****\n");
			return false;
		}
		mUdpFlag->setUdp(isOpenPull, isOpenPush);
		//udp 最大连接数
		T = cJSON_GetObjectItem(udp, "number");
		if (T != NULL &&
			(T->type == cJSON_Number))
		{
			mUdpFlag->setUdpConnNum(T->valueint);
		}
	}
	return true;
}

bool CConfig::parseTlsJson(cJSON *root, const char *configPath)
{
	mcertKey = new CSSLCertKey();
	cJSON *tls = cJSON_GetObjectItem(root, "tls");
	if (tls != NULL
		&& tls->type == cJSON_Object)
	{
		std::string cert, key, dhparam, version;
		cJSON *T = NULL;
		cJSON *ck = cJSON_GetObjectItem(tls, "cert-key");
		if (ck != NULL && ck->type == cJSON_Array)
		{
			char *pcert = NULL;
			char *pkey = NULL;
			if (cJSON_GetArraySize(ck) > 0)
			{
				mcertKey->setCertKeyNum(cJSON_GetArraySize(ck));
			}
			for (int i = 0; i < cJSON_GetArraySize(ck); i++)
			{
				T = cJSON_GetArrayItem(ck, i);
				if (T == NULL || T->type != cJSON_Object)
				{
					printf("*** [CConfig::init] config file %s tls::cert-key term do not have object ***\n",
						configPath);
					return false;
				}
				cJSON *c = cJSON_GetObjectItem(T, "cert");
				cJSON *k = cJSON_GetObjectItem(T, "key");
				if (c == NULL || c->type != cJSON_String ||
					k == NULL || k->type != cJSON_String)
				{
					printf("*** [CConfig::init] config file %s tls::cert-key item[%d] is cert or key illegal ***\n",
						configPath, i);
					return false;
				}
				cert = c->valuestring;
				key = k->valuestring;

				if (!readAll(cert.c_str(), &pcert))
				{
					printf("*** [CConfig::init] open cert file fail,errstr=%s ***\n",
						strerror(errno));
					return false;
				}
				if (!readAll(key.c_str(), &pkey))
				{
					printf("*** [CConfig::init] open cert file fail,errstr=%s ***\n",
						strerror(errno));
					return false;
				}
				mcertKey->setCertKey(pcert, pkey);
				xfree(pcert);
				xfree(pkey);
			}
			T = cJSON_GetObjectItem(tls, "dhparam");
			if (T != NULL && T->type != cJSON_String)
			{
				printf("*** [CConfig::init] config file %s tls term do not have dhparam ***\n",
					configPath);
				return false;
			}
			else if (T != NULL)
			{
				dhparam = T->valuestring;
				char *pdhparam = NULL;
				if (!readAll(dhparam.c_str(), &pdhparam))
				{
					printf("*** [CConfig::init] open dhparam filefail,errstr=%s ***\n",
						strerror(errno));
					return false;
				}
				mcertKey->setdhparam(pdhparam);
				xfree(pdhparam);
			}

			T = cJSON_GetObjectItem(tls, "version");
			if (T != NULL && T->type != cJSON_String)
			{
				printf("*** [CConfig::init] config file %s tls term do not have version ***\n",
					configPath);
				return false;
			}
			else if (T != NULL)
			{
				version = T->valuestring;
				mcertKey->setCipherVersion((char*)version.c_str());
			}
		}
	}
	if (mcertKey->isOpenSSL())
	{
		if (!mcertKey->createS2NConfig())
		{
			printf("*** [CConfig::init] config file %s create s2n config fail ***\n",
				configPath);
			return false;
		}
	}
	return true;
}

bool CConfig::parseWorkerJson(cJSON *root, const char *configPath)
{
	cJSON *wroker = cJSON_GetObjectItem(root, "worker");
	if (wroker == NULL || wroker->type != cJSON_Object)
	{
		printf("*** [CConfig::init] config file %s do not have [worker] or illegal ***\n",
			configPath);
		return false;
	}
	cJSON *T = cJSON_GetObjectItem(wroker, "count");
	if (T == NULL || T->type != cJSON_Number)
	{
		printf("*** [CConfig::init] config file %s worker term do no have count ***\n",
			configPath);
		return false;
	}
	if (T->valueint > 0 && T->valueint <= 56)
	{
		mworker->setCount(T->valueint);
	}
	printf("config [worker]::: \n\tcount=%d\n", mworker->getCount());
	return true;
}

bool CConfig::parseLogJson(cJSON *root, const char *configPath)
{
	cJSON* log = cJSON_GetObjectItem(root, "log");
	if (log == NULL || log->type != cJSON_Object)
	{
		printf("*** [CConfig::init] config file %s do not have [log] ***\n",
			configPath);
		return false;
	}
	std::string file, level;
	int size = 0;
	cJSON *T = cJSON_GetObjectItem(log, "file");
	if (T == NULL || T->type != cJSON_String)
	{
		printf("*** [CConfig::init] config file %s log term do no have file ***\n",
			configPath);
		return false;
	}
	file = T->valuestring;

	T = cJSON_GetObjectItem(log, "size");
	if (T == NULL || T->type != cJSON_Number)
	{
		printf("*** [CConfig::init] config file %s log term do no have size ***\n",
			configPath);
		return false;
	}
	size = T->valueint;
	if (size < 1024 * 1024 * 10)
	{
		printf("*** [CConfig::init] config file %s log size must more than 10 MB ***\n",
			configPath);
		return false;
	}

	T = cJSON_GetObjectItem(log, "level");
	if (T == NULL || T->type != cJSON_String)
	{
		printf("*** [CConfig::init] config file %s log term do no have level ***\n",
			configPath);
		return false;
	}
	level = T->valuestring;

	bool console = false;
	T = cJSON_GetObjectItem(log, "console");
	if (T == NULL || T->type == cJSON_True)
	{
		console = true;
	}
	printf("config [log]::: \n\tfile=%s \n\tlevel=%s \n\tconsole=%s \n\tsize=%d\n",
		file.c_str(),
		level.c_str(),
		console ? "true" : "false",
		size);
	mlog = new Clog((char *)file.c_str(),
		(char *)level.c_str(),
		console,
		size);
	return true;
}

bool CConfig::parseMediaJson(cJSON *root, const char *configPath)
{
	mmedia = new CMedia();
	cJSON* media = cJSON_GetObjectItem(root, "media");
	if (media == NULL || media->type != cJSON_Object)
	{
		printf("*** [CConfig::init] config file %s do not have [media] ***\n",
			configPath);
		return false;
	}
	int   iFirstPlaySkipMilSecond = 0;		//首播丢帧
	bool  isResetStreamTimestamp = true;	//首播重设时间戳
	bool  isNoTimeout = false;				//
	int   iLiveStreamTimeout = 1000 * 30;	//直播流超时时间 单位毫秒
	int   iNoHashTimeout = 1000 * 10;		//播放任务不存在超时时间 单位毫秒
	bool  isRealTimeStream = false;			//低延时直播
	int64 llCacheTT = 1000 * 10;			//缓存时长 单位毫秒	
	bool  isOpenHls = false;
	int   tsDuration = 3;
	int   tsNum = 3;
	int   tsSaveNum = 5;
	bool  isOpenCover = false;
	cJSON *T = cJSON_GetObjectItem(media, "first_play_milsecond");
	if (T != NULL && T->type != cJSON_Number)
	{
		printf("*** [CConfig::init] config file %s media term first_play_milsecond illegal  ***\n",
			configPath);
		return false;
	}
	if (T)
	{
		iFirstPlaySkipMilSecond = T->valueint;
	}
	T = cJSON_GetObjectItem(media, "reset_timestamp");
	if (T != NULL && T->type != cJSON_True && T->type != cJSON_False)
	{
		printf("*** [CConfig::init] config file %s media term reset_timestamp illegal  ***\n",
			configPath);
		return false;
	}
	if (T)
	{
		if (T->type == cJSON_False)
		{
			isResetStreamTimestamp = false;
		}
	}
	T = cJSON_GetObjectItem(media, "no_timeout");
	if (T != NULL && T->type != cJSON_True && T->type != cJSON_False)
	{
		printf("*** [CConfig::init] config file %s media term no_timeout illegal  ***\n",
			configPath);
		return false;
	}
	if (T)
	{
		if (T->type == cJSON_True)
		{
			isNoTimeout = true;
		}
	}
	T = cJSON_GetObjectItem(media, "stream_timeout");
	if (T != NULL && T->type != cJSON_Number)
	{
		printf("*** [CConfig::init] config file %s media term stream_timeout illegal  ***\n",
			configPath);
		return false;
	}
	if (T)
	{
		if (T->valueint < 1000 * 5)
		{
			printf("*** [CConfig::init] config file %s media term stream_timeout must more than 5000 ms  ***\n",
				configPath);
			return false;
		}
		iLiveStreamTimeout = T->valueint;
	}
	T = cJSON_GetObjectItem(media, "no_hash_timeout");
	if (T != NULL && T->type != cJSON_Number)
	{
		printf("*** [CConfig::init] config file %s media term no_hash_timeout illegal  ***\n",
			configPath);
		return false;
	}
	if (T)
	{
		if (T->valueint < 1000 * 5)
		{
			printf("*** [CConfig::init] config file %s media term no_hash_timeout must more than 5000 ms  ***\n",
				configPath);
			return false;
		}
		iNoHashTimeout = T->valueint;
	}
	T = cJSON_GetObjectItem(media, "real_time_stream");
	if (T != NULL && T->type != cJSON_True && T->type != cJSON_False)
	{
		printf("*** [CConfig::init] config file %s media term real_time_stream illegal  ***\n",
			configPath);
		return false;
	}
	if (T)
	{
		if (T->type == cJSON_True)
		{
			isRealTimeStream = true;
		}
	}
	T = cJSON_GetObjectItem(media, "cache_ms");
	if (T != NULL && T->type != cJSON_Number)
	{
		printf("*** [CConfig::init] config file %s media term cache_ms illegal  ***\n",
			configPath);
		return false;
	}
	if (T)
	{
		if (T->valueint < 1000 * 3)
		{
			printf("*** [CConfig::init] config file %s media term cache_ms must more than 3000 ms  ***\n",
				configPath);
			return false;
		}
		llCacheTT = T->valueint;
	}
	T = cJSON_GetObjectItem(media, "open_hls");
	if (T != NULL && T->type != cJSON_True && T->type != cJSON_False)
	{
		printf("*** [CConfig::init] config file %s media term open_hls illegal  ***\n",
			configPath);
		return false;
	}
	if (T)
	{
		if (T->type == cJSON_True)
		{
			isOpenHls = true;
			T = cJSON_GetObjectItem(media, "ts_duradion");
			if (T != NULL && T->type != cJSON_Number)
			{
				printf("*** [CConfig::init] config file %s media term ts_duradion illegal  ***\n",
					configPath);
				return false;
			}
			if (T)
			{
				if (T->valueint < 1 || T->valueint>180)
				{
					printf("*** [CConfig::init] config file %s media term ts_duradion should be 1-180 s  ***\n",
						configPath);
					return false;
				}
				tsDuration = T->valueint;
			}
			T = cJSON_GetObjectItem(media, "ts_num");
			if (T != NULL && T->type != cJSON_Number)
			{
				printf("*** [CConfig::init] config file %s media term ts_num illegal  ***\n",
					configPath);
				return false;
			}
			if (T)
			{
				if (T->valueint < 1 || T->valueint>180)
				{
					printf("*** [CConfig::init] config file %s media term ts_num should be 1-180  ***\n",
						configPath);
					return false;
				}
				tsNum = T->valueint;
			}
			T = cJSON_GetObjectItem(media, "ts_save_num");
			if (T != NULL && T->type != cJSON_Number)
			{
				printf("*** [CConfig::init] config file %s media term ts_save_num illegal  ***\n",
					configPath);
				return false;
			}
			if (T)
			{
				if (T->valueint < tsNum)
				{
					printf("*** [CConfig::init] config file %s media term ts_save_num should more than ts_num  ***\n",
						configPath);
					return false;
				}
				tsSaveNum = T->valueint;
			}
		}
	}
	T = cJSON_GetObjectItem(media, "open_cover");
	if (T != NULL && T->type != cJSON_True && T->type != cJSON_False)
	{
		printf("*** [CConfig::init] config file %s media term open_cover illegal  ***\n",
			configPath);
		return false;
	}
	if (T->type == cJSON_True)
	{
		isOpenCover = true;
	}

	printf("config [media]::: "
		"\n\tfirstPlaySkipMilSecond=%d "
		"\n\tisResetStreamTimestamp=%s "
		"\n\tisNoTimeout=%s "
		"\n\tliveStreamTimeout=%d "
		"\n\tnoHashTimeout=%d "
		"\n\tisRealTimeStream=%s "
		"\n\tcacheMS=%lld "
		"\n\topenHls=%s "
		"\n\ttsDuradion=%d"
		"\n\ttsNum=%d "
		"\n\ttsSaveNum=%d"
		"\n\topenCover=%s\n",
		iFirstPlaySkipMilSecond,
		isResetStreamTimestamp ? "true" : "false",
		isNoTimeout ? "true" : "false",
		iLiveStreamTimeout,
		iNoHashTimeout,
		isRealTimeStream ? "true" : "false",
		llCacheTT,
		isOpenHls ? "true" : "false",
		tsDuration,
		tsNum,
		tsSaveNum,
		isOpenCover ? "true" : "false");

	mmedia->set(iFirstPlaySkipMilSecond,
		isResetStreamTimestamp,
		isNoTimeout,
		iLiveStreamTimeout,
		iNoHashTimeout,
		isRealTimeStream,
		llCacheTT,
		isOpenHls,
		tsDuration,
		tsNum,
		tsSaveNum,
		isOpenCover);
	return true;
}

CAddr *CConfig::addrHttp()
{
	return mHttp;
}

CAddr *CConfig::addrHttps()
{
	return mHttps;
}

CAddr *CConfig::addrRtmp()
{
	return mRtmp;
}

CAddr *CConfig::addrQuery()
{
	return mQuery;
}

CSSLCertKey *CConfig::certKey()
{
	return mcertKey;
}

Clog *CConfig::clog()
{
	return mlog;
}

CUpperAddr *CConfig::upperAddr()
{
	return muaAddr;
}

CUdpFlag *CConfig::udpFlag()
{
	return mUdpFlag;
}

CWorkerCfg *CConfig::workerCfg()
{
	return mworker;
}

CMedia *CConfig::media()
{
	return mmedia;
}

