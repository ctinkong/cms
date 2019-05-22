#include <enc/cms_crc32n.h>

// CRC code
#define DO1(buf) crc = crc32_table_n[((unsigned int)crc ^ (*buf++)) & 0xff] ^ ((unsigned int)crc >> 8);
#define DO2(buf)  DO1(buf); DO1(buf);
#define DO4(buf)  DO2(buf); DO2(buf);
#define DO8(buf)  DO4(buf); DO4(buf);

//可追加的CRC32校验，crc-->原有的crc校验值，buf-->待追加校验的缓冲，len-->缓冲长度
unsigned int crc32(unsigned int crc, const unsigned char* buf, int len)
{
	if (buf == 0 || len <= 0) return crc;
	crc = crc ^ 0xffffffff;
	while (len >= 8)
	{
		DO8(buf);
		len -= 8;
	}
	if (len) do {
		DO1(buf);
	} while (--len);
	return crc ^ 0xffffffff;
}

void Crc32Pack(unsigned char * buf, int len, unsigned int* crcCode)
{
	*crcCode = crc32(0, buf, len);

	return;
};

bool Crc32UnPack(unsigned char * buf, int len, unsigned int* oldCrcCode, unsigned int* newCrcCode)
{
	unsigned int crcCode = crc32(0, buf, len);
	if (*oldCrcCode != crcCode)
	{
		*newCrcCode = crcCode;
		return false;
	}

	return true;
};
