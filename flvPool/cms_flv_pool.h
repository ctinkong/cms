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
#ifndef __CMS_FLV_POOL_H__
#define __CMS_FLV_POOL_H__
#include <protocol/cms_flv.h>
#include <flvPool/cms_flv_var.h>
#include <core/cms_lock.h>
#include <core/cms_thread.h>
#include <strategy/cms_fast_bit_rate.h>
#include <app/cms_app_info.h>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <set>
#include <assert.h>


Slice *newSlice();
StreamSlice *newStreamSlice();

void atomicInc(Slice *s);
void atomicDec(Slice *s);

class CAutoSlice
{
public:
	CAutoSlice(Slice *s);
	~CAutoSlice();

private:
	Slice * ms;
};

class CFlvPool
{
public:
	CFlvPool();
	~CFlvPool();

	static void *routinue(void *param);
	void thread(uint32 i);
	bool run();

	static CFlvPool *instance();
	static void freeInstance();

	void stop();
	uint32 hashIdx(HASH &hash);
	void push(uint32 i, Slice *s);
	bool pop(uint32 i, Slice **s);
	bool justJump2VideoLastXSecond(uint32 i, HASH &hash, uint32 &st, uint32 &ts, int64 &transIdx);
	int  readFirstVideoAudioSlice(uint32 i, HASH &hash, Slice **s, bool isVideo);
	int  readSlice(uint32 i, HASH &hash, int64 &llIdx, Slice **s, int &sliceNum, bool isTrans, int64 llMetaDataIdx, int64 llFirstVideoIdx, int64 llFirstAudioIdx,
		bool &isExist, bool &isTaskRestart, bool isPublishTask, bool &isMetaDataChanged, bool &isFirstVideoAudioChanged, uint64 &transUid);
	bool isHaveMetaData(uint32 i, HASH &hash);
	bool isExist(uint32 i, HASH &hash);
	bool isFirstVideoChange(uint32 i, HASH &hash, int64 &videoIdx);
	bool isFirstAudioChange(uint32 i, HASH &hash, int64 &audioIdx);
	bool isMetaDataChange(uint32 i, HASH &hash, int64 &metaIdx);
	int  readMetaData(uint32 i, HASH &hash, Slice **s);
	int  getFirstVideo(uint32 i, HASH &hash, Slice **s);
	int  getFirstAudio(uint32 i, HASH &hash, Slice **s);
	bool isH264(uint32 i, HASH &hash);
	bool isH265(uint32 i, HASH &hash);
	int64 getMinIdx(uint32 i, HASH &hash);
	int64 getMaxIdx(uint32 i, HASH &hash);
	int	  getMediaRate(uint32 i, HASH &hash);
	void  updateAccessTime(uint32 i, HASH &hash);
	int   getFirstPlaySkipMilSecond(uint32 i, HASH &hash);
	uint32 getDistanceKeyFrame(uint32 i, HASH &hash);
	int   getVideoFrameRate(uint32 i, HASH &hash);
	int   getAudioFrameRate(uint32 i, HASH &hash);

	int			readBitRateMode(uint32 i, HASH &hash);
	std::string readChangeBitRateSuffix(uint32 i, HASH &hash);
	std::string readCodeSuffix(uint32 i, HASH &hash);

	bool seekKeyFrame(uint32 i, HASH &hash, uint32 &st, int64 &transIdx);
	bool mergeKeyFrame(char *desc, int descLen, char *key, int keyLen, char **src, int32 &srcLen, std::string url);
	int64  getCacheTT(uint32 i, HASH &hash);
	uint32 getKeyFrameDistance(uint32 i, HASH &hash);
	int	   getAutoBitRateFactor(uint32 i, HASH &hash);
	int	   getAutoFrameFactor(uint32 i, HASH &hash);
private:
	void handleSlice(uint32 i, Slice *s);
	void clear();
	void delHash(uint32 i, HASH &hash);
	void addHash(uint32 i, HASH &hash);
	void checkTimeout();
	bool isTimeout(uint32 i, HASH &hash);
	void getRelativeDuration(StreamSlice *ss, Slice *s, bool isNewSlice,
		int64 &maxRelativeDuration, int64 &minRelativeDuration);


	bool					misRun;
	static CFlvPool			*minstance;

	CLock					mqueueLock[APP_ALL_MODULE_THREAD_NUM];
	std::queue<Slice *>		mqueueSlice[APP_ALL_MODULE_THREAD_NUM];

	CRWlock					mhashSliceLock[APP_ALL_MODULE_THREAD_NUM];
	std::map<HASH, StreamSlice *> mmapHashSlice[APP_ALL_MODULE_THREAD_NUM];

	std::set<HASH>			msetHash[APP_ALL_MODULE_THREAD_NUM]; //超时记录，记录任务多久没访问就删除
	CLock					msetHashLock[APP_ALL_MODULE_THREAD_NUM];

	cms_thread_t			mtid[APP_ALL_MODULE_THREAD_NUM];
};
#endif
