#ifndef __CMS_RFC_1123_H__
#define __CMS_RFC_1123_H__
#include <time.h>

void   cmsRFC1123 (char *buf, time_t t);
time_t cmsParseRFC1123 (const char *str, int len);
#endif
