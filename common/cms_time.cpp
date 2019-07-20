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

uint32      gcmsCount = 0;
uint32      gcmsB = 0;
uint32      gcmsE = 0;

bool		gisCmsTimeSet = false;

bool  gcmsTimeRun = false;
bool  gcmsIsTimerAlarm = false;
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

unsigned long getTC() {
	int res;
	struct timespec sNow;
	res = clock_gettime(CLOCK_MONOTONIC, &sNow);
	if (res != 0)
	{
		return -1;
	}
	return sNow.tv_sec * 1000 + sNow.tv_nsec / 1000000; /* milliseconds */
}

void cmsUpdateTime()
{
	//经过测试 100 ms 会有几毫米的误差，建议不要使用
	if (gcmsB == 0)
	{
		gcmsB = getTC();
	}
	gcmsCount++;
	if (gcmsCount % 100 == 0)
	{
		gcmsE = getTC();
// 		printf("##### cmsUpdateTime 100 count take time %lu ms\n", gcmsE - gcmsB);
		gcmsB = gcmsE;
	}


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
}

void timefunc(int signo)
{
	switch (signo) {
	case SIGALRM:
		if (gcmsIsTimerAlarm)
		{
			cmsUpdateTime();
		}
		break;
	}
}

bool startTimer(long usecond)
{
	signal(SIGALRM, timefunc);
	struct itimerval value, ovalue;
	value.it_value.tv_sec = 0;
	value.it_value.tv_usec = usecond; // ms
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_usec = usecond; // ms
	//ITIMER_PROF: 以该进程在用户态下和内核态下所费的时间来计算，它送出SIGPROF信号
	return setitimer(ITIMER_REAL, &value, &ovalue) == 0 ? true : false;
}

void cmsTimeRun()
{
	printf("cmsTimeRun enter.\n");
	cmsRegister();	
	if (startTimer(999))
	{
		gcmsIsTimerAlarm = true;
	}
	else
	{
		gcmsTimeRun = true;
		cmsCreateThread(&gcmsTimeTid, cmsTimeThread, NULL, false);
	}
	do
	{
		cmsSleep(10);
	} while (!gisCmsTimeSet);
	printf("cmsTimeRun leave.\n");
}

void cmsTimeStop()
{
	if (gcmsIsTimerAlarm)
	{
		startTimer(0);
		gcmsIsTimerAlarm = false;
	}
	else
	{
		gcmsTimeRun = false;
		cmsWaitForThread(gcmsTimeTid, NULL);
		gcmsTimeTid = 0;
	}
}