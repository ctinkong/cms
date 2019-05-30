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
#include <static/cms_static.h>
#include <common/cms_utility.h>
#include <app/cms_app_info.h>
#include <log/cms_log.h>
#include <errno.h>

#define MapHashOneTaskIterator std::map<HASH,OneTask *>::iterator

CStatic	*CStatic::minstance = NULL;
CStatic::CStatic()
{
	misRun = false;
	mtid = 0;
	mdownloadTick = 0;
	mdownloadSpeed = 0;
	mdownloadTT = getTickCount();
	muploadTick = 0;
	muploadSpeed = 0;
	muploadTT = getTickCount();
	mupdateTime = mappStartTime = getTimeUnix();
	mtotalConn = 0;
	mcpuInfo0 = getCpuInfo();
}

CStatic::~CStatic()
{

}

CStatic *CStatic::instance()
{
	if (minstance == NULL)
	{
		minstance = new CStatic();
	}
	return minstance;
}

void CStatic::freeInstance()
{
	if (minstance)
	{
		delete minstance;
		minstance = NULL;
	}
}

bool CStatic::run()
{
	misRun = true;
	int res = cmsCreateThread(&mtid, routinue, this, false);
	if (res == -1)
	{
		char date[128] = { 0 };
		getTimeStr(date);
		logs->error("%s ***** file=%s,line=%d cmsCreateThread error *****", date, __FILE__, __LINE__);
		return false;
	}
	return true;
}

void CStatic::stop()
{
	logs->debug("##### CStatic::stop begin #####");
	misRun = false;
	cmsWaitForThread(mtid, NULL);
	mtid = 0;
	logs->debug("##### CStatic::stop finish #####");
}

void CStatic::thread()
{
	logs->info(">>>>> CStatic thread pid=%d", gettid());
	setThreadName("cms-static");
	OneTaskDownload *otd;
	OneTaskUpload *otu;
	OneTaskMeida *otma;
	OneTaskMem *otmm;
	OneTaskPacket *otp;
	bool isTrue;
	bool isHandleOne;
	for (; misRun;)
	{
		isHandleOne = false;
		//OneTaskDownload
		isTrue = pop(&otp);
		if (isTrue)
		{
			mupdateTime = getTimeUnix();
			switch (otp->packetID)
			{
			case PACKET_ONE_TASK_DOWNLOAD:
				otd = (OneTaskDownload *)otp;
				handle(otd);
				delete otd;
				break;
			case PACKET_ONE_TASK_UPLOAD:
				otu = (OneTaskUpload *)otp;
				handle(otu);
				delete otu;
				break;
			case PACKET_ONE_TASK_MEDIA:
				otma = (OneTaskMeida *)otp;
				handle(otma);
				delete otma;
				break;
			case PACKET_ONE_TASK_MEM:
				otmm = (OneTaskMem *)otp;
				handle(otmm);
				delete otmm;
				break;
			default:
				logs->error("*** [CStatic::thread] unexpect packet %d ***", otp->packetID);
				break;
			}
			isHandleOne = true;
		}
		if (!isHandleOne)
		{
			cmsSleep(10);
		}
	}
	logs->info(">>>>> CStatic thread leave pid=%d", gettid());
}

void *CStatic::routinue(void *param)
{
	CStatic *pIns = (CStatic*)param;
	pIns->thread();
	return NULL;
}

void CStatic::setAppName(std::string appName)
{
	mappName = appName;
}

void CStatic::push(OneTaskPacket *otp)
{
	mlockOneTaskPacket.Lock();
	mqueueOneTaskPacket.push(otp);
	mlockOneTaskPacket.Unlock();
}

bool CStatic::pop(OneTaskPacket **otp)
{
	bool isTrue = false;
	mlockOneTaskPacket.Lock();
	if (!mqueueOneTaskPacket.empty())
	{
		*otp = mqueueOneTaskPacket.front();
		mqueueOneTaskPacket.pop();
		isTrue = true;
	}
	mlockOneTaskPacket.Unlock();
	return isTrue;
}

