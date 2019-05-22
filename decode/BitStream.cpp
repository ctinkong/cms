#include "BitStream.h"
#include <math.h>

CBitStream::CBitStream(void)
{
}

CBitStream::~CBitStream(void)
{
}

void CBitStream::InitBits(uint8_t *buf, int buf_size)
{
	m_error = 0;
	m_index = 0;
	m_buffer = buf;
	m_bufferSize = buf_size;
}

uint32_t CBitStream::GetBits(int n)
{
	uint32_t ret = 0;
	if (GetBitsLeft() < n) {
		m_error = -1;
		goto end;
	}

	for (int i = 0; i < n; i++) {
		ret <<= 1;
		if (m_buffer[m_index / 8] & (0x80 >> (m_index % 8)))
			ret += 1;

		m_index++;
	}

end:
	return ret;
}

uint32_t CBitStream::PeekBits(int n)
{
	uint32_t ret = 0;
	int index = m_index;
	if (GetBitsLeft() < n) {
		m_error = -1;
		goto end;
	}

	for (int i = 0; i < n; i++) {
		ret <<= 1;
		if (m_buffer[index / 8] & (0x80 >> (index % 8)))
			ret += 1;

		index++;
	}

end:
	return ret;
}

void CBitStream::SkipBits(int n)
{
	if (n > GetBitsLeft())
		n = GetBitsLeft();

	m_index += n;
}

uint8_t* CBitStream::GetBuffer()
{
	return m_buffer + m_index / 8;
}

int CBitStream::GetBitsLeft()
{
	return m_bufferSize * 8 - m_index;
}

uint32_t CBitStream::GetUe()
{
	int nZeroNum = 0;
	while (GetBitsLeft() > 0) {
		if (m_buffer[m_index / 8] & (0x80 >> (m_index % 8)))
			break;

		nZeroNum++;
		m_index++;
	}
	m_index++;

	int ret = 0;
	for (int i = 0; i < nZeroNum; i++) {
		ret <<= 1;
		if (GetBitsLeft() > 0) {
			if (m_buffer[m_index / 8] & (0x80 >> (m_index % 8)))
				ret += 1;
		}
		else {
			m_error = -1;
			goto end;
		}

		m_index++;
	}

end:
	return (1 << nZeroNum) - 1 + ret;
}

int CBitStream::GetSe()
{
	int UeVal = GetUe();
	double k = UeVal;
	int nValue = ceil(k / 2);
	if (UeVal % 2 == 0)
		nValue = -nValue;

	return nValue;
}


