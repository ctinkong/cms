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

#ifndef __CMS_MF_MEM_H__
#define __CMS_MF_MEM_H__
#include <stdio.h>
#include <string>

#ifdef _CMS_LEAK_CHECK_
//重载内存分配
#define OperatorNewDelete \
void *operator new(size_t size)\
{\
	return xmalloc(size);\
}\
void *operator new[](std::size_t size)\
{\
	return xmalloc(size);\
}\
void operator delete(void *ptr)\
{\
	if (ptr)\
	{\
		xfree(ptr);\
	}\
}\
void operator delete[](void *ptr)\
{\
	if (ptr)\
	{\
		xfree(ptr);\
	}\
}\

#else
#define OperatorNewDelete
#endif

void xassert (const char *msg, const char *file, int line);
#ifdef _CMS_LEAK_CHECK_
void *cms_xcalloc (size_t, size_t, const char *, int);
void *cms_xmalloc (size_t, const char *, int);
void *cms_xrealloc (void *, size_t, const char *, int);
char *cms_xstrdup (const char *, const char *, int);
#define xmalloc(size) (cms_xmalloc((size), (__FILE__), (__LINE__)))
#define xcalloc(n, size) (cms_xcalloc((n), (size), (__FILE__), (__LINE__)))
#define xrealloc(ptr, size) (cms_xrealloc((ptr), (size), (__FILE__), (__LINE__)))
#define xstrdup(c) cms_xstrdup((c),(__FILE__),(__LINE__))
std::string printfMemUsage();
#else
void *xcalloc (size_t, size_t);
void *xmalloc (size_t);
void *xrealloc (void *, size_t);
char *xstrdup (const char *);
#endif
void xxfree (const void *s_const);
void xfree (void *s);
#endif
