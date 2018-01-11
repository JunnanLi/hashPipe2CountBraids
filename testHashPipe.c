#include "hash.h"
#include "common.h"
#include "taskHashPipe.h"
#include "readPcap.h"
#include "analysisPcap.h"
#include "taskCounterBraids.h"
#include "taskBloomFilter.h"

int main(){

	//hash initial
	initialHash();
	

	// fp_pcap used to read pcap;
	// fp_pkt used to record brief info. of each packet in .pcap
	// fp_pkt_tag used to add first_packet_tag in fp_pkt
	// fp_cb used to record countBraids statics;
	// fp_bf used to record real statics(big flow table);
	// fp_epoch used to record one epoch of fp_pkt_tag, e.g., 1s
	FILE *fp_pcap,*fp_pkt, *fp_pkt_tag,*fp_cb, *fp_bf, *fp_epoch;
	
	// num_pkt used to record the number of packets has been analysised (tcp packts) 
	int num_pkt = 0;

	// read .pcap and record brief info. in fp_pkt;  in the fromat of pktTuple
	readTrace(fp_pcap, fp_epoch);

	int perEpoch =0;
	for(perEpoch = 0; perEpoch < 10; perEpoch ++){
		struct pktTuple pkt;		// used to store pkt information read from fp_epoch;
		char file_name[100];
		sprintf(file_name, "./pkt_epoch/result_pkt_epoch_%d.txt", perEpoch);
		if((fp_epoch = fopen(file_name, "r"))==NULL){
			printf("read result_pkt_epoch.txt error in testHashPipe.c\n");
			exit(0);
		}
		if((fp_pkt = fopen("result_pkt.txt", "w"))==NULL){
			printf("write result_pkt.txt error in testHashPipe.c\n");
			exit(0);
		}
		while(fscanf(fp_epoch, "%x\t%x\t%hd\t%hd\t%hhu\t%u\t%d\t%d\n", &pkt.ft.src_ip, &pkt.ft.dst_ip,
				&pkt.ft.src_port, &pkt.ft.dst_port, &pkt.ft.proto, &pkt.pktLen, &pkt.tv.tv_sec, &pkt.tv.tv_usec) != EOF)
			fprintf(fp_pkt, "%x\t%x\t%hd\t%hd\t%hhu\t%u\t%d\t%d\n", pkt.ft.src_ip, pkt.ft.dst_ip,
				pkt.ft.src_port, pkt.ft.dst_port, pkt.ft.proto, pkt.pktLen, pkt.tv.tv_sec, pkt.tv.tv_usec);
		fclose(fp_pkt);

	//====================================bigFlowTable===================================//
		// analysis packe by big flow table;
		// bigFlowTable is a hash table, bigFlowTable_c is the confilict chain of hash table
		// topFlowTable is a sorted list including top K entries of bigFlowTable
		tBigFlowTable *bigFLowTable, *bigFLowTable_c, *topFlowTable_bf;
			// malloc;
		bigFLowTable = (tBigFlowTable *)malloc(NUM_BIG_FLOW_ENTRY*sizeof(tBigFlowTable));
		bigFLowTable_c = (tBigFlowTable *)malloc(NUM_BIG_FLOW_ENTRY*sizeof(tBigFlowTable));
		topFlowTable_bf = (tBigFlowTable *)malloc(TOP_K * sizeof(tBigFlowTable));
			// initial;
		analysisInitial(bigFLowTable, bigFLowTable_c);
			// analysis
		int num_flow_bigFlow = analysisPacket(fp_pkt, fp_pkt_tag, bigFLowTable, bigFLowTable_c);
	//====================================hashpipe=======================================//
		// hashpipe parameter
		// hashTable_hp is a muti-stage hash table
		// metadata_entry used to transmit the intermedia info. between hashPipe stages
		// aggHashTable aggreagtes the same entries in HashPipe, and it is a list that has been sorted
		tHashEntry *hashTable_hp[NUM_HASHPIPE_STAGE];
		tHashEntry *metadata_entry, *aggHashTable;
			// malloc
		int perStage;
		for(perStage = 0; perStage < NUM_HASHPIPE_STAGE; perStage ++)
			hashTable_hp[perStage] = (tHashEntry *)malloc(NUM_HASHPIPE_ENTRY*sizeof(tHashEntry));
		metadata_entry = (tHashEntry *)malloc(sizeof(tHashEntry));
		aggHashTable = NULL;	
			//initial
		intialHashPipe(hashTable_hp);
	//====================================bloomFilter======================================//
		//uint8 *bloomFilter;
		//bloomFilter = (uint8 *)malloc(NUM_BLOOMFILTER * sizeof(uint8));
		
		uint8 *bloomFilter[NUM_BLOOMFILTER_HASH];
		int perHash;
		for (perHash = 0; perHash < NUM_BLOOMFILTER_HASH; perHash++)
			bloomFilter[perHash] = (uint8 *)malloc(NUM_BLOOMFILTER * sizeof(uint8));
		
		intialBloomFilter(bloomFilter);
	//====================================countBraids=====================================//	
		// countBraids parameter;
		// hashTable and flowTable
		// hashTable Counter used to record each count of hash in hashTable
		tHashTable *hashTable, *hashTable_Layer2;
		tFlowTable *flowTable, *flowTable_Layer2;
		tCounter *hashTableCounter, *hashTableCounter_Layer2;
			// malloc;
		hashTable = (struct hashTable *)malloc(NUM_CONTER_1_LAYER*sizeof(struct hashTable));
		flowTable = (struct flowTable *)malloc(MAX_NUM_FLOW*sizeof(struct flowTable));
		hashTableCounter = (struct counter *)malloc(MAX_NUM_FLOW*NUM_HASH*sizeof(struct counter));
				// layer2
		hashTable_Layer2 = (struct hashTable *)malloc(NUM_CONTER_2_LAYER*sizeof(struct hashTable));
		flowTable_Layer2 = (struct flowTable *)malloc(NUM_CONTER_1_LAYER*sizeof(struct flowTable));
		hashTableCounter_Layer2 = (struct counter *)malloc(NUM_CONTER_1_LAYER*NUM_HASH*sizeof(struct counter));
			// initial;
		initialCounterBraids(hashTable, flowTable);
		initialCounterBraids_Layer2(hashTable_Layer2, flowTable_Layer2);
			// index used to allocate flowTable
		int index_flowTable = 0;
		int index_flowTable_Layer2 = 0;
	//====================================process=======================================//
	// read pkt from pacap		---->	updateHashPipe  	---->	updateCounterBraids
	//							 	 |
	//								+-->	lookupBloomFilter	----->	addFlow
	// can be offload to FPGA
		// read pkt from pcap;
		if((fp_pkt_tag = fopen("result_pkt_2.txt", "r"))==NULL){
			printf("read result_pkt_2.txt error in testHashPipe.c\n");
			exit(0);
		}
		
		int not_smallest_kickout = 0;	// used to statics the num of kickouted flow which is not the smallest count;
		int count_match = 0;		// used to store the num of heavy key matched with BigFlowTable;
		int min_pkt = 0;		// used to store the minimun value of hashPipe
		//int num_entry_hp = 0;	// used to statics the num of entries (not empty) in hashPipe;
		int num_tag_kickout = 0;	// used to statics the num of flow needed to be added to the flowTable;
		int num_kickout = 0;		// used to statics the count of updating the CounterBraids;
	 	int bf_error = 0;		// used to statics the count of bf_error;
	 	int bf_tag = 0;

		while(fscanf(fp_pkt_tag, "%x\t%x\t%hd\t%hd\t%hhu\t%hhu\t%u\t%d\t%d\n", &pkt.ft.src_ip, &pkt.ft.dst_ip,
				&pkt.ft.src_port, &pkt.ft.dst_port, &pkt.ft.proto, &pkt.ft.tag, &pkt.pktLen, &pkt.tv.tv_sec, &pkt.tv.tv_usec) != EOF){
			if(pkt.ft.src_ip == 0)
				break;
			if(pkt.ft.proto != 0x6) 
				continue;
				
			// update HashPipe
			if(updateHashPipe(hashTable_hp, &pkt, metadata_entry, &min_pkt) == 1){
				not_smallest_kickout += 1;
			}
			// update counterBraids
			if(metadata_entry->valid == 1){
				// lookup bloom filter
				if(metadata_entry->ft.tag == 1)
					bf_tag = 1;
				else
					bf_tag = 0;
				if(lookupBloomFilter(&metadata_entry->ft, bloomFilter) == 0){
				//if(metadata_entry->ft.tag == 1){
					addFlow(flowTable, &index_flowTable, &metadata_entry->ft);
					num_tag_kickout += 1;
					bf_tag = 0;
				}
				updateCounterBraids(hashTable, &metadata_entry->ft, metadata_entry->count_pkt, metadata_entry->count_size, hashTable_Layer2, &index_flowTable_Layer2, flowTable_Layer2);
				num_kickout += 1;
				bf_error += bf_tag;
			}

			num_pkt++;
			//if(num_pkt % 100000 == 0) printf("%d\n", num_pkt/10000);
		}

		//printHashPipe(hashTable_hp);

	//====================================decode=======================================//
		// aggregate hashPipe 
		// sortHashTable = sortHashTable(hashTable_hp, TOP_K);
		aggHashTable = sortHashTable(hashTable_hp, MAX_ENTRY);

		// decode CounterBraids for hashTable_Layer2 and flowTable_Layer2;
		//decodeCounterBraids(hashTable_Layer2, flowTable_Layer2, index_flowTable_Layer2, hashTableCounter_Layer2, NUM_CONTER_2_LAYER);
		//changeFlowTableToHashTable_Layer2(hashTable, flowTable_Layer2, index_flowTable_Layer2);
		// decode CounterBraids for hashTable and flowTable;
		decodeCounterBraids(hashTable, flowTable, index_flowTable, hashTableCounter, NUM_CONTER_1_LAYER);

		// calculate the final result by aggregateHashTableWithFlowTable
		aggregateHashTableWithFlowTable(aggHashTable, flowTable, &index_flowTable);
	//====================================analysis=======================================//

		//printFlowStatics(fp_cb, flowTable, index_flowTable);
		//printBigFlowStatics(fp_bf, flowTable, index_flowTable, bigFLowTable);
		int num_error_flow[2] = {0};	// {count_pkt, count_size}
		int count_error[2] ={0};	// {count_pkt, count_size}

		calculateRelatedError(flowTable, index_flowTable, bigFLowTable, num_error_flow, count_error);
		printf("===========hashPipe_info===========\n");
		printf("num_kickout:%d\n", num_kickout);
		printf("num_tag_kickout:%d\n", num_tag_kickout);
		printf("===========count_pkt===========\n");
		printf("num_error:%d\n", count_error[0]);
		printf("num_error_flow:%d\n", num_error_flow[0] + bf_error);
		printf("===========count_size===========\n");
		printf("num_error:%d\n", count_error[1]);
		printf("num_error_flow:%d\n", num_error_flow[1]);
		printf("===========flow infomation===========\n");
		printf("num_flow:%d\n", index_flowTable);
		printf("num_flow_bigFlow:%d\n", num_flow_bigFlow);
		printf("num_pkt:%d\n", num_pkt);

	// analysis hashPipe
		getTopFlowFromBigFlow(bigFLowTable, topFlowTable_bf, TOP_K);
		calculateRelatedError_topFLow(aggHashTable, topFlowTable_bf, TOP_K, &count_match);
		
	//print related information on hashPipe
		printf("===========hashPipe result===========\n");
		printf("num_notSmallest_kicked out: %d\n", not_smallest_kickout);
		printf("num_matched_top_k: %d\n", count_match);


		FILE *fp_result, *fp_result_2, *fp_result_3;
		if((fp_result = fopen("result.txt","a+"))==NULL){
			printf("open result.txt error\n");
			exit(0);
		}
		
		if((fp_result_2 = fopen("result.txt_ljn","a+"))==NULL){
			printf("open result.txt error\n");
			exit(0);
		}
		if((fp_result_3 = fopen("result.txt_ljn_2","a+"))==NULL){
			printf("open result.txt error\n");
			exit(0);
		}
		
		//fprintf(fp_result, "======%d=====hashPipe_info stage: %d\tentry: %d===========\n", TIME_USEC, NUM_HASHPIPE_STAGE, NUM_HASHPIPE_ENTRY);
		//fprintf(fp_result, "%d\n", num_kickout);
		//fprintf(fp_result, "%d\n", num_tag_kickout);
		//fprintf(fp_result, "%d\n", count_error[0]);
		//fprintf(fp_result, "%d\n", num_error_flow[0]);
		fprintf(fp_result_3, "%d\n", num_error_flow[0] + bf_error);
		//fprintf(fp_result, "%d\n", count_error[1]);
		//fprintf(fp_result, "%d\n", num_error_flow[1]);
		//fprintf(fp_result, "%d\n", index_flowTable);
		//fprintf(fp_result, "%d\n", num_flow_bigFlow);
		//fprintf(fp_result,"%d\n", count_match);
		fprintf(fp_result_2, "%d\n", bf_error);
		//fprintf(fp_result, "%d\n", num_pkt);

		fclose(fp_result);
		fclose(fp_result_2);
		fclose(fp_result_3);
		fclose(fp_pkt_tag);
		//printhashEntry(aggHashTable);
		
		/*
		free(bigFLowTable);
		free(bigFLowTable_c);
		free(topFlowTable_bf);
		free(hashTable_hp);
		free(metadata_entry);
		free(aggHashTable);
		free(hashTable);
		free(hashTable_Layer2);
		free(flowTable);
		free(flowTable_Layer2);
		free(hashTableCounter);
		free(hashTableCounter_Layer2);
		*/
	
	}




	return 0;
}