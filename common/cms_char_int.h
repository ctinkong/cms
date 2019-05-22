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
#ifndef __CMS_CHAR_INT_H__
#define __CMS_CHAR_INT_H__

int bigInt16(char *b);
int bigInt24(char *b);
int bigInt32(char *b);
long long bigInt64(char *b);

unsigned int bigUInt16(char *b);
unsigned int bigUInt24(char *b);
unsigned int bigUInt32(char *b);
unsigned long long bigUInt64(char *b);

void bigPutInt16(char *b, short v);
void bigPutInt24(char *b, int v);
void bigPutInt32(char *b, int v);
void bigPutInt64(char *b, long long v);

int littleInt16(char *b);
int littleInt24(char *b);
int littleInt32(char *b);
long long littleInt64(char *b);
void littlePutInt16(char *b, short v);
void littlePutInt24(char *b, int v);
void littlePutInt32(char *b, int v);
void littlePutInt64(char *b, long long v);

#endif