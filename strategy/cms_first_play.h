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
#ifndef __CMS_FIRST_PLAY_H__
#define __CMS_FIRST_PLAY_H__
#include <common/cms_type.h>
#include <flvPool/cms_flv_pool.h>
#include <string>

class CFirstPlay
{
public:
	CFirstPlay();
	~CFirstPlay();

	bool	isInit();
	void	init(HASH &hash, uint32 &hashIdx, std::string remoteAddr, std::string modeName, std::string url);
	bool	checkfirstPlay();
	bool	checkShouldDropFrameCount(int64 &transIdx, Slice *s);
	bool	needDropFrame(Slice *s);
private:
	bool			misInit;
	std::string		murl;
	std::string		mremoteAddr;
	std::string		modeName;
	HASH			mhash;
	uint32			mhashIdx;

	//首次播放
	int32	mfirstPlaySkipMilSecond;		//首播丢帧毫秒数
	int32	mdistanceKeyFrame;				//关键帧距离
	int32	mdropSliceNum;					//丢帧数
	int32	mhaveDropSliceNum;				//已经丢帧数
	bool	misSetFirstFrame;
	int		mvideoFrameRate;
	int		maudioFrameRate;
	int64	beginTT;
};
#endif
