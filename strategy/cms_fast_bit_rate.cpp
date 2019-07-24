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
#include <strategy/cms_fast_bit_rate.h>
#include <flvPool/cms_flv_pool.h>
#include <log/cms_log.h>
#include <common/cms_utility.h>

#define DropVideoCacheLen			5000
#define DropVideoStartTime			2000
#define DropVideoCheckInterval		200
#define DropVideoTimeDiffer			100
#define DropVideoClientBufLess		-800
#define DropVideoServerBufLess		2000
#define DropVideoLoseBufTime		5
#define DropVideoLoseBufMaxTime		10

CFastBitRate::CFastBitRate()
{
	misWaterMark = false;
	mwaterMarkOriHashIdx = 0;
	mhashIdx = 0;
	//码率切换
	mautoBitRateMode = AUTO_DROP_CHANGE_BITRATE_CLOSE;
	misChangeVideoBit = false;				//是否切换了码率
	mbeginVideoChangeTime = 0;				//首播丢帧
	mendVideoChangeTime = 0;	            //首播丢帧
	mfirstLowBitRateTimeStamp = 0;			//首播切码率最后发送时间戳，用于切回正常码率时，匹配最佳发送位置
	mhashLowBitRateIdx = 0;                 //当前低码率索引
	mmaxLowBitRateIdx = 0;                  //最大码率索引
	mchangeLowBitRateTimes = 0;             //码率切换次数
	mchangeBitRateTT = 0;
	mcreateBitRateTaskTT = 0;
	mautoBitRateFactor = 3;					//动态变码率系数
	mautoFrameFactor = 20;					//动态丢帧系数
	//码率切换保证时间戳平滑
	mlastVideoChangeBitRateTimestamp = 0;   //切换码率前最后视频时间戳
	mlastVideoChangeBitRateRelativeTimestamp = 0;		//切换码率后，lastVideoChangeBitRateTimestamp 与 切换码率后第一帧视频帧时间戳差异值
	mneedChangeVideoTimestamp = false;                  //切换码率后，需要调整时间戳标志
	mlastAudioChangeBitRateTimestamp = 0;				//切换码率前最后音频时间戳
	mlastAudioChangeBitRateRelativeTimestamp = 0;		//切换码率后，lastVideoChangeBitRateTimestamp 与 切换码率后第一帧音频帧时间戳差异值
	mneedChangeAudioTimestamp = false;					//切换码率后，需要调整时间戳标志
	//丢帧策略
	mno1AudioTimestamp = 0;					//首帧音频时间戳
	mno1VideoTimestamp = 0;					//首帧视频时间戳
	mconnectTimestamp = 0;					//连接时间戳
	mtransCodeNeedDropVideo = false;		//是否需要丢帧
	mtransCodeNoNeedDropVideo = false;		//是否需要丢帧
	mlastDiffer = 0;						//上次时间差异
	mloseBufferTimes = 0;					//预测卡顿次数
	mloseBufferInterval = getTickCount();	//记录统计间隔时间
	mlastVideoTimestamp = 0;
	mbeginDropVideoTimestamp = 0;
	//丢帧码率切换
	mhistoryDropTotal = 0;
	mhistoryDropNum = 0;
	mhistoryDropTime = 0;
	mhistoryDropTT = 0;
	misInit = false;
}

CFastBitRate::~CFastBitRate()
{

}

bool CFastBitRate::isInit()
{
	return misInit;
}

void CFastBitRate::init(std::string remoteAddr, std::string modeName, std::string url, bool isWaterMark,
	uint32 waterMarkOriHashIdx, uint32 hashIdx, HASH &waterMarkOriHash, HASH &hash)
{
	misInit = true;
	murl = url;
	mremoteAddr = remoteAddr;
	modeName = modeName;

	mwaterMarkOriHashIdx = waterMarkOriHashIdx;
	mwaterMarkOriHash = waterMarkOriHash;
	misWaterMark = isWaterMark;

	mhashIdx = hashIdx;
	mhash = hash;
}

void CFastBitRate::setFirstLowBitRateTimeStamp(uint32 timestamp)
{
	mfirstLowBitRateTimeStamp = timestamp;
}

