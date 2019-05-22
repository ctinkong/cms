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
#ifndef __SHM_MGR_H__
#define __SHM_MGR_H__

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>

class CShmMgr
{
private:
	struct ShmKey
	{
		char 	keyFilename[128];
		key_t	_key;
	};
public:
	enum
	{
		SHM_TYPE_UPDATE = 1,
		SHM_TYPE_MAX
	};

	CShmMgr();
	~CShmMgr();

	void *createShm(int keyIndex, unsigned int size);
	void *getShm(int keyIndex, unsigned int size);
	void detach(void *shmaddr);
	static	CShmMgr* instance();
private:
	ShmKey	m_key[SHM_TYPE_MAX];

private:

	void createShmKey(int entityNum, std::string key);
	key_t makeShmKey(char * buff, int reserved);
	static CShmMgr* minstance;
};

#endif

