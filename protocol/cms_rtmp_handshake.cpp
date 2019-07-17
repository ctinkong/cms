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
#include <arpa/inet.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/time.h>
#include <protocol/cms_rtmp_handshake.h>
#include <mem/cms_mf_mem.h>
#include <core/cms_lock.h>
#include <log/cms_log.h>

// 68bytes FMS key which is used to sign the sever packet.
unsigned char SrsGenuineFMSKey[] = {
	0x47, 0x65, 0x6e, 0x75, 0x69, 0x6e, 0x65, 0x20,
	0x41, 0x64, 0x6f, 0x62, 0x65, 0x20, 0x46, 0x6c,
	0x61, 0x73, 0x68, 0x20, 0x4d, 0x65, 0x64, 0x69,
	0x61, 0x20, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72,
	0x20, 0x30, 0x30, 0x31, // Genuine Adobe Flash Media Server 001
	0xf0, 0xee, 0xc2, 0x4a, 0x80, 0x68, 0xbe, 0xe8,
	0x2e, 0x00, 0xd0, 0xd1, 0x02, 0x9e, 0x7e, 0x57,
	0x6e, 0xec, 0x5d, 0x2d, 0x29, 0x80, 0x6f, 0xab,
	0x93, 0xb8, 0xe6, 0x36, 0xcf, 0xeb, 0x31, 0xae
}; // 68

// 62bytes FP key which is used to sign the client packet.
unsigned char SrsGenuineFPKey[] = {
	0x47, 0x65, 0x6E, 0x75, 0x69, 0x6E, 0x65, 0x20,
	0x41, 0x64, 0x6F, 0x62, 0x65, 0x20, 0x46, 0x6C,
	0x61, 0x73, 0x68, 0x20, 0x50, 0x6C, 0x61, 0x79,
	0x65, 0x72, 0x20, 0x30, 0x30, 0x31, // Genuine Adobe Flash Player 001
	0xF0, 0xEE, 0xC2, 0x4A, 0x80, 0x68, 0xBE, 0xE8,
	0x2E, 0x00, 0xD0, 0xD1, 0x02, 0x9E, 0x7E, 0x57,
	0x6E, 0xEC, 0x5D, 0x2D, 0x29, 0x80, 0x6F, 0xAB,
	0x93, 0xB8, 0xE6, 0x36, 0xCF, 0xEB, 0x31, 0xAE
}; // 62

#define cms_rtmp_server_version  0x0D0E0A0D
#define cms_rtmp_client_version  0x0C000D0E


CLock g_rtmpHandShakeLock;

#include <openssl/evp.h>
#include <openssl/hmac.h>
int openssl_HMACsha256(const void* data, int data_size, const void* key, int key_size, void* digest) {
	CAutoLock lock(g_rtmpHandShakeLock);
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	HMAC_CTX *ctx;
	ctx = HMAC_CTX_new();
	HMAC_Init_ex(ctx, (unsigned char*)key, key_size, EVP_sha256(), NULL);
	HMAC_Update(ctx, (unsigned char *)data, data_size);

	unsigned int digest_size;
	HMAC_Final(ctx, (unsigned char *)digest, &digest_size);
	HMAC_CTX_free(ctx);
	if (digest_size != 32) {
		return -1;
	}
	return 0;
#else
	HMAC_CTX ctx;
	HMAC_CTX_init(&ctx);
	HMAC_Init_ex(&ctx, (unsigned char*)key, key_size, EVP_sha256(), NULL);
	HMAC_Update(&ctx, (unsigned char *)data, data_size);

	unsigned int digest_size;
	HMAC_Final(&ctx, (unsigned char *)digest, &digest_size);
	HMAC_CTX_cleanup(&ctx);
	if (digest_size != 32) {
		return -1;
	}
	return 0;
#endif	
}

#include <openssl/dh.h>
#define RFC2409_PRIME_1024 \
	"FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD1" \
	"29024E088A67CC74020BBEA63B139B22514A08798E3404DD" \
	"EF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245" \
	"E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED" \
	"EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE65381" \
	"FFFFFFFFFFFFFFFF"
