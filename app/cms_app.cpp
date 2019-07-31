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
#include <config/cms_config.h>
#include <log/cms_log.h>
#include <dnscache/cms_dns_cache.h>
#include <ts/cms_hls_mgr.h>
#include <common/cms_shmmgr.h>
#include <common/cms_url.h>
#include <enc/cms_sha1.h>
#include <flvPool/cms_flv_pool.h>
#include <taskmgr/cms_task_mgr.h>
#include <worker/cms_master.h>
#include <static/cms_static.h>
#include <common/cms_time.h>
#include <app/cms_app_info.h>
#include <app/cms_parse_args.h>
#include <common/cms_utility.h>
#include <map>
#include <string>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/resource.h>
using namespace std;

void inorgSignal()
{
	signal(SIGPIPE, SIG_IGN);;
}

int daemon()
{
	int fd;
	//int pid;

	switch (fork())
	{
	case -1:
		printf("***** %s(%d)-%s: fork() error: %s *****\n",
			__FILE__, __LINE__, __FUNCTION__, strerror(errno));
		return -1;

	case 0:
		break;

	default:
		printf("Daemon exit\n");
		exit(0);
	}

	//pid = getpid();

	if (setsid() == -1) {
		printf("***** %s(%d)-%s: setsid() failed *****\n",
			__FILE__, __LINE__, __FUNCTION__);
		return -1;
	}

	umask(0);

	fd = open("/dev/null", O_RDWR);
	if (fd == -1) {
		printf("***** %s(%d)-%s: open(\"/dev/null\") failed *****\n",
			__FILE__, __LINE__, __FUNCTION__);
		return -1;
	}

	if (dup2(fd, STDIN_FILENO) == -1) {
		printf("***** %s(%d)-%s: dup2(STDIN) failed *****\n",
			__FILE__, __LINE__, __FUNCTION__);
		return -1;
	}

	if (dup2(fd, STDOUT_FILENO) == -1) {
		printf("***** %s(%d)-%s: dup2(STDOUT) failed *****\n",
			__FILE__, __LINE__, __FUNCTION__);
		return -1;
	}

	if (fd > STDERR_FILENO) {
		if (close(fd) == -1) {
			printf("***** %s(%d)-%s: close() failed ******\n",
				__FILE__, __LINE__, __FUNCTION__);
			return -1;
		}
	}
	return 0;
}

void initInstance()
{
	//必须先初始化日志，否则会崩溃
	cmsLogInit(CConfig::instance()->clog()->path(),
		CConfig::instance()->clog()->level(),
		CConfig::instance()->clog()->console(),
		CConfig::instance()->clog()->size());
	CConfig::instance();
	CFlvPool::instance();
	CMaster::instance();
	CDnsCache::instance();
	CShmMgr::instance();
	CTaskMgr::instance();
	CStatic::instance();
	CMissionMgr::instance();

	CStatic::instance()->setAppName(g_appName.c_str());
}

void runInstance()
{
	if (!CStatic::instance()->run())
	{
		logs->error("*** CStatic::instance()->run() fail ***");
		cmsSleep(1000 * 1);
		exit(0);
	}
	if (!CFlvPool::instance()->run())
	{
		logs->error("*** CFlvPool::instance()->run() fail ***");
		cmsSleep(1000 * 1);
		exit(0);
	}
	if (!CMissionMgr::instance()->run())
	{
		logs->error("*** CMissionMgr::instance()->run() fail ***");
		cmsSleep(1000 * 1);
		exit(0);
	}
	if (!CMaster::instance()->run())
	{
		logs->error("*** CMaster::instance()->run() fail ***");
		cmsSleep(1000 * 1);
		exit(0);
	}
	if (!CTaskMgr::instance()->run())
	{
		logs->error("*** CTaskMgr::instance()->run() fail ***");
		cmsSleep(1000 * 1);
		exit(0);
	}
}

void cycleServer()
{
#ifdef just_test__CMS_APP_DEBUG_SP__
	cmsSleep(100000);
#else
// 	do
// 	{
		g_appEvent.Lock();
		g_appEvent.Wait();
		g_appEvent.Unlock();
// 	} while (1);
#endif
	CStatic::instance()->stop();
	CFlvPool::instance()->stop();
	CMissionMgr::instance()->stop();
	CMaster::instance()->stop();
	CTaskMgr::instance()->stop();
#ifdef _CMS_APP_USE_TIME_
	cmsTimeStop();
#endif
	//CServer::instance()->stop();
	cmsSleep(1000);
	cmsLogStop();
}

void setRlimit()
{
	struct rlimit rlim;
	rlim.rlim_cur = 300000;
	rlim.rlim_max = 300000;
	setrlimit(RLIMIT_NOFILE, &rlim);

	rlim.rlim_cur = 0;
	rlim.rlim_max = 0;
	getrlimit(RLIMIT_NOFILE, &rlim);
	logs->debug("+++ open file cur %d,open file max %d +++", rlim.rlim_cur, rlim.rlim_max);

	struct rlimit rlcore;
	rlcore.rlim_cur = RLIM_INFINITY;
	rlcore.rlim_max = RLIM_INFINITY;
	if (setrlimit(RLIMIT_CORE, &rlcore) != 0)
	{
		logs->warn("!setrlimit of core to UNLIMITED failed %d %s\n", errno, strerror(errno));
	}
}

