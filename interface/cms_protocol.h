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
#ifndef __CMS_INTERFACE_PROTOCOL_H__
#define __CMS_INTERFACE_PROTOCOL_H__
#include <flvPool/cms_flv_pool.h>
#include <string>

class CProtocol
{
public:
	CProtocol();
	virtual ~CProtocol();
	virtual int sendMetaData(Slice *s) = 0;
	virtual int sendVideoOrAudio(Slice *s,uint32 uiTimestamp) = 0;
	virtual int writeBuffSize() = 0;
	virtual void setWriteBuffer(int size) = 0;
	virtual std::string remoteAddr() = 0;
	virtual std::string getUrl() = 0;
	virtual bool isCmsConnection() = 0;
	virtual std::string protocol() = 0;
};
#endif
