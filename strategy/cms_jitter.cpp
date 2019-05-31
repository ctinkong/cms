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
#include <strategy/cms_jitter.h>
#include <log/cms_log.h>
#include <stdlib.h>

CJitter::CJitter()
{
	misInit = false;
	mvideoFrameRate = 0;
	maudioFrameRate = 0;
	misOpenJitter = false;
	misForceJitter = false;
	mvideoaudioTimestampIllegal = 0;
	mvideoJitterTimestampIllegal = 0;
	mabsLastVideoTimestamp = 0;
	mvideoJitterFrameRate = 0;
	maudioJitterTimestampIllegal = 0;
	mabsLastAudioTimestamp = 0;
	maudioJitterFrameRate = 0;
	mvideoJitterNum = 0;
	mvideoJitterDetalTotal = 0;
	maudioJitterNum = 0;
	maudioJitterDetalTotal = 0;
	misJitterAudio = 0;
	misJitterVideo = 0;
	mvideoJitterCountFrameRate = 0;
	mvideoBeginJitterTimestamp = 0;
	mvideoJitterGrandTimestmap = 0;
	misCountVideoFrameRate = false;
	misLegallVideoFrameRate = false;
	maudioJitterCountFrameRate = 0;
	maudioBeginJitterTimestamp = 0;
	maudioJitterGrandTimestmap = 0;
	misCountAudioFrameRate = false;
	misLegallAudioFrameRate = false;
	mvideoAudioJitterBalance = 0;
}

CJitter::~CJitter()
{

}

bool CJitter::isInit()
{
	return misInit;
}

void CJitter::init(std::string remoteAddr, std::string modeName, std::string url)
{
	misInit = true;
	murl = url;
	mremoteAddr = remoteAddr;
	modeName = modeName;
}

void CJitter::reset()
{
	mvideoaudioTimestampIllegal = 0;
	mabsLastAudioTimestamp = 0.0;
	maudioJitterNum = 0;
	maudioJitterDetalTotal = 0.0;
	misJitterAudio = false;
	maudioJitterCountFrameRate = 0;
	maudioBeginJitterTimestamp = 0;
	misCountAudioFrameRate = false;
	misLegallAudioFrameRate = false;
	mvideoAudioJitterBalance = 0;
}

