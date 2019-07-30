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

#include <mem/cms_mf_mem.h>
#include <log/cms_log.h>
#include <core/cms_lock.h>
#include <config/cms_config.h>
#include <common/cms_utility.h>
#include <execinfo.h>
#include <stdlib.h>
#include <assert.h>
#include <algorithm>
#include <string>
#include <vector>
#include <map>

std::string  getFileName(char *szDTime)
{
	string fileName;
	char name[128];
	do
	{
		snprintf(name, sizeof(name), "creash.%s.log", szDTime);
		fileName = name;
	} while (0);
	return fileName;
}

void xassert(const char *msg, const char *file, int line)
{
	logs->fatal("assertion failed: %s:%d: \"%s\"\n", file, line, msg);
	void *(callarray[8192]);
	int n;

	std::string dir = CConfig::instance()->clog()->path();
	char szTime[21] = { 0 };
	getDateTime(szTime);
	if (dir.at(dir.length() - 1) != '/')
	{
		dir.append("/");
	}
	std::string name = getFileName(szTime);
	FILE *fp = fopen((dir + name).c_str(), "a+");
	if (fp)
	{
		n = backtrace(callarray, 8192);
		backtrace_symbols_fd(callarray, n, fileno(fp ? fp : stderr));
	}
	fclose(fp);
	abort();
}

#undef malloc
#undef free
#undef realloc
#undef calloc
#undef strdup

#ifdef _CMS_LEAK_CHECK_
#define CMS_LEADK_LIST_COUNT 4
#define CMS_FILENAME_LEN 56
typedef struct _CmsAllocNode
{
	char filename[CMS_FILENAME_LEN];
	int line;
	size_t size;
	void *addr;
	struct _CmsAllocNode *prev;
	struct _CmsAllocNode *next;
	int alloc_time;
} CmsAllocNode;

typedef struct _CmsMemSizeInfo
{
	unsigned long long size;
	unsigned int idx;
} CmsMemSizeInfo;

CmsAllocNode *g_AllocHead[CMS_LEADK_LIST_COUNT] = { NULL };
CLock g_AllocHeadLocker[CMS_LEADK_LIST_COUNT];

unsigned long long getAllocIdx(void *p)
{
	unsigned long long i = (unsigned long long)p % CMS_LEADK_LIST_COUNT;
	return i;
}

static void addAllocNode(void *p, size_t size, void *addr, const char *file, int line)
{
	unsigned long long i = getAllocIdx(p);
	g_AllocHeadLocker[i].Lock();
	//error_log(LOG_DEBUG, "oct_allocing %p %p to %p @ %s %d\n", p, (char *)p + sizeof(CmsAllocNode), (char *)p + sizeof(CmsAllocNode) + size, file, line);
	CmsAllocNode *n = (CmsAllocNode *)p;
	memset(n->filename, 0, CMS_FILENAME_LEN);
	const char *pp = strstr(file, "./");
	snprintf(n->filename, CMS_FILENAME_LEN * sizeof(char), "%s", pp == file ? pp + 2 : file);
	n->line = line;
	n->size = size;
	n->addr = addr;
	n->alloc_time = time(NULL);
	n->prev = NULL;
	n->next = g_AllocHead[i];
	if (g_AllocHead[i])
	{
		g_AllocHead[i]->prev = n;
	}
	else
	{
	}
	g_AllocHead[i] = n;
	g_AllocHeadLocker[i].Unlock();
}

static void delAllocNode(void *p)
{
	unsigned long long i = getAllocIdx(p);
	g_AllocHeadLocker[i].Lock();
	CmsAllocNode *s = (CmsAllocNode *)p;
	//error_log(LOG_DEBUG, "oct_freeing %p %p to %p @ %s %d\n", p, (char *)p + sizeof(CmsAllocNode), (char *)p + sizeof(CmsAllocNode) + s->size, s->filename, s->line);
	if (s == g_AllocHead[i])
	{
		g_AllocHead[i] = s->next;
	}
	if (s->prev)
	{
		s->prev->next = s->next;
	}
	if (s->next)
	{
		s->next->prev = s->prev;
	}
	g_AllocHeadLocker[i].Unlock();
}

void * cms_xmalloc(size_t sz, const char *file, int line)
{
	void *p;

	if (sz < 1)
		sz = 1;

	if ((p = calloc(1, sz + sizeof(CmsAllocNode))) == NULL)
	{
		exit(1);
	}
	addAllocNode(p, sz, p, file, line);
	return (((char*)p) + sizeof(CmsAllocNode));
}