void CFastBitRate::changeBitRateLastTimestamp(int32 dateType, uint32 timestamp)
{
	if (dateType == DATA_TYPE_VIDEO)
	{
		if (mbeginVideoChangeTime == 0)
		{
			mbeginVideoChangeTime = timestamp;
		}
		mendVideoChangeTime = timestamp;
		mlastVideoChangeBitRateTimestamp = timestamp;
		if (mlastAudioChangeBitRateTimestamp > mlastVideoChangeBitRateTimestamp)
		{
			mlastVideoChangeBitRateTimestamp = mlastAudioChangeBitRateTimestamp;
		}
	}
	else if (dateType == DATA_TYPE_AUDIO)
	{
		mlastAudioChangeBitRateTimestamp = timestamp;
		if (mlastVideoChangeBitRateTimestamp > mlastAudioChangeBitRateTimestamp)
		{
			mlastAudioChangeBitRateTimestamp = mlastVideoChangeBitRateTimestamp;
		}
	}
}

uint32 CFastBitRate::changeBitRateSetTimestamp(int32 dateType, uint32 sendTimestamp)
{
	if (dateType == DATA_TYPE_VIDEO && mneedChangeVideoTimestamp)
	{
		mneedChangeVideoTimestamp = false;
		mlastVideoChangeBitRateRelativeTimestamp = int32(mlastVideoChangeBitRateTimestamp) - int32(sendTimestamp);
	}
	else if (dateType == DATA_TYPE_AUDIO && mneedChangeAudioTimestamp)
	{
		mneedChangeAudioTimestamp = false;
		mlastAudioChangeBitRateRelativeTimestamp = int32(mlastAudioChangeBitRateTimestamp) - int32(sendTimestamp);
	}
	if (dateType == DATA_TYPE_VIDEO)
	{
		if (mlastVideoChangeBitRateRelativeTimestamp > 0)
		{
			sendTimestamp = sendTimestamp + uint32(mlastVideoChangeBitRateRelativeTimestamp) + 50;
		}
		else if (mlastVideoChangeBitRateRelativeTimestamp < 0)
		{
			sendTimestamp = sendTimestamp - uint32(mlastVideoChangeBitRateRelativeTimestamp*-1) + 50;
		}
		mlastVideoChangeBitRateTimestamp = sendTimestamp;
		if (mlastAudioChangeBitRateTimestamp > mlastVideoChangeBitRateTimestamp)
		{
			mlastVideoChangeBitRateTimestamp = mlastAudioChangeBitRateTimestamp;
		}
		mlastAudioChangeBitRateTimestamp = mlastVideoChangeBitRateTimestamp;
	}
	else if (dateType == DATA_TYPE_AUDIO)
	{
		if (mlastAudioChangeBitRateRelativeTimestamp > 0)
		{
			sendTimestamp = sendTimestamp + uint32(mlastAudioChangeBitRateRelativeTimestamp) + 50;
		}
		else if (mlastAudioChangeBitRateRelativeTimestamp < 0)
		{
			sendTimestamp = sendTimestamp - uint32(mlastAudioChangeBitRateRelativeTimestamp*-1) + 50;
		}
		mlastAudioChangeBitRateTimestamp = sendTimestamp;
		if (mlastVideoChangeBitRateTimestamp > mlastAudioChangeBitRateTimestamp)
		{
			mlastAudioChangeBitRateTimestamp = mlastVideoChangeBitRateTimestamp;
		}
		mlastVideoChangeBitRateTimestamp = mlastAudioChangeBitRateTimestamp;
	}
	return sendTimestamp;
}

