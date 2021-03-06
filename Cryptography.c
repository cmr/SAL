/** vim: set noet ci pi sts=0 sw=4 ts=4
 * @file Cryptography.c
 * @brief Cryptographic functions (hashes, random numbers)
 */

#include "Cryptography.h"
#include "Time.h"

#ifdef WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	#include <wincrypt.h>
#elif defined POSIX
	#include <openssl/evp.h>
#endif

#include <math.h>
#include <stdlib.h>

static boolean seeded = false;

/**
 * Hash a block of memory using SHA512.
 *
 * @param source [in] pointer to block of memory to hash
 * @param length [in] number of bytes from source to hash.
 * @returns pointer to digest (64 bytes in length)
 *
 * @warning You need to free the returned memory yourself
 */
uint8* SAL_Cryptography_SHA512(uint8* source, uint32 length) {
#ifdef WINDOWS
	HCRYPTPROV provider = 0;
	HCRYPTHASH hasher = 0;
	DWORD hashLength;
	uint8* hash;

	CryptAcquireContext(&provider, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT);
	CryptCreateHash(provider, CALG_SHA_512, 0, 0, &hasher);
	CryptHashData(hasher, source, length, 0);

	hash = AllocateArray(uint8, 64);
	CryptGetHashParam(hasher, HP_HASHVAL, hash, &hashLength, 0);

	CryptDestroyHash(hasher);
	CryptReleaseContext(provider, 0);

	return hash;
#elif defined POSIX
	EVP_MD_CTX *ctx = EVP_MD_CTX_create();
	uint8 *hash = AllocateArray(uint8, EVP_MD_size(EVP_sha512()));

	EVP_DigestInit_ex(ctx, EVP_sha512(), NULL);
	EVP_DigestUpdate(ctx, (void*)source, length);
	EVP_DigestFinal_ex(ctx, (unsigned char*)hash, NULL);

	return hash;
#endif
}

/**
 * Hash a block of memory using SHA-1
 *
 * @param source [in] pointer to block of memory to hash
 * @param length [in] number of bytes from source to hash.
 * @returns pointer to digest (20 bytes in length)
 *
 * @warning You need to free the returned memory yourself
 */
uint8* SAL_Cryptography_SHA1(uint8* source, uint32 length) {
#ifdef WINDOWS
	HCRYPTPROV provider = 0;
	HCRYPTHASH hasher = 0;
	DWORD hashLength;
	uint8* hash;

	CryptAcquireContext(&provider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
	CryptCreateHash(provider, CALG_SHA1, 0, 0, &hasher);
	CryptHashData(hasher, source, length, 0);

	hash = AllocateArray(uint8, 20);
	CryptGetHashParam(hasher, HP_HASHVAL, hash, &hashLength, 0);

	CryptDestroyHash(hasher);
	CryptReleaseContext(provider, 0);

	return hash;
#elif defined POSIX
	EVP_MD_CTX *ctx = EVP_MD_CTX_create();
	uint8 *hash = AllocateArray(uint8, EVP_MD_size(EVP_sha1()));

	EVP_DigestInit_ex(ctx, EVP_sha1(), NULL);
	EVP_DigestUpdate(ctx, (void*)source, length);
	EVP_DigestFinal_ex(ctx, (unsigned char*)hash, NULL);

	return hash;
#endif
}


/**
 * Generate pseudorandom bytes.
 *
 * @param count [in] number of bytes to generate
 * @returns pointer to @a count bytes.
 *
 * @warning You need to free the returned memory yourself
 */
uint8* SAL_Cryptography_RandomBytes(uint64 count) {
	uint8* bytes = NULL;
	uint8 i;

	if (count > 0) {
		bytes = AllocateArray(uint8, count); // Integer division rounds towards 0, count % 4 is the remainder

		if (!seeded) {
			srand( (uint32)SAL_Time_Now() );
			seeded = true;
		}

		for (; count > 3; count -= 4)
			*((uint32*)bytes + count - 4) = rand();

		for (i = 0; i < count; i++)
			*(bytes + i) = (uint8)rand();
	}

	return bytes;
}

/**
 * Generate a 8 byte pseudorandom value.
 *
 * @param floor [in] Lower bound of random value
 * @param ceiling [in] Upper bound of random value
 * @returns 8 pseudorandom bytes
 */
uint64 SAL_Cryptography_RandomUInt64(uint64 floor, uint64 ceiling) {
	uint64 result;

	if (!seeded) {
		srand( (uint32)SAL_Time_Now() );
		seeded = true;
	}

	result = (uint32)rand();
	result <<= 32;
	result += (uint32)rand();

	result = result % (ceiling - floor) + floor;

	return result;
}

/**
 * Generate a 4 byte pseudorandom value.
 *
 * @param floor [in] Lower bound of random value
 * @param ceiling [in] Upper bound of random value
 * @returns 4 pseudorandom bytes
 */
uint32 SAL_Cryptography_RandomUInt32(uint32 floor, uint32 ceiling) {
	uint32 result;

	if (!seeded) {
		srand( (uint32)SAL_Time_Now() );
		seeded = true;
	}
	result = rand() % (ceiling - floor) + floor;

	return (uint32)result;
}

/**
 * Generate a 2 byte pseudorandom value.
 *
 * @param floor [in] Lower bound of random value
 * @param ceiling [in] Upper bound of random value
 * @returns 2 pseudorandom bytes
 */
uint16 SAL_Cryptography_RandomUInt16(uint16 floor, uint16 ceiling) {
	uint16 result;

	if (!seeded) {
		srand( (uint32)SAL_Time_Now() );
		seeded = true;
	}
	result = rand() % (ceiling - floor) + floor;

	return (uint16)result;
}

/**
 * Generate a 1 byte pseudorandom value.
 *
 * @param floor [in] Lower bound of random value
 * @param ceiling [in] Upper bound of random value
 * @returns 1 pseudorandom byte
 */
uint8 SAL_Cryptography_RandomUInt8(uint8 floor, uint8 ceiling) {
	uint8 result;

	if (!seeded) {
		srand( (uint32)SAL_Time_Now() );
		seeded = true;
	}
	result = rand() % (ceiling - floor) + floor;

	return (uint8)result;
}
