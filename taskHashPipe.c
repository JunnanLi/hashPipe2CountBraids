# include "taskHashPipe.h"


void getHashValue_hashPipe(struct flowTuple *flow, uint32 *index_hash){
	uint8 key[13]; 
	flow2Byte(flow, key);
	int i =0;
	for(i = 0; i< NUM_HASHPIPE_STAGE; i++){
		index_hash[i] = calculateCRC32(key, 13, crc32Table[i]);
		index_hash[i] = calculateHash32_hashPipe(index_hash[i]);
	}
}

// do not copy eNext;
void cpyHashEntry(tHashEntry *hashEntryA, tHashEntry *hashEntryB){
	hashEntryA->valid = hashEntryB->valid;
	cpyFlowTuple(&hashEntryA->ft, &hashEntryB->ft);
	hashEntryA->count_pkt = hashEntryB->count_pkt;
	hashEntryA->count_size = hashEntryB->count_size;
}

// do not exchange eNext;
void exHashEntry(tHashEntry *hashEntryA, tHashEntry *hashEntryB){
	tHashEntry *hashEntryC;
	hashEntryC = (tHashEntry *)malloc(sizeof(tHashEntry));
	cpyHashEntry(hashEntryC, hashEntryB);
	cpyHashEntry(hashEntryB, hashEntryA);
	cpyHashEntry(hashEntryA, hashEntryC);
}


//initial hashTable: valid assigned "0"  && link all hashEntry:
void intialHashPipe(tHashEntry **hashTable){
	int perStage, perEntry;
	
	for( perStage = 0; perStage < NUM_HASHPIPE_STAGE; perStage++){
		hashTable[perStage][0].valid = 0;
		hashTable[perStage][0].count_pkt = 0;
		if(perStage == NUM_HASHPIPE_STAGE-1)
			hashTable[perStage][NUM_HASHPIPE_ENTRY-1].eNext = NULL;
		else
			hashTable[perStage][NUM_HASHPIPE_ENTRY-1].eNext = &hashTable[perStage+1][0];
		for( perEntry = 1; perEntry < NUM_HASHPIPE_ENTRY; perEntry++){
			hashTable[perStage][perEntry].valid = 0;
			hashTable[perStage][perEntry].count_pkt = 0;
			hashTable[perStage][perEntry-1].eNext = &hashTable[perStage][perEntry];
		}
	}
}


void updateMinValueHashTable(tHashEntry **hashTable, int *min_pkt){
	int perStage, perEntry; int min = 20000000;
	for(perStage = 0; perStage < NUM_HASHPIPE_STAGE; perStage++)
		for(perEntry = 0; perEntry < NUM_HASHPIPE_ENTRY; perEntry++)
			if(min > hashTable[perStage][perEntry].count_pkt)
				min = hashTable[perStage][perEntry].count_pkt;
	*min_pkt = min;
}