void CFastBitRate::setChangeBitRate()
{
	std::string suffixName = "";
	std::string transCodeSuffix = "";
	if (!misWaterMark)
	{
		mautoBitRateMode = CFlvPool::instance()->readBitRateMode(mhashIdx, mhash);
		suffixName = CFlvPool::instance()->readChangeBitRateSuffix(mhashIdx, mhash);
		transCodeSuffix = CFlvPool::instance()->readCodeSuffix(mhashIdx, mhash);
	}
	else
	{
		mautoBitRateMode = CFlvPool::instance()->readBitRateMode(mwaterMarkOriHashIdx, mwaterMarkOriHash);
		suffixName = CFlvPool::instance()->readChangeBitRateSuffix(mwaterMarkOriHashIdx, mwaterMarkOriHash);
		transCodeSuffix = CFlvPool::instance()->readCodeSuffix(mwaterMarkOriHashIdx, mwaterMarkOriHash);
	}

	logs->info("%s fast bit rate %s mautoBitRateMode=%d,suffixName=%s,transCodeSuffix=%s.",
		mremoteAddr.c_str(),
		murl.c_str(),
		mautoBitRateMode,
		suffixName.c_str(),
		transCodeSuffix.c_str());

	if ((mautoBitRateMode == AUTO_CHANGE_BITRATE_OPEN ||
		mautoBitRateMode == AUTO_DROP_CHANGE_BITRATE_OPEN) &&
		!suffixName.empty() && transCodeSuffix != suffixName)
	{
		mhashLowBitRate.clear();
		mhashIdxLowBitRate.clear();

		std::string oriUrl = murl;
		std::size_t i = oriUrl.find("?");
		if (i != std::string::npos)
		{
			oriUrl = oriUrl.substr(0, i);
		}
		i = oriUrl.find(".flv");
		if (i != std::string::npos)
		{
			oriUrl = oriUrl.substr(0, i);
			if (!transCodeSuffix.empty())
			{
				i = oriUrl.rfind(transCodeSuffix);
				if (i != std::string::npos)
				{
					if (i == oriUrl.length() - transCodeSuffix.length())
					{
						//需要校验，必须最后字符串完全匹配才可以
						oriUrl = oriUrl.substr(0, i);
					}
				}
			}
			oriUrl += suffixName;
			oriUrl += ".flv";
		}
		else
		{
			if (!transCodeSuffix.empty())
			{
				i = oriUrl.rfind(transCodeSuffix);
				if (i != std::string::npos)
				{
					if (i == oriUrl.length() - transCodeSuffix.length())
					{
						//需要校验，必须最后字符串完全匹配才可以
						oriUrl = oriUrl.substr(0, i);
					}
				}
			}
		}
		mhashLowBitRate.push_back(mhash);
		mhashIdxLowBitRate.push_back(mhashIdx);

		oriUrl += suffixName;
		mlowBitRateUrl = oriUrl;
		std::string hashUrlLow = readHashUrl(oriUrl);
		logs->info("%s fast bit rate %s low bit rate url %s,low bit rate hash url %s #####",
			mremoteAddr.c_str(),
			murl.c_str(),
			mlowBitRateUrl.c_str(),
			hashUrlLow.c_str());

		HASH hashLow = makeHash(hashUrlLow.c_str(), hashUrlLow.length());
		mhashLowBitRate.push_back(hashLow);
		uint32 hashIdx = CFlvPool::instance()->hashIdx(hashLow);
		mhashIdxLowBitRate.push_back(hashIdx);
		mlowBitRateHash = hashLow;

		mmaxLowBitRateIdx = 1;
		mhashLowBitRateIdx = 0;
	}
	else
	{
		if (mautoBitRateMode == AUTO_DROP_CHANGE_BITRATE_OPEN)
		{
			mautoBitRateMode = AUTO_DROP_BITRATE_OPEN;
		}
		else if (mautoBitRateMode == AUTO_CHANGE_BITRATE_OPEN)
		{
			mautoBitRateMode = AUTO_DROP_CHANGE_BITRATE_CLOSE;
		}
		else if (mautoBitRateMode == AUTO_DROP_BITRATE_OPEN)
		{
			//mautoBitRateMode = AUTO_DROP_BITRATE_OPEN;
		}
		else
		{
			mautoBitRateMode = AUTO_DROP_CHANGE_BITRATE_CLOSE;
		}
	}
}

void CFastBitRate::resetDropFlag()
{
	while (!mhistoryDropList.empty())
	{
		mhistoryDropList.pop();
	}
	mhistoryDropTotal = 0;
	mhistoryDropNum = 0;
}

int CFastBitRate::dropFramePer(int64 te, int32 sliceFrameRate)
{
	//检测丢帧率
	int dropPer = 0;
	if (te - mhistoryDropTT > 3000)
	{
		mhistoryDropTT = te;
		mhistoryDropTotal += mhistoryDropNum;
		mhistoryDropList.push(mhistoryDropNum);
		if (mhistoryDropList.size() > 40)
		{
			int num = mhistoryDropList.front();
			mhistoryDropTotal -= num;
			mhistoryDropList.pop();
		}
		mhistoryDropNum = 0;
		if (sliceFrameRate > 0)
		{
			dropPer = mhistoryDropTotal * 100 / int(sliceFrameRate * 120);
			if (dropPer >= 10)
			{
				logs->info("%s fast bit rate %s need change to lowbit #####", mremoteAddr.c_str(), murl.c_str());
			}
		}
	}
	//检测丢帧率结束
	return dropPer;
}

