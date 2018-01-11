# include "taskBloomFilter.h"


void getHashValue_bloomFilter(struct flowTuple *flow, uint32 *index_hash){
	uint8 key[13]; 
	flow2Byte(flow, key);
	int i =0;
	for(i = 0; i< NUM_BLOOMFILTER_HASH; i++){
		index_hash[i] = calculateCRC32(key, 13, crc32Table[i]);
		index_hash[i] = calculateHash32_bloomFilter(index_hash[i]);
	}
}


//initial bloomFilter---assigned "0" :
void intialBloomFilter(uint8 **bloomFilter){
	int perEntry, perHash;

	for(perHash = 0; perHash < NUM_BLOOMFILTER_HASH; perHash++)
		for( perEntry = 0; perEntry < NUM_BLOOMFILTER; perEntry++)
			bloomFilter[perHash][perEntry] = (uint8) 0;
	/*		
	for( perEntry = 0; perEntry < NUM_BLOOMFILTER; perEntry++)
		bloomFilter[perEntry] = (uint8) 0;
	*/	
}


int lookupBloomFilter(struct flowTuple *flow, uint8 **bloomFilter){
	uint32 index_hash[NUM_BLOOMFILTER_HASH];
	//uint32 index_hash_entry[NUM_BLOOMFILTER_HASH];	// which 8-bit;
	//uint8 mask_hash_entry[NUM_BLOOMFILTER_HASH];	// mask of 8-bit;
	getHashValue_bloomFilter(flow, index_hash);
	int perHash;
	int hitness = 1;
	for(perHash = 0; perHash < NUM_BLOOMFILTER_HASH; perHash++){
		if(bloomFilter[perHash][index_hash[perHash]] != (uint8) 1){
			bloomFilter[perHash][index_hash[perHash]] = (uint8) 1;
		//if(bloomFilter[index_hash[perHash]] != (uint8) 1){
		//	bloomFilter[index_hash[perHash]] = (uint8) 1;
			hitness = 0;
		}
		/*
		index_hash_entry[perHash] = index_hash[perHash]/8;
		mask_hash_entry[perHash] = (uint8) 1 << (index_hash[perHash]%8);
		if((bloomFilter[index_hash_entry[perHash]] & mask_hash_entry[perHash]) != mask_hash_entry[perHash]){
			bloomFilter[index_hash_entry[perHash]] = bloomFilter[index_hash_entry[perHash]] | mask_hash_entry[perHash];
			return 0;
		}
		*/
	}
	if(hitness == 1)
		return 1;
	else
		return 0;
}