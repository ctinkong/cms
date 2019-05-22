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
#ifndef __CMS_FAST_BIT_RATE_H__
#define __CMS_FAST_BIT_RATE_H__
#include <common/cms_type.h>
#include <string>
#include <vector>
#include <queue>

#define DropVideoKeyFrameLen		4000
//动态码率
#define AUTO_DROP_CHANGE_BITRATE_CLOSE  0
#define AUTO_CHANGE_BITRATE_OPEN        1
#define AUTO_DROP_BITRATE_OPEN          2
#define AUTO_DROP_CHANGE_BITRATE_OPEN   3

class CFastBitRate
{
public:
	CFastBitRate();
	~CFastBitRate();
	bool	isInit();
	void	init(std::string remoteAddr, std::string modeName, std::string url, bool isWaterMark,
		uint32 waterMarkOriHashIdx, uint32 hashIdx, HASH &waterMarkOriHash, HASH &hash);
	void	setFirstLowBitRateTimeStamp(uint32 timestamp);
	void	changeBitRateLastTimestamp(int32 dateType, uint32 timestamp);
	uint32	changeBitRateSetTimestamp(int32 dateType, uint32 sendTimestamp);
	void	setChangeBitRate();
	void	resetDropFlag();
	int		dropFramePer(int64 te, int32 sliceFrameRate);
	void	dropOneFrame();
	bool	changeRateBit(uint32 hashIdxOld, HASH &hashOld, bool isReset,
		uint32 timestamp, std::string referUrl, uint32 &hashIdxRead, HASH &hashRead, int64 &transIdx, int32 &mode);
	bool	dropVideoFrame(int32 edgeCacheTT, int32 dataType, int32 sliceFrameRate,
		int64 te, uint32 sendTimestamp, int sliceNum);
	bool	isDropEnoughTime(uint32 sendTimestamp);
	void	resetDropFrameFlags();
	bool	needResetFlags(int32 dateType, uint32 sendTimestamp);
	void	setNo1VideoAudioTimestamp(bool isVideo, uint32 sendTimestamp);

	bool    isChangeBitRate();
	int		getAutoBitRateMode();
	int		getAutoBitRateFactor();
	void	setAutoBitRateFactor(int autoBitRateFactor);
	int		getAutoFrameFactor();
	void	setAutoFrameFactor(int autoFrameFactor);
	bool	getTransCodeNeedDropVideo();
	void	setTransCodeNeedDropVideo(bool is);
	bool	getTransCodeNoNeedDropVideo();
	void	setTransCodeNoNeedDropVideo(bool is);
	int32	getLoseBufferTimes();
	bool	isChangeVideoBit();
private:
	bool			misInit;
	std::string		murl;
	std::string		mremoteAddr;
	std::string		modeName;
	//水印流
	bool				misWaterMark;	//是否是水印流
	uint32				mwaterMarkOriHashIdx;
	HASH				mwaterMarkOriHash;
	uint32				mhashIdx;
	HASH				mhash;
	//码率切换
	int					mautoBitRateMode;					//丢帧、切码率、丢帧切码率标志
	bool				misChangeVideoBit;					//是否切换了码率
	uint32				mbeginVideoChangeTime;				//首播丢帧
	uint32				mendVideoChangeTime;	            //首播丢帧
	uint32				mfirstLowBitRateTimeStamp;			//首播切码率最后发送时间戳，用于切回正常码率时，匹配最佳发送位置
	std::vector<HASH>	mhashLowBitRate;					//低码率hash
	std::vector<uint32>	mhashIdxLowBitRate;					//低码率hashIdx
	int32				mhashLowBitRateIdx;                 //当前低码率索引
	int32				mmaxLowBitRateIdx;                  //最大码率索引
	int32				mchangeLowBitRateTimes;             //码率切换次数
	int64				mchangeBitRateTT;
	std::string			mlowBitRateUrl;
	HASH				mlowBitRateHash;
	int64				mcreateBitRateTaskTT;
	HASH				mcreateBitRateHash;
	int					mautoBitRateFactor;					//动态变码率系数
	int					mautoFrameFactor;					//动态丢帧系数
		//码率切换保证时间戳平滑
	uint32				mlastVideoChangeBitRateTimestamp;   //切换码率前最后视频时间戳
	int32				mlastVideoChangeBitRateRelativeTimestamp;   //切换码率后，lastVideoChangeBitRateTimestamp 与 切换码率后第一帧视频帧时间戳差异值
	bool				mneedChangeVideoTimestamp;                  //切换码率后，需要调整时间戳标志
	uint32				mlastAudioChangeBitRateTimestamp;	        //切换码率前最后音频时间戳
	int32				mlastAudioChangeBitRateRelativeTimestamp;	//切换码率后，lastVideoChangeBitRateTimestamp 与 切换码率后第一帧音频帧时间戳差异值
	bool				mneedChangeAudioTimestamp;					//切换码率后，需要调整时间戳标志
		//丢帧策略
	uint32				mno1AudioTimestamp;					//首帧音频时间戳
	uint32				mno1VideoTimestamp;					//首帧视频时间戳
	int64				mconnectTimestamp;					//连接时间戳
	bool				mtransCodeNeedDropVideo;			//是否需要丢帧
	bool				mtransCodeNoNeedDropVideo;			//是否需要丢帧
	int64				mlastDiffer;						//上次时间差异
	int32				mloseBufferTimes;					//预测卡顿次数
	int64				mloseBufferInterval;				//记录统计间隔时间
	uint32				mlastVideoTimestamp;
	uint32				mbeginDropVideoTimestamp;
	//丢帧码率切换
	std::queue<int>		mhistoryDropList;
	int					mhistoryDropTotal;
	int					mhistoryDropNum;
	int					mhistoryDropTime;
	int64				mhistoryDropTT;
};

#endif