void CFastBitRate::dropOneFrame()
{
	mhistoryDropNum++;
}

bool CFastBitRate::changeRateBit(uint32 hashIdxOld, HASH &hashOld, bool isReset,
	uint32 timestamp, std::string referUrl, uint32 &hashIdxRead, HASH &hashRead, int64 &transIdx, int32 &mode)
{
	bool isSucc = false;
	bool isRaise = false;
	hashRead = hashOld;
	hashIdxRead = hashIdxOld;
	if (!isReset)
	{
		if (mhashLowBitRateIdx >= mmaxLowBitRateIdx)
		{
			return false;
		}
		mhashLowBitRateIdx++;
		isRaise = true;
		mode = 0;
	}
	else
	{
		if (mhashLowBitRateIdx <= 0)
		{
			return false;
		}
		mhashLowBitRateIdx--;
		mode = 1;
	}
	do
	{
		misChangeVideoBit = true;
		int64 tt = getTimeUnix();
		hashRead = mhashLowBitRate[mhashLowBitRateIdx];
		hashIdxRead = mhashIdxLowBitRate[mhashLowBitRateIdx];
		if (hashRead == mcreateBitRateHash && tt - mcreateBitRateTaskTT < 3)
		{
			//防止一直创建任务
			break;
		}
		if (hashRead == mlowBitRateHash && !CFlvPool::instance()->isExist(hashIdxRead, hashRead))
		{
			/*createTask(mlowBitRateUrl, hashRead, referUrl, CREATE_TASK_ACT_NORMAL)
			mcreateBitRateTaskTT = tt
			mcreateBitRateHash = hashRead*/
		}
		bool  res;
		int64 idx;
		res = CFlvPool::instance()->seekKeyFrame(hashIdxRead, hashRead, timestamp, idx); //flv.SeekSlice(r.hashIdxRead, r.hashRead, timestamp)
		if (!res)
		{
			logs->error("*** %s fast bit rate %s changeRateBit not find SeekKeyFrame ***", mremoteAddr.c_str(), murl.c_str());
			break;
		}

		//统计时间戳置位
		mneedChangeAudioTimestamp = true;
		mneedChangeVideoTimestamp = true;
		transIdx = idx;
		isSucc = true;
	} while (0);
	if (!isSucc)
	{
		if (isRaise)
		{
			mhashLowBitRateIdx--;
		}
		else
		{
			mhashLowBitRateIdx++;
		}
		hashRead = hashOld;
		hashIdxRead = hashIdxOld;
		misChangeVideoBit = false;
	}
	return isSucc;
}

