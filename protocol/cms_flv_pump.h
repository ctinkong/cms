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
#ifndef __CMS_FLV_PUMP_H__
#define __CMS_FLV_PUMP_H__
#include <common/cms_type.h>
#include <strategy/cms_jitter.h>
#include <protocol/cms_rtmp_const.h>
#include <flvPool/cms_flv_pool.h>
#include <protocol/cms_amf0.h>
#include <interface/cms_stream_info.h>
#include <mem/cms_mf_mem.h>
#ifdef __CMS_CYCLE_MEM__
#include <mem/cms_cycle_mem.h>
#endif
#include <string>

class CFlvPump
{
public:
	CFlvPump(CStreamInfo *super, HASH &hash, uint32 &hashIdx, std::string remoteAddr, std::string modeName, std::string url);
	~CFlvPump();

	int	decodeMetaData(char *data, int len, bool &isChangeMediaInfo);
	int decodeVideo(char *data, int len, uint32 timestamp, bool &isChangeMediaInfo);
	int decodeAudio(char *data, int len, uint32 timestamp, bool &isChangeMediaInfo);

	int getWidth();
	int getHeight();
	int getMediaRate();
	int getVideoFrameRate();
	int getVideoRate();
	int getAudioFrameRate();
	int getAudioRate();
	int getAudioSampleRate();
	byte getVideoType();
	byte getAudioType();
	void stop();
	
	void setPublish();

	OperatorNewDelete
private:
	void copy2Slice(Slice *s);
	HASH	mhash;
	uint32	mhashIdx;
	CStreamInfo *msuper;
	//流信息
	int   miWidth;
	int   miHeight;
	int   miMediaRate;
	int   miVideoFrameRate;
	int   miVideoRate;
	int   miAudioFrameRate;
	int   miAudioRate;
	int   miAudioSamplerate;
	byte  mvideoType;
	byte  maudioType;
	//
	std::string murl;
	std::string mremoteAddr;
	std::string mmodeName;
	CJitter			*mjitter;
	int64			mcreateTT;

	bool misH264;
	bool misH265;

	int64           mllIdx;
	bool			misPublish;		//是否是客户端publish
	bool            misPushFlv;		//是否往flvPool投递过数据
};
#endif
