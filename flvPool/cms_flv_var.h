#ifndef __CMS_FLV_VAR_H__
#define __CMS_FLV_VAR_H__
#include <common/cms_type.h>
#include <core/cms_lock.h>
#include <mem/cms_mf_mem.h>
#ifdef __CMS_CYCLE_MEM__
#include <mem/cms_cycle_mem.h>
#endif
#include <string>
#include <vector>
#include <map>

enum FlvPoolCode
{
	FlvPoolCodeError = -1,
	FlvPoolCodeOK,
	FlvPoolCodeNoData,
	FlvPoolCodeRestart,
	FlvPoolCodeTaskNotExist
};

enum FlvPoolDataType
{
	DATA_TYPE_NONE = -0x01,
	DATA_TYPE_AUDIO = 0x00,
	DATA_TYPE_VIDEO = 0x01,
	DATA_TYPE_VIDEO_AUDIO = 0x02,
	DATA_TYPE_FIRST_AUDIO = 0x03,
	DATA_TYPE_FIRST_VIDEO = 0x04,
	DATA_TYPE_FIRST_VIDEO_AUDIO = 0x05,
	DATA_TYPE_DATA_SLICE = 0X06
};

#define FLV_TAG_AUDIO		0x08
#define FLV_TAG_VIDEO		0x09
#define FLV_TAG_SCRIPT		0x12

#define OFFSET_FIRST_VIDEO_FRAME 0x05

#define DropVideoMinSeconds 1500

struct StreamSlice;

typedef struct Slice //该结构体不能出现有构造函数的变量!!!!!!!!!!
{
	uint32			midxFixMem;			//固定内存所在队列索引号
	int				mionly;				//0 表示没被使用，大于0表示正在被使用次数	
	bool            misHaveMediaInfo;   //是否有修改过流信息
	bool			misPushTask;
	bool			misNoTimeout;
	bool			misMetaData;		//该数据是否是metaData
	bool			misRemove;			//删除任务标志
	bool			misRealTimeStream;	//是否从最新的数据发送
	bool			misKeyFrame;
	bool			misResetStreamTimestamp;
	bool			misH264;
	bool			misH265;
	FlvPoolDataType miDataType;			//数据类型
	int64			mllP2PIndex;		//p2p索引号
	int64			mllIndex;           //该slice对应的序列号
	int64			mllOffset;			//偏移位置（保留）
	int64			mllStartTime;		//任务开始时间
	int64			mllCacheTT;			//缓存时间 毫秒
	uint32			muiTimestamp;	    //该slice数据对应rtmp的时间戳
	int				miNotPlayTimeout;	//超时时间，毫秒	
	int				miMediaRate;
	int				miVideoRate;		//视频码率
	int				miAudioRate;		//音频码率
	int				miVideoFrameRate;	//视频帧率
	int				miAudioFrameRate;	//音频帧率	
	int				miAudioChannelID;	//连麦流音频ID
	int				miPlayStreamTimeout;		//多久没播放超时时间	
	int				miFirstPlaySkipMilSecond;	//首播丢帧时长
	int				miAutoBitRateMode;			//动态丢帧模式(0/1/2)
	int				miAutoBitRateFactor;		//动态变码率系数
	int				miAutoFrameFactor;			//动态丢帧系数
	int				miBufferAbsolutely;			//buffer百分比
	int				miLiveStreamTimeout;
	int				miNoHashTimeout;
	int             miDataLen;
	char			*mData;				//数据	
	char			*mpMajorHash;		//对于转码任务，该hash表示源流hash
	char			*mpHash;			//当前任务hash
	char			*mpUrl;
	char			*mpVideoType;		//视频类型
	char			*mpAudioType;		//音频类型
	char			*mpReferUrl;
	char			*mpRemoteIP;
	char			*mpHost;
#ifdef __CMS_CYCLE_MEM__
	CmsCycleMem     *mcycMem;
	struct StreamSlice * mss;
#endif
}Slice;

