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
//固定内存分配器 用于固定大小内存频繁分配的情况
#ifndef __CMS_FIX_MEM_H__
#define __CMS_FIX_MEM_H__
#include <common/cms_type.h>
#include <mem/cms_mem_common.h>

/*
//by water 2019-07-26
														               struct _CmsFixChunk  初始化示意图
																		
																		   +-+-+-+-+-+-+-+-+-+-+-+-+-+ --------+
																		   |          begin          | --------+-------------------------------------+
																		   +-+-+-+-+-+-+-+-+-+-+-+-+-+         |                                     |
																		   |          end            | --------+---------------------------------+   |
																		   +-+-+-+-+-+-+-+-+-+-+-+-+-+         |                                 |   |
																	+----- |          curent         |         |---------> struct _CmsFixChunk   |   |
																	|	   +-+-+-+-+-+-+-+-+-+-+-+-+-+         |                                 |   |
																	|	   |          prev           |         |                                 |   |
																	|	   +-+-+-+-+-+-+-+-+-+-+-+-+-+         |                                 |   |
																	|	   |          next           |         |                                 |   |
														            +----> +-+-+-+-+-+-+-+-+-+-+-+-+-+ --------+  <------------------------------+---+---------------+
										+--------------------------------- |          begin          |         |                                 |   |               |
										|			                       +-+-+-+-+-+-+-+-+-+-+-+-+-+         |                                 |   |               |
										|	 +---------------------------- |          end            |         |                                 |   |               |
										|	 |                             +-+-+-+-+-+-+-+-+-+-+-+-+-+         |---------> struct _CmsFixNode    |   |               |
										|	 |                             |          prev           |         |                                 |   |               |
										|	 |                             +-+-+-+-+-+-+-+-+-+-+-+-+-+         |                                 |   |               |
										|	 |                             |          next           |         |                                 |   |               |
										|	 |                             +-+-+-+-+-+-+-+-+-+-+-+-+-+ --------+  <------------------------------+---+------+        |
										|	 |   +------------------------ |          begin          |         |                                 |   |      |        |
										|	 |   |                         +-+-+-+-+-+-+-+-+-+-+-+-+-+         |                                 |   |      |        |
										|	 |   |     +------------------ |          end            |         |                                 |   |      |        |
										|	 |   |     |                   +-+-+-+-+-+-+-+-+-+-+-+-+-+         |---------> struct _CmsFixNode    |   |      |        |
										|	 |   |     |                   |          prev           |         |                                 |   |      |        |
										|	 |   |     |                   +-+-+-+-+-+-+-+-+-+-+-+-+-+         |                                 |   |      |        |
										|	 |   |     |                   |          next           |         |                                 |   |      |        |
										|	 |   |     |                   +-+-+-+-+-+-+-+-+-+-+-+-+-+ --------+  <--+                           |   |      |        |
										|	 |   |     |    +------------- |          begin          |         |     |                           |   |      |        |
										|	 |   |     |    |              +-+-+-+-+-+-+-+-+-+-+-+-+-+     +---+-----+                           |   |      |        |
										|	 |   |     |    |      +------ |          end            |     |   |                                 |   |      |        |
										|	 |   |     |    |      |       +-+-+-+-+-+-+-+-+-+-+-+-+-+     |   |---------> struct _CmsFixNode    |   |      |        |
										|	 |   |     |    |      |       |          prev           |     |   |                                 |   |      |        |
										|	 |   |     |    |      |       +-+-+-+-+-+-+-+-+-+-+-+-+-+     |   |                                 |   |      |        |
										|	 |   |     |    |      |       |          next           |     |   |                                 |   |      |        |
										|	 |   |     |    |      |       +-+-+-+-+-+-+-+-+-+-+-+-+-+ ----+---+                                 |   |      |        |
										|	 |   |     |    |      |       |        .......          |     |                                     |   |      |        |
										|	 |   |     |    |      |       +-+-+-+-+-+-+-+-+-+-+-+-+-+ ----+---+ <-------------------------------+---+      |        |
										|	 |   |     |    |      |       |          fc             |     |   |                                 |          |        |
										|	 |   |     |    |      |       +-+-+-+-+-+-+-+-+-+-+-+-+-+     |   |           struct _CmsDataHeader |          |        |
										|	 |   |     |    |      |       |          node           | ----+   |---------> and                   |          |        |
										|	 |   |     |    +------------> +-+-+-+-+-+-+-+-+-+-+-+-+-+         |           data                  |          |        |
										|	 |   |     |           |       |          data           |         |                                 |          |        |
										|	 |   |     |           +-----> +-+-+-+-+-+-+-+-+-+-+-+-+-+ --------+                                 |          |        |
										|	 |   |     |                   |          fc             |                                           |          |        |
										|	 |   |     |                   +-+-+-+-+-+-+-+-+-+-+-+-+-+                                           |          |        |
										|	 |   |     |                   |          node           | ------------------------------------------+----------+        |
										|	 |   +-----+-----------------> +-+-+-+-+-+-+-+-+-+-+-+-+-+                                           |                   |
										|	 |         |                   |          data           |                                           |                   |
										|	 |         +-----------------> +-+-+-+-+-+-+-+-+-+-+-+-+-+                                           |                   |
										|	 |                             |          fc             |                                           |                   |
										|	 |                             +-+-+-+-+-+-+-+-+-+-+-+-+-+                                           |                   |
										|	 |                             |          node           | ------------------------------------------+-------------------+
										+--------------------------------> +-+-+-+-+-+-+-+-+-+-+-+-+-+                                           |
											 |                             |          data           |                                           |
										     +---------------------------> +-+-+-+-+-+-+-+-+-+-+-+-+-+ <-----------------------------------------+
																			
*/


typedef struct _CmsFixNode
{
	byte *begin;			//小块内存开始位置
	byte *end;				//小块内存结束位置	
	struct _CmsFixNode *prev;
	struct _CmsFixNode *next;
}CmsFixNode;

typedef struct _CmsFixChunk
{
	int  available;			//可用块数
	int  total;				//总块数
	byte *begin;			//整个内存开始位置
	byte *end;				//整个内存结束位置
	CmsFixNode *curent;		//当前可用小块

	struct _CmsFixChunk *prev;
	struct _CmsFixChunk *next;
}CmsFixChunk;

typedef struct _CmsDataHeader
{
	CmsFixChunk *fc;
	CmsFixNode *node;	
}CmsDataHeader;

typedef struct _CmsFixMem
{
	int idx;				//主要是为了测试
	int nodeSize;			//每个申请的内存大小
	int nodeCount;			//每个chunk包含多少个node
	int emptyCount;			//可用chunk数
	int curNum;				//临时使用
	CmsFixChunk *curent;	//当前可用块
}CmsFixMem;

CmsFixMem *mallocFixMem(int nodeSize, int count, const char * file, int line);
void freeFixMem(CmsFixMem *m);
void *mallocFix(CmsFixMem *m, const char * file, int line);
void freeFix(CmsFixMem *m, void *p);

#define xmallocFixMem(nodeSize, count) mallocFixMem((nodeSize), (count), (__FILE__), (__LINE__))
#define xfreeFixMem(m) freeFixMem((m))
#define xmallocFix(m) mallocFix((m), (__FILE__), (__LINE__))
#define xfreeFix(m, p) freeFix((m),(p))

#endif //__CMS_FIX_MEM_H__


