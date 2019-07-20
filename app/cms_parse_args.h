#ifndef __CMS_PARSE_ARGS_H__
#define __CMS_PARSE_ARGS_H__
#include <string>
#include <core/cms_lock.h>

extern bool g_isDebug;
extern std::string g_testUrl;
extern int  g_testTaskNum;
extern std::string g_pidPath;
extern std::string g_configPath;
extern std::string g_appName;
extern bool g_isTestServer;
extern CEevnt g_appEvent;

int parseOptions(int argc, char ** argv);
#endif
