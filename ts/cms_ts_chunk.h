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

#define TS_CHUNK_SIZE 188
#define TS_AUDIO_SLICE_LEN	(188*10)
#define TS_VIDEO_SLICE_LEN  (188*100*2)

typedef struct _TsChunk
{
	char *mdata;
	int  muse;
}TsChunk;

TsChunk *allocTsChunk(int chunkSize);
void freeTsChunk(TsChunk *tc);


typedef struct _TsChunkArray
{
	int mchunkSize;
	int msliceSize;
	int mexSliceSize; //被追加的长度
	std::vector<TsChunk *> mtsChunkArray;
}TsChunkArray;

TsChunkArray *allocTsChunkArray(int chunkSize);
void freeTsChunkArray(TsChunkArray *tca);

//writeChunk一次只能写满一个ts 如果需要拆分则需要多次调用 返回值 1 表示写满了一片 否则没写完
int writeChunk(char *tsHeader, int headerLen, TsChunkArray *tca, TsChunkArray *lastTca, char ch, int &writeLen);
int writeChunk(char *tsHeader, int headerLen, TsChunkArray *tca, TsChunkArray *lastTca, char *data, int len, int &writeLen);
int beginChunk(TsChunkArray *tca);
int endChunk(TsChunkArray *tca);
int getChunk(TsChunkArray *tca, int i, TsChunk **tc);

#endif