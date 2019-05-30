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
#include <net/cms_tcp_conn.h>
#include <log/cms_log.h>
#include <common/cms_utility.h>
#include <dnscache/cms_dns_cache.h>
#include <core/cms_errno.h>
#include <s2n/s2n.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <string>


TCPConn::TCPConn(int fd)
{
	mreadTimeout = 1;
	mreadBytes = 0;
	mwriteTimetou = 1;
	mwriteBytes = 0;
	mfd = fd;
	struct sockaddr_in from;
	socklen_t len = sizeof(from);
	if (getpeername(fd, (struct sockaddr *)&from, &len) != -1)
	{
		char szAddr[25] = { 0 };
		ipInt2ipStr(from.sin_addr.s_addr, szAddr);
		snprintf(szAddr + strlen(szAddr), sizeof(szAddr) - strlen(szAddr), ":%d", ntohs(from.sin_port));
		mraddr = szAddr;
	}
	else
	{
		logs->error("*** [TCPConn::TCPConn] getpeername fail,errno=%d,strerr=%s ***", errno, strerror(errno));
	}
	sockaddr_in saddr;
	memset(&saddr, 0, sizeof(saddr));
	len = sizeof(saddr);

	if (getsockname(mfd, (sockaddr*)&saddr, &len) != -1)
	{
		char szAddr[25] = { 0 };
		ipInt2ipStr(saddr.sin_addr.s_addr, szAddr);
		snprintf(szAddr + strlen(szAddr), sizeof(szAddr) - strlen(szAddr), ":%d", ntohs(saddr.sin_port));
		mladdr = szAddr;
		logs->info("##### TCPConn accept local addr %s fd=%d #####", mladdr.c_str(), mfd);
	}
	else
	{
		logs->error("*** [TCPConn::TCPConn] getpeername fail,errno=%d,strerr=%s ***", errno, strerror(errno));
	}
}

TCPConn::TCPConn()
{
	mreadTimeout = 1;
	mreadBytes = 0;
	mwriteTimetou = 1;
	mwriteBytes = 0;
	mfd = -1;
	merrcode = 0;
}

TCPConn::~TCPConn()
{
	if (mfd != -1)
	{
		::close(mfd);
		mfd = -1;
	}
}


int TCPConn::dialTcp(char *addr, ConnType connectType)
{
	std::string sAddr;
	sAddr.append(addr);
	mconnectType = connectType;
	std::string strHost;
	unsigned short port;
	size_t pos = sAddr.find(":");
	if (pos == std::string::npos)
	{
		logs->error("*** TCPConn dialTcp addr %s is illegal *****", addr);
		return CMS_ERROR;
	}
	strHost = sAddr.substr(0, pos);
	port = (unsigned short)atoi(sAddr.substr(pos + 1).c_str());
	mfd = socket(AF_INET, SOCK_STREAM, 0);
	if (mfd == CMS_INVALID_SOCK)
	{
		logs->error("*** TCPConn dialTcp create socket is error,errno=%d,errstr=%s *****", errno, strerror(errno));
		return CMS_ERROR;
	}
	memset(&mto, 0, sizeof(mto));
	mto.sin_family = AF_INET;
	mto.sin_port = htons(port);
	unsigned long ip;
	if (!CDnsCache::instance()->host2ip(strHost.c_str(), ip))
	{
		logs->error("*** TCPConn dialTcp dns cache error *****");
		::close(mfd);
		mfd = -1;
		return CMS_ERROR;
	}
	mto.sin_addr.s_addr = ip;
	char szAddr[25] = { 0 };
	ipInt2ipStr(ip, szAddr);
	snprintf(szAddr + strlen(szAddr), sizeof(szAddr) - strlen(szAddr), ":%d", port);
	mraddr = szAddr;
	logs->info("##### TCPConn dialTcp addr %s fd=%d #####", mraddr.c_str(), mfd);
	return CMS_OK;
}

