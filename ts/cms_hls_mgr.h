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
#ifndef __CMS_HLS_MGR_H__
#define __CMS_HLS_MGR_H__
#include <common/cms_type.h>
#include <core/cms_thread.h>
#include <ts/cms_ts.h>
#include <core/cms_lock.h>
#include <strategy/cms_duration_timestamp.h>
#include <app/cms_app_info.h>
#include <flvPool/cms_flv_var.h>
#include <libev/libev-4.25/ev.h>
#include <string>
#include <vector>
#include <map>
#include <queue>


typedef struct _SSlice {
	int		mionly;		  //0 表示没被使用，大于0表示正在被使用次数
	uint64	msliceRange;  //切片时长
	int		msliceLen;    //切片大小
	int64	msliceIndex;  //切片序号
	uint64	msliceStart;  //切片开始时间戳
	std::vector<TsChunkArray *> marray;	  //切片数据
}SSlice;

SSlice *newSSlice();
void atomicInc(SSlice *s);
void atomicDec(SSlice *s);

class CMission
{
public:
	CMission(HASH &hash, uint32 hashIdx, std::string url,
		int tsDuration, int tsNum, int tsSaveNum);
	~CMission();

	int  run(int i, struct ev_loop *loop);
	int  doFirstVideoAudio(bool isVideo);
	int  doit();
	void stop();
	int  pushData(Slice *s, byte frameType, uint64 timestamp);
	int  getTS(int64 idx, SSlice **s);
	int  getM3U8(std::string addr, std::string &outData);
	int64 getLastTsTime();
	int64 getUid();
private:
	void initMux();
	EvTimerParam		mtimerEtp;
	struct ev_loop		*mevLoop;
	ev_timer		    mevTimer;

	uint64  muid;
	HASH	mhash;			//用来识别任务的hash值
	uint32  mhashIdx;		//
	std::string murl;		//拼接用的URL

	char    mszM3u8Buf[1024];
	std::string mm3u8ContentPart2;

	int		mtsDuration;    //单个切片时长
	int		mtsNum;         //切片上限个数
	int		mtsSaveNum;     //缓存保留的切片个数
	std::vector<SSlice *>	msliceList; //切片列表
	int		msliceCount;    //切片计数
	int64	msliceIndx;     //当前切片的序号

	bool	misStop;		//用来控制任务协程

	int64	mreadIndex;		//读取的帧的序号

	int64	mreadFAIndex;	//读取的音频首帧的序号
	int64	mreadFVIndex;	//读取的视频首帧的序号

	bool	mFAFlag;	//是否读到首帧音频
	bool	mFVFlag;	//是否读到首帧视频(SPS/PPS)
	int64	mbTime;		//最后一个切片的生成时间
	uint32  mtimeStamp;        //读到的上一帧的时间戳，有时时间戳重置了会用到
	bool	mbFIFrame;  //首个I帧，用来做切片的参考
	CMux	*mMux;      //转码器
	TsChunkArray *mlastTca;//节省空间

	uint64  mullTransUid;

	CDurationTimestamp *mdurationtt;
};

class CMissionMgr
{
public:
	CMissionMgr();
	~CMissionMgr();

	static void *routinue(void *param);
	void thread(uint32 i);
	bool run();
	void stop();

	static CMissionMgr *instance();
	static void freeInstance();
	/*创建一个任务
	-- idx hash对应的索引号,切片内部需要保存
	-- hash 任务哈希
	-- url任务url
	-- tsDuration 一个切片ts的时长
	-- tsNum 该m3u8的ts片数
	-- tsSaveNum 切片模块缓存ts片数
	*/
	int	 create(uint32 i, HASH &hash, std::string url, int tsDuration, int tsNum, int tsSaveNum);
	/*销毁一个任务
	-- hash 任务哈希
	*/
	void destroy(uint32 i, HASH &hash);
	/*根据任务读取m3u8或ts
	-- hash 任务哈希
	-- url m3u8或者ts的地址
	*/
	int  readM3U8(uint32 i, HASH &hash, std::string url, std::string addr, std::string &outData, int64 &tt);
	int  readTS(uint32 i, HASH &hash, std::string url, std::string addr, SSlice **ss, int64 &tt);
	/*管理器的释放，管理器会有一些超时数据的缓存*/
	void release();
	void tick(uint32 i, uint64 uid);

	void tsAliveCallBack(struct ev_loop *loop, struct ev_timer *watcher, int revents);
	void tsTimerCallBack(struct ev_loop *loop, struct ev_timer *watcher, int revents);

private:
	struct ev_loop *mevLoop[APP_ALL_MODULE_THREAD_NUM];
	EvTimerParam	mtimerEtp[APP_ALL_MODULE_THREAD_NUM];
	ev_timer		mevTimer[APP_ALL_MODULE_THREAD_NUM];

	static CMissionMgr *minstance;
	cms_thread_t mtid[APP_ALL_MODULE_THREAD_NUM];
	bool misRunning[APP_ALL_MODULE_THREAD_NUM];
	std::map<HASH, CMission *> mMissionMap[APP_ALL_MODULE_THREAD_NUM];			//任务列表
	std::map<int64, CMission *> mMissionUidMap[APP_ALL_MODULE_THREAD_NUM];			//任务列表
	CRWlock					  mMissionMapLock[APP_ALL_MODULE_THREAD_NUM];

	std::map<HASH, int64>	  mMissionSliceCount[APP_ALL_MODULE_THREAD_NUM];	//任务的切片记录
	CRWlock					  mMissionSliceCountLock[APP_ALL_MODULE_THREAD_NUM];
};

struct HlsMgrThreadParam
{
	CMissionMgr *pinstance;
	uint32 i;
};

#endif
