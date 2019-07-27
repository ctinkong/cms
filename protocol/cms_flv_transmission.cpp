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
#include <protocol/cms_flv_transmission.h>
#include <flvPool/cms_flv_pool.h>
#include <log/cms_log.h>
#include <assert.h>


CFlvTransmission::CFlvTransmission(CProtocol *protocol, bool isPushTask/* = false*/)
{
	mllMetaDataIdx = -1;
	mllFirstVideoIdx = -1;
	mllFirstAudioIdx = -1;
	mllTransIdx = -1;
	misChangeFirstVideo = false;
	misTaskRestart = false;
	mchangeFristVideoTimes = 0;
	mprotocol = protocol;
	misWaterMark = false;
	mwaterMarkOriHashIdx = 0;
	mcacheTT = 0;
	msliceFrameRate = 0;
	misPushTask = isPushTask;
	mfastBitRate = new CFastBitRate;
	mdurationtt = new CDurationTimestamp();
	mfirstPlay = new CFirstPlay();
	mjumpLastXSeconds = new CJumpLastXSeconds();
	mullTransUid = 0;
	misRestart8AdjustTimestamp = false;
}

CFlvTransmission::~CFlvTransmission()
{
	if (mfastBitRate)
	{
		delete mfastBitRate;
	}
	if (mdurationtt)
	{
		delete mdurationtt;
	}
	if (mfirstPlay)
	{
		delete mfirstPlay;
	}
	if (mjumpLastXSeconds)
	{
		delete mjumpLastXSeconds;
	}
}

void CFlvTransmission::setHash(uint32 hashIdx, HASH &hash)
{
	mreadHashIdx = hashIdx;
	mreadHash = hash;
}

void CFlvTransmission::setWaterMarkHash(uint32 hashIdx, HASH &hash)
{
	misWaterMark = true;
	mwaterMarkOriHashIdx = hashIdx;
	mwaterMarkOriHash = hash;
}

int CFlvTransmission::doMetaData()
{
	int ret = CMS_ERROR;
	Slice *s = NULL;
	if (CFlvPool::instance()->readMetaData(mreadHashIdx, mreadHash, &s) == FlvPoolCodeError)
	{
		return ret;
	}
	ret = CMS_OK;
	if (s != NULL)
	{
		mllMetaDataIdx = s->mllIndex;
		ret = mprotocol->sendMetaData(s);
		atomicDec(s);
		logs->info("%s [CFlvTransmission::doMetaData] %s doMetaData send metaData",
			mprotocol->remoteAddr().c_str(), mprotocol->getUrl().c_str());
	}
	return ret;
}

int CFlvTransmission::doFirstVideoAudio(bool isVideo)
{
	int ret = CMS_ERROR;
	Slice *s = NULL;
	if (CFlvPool::instance()->readFirstVideoAudioSlice(mreadHashIdx, mreadHash, &s, isVideo) == FlvPoolCodeError)
	{
		return ret;
	}
	ret = CMS_OK;
	if (s != NULL)
	{
		if (isVideo)
		{
			misChangeFirstVideo = true;
			mchangeFristVideoTimes++;

			mllFirstVideoIdx = s->mllIndex;
			logs->info("%s [CFlvTransmission::doFirstVideoAudio] %s doFirstVideoAudio send first video",
				mprotocol->remoteAddr().c_str(), mprotocol->getUrl().c_str());
		}
		else
		{
			mllFirstAudioIdx = s->mllIndex;
			logs->info("%s [CFlvTransmission::doFirstVideoAudio] %s doFirstVideoAudio send first audio",
				mprotocol->remoteAddr().c_str(), mprotocol->getUrl().c_str());
		}
		ret = mprotocol->sendVideoOrAudio(s, 0);
		atomicDec(s);
	}
	return ret;
}

void CFlvTransmission::getSliceFrameRate()
{
	if (msliceFrameRate == 0)
	{
		int videoFrameRate = CFlvPool::instance()->getVideoFrameRate(mreadHashIdx, mreadHash);
		int audioFrameRate = CFlvPool::instance()->getAudioFrameRate(mreadHashIdx, mreadHash);
		if (videoFrameRate > 0 && audioFrameRate > 0)
		{
			msliceFrameRate = videoFrameRate = audioFrameRate;
		}
	}
}

