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
#include <ts/cms_hls_mgr.h>
#include <ts/cms_ts_chunk.h>
#include <log/cms_log.h>
#include <common/cms_utility.h>
#include <flvPool/cms_flv_var.h>
#include <flvPool/cms_flv_pool.h>
#include <app/cms_app_info.h>
#include <app/cms_parse_args.h>
#include <ts/cms_ts_callback.h>
#include <static/cms_static_common.h>
#include <mem/cms_mf_mem.h>
#include <time.h>
#include <errno.h>
#include <assert.h>

#define M3U8_CONTENT_PART1 "#EXTM3U\n\
#EXT-X-TARGETDURATION:%d\n\
#EXT-X-MEDIA-SEQUENCE:%lld\n"

//TEST
std::map<unsigned long, unsigned long> gmapSendTakeTime;
int64 gSendTakeTimeTT;
CLock gSendTakeTime;
//TEST end

uint64 guid[APP_ALL_MODULE_THREAD_NUM];
CLock guidLock[APP_ALL_MODULE_THREAD_NUM];

uint64 getUid(uint32 i)
{
	uint64 uid;
	guidLock[i].Lock();
	uid = guid[i];
	guid[i] += APP_ALL_MODULE_THREAD_NUM;
	guidLock[i].Unlock();
	return uid;
}

SSlice *newSSlice()
{
	SSlice *ss = new SSlice();
	ss->mionly = 1;
	ss->msliceRange = 0;  //切片时长
	ss->msliceLen = 0;    //切片大小
	ss->msliceIndex = 0;  //切片序号
	ss->msliceStart = 0;  //切片开始时间戳
	return ss;
}

void atomicInc(SSlice *s)
{
	__sync_add_and_fetch(&s->mionly, 1);//当数据超时，且没人使用时，删除
}

void atomicDec(SSlice *s)
{
	if (s && __sync_sub_and_fetch(&s->mionly, 1) == 0)//当数据超时，且没人使用时，删除
	{
		TsChunkArray *tca = NULL;
		//TsChunk *tc = NULL;
		std::vector<TsChunkArray *>::iterator itTca = s->marray.begin();
		for (; itTca != s->marray.end();)
		{
			tca = *itTca;
			freeTsChunkArray(tca);
			delete tca;
			itTca = s->marray.erase(itTca);
		}
		delete s;
	}
}

CMission::CMission(HASH &hash, uint32 hashIdx, uint32 threadID, std::string url,
	int tsDuration, int tsNum, int tsSaveNum)
{
	mthreadID = threadID;
	mevLoop = NULL;
	mtimerEtp.idx = 0;
	mtimerEtp.uid = 0;

	mhash = hash;
	mhashIdx = hashIdx;
	murl = url;

	muid = ::getUid(hashIdx%APP_ALL_MODULE_THREAD_NUM);

	mtsDuration = tsDuration;
	mtsNum = tsNum;
	mtsSaveNum = tsSaveNum;
	msliceCount = 0;    //切片计数
	msliceIndx = 0;     //当前切片的序号

	if (mtsSaveNum < mtsNum)
	{
		logs->warn("[CMission::CMission] %s ts save num %d is less than ts num %d", mtsSaveNum, mtsNum);
		mtsSaveNum = mtsNum + 2;
	}

	misStop = false;

	mreadIndex = -1;	//读取的帧的序号
	mreadFAIndex = -1;	//读取的音频首帧的序号
	mreadFVIndex = -1;	//读取的视频首帧的序号

	mFAFlag = false;	//是否读到首帧音频
	mFVFlag = false;	//是否读到首帧视频(SPS/PPS)
	mbTime = 0;			//最后一个切片的生成时间
	mbFIFrame = false;
	mullTransUid = 0;
	mtimeStamp = 0;

	mtotalMemSize = 0;
	mtotalDataSize = 0;
	mmemTick = getTickCount();

	mdurationtt = new CDurationTimestamp();
	initMux();
}

