#include "ParseSequenceHeader.h"
#include <string.h>

CParseSequenceHeader::CParseSequenceHeader(void)
{
	m_width = m_height = 0;
	m_sps = NULL;
	m_spsSize = 0;
	m_fps = 0;
}

CParseSequenceHeader::~CParseSequenceHeader(void)
{
	if (m_sps)
		delete[] m_sps;
}

int CParseSequenceHeader::H2645ExtractRbsp(const uint8_t *src, int length, uint8_t *dst)
{
	int i, si, di;

	for (i = 0; i + 1 < length; i += 5) {
		if (!((~FPS_RN32A(src + i) &
			(FPS_RN32A(src + i) - 0x01000101U)) &
			0x80008080U))
			continue;

		if (i > 0 && !src[i])
			i--;
		while (src[i])
			i++;

		if (i + 2 < length && src[i + 1] == 0 && src[i + 2] <= 3) {
			if (src[i + 2] != 3) {
				length = i;
			}
			break;
		}
		i -= 3;
	}

	memcpy(dst, src, i);
	si = di = i;
	while (si + 2 < length) {
		if (src[si + 2] > 3) {
			dst[di++] = src[si++];
			dst[di++] = src[si++];
		}
		else if (src[si] == 0 && src[si + 1] == 0) {
			if (src[si + 2] == 3) {
				dst[di++] = 0;
				dst[di++] = 0;
				si += 3;
				continue;
			}
			else
				goto nsc;
		}

		dst[di++] = src[si++];
	}
	while (si < length)
		dst[di++] = src[si++];

nsc:
	m_spsSize = di;
	return di;
}

int CParseSequenceHeader::SplitPacket(const uint8_t *buf, int buf_size)
{
	int buf_index = 0;
	int next_avc = 0;
	for (;;) {
		int dst_length;
		int bit_length;
		int i, nalsize = 0;

		if (buf_index >= next_avc) {
			if (buf_index >= buf_size - 2)
				break;
			nalsize = 0;
			for (i = 0; i < 2; i++)
				nalsize = (nalsize << 8) | buf[buf_index++];
			if (nalsize <= 0 || nalsize > buf_size - buf_index) {
				break;
			}
			next_avc = buf_index + nalsize;
		}

		int length = next_avc - buf_index;
		if (m_sps)
			delete[] m_sps;
		m_sps = new uint8_t[length + 16];
		if (!m_sps)
			return -1;

		memset(m_sps, 0, length + 16);
		return H2645ExtractRbsp(buf + buf_index, length, m_sps);
	}

	return 0;
}


