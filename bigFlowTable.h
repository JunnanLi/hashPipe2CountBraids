#ifndef __BIGFLOWTABLE_H
#define __BIGFLOWTABLE_H

#include "hashPipe.h"

#define TABLESIZE 10000000




struct space_saving_flow_entry
{
	struct flow_tuple FT;
	u_int32 count_packet;
	struct space_saving_flow_entry *eNext;
};




u_int32 hash_5_tuple(struct flow_tuple *ft);

void analysis_big_flow_table(struct space_saving_flow_entry *hFlowEntry, struct space_saving_flow_entry *cFlowEntry, struct flow_tuple *pktTuple, int *Cindex, int *num_overFLow);
int big_flow_table_sort(struct space_saving_flow_entry *hFlowEntry,struct space_saving_flow_entry *cFlowEntry, FILE *fp_flowSequence);

#endif