CMission::~CMission()
{
	if (mMux)
	{
		delete mMux;
	}
	if (mdurationtt)
	{
		delete mdurationtt;
	}
	std::vector<SSlice *>::iterator it = msliceList.begin();
	for (; it != msliceList.end();)
	{
		atomicDec(*it);
		it = msliceList.erase(it);
	}
}

void CMission::initMux()
{
	mMux = new CMux(); //转码器
	mMux->init(mthreadID);
	mMux->packPSI();
	mlastTca = NULL;
	SSlice *ss = newSSlice();
	ss->msliceIndex = msliceIndx;
	mlastTca = allocTsChunkArray();
	writeChunk(mthreadID, mlastTca, (char *)mMux->getPAT(), TS_CHUNK_SIZE);
	writeChunk(mthreadID, mlastTca, (char *)mMux->getPMT(), TS_CHUNK_SIZE);
	ss->marray.push_back(mlastTca);
	msliceList.push_back(ss);
}

int CMission::doFirstVideoAudio(bool isVideo)
{
	int ret = CMS_OK;
	Slice *s = NULL;
	if (CFlvPool::instance()->readFirstVideoAudioSlice(mhashIdx, mhash, &s, isVideo) == FlvPoolCodeError)
	{
		return ret;
	}
	assert(s != NULL);
	byte *outBuf = NULL;
	int  outLen = 0;
	if (isVideo)
	{
		logs->debug(">>>>>[CMission::doFirstVideoAudio] %s doFirstVideoAudio parse first video",
			murl.c_str());
		mreadFVIndex = s->mllIndex;
		mMux->parseNAL((byte *)s->mData, s->miDataLen, &outBuf, outLen);
		mFVFlag = true;
	}
	else
	{
		logs->debug(">>>>>[CMission::doFirstVideoAudio] %s doFirstVideoAudio parse first audio",
			murl.c_str());
		mreadFAIndex = s->mllIndex;
		mMux->parseAAC((byte *)s->mData, s->miDataLen, &outBuf, outLen);
		mFAFlag = true;
	}
	if (outLen)
	{
		xfree(outBuf);
	}
	atomicDec(s);
	return ret;
}

