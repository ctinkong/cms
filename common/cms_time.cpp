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
#include <common/cms_time.h>
#include <common/cms_utility.h>
#include <core/cms_thread.h>
#include <mem/cms_mf_mem.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>


#define cms_time_trylock(lock)	(*lock==0 && __sync_bool_compare_and_swap(lock, 0, 1))
#define cms_time_unlock(lock)	*(lock) = 0
#define CMS_DATE_BUFFER_LEN	64

//时间逻辑参考nginx
#define CMS_TIME_VAL_COUNT 64
typedef struct _CmsTimeVal
{
	int64	msec;
	int64	mmsec;
}CmsTimeVal;

int			gcmsSlot = 0;
CmsTimeVal	gcmsTimeVal[CMS_TIME_VAL_COUNT];
int64		gcmsUnixTime[CMS_TIME_VAL_COUNT] = { 0 };
int64		gcmsMilSecond[CMS_TIME_VAL_COUNT] = { 0 };
int			gcmsDay[CMS_TIME_VAL_COUNT] = { 0 };
char		*gcmsDayTime[CMS_TIME_VAL_COUNT] = { NULL };
char		*gcmsTimeStr[CMS_TIME_VAL_COUNT] = { NULL };
//指针
int64		*gcmsUnixTimeT;
int64		*gcmsMilSecondT;
int			*gcmsDayT;
char		*gcmsDayTimeT = NULL;
char		*gcmsTimeStrT = NULL;

bool		gisCmsTimeSet = false;

bool  gcmsTimeRun = false;
cms_thread_t gcmsTimeTid = 0;

int64 getCmsUnixTime()
{
	return *gcmsUnixTimeT;
}

int64 getCmsMilSecond()
{
	return *gcmsMilSecondT;
}

int getCmsDay()
{
	return *gcmsDayT;
}

char *getCmsDayTime()
{
	return gcmsDayTimeT;
}

char *getCmsTimeStr()
{
	return gcmsTimeStrT;
}

void cmsUpdateTime()
{
	int64 sec = 0;
	int64 msec = 0;
	int64 curMsec = 0;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	sec = tv.tv_sec;
	msec = tv.tv_usec / 1000;
	curMsec = sec * 1000 + msec;

	CmsTimeVal *ctv = &gcmsTimeVal[gcmsSlot];
	//秒值一致则更新毫秒
	if (ctv->msec == sec)
	{
		ctv->mmsec = msec;
		gcmsMilSecond[gcmsSlot] = curMsec;
		return;
	}
	if (gcmsSlot == CMS_TIME_VAL_COUNT - 1)
	{
		gcmsSlot = 0;
	}
	else
	{
		gcmsSlot++;
	}
	ctv = &gcmsTimeVal[gcmsSlot];
	ctv->msec = sec;
	ctv->mmsec = msec;

	gcmsUnixTime[gcmsSlot] = sec;
	gcmsMilSecond[gcmsSlot] = curMsec;
	struct tm st;
	time_t t = sec;
	localtime_r(&t, &st);
	gcmsDay[gcmsSlot] = st.tm_mday;

	gcmsUnixTimeT = &gcmsUnixTime[gcmsSlot];
	gcmsMilSecondT = &gcmsMilSecond[gcmsSlot];
	gcmsDayT = &gcmsDay[gcmsSlot];
	snprintf(gcmsTimeStr[gcmsSlot], CMS_DATE_BUFFER_LEN, " %04d-%02d-%02d %02d:%02d:%02d",
		st.tm_year + 1900, st.tm_mon + 1, st.tm_wday, st.tm_hour,
		st.tm_min, st.tm_sec);
	gcmsTimeStrT = gcmsTimeStr[gcmsSlot];

	snprintf(gcmsDayTime[gcmsSlot], CMS_DATE_BUFFER_LEN, "%04d-%02d-%02d",
		st.tm_year + 1900, st.tm_mon + 1, st.tm_mday);
	gcmsDayTimeT = gcmsDayTime[gcmsSlot];

	gisCmsTimeSet = true;
}

void *cmsTimeThread(void *lp)
{
	printf("cmsTimeThread enter.\n");
	setThreadName("cms-timer");
	do
	{
		cmsUpdateTime();
		cmsSleep(1);
	} while (gcmsTimeRun);
	printf("cmsTimeThread leave.\n");
	return NULL;
}

void cmsRegister()
{
	for (int i = 0; i < CMS_TIME_VAL_COUNT; i++)
	{
		gcmsTimeVal[i].msec = 0;
		gcmsTimeVal[i].mmsec = 0;
		gcmsUnixTime[i] = 0;
		gcmsMilSecond[i] = 0;
		gcmsDay[i] = 0;
		gcmsDayTime[i] = (char *)xmalloc(CMS_DATE_BUFFER_LEN);
		gcmsTimeStr[i] = (char *)xmalloc(CMS_DATE_BUFFER_LEN);
	}
	gcmsTimeRun = true;
	cmsCreateThread(&gcmsTimeTid, cmsTimeThread, NULL, false);
}

void cmsTimeRun()
{
	printf("cmsTimeRun enter.\n");
	cmsRegister();
	do
	{
		cmsSleep(10);
	} while (!gisCmsTimeSet);
	printf("cmsTimeRun leave.\n");
}

void cmsTimeStop()
{
	gcmsTimeRun = false;
	cmsWaitForThread(gcmsTimeTid, NULL);
	gcmsTimeTid = 0;
}