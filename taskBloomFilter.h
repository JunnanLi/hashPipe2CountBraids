#ifndef __BLOOMFILTER_H_
#define __BLOOMFILTER_H_

#include "common.h"
#include "hash.h"





// support 6 kinds of hash functions;



void getHashValue_bloomFilter(struct flowTuple *flow, uint32 *index_hash);

void intialBloomFilter(uint8 **bloomFilter);

// '1' is hit, '0' in mis;
int lookupBloomFilter(struct flowTuple *flow, uint8 **bloomFilter);


#endif