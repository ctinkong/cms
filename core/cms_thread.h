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
#ifndef __CMS_THREAD_H__
#define __CMS_THREAD_H__
#ifdef WIN32 /* WIN32 */
#include <process.h>
#define CMS_THREAD_RETURN void
typedef	unsigned long cms_thread_t;
typedef void(__cdecl *cms_routine_pt)(void*);
#else /* posix */

#include <pthread.h>

#define CMS_THREAD_RETURN void*
typedef pthread_t cms_thread_t;
typedef void *(*cms_routine_pt)(void*);

#endif /* posix end */



/* func */
int cmsCreateThread(cms_thread_t *tid, cms_routine_pt routine, void *arg, bool detached);
int cmsWaitForThread(cms_thread_t tid, void **value_ptr);
int cmsWaitForMultiThreads(int nCount, const cms_thread_t *handles);


#endif /* _QVOD_THREAD_H_ */