int __openssl_generate_key(
	unsigned char*& _private_key, unsigned char*& _public_key, int& size,
	DH*& pdh, int& bits_count, unsigned char*& shared_key, int& shared_key_length, BIGNUM*& peer_public_key
) {
	CAutoLock lock(g_rtmpHandShakeLock);
	int ret = 0;

	//1. Create the DH
	if ((pdh = DH_new()) == NULL) {
		ret = -1;
		return ret;
	}
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	//2. Create his internal p and g
	BIGNUM* p = BN_new();
	if (p == NULL)
	{
		ret = -1;
		return ret;
	}
	BIGNUM* g = BN_new();
	if (g == NULL)
	{
		ret = -1;
		return ret;
	}
	//3. initialize p, g and key length
	if (BN_hex2bn(&p, RFC2409_PRIME_1024) == 0) {
		ret = -1;
		return ret;
	}
	if (BN_set_word(g, 2) != 1) {
		ret = -1;
		return ret;
	}
	DH_set0_pqg(pdh, p, NULL, g);
#else
	//2. Create his internal p and g
	if ((pdh->p = BN_new()) == NULL) {
		ret = -1;
		return ret;
	}
	if ((pdh->g = BN_new()) == NULL) {
		ret = -1;
		return ret;
	}

	//3. initialize p, g and key length
	if (BN_hex2bn(&pdh->p, RFC2409_PRIME_1024) == 0) {
		ret = -1;
		return ret;
	}
	if (BN_set_word(pdh->g, 2) != 1) {
		ret = -1;
		return ret;
	}

	//4. Set the key length
	pdh->length = bits_count;
#endif
	//5. Generate private and public key
	if (DH_generate_key(pdh) != 1) {
		ret = -1;
		return ret;
	}

	// CreateSharedKey
	if (pdh == NULL) {
		ret = -1;
		return ret;
	}

	if (shared_key_length != 0 || shared_key != NULL) {
		ret = -1;
		return ret;
	}

	shared_key_length = DH_size(pdh);
	if (shared_key_length <= 0 || shared_key_length > 1024) {
		ret = -1;
		return ret;
	}
	shared_key = (unsigned char *)xmalloc(shared_key_length);
	memset(shared_key, 0, shared_key_length);

	peer_public_key = BN_bin2bn(_private_key, size, 0);
	if (peer_public_key == NULL) {
		ret = -1;
		return ret;
	}

	if (DH_compute_key(shared_key, peer_public_key, pdh) == -1) {
		ret = -1;
		return ret;
	}

	// CopyPublicKey
	if (pdh == NULL) {
		ret = -1;
		return ret;
	}
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	const BIGNUM *pub_key = DH_get0_priv_key(pdh);
	int keySize = BN_num_bytes(pub_key);
	if ((keySize <= 0) || (size <= 0) || (keySize > size)) {
		//("CopyPublicKey failed due to either invalid DH state or invalid call"); return ret;
		ret = -1;
		return ret;
	}

	if (BN_bn2bin(pub_key, _public_key) != keySize) {
		//("Unable to copy key"); return ret;
		ret = -1;
		return ret;
	}
#else
	int keySize = BN_num_bytes(pdh->pub_key);
	if ((keySize <= 0) || (size <= 0) || (keySize > size)) {
		//("CopyPublicKey failed due to either invalid DH state or invalid call"); return ret;
		ret = -1;
		return ret;
	}

	if (BN_bn2bin(pdh->pub_key, _public_key) != keySize) {
		//("Unable to copy key"); return ret;
		ret = -1;
		return ret;
	}
#endif		
	return ret;
}
int openssl_generate_key(char* _private_key, char* _public_key, int size)
{
	CAutoLock lock(g_rtmpHandShakeLock);
	int ret = 0;

	// Initialize
	DH* pdh = NULL;
	int bits_count = 1024;
	unsigned char* shared_key = NULL;
	int shared_key_length = 0;
	BIGNUM* peer_public_key = NULL;
	unsigned char *u_private_key = (unsigned char *)_private_key;
	unsigned char *u_public_key = (unsigned char *)_public_key;
	ret = __openssl_generate_key(
		(unsigned char*&)u_private_key, (unsigned char*&)u_public_key, size,
		pdh, bits_count, shared_key, shared_key_length, peer_public_key
	);
	if (pdh != NULL) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
		if (pdh->p != NULL) {
			BN_free(pdh->p);
			pdh->p = NULL;
		}
		if (pdh->g != NULL) {
			BN_free(pdh->g);
			pdh->g = NULL;
		}
#endif		
		DH_free(pdh);
		pdh = NULL;
	}

	if (shared_key != NULL) {
		xfree(shared_key);
		shared_key = NULL;
	}

	if (peer_public_key != NULL) {
		BN_free(peer_public_key);
		peer_public_key = NULL;
	}

	return ret;
}

