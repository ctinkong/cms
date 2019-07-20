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
#include "cms_lock.h"

CCriticalSection::CCriticalSection(cms_thread_lock_t *lock)
{
	m_lock = lock;
	m_blocked = false;
}

CCriticalSection::~CCriticalSection()
{
	if (m_lock != NULL && m_blocked == true) {

#ifdef WIN32
		LeaveCriticalSection(m_lock);
#else /* posix */
		pthread_mutex_unlock(m_lock);
#endif

		m_lock = NULL;
		m_blocked = false;
	}
}

int CCriticalSection::Lock()
{
	int res = -1;

	if (m_lock != NULL) {

#ifdef WIN32 /* WIN32 */

		EnterCriticalSection(m_lock); /* return void */
		res = 0;
		m_blocked = true;

#else /* posix */

		res = pthread_mutex_lock(m_lock);
		switch (res) {
		case 0:
		{
			/* success */
			m_blocked = true;
			res = 0;
		}
		break;

		default:
		{
			/* failed */
			res = -1;
		}
		}
#endif /* posix end */
	}

	return res;
}

int CCriticalSection::UnLock()
{
	int res = -1;

	if (m_lock != NULL) {

#ifdef WIN32 /* WIN32 */

		LeaveCriticalSection(m_lock); /* return void */
		res = 0;
		m_blocked = false;

#else /* posix */

		res = pthread_mutex_unlock(m_lock);
		switch (res) {
		case 0:
		{
			/* success */
			m_blocked = false;
			res = 0;
		}
		break;

		default:
		{
			/* failed */
			res = -1;
		}
		}
#endif /* posix end */
	}

	return res;
}


int CCriticalSection::TryLock()
{
	int res = -1;

	if (m_lock != NULL) {

#ifdef WIN32 /* WIN32 */

		res = TryEnterCriticalSection(m_lock); /* return bool */
		switch (res) {
		case 0:
		{
			/* another thread already owns */
			res = -1;
		}
		break;

		default:
		{
			/* successfully entered */
			res = 0;
			m_blocked = true;
		}
		}

#else /* posix */

		res = pthread_mutex_trylock(m_lock);
		switch (res) {
		case 0:
		{
			/* successfully entered */
			res = 0;
			m_blocked = true;
		}
		break;

		default:
		{
			/* failed */
			res = -1;
		}
		}

#endif /* posix end */
	}

	return res;
}


bool CCriticalSection::IsLocked()
{
	return m_blocked;
}






/* func */
int	CmsInitializeCS(cms_thread_lock_t *lock)
{
#ifdef WIN32

	InitializeCriticalSection(lock); /* return void */

#else /* posix */

	int res;
	pthread_mutexattr_t lock_attr;

	res = pthread_mutexattr_init(&lock_attr);
	if (res != 0) {
		/* error */
		return -1;
	}

	/* set lock recursion */
	res = pthread_mutexattr_settype(&lock_attr, PTHREAD_MUTEX_RECURSIVE);
	if (res != 0) {
		/* error */
		pthread_mutexattr_destroy(&lock_attr);
		return -1;
	}

	res = pthread_mutex_init(lock, &lock_attr);
	if (res != 0) {
		/* error */
		pthread_mutexattr_destroy(&lock_attr);
		return -1;
	}

	res = pthread_mutexattr_destroy(&lock_attr);
	if (res != 0) {
		/* error */
		return -1;
	}

#endif

	return 0;
}

int CmsDestroyCS(cms_thread_lock_t *lock)
{
#ifdef WIN32

	DeleteCriticalSection(lock); /* return void */

#else /* posix */

	int res;
	res = pthread_mutex_destroy(lock);
	if (res != 0) {
		/* error */
		return -1;
	}

#endif

	return 0;
}

int CmsCSLock(cms_thread_lock_t *lock)
{
	if (lock == NULL) {
		return -1;
	}

	int res = -1;

	if (lock != NULL) {

#ifdef WIN32 /* WIN32 */

		EnterCriticalSection(lock); /* return void */
		res = 0;

#else /* posix */

		res = pthread_mutex_lock(lock);
		switch (res) {
		case 0:
		{
			/* success */
			res = 0;
		}
		break;

		default:
		{
			/* failed */
			res = -1;
		}
		}
#endif /* posix end */
	}

	return res;
}

