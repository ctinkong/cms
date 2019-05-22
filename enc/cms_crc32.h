#ifndef __BASELIB_CRC32_H__
#define __BASELIB_CRC32_H__
#include <stdint.h>


unsigned long CRC32(unsigned char *block, unsigned int length);
unsigned long crc32n(unsigned char *block, unsigned int length);
#endif

