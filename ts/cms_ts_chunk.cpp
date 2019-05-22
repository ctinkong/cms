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
#include <ts/cms_ts_chunk.h>
#include <common/cms_utility.h>
#include <assert.h>

TsChunk *allocTsChunk(int chunkSize)
{
	TsChunk *tc = new TsChunk;
	tc->mdata = new char[chunkSize];
	tc->muse = 0;
	return tc;
}

void freeTsChunk(TsChunk *tc)
{
	assert(tc != NULL);
	if (tc->mdata)
	{
		delete[]tc->mdata;
	}
	delete tc;
}

TsChunkArray *allocTsChunkArray(int chunkSize)
{
	TsChunkArray *tca = new TsChunkArray;
	tca->mchunkSize = chunkSize;
	tca->msliceSize = 0;
	tca->mexSliceSize = 0;
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

int writeChunk(char *tsHeader, int headerLen, TsChunkArray *tca, TsChunkArray *lastTca, char ch, int &writeLen)
{
	return writeChunk(tsHeader, headerLen, tca, lastTca, &ch, 1, writeLen);
}

int writeChunk(char *tsHeader, int headerLen, TsChunkArray *tca, TsChunkArray *lastTca, char *data, int len, int &writeLen)
{
	int left = 0;
	TsChunk *tc;
	tc = NULL;
	// 	if (lastTca != NULL && !lastTca->mtsChunkArray.empty())
	// 	{
	// 		tc = lastTca->mtsChunkArray.at(lastTca->mtsChunkArray.size()-1);
	// 		if (tc->muse < lastTca->mchunkSize)
	// 		{
	// 			left = TS_CHUNK_SIZE-(tc->muse%TS_CHUNK_SIZE);
	// 			left = cmsMin(left,len);
	// 			memcpy(tc->mdata+tc->muse,data,left);
	// 			tc->muse += left;
	// 			writeLen = left;
	// 			lastTca->msliceSize += writeLen;
	// 			lastTca->mexSliceSize += writeLen;
	// 			assert(tc->muse <= lastTca->mchunkSize);
	// 			return (tc->muse%TS_CHUNK_SIZE) == 0 ? 1 : 0;
	// 		}
	// 	}
	// 	tc = NULL;
	if (tca->mtsChunkArray.empty())
	{
		tc = allocTsChunk(tca->mchunkSize);
		if (headerLen > 0)
		{
			memcpy(tc->mdata, tsHeader, headerLen);
			tc->muse = headerLen;
		}
		tca->mtsChunkArray.push_back(tc);
	}
	else
	{
		tc = tca->mtsChunkArray.at(tca->mtsChunkArray.size() - 1);
		if (tc->muse == tca->mchunkSize)
		{
			tc = allocTsChunk(tca->mchunkSize);
			if (headerLen > 0)
			{
				memcpy(tc->mdata, tsHeader, headerLen);
				tc->muse = headerLen;
			}
			tca->mtsChunkArray.push_back(tc);
		}
	}
	left = TS_CHUNK_SIZE - (tc->muse%TS_CHUNK_SIZE);
	left = cmsMin(left, len);
	memcpy(tc->mdata + tc->muse, data, left);
	tc->muse += left;
	writeLen = left;
	tca->msliceSize += writeLen;
	assert(tc->muse <= tca->mchunkSize);
	return (tc->muse%TS_CHUNK_SIZE) == 0 ? 1 : 0;
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