void CJitter::jitterTimestamp(float tba, float tbv, float lastTimestamp,
	uint32 curTimestamp, bool isVideo, float &rlastTimestamp, uint32 &rcurTimestamp)
{
	float audioFrameRate = tba;
	float videoFrameRate = tbv;
	float delta = (float)curTimestamp - lastTimestamp;
	if (isVideo)
	{
		float standarDelta = (float)(1000 / videoFrameRate);
		if ((int)delta <= (int)(standarDelta * 2 / 3) ||
			((int)delta > (int)(standarDelta * 2)))
		{
			mvideoJitterTimestampIllegal++;
		}
		else
		{
			mvideoJitterTimestampIllegal = 0;
		}
		//持续5秒时间戳有异常
		if (mvideoJitterTimestampIllegal > (int)(videoFrameRate * 5) ||
			misJitterVideo)
		{
			if (!misJitterVideo)
			{
				misJitterVideo = true;
				misJitterAudio = true;
				logs->debug("%s %s jitter task %s video timestamp unexpect too long", mremoteAddr.c_str(), modeName.c_str(), murl.c_str());
				//同步时间
				int32 duration = (int32)((int64)lastTimestamp - (int64)mabsLastAudioTimestamp);
				if (duration > 0)
				{
					mabsLastAudioTimestamp = lastTimestamp;
				}
				else
				{
					lastTimestamp = mabsLastAudioTimestamp;
				}
			}
			mvideoAudioJitterBalance++;
			delta = standarDelta;
			mvideoJitterDetalTotal += standarDelta;
			mvideoJitterGrandTimestmap += standarDelta;
			mvideoJitterNum++;

			if (mvideoJitterNum == (int)videoFrameRate)
			{
				//时间补偿
				float audioStandarDelta = (float)(1000 / audioFrameRate);
				float balance = audioStandarDelta * (float)mvideoAudioJitterBalance;
				if ((int64)mvideoJitterGrandTimestmap - (int64)maudioJitterGrandTimestmap < (int64)balance + (int64)(standarDelta * 2))
				{
					lastTimestamp = lastTimestamp + (1000.0 - mvideoJitterDetalTotal);
					mvideoJitterGrandTimestmap += (1000.0 - mvideoJitterDetalTotal);
				}
				mvideoJitterDetalTotal = 0.0;
				mvideoJitterNum = 0;
				mvideoAudioJitterBalance = 0;
			}
		}
	}
	else
	{
		float standarDelta = (float)(1000 / audioFrameRate);
		if ((int)delta <= (int)(standarDelta * 2 / 3) ||
			((int)delta > (int)(standarDelta * 2)))
		{
			maudioJitterTimestampIllegal++;
		}
		else
		{
			maudioJitterTimestampIllegal = 0;
		}
		//持续5秒时间戳有异常
		if (maudioJitterTimestampIllegal > (int)(audioFrameRate * 5) ||
			misJitterAudio)
		{
			if (!misJitterAudio)
			{
				misJitterAudio = true;
				misJitterVideo = true;
				logs->debug("%s %s jitter task %s audio timestamp unexpect too long", mremoteAddr.c_str(), modeName.c_str(), murl.c_str());
				//同步时间
				int32 duration = (int32)((int64)mabsLastVideoTimestamp - (int64)lastTimestamp);
				if (duration > 0)
				{
					lastTimestamp = mabsLastVideoTimestamp;
				}
				else
				{
					mabsLastVideoTimestamp = lastTimestamp;
				}
			}
			if (mvideoAudioJitterBalance > 0)
			{
				mvideoAudioJitterBalance--;
			}
			delta = standarDelta;
			maudioJitterDetalTotal += standarDelta;
			maudioJitterGrandTimestmap += standarDelta;
			maudioJitterNum++;

			if (maudioJitterNum == (int)audioFrameRate)
			{
				//时间补偿
				float videoStandarDelta = (float)(1000 / videoFrameRate);
				float balance = videoStandarDelta * (float)(mvideoAudioJitterBalance*-1);
				if ((int64)maudioJitterGrandTimestmap - (int64)mvideoJitterGrandTimestmap < (int64)balance + (int64)(standarDelta * 2))
				{
					lastTimestamp = lastTimestamp + (1000.0 - maudioJitterDetalTotal);
					maudioJitterGrandTimestmap += (1000.0 - maudioJitterDetalTotal);
				}
				maudioJitterDetalTotal = 0.0;
				maudioJitterNum = 0;
			}
		}
	}
	rlastTimestamp = lastTimestamp + delta;
	rcurTimestamp = uint32(lastTimestamp);
}

uint32  CJitter::judgeNeedJitter(bool isVideo, uint32 absoluteTimestamp)
{
	if (misLegallAudioFrameRate &&
		misLegallVideoFrameRate)
	{
		if (isVideo)
		{
			if ((int)mabsLastVideoTimestamp > 0)
			{
				if (!misJitterVideo)
				{
					//音视频不同步
					int32 duration = (int32)((int64)mabsLastVideoTimestamp - (int64)mabsLastAudioTimestamp);
					if (abs(duration) > 1000)
					{
						mvideoaudioTimestampIllegal++;
					}
					else
					{
						mvideoaudioTimestampIllegal = 0;
					}
					if (mvideoaudioTimestampIllegal > mvideoFrameRate * 5)
					{
						misJitterVideo = true;
						misJitterAudio = true;
						logs->debug("%s %s jitter task %s video audio differ too much", mremoteAddr.c_str(), modeName.c_str(), murl.c_str());

						if (duration > 0)
						{
							mabsLastAudioTimestamp = mabsLastVideoTimestamp;
						}
						else
						{
							mabsLastVideoTimestamp = mabsLastAudioTimestamp;
						}
					}
				}
				jitterTimestamp(maudioJitterCountFrameRate, mvideoJitterCountFrameRate,
					mabsLastVideoTimestamp, absoluteTimestamp, true, mabsLastVideoTimestamp, absoluteTimestamp);
			}
			else
			{
				mabsLastVideoTimestamp = (float)absoluteTimestamp;
				if (misForceJitter)
				{
					int32 duration = int32((int64)mabsLastVideoTimestamp - (int64)mabsLastAudioTimestamp);
					misJitterVideo = true;
					misJitterAudio = true;

					if (duration > 0)
					{
						mabsLastAudioTimestamp = mabsLastVideoTimestamp;
					}
					else {
						mabsLastVideoTimestamp = mabsLastAudioTimestamp;
					}
				}
			}
		}
		else
		{
			if ((int)mabsLastAudioTimestamp > 0)
			{
				//misJitterAudio = true
				jitterTimestamp(maudioJitterCountFrameRate, mvideoJitterCountFrameRate,
					mabsLastAudioTimestamp, absoluteTimestamp, false, mabsLastAudioTimestamp, absoluteTimestamp);
			}
			else
			{
				mabsLastAudioTimestamp = float(absoluteTimestamp);
				if (misForceJitter)
				{
					int32 duration = (int32)((int64)mabsLastVideoTimestamp - (int64)mabsLastAudioTimestamp);
					misJitterVideo = true;
					misJitterAudio = true;

					if (duration > 0)
					{
						mabsLastAudioTimestamp = mabsLastVideoTimestamp;
					}
					else {
						mabsLastVideoTimestamp = mabsLastAudioTimestamp;
					}
				}
			}
		}
	}
	return absoluteTimestamp;
}