bool CFastBitRate::dropVideoFrame(int32 edgeCacheTT, int32 dataType, int32 sliceFrameRate,
	int64 te, uint32 sendTimestamp, int sliceNum)
{
	bool isCaton = false;
	if (edgeCacheTT >= DropVideoCacheLen)
	{
		//针对缓冲时间设计超过10秒的有效
		if ((te - mconnectTimestamp) > DropVideoStartTime)
		{
			if (dataType == DATA_TYPE_AUDIO)
			{
				mlastVideoTimestamp = sendTimestamp;
				int64 differ = (te - mconnectTimestamp) - int64(sendTimestamp - mno1AudioTimestamp);
				int32 guestBufLen = 0;
				if (sliceFrameRate > 0)
				{
					guestBufLen = int32(sliceNum * 1000) / sliceFrameRate; //转换成毫秒
				}
				int64 compareDiffer = mlastDiffer - differ;
				if (te - mloseBufferInterval > DropVideoCheckInterval)
				{
					//200ms判断一次
					if (MathAbs(compareDiffer) > DropVideoTimeDiffer)
					{
						if (compareDiffer < 0 && mloseBufferTimes < DropVideoLoseBufMaxTime)
						{
							mloseBufferTimes++;
						}
						if (compareDiffer > 0 && mloseBufferTimes > 0)
						{
							mloseBufferTimes--;
						}
					}
					//判断用户剩余缓冲数据小于800ms
					if (differ >= DropVideoClientBufLess)
					{
						mloseBufferTimes = DropVideoLoseBufMaxTime;
					}
					//判断服务器缓冲区数据小于2000ms
					if (guestBufLen < DropVideoServerBufLess)
					{
						mloseBufferTimes = 0;
					}
					if (mtransCodeNeedDropVideo || mtransCodeNoNeedDropVideo)
					{
						logs->info("%s %s %s AutoBitRateMode=%d,"\
							" $$$rtmp drop video frame differ=%lld,lastdiffer=%lld,guestBufLen=%d,"\
							"left slice=%d, sliceFrameRate=%d"\
							"mloseBufferTimes=%d,int32(edgeCacheTT/2)=%d,te=%lld,r.connectTimestamp=%lld,sendTimestamp=%u,"\
							"mno1AudioTimestamp=%u,(te - r.connectTimestamp)=%lld,int64(sendTimestamp-mno1AudioTimestamp)=%lld",
							mremoteAddr.c_str(),
							modeName.c_str(),
							murl.c_str(),
							mautoBitRateMode,
							differ,
							mlastDiffer,
							guestBufLen,
							sliceNum,
							sliceFrameRate,
							mloseBufferTimes,
							int32(edgeCacheTT / 2),
							te,
							mconnectTimestamp,
							sendTimestamp,
							mno1AudioTimestamp,
							(te - mconnectTimestamp),
							int64(sendTimestamp - mno1AudioTimestamp));
					}
					mloseBufferInterval = te;
					mlastDiffer = differ;
					if (guestBufLen >= int32(edgeCacheTT / 2))
					{
						if (mloseBufferTimes >= int32(mautoFrameFactor))
						{
							if (mautoBitRateMode == AUTO_CHANGE_BITRATE_OPEN)
							{
								if (!mtransCodeNoNeedDropVideo)
								{
									isCaton = true;
								}
								mtransCodeNoNeedDropVideo = true;
							}
							else
							{
								if (!mtransCodeNeedDropVideo)
								{
									isCaton = true;
								}
								mbeginDropVideoTimestamp = sendTimestamp;
								mtransCodeNeedDropVideo = true;
							}
						}
					}
				}
			}
			if (dataType == DATA_TYPE_VIDEO)
			{
				int64 differ = (te - mconnectTimestamp) - int64(sendTimestamp - mno1VideoTimestamp);
				int32 guestBufLen = 0;
				if (sliceFrameRate > 0)
				{
					guestBufLen = int32(sliceNum * 1000) / sliceFrameRate; //转换成毫秒
				}
				int64 compareDiffer = mlastDiffer - differ;
				if (te - mloseBufferInterval > DropVideoCheckInterval)
				{
					if (MathAbs(compareDiffer) > DropVideoTimeDiffer)
					{
						if (compareDiffer < 0 && mloseBufferTimes < DropVideoLoseBufMaxTime)
						{
							mloseBufferTimes++;
						}
						if (compareDiffer > 0 && mloseBufferTimes > 0)
						{
							mloseBufferTimes--;
						}
					}
					//判断用户剩余缓冲数据小于800ms
					if (differ >= DropVideoClientBufLess)
					{
						mloseBufferTimes = DropVideoLoseBufMaxTime;
					}
					//判断服务器缓冲区数据小于2000ms
					if (guestBufLen < DropVideoServerBufLess)
					{
						mloseBufferTimes = 0;
					}

					if (mtransCodeNeedDropVideo || mtransCodeNoNeedDropVideo)
					{
						logs->info("%s %s %s AutoBitRateMode=%d,"\
							" $$$rtmp drop video frame differ=%lld,lastdiffer=%lld,guestBufLen=%d,"\
							"left slice=%d, sliceFrameRate=%d"\
							"mloseBufferTimes=%d,int32(edgeCacheTT/2)=%d,te=%lld,r.connectTimestamp=%lld,sendTimestamp=%u,"\
							"mno1VideoTimestamp=%u,(te - r.connectTimestamp)=%lld,int64(sendTimestamp-mno1VideoTimestamp)=%lld",
							mremoteAddr.c_str(),
							modeName.c_str(),
							murl.c_str(),
							mautoBitRateMode,
							differ,
							mlastDiffer,
							guestBufLen,
							sliceNum,
							sliceFrameRate,
							mloseBufferTimes,
							int32(edgeCacheTT / 2),
							te,
							mconnectTimestamp,
							sendTimestamp,
							mno1VideoTimestamp,
							(te - mconnectTimestamp),
							int64(sendTimestamp - mno1VideoTimestamp));
					}
					mlastDiffer = differ;
					mloseBufferInterval = te;

					if (guestBufLen >= int32(edgeCacheTT / 2))
					{
						if (mloseBufferTimes >= int32(mautoFrameFactor))
						{
							if (mautoBitRateMode == AUTO_CHANGE_BITRATE_OPEN)
							{
								if (!mtransCodeNoNeedDropVideo)
								{
									isCaton = true;
								}
								mtransCodeNoNeedDropVideo = true;
							}
							else
							{
								if (!mtransCodeNeedDropVideo)
								{
									isCaton = true;
								}
								mbeginDropVideoTimestamp = mlastVideoTimestamp;
								mtransCodeNeedDropVideo = true;
							}
						}
					}
				}
			}
		}
		else
		{
			if (dataType == DATA_TYPE_AUDIO)
			{
				mlastDiffer = (te - mconnectTimestamp) - int64(sendTimestamp - mno1AudioTimestamp);
			}
			if (dataType == DATA_TYPE_VIDEO)
			{
				mlastDiffer = (te - mconnectTimestamp) - int64(sendTimestamp - mno1VideoTimestamp);
			}
		}
	}
	return isCaton;
}

