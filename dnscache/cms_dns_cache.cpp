/*
The MIT License (MIT)

Copyright (c) 2017- cms(hsc)

Author: Ìì¿ÕÃ»ÓÐÎÚÔÆ/kisslovecsh@foxmail.com

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
#include <dnscache/cms_dns_cache.h>
#include <common/cms_utility.h>
#include <log/cms_log.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>

#define CdnTimeout (1000*60*5)
CDnsCache* CDnsCache::minstance = NULL;
CDnsCache::CDnsCache()
{

}

CDnsCache::~CDnsCache()
{

}

CDnsCache *CDnsCache::instance()
{
	if (minstance == NULL)
	{
		minstance = new CDnsCache();
	}
	return minstance;
}

void CDnsCache::freeInstance()
{
	if (minstance != NULL)
	{
		delete minstance;
		minstance = NULL;
	}
}

bool CDnsCache::host2ip(const char* host, unsigned long &ip)
{
	if (isLegalIp(host))
	{
		ip = ipStr2ipInt(host);
	}
	else
	{
		unsigned long tt = getTickCount();
		bool needGetHost = true;
		mHostInfoLock.Lock();
		MapHostInfoIter it = mmapHostInfo.find(host);
		if (it != mmapHostInfo.end() && tt - it->second->tt < CdnTimeout)
		{
			ip = it->second->ip;
			needGetHost = false;
		}
		mHostInfoLock.Unlock();
		if (needGetHost)
		{
			struct sockaddr_in saddr;
			struct addrinfo hints, *ares;
			int err;
			memset(&hints, 0, sizeof(struct addrinfo));
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_family = AF_INET;
			if ((err = getaddrinfo(host, NULL, &hints, &ares)) != 0)
			{
				logs->error("*** [CDnsCache::host2ip] can't resolve address: %s,errno=%d,strerrno=%s ***", host, errno, strerror(errno));
				return false;
			}
			saddr.sin_addr.s_addr = ((struct sockaddr_in*)(ares->ai_addr))->sin_addr.s_addr;
			ip = saddr.sin_addr.s_addr;
			char szIP[30] = { 0 };
			ipInt2ipStr(ip, szIP);
			
			logs->debug(">>>>>>[CDnsCache::host2ip] host %s resolve ip %s <<<<<", host, szIP);
			mHostInfoLock.Lock();
			MapHostInfoIter it = mmapHostInfo.find(host);
			if (it != mmapHostInfo.end())
			{
				it->second->ip = ip;
				it->second->tt = tt;
			}
			else
			{
				HostInfo *hi = new HostInfo;
				hi->ip = ip;
				hi->tt = tt;
				mmapHostInfo.insert(make_pair(host, hi));
			}
			mHostInfoLock.Unlock();
			freeaddrinfo(ares);
		}
	}
	return true;
}
