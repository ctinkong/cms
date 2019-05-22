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
#ifndef BinaryWriter_INCLUDED
#define BinaryWriter_INCLUDED

#include <ostream>
#include <stdlib.h>
#include <string.h>
class  BinaryWriter
{
public:

	BinaryWriter();

	~BinaryWriter();

	BinaryWriter& operator << (bool value);
	BinaryWriter& operator << (char value);
	BinaryWriter& operator << (unsigned char value);
	BinaryWriter& operator << (signed char value);
	BinaryWriter& operator << (short value);
	BinaryWriter& operator << (unsigned short value);
	BinaryWriter& operator << (int value);
	BinaryWriter& operator << (unsigned int value);
	BinaryWriter& operator << (long value);
	BinaryWriter& operator << (unsigned long value);
	BinaryWriter& operator << (float value);
	BinaryWriter& operator << (double value);
	BinaryWriter& operator << (long long value);


	BinaryWriter& operator << (const std::string& value);
	BinaryWriter& operator << (const char* value);

	void writeRaw(const char* value, int length);
	void rrealloc(const char* pBuf, int iSize);

	char* getData()
	{
		return m_pHead;
	}

	void reset()
	{
		m_iLen = 0;
		m_pPos = m_pHead;
	}

	int getLength()
	{
		return m_iLen;
	}

private:
	char* m_pHead;
	char* m_pPos;
	int   m_iBufSize;
	int   m_iLen;
};




#endif 
