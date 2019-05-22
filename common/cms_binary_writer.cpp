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
#include <common/cms_binary_writer.h>
#include <cstring>

BinaryWriter::BinaryWriter()
{
	m_pHead = (char*)malloc(1024);
	m_pPos = m_pHead;
	m_iBufSize = 1024;
	m_iLen = 0;
}


BinaryWriter::~BinaryWriter()
{
	if (m_pHead != NULL)
	{
		free(m_pHead);
	}
}

void BinaryWriter::rrealloc(const char* pBuf, int iSize)
{
	while (m_iLen + iSize > m_iBufSize)
	{
		m_iBufSize += 1024;
		m_pHead = (char*)::realloc(m_pHead, m_iBufSize);
	}
	m_pPos = m_pHead + m_iLen;
	memcpy(m_pPos, pBuf, iSize);
	m_pPos += iSize;
	m_iLen += iSize;
}

BinaryWriter& BinaryWriter::operator << (bool value)
{
	rrealloc((const char*)&value, sizeof(value));
	return *this;
}


BinaryWriter& BinaryWriter::operator << (char value)
{
	rrealloc((const char*)&value, sizeof(value));
	return *this;
}


BinaryWriter& BinaryWriter::operator << (unsigned char value)
{
	rrealloc((const char*)&value, sizeof(value));
	return *this;
}


BinaryWriter& BinaryWriter::operator << (signed char value)
{
	rrealloc((const char*)&value, sizeof(value));
	return *this;
}


BinaryWriter& BinaryWriter::operator << (short value)
{
	rrealloc((const char*)&value, sizeof(value));
	return *this;
}


BinaryWriter& BinaryWriter::operator << (unsigned short value)
{
	rrealloc((const char*)&value, sizeof(value));
	return *this;
}


BinaryWriter& BinaryWriter::operator << (int value)
{
	rrealloc((const char*)&value, sizeof(value));
	return *this;
}


BinaryWriter& BinaryWriter::operator << (unsigned int value)
{
	rrealloc((const char*)&value, sizeof(value));
	return *this;
}


BinaryWriter& BinaryWriter::operator << (long value)
{
	rrealloc((const char*)&value, sizeof(value));
	return *this;
}


BinaryWriter& BinaryWriter::operator << (unsigned long value)
{
	rrealloc((const char*)&value, sizeof(value));
	return *this;
}


BinaryWriter& BinaryWriter::operator << (float value)
{
	rrealloc((const char*)&value, sizeof(value));
	return *this;
}


BinaryWriter& BinaryWriter::operator << (double value)
{
	rrealloc((const char*)&value, sizeof(value));
	return *this;
}

BinaryWriter& BinaryWriter::operator << (long long value)
{
	rrealloc((const char *)&value, sizeof(value));
	return *this;
}

BinaryWriter& BinaryWriter::operator << (const std::string& value)
{
	int length = (int)value.size();
	rrealloc(value.data(), length + 1);
	return *this;
}


BinaryWriter& BinaryWriter::operator << (const char* value)
{
	int length = (int)strlen(value);
	rrealloc(value, length + 1);
	return *this;
}




void BinaryWriter::writeRaw(const char* value, int length)
{
	rrealloc(value, length);
}



