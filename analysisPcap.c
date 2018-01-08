#include "analysisPcap.h"


uint32 hash_5_tuple(struct flowTuple *pkt){
	uint32 hash_index = 0;
	//hash_index = pkt->src_ip % PRIME;
	hash_index = (pkt->src_ip + pkt->dst_ip + (uint32) pkt->src_port + (uint32) pkt->dst_port) % PRIME;
	return hash_index;
}

void analysisInitial(tBigFlowTable *bigFlowTable, tBigFlowTable *bigFlowTable_c){
	int i = 0;
	for(i =0; i < NUM_BIG_FLOW_ENTRY; i++){
		bigFlowTable[i].count_pkt = 0;
		bigFlowTable[i].count_size = 0;
		bigFlowTable[i].eNext = NULL;
		bigFlowTable_c[i].count_pkt = 0;
		bigFlowTable_c[i].count_size = 0;
		bigFlowTable_c[i].eNext = NULL;
	}
}


int analysisPacket(FILE *fp_pkt, FILE * fp_pkt_tag,tBigFlowTable *bigFlowTable, tBigFlowTable *bigFlowTable_c){
	if((fp_pkt_tag = fopen("result_pkt_2.txt","w"))==NULL){
		printf("open result_pkt_2.txt error in analysisPcap.c\n");
		exit(0);
	}

	if((fp_pkt = fopen("result_pkt.txt","r"))==NULL){
		printf("read result_pkt.txt error in analysisPcap.c\n");
		exit(0);
	}

	uint32 hash_index = 0;
	uint32 conflict_index = 0;	// used to allocate bigFlowTable_c entry;
	tBigFlowTable *pBFT, *preBFT;
	int num_flow = 0;
	int num_pkt = 0;
	//for(i = 0; i< num_pkt; i++){
	struct pktTuple pkt;
	while(fscanf(fp_pkt, "%x\t%x\t%hd\t%hd\t%hhu\t%u\t%d\t%d\n", &pkt.ft.src_ip, &pkt.ft.dst_ip, &pkt.ft.src_port, &pkt.ft.dst_port, &pkt.ft.proto,
		&pkt.pktLen,&pkt.tv.tv_sec, &pkt.tv.tv_usec)!= EOF){
		if(pkt.ft.src_ip == 0)
			break;
		if(pkt.ft.proto != 0x6)
			continue;
		/*fprintf(fp, "%d\t%x\t%x\t%hd\t%hd\n", i, pkt[i].src_ip, pkt[i].dst_ip,
			pkt[i].src_port,pkt[i].dst_port);*/

		// calculate hash value;
		hash_index = hash_5_tuple(&pkt.ft);
		pBFT = bigFlowTable[hash_index].eNext;
		preBFT = &bigFlowTable[hash_index];

		// updage bigFlowTable
		if(bigFlowTable[hash_index].count_pkt == 0){
			cpyFlowTuple(&(bigFlowTable[hash_index].ft), &pkt.ft);
			pkt.ft.tag = 1;
			bigFlowTable[hash_index].eNext = NULL;	
			num_flow++;
			bigFlowTable[hash_index].count_pkt = 1;
			bigFlowTable[hash_index].count_size = pkt.pktLen;
		}
		else if(cmpFlowTuple(&bigFlowTable[hash_index].ft, &pkt.ft) == 0){
			pkt.ft.tag = 0;
			bigFlowTable[hash_index].count_pkt ++;
			bigFlowTable[hash_index].count_size += pkt.pktLen;
		}
		else{
			while(pBFT != NULL){
				if(cmpFlowTuple(&(pBFT->ft), &pkt.ft) == 0){
					pBFT->count_pkt ++;
					pBFT->count_size += pkt.pktLen;
					pkt.ft.tag = 0;
					break;
				}
				preBFT = pBFT;
				pBFT = pBFT->eNext;
			}
			if(pBFT == NULL){
				cpyFlowTuple(&(bigFlowTable_c[conflict_index].ft), &pkt.ft);
				pkt.ft.tag = 1;
				num_flow++;

				bigFlowTable_c[conflict_index].count_pkt = 1;
				bigFlowTable_c[conflict_index].count_size += pkt.pktLen;
				preBFT->eNext = &bigFlowTable_c[conflict_index];
				conflict_index +=1;
			}
		}
		fprintf(fp_pkt_tag, "%x\t%x\t%hd\t%hd\t%d\t%d\t%u\t%d\t%d\n", pkt.ft.src_ip, pkt.ft.dst_ip,
			pkt.ft.src_port,pkt.ft.dst_port, pkt.ft.proto, pkt.ft.tag, pkt.pktLen, pkt.tv.tv_sec, pkt.tv.tv_usec);

		num_pkt++;
		if((num_pkt >= MAX_NUM_PACKET) || (num_flow >= MAX_NUM_FLOW))
			break;
	}
	fclose(fp_pkt);
	fclose(fp_pkt_tag);
	printf("num_pkt analysised by bigFlowTable in analysisPcap.c:\t%d\n", num_pkt);
	return num_flow;
}