bool CFastBitRate::isDropEnoughTime(uint32 sendTimestamp)
{
	return sendTimestamp - mbeginDropVideoTimestamp > 2000;
}

void CFastBitRate::resetDropFrameFlags()
{
	mno1AudioTimestamp = 0;
	mno1VideoTimestamp = 0;
	mconnectTimestamp = getTickCount();
	mtransCodeNeedDropVideo = false;
}

bool CFastBitRate::needResetFlags(int32 dateType, uint32 sendTimestamp)
{
	if ((dateType == DATA_TYPE_AUDIO && sendTimestamp < mno1AudioTimestamp) ||
		(dateType == DATA_TYPE_VIDEO && sendTimestamp < mno1VideoTimestamp))
	{
		return true;
	}
	return false;
}

void CFastBitRate::setNo1VideoAudioTimestamp(bool isVideo, uint32 sendTimestamp)
{
	if (isVideo)
	{
		if (mno1VideoTimestamp == 0)
		{
			mno1VideoTimestamp = sendTimestamp;
			if (mconnectTimestamp == 0)
			{
				mconnectTimestamp = getTickCount();
			}
		}
	}
	else
	{
		if (mno1AudioTimestamp == 0)
		{
			mno1AudioTimestamp = sendTimestamp;
			if (mconnectTimestamp == 0)
			{
				mconnectTimestamp = getTickCount();
			}
		}
	}
}

bool CFastBitRate::isChangeBitRate()
{
	return misChangeVideoBit;
}

int	CFastBitRate::getAutoBitRateMode()
{
	return mautoBitRateMode;
}

int	CFastBitRate::getAutoBitRateFactor()
{
	return mautoBitRateFactor;
}

void CFastBitRate::setAutoBitRateFactor(int autoBitRateFactor)
{
	mautoBitRateFactor = autoBitRateFactor;
}

int	CFastBitRate::getAutoFrameFactor()
{
	return mautoFrameFactor;
}

void CFastBitRate::setAutoFrameFactor(int autoFrameFactor)
{
	mautoFrameFactor = autoFrameFactor;
}

bool CFastBitRate::getTransCodeNeedDropVideo()
{
	return mtransCodeNeedDropVideo;
}

void CFastBitRate::setTransCodeNeedDropVideo(bool is)
{
	mtransCodeNeedDropVideo = is;
}

bool CFastBitRate::getTransCodeNoNeedDropVideo()
{
	return mtransCodeNoNeedDropVideo;
}

void CFastBitRate::setTransCodeNoNeedDropVideo(bool is)
{
	mtransCodeNoNeedDropVideo = is;
}

int32 CFastBitRate::getLoseBufferTimes()
{
	return mloseBufferTimes;
}

bool CFastBitRate::isChangeVideoBit()
{
	return misChangeVideoBit;
}
