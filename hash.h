#ifndef __HASH_H_
#define __HASH_H_


#include "common.h"


uint32 crc32Table[12][256];

// initial hash functions
uint32 bitRev(uint32 input, int bw);
void crc32_init(uint32 poly, uint32 *table);
void initialHash();

// calculate hash values
uint32 calculateCRC32(uint8 *key, int keyByteLength, uint32 *table);
// change 32bit to countBraids
uint32 calculateHash32(uint32 hash_32bit);
// chage 32bit to hashPipe
uint32 calculateHash32_hashPipe(uint32 hash_32bit);

uint32 calculateHash32_bloomFilter(uint32 hash_32bit);

#endif