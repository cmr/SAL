#ifndef SAL_INCLUDE_CRYPTOGRAPHY
#define SAL_INCLUDE_CRYPTOGRAPHY

#include "Common.h"

public uint8* SAL_Cryptography_SHA512(uint8* source, uint32 length);
public uint8* SAL_Cryptography_SHA1(uint8* source, uint32 length);

public uint8* SAL_Cryptography_RandomBytes(uint64 count);

public uint64 SAL_Cryptography_RandomUInt64(uint64 floor, uint64 ceiling);
public uint32 SAL_Cryptography_RandomUInt32(uint32 floor, uint32 ceiling);
public uint16 SAL_Cryptography_RandomUInt16(uint16 floor, uint16 ceiling);
public uint8 SAL_Cryptography_RandomUInt8(uint8 floor, uint8 ceiling);

#endif
