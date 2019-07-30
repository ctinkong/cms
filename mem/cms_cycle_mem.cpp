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


#include <mem/cms_cycle_mem.h>
#include <assert.h>

void freeCycleNode(CmsCycleNode *node);

CmsCycleMem *mallocCycleMem(uint32 nodeSize, const char *file, int line)
{
#ifdef _CMS_LEAK_CHECK_
	CmsCycleMem *m = (CmsCycleMem *)cms_xmalloc(sizeof(CmsCycleMem), file, line);
#else
	CmsCycleMem *m = (CmsCycleMem *)xmalloc(sizeof(CmsCycleMem));
#endif
	m->lock = new CLock();
	m->nodeSize = nodeSize;
	m->totalMemSize = 0;
	m->releaseNode = NULL;
	m->curent = NULL;
	return m;
}

void freeCycleMem(CmsCycleMem *m)
{
	if (!m)
	{
		return;
	}
	CmsCycleNode *node = m->releaseNode;
	CmsCycleNode *next = NULL;
	for (; node != NULL;)
	{
		next = node->next;
		freeCycleNode(node);
		node = next;
	}
	delete m->lock;
	xfree(m);
}

CmsCycleNode *mallocCycleNode(uint32 nodeSize, const char *file, int line)
{
#ifdef _CMS_LEAK_CHECK_
	CmsCycleNode *n = (CmsCycleNode *)cms_xmalloc(sizeof(CmsCycleNode), file, line);
#else
	CmsCycleNode *n = (CmsCycleNode *)xmalloc(sizeof(CmsCycleNode));
#endif
	n->begin = 0;
	n->end = 0;
	n->size = nodeSize;
#ifdef _CMS_LEAK_CHECK_
	n->buf = (byte *)cms_xmalloc(nodeSize, file, line);
#else
	n->buf = (byte *)xmalloc(nodeSize);
#endif
	n->prev = NULL;
	n->next = NULL;
	return n;
}

void freeCycleNode(CmsCycleNode *node)
{
	xfree(node->buf);
	xfree(node);
}

uint32 cycleNodeUnuseSize(CmsCycleNode *n)
{
	return n->size - n->end;
}

void *mallocFromCycleNode(CmsCycleNode *node, uint32 size)
{
	byte *ptr = node->buf + node->end;
	CmsCycleChunk *cc = (CmsCycleChunk *)ptr;
	cc->g = 0;
	cc->b = node->end;

	node->end += size;

	cc->e = node->end;
	cc->size = size; //实际内存大小
	cc->node = node;

	ptr += sizeof(CmsCycleChunk);
	assert(node->end <= node->size);
	return ptr;
}

void free2CycleNode(CmsCycleNode *node, CmsCycleChunk *cc)
{
	node->begin += cc->size;
	assert(node->begin <= node->end);
	if (node->begin == node->end)
	{
		//变成空闲了
		node->begin = 0;
		node->end = 0;
	}
}
//反序释放
void umallocCycleNode(CmsCycleNode *node, CmsCycleChunk *cc)
{
	node->end -= cc->size;
	assert(node->begin <= node->end);
	if (node->begin == node->end)
	{
		//变成空闲了
		node->begin = 0;
		node->end = 0;
	}
}

int cycleNodeEmpty(CmsCycleNode *node)
{
	if (node->begin == 0 && node->end == 0)
	{
		return 1;
	}
	return 0;
}

void *mallocCycleBuf(CmsCycleMem *m, uint32 size, int g, const char *file, int line)
{
	byte *ptr = NULL;
	m->lock->Lock();
	do
	{
		uint32 totalSize = sizeof(CmsCycleChunk) + size;
		if (g || totalSize > m->nodeSize * 90 / 100)
		{
			//从glibc分配内存
#ifdef _CMS_LEAK_CHECK_
			ptr = (byte *)cms_xmalloc(totalSize, file, line);
#else
			ptr = (byte *)xmalloc(totalSize);
#endif
			CmsCycleChunk *cc = (CmsCycleChunk *)ptr;
			cc->g = 1;
			cc->b = 0; //忽略
			cc->e = 0; //忽略
			cc->size = totalSize; //大小
			cc->node = NULL;
			ptr += sizeof(CmsCycleChunk);
			break;
		}
		if (!m->curent)
		{
			m->curent = mallocCycleNode(m->nodeSize, file, line);
			m->releaseNode = m->curent;
			m->totalMemSize += m->nodeSize;
		}
		CmsCycleNode *node = m->curent;
		CmsCycleNode *next = NULL;
		uint32 left = cycleNodeUnuseSize(node);
		if (left >= totalSize)
		{
			ptr = (byte *)mallocFromCycleNode(node, totalSize);
			break;
		}
		//最多只查看下一个节点
		next = node->next;
		if (next)
		{
			left = cycleNodeUnuseSize(next);
			if (left >= totalSize)
			{
				ptr = (byte *)mallocFromCycleNode(next, totalSize);
				m->curent = next;  //更新curent
				break;
			}
		}
		//当前节点和下一节点没有满足的 新开辟node
		next = mallocCycleNode(m->nodeSize, file, line);
		m->totalMemSize += m->nodeSize;
 		if (node->next)
 		{
 			node->next->prev = next;
 			next->next = node->next;
 		}
		node->next = next;
		next->prev = node;
		ptr = (byte *)mallocFromCycleNode(next, totalSize);
		m->curent = next; //更新curent
	} while (0);

	m->lock->Unlock();
	return ptr;
}

void freeCycleBuf(CmsCycleMem *m, void *p)
{
	byte *ptr = (byte *)p - sizeof(CmsCycleChunk);
	CmsCycleChunk *cc = (CmsCycleChunk *)(ptr);
	if (cc->g)
	{
		//不属于循环内存的数据包
		xfree(ptr);
		return;
	}

	m->lock->Lock();
	//由于是按顺序释放 释放的内存必然属于 releaseNode
	assert(cc->node == m->releaseNode);
	free2CycleNode(m->releaseNode, cc);
	if (cycleNodeEmpty(m->releaseNode))
	{
		if (m->releaseNode != m->curent)//小心成为闭环
		{
			CmsCycleNode *next = m->releaseNode->next;
			if (m->curent->next)
			{
				m->curent->next->prev = m->releaseNode;
				m->releaseNode->next = m->curent->next;
			}
			else
			{
				m->releaseNode->next = NULL;
			}
			m->curent->next = m->releaseNode;
			m->releaseNode->prev = m->curent;
			m->releaseNode = next;
			m->releaseNode->prev = NULL; //必须置零
		}
	}
	m->lock->Unlock();
}


void umallocCycleBuf(CmsCycleMem *m, void *p)
{
	byte *ptr = (byte *)p - sizeof(CmsCycleChunk);
	CmsCycleChunk *cc = (CmsCycleChunk *)(ptr);
	if (cc->g)
	{
		//不属于循环内存的数据包
		xfree(ptr);
		return;
	}
	m->lock->Lock();
	//由于是反序释放 释放的内存必然属于  curent
	assert(cc->node == m->curent);
	umallocCycleNode(m->curent, cc);
	m->lock->Unlock();
}


