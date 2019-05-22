
#ifndef __SHA_1_H__
#define __SHA_1_H__

#include <string>

using namespace std;



#if defined(BSD) || defined(WIN32)
typedef unsigned int unsigned int;
typedef int int;
typedef unsigned char unsigned char;
#endif

/*
 * If you do not have the ISO standard stdint.h header file, then you
 * must typdef the following:
 *    name              meaning
 *  unsigned int         unsigned 32 bit integer
 *  unsigned char          unsigned 8 bit integer (i.e., unsigned char)
 *  int    integer of >= 16 bits
 *
 */

#ifndef _SHA_enum_
#define _SHA_enum_
enum
{
	shaSuccess = 0,
	shaNull,            /* Null pointer parameter */
	shaInputTooLong,    /* input data too long */
	shaStateError       /* called Input after Result */
};
#endif
#define SHA1HashSize 20

/*
 *  This structure will hold context information for the SHA-1
 *  hashing operation
 */
typedef struct SHA1Context
{
	unsigned int Intermediate_Hash[SHA1HashSize / 4]; /* Message Digest  */

	unsigned int Length_Low;            /* Message length in bits      */
	unsigned int Length_High;           /* Message length in bits      */

							   /* Index into message block array   */
	int Message_Block_Index;
	unsigned char Message_Block[64];      /* 512-bit message blocks      */

	int Computed;               /* Is the digest computed?         */
	int Corrupted;             /* Is the message digest corrupted? */
} SHA1Context;

/*
 *  Function Prototypes
 */

int SHA1Reset(SHA1Context *);
int SHA1Input(SHA1Context *,
	const void *,
	unsigned int);
int SHA1Result(SHA1Context *,
	unsigned char Message_Digest[SHA1HashSize]);

class  CSHA1
{
public:
	CSHA1();
	CSHA1(const void*, int);
public:
	int reset();
	void read(void*);
	string read();
	void write(const void*, int);
private:
	SHA1Context m_context;
};


#endif

