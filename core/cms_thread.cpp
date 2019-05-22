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
#include <core/cms_thread.h>
/* tid and arg may be NULL */
int cmsCreateThread(cms_thread_t *tid, cms_routine_pt routine, void *arg, bool detached)
{
#ifdef WIN32 /* WIN32 */
	cms_thread_t res;
	/* ignore tid */
	res = _beginthread(routine, 0, arg);
	if (res == -1) {
		/* error */
		return -1;
	}
	/* ok */
	if (tid != NULL) {
		*tid = res;
	}
#else /* posix */
	int res;
	cms_thread_t tid_tmp;
	pthread_attr_t tattr;

	res = pthread_attr_init(&tattr);
	if (res != 0) {
		/* error */
		return -1;
	}

	if (detached)
	{
		res = pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
		if (res != 0)
		{
			return -1;
		}
	}

	res = pthread_create(&tid_tmp, &tattr, routine, arg);
	if (res != 0) {
		/* error */
		return -1;
	}

	res = pthread_attr_destroy(&tattr);
	if (res != 0)
	{
		return -1;
	}

	/* ok */
	if (tid != NULL)
	{
		*tid = tid_tmp;
	}

#endif /* posix end */

	return 0;
}

int cmsWaitForThread(cms_thread_t tid, void **value_ptr)
{
	if (tid == 0)
	{
		return 0;
	}
	int res;

#ifdef WIN32

	/*
	 * win32 do not check the return value of the thread,
	 * so you should set value_ptr NULL.
	 */
	res = WaitForSingleObject(tid, INFINITE);
	if (res == WAIT_FAILED) {
		return -1;
	}

#else /* posix */

	res = pthread_join(tid, value_ptr);
	if (res != 0) {
		return -1;
	}

#endif

	return 0;
}


int cmsWaitForMultiThreads(int nCount, const cms_thread_t *handles)
{
	int ret = 0;
	//(void*)&ret;

#ifdef WIN32 /* WIN32 */

	int res;

	res = WaitForMultipleObjects(nCount, handles, true, INFINITE);
	if (res == WAIT_FAILED) {
		return -1;
	}

#else /* posix */

	int res, i;

	for (i = 0; i < nCount; ++i) {

		res = pthread_join(handles[i], NULL);
		if (res != 0) {
			ret = -1;
		}
	}

#endif /* posix end */

	return ret;
}