// calc the offset of key,
// the key->offset cannot be used as the offset of key.
int srs_key_block_get_offset(key_block* key)
{
	int max_offset_size = 764 - 128 - 4;
	int offset = 0;
	unsigned char* pp = (unsigned char*)&key->offset;
	offset += *pp++;
	offset += *pp++;
	offset += *pp++;
	offset += *pp++;

	return offset % max_offset_size;
}
// create new key block data.
// if created, user must free it by srs_key_block_free
void srs_key_block_init(key_block* key)
{
	key->offset = (int)rand();
	key->random0 = NULL;
	key->random1 = NULL;
	int offset = srs_key_block_get_offset(key);
	assert(offset >= 0);
	key->random0_size = offset;
	if (key->random0_size > 0) {
		key->random0 = (char*)xmalloc(key->random0_size);
		for (int i = 0; i < key->random0_size; i++) {
			*(key->random0 + i) = rand() % 256;
		}
	}
	for (int i = 0; i < (int)sizeof(key->key); i++) {
		*(key->key + i) = rand() % 256;
	}
	key->random1_size = 764 - offset - 128 - 4;
	if (key->random1_size > 0) {
		key->random1 = (char*)xmalloc(key->random1_size);
		for (int i = 0; i < key->random1_size; i++) {
			*(key->random1 + i) = rand() % 256;
		}
	}
}
// parse key block from c1s1.
// if created, user must free it by srs_key_block_free
// @c1s1_key_bytes the key start bytes, maybe c1s1 or c1s1+764
int srs_key_block_parse(key_block* key, char* c1s1_key_bytes)
{
	int ret = 0;

	char* pp = c1s1_key_bytes + 764;
	pp -= sizeof(int);
	key->offset = *(int*)pp;
	key->random0 = NULL;
	key->random1 = NULL;
	int offset = srs_key_block_get_offset(key);
	assert(offset >= 0);
	pp = c1s1_key_bytes;
	key->random0_size = offset;
	if (key->random0_size > 0) {
		key->random0 = (char*)xmalloc(key->random0_size);
		memcpy(key->random0, pp, key->random0_size);
	}
	pp += key->random0_size;
	memcpy(key->key, pp, sizeof(key->key));
	pp += sizeof(key->key);
	key->random1_size = 764 - offset - 128 - 4;
	if (key->random1_size > 0) {
		key->random1 = (char*)xmalloc(key->random1_size);
		memcpy(key->random1, pp, key->random1_size);
	}
	return ret;
}
// free the block data create by
// srs_key_block_init or srs_key_block_parse
void srs_key_block_free(key_block* key)
{
	if (key->random0) {
		xfree(key->random0);
	}
	if (key->random1) {
		xfree(key->random1);
	}
}

