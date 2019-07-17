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
#ifndef __CMS_DURATION_TIMESTAMP_H__
#define __CMS_DURATION_TIMESTAMP_H__
#include <common/cms_type.h>
#include <mem/cms_mf_mem.h>
#include <string>

class CDurationTimestamp
{
public:
	CDurationTimestamp();
	~CDurationTimestamp();

	bool	isInit();
	void	init(std::string remoteAddr, std::string modeName, std::string url);
	void	setResetTimestamp(bool is);
	uint32  resetTimestamp(uint32 timestamp, bool isVideo);
	void	resetDeltaTimestamp(uint32 timestamp);
	uint32	keepTimestampIncrease(bool isVideo, uint32 timestamp);

	OperatorNewDelete
private:
	bool			misInit;
	std::string		murl;
	std::string		mremoteAddr;
	std::string		modeName;

	//重设时间戳
	bool	misStreamResetTimestamp;  //开启重设时间戳标志
	uint32	mvideoResetTimestamp;     //记录视频时间戳
	uint32	maudioResetTimestamp;     //记录音频时间戳
		//播放连接保持时间戳连续性 只有用户播放（或者转码、切片、截图、录制、转推才使用，保留）
	uint32	mdeltaVideoTimestamp;
	uint32	mdeltaLastVideoTimestamp;
	uint32	mdeltaAudioTimestamp;
	uint32	mdeltaLastAudioTimestamp;
};
#endif