/*
 *  xfree() - same as free(3).  Will not call free(3) if s == NULL.
 */
void xfree(void *s)
{
	if (s == NULL)
		return;
	void *n = ((char*)s - sizeof(CmsAllocNode));
	delAllocNode(n);
	if (n != NULL)
		free(n);
}

/* xxfree() - like xfree(), but we already know s != NULL */
void xxfree(const void *s_const)
{
	if (!s_const)
	{
		fprintf(stderr, "Oh NULL\n");
		abort();
	}
	void *s = (void *)s_const;
	void *n = ((char*)s - sizeof(CmsAllocNode));
	delAllocNode(n);
	free(n);
}

/*
 *  xrealloc() - same as realloc(3). Used for portability.
 *  Never returns NULL; fatal on error.
 */
void * cms_xrealloc(void *s, size_t sz, const char *file, int line)
{
	void *n = NULL;
	if (s)
	{
		n = ((char*)s - sizeof(CmsAllocNode));
		delAllocNode(n);
	}
	void *p;
	if ((p = realloc(s ? n : s, sz + sizeof(CmsAllocNode))) == NULL)
	{
		abort();
	}
	addAllocNode(p, sz, p, file, line);
	return (((char*)p) + sizeof(CmsAllocNode));
}

/*
 *  xcalloc() - same as calloc(3).  Used for portability.
 *  Never returns NULL; fatal on error.
 */
void * cms_xcalloc(size_t n, size_t sz, const char *file, int line)
{
	void *p;

	if (n < 1)
		n = 1;
	if (sz < 1)
		sz = 1;
	if ((p = calloc(1, n * sz + sizeof(CmsAllocNode))) == NULL)
	{
		perror("xcalloc");
		abort();
	}
	addAllocNode(p, n*sz, p, file, line);
	return (((char*)p) + sizeof(CmsAllocNode));
}


//自定义排序函数  
bool sortFun(const CmsMemSizeInfo &v1, const CmsMemSizeInfo &v2)
{
	return v1.size < v2.size;//升序排列  
}



static CmsAllocNode * addTmpNode(CmsAllocNode *head, size_t size, const char *file, int line)
{
	CmsAllocNode *n = (CmsAllocNode *)malloc(sizeof(CmsAllocNode));
	snprintf(n->filename, CMS_FILENAME_LEN * sizeof(char), "%s", file);
	n->line = line;
	n->size = size;
	n->addr = NULL;
	n->prev = NULL;
	n->next = head;
	if (head)
	{
		head->prev = n;
	}
	else
	{
	}
	head = n;
	return head;
}

static void delAllTmpNode(CmsAllocNode *head)
{
	CmsAllocNode *node = head;
	CmsAllocNode *next = NULL;
	while (node)
	{
		next = node->next;
		free(node);
		node = next;
	}
}