int CmsCSTrylock(cms_thread_lock_t *lock)
{
	if (lock == NULL) {
		return -1;
	}

	int res = -1;

	if (lock != NULL) {

#ifdef WIN32 /* WIN32 */

		res = TryEnterCriticalSection(lock); /* return bool */
		switch (res) {
		case 0:
		{
			/* another thread already owns */
			res = -1;
		}
		break;

		default:
		{
			/* successfully entered */
			res = 0;
		}
		}

#else /* posix */

		res = pthread_mutex_trylock(lock);
		switch (res) {
		case 0:
		{
			/* successfully entered */
			res = 0;
		}
		break;

		default:
		{
			/* failed */
			res = -1;
		}
		}

#endif /* posix end */
	}

	return res;
}

int CmsCSUnlock(cms_thread_lock_t *lock)
{
	if (lock == NULL) {
		return -1;
	}

	int res = -1;

	if (lock != NULL) {

#ifdef WIN32 /* WIN32 */

		LeaveCriticalSection(lock); /* return void */
		res = 0;

#else /* posix */

		res = pthread_mutex_unlock(lock);
		switch (res) {
		case 0:
		{
			/* success */
			res = 0;
		}
		break;

		default:
		{
			/* failed */
			res = -1;
		}
		}
#endif /* posix end */
	}

	return res;
}




CLock::CLock()
{
	CmsInitializeCS(&m_cs);
}

CLock::~CLock()
{
	CmsDestroyCS(&m_cs);
}

void CLock::Lock()
{
	CmsCSLock(&m_cs);
}

void CLock::Unlock()
{
	CmsCSUnlock(&m_cs);
}

cms_thread_lock_t &CLock::GetLock()
{
	return m_cs;
}

///////////////////////////////////////////////////////////////////////////////
CAutoLock::CAutoLock(CLock &lock) : m_lock(lock)
{
	m_lock.Lock();
}

CAutoLock::~CAutoLock()
{
	m_lock.Unlock();
}


#ifndef WIN32
CEevnt::CEevnt()
{
	pthread_cond_init(&m_cs, NULL);
}

CEevnt::~CEevnt()
{
	pthread_cond_destroy(&m_cs);
}

void CEevnt::Lock()
{
	m_lock.Lock();
}

void CEevnt::Unlock()
{
	m_lock.Unlock();
}

int CEevnt::Wait()
{
	return pthread_cond_wait(&m_cs, &m_lock.GetLock());
}

int CEevnt::Signal()
{
	return pthread_cond_signal(&m_cs);
}

CRWlock::CRWlock()
{
	pthread_rwlock_init(&m_rwlock, NULL);
}

CRWlock::~CRWlock()
{
	pthread_rwlock_destroy(&m_rwlock);
}

void CRWlock::RLock()
{
	pthread_rwlock_rdlock(&m_rwlock);
}

void CRWlock::UnRLock()
{
	pthread_rwlock_unlock(&m_rwlock);
}

void CRWlock::WLock()
{
	pthread_rwlock_wrlock(&m_rwlock);
}

void CRWlock::UnWLock()
{
	pthread_rwlock_unlock(&m_rwlock);
}
#endif
#if 0

#ifdef EMBEDED /* EMBEDED */

int CmsAtomAdd(qvod_atom_t *value)
{
	++(*value);
	return 0;
}

int CmsAtomDec(qvod_atom_t *value)
{
	--(*value);
	return 0;
}

#else /* non-EMBEDED */

int CmsAtomAdd(qvod_atom_t *value)
{
#ifdef WIN32 /* WIN32 */

	InterlockedIncrement(value);

#else /* posix */

	__sync_add_and_fetch(value, 1);

#endif /* posix end */

	return 0;
}

int CmsAtomDec(qvod_atom_t *value)
{
#ifdef WIN32 /* WIN32 */

	InterlockedDecrement(value);

#else /* posix */

	__sync_sub_and_fetch(value, 1);

#endif /* posix end */

	return 0;
}

#endif /* non-EMBEDED end */

#endif /* if 0/1 */




