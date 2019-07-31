/*
The MIT License (MIT)

Copyright (c) 2017- cms(hsc)

Author: ���û������/kisslovecsh@foxmail.com

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
#ifndef __CMS_INTERFACE_STREAM_INFO_H__
#define __CMS_INTERFACE_STREAM_INFO_H__
#include <common/cms_type.h>
#ifdef __CMS_CYCLE_MEM__
#include <mem/cms_cycle_mem.h>
#endif
#include <string>

class CStreamInfo
{
public:
	CStreamInfo();
	virtual ~CStreamInfo();

	virtual int		firstPlaySkipMilSecond() = 0;
	virtual bool	isResetStreamTimestamp() = 0;
	virtual bool	isNoTimeout() = 0;
	virtual int		liveStreamTimeout() = 0;
	virtual int		noHashTimeout() = 0;
	virtual bool	isRealTimeStream() = 0;
	virtual int64   cacheTT() = 0;
	virtual std::string &getRemoteIP() = 0;
	virtual std::string &getHost() = 0;
	virtual void    makeOneTask() = 0;
#ifdef __CMS_CYCLE_MEM__
	virtual CmsCycleMem *getCycMem() = 0;
#endif
};
#endif
