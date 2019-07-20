#ifndef __CMS_MF_MEM_H__
#define __CMS_MF_MEM_H__
#include <stdio.h>
#include <string>

#ifdef _CMS_LEAK_CHECK_
//÷ÿ‘ÿƒ⁄¥Ê∑÷≈‰
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
