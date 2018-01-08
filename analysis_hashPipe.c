# include "hashPipe.h"
# include "bigFlowTable.h"

int main()
{
	struct flow_tuple *pktTuple;
	struct  hash_entry hashTable[NUM_STAGE][NUM_ENTRY];
	struct hash_entry *metaEntry, *sortEntry;
	struct space_saving_flow_entry * hFlowEntry, *cFlowEntry;
	//malloc
	pktTuple = (struct flow_tuple *)malloc(sizeof(struct flow_tuple));
	metaEntry = (struct hash_entry *)malloc(sizeof(struct hash_entry));
	sortEntry = (struct hash_entry *)malloc(TABLESIZE*sizeof(struct hash_entry));
	hFlowEntry = (struct space_saving_flow_entry *)malloc(TABLESIZE*sizeof(struct space_saving_flow_entry));
	cFlowEntry = (struct space_saving_flow_entry *)malloc(TABLESIZE*sizeof(struct space_saving_flow_entry));

	hashPipe_intial(hashTable);

	FILE * fp_trace, *fp_top300, *fp_flowSequence;
	if((fp_trace = fopen("output_0.txt","r")) == NULL)
	{
		printf("error: can not open output.txt file\n");
		exit(0);
	}
	if((fp_top300 = fopen("top300.txt","w")) == NULL)
	{
		printf("error: can not open top300 file\n");
		exit(0);
	}
	if((fp_flowSequence = fopen("flowSequence.txt","w")) == NULL)
	{
		printf("error: can not open flowSequence file\n");
		exit(0);
	}


	//read packet and send it to hashPipe;
	int i, pkt_length;
	int Cindex = 0;
	int num_overFlow = 0;
	while(fscanf(fp_trace,"%d\t%x\t%x\t%hd\t%hd%d\n",&i, &(pktTuple->SrcIP),&(pktTuple->DstIP),&(pktTuple->SrcPort),&(pktTuple->DstPort), &pkt_length ) != EOF){
		hashPipe_lookup(hashTable, pktTuple, metaEntry);
		analysis_big_flow_table(hFlowEntry, cFlowEntry, pktTuple, &Cindex, &num_overFlow);
	}

	int numEntry = hashPipe_sort(hashTable, sortEntry);
	big_flow_table_sort(hFlowEntry, cFlowEntry, fp_flowSequence);

	for(i=0; (i < numEntry) && (i < TABLESIZE); i++){
		fprintf(fp_top300, "%d\t%x\t%x\t%hd\t%hd\t%u\n", i, sortEntry[i].FT.SrcIP,sortEntry[i].FT.DstIP,sortEntry[i].FT.SrcPort,sortEntry[i].FT.DstPort,sortEntry[i].count_packet);
	}



	fclose(fp_trace);
	fclose(fp_top300);
	fclose(fp_flowSequence);


	return 0;
}
