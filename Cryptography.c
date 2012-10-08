/** @file Cryptography.c
 * @brief Cryptographic functions (hashes, random numbers)
 */

#include <math.h>

#include "Cryptography.h"
#include "Time.h"

#ifdef WINDOWS
	#include <Windows.h>
	#pragma comment(lib, "crypt32.lib")
#elif defined POSIX
	#include <openssl/evp.h>
#endif

static boolean seeded = false;

/**
 * Hash a block of memory using SHA512.
 *
 * @param source [in] pointer to block of memory to hash
 * @param length [in] number of bytes from source to hash.
 * @returns pointer to digest (64 bytes in length)
 */
uint8* SAL_Cryptography_SHA512(uint8* source, uint32 length) {
	#ifdef WINDOWS
		HCRYPTPROV provider = 0;
		HCRYPTHASH hasher = 0;
		DWORD hashLength;
		uint8* hash;
    
		CryptAcquireContext(&provider, null, null, PROV_RSA_AES, CRYPT_VERIFYCONTEXT);
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


		EVP_DigestInit_ex(ctx, EVP_sha512(), null);
		EVP_DigestUpdate(ctx, (void*)source, length);
		EVP_DigestFinal_ex(ctx, (uint8*)hash, null);


		return hash;
	#endif
}

/**
 * Hash a block of memory using SHA-1
 *
 * @param source [in] pointer to block of memory to hash
 * @param length [in] number of bytes from source to hash.
 * @returns pointer to digest (20 bytes in length)
 */
uint8* SAL_Cryptography_SHA1(uint8* source, uint32 length) {
	#ifdef WINDOWS
	    HCRYPTPROV provider = 0;
		HCRYPTHASH hasher = 0;
		DWORD hashLength;
		uint8* hash;
    
		CryptAcquireContext(&provider, null, null, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
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


		EVP_DigestInit_ex(ctx, EVP_sha1(), null);
		EVP_DigestUpdate(ctx, (void*)source, length);
		EVP_DigestFinal_ex(ctx, (uint8*)hash, null);


		return hash;
	#endif
}


/**
 * Generate pseudorandom bytes.
 *
 * @param count [in] number of bytes to generate
 * @return pointer to @a count bytes.
 */
uint8* SAL_Cryptography_RandomBytes(uint64 count) {
	uint32 *bytes = null;
	uint8 rem = count % 4;
	uint8 i;

	if (count > 0) {
		bytes = AllocateArray(uint32, (count / 4) + rem); // Integer division rounds towards 0, count % 4 is the remainder

		if (!seeded) {
			srand( (uint32)SAL_Time_Now() );
			seeded = true;
		}

		for (; count > 3; count -= 4) 
		  *(bytes + count) = (uint32)rand();


		for (i = 0; i < rem; i++)
		  *((uint8*)bytes + i) = (uint8)rand();

	}

	return (uint8*)bytes;
}

/**
 * Generate a 4 byte pseudorandom value.
 *
 * @param floor [in] Lower bound of random value
 * @param ceiling [in] Upper bound of random value
 * @return 4 pseudorandom bytes
 */
uint32 SAL_Cryptography_RandomUInt32(uint32 floor, uint32 ceiling) {
	uint32 result;
	
	if (!seeded) {
		srand( (uint32)SAL_Time_Now() );
		seeded = true;
	}
	result = rand() % (ceiling - floor) + floor;

	return result;
}

/**
 * Generate a 2 byte pseudorandom value.
 *
 * @param floor [in] Lower bound of random value
 * @param ceiling [in] Upper bound of random value
 * @return 2 pseudorandom bytes
 */
uint16 SAL_Cryptography_RandomUInt16(uint16 floor, uint16 ceiling) {
	uint16 result;
	
	if (!seeded) {
		srand( (uint32)SAL_Time_Now() );
		seeded = true;
	}
	result = rand() % (ceiling - floor) + floor;

	return result;
}

/**
 * Generate a 1 byte pseudorandom value.
 *
 * @param floor [in] Lower bound of random value
 * @param ceiling [in] Upper bound of random value
 * @return 1 pseudorandom byte
 */
uint8 SAL_Cryptography_RandomUInt8(uint8 floor, uint8 ceiling) {
	uint8 result;
	
	if (!seeded) {
		srand( (uint32)SAL_Time_Now() );
		seeded = true;
	}
	result = rand() % (ceiling - floor) + floor;

	return result;
}

int32 SAL_Cryptography_RandomInt32(int32 floor, int32 ceiling) {
	int32 result;
	
	if (!seeded) {
		srand( (int32)SAL_Time_Now() );
		seeded = true;
	}
	result = rand() % (ceiling - floor) + floor;

	return result;
}

int16 SAL_Cryptography_RandomInt16(int16 floor, int16 ceiling) {
	int16 result;
	
	if (!seeded) {
		srand( (int32)SAL_Time_Now() );
		seeded = true;
	}
	result = rand() % (ceiling - floor) + floor;

	return result;
}

int8 SAL_Cryptography_RandomInt8(int8 floor, int8 ceiling) {
	int8 result;
	
	if (!seeded) {
		srand( (int32)SAL_Time_Now() );
		seeded = true;
	}
	result = rand() % (ceiling - floor) + floor;

	return result;
}
