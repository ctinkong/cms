#ifndef __CMS_RFC1738_H__
#define __CMS_RFC1738_H__
#include <common/cms_type.h>

char * rfc1738_xcalloc_by_url(const char *url);
uint32 rfc1738_do_escape (const char *url, char* escaped_url);
#endif
