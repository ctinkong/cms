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
#ifndef __CMS_STATIC_COMMON_H__
#define __CMS_STATIC_COMMON_H__
#include <common/cms_type.h>
#include <mem/cms_mf_mem.h>
#include <string>

#define PACKET_ONE_TASK_DOWNLOAD	0x00
#define PACKET_ONE_TASK_UPLOAD		0x01
#define PACKET_ONE_TASK_MEDIA		0x02
#define PACKET_ONE_TASK_MEM			0x03

#define PACKET_CONN_ADD				0x01
#define PACKET_CONN_DEL				0x02
#define PACKET_CONN_DATA			0x03

struct OneTaskPacket
{
	int	packetID;
};

struct OneTaskDownload
{
	int		packetID;
	HASH	hash;
	int32	downloadBytes;					//下载字节数
	bool	isRemove;
	bool    isFromeTask;					//是否是直播流下载任务 是则为true，如果是播放请求或者是转推流请求 则为false
	bool    isPublishTask;					//是否推流任务

	OperatorNewDelete
};

struct OneTaskUpload
{
	int		packetID;
	HASH	hash;
	int32	uploadBytes;					//上传字节数
	int		connAct;

	OperatorNewDelete
};

struct OneTaskMeida
{
	int				packetID;
	HASH			hash;
	int32			videoFramerate;			//视频帧率
	int32			audioFramerate;			//音频帧率
	int32			audioSamplerate;		//音频采样率
	int32			mediaRate;				//直播流码率
	int32			width;					//视频宽
	int32			height;					//视频高
	std::string		videoType;				//视频类型
	std::string		audioType;				//音频类型
	std::string		remoteAddr;				//对端ip:port
	std::string		url;					//url地址
	bool			isUdp;				    //是否是udp连接

	OperatorNewDelete
};

struct OneTaskMem
{
	int		packetID;
	HASH	hash;
	int64	totalMem;
#ifdef __CMS_CYCLE_MEM__
	int64	totalCycMem;
#endif

	OperatorNewDelete
};

struct OneTask
{
	std::string		murl;
	int64			mdownloadTotal;		//用于统计下载速度
	int64			mdownloadTick;
	int64			mdownloadSpeed;
	uint64			mdownloadTT;

	int64			muploadTotal;		//用于统计上传速度
	int64			muploadTick;
	int64			muploadSpeed;
	uint64			muploadTT;

	int32			mmediaRate;			//直播流码率
	int32			mvideoFramerate;	//视频帧率
	int32			maudioFramerate;	//音频帧率
	int32			maudioSamplerate;	//音频采样率
	int32			miWidth;			//视频宽
	int32			miHeight;			//视频高
	std::string		mvideoType;			//视频类型
	std::string		maudioType;			//音频类型

	int32			mtotalConn;			//该任务当前连接数
	std::string		mreferer;			//refer

	int64			mtotalMem;			//当前任务数据占用内存
#ifdef __CMS_CYCLE_MEM__
	int64			mtotalCycMem;		//循环内存大小
#endif

	time_t			mttCreate;
	std::string		mremoteAddr;

	bool			misUDP;
	bool			misPublishTask;

	OperatorNewDelete
};

struct CpuInfo
{
	long long user;
	long long nice;
	long long sys;
	long long idle;

	OperatorNewDelete
};

typedef struct
{
	/** 01 */ char interface_name[128]; /** 网卡名，如eth0 */

	/** 接收数据 */
	/** 02 */ unsigned long receive_bytes;             /** 此网卡接收到的字节数 */
	/** 03 */ unsigned long receive_packets;
	/** 04 */ unsigned long receive_errors;
	/** 05 */ unsigned long receive_dropped;
	/** 06 */ unsigned long receive_fifo_errors;
	/** 07 */ unsigned long receive_frame;
	/** 08 */ unsigned long receive_compressed;
	/** 09 */ unsigned long receive_multicast;

	/** 发送数据 */
	/** 10 */ unsigned long transmit_bytes;             /** 此网卡已发送的字节数 */
	/** 11 */ unsigned long transmit_packets;
	/** 12 */ unsigned long transmit_errors;
	/** 13 */ unsigned long transmit_dropped;
	/** 14 */ unsigned long transmit_fifo_errors;
	/** 15 */ unsigned long transmit_collisions;
	/** 16 */ unsigned long transmit_carrier;
	/** 17 */ unsigned long transmit_compressed;

	OperatorNewDelete
}net_info_t;


OneTask *newOneTask();
//static 函数
void makeOneTaskDownload(HASH &hash, 
	int32 downloadBytes, 
	bool isRemove, 
	bool isFromeTask,
	bool isPublishTask = false);							//统计下载任务
void makeOneTaskupload(HASH	&hash, 
	int32 uploadBytes, 
	int connAct);											//统计上传任务
void makeOneTaskMedia(HASH	&hash, 
	int32 videoFramerate, 
	int32 audioFramerate, 
	int32 iWidth, 
	int32 iHeight,											//统计任务媒体信息
	int32 audioSamplerate, 
	int32 mediaRate, 
	std::string videoType, 
	std::string audioType, 
	std::string url,
	std::string remoteAddr, 
	bool isUdp);
#ifdef __CMS_CYCLE_MEM__
void makeOneTaskMem(HASH hash,
	int64 totalMem, 
	int64 totalCycMem);										//统计内存占用
#else
void makeOneTaskMem(HASH hash,
	int64 totalMem);										//统计内存占用
#endif

#endif
