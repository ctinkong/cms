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
#include <common/cms_url.h>
#include <log/cms_log.h>
#include <algorithm>
using namespace std;

bool parseUrl(std::string url, LinkUrl &linkUrl)
{
	if (url.empty())
	{
		return false;
	}
	string oriUrl = url;
	size_t begin = url.find("://");
	if (begin == string::npos) {
		logs->error("***** [parseUrl] %s parse fail can't find [ :// ] *****", url.c_str());
		return false;
	}
	else if (begin == 0) {
		logs->error("***** [parseUrl] %s parse fail can't find Protocol string *****", url.c_str());
		return false;
	}
	linkUrl.protocol = url.substr(0, begin);
	transform(linkUrl.protocol.begin(), linkUrl.protocol.end(), linkUrl.protocol.begin(), ::tolower);
	begin += 3;
	size_t end = url.find("/", begin);
	if (end == string::npos)
	{
		logs->error("***** [parseUrl] %s parse fail can't find App string *****", url.c_str());
		return false;
	}
	linkUrl.uri = url.substr(end);
	string addr = url.substr(begin, end - begin);
	begin = end + 1;
	end = addr.find(":");
	if (end == string::npos)
	{
		linkUrl.host = addr;
		linkUrl.isDefault = true;
		if (linkUrl.protocol == PROTOCOL_HTTP)
		{
			linkUrl.port = 80;
			linkUrl.addr = addr + ":80";
		}
		else if (linkUrl.protocol == PROTOCOL_HTTPS)
		{
			linkUrl.port = 443;
			linkUrl.addr = addr + ":443";
		}
		else if (linkUrl.protocol == PROTOCOL_RTMP)
		{
			linkUrl.port = 1935;
			linkUrl.addr = addr + ":1935";
		}
		else if (linkUrl.protocol == PROTOCOL_WS)
		{
			linkUrl.port = 80;
			linkUrl.addr = addr + ":80";
		}

		else if (linkUrl.protocol == PROTOCOL_WSS)
		{
			linkUrl.port = 443;
			linkUrl.addr = addr + ":443";
		}
		else
		{
			logs->error("***** [parseUrl] %s parse fail protocol error *****", url.c_str());
			return false;
		}
	}
	else
	{
		linkUrl.host = addr.substr(0, end);
		string port = addr.substr(end + 1);
		linkUrl.port = atoi(port.c_str());
		linkUrl.addr = addr;
	}
	end = url.rfind("/");
	if (end == string::npos || end < begin)
	{
		logs->error("***** [parseUrl] %s parse fail can't find instance name string *****", url.c_str());
		//return false;
		linkUrl.app = url.substr(begin);
	}
	else
	{
		linkUrl.app = url.substr(begin, end - begin);
		linkUrl.instanceName = url.substr(end + 1);
	}
	logs->debug(">>> [parseUrl] %s parse host=%s,port=%d,app=%s,instance=%s,uri=%s ", url.c_str(),
		linkUrl.host.c_str(), linkUrl.port, linkUrl.app.c_str(), linkUrl.instanceName.c_str(), linkUrl.uri.c_str());
	return true;
}

void resetLinkUrl(LinkUrl *linkUrl)
{
	linkUrl->protocol.clear();
	linkUrl->host.clear();
	linkUrl->addr.clear();
	linkUrl->port = 0;
	linkUrl->isDefault = false;
	linkUrl->app.clear();
	linkUrl->instanceName.clear();
	linkUrl->uri.clear();
}
