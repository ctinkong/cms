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
#include <strategy/cms_duration_timestamp.h>

#define ResetTimestamp  100

CDurationTimestamp::CDurationTimestamp()
{
	misInit = false;
	//重设时间戳
	misStreamResetTimestamp = false;
	mvideoResetTimestamp = 0;
	maudioResetTimestamp = 0;
	//播放连接保持时间戳连续性 只有用户播放（或者转码、切片、截图、录制、转推才使用，保留）
	mdeltaVideoTimestamp = 0;
	mdeltaLastVideoTimestamp = 0;
	mdeltaAudioTimestamp = 0;
	mdeltaLastAudioTimestamp = 0;
}

CDurationTimestamp::~CDurationTimestamp()
{

}

bool CDurationTimestamp::isInit()
{
	return misInit;
}

void CDurationTimestamp::init(std::string remoteAddr, std::string modeName, std::string url)
{
	misInit = true;
	murl = url;
	mremoteAddr = remoteAddr;
	modeName = modeName;
}

void CDurationTimestamp::setResetTimestamp(bool is)
{
	misStreamResetTimestamp = is;
}

uint32 CDurationTimestamp::resetTimestamp(uint32 timestamp, bool isVideo)
{
	if (misStreamResetTimestamp)
	{
		if (isVideo)
		{
			if (mvideoResetTimestamp == 0)
			{
				mvideoResetTimestamp = timestamp - ResetTimestamp;
			}
		}
		else
		{
			if (maudioResetTimestamp == 0)
			{
				maudioResetTimestamp = timestamp - ResetTimestamp;
			}
		}
	}
	if (misStreamResetTimestamp)
	{
		if (isVideo)
		{
			if ((int32)timestamp - (int32)mvideoResetTimestamp >= 0)
			{
				//时间戳是递增的
				timestamp -= mvideoResetTimestamp;
			}
			else
			{
				//时间戳变小了?放弃修改 只重设时间戳增量 不纠正时间戳
			}
		}
		else
		{
			if ((int32)timestamp - (int32)maudioResetTimestamp >= 0)
			{
				//时间戳是递增的
				timestamp -= maudioResetTimestamp;
			}
			else
			{
				//时间戳变小了?放弃修改 只重设时间戳增量 不纠正时间戳
			}
		}
	}
	return timestamp;
}

void CDurationTimestamp::resetDeltaTimestamp(uint32 timestamp)
{
	if (mdeltaLastVideoTimestamp > mdeltaLastAudioTimestamp)
	{
		mdeltaVideoTimestamp = mdeltaLastVideoTimestamp - timestamp + 40;
		mdeltaAudioTimestamp = mdeltaLastVideoTimestamp - timestamp + 40;
	}
	else
	{
		mdeltaVideoTimestamp = mdeltaLastAudioTimestamp - timestamp + 23;
		mdeltaAudioTimestamp = mdeltaLastAudioTimestamp - timestamp + 23;
	}
}

uint32 CDurationTimestamp::keepTimestampIncrease(bool isVideo, uint32 timestamp)
{
	//如果任务重启了,预防时间戳异常
	if (isVideo)
	{
		timestamp += mdeltaVideoTimestamp;
		mdeltaLastVideoTimestamp = timestamp;
	}
	else
	{
		timestamp += mdeltaAudioTimestamp;
		mdeltaLastAudioTimestamp = timestamp;
	}
	return timestamp;
}