// calc the offset of digest,
// the key->offset cannot be used as the offset of digest.
int srs_digest_block_get_offset(digest_block* digest)
{
	int max_offset_size = 764 - 32 - 4;
	int offset = 0;
	unsigned char* pp = (unsigned char*)&digest->offset;
	offset += *pp++;
	offset += *pp++;
	offset += *pp++;
	offset += *pp++;

	return offset % max_offset_size;
}
// create new digest block data.
// if created, user must free it by srs_digest_block_free
void srs_digest_block_init(digest_block* digest)
{
	digest->offset = (int)rand();
	digest->random0 = NULL;
	digest->random1 = NULL;
	int offset = srs_digest_block_get_offset(digest);
	assert(offset >= 0);
	digest->random0_size = offset;
	if (digest->random0_size > 0) {
		digest->random0 = (char*)xmalloc(digest->random0_size);
		for (int i = 0; i < digest->random0_size; i++) {
			*(digest->random0 + i) = rand() % 256;
		}
	}
	for (int i = 0; i < (int)sizeof(digest->digest); i++) {
		*(digest->digest + i) = rand() % 256;
	}
	digest->random1_size = 764 - 4 - offset - 32;
	if (digest->random1_size > 0) {
		digest->random1 = (char*)xmalloc(digest->random1_size);
		for (int i = 0; i < digest->random1_size; i++) {
			*(digest->random1 + i) = rand() % 256;
		}
	}
}
// parse digest block from c1s1.
// if created, user must free it by srs_digest_block_free
// @c1s1_digest_bytes the digest start bytes, maybe c1s1 or c1s1+764
int srs_digest_block_parse(digest_block* digest, char* c1s1_digest_bytes)
{
	int ret = 0;

	char* pp = c1s1_digest_bytes;
	digest->offset = *(int*)pp;
	pp += sizeof(int);
	digest->random0 = NULL;
	digest->random1 = NULL;
	int offset = srs_digest_block_get_offset(digest);
	assert(offset >= 0);
	digest->random0_size = offset;
	if (digest->random0_size > 0) {
		digest->random0 = (char*)xmalloc(digest->random0_size);
		memcpy(digest->random0, pp, digest->random0_size);
	}
	pp += digest->random0_size;
	memcpy(digest->digest, pp, sizeof(digest->digest));
	pp += sizeof(digest->digest);
	digest->random1_size = 764 - 4 - offset - 32;
	if (digest->random1_size > 0) {
		digest->random1 = (char*)xmalloc(digest->random1_size);
		memcpy(digest->random1, pp, digest->random1_size);
	}
	return ret;
}
// free the block data create by
// srs_digest_block_init or srs_digest_block_parse
void srs_digest_block_free(digest_block* digest)
{
	if (digest->random0) {
		xfree(digest->random0);
	}
	if (digest->random1) {
		xfree(digest->random1);
	}
}

void __srs_time_copy_to(char*& pp, int time)
{
	// 4bytes time
	*(int*)pp = time;
	pp += 4;
}
void __srs_version_copy_to(char*& pp, int version)
{
	// 4bytes version
	*(int*)pp = version;
	pp += 4;
}
void __srs_key_copy_to(char*& pp, key_block* key)
{
	// 764bytes key block
	if (key->random0_size > 0) {
		memcpy(pp, key->random0, key->random0_size);
	}
	pp += key->random0_size;
	memcpy(pp, key->key, sizeof(key->key));
	pp += sizeof(key->key);
	if (key->random1_size > 0) {
		memcpy(pp, key->random1, key->random1_size);
	}
	pp += key->random1_size;
	*(int*)pp = key->offset;
	pp += 4;
}
void __srs_digest_copy_to(char*& pp, digest_block* digest, bool with_digest)
{
	// 732bytes digest block without the 32bytes digest-data
	// nbytes digest block part1
	*(int*)pp = digest->offset;
	pp += 4;
	if (digest->random0_size > 0) {
		memcpy(pp, digest->random0, digest->random0_size);
	}
	pp += digest->random0_size;
	// digest
	if (with_digest) {
		memcpy(pp, digest->digest, 32);
		pp += 32;
	}
	// nbytes digest block part2
	if (digest->random1_size > 0) {
		memcpy(pp, digest->random1, digest->random1_size);
	}
	pp += digest->random1_size;
}

