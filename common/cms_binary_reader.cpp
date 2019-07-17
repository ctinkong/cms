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
#include <common/cms_binary_reader.h>
#include <mem/cms_mf_mem.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>

BinaryReader::BinaryReader(int sock)
{
	m_bError = false;
	m_iLen = 0;
	int iCurrLen = 0;
	m_pHead = (char*)xmalloc(1024);

	while (iCurrLen < m_iLen || m_iLen == 0)
	{
		int iRecv = ::recv(sock, m_pHead + iCurrLen, 1024, 0);
		if (iRecv != -1)
		{
			if (iCurrLen == 0)
				m_iLen = ntohl(*(int*)m_pHead);
			//修正m_iLen过大的错误
			if (m_iLen > 1024)
				break;
			iCurrLen += iRecv;
			if (iCurrLen < m_iLen)
				m_pHead = (char*)xrealloc(m_pHead, iCurrLen + 1024);
			else
				break;
		}
		else
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				continue;
			else
			{
				m_bError = true;
				break;
			}
		}
	}
	m_pPos = m_pHead;
}


BinaryReader::~BinaryReader()
{
	xfree(m_pHead);
}


BinaryReader& BinaryReader::operator >> (bool& value)
{
	if (!m_bError)
	{
		memcpy((char*)&value, m_pPos, sizeof(value));
		m_pPos += sizeof(value);
	}
	return *this;
}


BinaryReader& BinaryReader::operator >> (char& value)
{
	if (!m_bError)
	{
		memcpy((char*)&value, m_pPos, sizeof(value));
		m_pPos += sizeof(value);
	}
	return *this;
}


BinaryReader& BinaryReader::operator >> (unsigned char& value)
{
	if (!m_bError)
	{
		memcpy((char*)&value, m_pPos, sizeof(value));
		m_pPos += sizeof(value);
	}
	return *this;
}


BinaryReader& BinaryReader::operator >> (signed char& value)
{
	if (!m_bError)
	{
		memcpy((char*)&value, m_pPos, sizeof(value));
		m_pPos += sizeof(value);
	}
	return *this;
}


BinaryReader& BinaryReader::operator >> (short& value)
{
	if (!m_bError)
	{
		memcpy((char*)&value, m_pPos, sizeof(value));
		m_pPos += sizeof(value);
	}
	return *this;
}


BinaryReader& BinaryReader::operator >> (unsigned short& value)
{
	if (!m_bError)
	{
		memcpy((char*)&value, m_pPos, sizeof(value));
		m_pPos += sizeof(value);
	}
	return *this;
}


BinaryReader& BinaryReader::operator >> (int& value)
{
	if (!m_bError)
	{
		memcpy((char*)&value, m_pPos, sizeof(value));
		m_pPos += sizeof(value);
	}
	return *this;
}


BinaryReader& BinaryReader::operator >> (unsigned int& value)
{
	if (!m_bError)
	{
		memcpy((char*)&value, m_pPos, sizeof(value));
		m_pPos += sizeof(value);
	}
	return *this;
}


BinaryReader& BinaryReader::operator >> (long& value)
{
	if (!m_bError)
	{
		memcpy((char*)&value, m_pPos, sizeof(value));
		m_pPos += sizeof(value);
	}
	return *this;
}


BinaryReader& BinaryReader::operator >> (unsigned long& value)
{
	if (!m_bError)
	{
		memcpy((char*)&value, m_pPos, sizeof(value));
		m_pPos += sizeof(value);
	}
	return *this;
}


BinaryReader& BinaryReader::operator >> (float& value)
{
	if (!m_bError)
	{
		memcpy((char*)&value, m_pPos, sizeof(value));
		m_pPos += sizeof(value);
	}
	return *this;
}


BinaryReader& BinaryReader::operator >> (double& value)
{
	if (!m_bError)
	{
		memcpy((char*)&value, m_pPos, sizeof(value));
		m_pPos += sizeof(value);
	}
	return *this;
}

BinaryReader& BinaryReader::operator >> (long long& value)
{
	if (!m_bError)
	{
		memcpy((char*)&value, m_pPos, sizeof(value));
		m_pPos += sizeof(value);
	}
	return *this;
}

BinaryReader& BinaryReader::operator >> (std::string& value)
{
	if (!m_bError)
	{
		while (*m_pPos != '\0')
		{
			value += *m_pPos;
			m_pPos++;
		}
		m_pPos++;
	}
	return *this;
}



void BinaryReader::readRaw(char* value, int length)
{
	if (!m_bError)
	{
		memcpy(value, m_pPos, length);
		m_pPos += length;
	}
}