void CStatic::handle(OneTaskDownload *otd)
{
	uint32 tt = (uint64)getTickCount();
	mlockHashTask.Lock();
	MapHashOneTaskIterator it = mmapHashTask.find(otd->hash);
	if (it != mmapHashTask.end())
	{
		if (otd->isRemove)
		{
			delete it->second;
			mmapHashTask.erase(it);
		}
		else
		{
			it->second->mdownloadTick += otd->downloadBytes;
			it->second->mdownloadTotal += otd->downloadBytes;

			if (tt - it->second->mdownloadTT >= 1000 * 5)
			{
				it->second->mdownloadSpeed = it->second->mdownloadTick * 1000 / (tt - it->second->mdownloadTT);
				it->second->mdownloadTT = tt;
				it->second->mdownloadTick = 0;
			}
		}
	}
	else
	{
		if (!otd->isRemove && otd->isFromeTask)
		{
			OneTask *otk = newOneTask();
			otk->mdownloadTick += otd->downloadBytes;
			otk->mdownloadTotal += otd->downloadBytes;
			otk->mdownloadTT = (uint64)getTickCount();
			mmapHashTask[otd->hash] = otk;
		}
	}
	mlockHashTask.Unlock();

	if (!otd->isRemove)
	{
		mlockDownload.Lock();
		mdownloadTick += otd->downloadBytes;
		if (tt - mdownloadTT >= 1000 * 5)
		{
			mdownloadSpeed = mdownloadTick * 1000 / (tt - mdownloadTT);
			mdownloadTT = tt;
			mdownloadTick = 0;
		}
		mlockDownload.Unlock();
	}
}

void CStatic::handle(OneTaskUpload *otu)
{
	uint32 tt = (uint64)getTickCount();
	mlockHashTask.Lock();
	MapHashOneTaskIterator it = mmapHashTask.find(otu->hash);
	if (it != mmapHashTask.end())
	{
		if (otu->connAct == PACKET_CONN_ADD)
		{
			it->second->mtotalConn++;
			mtotalConn++;
		}
		else if (otu->connAct == PACKET_CONN_DEL)
		{
			it->second->mtotalConn--;
			mtotalConn--;
			if (it->second->mtotalConn < 0)
			{
				it->second->mtotalConn = 0;
				mtotalConn = 0;
			}
		}
		else if (otu->connAct == PACKET_CONN_DATA)
		{
			it->second->muploadTick += otu->uploadBytes;
			it->second->muploadTotal += otu->uploadBytes;

			if (tt - it->second->muploadTT >= 1000 * 5)
			{
				it->second->muploadSpeed = it->second->muploadTick * 1000 / (tt - it->second->muploadTT);
				it->second->muploadTT = tt;
				it->second->muploadTick = 0;
			}
		}
		else
		{
			logs->error("*** [CStatic::handle] handle task %s upload packet unknow packet id %d ***",
				it->second->murl.c_str(), otu->connAct);
		}
	}
	mlockHashTask.Unlock();

	if (otu->connAct == PACKET_CONN_DATA)
	{
		mlockUpload.Lock();
		muploadTick += otu->uploadBytes;
		if (tt - muploadTT >= 1000 * 5)
		{
			muploadSpeed = muploadTick * 1000 / (tt - muploadTT);
			muploadTT = tt;
			muploadTick = 0;
		}
		mlockUpload.Unlock();
	}
}

void CStatic::handle(OneTaskMeida *otm)
{
	mlockHashTask.Lock();
	MapHashOneTaskIterator it = mmapHashTask.find(otm->hash);
	if (it != mmapHashTask.end())
	{
		if (otm->videoFramerate > 0)
		{
			it->second->mvideoFramerate = otm->videoFramerate;
		}
		if (otm->audioFramerate > 0)
		{
			it->second->maudioFramerate = otm->audioFramerate;
		}
		if (otm->audioSamplerate > 0)
		{
			it->second->maudioSamplerate = otm->audioSamplerate;
		}
		if (otm->mediaRate > 0)
		{
			it->second->mmediaRate = otm->mediaRate;
		}
		if (otm->width > 0)
		{
			it->second->miWidth = otm->width;
		}
		if (otm->height > 0)
		{
			it->second->miHeight = otm->height;
		}
		if (!otm->videoType.empty())
		{
			it->second->mvideoType = otm->videoType;
		}
		if (!otm->audioType.empty())
		{
			it->second->maudioType = otm->audioType;
		}
		if (!otm->remoteAddr.empty())
		{
			it->second->mremoteAddr = otm->remoteAddr;
		}
		it->second->murl = otm->url;
		it->second->misUDP = otm->isUdp;
	}
	mlockHashTask.Unlock();
}