/**
* copy whole c1s1 to bytes.
*/
void srs_schema0_copy_to(char* bytes, bool with_digest,
	int time, int version, key_block* key, digest_block* digest)
{
	char* pp = bytes;

	__srs_time_copy_to(pp, time);
	__srs_version_copy_to(pp, version);
	__srs_key_copy_to(pp, key);
	__srs_digest_copy_to(pp, digest, with_digest);
	if (with_digest) {
		assert(pp - bytes == 1536);
	}
	else {
		assert(pp - bytes == 1536 - 32);
	}
}
void srs_schema1_copy_to(char* bytes, bool with_digest,
	int time, int version, digest_block* digest, key_block* key)
{
	char* pp = bytes;

	__srs_time_copy_to(pp, time);
	__srs_version_copy_to(pp, version);
	__srs_digest_copy_to(pp, digest, with_digest);
	__srs_key_copy_to(pp, key);
	if (with_digest) {
		assert(pp - bytes == 1536);
	}
	else {
		assert(pp - bytes == 1536 - 32);
	}
}
/**
* c1s1 is splited by digest:
* c1s1-part1: n bytes (time, version, key and digest-part1).
* digest-data: 32bytes
* c1s1-part2: (1536-n-32)bytes (digest-part2)
*/
char* srs_bytes_join_schema0(int time, int version, key_block* key, digest_block* digest)
{
	char* bytes = (char*)xmalloc(1536 - 32);
	srs_schema0_copy_to(bytes, false, time, version, key, digest);
	return bytes;
}
/**
* c1s1 is splited by digest:
* c1s1-part1: n bytes (time, version and digest-part1).
* digest-data: 32bytes
* c1s1-part2: (1536-n-32)bytes (digest-part2 and key)
*/
char* srs_bytes_join_schema1(int time, int version, digest_block* digest, key_block* key)
{
	char* bytes = (char*)xmalloc(1536 - 32);
	srs_schema1_copy_to(bytes, false, time, version, digest, key);
	return bytes;
}

/**
* compare the memory in bytes.
*/
bool srs_bytes_equals(void* pa, void* pb, int size) {
	unsigned char* a = (unsigned char*)pa;
	unsigned char* b = (unsigned char*)pb;
	for (int i = 0; i < size; i++) {
		if (a[i] != b[i]) {
			return false;
		}
	}

	return true;
}


long get_rtmp_time()
{
	struct	timeval		tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}



c2s2::c2s2()
{
	for (int i = 0; i < 1504; i++) {
		*(random + i) = rand();
	}
	for (int i = 0; i < 32; i++) {
		*(digest + i) = rand();
	}
}

c2s2::~c2s2()
{
}

void c2s2::dump(char* _c2s2)
{
	memcpy(_c2s2, random, 1504);
	memcpy(_c2s2 + 1504, digest, 32);
}

int c2s2::c2_create(c1s1* s1)
{
	int ret = 0;
	char temp_key[OpensslHashSize];
	if ((ret = openssl_HMACsha256(s1->get_digest(), 32, SrsGenuineFPKey, 62, temp_key)) != 0) {
		logs->error("create c2 temp key failed. ret=%d", ret);
		return ret;
	}
	logs->debug("generate c2 temp key success.");
	char _digest[OpensslHashSize];
	if ((ret = openssl_HMACsha256(random, 1504, temp_key, 32, _digest)) != 0) {
		logs->error("create c2 digest failed. ret=%d", ret);
		return ret;
	}
	logs->debug("generate c2 digest success.");
	memcpy(digest, _digest, 32);
	return ret;
}

int c2s2::s2_create(c1s1* c1)
{
	int ret = 0;
	char temp_key[OpensslHashSize];
	if ((ret = openssl_HMACsha256(c1->get_digest(), 32, SrsGenuineFMSKey, 68, temp_key)) != 0) {
		logs->error("create s2 temp key failed. ret=%d", ret);
		return ret;
	}
	logs->debug("generate s2 temp key success.");
	char _digest[OpensslHashSize];
	if ((ret = openssl_HMACsha256(random, 1504, temp_key, 32, _digest)) != 0) {
		logs->error("create s2 digest failed. ret=%d", ret);
		return ret;
	}
	logs->debug("generate s2 digest success.");
	memcpy(digest, _digest, 32);
	return ret;
}

