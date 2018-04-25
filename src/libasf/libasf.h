#ifndef LIBASF_H
#define LIBASF_H

#include <stdio.h>
#include "asfint.h"

#define MAXPACKETSIZE 100000
void writeInt32(int integer, FILE* fp);
void writeInt16(uint16_t integer, FILE* fp);
static void copyData(FILE* ifp, FILE* ofp, char* data, int size);
void UpdateIndexFile(uint32_t pid, uint16_t seq, uint32_t ts, uint32_t loc, FILE* idxfp);
void packetize(const char* input, const char* output, int interval);

#endif