int  CMission::doit()
{
	int ret = 0;
	Slice *s = NULL;
	int flvPoolCode;
	uint32 uiTimestamp = 0;
	int	sliceNum = 0;
	bool isVideo = false;
	bool isAudio = false;
	bool isTransPlay = false;
	bool isExist = false;
	bool isPublishTask = false;
	bool isTaskRestart = false;
	bool isMetaDataChanged = false;
	bool isFirstVideoAudioChanged = false;
	int64 llMetaDataIdx = -1;
	bool needHandle = true;
	byte frameType;
	do
	{
		isTransPlay = false;
		isExist = false;
		isPublishTask = false;
		isTaskRestart = false;
		isMetaDataChanged = false;
		isFirstVideoAudioChanged = false;
		needHandle = true;

		sliceNum = 0;
		flvPoolCode = CFlvPool::instance()->readSlice(mhashIdx, mhash, mreadIndex, &s, sliceNum, isTransPlay,
			llMetaDataIdx, mreadFVIndex, mreadFAIndex, isExist, isTaskRestart, isPublishTask, isMetaDataChanged, isFirstVideoAudioChanged, mullTransUid);
		CAutoSlice autoSlice(s);
		if (!isExist)
		{
			ret = 2;
			break;
		}

		if (flvPoolCode == FlvPoolCodeError)
		{
			//logs->error("*** [CMission::doit] %s task is missing ***",
			//	murl.c_str());
			//ret = -1;
			break;
		}
		else if (flvPoolCode == FlvPoolCodeOK)
		{
			//首帧改变
			if (isFirstVideoAudioChanged)
			{
				if (CFlvPool::instance()->isFirstVideoChange(mhashIdx, mhash, mreadFVIndex))
				{
					if (doFirstVideoAudio(true) == CMS_ERROR)
					{
						ret = -1;
						break;
					}
				}
				if (CFlvPool::instance()->isFirstAudioChange(mhashIdx, mhash, mreadFAIndex))
				{
					if (doFirstVideoAudio(false) == CMS_ERROR)
					{
						ret = -1;
						break;
					}
				}
			}

			if (!mdurationtt->isInit())
			{
				mdurationtt->init("0.0.0.0", "flv", murl);
				mdurationtt->setResetTimestamp(false);
			}
			if (s)
			{
				isVideo = s->miDataType == DATA_TYPE_VIDEO;
				isAudio = s->miDataType == DATA_TYPE_AUDIO;

				uiTimestamp = s->muiTimestamp;
				//预防时间戳变小的情况
				uiTimestamp = mdurationtt->keepTimestampIncrease(isVideo, uiTimestamp);
				//预防时间戳变小的情况 结束
				if (mreadIndex + 1 != s->mllIndex && mreadIndex != -1)
				{
					logs->warn("*** [CMission::doit] %s doit drop frame,cur idx %lld,next idx %lld ***",
						murl.c_str(), mreadIndex, s->mllIndex);
				}
				mreadIndex = s->mllIndex;

				if (isAudio) {
					frameType = 'A';
					if ((s->mData[0] & 0xF0) >> 4 == 10)
					{
						mMux->setAudioType(0x0f);
					}
					else if ((s->mData[0] & 0xF0) >> 4 == 2)
					{
						mMux->setAudioType(0x03);
						mFAFlag = true;
					}
					if (!mFAFlag)
					{
						needHandle = false;
					}
				}
				else if (isVideo)
				{
					frameType = 'P';
					if (s->mData[0] == VideoTypeAVCKey/*0x17*/)
					{
						frameType = 'I';
					}
					if (s->miDataLen > 0 && frameType == 'I' && mbFIFrame == false) {
						mbFIFrame = true;
						logs->debug("[slice][OnTime]Found The First Key Frame !!!DataLen=%d", s->miDataLen);
					}
					if (!mFVFlag)
					{
						needHandle = false;
						logs->error("[slice][OnTime] Never Found SPS And PPS!!! %s", murl.c_str());
					}
				}
				if (needHandle) {
					pushData(s, frameType, s->muiTimestamp);
					mtimeStamp = s->muiTimestamp;
				}
				if (ret == CMS_ERROR)
				{
					ret = -1;
					break;
				}
				ret = 1;
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
			mreadIndex = -1;
			mreadFVIndex = -1;
			mreadFAIndex = -1;

			uint32 sendTimestamp = 0;
			if (s != NULL)
			{
				sendTimestamp = s->muiTimestamp;
			}

			mdurationtt->resetDeltaTimestamp(sendTimestamp);
			logs->debug("[CMission::doit] %s doit task is been restart",
				murl.c_str());
			ret = 0;
			break;
		}
		else
		{
			logs->error("*** [CMission::doit] %s doit unknow FlvPoolCode=%d ***",
				murl.c_str(), flvPoolCode);
			ret = -1;
			break;
		}
	} while (true);
	return ret;
}

int CMission::run(int i, struct ev_loop *loop)
{
	mevLoop = loop;
	//timer
	mtimerEtp.idx = i;
	mtimerEtp.uid = muid;
	mevTimer.data = (void *)&mtimerEtp;
	ev_timer_init(&mevTimer, ::tsTimerCallBack, 0., CMS_TS_TIMEOUT_MILSECOND); //检测线程是否被外部停止
	ev_timer_again(mevLoop, &mevTimer); //需要重复
	return 0;
}

void CMission::stop()
{
	if (mevLoop)
	{
		ev_timer_stop(mevLoop, &mevTimer);
	}
}

/*输入一帧TS数据
-- inbuf 送入的数据缓冲区
-- length 数据大小
-- framtype	帧类型(用于切片)
-- timestamp 时间戳
*/
int CMission::pushData(Slice *s, byte frameType, uint64 timestamp)
{
	// 	logs->debug("[CMission::pushData] 1 %s pushData one ts,timestamp=%lu-%lu, fremeType=%c, mFVFlag=%s, tsNum=%d, tsSaveNum=%d, duration=%d",
	// 		murl.c_str(),
	// 		timestamp,
	// 		msliceList[msliceCount]->msliceStart,
	// 		frameType,
	// 		mFVFlag ? "true" : "false",
	// 		mtsNum,
	// 		mtsSaveNum,
	// 		mtsDuration);
	unsigned long tt = getTickCount();
	SSlice *ss = NULL;
	if ((msliceList[msliceCount]->msliceStart != 0) &&
		timestamp - msliceList[msliceCount]->msliceStart > (uint64)(mtsDuration * 900) &&
		(frameType == 'I' /*|| (mFVFlag == false && frameType == 'A')*/))
	{
		int64 now = getTimeUnix();
		mbTime = now;
		if (msliceCount + 1 > /*mtsNum+*/mtsSaveNum)
		{
			//要加上追加的长度
			if (mlastTca != NULL)
			{
				ss = msliceList[msliceCount];
				if (ss != NULL)
				{
					ss->msliceLen += mlastTca->mchunkTotalSize;
				}
			}
			logs->debug("[CMission::pushData] 1 %s pushData one ts succ, "
				"count=%d, "
				"msliceList.size=%u, "
				"frameType=%c,"
				"cur timestamp=%lu, "
				"start timestamp=%lu, "
				"timestamp=%lu",
				murl.c_str(),
				msliceCount,
				msliceList.size(),
				frameType,
				timestamp,
				msliceList[msliceCount]->msliceStart,
				timestamp - msliceList[msliceCount]->msliceStart);

			msliceList[msliceCount]->msliceRange = (float)((timestamp - msliceList[msliceCount]->msliceStart));

			if (!msliceList[msliceCount]->marray.empty())
			{
				mtotalDataSize += msliceList[msliceCount]->marray.at(0)->mchunkTotalSize;
				mtotalMemSize += msliceList[msliceCount]->marray.at(0)->mtotalMemSize;
			}

			msliceIndx++;
			ss = newSSlice();
			ss->msliceIndex = msliceIndx;
			ss->msliceStart = timestamp;
			mMux->packPSI();

			mlastTca = allocTsChunkArray();
			writeChunk(mthreadID, mlastTca, (char *)mMux->getPAT(), TS_CHUNK_SIZE);
			writeChunk(mthreadID, mlastTca, (char *)mMux->getPMT(), TS_CHUNK_SIZE);
			mMux->onData(mlastTca, (byte*)s->mData, s->miDataLen, frameType, timestamp);

			ss->marray.push_back(mlastTca);
			msliceList.push_back(ss);

			std::vector<SSlice *>::iterator it = msliceList.begin();
			ss = *it;
			msliceList.erase(it);
			if (!ss->marray.empty())
			{
				mtotalDataSize -= ss->marray.at(0)->mchunkTotalSize;
				mtotalMemSize -= ss->marray.at(0)->mtotalMemSize;
			}
			atomicDec(ss);
		}
		else
		{
			//要加上追加的长度
			if (mlastTca != NULL)
			{
				ss = msliceList[msliceCount];
				if (ss != NULL)
				{
					ss->msliceLen += mlastTca->mchunkTotalSize;
				}
			}
			logs->debug("[CMission::pushData] 1 %s pushData one ts succ, "
				"count=%d, "
				"msliceList.size=%u, "
				"frameType=%c,"
				"cur timestamp=%lu, "
				"start timestamp=%lu, "
				"timestamp=%lu",
				murl.c_str(),
				msliceCount,
				msliceList.size(),
				frameType,
				timestamp,
				msliceList[msliceCount]->msliceStart,
				timestamp - msliceList[msliceCount]->msliceStart);
			msliceList[msliceCount]->msliceRange = (float)((timestamp - msliceList[msliceCount]->msliceStart));

			if (!msliceList[msliceCount]->marray.empty())
			{
				mtotalDataSize += msliceList[msliceCount]->marray.at(0)->mchunkTotalSize;
				mtotalMemSize += msliceList[msliceCount]->marray.at(0)->mtotalMemSize;
			}

			msliceCount++;
			msliceIndx++;
			ss = newSSlice();
			ss->msliceIndex = msliceIndx;
			ss->msliceStart = timestamp;
			mMux->packPSI();

			mlastTca = allocTsChunkArray();
			writeChunk(mthreadID, mlastTca, (char *)mMux->getPAT(), TS_CHUNK_SIZE);
			writeChunk(mthreadID, mlastTca, (char *)mMux->getPMT(), TS_CHUNK_SIZE);
			mMux->onData(mlastTca, (byte*)s->mData, s->miDataLen, frameType, timestamp);

			ss->marray.push_back(mlastTca);
			msliceList.push_back(ss);
		}
	}
	else
	{
		ss = msliceList[msliceCount];
		if (ss->msliceStart == 0)
		{
			ss->msliceStart = timestamp;
		}
		mMux->onData(mlastTca, (byte*)s->mData, s->miDataLen, frameType, timestamp);
	}
	if (tt - mmemTick > 1000)
	{
		makeOneTaskHlsMem(mhash,
			mtotalMemSize + (mlastTca ? mlastTca->mtotalMemSize : 0),
			mtotalDataSize + (mlastTca ? mlastTca->mchunkTotalSize : 0));
		mmemTick = tt;
	}
	return 0;
}

/*获取切片TS
-- Index 切片序号
*/
int  CMission::getTS(int64 idx, SSlice **s)
{
	if (idx >= msliceIndx || idx < msliceList[0]->msliceIndex)
	{
		return -1;
	}
	int64 pos = idx - msliceList[0]->msliceIndex;
	*s = msliceList[pos];
	atomicInc(*s);
	return 1;
}
/*获取最后一个切片*/
int  CMission::getLastTS(SSlice **s)
{
	if (msliceCount <= 0)
	{
		return -1;
	}
	*s = msliceList[msliceCount - 1];
	atomicInc(*s);
	return 1;
}
/*获取M3U8列表
-- addr 组装绝对路径的addr
*/
int  CMission::getM3U8(std::string addr, std::string &outData)
{
	mm3u8ContentPart2.clear();
	size_t end = murl.rfind("/");
	std::string url = murl.substr(7, end - 7);

	int pos = 0;
	if (msliceCount <= mtsNum)
	{
		pos = 0;
	}
	else if (msliceCount >= (mtsSaveNum/* + mtsNum*/))
	{
		pos = mtsSaveNum - mtsNum - 1;
	}
	else
	{
		pos = msliceCount - mtsNum - 1;
	}
	char szData[30] = { 0 };
	int64 sliceIndex = msliceList[pos]->msliceIndex;

	int maxDuration = 0;
	uint16 range = 0;
	int count = 0;
	for (int64 i = msliceList[pos]->msliceIndex; i < msliceIndx && count < mtsNum; i++)
	{
		range = 0;
		if (msliceList[pos]->msliceRange < 500)
		{
			range += 500;
		}
		else
		{
			range = msliceList[pos]->msliceRange;
		}
		//ms 转 s
		range = (range - 1000 / 2) / (1000) + 1;
		if (range > maxDuration)
		{
			maxDuration = range;
		}
		//logs->debug("\n\nm3u8 url %s ,murl %s,end %d",url.c_str(),murl.c_str(),end);
		snprintf(szData, sizeof(szData), "%d", range);
		mm3u8ContentPart2 += "#EXTINF:";
		mm3u8ContentPart2 += szData;
		mm3u8ContentPart2 += ",\n";
		mm3u8ContentPart2 += "http://";
		mm3u8ContentPart2 += addr;
		mm3u8ContentPart2 += "/";
		mm3u8ContentPart2 += url;
		mm3u8ContentPart2 += "/";
		snprintf(szData, sizeof(szData), "%lld", i);
		mm3u8ContentPart2 += szData;
		mm3u8ContentPart2 += ".ts\n";

		count++;
		pos++;
	}
	snprintf(mszM3u8Buf, sizeof(mszM3u8Buf), M3U8_CONTENT_PART1, maxDuration, sliceIndex);
	outData = mszM3u8Buf;
	outData += mm3u8ContentPart2;
	return 1;
}

int64 CMission::getLastTsTime()
{
	return mbTime;
}

int64 CMission::getUid()
{
	return muid;
}

//mgr
CMissionMgr *CMissionMgr::minstance = NULL;
CMissionMgr::CMissionMgr()
{
	for (int i = 0; i < APP_ALL_MODULE_THREAD_NUM; i++)
	{
		guid[i] = i;
		misRunning[i] = false;
		mtid[i] = 0;
		mevLoop[i] = NULL;
		mfdPipe[i][0] = 0;
		mfdPipe[i][1] = 0;
		memset(&mreadPipeEcp[i], 0, sizeof(EvCallBackParam));
		memset(&mevTimer[i], 0, sizeof(ev_timer));
		memset(&mevIO[i], 0, sizeof(ev_io));
	}
}

CMissionMgr::~CMissionMgr()
{

}

void *CMissionMgr::routinue(void *param)
{
	HlsMgrThreadParam *phmtp = (HlsMgrThreadParam *)param;
	CMissionMgr *pInstance = phmtp->pinstance;
	uint32 i = phmtp->i;
	delete phmtp;
	pInstance->thread(i);
	return NULL;
}

void CMissionMgr::thread(uint32 i)
{
	setThreadName("cms-hls-mgr");

	mtimerEtp[i].idx = i;
	mtimerEtp[i].uid = 0;
	mevTimer[i].data = (void *)&mtimerEtp[i];
	ev_timer_init(&mevTimer[i], ::tsAliveCallBack, 0., CMS_TS_TIMEOUT_MILSECOND); //检测线程是否被外部停止
	ev_timer_again(mevLoop[i], &mevTimer[i]); //需要重复

	ev_run(mevLoop[i], 0);
}

bool CMissionMgr::run()
{
	if (gSendTakeTimeTT == 0)
	{
		gSendTakeTimeTT = getTimeUnix();
	}
	for (uint32 i = 0; i < APP_ALL_MODULE_THREAD_NUM; i++)
	{
		if (pipe(mfdPipe[i]) != 0)
		{
			logs->error("[CMissionMgr::run] create pipe err,errno=%d, strerror=%s *****", errno, strerror(errno));
			return false;
		}
		nonblocking(mfdPipe[i][0]);//设置管道非阻塞

		misRunning[i] = true;
		HlsMgrThreadParam *hmtp = new HlsMgrThreadParam;
		hmtp->pinstance = this;
		hmtp->i = i;
		mevLoop[i] = ev_loop_new(EVBACKEND_EPOLL | EVBACKEND_POLL | EVBACKEND_SELECT | EVFLAG_NOENV);

		if (ev_backend(mevLoop[i]) & EVBACKEND_EPOLL)
		{
			printf("gsdn epoll mode\n");
		}
		else if (ev_backend(mevLoop[i]) & EVBACKEND_POLL)
		{
			printf("gsdn poll mode\n");
		}
		else if (ev_backend(mevLoop[i]) & EVBACKEND_SELECT)
		{
			printf("gsdn select mode\n");
		}

		mreadPipeEcp[i].base = this;
		mreadPipeEcp[i].idx = i;
		mevIO[i].data = (void*)&mreadPipeEcp[i];
		ev_io_init(&mevIO[i], ::hlsMgrPipeCallBack, mfdPipe[i][0], EV_READ);
		ev_io_start(mevLoop[i], &mevIO[i]);

		int res = cmsCreateThread(&mtid[i], routinue, hmtp, false);
		if (res == -1)
		{
			misRunning[i] = false;
			char date[128] = { 0 };
			getTimeStr(date);
			logs->error("%s ***** file=%s,line=%d cmsCreateThread error *****", date, __FILE__, __LINE__);
			return false;
		}
	}
#ifdef __CMS_POOL_MEM__
	initTsMem();
#endif
	return true;
		}

void CMissionMgr::stop()
{
	logs->debug("##### CMissionMgr::stop begin #####");
	for (uint32 i = 0; i < APP_ALL_MODULE_THREAD_NUM; i++)
	{
		misRunning[i] = false;
		cmsWaitForThread(mtid[i], NULL);
		mtid[i] = 0;
		ev_loop_destroy(mevLoop[i]);
		mevLoop[i] = NULL;

		if (mfdPipe[i][0])
		{
			::close(mfdPipe[i][0]);
		}
		if (mfdPipe[i][1])
		{
			::close(mfdPipe[i][1]);
		}
	}
#ifdef __CMS_POOL_MEM__
	releaseTsMem();
#endif
	logs->debug("##### CMissionMgr::stop finish #####");
		}

CMissionMgr *CMissionMgr::instance()
{
	if (minstance == NULL)
	{
		minstance = new CMissionMgr();
	}
	return minstance;
}

void CMissionMgr::freeInstance()
{
	if (minstance)
	{
		delete minstance;
		minstance = NULL;
	}
}

int	 CMissionMgr::create(uint32 i, HASH &hash, std::string url, int tsDuration, int tsNum, int tsSaveNum)
{
	if (g_isTestServer)
	{
		return CMS_OK;
	}
	HlsMissionMsg *msg = new HlsMissionMsg();
	msg->act = CMS_HLS_CREATE;
	msg->hasIdx = i;
	msg->hash = hash;
	msg->url = url;
	msg->tsDuration = tsDuration;
	msg->tsNum = tsNum;
	msg->tsSaveNum = tsSaveNum;

	uint32 idx = i % APP_ALL_MODULE_THREAD_NUM;
	mmsgLock[idx].Lock();
	mmsgQueue[idx].push(msg);
	mmsgLock[idx].Unlock();
	write(mfdPipe[idx][1], &i, sizeof(i));
	return CMS_OK;
}

void CMissionMgr::destroy(uint32 i, HASH &hash)
{
	HlsMissionMsg *msg = new HlsMissionMsg();
	msg->act = CMS_HLS_DELETE;
	msg->hasIdx = i;
	msg->hash = hash;

	uint32 idx = i % APP_ALL_MODULE_THREAD_NUM;
	mmsgLock[idx].Lock();
	mmsgQueue[idx].push(msg);
	mmsgLock[idx].Unlock();
	write(mfdPipe[idx][1], &i, sizeof(i));
}

int CMissionMgr::readHlsM3U8(uint32 i, HASH &hash, std::string url, std::string addr, std::string &outData, int64 &tt)
{
	i = i % APP_ALL_MODULE_THREAD_NUM;
	int ret = -1;
	mMissionMapLock[i].RLock();
	std::map<HASH, CMission *>::iterator it = mMissionMap[i].find(hash);
	if (it != mMissionMap[i].end())
	{
		it->second->getM3U8(addr, outData);
		tt = it->second->getLastTsTime();
		ret = 1;
	}
	mMissionMapLock[i].UnRLock();
	return ret;
}

int CMissionMgr::readHlsTS(uint32 i, HASH &hash, std::string url, std::string addr, SSlice **ss, int64 &tt)
{
	i = i % APP_ALL_MODULE_THREAD_NUM;
	int ret = -1;
	mMissionMapLock[i].RLock();
	std::map<HASH, CMission *>::iterator it = mMissionMap[i].find(hash);
	if (it != mMissionMap[i].end())
	{
		size_t start = url.rfind("/") + 1;
		if (start == std::string::npos)
		{

		}
		size_t end = url.find(".ts");
		if (end == std::string::npos)
		{

		}
		string strIdx = url.substr(start, end);
		int64 llIdx = _atoi64(strIdx.c_str());
		ret = it->second->getTS(llIdx, ss);
		tt = it->second->getLastTsTime();
	}
	mMissionMapLock[i].UnRLock();
	return ret;
}

int CMissionMgr::readTsStream(uint32 i, HASH &hash, int64 lastIdx, SSlice **ss)
{
	i = i % APP_ALL_MODULE_THREAD_NUM;
	int ret = -1;
	mMissionMapLock[i].RLock();
	std::map<HASH, CMission *>::iterator it = mMissionMap[i].find(hash);
	if (it != mMissionMap[i].end())
	{
		if (lastIdx == -1)
		{
			ret = it->second->getLastTS(ss);
		}
		else
		{
			ret = it->second->getTS(lastIdx, ss);
		}
	}
	mMissionMapLock[i].UnRLock();
	return ret;
}

void CMissionMgr::release()
{

}

void CMissionMgr::tick(uint32 i, uint64 uid)
{
	unsigned long tB = getTickCount();


	i = i % APP_ALL_MODULE_THREAD_NUM;
	mMissionMapLock[i].WLock();
	std::map<int64, CMission *>::iterator it = mMissionUidMap[i].find(uid);
	if (it != mMissionUidMap[i].end())
	{
		it->second->doit();
	}
	mMissionMapLock[i].UnWLock();



	unsigned long tE = getTickCount();
	gSendTakeTime.Lock();
	int64 sendTakeTimeTT = getTimeUnix();
	bool isPrintf = false;
	if (sendTakeTimeTT - gSendTakeTimeTT > 60)
	{
		isPrintf = true;
		gSendTakeTimeTT = sendTakeTimeTT;
	}
	printTakeTime(gmapSendTakeTime, tB, tE, (char *)"CMissionMgr", isPrintf);
	gSendTakeTime.Unlock();
}

void CMissionMgr::tsAliveCallBack(struct ev_loop *loop, struct ev_timer *watcher, int revents)
{
	EvTimerParam *etp = (EvTimerParam*)watcher->data;
	if (!misRunning[etp->idx])
	{
		logs->warn("@@@ [CMissionMgr::tsAliveCallBack] worker %d has been stop @@@", etp->idx);
		std::map<HASH, CMission *>::iterator it = mMissionMap[etp->idx].begin();
		for (; it != mMissionMap[etp->idx].end();)
		{
			it->second->stop();
			delete it->second;
			mMissionMap[etp->idx].erase(it++);
		}
		ev_break(EV_A_ EVBREAK_ALL);
	}
	else
	{

	}
}

void CMissionMgr::tsTimerCallBack(struct ev_loop *loop, struct ev_timer *watcher, int revents)
{
	EvTimerParam *etp = (EvTimerParam*)watcher->data;
	tick(etp->idx, etp->uid);
}

void CMissionMgr::hlsMgrPipeCallBack(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
	EvCallBackParam	*readPipeEcp = (EvCallBackParam *)watcher->data;
	uint32 &idx = readPipeEcp->idx;
	do
	{
		int n = read(watcher->fd, mpipeBuff[idx], CMS_PIPE_BUF_SIZE);
		if (n <= 0)
		{
			break;
		}
	} while (1);
	logs->debug("[CMissionMgr::hlsMgrPipeCallBack] hls-worker-%d handle one", idx);

	mmsgLock[idx].Lock();
	while (!mmsgQueue[idx].empty())
	{
		HlsMissionMsg* msg = mmsgQueue[idx].front();
		mmsgQueue[idx].pop();

		if (msg->act == CMS_HLS_CREATE)
		{
			mMissionMapLock[idx].WLock();
			std::map<HASH, CMission *>::iterator it = mMissionMap[idx].find(msg->hash);
			if (it == mMissionMap[idx].end())
			{
				CMission *cm = new CMission(msg->hash, msg->hasIdx, idx, msg->url, msg->tsDuration, msg->tsNum, msg->tsSaveNum);
				mMissionMap[idx][msg->hash] = cm;
				mMissionUidMap[idx][cm->getUid()] = cm;
				cm->run(idx, mevLoop[idx]);
				logs->debug("### [CMissionMgr::create] thread idx=%d have hls num %d ###", idx, mMissionUidMap[idx].size());
			}
			mMissionMapLock[idx].UnWLock();
		}
		else
		{
			mMissionMapLock[idx].WLock();
			std::map<HASH, CMission *>::iterator it = mMissionMap[idx].find(msg->hash);
			if (it != mMissionMap[idx].end())
			{
				std::map<int64, CMission *>::iterator it1 = mMissionUidMap[idx].find(it->second->getUid());
				if (it1 != mMissionUidMap[idx].end())
				{
					mMissionUidMap[idx].erase(it1);
				}

				it->second->stop();
				delete it->second;
				mMissionMap[idx].erase(it);
			}
			mMissionMapLock[idx].UnWLock();
		}
	}
	mmsgLock[idx].Unlock();
}





