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
#include <ts/cms_ts_chunk.h>
#include <common/cms_utility.h>
#include <mem/cms_mf_mem.h>
#include <mem/cms_fix_mem.h>
#include <app/cms_app_info.h>
#include <assert.h>

#ifdef __CMS_POOL_MEM__

static CmsFixMem *tsFixMem[APP_ALL_MODULE_THREAD_NUM]; //每个线程独享 不需要加锁

static void initTsFixMem()
{
	for (int i = 0; i < APP_ALL_MODULE_THREAD_NUM; i++)
	{
		tsFixMem[i] = xmallocFixMem(TS_SLICE_LEN, 10);
	}
}

static void releaseTsFixMem()
{
	for (int i = 0; i < APP_ALL_MODULE_THREAD_NUM; i++)
	{
		xfreeFixMem(tsFixMem[i]);
	}
}

static void *mallocTsSlice(int i)
{
	void *ptr = NULL;
	ptr = xmallocFix(tsFixMem[i]);
	return ptr;
}

static void freeTsSlice(int i, void *ptr)
{
	xfreeFix(tsFixMem[i], ptr);
}

void initTsMem()
{
	initTsFixMem();
}

void releaseTsMem()
{
	releaseTsFixMem();
}

#endif //__CMS_POOL_MEM__

TsChunk *allocTsChunk(uint32 idx, int chunkSize)
{
	TsChunk *tc = new TsChunk;
	tc->midxMem = idx;

	tc->mchunkSize = chunkSize;
#ifdef __CMS_POOL_MEM__
	tc->mdata = (char*)mallocTsSlice(idx);
#else
	tc->mdata = (char*)xmalloc(chunkSize);
#endif	
	tc->muse = 0;
	return tc;
}

void freeTsChunk(TsChunk *tc)
{
	assert(tc != NULL);
	if (tc->mdata)
	{
#ifdef __CMS_POOL_MEM__
		freeTsSlice(tc->midxMem, tc->mdata);
#else
		xfree(tc->mdata);
#endif			
	}
	delete tc;
}

TsChunkArray *allocTsChunkArray()
{
	TsChunkArray *tca = new TsChunkArray;
	tca->mchunkTotalSize = 0;
	return tca;
}

void freeTsChunkArray(TsChunkArray *tca)
{
	std::vector<TsChunk *>::iterator it = tca->mtsChunkArray.begin();
	for (; it != tca->mtsChunkArray.end();)
	{
		freeTsChunk(*it);
		it = tca->mtsChunkArray.erase(it);
	}
}

int writeChunk(uint32 idx, TsChunkArray *tca, char *data, int len)
{
	//返回一个ts剩余长度
	int left = 0;
	TsChunk *tc;
	tc = NULL;
	if (tca->mtsChunkArray.empty())
	{
		tc = allocTsChunk(idx, TS_SLICE_LEN);
		tca->mtsChunkArray.push_back(tc);
	}
	else
	{
		tc = tca->mtsChunkArray.at(tca->mtsChunkArray.size() - 1);
		if (tc->mchunkSize - tc->muse < len)
		{
			assert((tc->mchunkSize - tc->muse) % TS_CHUNK_SIZE == 0);
			tc = allocTsChunk(idx, TS_SLICE_LEN);
			tca->mtsChunkArray.push_back(tc);
		}
	}
	left = TS_CHUNK_SIZE - (tc->muse%TS_CHUNK_SIZE);
	assert(len <= left);
	int writeLen = cmsMin(left, len);
	memcpy(tc->mdata + tc->muse, data, writeLen);
	tc->muse += writeLen;
	tca->mchunkTotalSize += writeLen;
	return left - writeLen;
}

int  beginChunk(TsChunkArray *tca)
{
	if (tca->mtsChunkArray.empty())
	{
		return -1;
	}
	return 0;
}

int  endChunk(TsChunkArray *tca)
{
	if (tca->mtsChunkArray.empty())
	{
		return -1;
	}
	return (int)tca->mtsChunkArray.size();
}

int  getChunk(TsChunkArray *tca, int i, TsChunk **tc)
{
	*tc = NULL;
	if (i < 0)
	{
		return -1;
	}
	if (i >= (int)tca->mtsChunkArray.size())
	{
		return -1;
	}
	*tc = tca->mtsChunkArray.at(i);
	return 1;
}