bool CJitter::countVideoAudioFrame(bool isVideo, int64 tk, uint32 absoluteTimestamp)
{
	bool is = false;
	if (isVideo)
	{
		if (!misCountVideoFrameRate && tk > 30)
		{
			is = true;
			if (mvideoBeginJitterTimestamp == 0)
			{
				mvideoBeginJitterTimestamp = absoluteTimestamp;
			}
			if (mvideoBeginJitterTimestamp > 0)
			{
				mvideoJitterCountFrameRate += 1.0;
			}
			if (tk > 60)
			{
				mvideoJitterCountFrameRate /= tk;
				logs->debug("%s %s jitter task %s video statistics frame rate %.02f,frame rate %.02f",
					mremoteAddr.c_str(), modeName.c_str(), murl.c_str(), mvideoJitterCountFrameRate, mvideoJitterFrameRate);
				int32 num = (int32)(mvideoJitterCountFrameRate - mvideoJitterFrameRate);
				num = abs(num);
				if (num <= 4)
				{
					misLegallVideoFrameRate = true;
				}
				misCountVideoFrameRate = true;
			}
		}
	}
	else
	{
		if (!misCountAudioFrameRate && tk > 30)
		{
			is = true;
			if (maudioBeginJitterTimestamp == 0)
			{
				maudioBeginJitterTimestamp = absoluteTimestamp;
			}
			if (maudioBeginJitterTimestamp > 0)
			{
				maudioJitterCountFrameRate += 1.0;
			}
			if (tk > 60)
			{
				maudioJitterCountFrameRate /= tk;
				logs->debug("%s %s jitter task %s audio statistics frame rate %.02f,frame rate %.02f",
					mremoteAddr.c_str(), modeName.c_str(), murl.c_str(), maudioJitterCountFrameRate, maudioJitterFrameRate);
				int32 num = (int32)(maudioJitterCountFrameRate - maudioJitterFrameRate);
				num = abs(num);
				if (num <= 4)
				{
					misLegallAudioFrameRate = true;
				}
				misCountAudioFrameRate = true;
			}
		}
	}
	return is;
}

float CJitter::getAudioJitterFrameRate()
{
	return maudioJitterFrameRate;
}

float CJitter::getVideoJitterFrameRate()
{
	return mvideoJitterFrameRate;
}

void CJitter::setAudioJitterFrameRate(float audioJitterFrameRate)
{
	maudioJitterFrameRate = audioJitterFrameRate;
}

void CJitter::setVideoJitterFrameRate(float videoJitterFrameRate)
{
	mvideoJitterFrameRate = videoJitterFrameRate;
}

float CJitter::getAudioJitterCountFrameRate()
{
	return maudioJitterCountFrameRate;
}

void CJitter::setAudioFrameRate(int audioFrameRate)
{
	maudioFrameRate = audioFrameRate;
}

void CJitter::setVideoFrameRate(int videoFrameRate)
{
	mvideoFrameRate = videoFrameRate;
}

void CJitter::setOpenJitter(bool is)
{
	misOpenJitter = is;
}

void CJitter::setOpenForceJitter(bool is)
{
	misForceJitter = is;
}

bool CJitter::isOpenJitter()
{
	return misOpenJitter;
}

bool CJitter::isForceJitter()
{
	return misForceJitter;
}

