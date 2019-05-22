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
#ifndef __CMS_READ_WRITE_H__
#define __CMS_READ_WRITE_H__
#include <string>
#include <net/cms_net_var.h>

class CReaderWriter
{
public:
	CReaderWriter();
	virtual ~CReaderWriter();
	virtual int   read(char *dstBuf, int len, int &nread) = 0;
	virtual int   write(char *srcBuf, int len, int &nwrite) = 0;
	virtual int   remoteAddr(char *addr, int len) = 0;
	virtual int   localAddr(char *addr, int len) = 0;
	virtual int   fd() = 0;
	virtual void  close() = 0;
	virtual char  *errnoCode() = 0;
	virtual int   errnos() = 0;
	virtual int   setNodelay(int on) = 0;
	virtual int	  setReadBuffer(int size) = 0;
	virtual int	  setWriteBuffer(int size) = 0;
	virtual int   flushR() = 0;
	virtual int   flushW() = 0;
	virtual int   netType() = 0;
};

#endif