struct TTandKK
{
	int64			mllIndex;		//普通视频数据
	int64			mllKeyIndex;
	uint32			muiTimestamp;	//时间戳
};

struct StreamSlice
{
#ifdef __CMS_CYCLE_MEM__
	int							mionly;				//0 表示没被使用，大于0表示正在被使用次数
													//使用循环内存时，会被Slice引用
#endif
	//id唯一性 用于发送数据时 判断任务是否重启
	CRWlock						mLock;
	std::vector<TTandKK *>		msliceTTKK;	//按时间戳查找
	std::string					mstrUrl;
	std::string					mstrReferUrl;
	std::string					mstrVideoType;		//视频类型
	std::string					mstrAudioType;		//音频类型
	std::string					mstrRemoteIP;
	std::string					mstrHost;
	std::vector<Slice *>		mavSlice;
	std::vector<int64>			mavSliceIdx;
	std::vector<int64>			mvKeyFrameIdx;		//关键帧位置
	std::vector<int64>			mvP2PKeyFrameIdx;	//关键帧位置
	std::map<int64, int64>		mp2pIdx2PacketIdx;
	HASH						mhMajorHash;

	uint64						muid;
	//两个临时变量
	int64						maxRelativeDuration;
	int64						minRelativeDuration;	
	int64						mllNearKeyFrameIdx;	
	int64						mllAccessTime;		//记录时间戳，若一段时间没有用户访问，删除
	int64						mllCreateTime;		//任务创建时间
	uint32						muiTheLastVideoTimestamp;	
	int							miNotPlayTimeout;	//超时时间，毫秒	
	int							miVideoFrameCount;
	int							miAudioFrameCount;
	int							miMediaRate;
	int							miVideoRate;		//视频码率
	int							miAudioRate;		//音频码率
	int							miVideoFrameRate;	//视频帧率
	int							miAudioFrameRate;	//音频帧率
	bool						misPushTask;		//是否是推流任务
	bool						mnoTimeout;			//任务是否不超时
	bool						misH264;
	bool						misH265;
	
	uint32						muiKeyFrameDistance;
	uint32						muiLastKeyFrameDistance;
	int64						mllLastSliceIdx;
	int64						mllFirstVideoIdx;
	int64						mllFirstAudioIdx;
	Slice						*mfirstVideoSlice;	
	Slice						*mfirstAudioSlice;
	Slice						*mmetaDataSlice;
	
	int64						mllVideoAbsoluteTimestamp;	//用于计算缓存数据
	int64						mllAudioAbsoluteTimestamp;	//用于计算缓存数据	
	int64						mllMetaDataIdx;
	int64						mllMetaDataP2PIdx;

	bool						misNoTimeout;
	bool						misHaveMetaData;
	bool						misRealTimeStream;			//是否从最新的数据发送
	bool						misResetStreamTimestamp;
	int64						mllLastMemSize;
	int64						mllMemSize;
	int64						mllMemSizeTick;	
	int64						mllCacheTT;					//缓存时间 毫秒
	int							miPlayStreamTimeout;		//多久没播放超时时间	
	int							miFirstPlaySkipMilSecond;	//首播丢帧时长
	int							miAutoBitRateMode;			//动态丢帧模式(0/1/2)
	int							miAutoBitRateFactor;		//动态变码率系数
	int							miAutoFrameFactor;			//动态丢帧系数
	int							miBufferAbsolutely;			//buffer百分比
	

	//边缘才会用到的保存流断开时的状态
	bool						misNeedJustTimestamp;
	bool						misRemove;
	bool						misHasBeenRemove;
	int64						mllRemoveTimestamp;
	uint32						muiLastVideoTimestamp;
	uint32						muiLastAudioTimestamp;
	uint32						muiLast2VideoTimestamp;
	uint32						muiLast2AudioTimestamp;

	int64						mllUniqueID;

	//边推才有效
	int							miLiveStreamTimeout;
	int							miNoHashTimeout;

	OperatorNewDelete
};
#endif