std::string printfMemUsage()
{
	unsigned long long nodeNum = 0;
	unsigned long long extraMem = 0;
	unsigned long long totalMem = 0;
	CmsAllocNode *tmpNodes = NULL;
	unsigned long beginTime = getTickCount();
	for (int i = 0; i < CMS_LEADK_LIST_COUNT; i++)
	{
		//直接统计数据太耗时间 弄个临时list 记录每个node
		g_AllocHeadLocker[i].Lock();
		CmsAllocNode *head = g_AllocHead[i];
		while (head)
		{
			tmpNodes = addTmpNode(tmpNodes, head->size, head->filename, head->line); //临时拷贝
			head = head->next;
			nodeNum++;
		}
		g_AllocHeadLocker[i].Unlock();
	}
	unsigned long memInfoEndTime = getTickCount();

	std::map<std::string, std::map<int, unsigned long long> > mapMemInfo;
	CmsAllocNode *head = tmpNodes;
	while (head)
	{
		totalMem += sizeof(CmsAllocNode) + head->size;
		extraMem += sizeof(CmsAllocNode);
		std::map<std::string, std::map<int, unsigned long long> >::iterator itFileName = mapMemInfo.find(head->filename);
		if (itFileName != mapMemInfo.end())
		{
			std::map<int, unsigned long long>::iterator itLine = itFileName->second.find(head->line);
			if (itLine != itFileName->second.end())
			{
				itLine->second += head->size;
			}
			else
			{
				itFileName->second.insert(std::make_pair(head->line, head->size));
			}
		}
		else
		{
			std::map<int, unsigned long long> mapRecord;
			mapRecord.insert(std::make_pair(head->line, head->size));
			mapMemInfo.insert(std::make_pair(head->filename, mapRecord));
		}
		head = head->next;
	}
	delAllTmpNode(tmpNodes);
	unsigned long tmpTime = getTickCount();

	std::string memInfo = "mem info:\n";
	std::string memSize;
	std::string unit;
	std::vector<std::string> vecMemInfo;
	std::vector<CmsMemSizeInfo> vecMemSize;
	char szMenInfo[1024 * 4];
	std::map<std::string, std::map<int, unsigned long long> >::iterator itFileName;
	for (itFileName = mapMemInfo.begin(); itFileName != mapMemInfo.end(); itFileName++)
	{
		std::map<int, unsigned long long>::iterator itLine;
		for (itLine = itFileName->second.begin(); itLine != itFileName->second.end(); itLine++)
		{
			parseSpeed8Mem(itLine->second, false, memSize, unit);
			snprintf(szMenInfo,
				sizeof(szMenInfo),
				"+++++file   %-40s line %-10d size %-8s %s\n",
				itFileName->first.c_str(),
				itLine->first,
				memSize.c_str(),
				unit.c_str());
			CmsMemSizeInfo cmsi;
			cmsi.size = itLine->second;
			cmsi.idx = vecMemSize.size();
			vecMemSize.push_back(cmsi);
			vecMemInfo.push_back(szMenInfo);
		}
	}
	if (vecMemSize.size())
	{
		std::sort(vecMemSize.begin(), vecMemSize.end(), sortFun); //排序
		std::vector<CmsMemSizeInfo>::iterator itVec = vecMemSize.begin();
		for (; itVec != vecMemSize.end(); itVec++)
		{
			memInfo += vecMemInfo.at((*itVec).idx).c_str();
		}
	}
	unsigned long endTime = getTickCount();
	snprintf(szMenInfo,
		sizeof(szMenInfo),
		"\n#####total mem %s, extra mem %s, extra percentage %llu, node num %llu, mem take time %lu ms, tmp node time %lu ms, total take time %lu ms\n",
		parseSpeed8Mem(totalMem, false).c_str(),
		parseSpeed8Mem(extraMem, false).c_str(),
		totalMem ? (extraMem * 100 / totalMem) : 0,
		nodeNum,
		memInfoEndTime - beginTime,
		tmpTime - memInfoEndTime,
		endTime - beginTime);
	memInfo += szMenInfo;

	logs->fatal("%s", memInfo.c_str());
	return memInfo;
}
#else
void * xmalloc(size_t sz)
{
	void *p;
	if (sz < 1)
		sz = 1;
	if ((p = malloc(sz)) == NULL)
	{
		perror("malloc");
		abort();
	}
	return (p);
}

/*
 *  xfree() - same as free(3).  Will not call free(3) if s == NULL.
 */
void xfree(void *s)
{
	if (s != NULL)
		free(s);
}

/* xxfree() - like xfree(), but we already know s != NULL */
void xxfree(const void *s_const)
{
	void *s = (void *)s_const;
	free(s);
}

/*
 *  xrealloc() - same as realloc(3). Used for portability.
 *  Never returns NULL; fatal on error.
 */
void * xrealloc(void *s, size_t sz)
{
	void *p;
	if (sz < 1)
		sz = 1;
	if ((p = realloc(s, sz)) == NULL)
	{
		perror("realloc");
		abort();
	}
	return (p);
}

/*
 *  xcalloc() - same as calloc(3).  Used for portability.
 *  Never returns NULL; fatal on error.
 */
void * xcalloc(size_t n, size_t sz)
{
	void *p;
	if (n < 1)
		n = 1;
	if (sz < 1)
		sz = 1;
	if ((p = calloc(n, sz)) == NULL)
	{
		perror("xcalloc");
		abort();
	}
	return (p);
}

#endif

#ifdef _CMS_LEAK_CHECK_
char * cms_xstrdup(const char *s, const char *file, int line)
#else
char * xstrdup(const char *s)
#endif
{
	size_t sz;
	assert(s);
	/* copy string, including terminating character */
	sz = strlen(s) + 1;
#ifdef _CMS_LEAK_CHECK_
	return (char *)memcpy(cms_xmalloc(sz, file, line), s, sz);
#else
	return (char *)memcpy(xmalloc(sz), s, sz);
#endif
}


