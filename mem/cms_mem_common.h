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

#ifndef __CMS_MEM_COMMON_H__
#define __CMS_MEM_COMMON_H__
#include <common/cms_type.h>


#define CMS_NEED_ALIGNMENT       //是否启用地址对齐

#define CMS_POOL_ALIGNMENT       16
#ifndef CMS_ALIGNMENT
#define CMS_ALIGNMENT   sizeof(unsigned long)    /* platform word */
#endif //CMS_ALIGNMENT

#ifdef CMS_NEED_ALIGNMENT
//from nginx
#define cms_align(d, a)    (((d) + (a - 1)) & ~(a - 1))
// 对指针地址进行内存对齐，原理同上
#define cms_align_ptr(p, a)                                                   \
    (byte *) (((uint64) (p) + ((uint64) a - 1)) & ~((uint64) a - 1))
#else
#define cms_align(d, a)    (d)
#define cms_align_ptr(p, a)  (p)
#endif

#define CMS_VOID_PTR void*

#endif //__CMS_MEM_COMMON_H__

