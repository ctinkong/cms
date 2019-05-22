#ifndef __SRC_HANDSHAKE_H__
#define __SRC_HANDSHAKE_H__
/*
The MIT License (MIT)

Copyright (c) 2013 winlin

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

#ifndef SRS_CORE_COMPLEX_HANDSHKAE_HPP
#define SRS_CORE_COMPLEX_HANDSHKAE_HPP

/*
#include <srs_core_complex_handshake.hpp>
*/

/**
* rtmp complex handshake,
* @see also crtmp(crtmpserver) or librtmp,
* @see also: http://blog.csdn.net/win_lin/article/details/13006803
* @doc update the README.cmd
*/

#endif

/*
The MIT License (MIT)

Copyright (c) 2013 winlin

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


// the digest key generate size.
#define OpensslHashSize 512

/**
* 764bytes key结构
* random-data: (offset)bytes
* key-data: 128bytes
* random-data: (764-offset-128-4)bytes
* offset: 4bytes
*/
struct key_block
{
	// (offset)bytes
	char* random0;
	int random0_size;
	// 128bytes
	char key[128];
	// (764-offset-128-4)bytes
	char* random1;
	int random1_size;
	// 4bytes
	int offset;
};


/**
* 764bytes digest结构
* offset: 4bytes
* random-data: (offset)bytes
* digest-data: 32bytes
* random-data: (764-4-offset-32)bytes
*/
struct digest_block
{
	// 4bytes
	int offset;
	// (offset)bytes
	char* random0;
	int random0_size;
	// 32bytes
	char digest[32];
	// (764-4-offset-32)bytes
	char* random1;
	int random1_size;
};


/**
* the schema type.
*/
enum srs_schema_type {
	srs_schema0 = 0, // key-digest sequence
	srs_schema1 = 1, // digest-key sequence
	srs_schema_invalid = 2,
};



/**
* c1s1 schema0
* time: 4bytes
* version: 4bytes
* key: 764bytes
* digest: 764bytes
* c1s1 schema1
* time: 4bytes
* version: 4bytes
* digest: 764bytes
* key: 764bytes
*/
struct c1s1
{
	union block {
		key_block key;
		digest_block digest;
	};
	// 4bytes
	int time;
	// 4bytes
	int version;
	// 764bytes
	// if schema0, use key
	// if schema1, use digest
	block block0;
	// 764bytes
	// if schema0, use digest
	// if schema1, use key
	block block1;
	// the logic schema
	srs_schema_type schema;
	c1s1();
	virtual ~c1s1();
	/**
	* get the digest key.
	*/
	virtual char* get_digest();
	/**
	* copy to bytes.
	*/
	virtual void dump(char* _c1s1);
	/**
	* client: create and sign c1 by schema.
	* sign the c1, generate the digest.
	* calc_c1_digest(c1, schema) {
	* get c1s1-joined from c1 by specified schema
	* digest-data = HMACsha256(c1s1-joined, FPKey, 30)
	* return digest-data;
	* }
	* random fill 1536bytes c1 // also fill the c1-128bytes-key
	* time = time() // c1[0-3]
	* version = [0x80, 0x00, 0x07, 0x02] // c1[4-7]
	* schema = choose schema0 or schema1
	* digest-data = calc_c1_digest(c1, schema)
	* copy digest-data to c1
	*/
	virtual int c1_create(srs_schema_type _schema);
	/**
	* server: parse the c1s1, discovery the key and digest by schema.
	* use the c1_validate_digest() to valid the digest of c1.
	*/
	virtual int c1_parse(char* _c1s1, srs_schema_type _schema);
	/**
	* server: validate the parsed schema and c1s1
	*/
	virtual int c1_validate_digest(bool& is_valid);
	/**
	* server: create and sign the s1 from c1.
	*/
	virtual int s1_create(c1s1* c1);
private:
	virtual int calc_s1_digest(char*& digest);
	virtual int calc_c1_digest(char*& digest);
	virtual void destroy_blocks();
};

/**
* the c2s2 complex handshake structure.
* random-data: 1504bytes
* digest-data: 32bytes
*/
struct c2s2
{
	char random[1504];
	char digest[32];
	c2s2();
	virtual ~c2s2();
	/**
	* copy to bytes.
	*/
	virtual void dump(char* _c2s2);

	/**
	* create c2.
	* random fill c2s2 1536 bytes
	*
	* // client generate C2, or server valid C2
	* temp-key = HMACsha256(s1-digest, FPKey, 62)
	* c2-digest-data = HMACsha256(c2-random-data, temp-key, 32)
	*/
	virtual int c2_create(c1s1* s1);
	/**
	* create s2.
	* random fill c2s2 1536 bytes
	*
	* // server generate S2, or client valid S2
	* temp-key = HMACsha256(c1-digest, FMSKey, 68)
	* s2-digest-data = HMACsha256(s2-random-data, temp-key, 32)
	*/
	virtual int s2_create(c1s1* c1);
};


#endif

