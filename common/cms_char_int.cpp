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
#include <common/cms_char_int.h>

int bigInt16(char *b)
{
	int i16 = 0;
	char *p = (char *)&i16;
	p[1] = *b++;
	p[0] = *b++;
	p[2] = 0;
	p[3] = 0;
	return i16;
	//return int(b[1]) | int(b[0])<<8;
}

int bigInt24(char *b)
{
	int i24 = 0;
	char *p = (char *)&i24;
	p[2] = *b++;
	p[1] = *b++;
	p[0] = *b++;
	p[3] = 0;
	return i24;
	//return int(b[2]) | int(b[1])<<8 | int(b[0])<<16;
}

int bigInt32(char *b)
{
	int i32 = 0;
	char *p = (char *)&i32;
	p[3] = *b++;
	p[2] = *b++;
	p[1] = *b++;
	p[0] = *b++;
	return i32;
	//return int(b[3]) | int(b[2])<<8 | int(b[1])<<16 | int(b[0])<<24;
}

long long bigInt64(char *b)
{
	long long i64 = 0;
	char *p = (char *)&i64;
	p[7] = *b++;
	p[6] = *b++;
	p[5] = *b++;
	p[4] = *b++;
	p[3] = *b++;
	p[2] = *b++;
	p[1] = *b++;
	p[0] = *b++;
	return i64;
	//return (long long)(b[7]) | (long long)(b[6])<<8 | (long long)(b[5])<<16 | (long long)(b[4])<<24 | 
	//	(long long)(b[3])<<32 | (long long)(b[2])<<40 | (long long)(b[1])<<48 | (long long)(b[0])<<56;
}

unsigned int bigUInt16(char *b)
{
	return (unsigned int)b[1] | ((unsigned int)(b[0])) << 8;
}

unsigned int bigUInt24(char *b)
{
	return (unsigned int)(b[2]) | ((unsigned int)(b[1])) << 8 | ((unsigned int)(b[0])) << 16;
}

unsigned int bigUInt32(char *b)
{
	return (unsigned int)(b[3]) | ((unsigned int)(b[2])) << 8 | ((unsigned int)(b[1])) << 16 | ((unsigned int)(b[0])) << 24;
}

unsigned long long bigUInt64(char *b)
{
	return (unsigned long long)(b[7]) | (unsigned long long)(b[6]) << 8 | (unsigned long long)(b[5]) << 16 | (unsigned long long)(b[4]) << 24 |
		(unsigned long long)(b[3]) << 32 | (unsigned long long)(b[2]) << 40 | (unsigned long long)(b[1]) << 48 | (unsigned long long)(b[0]) << 56;
}

void bigPutInt16(char *b, short v)
{
	b[0] = char(v >> 8);
	b[1] = char(v);
}

void bigPutInt24(char *b, int v)
{
	b[0] = char(v >> 16);
	b[1] = char(v >> 8);
	b[2] = char(v);
}

void bigPutInt32(char *b, int v)
{
	b[0] = char(v >> 24);
	b[1] = char(v >> 16);
	b[2] = char(v >> 8);
	b[3] = char(v);
}

void bigPutInt64(char *b, long long v)
{
	b[0] = char(v >> 56);
	b[1] = char(v >> 48);
	b[2] = char(v >> 40);
	b[3] = char(v >> 32);
	b[4] = char(v >> 24);
	b[5] = char(v >> 16);
	b[6] = char(v >> 8);
	b[7] = char(v);
}

int littleInt16(char *b)
{
	return int(b[0]) | int(b[1]) << 8;
}

int littleInt24(char *b)
{
	return int(b[0]) | int(b[1]) << 8 | int(b[2]) << 16;
}

int littleInt32(char *b)
{
	return int(b[0]) | int(b[1]) << 8 | int(b[2]) << 16 | int(b[3]) << 24;
}

long long littleInt64(char *b)
{
	return (long long)(b[0]) | (long long)(b[1]) << 8 | (long long)(b[2]) << 16 | (long long)(b[3]) << 24 |
		(long long)(b[4]) << 32 | (long long)(b[5]) << 40 | (long long)(b[6]) << 48 | (long long)(b[7]) << 56;
}

void littlePutInt16(char *b, short v)
{
	b[0] = char(v);
	b[1] = char(v >> 8);
}

void littlePutInt24(char *b, int v)
{
	b[0] = char(v);
	b[1] = char(v >> 8);
	b[2] = char(v >> 16);
}

void littlePutInt32(char *b, int v)
{
	b[0] = char(v);
	b[1] = char(v >> 8);
	b[2] = char(v >> 16);
	b[3] = char(v >> 24);
}

void littlePutInt64(char *b, long long v)
{
	b[0] = char(v);
	b[1] = char(v >> 8);
	b[2] = char(v >> 16);
	b[3] = char(v >> 24);
	b[4] = char(v >> 32);
	b[5] = char(v >> 40);
	b[6] = char(v >> 48);
	b[7] = char(v >> 56);
}
