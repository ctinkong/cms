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
#ifndef __CMS_LOCK_H__
#define __CMS_LOCK_H__

#ifdef WIN32  /* WIN32 */

typedef	CRITICAL_SECTION cms_thread_lock_t;
typedef	volatile long cms_atom_t;

#else /* posix */

#include <pthread.h>

typedef pthread_mutex_t cms_thread_lock_t;
typedef pthread_rwlock_t cms_thread_rd_lock_t;
typedef volatile long cms_atom_t;

#endif /* posix end */


/* qvod CCriticalSection */
class CCriticalSection
{
public:

	CCriticalSection(cms_thread_lock_t *lock);
	~CCriticalSection();
	int Lock();
	int TryLock();
	int UnLock();
	bool IsLocked();

private:

	cms_thread_lock_t * m_lock;
	bool m_blocked;
};


/* qvod CLock */
class CLock
{
public:
	CLock();
	~CLock();

	void Lock();
	void Unlock();

private:
	cms_thread_lock_t  m_cs;
};

class CAutoLock
{
public:
	CAutoLock(CLock &lock);
	~CAutoLock();

private:
	CLock & m_lock;
};

#ifndef WIN32
//read write lock
class CRWlock
{
public:
	CRWlock();
	virtual ~CRWlock();

	void RLock();
	void UnRLock();
	void WLock();
	void UnWLock();
private:
	cms_thread_rd_lock_t   m_rwlock;
};

class CAutoRWLock
{
public:
	CAutoRWLock(CRWlock &lock, bool bRLock) :m_lock(lock)
	{
		m_bRLock = bRLock;
		if (bRLock)
		{
			m_lock.RLock();
		}
		else
		{
			m_lock.WLock();
		}
	}
	~CAutoRWLock()
	{
		if (m_bRLock)
		{
			m_lock.UnRLock();
		}
		else
		{
			m_lock.UnWLock();
		}
	}
private:
	bool  m_bRLock;
	CRWlock &m_lock;
};
#endif
/* func */
int	CmsInitializeCS(cms_thread_lock_t *lock);
int CmsDestroyCS(cms_thread_lock_t *lock);
int CmsCSLock(cms_thread_lock_t *lock);
int CmsCSTrylock(cms_thread_lock_t *lock);
int CmsCSUnlock(cms_thread_lock_t *lock);


/* qvod atom operation */
int CmsAtomAdd(cms_atom_t *value);
int CmsAtomDec(cms_atom_t *value);





#endif /* _QVOD_LOCK_H_ */