void createTestTask()
{
	for (int i = 0; i < g_testTaskNum; i++)
	{
		char szNum[20] = { 0 };
		snprintf(szNum, sizeof(szNum), "_%d", i);
		string hashUrl = g_testUrl + szNum;
		printf(">>>>createTestTask hash url %s\n", hashUrl.c_str());
		CSHA1 sha;
		sha.write(hashUrl.c_str(), hashUrl.length());
		string strHash = sha.read();
		HASH hash = HASH((char *)strHash.c_str());
		CTaskMgr::instance()->createTask(hash, g_testUrl, "", g_testUrl, "", CREATE_ACT_PULL, false, false);
	}
}

void getOldPid()
{
	// 	FILE *fppid = fopen(g_pidPath.c_str(), "r");
	// 	if (!fppid)
	// 	{
	// 		cycle.smooth_restart = 0;
	// 		return;
	// 	}
	// 	char strpid[10];
	// 	if (fgets(strpid, 10, fppid) == NULL)
	// 	{
	// 		fclose(fppid);
	// 		cycle.smooth_restart = 0;
	// 		return;
	// 	}
	// 	fclose(fppid);
	// 	int pid = atoi(strpid);
	// 
	// 	char filename_in_proc[128];
	// 	snprintf(filename_in_proc, 128, "/proc/%d/status", pid);
	// 	FILE *procfp = fopen(filename_in_proc, "r");
	// 	if (!procfp)
	// 	{
	// 		cycle.smooth_restart = 0;
	// 		return;
	// 	}
	// 	char procname[1024];
	// 	if (fgets(procname, 1024, procfp) == NULL)
	// 	{
	// 		fclose(procfp);
	// 		cycle.smooth_restart = 0;
	// 		return;
	// 	}
	// 	fclose(procfp);
	// 	if (!strstr(procname, g_appName.c_str()))
	// 	{
	// 		cycle.smooth_restart = 0;
	// 		return;
	// 	}
	// 	cycle.old_pid = pid;
}

int checkRunningPid()
{
	FILE *fppid = fopen(g_pidPath.c_str(), "r");
	if (!fppid)
	{
		return 0;
	}
	char strpid[10];
	if (fgets(strpid, 10, fppid) == NULL)
	{
		fclose(fppid);
		return 0;
	}
	fclose(fppid);
	int pid = atoi(strpid);

	char filename_in_proc[128];
	snprintf(filename_in_proc, 128, "/proc/%d/status", pid);
	FILE *procfp = fopen(filename_in_proc, "r");
	if (!procfp)
	{
		return 0;
	}
	char procname[1024];
	if (fgets(procname, 1024, procfp) == NULL)
	{
		fclose(procfp);
		return 0;
	}
	fclose(procfp);
	if (!strstr(procname, g_appName.c_str()))
	{
		return 0;
	}

	return 1;
}

static void writePid()
{
	if (g_pidPath.empty())
	{
		return;
	}
	FILE *fppid = fopen(g_pidPath.c_str(), "w");
	if (!fppid)
	{
		printf("Can not write pid file %s %s\n", g_pidPath.c_str(), strerror(errno));
		exit(2);
	}
	fprintf(fppid, "%d", (int)getpid());
	fclose(fppid);
}

void runAsTestServer()
{
	//作为压测服务
	if (g_isTestServer)
	{
		if (g_testUrl.empty())
		{
			printf("***** app as test server.but url is empty *****\n");
			exit(0);
		}
		if (g_testTaskNum <= 0)
		{
			g_testTaskNum = 100;
		}
		g_isTestServer = true;
		printf("##### app as test server.url %s,make task num %d #####\n", g_testUrl.c_str(), g_testTaskNum);
		LinkUrl linkUrl;
		if (!parseUrl(g_testUrl, linkUrl))
		{
			printf("***** app as test server.but url %s parse error *****\n", g_testUrl.c_str());
			exit(0);
		}
		createTestTask();
	}
}

void initConfig()
{
	if (!CConfig::instance()->init(g_configPath.c_str()))
	{
		printf("***** initConfig fail *****\n");
		exit(0);
	}
}

void getAppName(char *binPath)
{
	char *pPos = strrchr(binPath, '/');
	if (pPos)
	{
		++pPos;
		g_appName = pPos;
	}
	else
	{
		g_appName = binPath;
	}
}

int main(int argc, char *argv[])
{
	//必须先获取appname 否则help显示异常
	getAppName(argv[0]);
	parseOptions(argc, argv);
	if (checkRunningPid())
	{
		printf("%s is already running!\n", g_appName.c_str());
		return 0;
	}
	initConfig();
	if (!g_isDebug)
	{
		daemon();
	}

#ifdef _CMS_APP_USE_TIME_
	cmsTimeRun();
#endif

	writePid();
	inorgSignal();
	initInstance();
	setRlimit();
	runInstance();
	runAsTestServer();
	cycleServer();
	logs->debug("cms app exit.");
	}