void CStatic::handle(OneTaskMem *otm)
{
	mlockHashTask.Lock();
	MapHashOneTaskIterator it = mmapHashTask.find(otm->hash);
	if (it != mmapHashTask.end())
	{
		it->second->mtotalMem = otm->totalMem;
	}
	mlockHashTask.Unlock();
}

std::string CStatic::dump()
{
	cJSON *root = cJSON_CreateObject();

	cJSON *taskArray = NULL;
	std::string appVersion;
	appVersion.append(APP_NAME);
	appVersion.append("/");
	appVersion.append(APP_VERSION);
	std::string strBuildTime = __DATE__;
	strBuildTime.append(" ");
	strBuildTime.append(__TIME__);

	cJSON_AddItemToObject(root, "version", cJSON_CreateString(appVersion.c_str()));
	cJSON_AddItemToObject(root, "build_time", cJSON_CreateString(strBuildTime.c_str()));
	cJSON_AddItemToObject(root, "task_num", cJSON_CreateNumber(getTaskInfo(&taskArray)));

	cJSON_AddItemToObject(root, "task_list", taskArray);
	cJSON_AddItemToObject(root, "conn_num", cJSON_CreateNumber(mtotalConn));
	cJSON_AddItemToObject(root, "upload_speed", cJSON_CreateNumber(muploadSpeed));
	cJSON_AddItemToObject(root, "download_speed", cJSON_CreateNumber(mdownloadSpeed));
	cJSON_AddItemToObject(root, "upload_speed_s", cJSON_CreateString(parseSpeed8Mem(muploadSpeed, true).c_str()));
	cJSON_AddItemToObject(root, "download_speed_s", cJSON_CreateString(parseSpeed8Mem(mdownloadSpeed, true).c_str()));

	int cpu = getCpuUsage();
	cJSON_AddItemToObject(root, "cpu", cJSON_CreateNumber(cpu));
	float mem = getMemUsage();
	cJSON_AddItemToObject(root, "mem", cJSON_CreateNumber(mem));
	int   memsize = getMemSize();
	cJSON_AddItemToObject(root, "mem_size", cJSON_CreateNumber(memsize));

	struct tm st;
	localtime_r(&mappStartTime, &st);
	char szStartTime[128] = { 0 };
	snprintf(szStartTime, sizeof(szStartTime), "%04d-%02d-%02d %02d:%02d:%02d",
		st.tm_year + 1900, st.tm_mon + 1, st.tm_mday, st.tm_hour, st.tm_min, st.tm_sec);
	cJSON_AddItemToObject(root, "start_time", cJSON_CreateString(szStartTime));

	localtime_r(&mupdateTime, &st);
	char szUpdateTime[128] = { 0 };
	snprintf(szUpdateTime, sizeof(szUpdateTime), "%04d-%02d-%02d %02d:%02d:%02d",
		st.tm_year + 1900, st.tm_mon + 1, st.tm_mday, st.tm_hour, st.tm_min, st.tm_sec);
	cJSON_AddItemToObject(root, "update_data_time", cJSON_CreateString(szUpdateTime));

	std::string strJson = cJSON_PrintUnformatted(root);

	cJSON_Delete(root);

	return strJson;
}

