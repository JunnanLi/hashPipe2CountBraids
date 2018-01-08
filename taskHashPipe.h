#ifndef __HASHPIPE_H_
#define __HASHPIPE_H_

#include "common.h"
#include "hash.h"
//#include "taskCounterBraids.h"

// hash entry struct
typedef struct hashEntry
{
	uint8 valid;
	struct flowTuple ft;
	uint32 count_pkt;
	uint32 count_size;
	struct hashEntry *eNext;	// used to sort hash entry
} tHashEntry;




// support 6 kinds of hash functions;



void getHashValue_hashPipe(struct flowTuple *flow, uint32 *index_hash);
void intialHashPipe(tHashEntry **hashTable);
void cpyHashEntry(tHashEntry *hashEntryA, tHashEntry *hashEntryB);
void exHashEntry(tHashEntry *hashEntryA, tHashEntry *hashEntryB);

void updateMinValueHashTable(tHashEntry **hashTable, int *min_pkt);

// '0' represents kick out the smallest entry; 1' represents kick out not the smallest entry; '2' is hitted
int updateHashPipe(tHashEntry **hashTable, struct pktTuple *pkt, tHashEntry *metadata_entry, int *min_pkt);

// return list header
tHashEntry * sortHashTable(tHashEntry **hashTable, int num_top);

void aggregateHashTableWithFlowTable(tHashEntry *aggHashTable, tFlowTable *flowTable, int *index_flowTable);

//test//
void printHashPipe(tHashEntry **hashTable);

void printhashEntry(tHashEntry *hashEntry);
#endif