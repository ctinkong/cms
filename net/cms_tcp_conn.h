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
#ifndef __CMS_TCP_CONN_H__
#define __CMS_TCP_CONN_H__
#include <interface/cms_read_write.h>
#include <interface/cms_conn_listener.h>
#include <common/cms_type.h>
#include <core/cms_lock.h>
#include <netinet/in.h>
#include <string>
#include <queue>
using namespace std;

class TCPConn :public CReaderWriter
{
public:
	TCPConn(int fd);
	TCPConn();
	~TCPConn();

	int   dialTcp(char *addr, ConnType connectType);
	int	  connect();
	int   read(char* dstBuf, int len, int &nread);
	int   write(char *srcBuf, int len, int &nwrite);
	void  close();
	int   fd();
	int   remoteAddr(char *addr, int len);
	int   localAddr(char *addr, int len);
	void  setReadTimeout(long long readTimeout);
	long long getReadTimeout();
	void  setWriteTimeout(long long writeTimeout);
	long long getWriteTimeout();
	long long getReadBytes();
	long long getWriteBytes();
	ConnType connectType();
	char  *errnoCode();
	int   errnos();
	int   setNodelay(int on);
	int	  setReadBuffer(int size);
	int	  setWriteBuffer(int size);
	int   flushR();
	int   flushW();
	int   netType() { return NetTcp; };
private:
	int mfd;
	struct sockaddr_in mto;
	string mraddr;
	string mladdr;
	long long mreadTimeout;
	long long mreadBytes;
	long long mwriteTimetou;
	long long mwriteBytes;
	ConnType mconnectType;
	int  merrcode;
};

class TCPListener :public CConnListener
{
public:
	TCPListener();
	~TCPListener();
	//重载
	int  listen(char* addr, ConnType listenType);
	void stop();
	int  accept();
	int  fd();
	ConnType listenType();
	bool isTcp();
	void *oneConn();
private:
	string  mlistenAddr;
	bool	mruning;
	int     mfd;
	ConnType mlistenType;
};
#endif