int CStatic::getTaskInfo(cJSON **value)
{
	*value = cJSON_CreateArray();
	int size = 0;
	mlockHashTask.Lock();
	size = mmapHashTask.size();
	MapHashOneTaskIterator it = mmapHashTask.begin();
	for (; it != mmapHashTask.end(); ++it)
	{
		OneTask *otk = it->second;
		cJSON *v = cJSON_CreateObject();
		//HASH hash = it->first;
		struct tm st;
		localtime_r(&otk->mttCreate, &st);
		char szTime[128] = { 0 };
		snprintf(szTime, sizeof(szTime), "%04d-%02d-%02d %02d:%02d:%02d",
			st.tm_year + 1900, st.tm_mon + 1, st.tm_mday, st.tm_hour, st.tm_min, st.tm_sec);
		cJSON_AddItemToObject(v, "time", cJSON_CreateString(szTime));

		cJSON_AddItemToObject(v, "url", cJSON_CreateString(otk->murl.c_str()));
		cJSON_AddItemToObject(v, "addr", cJSON_CreateString(otk->mremoteAddr.c_str()));
		cJSON_AddItemToObject(v, "conn_num", cJSON_CreateNumber(otk->mtotalConn));

		cJSON_AddItemToObject(v, "media_rate", cJSON_CreateNumber(otk->mmediaRate));
		cJSON_AddItemToObject(v, "video_frame_rate", cJSON_CreateNumber(otk->mvideoFramerate));
		cJSON_AddItemToObject(v, "video_type", cJSON_CreateString(otk->mvideoType.c_str()));
		cJSON_AddItemToObject(v, "video_width", cJSON_CreateNumber(otk->miWidth));
		cJSON_AddItemToObject(v, "video_height", cJSON_CreateNumber(otk->miHeight));

		cJSON_AddItemToObject(v, "audio_frame_rate", cJSON_CreateNumber(otk->maudioFramerate));
		cJSON_AddItemToObject(v, "audio_sample_rate", cJSON_CreateNumber(otk->maudioSamplerate));
		cJSON_AddItemToObject(v, "audio_type", cJSON_CreateString(otk->maudioType.c_str()));

		cJSON_AddItemToObject(v, "udp", cJSON_CreateBool(otk->misUDP ? 1 : 0));

		cJSON_AddItemToObject(v, "download_speed", cJSON_CreateNumber(otk->mdownloadSpeed));
		cJSON_AddItemToObject(v, "download_speed_s", cJSON_CreateString(parseSpeed8Mem(otk->mdownloadSpeed, true).c_str()));
		cJSON_AddItemToObject(v, "upload_speed", cJSON_CreateNumber(otk->muploadSpeed));
		cJSON_AddItemToObject(v, "upload_speed_s", cJSON_CreateString(parseSpeed8Mem(otk->muploadSpeed, true).c_str()));

		cJSON_AddItemToObject(v, "total_mem", cJSON_CreateNumber(otk->mtotalMem));
		cJSON_AddItemToObject(v, "total_mem_s", cJSON_CreateString(parseSpeed8Mem(otk->mtotalMem, false).c_str()));

		cJSON_AddItemToArray(*value, v);
	}
	mlockHashTask.Unlock();
	return size;
}

int CStatic::getCpuUsage()
{
	CpuInfo cupInfo = getCpuInfo();
	int busytime = cupInfo.user + cupInfo.nice + cupInfo.sys
		- mcpuInfo0.user - mcpuInfo0.nice - mcpuInfo0.sys;
	int sumtime = busytime + cupInfo.idle - mcpuInfo0.idle;
	mcpuInfo0 = cupInfo;
	if (0 != sumtime)
	{
		return (100 * busytime / sumtime);
	}
	return 0;
}


CpuInfo CStatic::getCpuInfo()
{
	FILE* file = NULL;
	file = fopen("/proc/stat", "r");
	CpuInfo cpu;
	cpu.user = cpu.sys = cpu.nice = cpu.idle = 0;
	if (NULL == file)
	{	//open error
		logs->error("getCpuInfo fopen failed with error %d!", errno);
		return mcpuInfo0;
	}
	char strtemp[10];
	fscanf(file, "%s%lld%lld%lld%lld", strtemp, &cpu.user, &cpu.nice, &cpu.sys, &cpu.idle);
	fclose(file);
	return cpu;
}

float CStatic::getMemUsage()
{
	std::string strcmd = "ps aux | grep ";
	strcmd += mappName;
	strcmd += "> /tmp/memtemp.txt";
	if (system(strcmd.c_str()) == -1)
	{
		logs->error("*** GetMemUsage system failed! ***");
		return 0;
	}
	FILE* file = fopen("/tmp/memtemp.txt", "r");
	if (NULL == file)
	{
		logs->error("*** GetMemUsage fopen failed with error %d! ***", errno);
		return 0;
	}
	char username[100];
	int  pid;
	float cpurate, memrate;
	fscanf(file, "%s%d%f%f", username, &pid, &cpurate, &memrate);
	fclose(file);
	return memrate;
}

int CStatic::getMemSize()
{
	std::string strcmd = "ps -e -o 'comm,rsz' | grep ";
	strcmd += mappName;
	strcmd += "> /tmp/memsizetemp.txt";
	if (system(strcmd.c_str()) == -1)
	{
		logs->error("*** getMemSize system failed! ***");
		return 0;
	}
	FILE* file = fopen("/tmp/memsizetemp.txt", "r");
	if (NULL == file)
	{
		logs->error("*** getMemSize fopen failed with error %d! ***", errno);
		return 0;
	}
	char appName[100];
	int  memsize = 0;
	fscanf(file, "%s%d", appName, &memsize);
	fclose(file);
	return memsize;
}