int	  TCPConn::connect()
{
	nonblocking(mfd);
	if (::connect(mfd, (struct sockaddr *)&mto, sizeof(mto)) < 0)
	{
		if (errno != EINPROGRESS)
		{
			logs->error("*** [TCPConn::connect] connect socket is error,errno=%d,errstr=%s *****", errno, strerror(errno));
			::close(mfd);
			return CMS_ERROR;
		}
		sockaddr_in saddr;
		memset(&saddr, 0, sizeof(saddr));
		socklen_t len = sizeof(saddr);
		if (getsockname(mfd, (sockaddr*)&saddr, &len) != -1)
		{
			char szAddr[25] = { 0 };
			ipInt2ipStr(saddr.sin_addr.s_addr, szAddr);
			snprintf(szAddr + strlen(szAddr), sizeof(szAddr) - strlen(szAddr), ":%d", ntohs(saddr.sin_port));
			mladdr = szAddr;
			logs->info("##### TCPConn connect local addr %s fd=%d #####", mraddr.c_str(), mfd);
		}
	}
	logs->info("##### TCPConn connect addr %s succ #####", mladdr.c_str());
	return CMS_OK;
}

int   TCPConn::read(char* dstBuf, int len, int &nread)
{
	int nbRead = recv(mfd, dstBuf, len, 0);
	if (nbRead <= 0)
	{
		if (nbRead < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
			return CMS_OK;
		}
		if (nbRead == 0)
		{
			merrcode = CMS_ERRNO_FIN;
		}
		else
		{
			merrcode = errno;
		}
		return CMS_ERROR;
	}
	mreadBytes += (long long)nbRead;
	nread = nbRead;
	return CMS_OK;
}

int   TCPConn::write(char *srcBuf, int len, int &nwrite)
{
	int nbWrite = send(mfd, srcBuf, len, 0);
	if (nbWrite <= 0)
	{
		if (nbWrite < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
			return CMS_OK;
		}
		if (nbWrite == 0)
		{
			merrcode = CMS_ERRNO_FIN;
		}
		else
		{
			//logs->info("##### TCPConn write addr %s fail,fd=%d #####",mraddr.c_str(),mfd);
			merrcode = errno;
		}
		return CMS_ERROR;
	}
	mwriteBytes += (long long)nbWrite;
	nwrite = nbWrite;
	return CMS_OK;
}

char  *TCPConn::errnoCode()
{
	return cmsStrErrno(merrcode);
}

int  TCPConn::errnos()
{
	return merrcode;
}

int TCPConn::setNodelay(int on)
{
	return setsockopt(mfd, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));
}

int	TCPConn::setReadBuffer(int size)
{
	return setsockopt(mfd, IPPROTO_TCP, SO_RCVBUF, (void *)&size, sizeof(size));
}

int	TCPConn::setWriteBuffer(int size)
{
	return setsockopt(mfd, IPPROTO_TCP, SO_SNDBUF, (void *)&size, sizeof(size));
}

int TCPConn::remoteAddr(char *addr, int len)
{
	memcpy(addr, mraddr.c_str(), cmsMin(len, (int)mraddr.length()));
	return CMS_OK;
}

int TCPConn::localAddr(char *addr, int len)
{
	memcpy(addr, mladdr.c_str(), cmsMin(len, (int)mladdr.length()));
	return CMS_OK;
}

void  TCPConn::setReadTimeout(long long readTimeout)
{
	mreadTimeout = readTimeout;
}

long long TCPConn::getReadTimeout()
{
	return mreadTimeout;
}

void  TCPConn::setWriteTimeout(long long writeTimeout)
{
	mwriteTimetou = writeTimeout;
}

long long TCPConn::getWriteTimeout()
{
	return mwriteTimetou;
}

long long TCPConn::getReadBytes()
{
	return mreadBytes;
}

long long TCPConn::getWriteBytes()
{
	return mwriteBytes;
}