void calculateRelatedError(tFlowTable *flowTable, int num_flow, tBigFlowTable *bigFlowTable, int *num_error_flow, int *count_error){
	int i = 0;
	int hash_index;
	tBigFlowTable *pBFT;
	for(i = 0; i< num_flow; i++){
		hash_index = hash_5_tuple(&flowTable[i].ft);
		pBFT = &bigFlowTable[hash_index];
		while((pBFT) && (pBFT->count_pkt != 0)){
			if(cmpFlowTuple(&(pBFT->ft), &(flowTable[i].ft)) == 0){
				count_error[0] += uABS(pBFT->count_pkt, flowTable[i].count);
				if(uABS(pBFT->count_pkt, flowTable[i].count) !=0){
					num_error_flow[0] += 1;
					//printf("flowTable: %x\t%x\t%hd\t%hd\t%u\n", flowTable[i].ft.src_ip, flowTable[i].ft.dst_ip, flowTable[i].ft.src_port, 
					//	flowTable[i].ft.dst_port, flowTable[i].count);
					//printf("bigFlowTable: %x\t%x\t%hd\t%hd\t%u\n", pBFT->ft.src_ip, pBFT->ft.dst_ip, pBFT->ft.src_port, 
					//	pBFT->ft.dst_port, pBFT->count_pkt);
				}	
				count_error[1] += uABS(pBFT->count_size, flowTable[i].count_size);
				if(uABS(pBFT->count_size, flowTable[i].count_size) !=0)
					num_error_flow[1] += 1;
				
				break;
			}
			pBFT = pBFT->eNext;
		}
	}
}

void printBigFlowStatics(FILE *fp, tFlowTable *flowTable, int num_flow, tBigFlowTable *bigFlowTable){
	if((fp = fopen("result_bf.txt","w"))==NULL){
		printf("open result_bf.txt error in testHashPipe.c\n");
		exit(0);
	}
	int i = 0;
	int hash_index;
	tBigFlowTable *pBFT;
	for(i = 0; i< num_flow; i++){
		hash_index = hash_5_tuple(&flowTable[i].ft);
		pBFT = &bigFlowTable[hash_index];
		while((pBFT) && (pBFT->count_pkt != 0)){
			if(cmpFlowTuple(&(pBFT->ft), &(flowTable[i].ft)) == 0){
				fprintf(fp, "%d\t%x\t%x\t%hd\t%hd\t%u\n", i, pBFT->ft.src_ip, pBFT->ft.dst_ip,
					pBFT->ft.src_port, pBFT->ft.dst_port, pBFT->count_pkt);
				break;
			}
			pBFT = pBFT->eNext;
		}
	}
	fclose(fp);
}

void getTopFlowFromBigFlow(tBigFlowTable *bigFlowTable, tBigFlowTable *topFlowTable_bf, int num_top){
	int index_top, perFlow; 
	tBigFlowTable *headFlow, *maxFlow, *maxPreFlow;
	tBigFlowTable *pBFT = NULL;
	tBigFlowTable *preBFT = NULL;

	// relink all bigFlowTable;
	for(perFlow = 0; perFlow < NUM_BIG_FLOW_ENTRY; perFlow ++){
		pBFT = &bigFlowTable[perFlow];
		while(pBFT){
			if(pBFT->count_pkt == 0) break;
			else {
				if(preBFT == NULL) {
					headFlow = pBFT;
					preBFT = pBFT;
				}
				else{
					preBFT->eNext = pBFT;
					preBFT = pBFT;
				}
			}
			pBFT = pBFT->eNext;
		}
	}

	pBFT = headFlow;


	// assign headFlow_list to topFlowTable_bf;
	for(index_top = 0; index_top < num_top; index_top ++){
		int max = 0;
		pBFT = headFlow;
		preBFT = NULL;
		while(pBFT){
			if(pBFT->count_pkt > max){
				maxPreFlow = preBFT;
				maxFlow = pBFT;
				max = pBFT->count_pkt;
			}
			preBFT = pBFT;
			pBFT = pBFT->eNext;
		}
		cpyFlowTuple(&topFlowTable_bf[index_top].ft, &maxFlow->ft);
		topFlowTable_bf[index_top].count_pkt = maxFlow->count_pkt;
		topFlowTable_bf[index_top].count_size = maxFlow->count_size;
		if(maxPreFlow == NULL)
			headFlow = maxFlow->eNext;
		else
			maxPreFlow->eNext = maxFlow->eNext;
	}

}

void calculateRelatedError_topFLow(tHashEntry *headHashEntry, tBigFlowTable *topFlowTable_bf, int num_top, int *count_match){
	int perTopFlow;
	tHashEntry *hashEntry = headHashEntry;


//	for(perTopFlow = 0; perTopFlow < num_top; perTopFlow++){
//		printf("src_ip: %x, count_pkt: %u\n", topFlowTable_bf[perTopFlow].ft.src_ip, topFlowTable_bf[perTopFlow].count_pkt);
//	}
	int index_top;
	while(hashEntry){
		for(perTopFlow = 0; perTopFlow < num_top; perTopFlow++){
			if(cmpFlowTuple(&topFlowTable_bf[perTopFlow].ft, &hashEntry->ft) == 0)
				*count_match += 1;
		}
		hashEntry = hashEntry->eNext;
		index_top ++;
		if(index_top >= num_top)
			break;
	}
}