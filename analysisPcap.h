#ifndef __ANALYSIS_PCAP_H_
#define __ANALYSIS_PCAP_H_

#include "common.h"
#include "taskHashPipe.h"


#define NUM_BIG_FLOW_ENTRY 10000000
#define PRIME 9999991


typedef struct big_flow_table{
	struct flowTuple ft;
	uint32 count_pkt;
	uint32 count_size;
	struct big_flow_table *eNext;
}tBigFlowTable;


uint32 hash_5_tuple(struct flowTuple *pkt);

void analysisInitial(tBigFlowTable *bigFlowTable, tBigFlowTable *bigFlowTable_c);
// return count of flows
int analysisPacket(FILE *fp_pkt, FILE *fp_pkt_tag, tBigFlowTable *bigFlowTable, tBigFlowTable *bigFlowTable_c);

void calculateRelatedError(tFlowTable *flowTable, int num_flow, tBigFlowTable *bigFlowTable, int * num_error_flow, int *count_error);

//test
void printBigFlowStatics(FILE *fp, tFlowTable *flowTable, int num_flow, tBigFlowTable *bigFlowTable);

void getTopFlowFromBigFlow(tBigFlowTable *bigFlowTable, tBigFlowTable *topFlowTable_bf, int num_top);

void calculateRelatedError_topFLow(tHashEntry *headHashEntry, tBigFlowTable *topFlowTable_bf, int num_top, int *count_match);
#endif