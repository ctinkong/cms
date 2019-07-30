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

//循环内存分配 内存必须按顺序申请和释放
//适用于用于流媒体数据周期性使用 
//对于音视频首帧(编解码信息帧)和metaData等特殊的数据帧 
//不在循环内存内开辟
#ifndef __CMS_CYCLE_MEM_H__
#define __CMS_CYCLE_MEM_H__

#include <mem/cms_mem_common.h>
#include <mem/cms_mf_mem.h>
#include <common/cms_type.h>
#include <core/cms_lock.h>

//内存链表节点，节点会动态调整，内存分配满了 自动调整到链表末尾
//内存不够用是 会自动扩容
typedef struct _CmsCycleNode
{
	uint32 begin;  //内存分配起始位置
	uint32 end;    //内存分配结束位置
	uint32 size;   //buf size
	byte  *buf;
	struct _CmsCycleNode *prev;
	struct _CmsCycleNode *next;
}CmsCycleNode;

//返回的内存块的内存头结构
typedef struct _CmsCycleChunk
{
	uint32 g;  //属于glic内存
	uint32 b;    //struct _CmsBufNode buf 起始位置
	uint32 e;    //struct _CmsBufNode buf 结束位置	
	uint32 size; //大小 e-b >= size 考虑对齐情况	
	struct _CmsCycleNode *node;
}CmsCycleChunk;

typedef struct _CmsCycleMem
{
	uint32  nodeSize;
	uint32  totalMemSize;
	CLock  *lock;
	struct _CmsCycleNode *releaseNode;		//释放的内存必然属于该节点
	struct _CmsCycleNode *curent;	//当前分配的node 必然指向链表第一个
}CmsCycleMem;


CmsCycleMem *mallocCycleMem(uint32 nodeSize, const char *file, int line);
void freeCycleMem(CmsCycleMem *m);
void *mallocCycleBuf(CmsCycleMem *m, uint32 size, int g, const char *file, int line);
void freeCycleBuf(CmsCycleMem *m, void *p);
//该内存只能应用在顺序内存中，如果开辟内存后，
//需要马上释放内存，请调用该函数，不能调用Free释放！！！！！
void umallocCycleBuf(CmsCycleMem *m, void *p);

#define xmallocCycleMem(nodeSize) mallocCycleMem((nodeSize), __FILE__, __LINE__)
#define xfreeCycleMem(m) freeCycleMem((m))
#define xmallocCycleBuf(m, size, g) mallocCycleBuf((m), (size), (g), __FILE__, __LINE__)
#define xfreeCycleBuf(m, p) freeCycleBuf((m), (p))
#define xumallocCycleBuf(m, p) freeCycleBuf((m), (p))

#endif


