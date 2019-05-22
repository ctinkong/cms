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
#ifndef __CMS_DNS_CACHE_H__
#define __CMS_DNS_CACHE_H__
#include <core/cms_lock.h>
#include <string>
#include <map>
using namespace std;

struct HostInfo
{
	unsigned long ip;
	unsigned long tt;
};
#define MapHostInfoIter map<string,HostInfo*>::iterator

class CDnsCache
{
public:
	CDnsCache();
	~CDnsCache();
	static CDnsCache *instance();
	static void freeInstance();
	bool host2ip(const char* host, unsigned long &ip);
private:
	static CDnsCache *minstance;
	map<string, HostInfo*> mmapHostInfo;
	CLock mHostInfoLock;
};
#endif
