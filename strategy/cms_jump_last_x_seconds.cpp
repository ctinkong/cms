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
#include <strategy/cms_jump_last_x_seconds.h>
#include <flvPool/cms_flv_pool.h>
#include <common/cms_utility.h>
#include <log/cms_log.h>
#include <stdlib.h>

#define JXDropVideoCheckInterval		200
#define JXDropVideoTimeDiffer			100
#define JXDropVideoClientBufLess		-800
#define JXDropVideoLoseBufMaxTime		10
#define JXDropMaxVideoLoseTime			3

CJumpLastXSeconds::CJumpLastXSeconds()
{
	misInit = false;
	mlastJumpXMilSeconds = 0;
	mlastJumpXSecondsDropTime = JXDropMaxVideoLoseTime;
	mlastJumpXSecondsSendFrames = 0;

	mdeltaLastVideoTimestamp = 0;
	mno1VideoTimestamp = 0;
	mconnectTimestamp = 0;

	mlastDiffer = 0;
	mloseBufferInterval = 0;
	mloseBufferTimes = 0;
}

CJumpLastXSeconds::~CJumpLastXSeconds()
{

}

bool CJumpLastXSeconds::isInit()
{
	return misInit;
}

void CJumpLastXSeconds::init(std::string remoteAddr, std::string modeName, std::string url, uint32 hashIdx, HASH &hash)
{
	misInit = true;
	murl = url;
	mremoteAddr = remoteAddr;
	modeName = modeName;
	mhashIdx = hashIdx;
	mhash = hash;
}

void CJumpLastXSeconds::reset()
{
	mlastJumpXMilSeconds = 0;
	mlastJumpXSecondsDropTime = JXDropMaxVideoLoseTime;
	mlastJumpXSecondsSendFrames = 0;

	mdeltaLastVideoTimestamp = 0;
	mno1VideoTimestamp = 0;
	mconnectTimestamp = 0;

	mlastDiffer = 0;
	mloseBufferInterval = 0;
	mloseBufferTimes = 0;
}

void CJumpLastXSeconds::record()
{
	mlastJumpXSecondsSendFrames++;
}

bool CJumpLastXSeconds::judge(int32 dataType, int64 &jumpIdx, int64 tt, int sliceNum, int frameRate, uint32 sendTimestamp)
{
	bool isChange = false;
	if (mlastJumpXMilSeconds == 0)
	{
		mlastJumpXMilSeconds = tt;
	}
	if (mconnectTimestamp == 0)
	{
		mconnectTimestamp = tt;
		mno1VideoTimestamp = sendTimestamp;
	}
	if (tt - mlastJumpXMilSeconds > 1000 * 5)
	{
		if (dataType == DATA_TYPE_VIDEO)
		{
			int64 guestBufLen = 0;
			int64 sendBufLen = 0;
			if (frameRate > 0)
			{
				guestBufLen = (int64)sliceNum * 1000 / (int64)frameRate;                         //转换成毫秒
				sendBufLen = (mlastJumpXSecondsSendFrames * 1000) / (int64)frameRate;			 //转换成毫秒
			}
			if ((int64)(sendTimestamp - mno1VideoTimestamp) < sendBufLen)
			{
				//logs->debug("one conn hsc %s,%s %s task before unfluency,tt timestamp=%lu,frame timestamp=%llu",
				//	mremoteAddr.c_str(), modeName.c_str(), murl.c_str(), sendTimestamp - mno1VideoTimestamp, sendBufLen);
				sendBufLen = int64(sendTimestamp - mno1VideoTimestamp);		//取时间戳统计与帧率统计最小的时长
			}
			int64 differ = (tt - mconnectTimestamp) - sendBufLen;			//differ		偏向负数 表示发送流畅，如果偏向正数 表示发送缓慢
			int64 compareDiffer = mlastDiffer - differ;						//compareDiffer	偏向正数 表示最近两次发送越来越流畅，偏向负数 表示最近两次发送越来越慢
			if (tt - mloseBufferInterval > JXDropVideoCheckInterval)		//200ms判断一次
			{
				if (MathAbs(compareDiffer) > JXDropVideoTimeDiffer)
				{
					if (compareDiffer < 0 && mloseBufferTimes < JXDropVideoLoseBufMaxTime)
					{
						mloseBufferTimes++;
					}
					if (compareDiffer > 0 && mloseBufferTimes > 0)
					{
						mloseBufferTimes--;
					}
				}
				mloseBufferInterval = tt;
				mlastDiffer = differ;
				//判断剩余缓冲数据小于 JXDropVideoClientBufLess
				if (differ >= (int64)(-DropVideoMinSeconds) && mloseBufferTimes >= mlastJumpXSecondsDropTime && guestBufLen < 1000)
				{
					int64 transIdx = -1;
					uint32 ts = 0;
					// 					logs->debug("one conn hsc %s,%s %s task before unfluency,sendTimestamp=%u",
					// 						mremoteAddr.c_str(), modeName.c_str(), murl.c_str(), sendTimestamp);
					bool ok = CFlvPool::instance()->justJump2VideoLastXSecond(mhashIdx, mhash, sendTimestamp, ts, transIdx);
					if (ok)
					{
						// 						logs->debug("one conn hsc %s,%s %s task maybe unfluency,ts=%u,sendTimestamp=%u",
						// 							mremoteAddr.c_str(), modeName.c_str(), murl.c_str(), ts, sendTimestamp);
						mlastJumpXSecondsDropTime = JXDropMaxVideoLoseTime;
						mlastJumpXMilSeconds = tt;
						jumpIdx = transIdx;
						mloseBufferTimes = 0;
						isChange = true;
					}
				}
			}
			else
			{
				if (differ > 3000)
				{
					mlastJumpXSecondsDropTime = (differ - 1000) / JXDropVideoCheckInterval;
					if (mlastJumpXSecondsDropTime > JXDropMaxVideoLoseTime)
					{
						mlastJumpXSecondsDropTime = JXDropMaxVideoLoseTime;
					}
				}
			}
		}
	}
	return isChange;
}