int updateHashPipe(tHashEntry **hashTable, struct pktTuple *pkt, tHashEntry *metadata_entry, int *min_pkt){
	int preMin_pkt = *min_pkt;
	// calculate hash value (NUM_HASHPIPE_STAGE)
	uint32 index_hash[NUM_HASHPIPE_STAGE];
	getHashValue_hashPipe(&pkt->ft, index_hash);
	int index = index_hash[0];
	
	// initial metadata_entry;
	metadata_entry->valid = 1;
	cpyFlowTuple(&metadata_entry->ft, &pkt->ft);
	metadata_entry->count_pkt = 1;
	metadata_entry->count_size = pkt->pktLen;
	
	// compare with the first stage
	if(cmpFlowTuple(&hashTable[0][index].ft, &metadata_entry->ft) == 0){
		// update entry
		hashTable[0][index].count_pkt +=1;
		hashTable[0][index].count_size += pkt->pktLen;
		// update min value of hashPipe
		if(hashTable[0][index].count_pkt == (*min_pkt+1))
			updateMinValueHashTable(hashTable, min_pkt);
		// update metadata_entry
		metadata_entry->valid = 0;
		return 2;	// hitted
	}
	// insert metadata in the first stage;
	else{
		exHashEntry(&hashTable[0][index], metadata_entry);
		updateMinValueHashTable(hashTable, min_pkt);
	}
	// compare with the left stages
	int perStage;
	for(perStage = 1; perStage < NUM_HASHPIPE_STAGE; perStage++){
		// calculate hash value (NUM_HASHPIPE_STAGE)
		getHashValue_hashPipe(&metadata_entry->ft, index_hash);
		index = index_hash[perStage];
		if(metadata_entry->valid == 1){
			// compare
			if(cmpFlowTuple(&hashTable[perStage][index].ft, &metadata_entry->ft) == 0){
				hashTable[perStage][index].count_pkt += metadata_entry->count_pkt;
				hashTable[perStage][index].count_size += metadata_entry->count_size;
				// update minValue;
				if(hashTable[perStage][index].count_pkt == (*min_pkt + metadata_entry->count_pkt))
					updateMinValueHashTable(hashTable, min_pkt);
				metadata_entry->valid = 0;
				return 2;
			}
			// exchane
			else{
				if(hashTable[perStage][index].count_pkt < metadata_entry->count_pkt){
					exHashEntry(&hashTable[perStage][index], metadata_entry);
					updateMinValueHashTable(hashTable, min_pkt);
				}
			}
		}
		else
			return 2;
	}
	if((preMin_pkt < metadata_entry->count_pkt) && (metadata_entry->valid == 1)) return 1;	// the entry kickouted is not the smallest value;
	else return 0;	// kickout the smallest value;
}


tHashEntry *sortHashTable(tHashEntry **hashTable, int num_top){
	int empty_entry = 0;	// statics

	int perStage, perEntry, perStage_2, perEntry_2;
	for(perStage = 0; perStage < NUM_HASHPIPE_STAGE; perStage++)
		for(perEntry =0; perEntry < NUM_HASHPIPE_ENTRY; perEntry++)
			if(hashTable[perStage][perEntry].valid == 0)
				empty_entry++;
	printf("empty_entry in taskHashPipe.c: %d\n", empty_entry);

	// aggregate the same entries
	for(perStage = 0; perStage < NUM_HASHPIPE_STAGE; perStage++)
		for(perEntry =0; perEntry < NUM_HASHPIPE_ENTRY; perEntry++){
			//printf("stage:%d, entry: %d\n", perStage, perEntry);
			if(hashTable[perStage][perEntry].valid == 0)
				continue;
			for(perStage_2 = perStage+1; perStage_2 < NUM_HASHPIPE_STAGE; perStage_2++)
				for(perEntry_2 = 0; perEntry_2 < NUM_HASHPIPE_ENTRY; perEntry_2++)
					if((cmpFlowTuple(&hashTable[perStage][perEntry].ft, &hashTable[perStage_2][perEntry_2].ft) == 0) && 
						(hashTable[perStage_2][perEntry_2].valid == 1)){
						hashTable[perStage][perEntry].count_pkt += hashTable[perStage_2][perEntry_2].count_pkt;
						hashTable[perStage][perEntry].count_size += hashTable[perStage_2][perEntry_2].count_size;
						hashTable[perStage_2][perEntry_2].valid = 0;
					}
			//printf("src_ip: %x, count_pkt: %u\n", hashTable[perStage][perEntry].ft.src_ip, hashTable[perStage][perEntry].count_pkt);
		}
	

	int perTop = 0;
	tHashEntry *aggHashTable = NULL;
	tHashEntry *preHashEntry = NULL;
	tHashEntry *pHashEntry = NULL;
	for(perTop = 0; perTop < num_top; perTop++){
		int max = 0;
		pHashEntry = NULL;
		for(perStage = 0; perStage < NUM_HASHPIPE_STAGE; perStage++)
			for(perEntry =0; perEntry < NUM_HASHPIPE_ENTRY; perEntry++)
				if((hashTable[perStage][perEntry].valid == 1)&&(hashTable[perStage][perEntry].count_pkt > max)){
					pHashEntry = &hashTable[perStage][perEntry];
					max = hashTable[perStage][perEntry].count_pkt;
				}
		// end condition
		if(pHashEntry == NULL){
			preHashEntry->eNext = pHashEntry;
			return aggHashTable;
		}
		// unvalid out entry
		pHashEntry->valid = 0;
		// set list header
		if(preHashEntry == NULL){
			aggHashTable = pHashEntry;
			preHashEntry = pHashEntry;
		}
		else{	// construct list
			preHashEntry->eNext = pHashEntry;
			preHashEntry = pHashEntry;
		}
	}
	// set list end
	preHashEntry->eNext = NULL;
	return aggHashTable;
}

