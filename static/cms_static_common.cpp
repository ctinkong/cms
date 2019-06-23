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
#include <static/cms_static_common.h>
#include <static/cms_static.h>
#include <common/cms_utility.h>
#include <log/cms_log.h>

OneTask *newOneTask()
{
	OneTask *otk = new OneTask();
	otk->mdownloadTotal = 0;
	otk->mdownloadTick = 0;
	otk->mdownloadSpeed = 0;
	otk->mdownloadTT = getTickCount();

	otk->muploadTotal = 0;
	otk->muploadTick = 0;
	otk->muploadSpeed = 0;
	otk->muploadTT = getTickCount();

	otk->mvideoFramerate = 0;
	otk->maudioFramerate = 0;
	otk->maudioSamplerate = 0;
	otk->mmediaRate = 0;
	otk->miWidth = 0;
	otk->miHeight = 0;

	otk->mtotalConn = 0;		//该任务当前连接数

	otk->mtotalMem = 0;		//当前任务数据占用内存

	otk->mttCreate = getTimeUnix();
	return otk;
}

void makeOneTaskDownload(HASH &hash, 
	int32 downloadBytes, 
	bool isRemove, 
	bool isFromeTask,
	bool isPublishTask/* = false*/)
{
	OneTaskDownload *otd = new OneTaskDownload;
	otd->packetID = PACKET_ONE_TASK_DOWNLOAD;
	otd->hash = hash;
	otd->downloadBytes = downloadBytes;
	otd->isRemove = isRemove;
	otd->isFromeTask = isFromeTask;
	otd->isPublishTask = isPublishTask;

	CStatic::instance()->push((OneTaskPacket *)otd);
}

void makeOneTaskupload(HASH	&hash, 
	int32 uploadBytes, 
	int connAct)
{
	OneTaskUpload *otu = new OneTaskUpload;
	otu->packetID = PACKET_ONE_TASK_UPLOAD;
	otu->hash = hash;
	otu->uploadBytes = uploadBytes;
	otu->connAct = connAct;
	CStatic::instance()->push((OneTaskPacket *)otu);
}

void makeOneTaskMedia(HASH	&hash, 
	int32 videoFramerate, 
	int32 audioFramerate, 
	int32 iWidth, 
	int32 iHeight,
	int32 audioSamplerate, 
	int32 mediaRate, 
	std::string videoType, 
	std::string audioType, 
	std::string url,
	std::string remoteAddr, 
	bool isUdp)
{
	OneTaskMeida *otm = new OneTaskMeida;
	otm->packetID = PACKET_ONE_TASK_MEDIA;
	otm->hash = hash;
	otm->videoFramerate = videoFramerate;
	otm->audioFramerate = audioFramerate;
	otm->audioSamplerate = audioSamplerate;
	otm->mediaRate = mediaRate;
	otm->width = iWidth;
	otm->height = iHeight;
	otm->videoType = videoType;
	otm->audioType = audioType;
	otm->remoteAddr = remoteAddr;
	otm->url = url;
	otm->isUdp = isUdp;
	CStatic::instance()->push((OneTaskPacket *)otm);
}

void makeOneTaskMem(HASH &hash, 
	int64 totalMem)
{
	OneTaskMem *otm = new OneTaskMem;
	otm->packetID = PACKET_ONE_TASK_MEM;
	otm->hash = hash;
	otm->totalMem = totalMem;
	CStatic::instance()->push((OneTaskPacket *)otm);
}
