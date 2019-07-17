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
#ifndef __CMS_BUFFER_H__
#define __CMS_BUFFER_H__
#include <interface/cms_read_write.h>
#include <common/cms_type.h>
#include <s2n/s2n.h>
#include <mem/cms_mf_mem.h>

#define DEFAULT_BUFFER_SIZE (32*1024)

class CBufferReader
{
public:
	CBufferReader(CReaderWriter *rd, int size);
	CBufferReader(s2n_connection *s2nConn, int size);
	~CBufferReader();
	char  *readBytes(int n);
	char  *peek(int n);
	char  readByte();
	void  skip(int n);
	int   size();
	int   grow(int n);
	char  *errnoCode();
	int   errnos();
	void  close();
	int32 readBytesNum();

	OperatorNewDelete
private:
	void  resize();
	int   seterrno(int err);
	char *mbuffer;
	int  mb;
	int  me;
	int  mbufferSize;
	int  merrcode;
	int32 mtotalReadBytes;
	CReaderWriter *mrd;
	struct s2n_connection	*ms2nConn;

	//test
	int mkcpReadLen;
};

class CBufferWriter
{
public:
	CBufferWriter(CReaderWriter *rd, int size = DEFAULT_BUFFER_SIZE);
	CBufferWriter(s2n_connection *s2nConn, int size = DEFAULT_BUFFER_SIZE);
	~CBufferWriter();
	int writeBytes(const char *data, int n); //要么出错返回 -1,要么返回实际发送的数据,如果本次没发送完毕，会保存到底层
	int writeByte(char ch);
	int flush();
	bool  isUsable();
	void  resize();
	char  *errnoCode();
	int   errnos();
	int   size();
	void  close();
	int32 writeBytesNum();

	OperatorNewDelete
private:
	int   seterrno(int err);
	char *mbuffer;
	int  mb;
	int  me;
	int  mbufferSize;
	int  merrcode;
	int32 mtotalWriteBytes;
	CReaderWriter *mrd;
	struct s2n_connection	*ms2nConn;
};

class CByteReaderWriter
{
public:
	CByteReaderWriter(int size = DEFAULT_BUFFER_SIZE);
	~CByteReaderWriter();
	int	  writeBytes(const char *data, int n);
	int   writeByte(char ch);
	char  *readBytes(int n);
	char  readByte();
	char  *peek(int n);
	void  skip(int n);
	void  resize();
	int   size();

	OperatorNewDelete
private:
	char *mbuffer;
	int  mb;
	int  me;
	int  mbufferSize;
};
#endif