int CFlvTransmission::doTransmission(bool &isSendData)
{
	isSendData = false;
	int ret = 0;
	Slice *s = NULL;
	int flvPoolCode;
	uint32 uiTimestamp = 0;
	int	sliceNum = 0;
	bool needSend = false;
	bool isVideo = false;
	bool isAudio = false;
	bool isTransPlay = false;
	bool isExist = false;
	bool isPublishTask = false;
	bool isTaskRestart = false;
	bool isMetaDataChanged = false;
	bool isFirstVideoAudioChanged = false;
	bool isError = false;
	getSliceFrameRate();
	uint32 tt = getTickCount();
	if (!mjumpLastXSeconds->isInit())
	{
		mjumpLastXSeconds->init(mprotocol->remoteAddr(),
			mprotocol->protocol(),
			mprotocol->getUrl(),
			mreadHashIdx,
			mreadHash);
	}
	do
	{
		isTransPlay = false;
		isExist = false;
		isPublishTask = false;
		isTaskRestart = false;
		isMetaDataChanged = false;
		isFirstVideoAudioChanged = false;
		isError = false;

		//首播丢帧初始化
		if (!checkFirstPlay())
		{
			ret = 0;
			break;
		}
		//首播丢帧初始化 结束
		sliceNum = 0;
		flvPoolCode = CFlvPool::instance()->readSlice(mreadHashIdx,
			mreadHash,
			mllTransIdx,
			&s,
			sliceNum,
			isTransPlay,
			mllMetaDataIdx,
			mllFirstVideoIdx,
			mllFirstAudioIdx,
			isExist,
			isTaskRestart,
			isPublishTask,
			isMetaDataChanged,
			isFirstVideoAudioChanged,
			mullTransUid);

		CAutoSlice autoSlice(s);
		if (!isExist)
		{
			ret = 2;
			if (checkPush8Missing())
			{
				ret = -1;//推流失败
			}
			break;
		}

		if (misPushTask && !isPublishTask)
		{
			//只有推流任务才能转推
			ret = -1;
			logs->error("*** %s [CFlvTransmission::doTransmission] %s doTransmission is pushing task but read task is not publish task ***",
				mprotocol->remoteAddr().c_str(), mprotocol->getUrl().c_str());
			break;
		}

		if (flvPoolCode == FlvPoolCodeError)
		{
			if (checkPush8Missing())
			{
				ret = -1;//推流失败
			}
			break;
		}
		else if (flvPoolCode == FlvPoolCodeOK)
		{
			//metaData 改变
			if (isMetaDataChanged)
			{
				if (doMetaData() == CMS_ERROR)
				{
					ret = -1;
					break;
				}
			}
			//首帧改变
			if (isFirstVideoAudioChanged)
			{
				if (CFlvPool::instance()->isFirstVideoChange(mreadHashIdx, mreadHash, mllFirstVideoIdx))
				{
					if (doFirstVideoAudio(true) == CMS_ERROR)
					{
						ret = -1;
						break;
					}
				}
				if (CFlvPool::instance()->isFirstAudioChange(mreadHashIdx, mreadHash, mllFirstAudioIdx))
				{
					if (doFirstVideoAudio(false) == CMS_ERROR)
					{
						ret = -1;
						break;
					}
				}
			}
			//如果首帧改变 刚连上来的用户有可能发送的是旧的数据帧 花屏
			if (isFirstPlay8StreamChange(s))
			{
				continue;
			}
			//如果首帧改变 刚连上来的用户有可能发送的是旧的数据帧 花屏 结束
			playMagicInit();
			if (s)
			{
				needSend = true;
				//首播丢帧
				if (!misPushTask && !mfirstPlay->checkShouldDropFrameCount(mllTransIdx, s))
				{
					ret = 0;
					continue;
				}
				needSend = !mfirstPlay->needDropFrame(s);
				if (misPushTask)
				{
					//推流没有首播丢帧
					needSend = true;
				}
				//首播丢帧 结束
				isVideo = s->miDataType == DATA_TYPE_VIDEO;
				isAudio = s->miDataType == DATA_TYPE_AUDIO;

				uiTimestamp = s->muiTimestamp;
				if (misRestart8AdjustTimestamp) //p2p 不能修改时间戳 目前没有p2p逻辑
				{
					mdurationtt->resetDeltaTimestamp(uiTimestamp);
					misRestart8AdjustTimestamp = false;
				}
				//判断是否合并帧
				bool isMergerFrame = isShouldMergerFrame(s, isVideo, isError);
				if (isError)
				{
					ret = -1;
					break;
				}
				//如果切换码率了,需要修改时间戳
				uiTimestamp = mfastBitRate->changeBitRateSetTimestamp(s->miDataType, uiTimestamp);
				//预防时间戳变小的情况
				uiTimestamp = mdurationtt->keepTimestampIncrease(isVideo, uiTimestamp);
				//预防时间戳变小的情况 结束
				//重设时间戳
				uiTimestamp = mdurationtt->resetTimestamp(uiTimestamp, isVideo);
				//重设时间戳 结束				
				//如果切换码率了,需要修改时间戳 结束

				//动态丢帧
				checkFastBitRate(s, uiTimestamp, tt, needSend, isVideo, isAudio, sliceNum);
				//动态丢帧 结束
				if (needSend)
				{
					isSendData = true;
					ret = mprotocol->sendVideoOrAudio(s, uiTimestamp);
				}
				if (mllTransIdx != -1 &&
					mllTransIdx + 1 != s->mllIndex)
				{
					logs->info("*** %s [CFlvTransmission::doTransmission] %s doTransmission drop slice ***",
						mprotocol->remoteAddr().c_str(), mprotocol->getUrl().c_str());
				}

				if (!mfastBitRate->isChangeVideoBit())
				{
					mllTransIdx = s->mllIndex;
				}
				if (mjumpLastXSeconds->judge(s->miDataType, mllTransIdx, tt, sliceNum, msliceFrameRate, uiTimestamp))
				{
					misRestart8AdjustTimestamp = true;
					logs->debug("%s [CFlvTransmission::doTransmission] %s doTransmission task is been jump last x seconds,need adjust timestamp.",
						mprotocol->remoteAddr().c_str(), mprotocol->getUrl().c_str());
				}
				if (isMergerFrame)
				{
					//因为是在该函数申请的内存，释放
					atomicDec(s);
				}
				if (ret == CMS_ERROR)
				{
					ret = -1;
					break;
				}
				mjumpLastXSeconds->record();
				ret = 1;
				if (mprotocol->writeBuffSize() > 0)
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
		else if (flvPoolCode == FlvPoolCodeNoData)
		{
			ret = 0;
			break;
		}
		else if (flvPoolCode == FlvPoolCodeRestart)
		{
			setRestart();
			ret = 0;
			break;
		}
		else
		{
			logs->error("*** %s [CFlvTransmission::doTransmission] %s doTransmission unknow FlvPoolCode=%d ***",
				mprotocol->remoteAddr().c_str(), mprotocol->getUrl().c_str(), flvPoolCode);
			ret = -1;
			break;
		}
	} while (true);
	return ret;
}

void CFlvTransmission::setWriteBuffer()
{
	int medieRate = CFlvPool::instance()->getMediaRate(mreadHashIdx, mreadHash);
	int sendBufSize = medieRate / 8 * 1024;
	if (sendBufSize < 32 * 1024)
	{
		sendBufSize = 32 * 1024;
	}
	if (sendBufSize > 512 * 1024)
	{
		sendBufSize = 512 * 1024;
	}
	mprotocol->setWriteBuffer(sendBufSize);
}

bool CFlvTransmission::checkFirstPlay()
{
	if (!mfirstPlay->isInit())
	{
		//设置网络层发送缓冲
		setWriteBuffer();
		//设置网络层发送缓冲 结束
		mfirstPlay->init(mreadHash,
			mreadHashIdx,
			mprotocol->remoteAddr(),
			mprotocol->protocol(),
			mprotocol->getUrl());
	}
	if (!misPushTask && !mfirstPlay->checkfirstPlay())
	{
		return false;
	}
	return true;
}

bool CFlvTransmission::isFirstPlay8StreamChange(Slice *s)
{
	if (mllTransIdx == -1 && mllFirstVideoIdx != -1 && s->mllIndex < mllFirstVideoIdx)
	{
		mllTransIdx = mllFirstVideoIdx < mllFirstAudioIdx ? mllFirstVideoIdx - 1 : mllFirstAudioIdx - 1;
		return true;
	}
	if (mllTransIdx == -1 && mllFirstAudioIdx != -1 && s->mllIndex < mllFirstAudioIdx)
	{
		mllTransIdx = mllFirstVideoIdx < mllFirstAudioIdx ? mllFirstVideoIdx - 1 : mllFirstAudioIdx - 1;
		return true;
	}
	return false;
}

void CFlvTransmission::setRestart()
{
	mllTransIdx = -1;
	mllMetaDataIdx = -1;
	mllFirstVideoIdx = -1;
	mllFirstAudioIdx = -1;

	mullTransUid = 0;
	misTaskRestart = true; //很有可能码率切换了

	misRestart8AdjustTimestamp = true;
	//还原丢帧转码状态
	mfastBitRate->resetDropFrameFlags();
	mjumpLastXSeconds->reset();
	logs->info("%s [CFlvTransmission::doTransmission] %s doTransmission task is been restart",
		mprotocol->remoteAddr().c_str(), mprotocol->getUrl().c_str());
}

bool CFlvTransmission::checkPush8Missing()
{
	if ((misPushTask || mprotocol->isCmsConnection()) && mllTransIdx != -1)
	{
		logs->error("*** %s [CFlvTransmission::doTransmission] %s doTransmission task is missing ***",
			mprotocol->remoteAddr().c_str(), mprotocol->getUrl().c_str());
		return true;
	}
	return false;
}

void CFlvTransmission::playMagicInit()
{
	if (!mfastBitRate->isInit())
	{
		mfastBitRate->init(mprotocol->remoteAddr(),
			mprotocol->protocol(),
			mprotocol->getUrl(),
			misWaterMark,
			mwaterMarkOriHashIdx,
			mreadHashIdx,
			mwaterMarkOriHash,
			mreadHash);

		mfastBitRate->setChangeBitRate();
	}
	if (!mdurationtt->isInit())
	{
		mdurationtt->init(mprotocol->remoteAddr(),
			mprotocol->protocol(),
			mprotocol->getUrl());

		mdurationtt->setResetTimestamp(true);
	}
}

bool CFlvTransmission::isShouldMergerFrame(Slice *&s, bool &isVideo, bool &isError)
{
	bool isMergerFrame = false;
	isError = false;
	if (isVideo && (mfastBitRate->isChangeBitRate() ||
		misTaskRestart ||
		(misChangeFirstVideo && mchangeFristVideoTimes > 1)))
	{
		misTaskRestart = false;
		misChangeFirstVideo = false;
		Slice *fs = NULL;
		if (CFlvPool::instance()->readFirstVideoAudioSlice(mreadHashIdx, mreadHash, &fs, true) == FlvPoolCodeError)
		{
			logs->info("*** %s [CFlvTransmission::doTransmission] %s merger key frame but not found first video ***",
				mprotocol->remoteAddr().c_str(), mprotocol->getUrl().c_str());
			isError = true;
		}
		char *d = NULL;
		int32 dLen = 0;
		if (CFlvPool::instance()->mergeKeyFrame(fs->mData, fs->miDataLen, s->mData, s->miDataLen, &d, dLen, mprotocol->getUrl()))
		{
			Slice *ss = newSlice();
			ss->mData = d;
			ss->miDataLen = dLen;
			ss->miDataType = s->miDataType;
			ss->mllIndex = s->mllIndex;
			ss->muiTimestamp = s->muiTimestamp;
			ss->misKeyFrame = s->misKeyFrame;
			isMergerFrame = true;
			//atomicDec(s); //CAutoSlice autoSlice(s) 会自动释放 这里虽然改变但不需要再次释放
			s = ss;
		}
		atomicDec(fs);
	}
	return isMergerFrame;
}

void CFlvTransmission::checkFastBitRate(Slice *s, uint32 &uiTimestamp, uint32 &tt, bool &needSend, bool &isVideo, bool &isAudio, int &sliceNum)
{
	if (mfastBitRate->needResetFlags(s->miDataType, uiTimestamp))
	{
		//时间戳变小了重设标志
		mfastBitRate->resetDropFrameFlags();
	}
	if (isAudio)
	{
		mfastBitRate->setNo1VideoAudioTimestamp(false, uiTimestamp);
	}
	else if (isVideo)
	{
		mfastBitRate->setNo1VideoAudioTimestamp(true, uiTimestamp);
	}
	if (((mfastBitRate->getAutoBitRateMode() == AUTO_DROP_CHANGE_BITRATE_OPEN ||
		mfastBitRate->getAutoBitRateMode() == AUTO_DROP_BITRATE_OPEN) &&
		muiKeyFrameDistance < DropVideoKeyFrameLen) ||
		mfastBitRate->getAutoBitRateMode() == AUTO_CHANGE_BITRATE_OPEN)
	{
		if (mcacheTT == 0)
		{
			mcacheTT = CFlvPool::instance()->getCacheTT(mreadHashIdx, mreadHash);
			muiKeyFrameDistance = CFlvPool::instance()->getKeyFrameDistance(mreadHashIdx, mreadHash);
			mfastBitRate->setAutoBitRateFactor(CFlvPool::instance()->getAutoBitRateFactor(mreadHashIdx, mreadHash));
			mfastBitRate->setAutoFrameFactor(CFlvPool::instance()->getAutoFrameFactor(mreadHashIdx, mreadHash));
			logs->info("%s [CFlvTransmission::doTransmission] %s cache=%lld, "
				"keyFrameDistance=%lu, "
				"autoBitRateFactor=%d, "
				"autoFrameFactor=%d",
				mprotocol->remoteAddr().c_str(),
				mprotocol->getUrl().c_str(),
				mcacheTT,
				muiKeyFrameDistance,
				mfastBitRate->getAutoBitRateFactor(),
				mfastBitRate->getAutoFrameFactor());
		}
		mfastBitRate->dropVideoFrame(mcacheTT, s->miDataType, msliceFrameRate, tt, uiTimestamp, sliceNum);
	}
	if (isVideo && misPushTask) //推流没有动态丢帧
	{
		if (mfastBitRate->getTransCodeNeedDropVideo())//是否需要丢帧
		{
			if (s->misKeyFrame && mfastBitRate->isDropEnoughTime(uiTimestamp))//判断是否满足条件 重设丢帧标志
			{
				if (mfastBitRate->getLoseBufferTimes() <= 0)
				{
					mfastBitRate->setTransCodeNeedDropVideo(false);
				}
			}
			else
			{
				needSend = false;
				mfastBitRate->dropOneFrame();
			}
		}
		if (mfastBitRate->getTransCodeNoNeedDropVideo())//动态码率 模拟丢帧逻辑 实际不丢帧
		{
			if (s->misKeyFrame)
			{
				if (mfastBitRate->getLoseBufferTimes() <= 0)
				{
					mfastBitRate->setTransCodeNoNeedDropVideo(false);
				}
			}
			else
			{
				mfastBitRate->dropOneFrame();
			}
		}
	}
	else if (isAudio)
	{

	}
	int dropPer = mfastBitRate->dropFramePer(tt, msliceFrameRate);
	if (dropPer > 0)
	{
		logs->info("%s [CFlvTransmission::doTransmission] %s doTransmission dropPer %d",
			mprotocol->remoteAddr().c_str(), mprotocol->getUrl().c_str(), dropPer);
	}
}





