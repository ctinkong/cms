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
#ifndef __CMS_TS_COMMON_H__
#define __CMS_TS_COMMON_H__
#include <common/cms_type.h>

#define PATpid  0
#define PMTpid  0x1000
#define Apid    0x101
#define Vpid    0x100
#define PCRpid  0x100

#define CMS_TS_TIMEOUT_MILSECOND 0.03

typedef struct _EvTimerParam
{
	uint32	  idx;
	uint64    uid;
}EvTimerParam;

//FLV头基本信息
typedef struct _SHead
{
	byte mversion;
	byte mstreamInfo;   //流信息 4-a 1-v 5-a/v
	int  mlenght;       //头长度
}SHead;

//FLV ScriptTag信息
typedef struct _SDataInfo
{
	int mduration;         //时长
	int	mwidth;            //视频宽度
	int	mheight;           //视频高度
	int	mvideodatarate;    //视频码率
	int	mframerate;        //视频帧率
	int	mvideocodecid;     //视频编码方式
	int	maudiosamplerate;  //音频采样率
	int	maudiosamplesize;  //音频采样精度
	int	mstereo;           //是否为立体声
	int	maudiocodecid;     //音频编码方式
	int	mfilesize;         //文件大小
}SDataInfo;

//FLV Tag头基本信息
typedef struct _STagHead
{
	byte	mtagType;		//tag类型
	int		mdataSize;      //tag长度
	uint32	mtimeStamp;		//时间戳
	int		mstreamId;      //流ID
	int		mdeviation;     //时间戳偏移量
}STagHead;

//FLV 音频Tag信息
typedef struct _SAudioInfo
{
	byte mcodeType;   //编码类型
	byte mrate;       //采样率
	byte mprecision;  //精度
	byte maudioType;  //音频类型
}SAudioInfo;

//FLV 视频Tag信息
typedef struct _SVideoInfo
{
	byte mframType;  //帧类型
	byte mcodeId;    //编码类型
}SVideoInfo;

//单个Tag信息
typedef struct _STagInfo
{
	STagHead	mhead;
	byte		mflag;   //v:video a:audio
	SAudioInfo	maudio;
	SVideoInfo	mvideo;
}STagInfo;

//AAC特殊信息
typedef struct _SAudioSpecificConfig
{
	byte	mObjectType;       //5
	byte	mSamplerateIndex;  //4
	byte	mChannels;         //4
	byte	mFramLengthFlag;   //1
	byte	mDependOnCCoder;   //1
	byte	mExtensionFlag;    //1
}SAudioSpecificConfig;
#endif