c1s1::c1s1()
{
	schema = srs_schema_invalid;
}
c1s1::~c1s1()
{
	destroy_blocks();
}

char* c1s1::get_digest()
{
	assert(schema != srs_schema_invalid);
	if (schema == srs_schema0) {
		return block1.digest.digest;
	}
	else {
		return block0.digest.digest;
	}
}

void c1s1::dump(char* _c1s1)
{
	assert(schema != srs_schema_invalid);
	if (schema == srs_schema0) {
		srs_schema0_copy_to(_c1s1, true, time, version, &block0.key, &block1.digest);
	}
	else {
		srs_schema1_copy_to(_c1s1, true, time, version, &block0.digest, &block1.key);
	}
}

int c1s1::c1_create(srs_schema_type _schema)
{
	int ret = 0;
	if (_schema == srs_schema_invalid) {
		ret = -1;
		logs->error("create c1 failed. invalid schema=%d, ret=%d", _schema, ret);
		return ret;
	}
	destroy_blocks();
	time = htonl(get_rtmp_time());
	version = htonl(cms_rtmp_client_version);//0x027c0080; // client c1 version
	if (_schema == srs_schema0) {
		srs_key_block_init(&block0.key);
		srs_digest_block_init(&block1.digest);
	}
	else {
		srs_digest_block_init(&block0.digest);
		srs_key_block_init(&block1.key);
	}
	schema = _schema;
	char* digest = NULL;
	if ((ret = calc_c1_digest(digest)) != 0) {
		logs->error("sign c1 error, failed to calc digest. ret=%d", ret);
		return ret;
	}
	assert(digest != NULL);

	if (schema == srs_schema0) {
		memcpy(block1.digest.digest, digest, 32);
	}
	else {
		memcpy(block0.digest.digest, digest, 32);
	}
	xfree(digest);
	return ret;
}

int c1s1::c1_parse(char* _c1s1, srs_schema_type _schema)
{
	int ret = 0;
	if (_schema == srs_schema_invalid) {
		ret = -1;
		logs->error("parse c1 failed. invalid schema=%d, ret=%d", _schema, ret);
		return ret;
	}
	destroy_blocks();
	time = *(int*)_c1s1;
	version = *(int*)(_c1s1 + 4); // client c1 version
	if (_schema == srs_schema0) {
		if ((ret = srs_key_block_parse(&block0.key, _c1s1 + 8)) != 0) {
			logs->error("parse the c1 key failed. ret=%d", ret);
			return ret;
		}
		if ((ret = srs_digest_block_parse(&block1.digest, _c1s1 + 8 + 764)) != 0) {
			logs->error("parse the c1 digest failed. ret=%d", ret);
			return ret;
		}
		logs->debug("parse c1 key-digest success");
	}
	else if (_schema == srs_schema1) {
		if ((ret = srs_digest_block_parse(&block0.digest, _c1s1 + 8)) != 0) {
			logs->error("parse the c1 key failed. ret=%d", ret);
			return ret;
		}
		if ((ret = srs_key_block_parse(&block1.key, _c1s1 + 8 + 764)) != 0) {
			logs->error("parse the c1 digest failed. ret=%d", ret);
			return ret;
		}
		logs->debug("parse c1 digest-key success");
	}
	else {
		ret = -1;
		logs->error("parse c1 failed. invalid schema=%d, ret=%d", _schema, ret);
		return ret;
	}
	schema = _schema;
	return ret;
}

int c1s1::c1_validate_digest(bool& is_valid)
{
	int ret = 0;
	char* c1_digest = NULL;
	if ((ret = calc_c1_digest(c1_digest)) != 0) {
		logs->error("validate c1 error, failed to calc digest. ret=%d", ret);
		return ret;
	}
	assert(c1_digest != NULL);

	if (schema == srs_schema0) {
		is_valid = srs_bytes_equals(block1.digest.digest, c1_digest, 32);
	}
	else {
		is_valid = srs_bytes_equals(block0.digest.digest, c1_digest, 32);
	}
	xfree(c1_digest);
	return ret;
}

