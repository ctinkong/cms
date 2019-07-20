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
#ifndef __CMS_LOG_H__
#define __CMS_LOG_H__
#include <core/cms_thread.h>
#include <core/cms_lock.h>
#include <stdio.h>
#include <string>
#include <queue>

#define logs cmsLogInstance()
using namespace std;

enum LogLevel
{
	OFF,
	FATAL,
	ERROR1,
	WARN,
	INFO,
	DEBUG,
	ALL_LEVEL
};

struct LogInfo
{
	int		day;
	char*	log;
	int     len;
};

class CLog
{
private:
	cms_thread_t mtid;
	bool		mconsole;
	bool		misRun;
	FILE*		mfp;
	int			mfiseSize;
	int			mlimitSize;
	int			midx;
	LogLevel	mlevel;
	string		mdir;
	string		mname;
	queue<LogInfo *> mqueueLog;
	CEevnt		mqueueLock;
	void push(LogInfo* logInfo);
	bool pop(LogInfo** logInfo);
public:
	CLog();
	static void *routinue(void *param);
	string getFileName(char *szDTime);
	void thread();
	bool run(string dir, LogLevel level, bool console, int limitSize = 1024 * 1024 * 500);
	void stop();
	void debug(const char* fmt, ...);
	void info(const char* fmt, ...);
	void warn(const char* fmt, ...);
	void error(const char* fmt, ...);
	void fatal(const char* fmt, ...);
};

CLog*	cmsLogInstance();
void	cmsLogInit(string dir, LogLevel level, bool console, int limitSize = 1024 * 1024 * 500);
void	cmsLogStop();

#endif