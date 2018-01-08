#ifndef __COMMON_H_
#define __COMMON_H_

#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


//==================CountBraids parameter================//
#define NUM_LAYER 1				// layers of countBraids (maximun = 2)
#define NUM_HASH 3				// hashes of each packet
#define NUM_CONTER_1_LAYER 262144		// counters of the first layer
#define NUM_CONTER_2_LAYER 1048576	// counters of the second layer
#define MAX_NUM_1_LAYER 20000000		// 32bit; or namely, threshold
#define BIT_HASH_INDEX 18 			// used to calculate hash value iteratively
#define HASH_MASK NUM_CONTER_1_LAYER-1
#define REMINDER 262144 				// used by hash function
#define MAX_NUM_FLOW  10000000
#define MAX_NUM_PACKET 40000000
#define NUM_ITERATION 10
#define MIN_VALUE 1


/*	2^16	65536
	2^17	131072
	2^18	262144
	2^19	524288
	2^20	1048576
	2^21   	2097152
	2^22    	4194304
*/

#define TIME_USEC	3300000			// epoch interval

//================HashPipe parameter================//
#define TOP_K 300
#define NUM_HASHPIPE_STAGE 8 		// hashPipe stages
#define BIT_HASH_INDEX_HASHPIPE 10 		// used to calculate hash value iteratively
#define NUM_HASHPIPE_ENTRY 1024 		// entry number
#define REMINDER_HP 1024 			// used by hash function
#define MAX_ENTRY NUM_HASHPIPE_ENTRY*NUM_HASHPIPE_STAGE
#define HASH_MASK_HASHPIPE NUM_HASHPIPE_ENTRY-1

//================bloomFilter================//
#define BIT_HASH_INDEX_BLOOMFILTER 22
#define NUM_BLOOMFILTER_HASH 4
#define NUM_BLOOMFILTER 4194304
#define REMINDER_BF 4194304
#define HASH_MASK_BLOOMFILTER NUM_BLOOMFILTER-1


typedef unsigned long uint64;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

struct time_val{
	int tv_sec;
	int tv_usec;
};

// 5-tuple + fist packet tag
struct flowTuple{
	uint32 src_ip;
	uint32 dst_ip;
	uint16 src_port;
	uint16 dst_port;
	uint8 proto;
	uint8 tag;	// first packet of the flow;
};

// 8-tuple {5-tuple, pktLen, tv}
struct pktTuple{
	struct flowTuple ft;
	uint32 pktLen;
	struct time_val tv;
};

// counter
typedef struct counter{
	uint32 count_value;		// pkt num;
	uint32 count_size;		// flow size;
	int hashID;			// the id of hash function;
	int flowID;			// the id fo flow;
	struct counter * cNext;	// used by hashTable;
} tCounter;

// used to record each flow according to countBraids
typedef struct flowTable{
	struct flowTuple ft;
	uint32 entryPosition;			// the position in previous Position;
	uint32 count;				// used to record the final estimation value;
	uint32 count_size;			// used to record the final estimation size;
	uint32 index_hash[NUM_HASH];	// used to record the hash values of each hash function;
	tCounter uList[NUM_HASH];		// used to record the U values returned from hashTable;
} tFlowTable;

// used to record counts of each packets hashed to countBraids
typedef struct hashTable {
	uint32 count;			// number of packets hashed to this entry;
	uint32 count_size;		// total size of packets hashed to this entry;
	uint8 statusBit;		// '1' represent the count is overflow;
	//uint32 total_count;
	tCounter *vList;		// used to record the V values returned from flowTable;
} tHashTable;


typedef union{
	uint32 as_int32;
	uint16 as_int16s[2];
	uint8 as_int8s[4];
} int32views;


typedef union{
	uint16 as_int16;
	uint8 as_int8s[2];
} int16views;





// change flow type to byte type
void flow2Byte(struct flowTuple *flow, uint8 *key);
// get the max num number between a and b
uint32 getMaxValue(uint32 a, uint32 b);
// get the result of ASB() of a minus b
uint32 uABS(uint32 a, uint32 b);
// comparation of A and B, equal wiil return 0, else return 1
int cmpFlowTuple(struct flowTuple *flowA, struct flowTuple *flowB);
// copy B to A
void  cpyFlowTuple(struct flowTuple *flowA, struct flowTuple *flowB);



#endif