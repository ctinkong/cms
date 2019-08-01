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
#include <mem/cms_mf_mem.h>

#define PATpid	0
#define PMTpid	0x1000
#define Apid	0x101
#define Vpid	0x100
#define PCRpid	0x100

#define CMS_TS_TIMEOUT_MILSECOND 0.1

//FLV头基本信息
typedef struct _SHead {
	byte	version;	//
	byte	streamInfo; //流信息 4-a 1-v 5-a/v
	int		lenght;		//头长度
}SHead;
//FLV Tag头基本信息
typedef struct _STagHead {
	byte	tagType;      //tag类型
	int		dataSize;     //tag长度
	uint32	timeStamp;	  //时间戳
	int		streamId;     //流ID
	int		deviation;    //时间戳偏移量
}STagHead;
//FLV 音频Tag信息
typedef struct _SAudioInfo
{
	byte	codeType;	//编码类型
	byte	rate;		//采样率
	byte	precision;	//精度
	byte	audioType;	//音频类型
}SAudioInfo;
//FLV 视频Tag信息
typedef struct _SVideoInfo
{
	byte	framType; //帧类型
	byte	codeId;   //编码类型
}SVideoInfo;
//FLV ScriptTag信息
typedef struct _SDataInfo
{
	int duration;		//时长
	int width;			//视频宽度
	int height;			//视频高度
	int videodatarate;	//视频码率
	int framerate;		//视频帧率
	int videocodecid;	//视频编码方式
	int audiosamplerate;//音频采样率
	int audiosamplesize;//音频采样精度
	int stereo;			//是否为立体声
	int audiocodecid;	//音频编码方式
	int filesize;		//文件大小
}SDataInfo;
//AAC特殊信息
typedef struct _SAudioSpecificConfig
{
	byte ObjectType;		//5
	byte SamplerateIndex;	//4
	byte Channels;			//4
	byte FramLengthFlag;	//1
	byte DependOnCCoder;	//1
	byte ExtensionFlag;		//1
}SAudioSpecificConfig;
//单个Tag信息
typedef struct _STagInfo
{
	STagHead	head;
	byte		flag; //v:video a:audio
	SAudioInfo	audio;
	SVideoInfo	video;
}STagInfo;

//hls mgr msg
typedef struct _HlsMissionMsg
{	
	int act;
	int tsDuration;
	int tsNum;
	int tsSaveNum;
	uint32 hasIdx;
	HASH hash;
	std::string url;

	OperatorNewDelete
}HlsMissionMsg;

#define CMS_HLS_CREATE 0x01
#define CMS_HLS_DELETE 0x02

#endif
