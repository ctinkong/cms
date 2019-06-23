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
#include <errno.h>
#include <memory.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>	   
#include <common/cms_shmmgr.h>
#include <log/cms_log.h>
#include <enc/cms_crc32n.h>
#include <common/cms_utility.h>

CShmMgr* CShmMgr::minstance = NULL;

CShmMgr::CShmMgr()
{
	createShmKey(0, "cms");
}

CShmMgr::~CShmMgr()
{

}

CShmMgr* CShmMgr::instance()
{
	if (minstance == NULL)
		minstance = new CShmMgr();

	return minstance;
}

void * 	CShmMgr::createShm(int keyIndex, unsigned int size)
{
	char * 	shmBuff = NULL;

	if (keyIndex <= 0 || keyIndex >= SHM_TYPE_MAX || !size)
	{
		logs->error("***[CShmMgr::CreateShm] Error : CreateShm -  keyIndex = %d size %d ***", keyIndex, size);
		return NULL;
	}

	key_t _key = m_key[keyIndex]._key;

	int tmpid = shmget(_key, 0, 0);

	shmctl(tmpid, IPC_RMID, 0);

	int _shmid = shmget(_key, size, IPC_CREAT | 0666);

	if (_shmid != -1)
	{
		shmBuff = (char*)shmat(_shmid, NULL, 0);
		if ((long)shmBuff != -1)
		{
			memset(shmBuff, 0, size);
			return shmBuff;
		}
		else
		{
			logs->error("***[CShmMgr::CreateShm]Error : CreateShm - shmat error  key %u size %d %s ***", m_key[keyIndex]._key, size, strerror(errno));
		}
	}
	else
	{
		logs->error("***[CShmMgr::CreateShm]Error : CreateShm - shmget error  key %u size %d %s ***", m_key[keyIndex]._key, size, strerror(errno));
	}

	return NULL;
}

void * 	CShmMgr::getShm(int keyIndex, unsigned int size)
{
	char * 		shmBuff = NULL;

	if (keyIndex <= 0 || keyIndex >= SHM_TYPE_MAX)
	{
		logs->error("***[CShmMgr::GetShm]Error : GetShm -  keyIndex = %d ***", keyIndex);
		return NULL;
	}

	key_t _key = m_key[keyIndex]._key;

	int _shmid = shmget(_key, size, IPC_EXCL);
	if (_shmid != -1)
	{
		shmBuff = (char*)shmat(_shmid, NULL, 0);
		if ((long)shmBuff != -1)
		{
			return shmBuff;
		}
	}

	return NULL;
}

void CShmMgr::createShmKey(int entityNum, std::string key)
{
	int i = 0;

	memset((char *)m_key, 0, sizeof(m_key));

	for (i = 0; i < SHM_TYPE_MAX; i++)
	{
		snprintf(m_key[i].keyFilename, sizeof(m_key[i].keyFilename) - 1, "TinKong_%s_RTMP_LIVE_%d",
			key.c_str(), i);
		logs->info("+++ [CShmMgr::CreateShmKey] key name %s +++", m_key[i].keyFilename);
	}

	for (i = 0; i < SHM_TYPE_MAX; i++)
	{
		m_key[i]._key = makeShmKey(m_key[i].keyFilename, 0);
	}
}

key_t CShmMgr::makeShmKey(char * buff, int reserved)
{
	unsigned int value = crc32(0, (const unsigned char*)buff, strlen(buff));

	key_t _key = (key_t)value;

	return _key;
}

void CShmMgr::detach(void *shmaddr)
{
	shmdt(shmaddr);
}