void TCPConn::close()
{
	if (mfd != -1)
	{
		::close(mfd);
		mfd = -1;
	}
}

ConnType TCPConn::connectType()
{
	return mconnectType;
}

int TCPConn::fd()
{
	return mfd;
}

int TCPConn::flushR()
{
	//tcp 不需要实现
	return CMS_OK;
}

int TCPConn::flushW()
{
	//tcp 不需要实现
	return CMS_OK;
}

TCPListener::TCPListener()
{
	mruning = false;
	mfd = -1;
}

TCPListener::~TCPListener()
{

}

int  TCPListener::listen(char* addr, ConnType listenType)
{
	std::string sAddr;
	sAddr.append(addr);
	mlistenAddr = addr;
	struct sockaddr_in serv_addr;
	std::string strHost;
	unsigned short port;
	size_t pos = sAddr.find(":");
	if (pos == std::string::npos)
	{
		logs->error("*** TCPListener addr %s is illegal *****", addr);
		return CMS_ERROR;
	}
	strHost = sAddr.substr(0, pos);
	port = (unsigned short)atoi(sAddr.substr(pos + 1).c_str());
	logs->info("##### TCPListener listen addr %s #####", addr);
	if (strHost.length() == 0)
	{
		strHost = "0.0.0.0";
	}
	mfd = socket(AF_INET, SOCK_STREAM, 0);
	if (mfd == CMS_INVALID_SOCK)
	{
		logs->error("*** TCPListener listen create socket is error,errno=%d,errstr=%s *****", errno, strerror(errno));
		return CMS_ERROR;
	}
	int n = 1;
	if (setsockopt(mfd, SOL_SOCKET, SO_REUSEADDR, (char *)&n, sizeof(n)) < 0)
	{
		logs->error("*** TCPListener listen set SO_REUSEADDR fail,errno=%d,errstr=%s *****", errno, strerror(errno));
	}
	nonblocking(mfd);
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	unsigned long ip;
	if (!CDnsCache::instance()->host2ip(strHost.c_str(), ip))
	{
		logs->error("*** TCPListener listen dns cache error *****");
		::close(mfd);
		mfd = -1;
		return CMS_ERROR;
	}
	serv_addr.sin_addr.s_addr = ip;
	if (bind(mfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		logs->error("*** TCPListener listen bind %s socket is error,errno=%d,errstr=%s *****", mlistenAddr.c_str(), errno, strerror(errno));
		::close(mfd);
		mfd = -1;
		return CMS_ERROR;
	}
	if (::listen(mfd, 256) < 0)
	{
		logs->error("*** TCPListener listen ::listen socket is error,errno=%d,errstr=%s *****", errno, strerror(errno));
		::close(mfd);
		mfd = -1;
		return CMS_ERROR;
	}
	mruning = true;
	//createThread(this);
	mlistenType = listenType;
	return CMS_OK;
}

ConnType TCPListener::listenType()
{
	return mlistenType;
}

bool TCPListener::isTcp()
{
	return true;
}

void *TCPListener::oneConn()
{
	return NULL;
}

void TCPListener::stop()
{
	logs->info("### TCPListener begin stop listening %s ###", mlistenAddr.c_str());
	if (mruning)
	{
		mruning = false;
		::close(mfd);
		mfd = -1;
	}
	logs->info("### TCPListener finish stop listening %s ###", mlistenAddr.c_str());
}

int  TCPListener::accept()
{
	int cnfd = -1;
	struct sockaddr_in from;
	socklen_t fromlen;
	fromlen = sizeof(from);
	cnfd = ::accept(mfd, (struct sockaddr *)&from, &fromlen);
	if (cnfd == -1) {
		if (errno != EAGAIN)
		{
			logs->error("*** TCPListener can't accept connection err=%d,strerr=%s ***", errno, strerror(errno));
		}
		return CMS_ERROR;
	}
	return cnfd;
}

int  TCPListener::fd()
{
	return mfd;
}


