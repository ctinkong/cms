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
#ifndef __CMS_COMMON_URL_H__
#define __CMS_COMMON_URL_H__
#include <string>
#include <mem/cms_mf_mem.h>

#define PROTOCOL_HTTP	"http"
#define PROTOCOL_HTTPS	"https"
#define PROTOCOL_RTMP	"rtmp"
#define PROTOCOL_WS		"ws"
#define PROTOCOL_WSS	"wss"

typedef struct _LinkUrl
{
	std::string protocol;
	std::string host;
	std::string addr;
	unsigned short port;
	bool isDefault;
	std::string app;
	std::string instanceName;
	std::string uri;

	OperatorNewDelete
}LinkUrl;

bool parseUrl(std::string url, LinkUrl &linkUrl);
void resetLinkUrl(LinkUrl *linkUrl);
#endif
