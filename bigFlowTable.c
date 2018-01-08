# include "bigFlowTable.h"

u_int32 hash_5_tuple(struct flow_tuple *ft){
	u_int32 hash_index =0;
	hash_index = (ft->SrcIP + ft->DstIP + (u_int32) ft->SrcPort + (u_int32) ft->DstPort)% (u_int32) 9999997;
	return hash_index;
}


void analysis_big_flow_table(struct space_saving_flow_entry *hFlowEntry, struct space_saving_flow_entry *cFlowEntry,struct flow_tuple *pktTuple, int *Cindex, int *num_overFlow){
	struct space_saving_flow_entry *nFlowEntry, *mFlowEntry, *preFlowEntry;

	// initial
	int i;
	for(i=0; i < TABLESIZE; i++){
		hFlowEntry[i].count_packet =0;
		hFlowEntry[i].eNext = NULL;
	}
	int Hindex = hash_5_tuple(pktTuple);

	nFlowEntry = &hFlowEntry[Hindex];
	while(nFlowEntry){
		if(nFlowEntry->count_packet == 0){
			nFlowEntry->FT.SrcIP = pktTuple->SrcPort;
			nFlowEntry->FT.DstIP = pktTuple->DstIP;
			nFlowEntry->FT.SrcPort = pktTuple->SrcPort;
			nFlowEntry->FT.DstPort = pktTuple->DstPort;
			nFlowEntry->count_packet = 1;
			nFlowEntry->eNext = NULL;
			break;
		}
		else if((nFlowEntry->FT.SrcIP = pktTuple->SrcIP) &&
			(nFlowEntry->FT.DstIP = pktTuple->DstIP) &&
			(nFlowEntry->FT.SrcPort = pktTuple->SrcPort) &&
			(nFlowEntry->FT.DstPort = pktTuple->DstPort)){

			nFlowEntry->count_packet ++;
			break;
		}
		else if(nFlowEntry->eNext == NULL){
			if(*Cindex < TABLESIZE -2){
				cFlowEntry[*Cindex].FT.SrcIP = pktTuple.SrcIP;
				cFlowEntry[*Cindex].FT.DstIP = pktTuple.DstIP;
				cFlowEntry[*Cindex].FT.SrcPort = pktTuple.SrcPort;
				cFlowEntry[*Cindex].FT.DstPort = pktTuple.DstPort;
				cFlowEntry[*Cindex].count_packet = 1;
				cFlowEntry[*Cindex].eNext = NULL:
				nFlowEntry->eNext = cFlowEntry[*Cindex];
				*Cindex+= 1;
			}
			else *num_overFlow +=1;
			break;
		}
		else nFlowEntry = nFlowEntry->eNext;
	}
}


int big_flow_table_sort(struct space_saving_flow_entry *hFlowEntry,struct space_saving_flow_entry *cFlowEntry, FILE *fp_flowSequence){
	int i=0;
	struct space_saving_flow_entry * headFLowEntry, *nFlowEntry, *preFlowEntry, *mFlowEntry, *sortFlowEntry;
	//malloc
	sortFlowEntry = (struct space_saving_flow_entry *)malloc(TABLESIZE*sizeof(struct space_saving_flow_entry));

	int Sindex = 0;
	// find an exist entry as the headFlowEntry;
	for(i=0; i < TABLESIZE; i++){
		if(hFlowEntry[i].count_packet > 0){
			nFlowEntry = &hFlowEntry[i];
			while(nFlowEntry){
				sortFlowEntry[Sindex].FT.SrcIP = nFlowEntry->FT.SrcIP;
				sortFlowEntry[Sindex].FT.DstIP = nFlowEntry->FT.DstIP;
				sortFlowEntry[Sindex].FT.SrcPort = nFlowEntry->FT.SrcPort;
				sortFlowEntry[Sindex].FT.DstPort = nFlowEntry->FT.DstPort;
				sortFlowEntry[Sindex].count_packet = nFlowEntry->count_packet;
				sortFlowEntry[Sindex].eNext = NULL;
				Sindex++;
				nFlowEntry = nFlowEntry->eNext;
			}
		}
	}

	// sorting;
	headFlowEntry = sortFlowEntry;
	for(i=1; i< Sindex; i++){
		mFlowEntry = &sortFlowEntry[i];
		preFlowEntry = NULL;
		nFlowEntry = headFlowEntry;
		while(mFlowEntry){
			if(mFlowEntry->count_packet < nFlowEntry->count_packet){
				preFlowEntry = nFlowEntry;
				nFlowEntry = nFlowEntry->eNext;
			}
			else if(preFlowEntry == NULL){
				headFlowEntry = mFlowEntry;
				mFlowEntry->eNext = nFlowEntry;
			}
			else {
				preFlowEntry->eNext = mFlowEntry;
				mFlowEntry->eNext = nFlowEntry;
			}

		}
	}
	nFlowEntry = headFlowEntry;
	i=0;
	while(nFlowEntry){
		fprintf(fp_flowSequence, "%d\t%x\t%x\t%hd\t%hd\t%u\n",i++, nFlowEntry->FT.SrcIP,nFlowEntry->FT.DstIP,nFlowEntry->FT.SrcPort,nFlowEntry->FT.DstPort, nFlowEntry->count_packet);
	}


	return Sindex;
}


