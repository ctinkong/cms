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
#ifndef __CMS_TS_CHUNKED_H__
#define __CMS_TS_CHUNKED_H__
#include <vector>
#include <mem/cms_mf_mem.h>
#include <common/cms_type.h>

#define TS_CHUNK_SIZE 188
#define TS_SLICE_LEN  (TS_CHUNK_SIZE*100)

typedef struct _TsChunk
{
	uint32 midxMem;
	char *mdata;
	int  muse;		//使用大小
	int  mchunkSize;//开辟空间
}TsChunk;

void initTsMem();
void releaseTsMem();

TsChunk *allocTsChunk(uint32 idx, int chunkSize);
void freeTsChunk(TsChunk *tc);


typedef struct _TsChunkArray
{
	int mchunkTotalSize;		//ts 有效数据长度
	std::vector<TsChunk *> mtsChunkArray;

	OperatorNewDelete
}TsChunkArray;

TsChunkArray *allocTsChunkArray();
void freeTsChunkArray(TsChunkArray *tca);

int writeChunk(uint32 idx, TsChunkArray *tca, char *data, int len);
int beginChunk(TsChunkArray *tca);
int endChunk(TsChunkArray *tca);
int getChunk(TsChunkArray *tca, int i, TsChunk **tc);

#endif