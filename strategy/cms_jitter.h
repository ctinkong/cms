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
#ifndef __CMS_JITTER_H__
#define __CMS_JITTER_H__
#include <common/cms_type.h>
#include <string>

class CJitter
{
public:
	CJitter();
	~CJitter();

	bool	isInit();
	void	init(std::string remoteAddr, std::string modeName, std::string url);
	void	reset();
	void	jitterTimestamp(float tba, float tbv, float lastTimestamp,
		uint32 curTimestamp, bool isVideo, float &rlastTimestamp, uint32 &rcurTimestamp);
	uint32  judgeNeedJitter(bool isVideo, uint32 absoluteTimestamp);
	bool	countVideoAudioFrame(bool isVideo, int64 tk, uint32 absoluteTimestamp);
	float	getAudioJitterFrameRate();
	float	getVideoJitterFrameRate();
	void	setAudioJitterFrameRate(float audioJitterFrameRate);
	void	setVideoJitterFrameRate(float videoJitterFrameRate);
	float	getAudioJitterCountFrameRate();
	void	setAudioFrameRate(int audioFrameRate);
	void	setVideoFrameRate(int videoFrameRate);
	void	setOpenJitter(bool is);
	void	setOpenForceJitter(bool is);
	bool	isOpenJitter();
	bool	isForceJitter();
private:
	bool			misInit;
	std::string		murl;
	std::string		mremoteAddr;
	std::string		modeName;
	int				mvideoFrameRate;
	int				maudioFrameRate;
	bool			misOpenJitter;
	bool			misForceJitter;
	int				mvideoaudioTimestampIllegal;		//音视频时间戳差异
	int				mvideoJitterTimestampIllegal;		//视频时间戳非法增长次数
	float			mabsLastVideoTimestamp;				//最后一次更新的视频绝对时间戳
	float			mvideoJitterFrameRate;				//视频帧率
	int				maudioJitterTimestampIllegal;		//音频时间戳非法增长次数
	float			mabsLastAudioTimestamp;				//最后一次更新的音频绝对时间戳
	float			maudioJitterFrameRate;				//音频帧率
	int				mvideoJitterNum;					//累计纠正多少帧
	float			mvideoJitterDetalTotal;				//累计纠正时长
	int				maudioJitterNum;					//累计纠正多少帧
	float			maudioJitterDetalTotal;				//累计纠正时长
	bool			misJitterAudio;						//是否需要纠正音频帧
	bool			misJitterVideo;						//是否需要纠正视频帧
		//人为统计视频帧率,只有统计的帧率和获取到的帧率相接近时，才会启用时间纠正
	float			mvideoJitterCountFrameRate;			//统计的视频帧率
	uint32			mvideoBeginJitterTimestamp;			//
	float			mvideoJitterGrandTimestmap;			//视频纠正增量
	bool			misCountVideoFrameRate;
	bool			misLegallVideoFrameRate;
	//人为统计音频帧率,只有统计的帧率和获取到的帧率相接近时，才会启用时间纠正
	float			maudioJitterCountFrameRate;
	uint32			maudioBeginJitterTimestamp;
	float			maudioJitterGrandTimestmap;			//音频纠正增量
	bool			misCountAudioFrameRate;
	bool			misLegallAudioFrameRate;
	int				mvideoAudioJitterBalance;			//音视频天平，正数表示视频，负数表示音频
};
#endif
