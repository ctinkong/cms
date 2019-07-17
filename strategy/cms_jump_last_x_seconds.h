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
#ifndef __CMS_JUMP_LAST_X_SECONDS_H__
#define __CMS_JUMP_LAST_X_SECONDS_H__
#include <common/cms_type.h>
#include <mem/cms_mf_mem.h>
#include <string>

//判断源流是否卡顿 如果是，则需要发送旧x秒前的数据--主要提高流畅度统计
class CJumpLastXSeconds
{
public:
	CJumpLastXSeconds();
	~CJumpLastXSeconds();

	bool	isInit();
	void	init(std::string remoteAddr, std::string modeName, std::string url, uint32 hashIdx, HASH &hash);
	void	reset();

	void	record();
	bool    judge(int32 dataType, int64 &jumpIdx, int64 tt, int sliceNum, int frameRate, uint32 sendTimestamp);

	OperatorNewDelete
private:
	bool			misInit;
	std::string		murl;
	std::string		mremoteAddr;
	std::string		modeName;
	uint32			mhashIdx;
	HASH			mhash;

	int64			mlastJumpXMilSeconds;
	int64			mlastJumpXSecondsDropTime;
	int64			mlastJumpXSecondsSendFrames;
	uint32			mno1VideoTimestamp;					//首帧视频时间戳
	int64			mconnectTimestamp;					//连接时间戳
	uint32			mdeltaLastVideoTimestamp;
	int64			mlastDiffer;
	int64			mloseBufferInterval;
	int				mloseBufferTimes;
};
#endif