int c1s1::s1_create(c1s1* c1)
{
	int ret = 0;
	if (c1->schema == srs_schema_invalid) {
		ret = -1;
		logs->error("create s1 failed. invalid schema=%d, ret=%d", c1->schema, ret);
		return ret;
	}
	destroy_blocks();
	schema = c1->schema;
	time = htonl(get_rtmp_time());
	version = htonl(cms_rtmp_server_version);//0x01000504; // server s1 version
	if (schema == srs_schema0) {
		srs_key_block_init(&block0.key);
		srs_digest_block_init(&block1.digest);
	}
	else {
		srs_digest_block_init(&block0.digest);
		srs_key_block_init(&block1.key);
	}
	if (schema == srs_schema0) {
		if ((ret = openssl_generate_key(c1->block0.key.key, block0.key.key, 128)) != 0) {
			logs->error("calc s1 key failed. ret=%d", ret);
			return ret;
		}
	}
	else {
		if ((ret = openssl_generate_key(c1->block1.key.key, block1.key.key, 128)) != 0) {
			logs->error("calc s1 key failed. ret=%d", ret);
			return ret;
		}
	}
	logs->debug("calc s1 key success.");
	char* s1_digest = NULL;
	if ((ret = calc_s1_digest(s1_digest)) != 0) {
		logs->error("calc s1 digest failed. ret=%d", ret);
		return ret;
	}
	logs->debug("calc s1 digest success.");
	assert(s1_digest != NULL);

	if (schema == srs_schema0) {
		memcpy(block1.digest.digest, s1_digest, 32);
	}
	else {
		memcpy(block0.digest.digest, s1_digest, 32);
	}
	logs->debug("copy s1 key success.");
	xfree(s1_digest);
	return ret;
}

int c1s1::calc_s1_digest(char*& digest)
{
	int ret = 0;
	assert(schema == srs_schema0 || schema == srs_schema1);
	char* c1s1_joined_bytes = NULL;

	if (schema == srs_schema0) {
		c1s1_joined_bytes = srs_bytes_join_schema0(time, version, &block0.key, &block1.digest);
	}
	else {
		c1s1_joined_bytes = srs_bytes_join_schema1(time, version, &block0.digest, &block1.key);
	}
	assert(c1s1_joined_bytes != NULL);

	digest = (char*)xmalloc(OpensslHashSize);
	if ((ret = openssl_HMACsha256(c1s1_joined_bytes, 1536 - 32, SrsGenuineFMSKey, 36, digest)) != 0) {
		logs->error("calc digest for s1 failed. ret=%d", ret);
		return ret;
	}
	xfree(c1s1_joined_bytes);
	logs->debug("digest calculated for s1");
	return ret;
}

int c1s1::calc_c1_digest(char*& digest)
{
	int ret = 0;
	assert(schema == srs_schema0 || schema == srs_schema1);
	char* c1s1_joined_bytes = NULL;

	if (schema == srs_schema0) {
		c1s1_joined_bytes = srs_bytes_join_schema0(time, version, &block0.key, &block1.digest);
	}
	else {
		c1s1_joined_bytes = srs_bytes_join_schema1(time, version, &block0.digest, &block1.key);
	}
	assert(c1s1_joined_bytes != NULL);

	digest = (char*)xmalloc(OpensslHashSize);
	if ((ret = openssl_HMACsha256(c1s1_joined_bytes, 1536 - 32, SrsGenuineFPKey, 30, digest)) != 0) {
		logs->error("calc digest for c1 failed. ret=%d", ret);
		return ret;
	}
	xfree(c1s1_joined_bytes);
	logs->debug("digest calculated for c1");
	return ret;
}

void c1s1::destroy_blocks()
{
	if (schema == srs_schema_invalid) {
		return;
	}
	if (schema == srs_schema0) {
		srs_key_block_free(&block0.key);
		srs_digest_block_free(&block1.digest);
	}
	else {
		srs_digest_block_free(&block0.digest);
		srs_key_block_free(&block1.key);
	}
}
