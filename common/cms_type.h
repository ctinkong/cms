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
#ifndef __CMS_COMMON_VAR_H__
#define __CMS_COMMON_VAR_H__
#include <string.h>
#include <mem/cms_mf_mem.h>

enum ConnType
{
	TypeNetNone,
	TypeHttp,
	TypeHttps,
	TypeRtmp,
	TypeQuery
};

#ifndef CMS_BASIC_TYPE
#define CMS_BASIC_TYPE
typedef long long			int64;
typedef unsigned long long	uint64;
typedef int					int32;
typedef unsigned int		uint32;
typedef short				int16;
typedef unsigned short		uint16;
typedef char				int8;
typedef unsigned char		uint8;
typedef unsigned char		byte;
#endif

#define CMS_CYCLE_MEM_NODE_SIZE (1024*512)
#define CMS_PIPE_BUF_SIZE 8192
#define CMS_HASH_LEN 20

typedef struct _EvTimerParam
{
	uint32	  idx;
	uint64    uid;
}EvTimerParam;

typedef struct _HASH {
public:
	unsigned char data[CMS_HASH_LEN];
	_HASH()
	{
		memset(data, 0, CMS_HASH_LEN);
	}
	_HASH(char* hash)
	{
		memcpy(data, hash, CMS_HASH_LEN);
	}
	_HASH(unsigned char* hash)
	{
		memcpy(data, hash, CMS_HASH_LEN);
	}
	//<
	bool operator < (_HASH const& _A) const
	{
		if (memcmp(data, _A.data, CMS_HASH_LEN) < 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	bool operator < (unsigned char *hash) const
	{
		if (memcmp(data, hash, CMS_HASH_LEN) < 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	bool operator < (char *hash) const
	{
		if (memcmp(data, hash, CMS_HASH_LEN) < 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	//==
	bool operator == (_HASH const& _A) const
	{
		if (memcmp(data, _A.data, CMS_HASH_LEN) == 0)
			return true;
		else
			return false;
	}
	bool operator == (unsigned char *hash) const
	{
		if (memcmp(data, hash, CMS_HASH_LEN) == 0)
			return true;
		else
			return false;
	}
	bool operator == (char *hash) const
	{
		if (memcmp(data, hash, CMS_HASH_LEN) == 0)
			return true;
		else
			return false;
	}
	//!=
	bool operator != (_HASH const& _A) const
	{
		if (memcmp(data, _A.data, CMS_HASH_LEN) == 0)
			return false;
		else
			return true;
	}
	bool operator != (unsigned char *hash) const
	{
		if (memcmp(data, hash, CMS_HASH_LEN) == 0)
			return false;
		else
			return true;
	}
	bool operator != (char *hash) const
	{
		if (memcmp(data, hash, CMS_HASH_LEN) == 0)
			return false;
		else
			return true;
	}
	//=
	_HASH& operator = (_HASH const& _A)
	{
		memcpy(data, _A.data, CMS_HASH_LEN);
		return *this;
	}
	_HASH& operator = (unsigned char *hash)
	{
		memcpy(data, hash, CMS_HASH_LEN);
		return *this;
	}
	_HASH& operator = (char *hash)
	{
		memcpy(data, hash, CMS_HASH_LEN);
		return *this;
	}

	OperatorNewDelete
} HASH;

#endif