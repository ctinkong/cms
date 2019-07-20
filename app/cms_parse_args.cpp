#include <app/cms_parse_args.h>
#include <app/cms_app_info.h>
#include <mem/cms_mf_mem.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

bool g_isDebug = false;
std::string g_testUrl;
int  g_testTaskNum = 0;
std::string g_pidPath = "/var/lock/cms.pid";
std::string g_configPath = "config.json";
std::string g_appName;
bool g_isTestServer = false;

CEevnt g_appEvent;


#define FirstIndent "  "
//参数
#define OPTION_HELP			'h'
#define OPTION_VERSION		'v'
#define OPTION_DEBUG		'd'
#define OPTION_PID_FILE		'i'
#define OPTION_CONFIG_FILE	'c'
#define OPTION_TEST_SERVER	't'
#define OPTION_TEST_URL		'u'
#define OPTION_TASK_NUM		'n'
//参数索引
enum OptionsArgs
{
	OptionsHelp = 0x00,
	OptionsVersion,
	OptionsDebug,
	OptionsPidPath,
	OptionsConfigFile,
	OptionsTestServer,
	OptionsTestUrl,
	OptionsTaskNum,
	OptionsEnd,
};
//参数标记
char OptionsFlags[OptionsEnd] = {
	OPTION_HELP,
	OPTION_VERSION,
	OPTION_DEBUG,
	OPTION_PID_FILE,
	OPTION_CONFIG_FILE,
	OPTION_TEST_SERVER,
	OPTION_TEST_URL,
	OPTION_TASK_NUM,
};
//为NULL表示不需要参数 否则需要参数
char *OptionsShowSample[OptionsEnd] = {
	NULL,
	NULL,
	NULL,
	(char *)"cms.pid",
	(char *)"cms.json",
	NULL,
	(char *)"rtmp://test.cms.com/live/test",
	(char *)"100",
};
//参数描述
char *OptionsDesc[OptionsEnd] = {
	(char *)"print help",
	(char *)"print version",
	(char *)"debug mode",
	(char *)"set pid file [cms.pid]",
	(char *)"set configure file [config.json]",
	(char *)"as test server",
	(char *)"test url(t should be set)",
	(char *)"test task num(t should be set)"
};

std::string getNoParams()
{
	std::string value;
	for (int i = 0; i < OptionsEnd; i++)
	{
		if (OptionsShowSample[i] == NULL)
		{
			if (value.empty())
			{
				value = "[-";
			}
			value += OptionsFlags[i];
		}
	}
	if (!value.empty())
	{
		value += "]";
	}
	return value;
}

std::string getNeedParams()
{
	std::string value;
	for (int i = 0; i < OptionsEnd; i++)
	{
		if (OptionsShowSample[i] != NULL)
		{
			value += "[-";
			value += OptionsFlags[i];
			value += " ";
			value += OptionsShowSample[i];
			value += "]";
		}
	}
	return value;
}

std::string getDescribe()
{
	std::string value;
	for (int i = 0; i < OptionsEnd; i++)
	{
		value += FirstIndent;
		value += OptionsFlags[i];
		value += ": ";
		value += OptionsDesc[i];
		value += "\n";
	}
	return value;
}

void printHelp()
{
	std::string noParams = getNoParams();
	std::string needParams = getNeedParams();
	std::string flagsDesc = getDescribe();
	printf("  Usage: %s %s%s\n",
		g_appName.c_str(),
		noParams.c_str(),
		needParams.c_str());
	printf("%s", flagsDesc.c_str());
}

std::string getOptstring()
{
	std::string value;
	//需要参数
	for (int i = 0; i < OptionsEnd; i++)
	{
		if (OptionsShowSample[i] != NULL)
		{
			if (!value.empty())
			{
				value += ":";
			}
			value += OptionsFlags[i];
		}
	}
	//不需要参数
	if (!value.empty())
	{
		value += ":";
	}
	for (int i = 0; i < OptionsEnd; i++)
	{
		if (OptionsShowSample[i] == NULL)
		{
			value += OptionsFlags[i];
		}
	}
	return value;
}


int parseOptions(int argc, char ** argv)
{
	/* parse and process options */
	int ch;
	char *p;
	std::string optstring = getOptstring();//"i:c:t:u:n:hvd"
	while ((ch = getopt(argc, argv, optstring.c_str())) != -1) {
		switch (ch) {
		case OPTION_HELP:
			printHelp();
			exit(0);
		case OPTION_VERSION:
			printf("%s version %s, builded on %s\n", argv[0], APP_VERSION, __DATE__);
			exit(0);
		case OPTION_DEBUG:
			g_isDebug = true;
			break;
		case OPTION_PID_FILE:
			p = xstrdup(optarg);
			g_pidPath = p;
			xfree(p);
			break;
		case OPTION_CONFIG_FILE:
			p = xstrdup(optarg);
			g_configPath = p;
			xfree(p);
			break;
		case OPTION_TEST_SERVER:
			g_isTestServer = true;
			break;
		case OPTION_TEST_URL:
			p = xstrdup(optarg);
			g_testUrl = p;
			xfree(p);
			break;
		case OPTION_TASK_NUM:
			g_testTaskNum = atoi(optarg);
			break;
		default:
			printHelp();
			exit(0);
		}
	}
	if (g_isTestServer && (g_testUrl.empty() || g_testTaskNum == 0))
	{
		printHelp();
		exit(0);
	}
	return 0;
}