/*
tHashEntry * aggregateHashTable(tHashEntry **hashTable){
	tHashEntry * headHashEntry = hashTable[0];
	tHashEntry * sHashEntry = hashTable[0];
	tHashEntry * pHashEntry, *preHashEntry;
	while(sHashEntry){
		pHashEntry = headHashEntry;
		preHashEntry = NULL;
		while(pHashEntry){
			if(pHashEntry == sHashEntry)
				break;
			if(pHashEntry->count_pkt <= sHashEntry->count_pkt){
				if(preHashEntry == NULL){
					headHashEntry = sHashEntry;
					break;
				}
				else{
					preHashEntry->eNext = sHashEntry;
					sHashEntry->eNext = pHashEntry;
					break;
				}
			}
			preHashEntry = pHashEntry;
			pHashEntry = pHashEntry->eNext;
		}
		sHashEntry = sHashEntry->eNext;
	}
	return headHashEntry;
}
*/

void aggregateHashTableWithFlowTable(tHashEntry *aggHashTable, tFlowTable *flowTable, int *index_flowTable){
	int perFlowTuple;
	tHashEntry *hashEntry = aggHashTable;
	while(hashEntry){
		for(perFlowTuple = 0; perFlowTuple < *index_flowTable; perFlowTuple++){
			if(cmpFlowTuple(&hashEntry->ft, &flowTable[perFlowTuple].ft) == 0){
				flowTable[perFlowTuple].count += hashEntry->count_pkt;
				flowTable[perFlowTuple].count_size += hashEntry->count_size;
				break;
			}
		}
		if(perFlowTuple == *index_flowTable){
			cpyFlowTuple(&flowTable[*index_flowTable].ft, &hashEntry->ft);
			flowTable[*index_flowTable].count = hashEntry->count_pkt;
			flowTable[*index_flowTable].count_size = hashEntry->count_size;
			*index_flowTable += 1;
		}
		hashEntry = hashEntry->eNext;
	}
}


void printHashPipe(tHashEntry **hashTable){
	FILE *fp;
	if((fp = fopen("result_hashTable.txt","a+"))==NULL){
		printf("open result_hashTable.txt error\n");
		exit(0);
	}
	int perStage, perEntry;
	tHashEntry *hashEntry;
	for(perStage = 0; perStage < NUM_HASHPIPE_STAGE; perStage++){
		fprintf(fp, "==stage: %d==\n", perStage);
		for(perEntry = 0; perEntry < NUM_HASHPIPE_ENTRY; perEntry++){
			hashEntry = &hashTable[perStage][perEntry];
			if(hashEntry->valid == 0) 
				fprintf(fp, "%d:\tunValid\n", perEntry);
			else
				fprintf(fp, "%d:\t%x\t%x\t%hd\t%hd\t%u\n", perEntry, hashEntry->ft.src_ip, hashEntry->ft.dst_ip,
					hashEntry->ft.src_port, hashEntry->ft.dst_port, hashEntry->count_pkt);
		}
	}
	fprintf(fp, "============================\n");
	fclose(fp);
}


void printhashEntry(tHashEntry *hashEntry){
	while(hashEntry){
		printf("src_ip: %x, count_pkt:%u\n", hashEntry->ft.src_ip, hashEntry->count_pkt);
		hashEntry = hashEntry->eNext;
	}
}