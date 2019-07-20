/*
The MIT License (MIT)

Copyright (c) 2017- cms(hsc)

Author: Ìì¿ÕÃ»ÓÐÎÚÔÆ/kisslovecsh@foxmail.com

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
#ifndef __CMS_STATIC_H__
#define __CMS_STATIC_H__
#include <common/cms_type.h>
#include <core/cms_lock.h>
#include <core/cms_thread.h>
#include <cJSON/cJSON.h>
#include <static/cms_static_common.h>
#include <mem/cms_mf_mem.h>
#include <string>
#include <map>
#include <queue>

class CStatic
{
public:
	CStatic();
	~CStatic();

	static CStatic *instance();
	static void freeInstance();

	bool run();
	void stop();
	void thread();
	static void *routinue(void *param);
	void setAppName(std::string appName);

	void push(OneTaskPacket *otp);
	std::string dump();

	OperatorNewDelete
private:
	bool pop(OneTaskPacket **otp);

	void handle(OneTaskDownload *otd);
	void handle(OneTaskUpload *otu);
	void handle(OneTaskMeida *otm);
	void handle(OneTaskMem *otm);

	int getTaskInfo(cJSON **value);

	float		getMemUsage();
	int			getMemSize();
	int			getCpuUsage();
	std::string getUploadBytes();
	std::string getDownloadBytes();
	CpuInfo		getCpuInfo();

	static CStatic				*minstance;
	std::map<HASH, OneTask *>	mmapHashTask;
	CLock						mlockHashTask;

	std::queue<OneTaskPacket*>	mqueueOneTaskPacket;
	CEevnt						mlockOneTaskPacket;

	CLock						mlockDownload;
	int64						mdownloadTick;
	int64						mdownloadSpeed;
	uint64						mdownloadTT;
	CLock						mlockUpload;
	int64						muploadTick;
	int64						muploadSpeed;
	uint64						muploadTT;

	time_t						mappStartTime;
	time_t						mupdateTime;
	int32						mtotalConn;

	bool			misRun;
	cms_thread_t	mtid;

	CpuInfo mcpuInfo0;
	std::string mappName;
};
#endif