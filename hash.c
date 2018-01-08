#include "hash.h"


uint32 bitRev(uint32 input, int bw){
	int i;  
	uint32 var = 0;
	for(i=0;i<bw;i++){
		if(input & 0x01){ 
			var |= 1<<(bw-1-i);  
		} 
		input >>= 1;
	}  
	return var;  
} 

void crc32_init(uint32 poly, uint32 *table){
	int i, j;
	uint32 c;
	poly = bitRev(poly, 32);
	for(i = 0; i< 256; i++){
		c = i;
		for(j = 0; j <8; j++){
			if(c & 1)
				c = poly ^ (c >> 1);
			else 
				c = c >> 1;
		}
		table[i] = c;
	}
}

void initialHash(){
	//hash initial
	crc32_init(0x4c11db7, crc32Table[0]);
	crc32_init(0x1edc6f41, crc32Table[1]);
	crc32_init(0x741b8cd7, crc32Table[2]);
      	crc32_init(0x814141ab, crc32Table[3]);
      	crc32_init(0xD5828281, crc32Table[4]);
      	crc32_init(0xEDB88320, crc32Table[5]);
      	crc32_init(0x82608EDB, crc32Table[6]);
      	crc32_init(0x82F63B78, crc32Table[7]);
      	crc32_init(0x992C1A4C, crc32Table[8]);
      	crc32_init(0x80108400, crc32Table[9]);
      	crc32_init(0x90022004, crc32Table[10]);
      	crc32_init(0xBA0DC66B, crc32Table[11]);
	/* more	*/
	// 0x814141ab		from www.thefullwiki.org/crc32;
      //0x82608EDB, 0xFA567D89, 0xD419CC15
      //0x8F6E37A0, 0x992C1A4C, 0x80108400
      //0xBA0DC66B, 0x90022004, 

}

uint32 calculateCRC32(uint8 *key, int keyByteLength, uint32 *table){
	int i;
	uint8 index;
	uint32 crc = 0xFFFFFFFF;
	for(i = 0; i < keyByteLength; i++){
		index = crc ^ key[i];
		crc = (crc >> 8) ^ table[index];
	}
	return ~crc;
}



uint32 calculateHash32(uint32 hash_32bit){
	uint32 hash_index = 0;
	int i = 0;
	int range = 32/BIT_HASH_INDEX+1;
	for(i = 0; i < range; i++){
		hash_index ^= (hash_32bit & HASH_MASK);
		//printf("hash_16bit:%x\thash_index:%x\n", hash_16bit,(hash_index>>13));
		hash_32bit = hash_32bit >> BIT_HASH_INDEX;
	}
	return ((hash_index & HASH_MASK) %REMINDER);
	//return (hash_index & HASH_MASK);
      //return ((hash_index & HASH_MASK)%NUM_CONTER_1_LAYER);
}


uint32 calculateHash32_hashPipe(uint32 hash_32bit){
  uint32 hash_index = 0;
  int i = 0;
  int range = 32/BIT_HASH_INDEX_HASHPIPE+1;
  for(i = 0; i < range; i++){
    hash_index ^= (hash_32bit & HASH_MASK_HASHPIPE);
    //printf("hash_16bit:%x\thash_index:%x\n", hash_16bit,(hash_index>>13));
    hash_32bit = hash_32bit >> BIT_HASH_INDEX_HASHPIPE;
  }
  return ((hash_index & HASH_MASK_HASHPIPE)%REMINDER_HP);
}

uint32 calculateHash32_bloomFilter(uint32 hash_32bit){
	uint32 hash_index = 0;
	int i = 0;
	int range = 32/BIT_HASH_INDEX_BLOOMFILTER +1;
	for(i = 0; i< range; i++){
		hash_index ^= (hash_32bit & HASH_MASK_BLOOMFILTER);
		hash_32bit = hash_32bit >> BIT_HASH_INDEX_BLOOMFILTER;
	}
	return ((hash_index & HASH_MASK_BLOOMFILTER)%REMINDER_BF);
}