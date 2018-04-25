#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libasf.h"
#include "asf.h"
#include "asfint.h"

#define BIGPACKETSIZE 1
#define NEW_PREROLL 3000 // in ms
#define CHANGE_PREROLL

void
writeInt32(int integer, FILE* fp){
	int i;
	char charArray[4];
	for (i=3; i>=0; i--)
	{
		charArray[i] = integer & 0x000000ff;
		integer >>= 8;
	}
	for (i=0; i<4; i++)
		fputc(charArray[i], fp);
}

void
writeLittleInt32(int integer, FILE* fp){
	int i;
	char ch;
	for (i=0; i<4; i++)
		ch = integer & 0x000000ff;
		fputc(ch, fp);
		integer >>= 8;
}

void
writeInt16(uint16_t integer, FILE* fp){
	int i;
	char charArray[2];
	for (i=1; i>=0; i--)
	{
		charArray[i] = integer & 0x00ff;
		integer >>= 8;
	}
	for (i=0; i<2; i++)
		fputc(charArray[i], fp);
}

void
writeLittleInt16(uint16_t integer, FILE* fp){
	int i;
	char ch;
	for (i=0; i<2; i++)
		ch = integer & 0x00ff;
		fputc(ch, fp);
		integer >>= 8;
}

void
writeLittleInt64(uint64_t integer, FILE* fp){
	int i;
	char ch;
	for (i=0; i<8; i++){
		ch = integer & 0x00ff;
		fputc(ch, fp);
		//printf("%c \t", ch);
		integer >>= 8;
	}
}


uint64_t readLittleInt64(FILE* fp){
	int i;
	uint64_t ch;
	uint64_t int64;
	int64 = 0;
	for (i=0; i<8; i++){
		ch = fgetc(fp);
		ch <<= i * 8;
		int64 |= ch;
	}
	return int64;
}



static void
copyData(FILE* ifp, FILE* ofp, char* data, int size){
	int curr_size = size;
	int temp;
	if (size < MAXPACKETSIZE){
		temp = fread(data, 1, size, ifp);
		if (temp != size){
			printf("Reading error\n");
			exit(1);
		}
		temp = fwrite(data, 1, size, ofp);
		if (temp != size){
			printf("Writing error\n");
			exit(1);
		}
		return;
	}

	while (curr_size > MAXPACKETSIZE){
		temp = fread(data, 1, MAXPACKETSIZE, ifp);
		if (temp != MAXPACKETSIZE){
			printf("Reading error\n");
			exit(1);
		}
		temp = fwrite(data, 1, MAXPACKETSIZE, ofp);
		if (temp != MAXPACKETSIZE){
			printf("Writing error\n");
			exit(1);
		}
		curr_size = curr_size - MAXPACKETSIZE;
		if (curr_size == 0)
			return;
	}
	temp = fread(data, 1, curr_size, ifp);
	if (temp != curr_size){
		printf("Reading error\n");
		exit(1);
	}
	temp = fwrite(data, 1, curr_size, ofp);
	if (temp != curr_size){
		printf("Reading error\n");
		exit(1);
	}
}


void UpdateIndexFile(uint32_t pid, uint16_t seq, uint32_t ts, uint32_t loc, FILE* idxfp){
	writeInt32(pid, idxfp);
	writeInt16(seq, idxfp);
	writeInt32(ts, idxfp);
	writeInt32(loc, idxfp);
}

void packetize(const char* input, const char* output, int interval){
	asf_file_t *file;
	asf_metadata_t *metadata;
	asf_packet_t *pkt;
	//uint64_t mSecPlaying;
	uint64_t length;
	FILE *pktOutputFile;
	FILE *idxOutputFile;
	FILE *pktNullOutputFile;
	FILE *idxNullOutputFile;
	FILE* asfInputFile;
	FILE* headerOutputFile;
	FILE *dataOutputFile;
	FILE *tmpOutputFile;
	uint64_t header_size;
	int pktLength;
	struct test * newtest;
	char* strOutFileName;
	char* strPktOutFileName;
	char* strIdxOutFileName;
	char* strPktNullOutFileName;
	char* strIdxNullOutFileName;
	char* strHeaderOutFileName;
	char* strDataOutFileName;
	int i, j;
	int strLength;
	char packetdata[MAXPACKETSIZE+1];
	int location;
	int packetNumber;
	uint64_t prev_position, curr_position;
	int temp;
	uint64_t pkt_size;
	uint64_t  packet_no, position;
	short isPktWritten;
	int tmp;
	uint64_t timestamp;
	int pkt_loc;
	int pktid;
	int prev_location;
	int prev_pktnum;
	int count ;
	uint64_t *preroll_pos;
	uint64_t preroll;
	uint64_t preroll_offset;
	file = asf_open_file(input);
	if (!file) {
		printf("Error opening file %s\n", input);
		return -1;
	}
	preroll_pos = malloc(sizeof(uint64_t));
	asf_init(file, preroll_pos);
	*preroll_pos += 80;
	header_size = file->position;
	metadata = asf_get_metadata(file);
	if (metadata) {
		//print_metadata(metadata);
		asf_free_metadata(metadata);
	}
	//printf("File Properties position: %u \n", *preroll_pos);
	//printf("Play Duration: %d\n", file->play_duration);
	//printf("Send Duration: %d\n", file->send_duration);
	
	//mSecPlaying = file->play_duration / 20000; // 100-nano second unit converted to mSec and divided by 2
	//printf("Seeking to: %d\n", mSecPlaying);
	//asf_seek_to_msec(file, mSecPlaying);
	
	pkt = asf_packet_create();
	
	
	strOutFileName = malloc(strlen(input)+5);
	strPktOutFileName = malloc(strlen(input)+5);
	strIdxOutFileName = malloc(strlen(input)+5);
	strPktNullOutFileName = malloc(strlen(input)+5);
	strIdxNullOutFileName = malloc(strlen(input)+5);
	strHeaderOutFileName = malloc(strlen(input)+12);
	strDataOutFileName = malloc(strlen(input)+6);
	for (i=strlen(input)-1; i>=0; i--)
		if (input[i] == '.')
			break;
	if (i > 0){
		strncpy(strOutFileName, input, i);
		strOutFileName[i] = '\0';
	}
	else
		strcpy(strOutFileName, input);
	strcpy(strPktOutFileName,strOutFileName);
	strcpy(strIdxOutFileName,strOutFileName);
	strcpy(strPktNullOutFileName,strOutFileName);
	strcpy(strIdxNullOutFileName,strOutFileName);
	strcpy(strHeaderOutFileName, strOutFileName);
	strcpy(strDataOutFileName, strOutFileName);
	strcat(strPktOutFileName, ".aud");
	strcat(strIdxOutFileName, ".ain");
	strcat(strPktNullOutFileName, ".vid");
	strcat(strIdxNullOutFileName, ".vin");
	strcat(strHeaderOutFileName, "_header.asf");
	strcat(strDataOutFileName, ".data");

	pktOutputFile = fopen(strPktOutFileName, "wb");
	idxOutputFile = fopen(strIdxOutFileName, "wb");
	pktNullOutputFile = fopen(strPktNullOutFileName, "wb");
	idxNullOutputFile = fopen(strIdxNullOutFileName, "wb");
	//headerOutputFile = fopen(strHeaderOutFileName, "wb");
	dataOutputFile = fopen(strDataOutFileName, "wb");
	tmpOutputFile = tmpfile();
	asfInputFile = fopen(input, "rb"); 

	packetNumber = 0;


	// Copy and modify header 
#ifdef CHANGE_PREROLL
	copyData(asfInputFile, tmpOutputFile, packetdata, *preroll_pos);
	preroll = readLittleInt64(asfInputFile);
	preroll_offset = 0;
	//printf("preroll  = %d \n", preroll);
	if (preroll > NEW_PREROLL)
		preroll_offset = preroll - NEW_PREROLL;
	writeLittleInt64(NEW_PREROLL, tmpOutputFile);
	copyData(asfInputFile, tmpOutputFile, packetdata, header_size - *preroll_pos - 8);
	//printf("preroll offset = %d \n", preroll_offset);

	writeLittleInt64(preroll_offset, dataOutputFile);
	rewind(tmpOutputFile);
	copyData(tmpOutputFile, dataOutputFile, packetdata, header_size);
#else
	writeLittleInt64(0, dataOutputFile);
	copyData(asfInputFile, dataOutputFile, packetdata, header_size);
#endif

	count = 0;
	writeInt32(file->packet_size * BIGPACKETSIZE, pktOutputFile);
	// Copy all packets data
	while (asf_get_packet(file, pkt)){
		if (count == BIGPACKETSIZE){
			writeInt32(file->packet_size * BIGPACKETSIZE, pktOutputFile);
			count = 0;
		}
		copyData(asfInputFile, pktOutputFile, packetdata, file->packet_size);
		count++;
	}


	// Write Packet Zero
	UpdateIndexFile(0 ,0, 0, 4, idxOutputFile);
	//writeInt32(0, dataOutputFile);	

	prev_position = header_size;
	curr_position = header_size;

	timestamp = 0;
	location = 4;
	pktid = 1;
	prev_location = location;
	prev_pktnum = 0;
	for (i=1; ; i++)
	{
		
		if ((tmp = asf_seek_to_msec(file, (uint64_t) i*1000)) < 0) 
			break;
		//printf("Packet = %d \n", file->packet);
		if (tmp == prev_position){
			//UpdateIndexFile(i ,0, pkt->send_time, location, idxOutputFile);
			continue;
		} 
		
		

		prev_position = tmp;
		location = (file->packet / BIGPACKETSIZE) * ( file->packet_size * BIGPACKETSIZE + 4) + 4;
		packetNumber = file->packet / BIGPACKETSIZE;
		if (packetNumber == 1)
			packetNumber = 0;
		
		if ((tmp = asf_get_packet(file, pkt)) < 0) {
			printf("Error %d getting packet\n", tmp);
			break;
		}

		if (pkt->send_time == 0 && pkt->duration ==0)
			continue;
		if (!tmp)
			break;
		if (pkt->send_time % 1000 == 0)
			tmp = pkt->send_time/1000;
		else tmp = pkt->send_time / 1000 + 1;

		if (timestamp < 1000){
			timestamp = 0;
			prev_location = 4;
		}
		for (j=pktid; j< tmp; j++)
			UpdateIndexFile(prev_pktnum ,j, timestamp, prev_location, idxOutputFile);
		prev_location = location;
		prev_pktnum = packetNumber;
		timestamp = pkt->send_time;
		
		pktid = tmp;
		
	}
	for (j=pktid; j<=  (file->play_duration / 10000000); j++)
		UpdateIndexFile(prev_pktnum , j, timestamp, prev_location, idxOutputFile);
	//printf("Index position %d \n", file->data_position + file->data->size);
	fclose(pktOutputFile);
	fclose(idxOutputFile);
	fclose(pktNullOutputFile);
	fclose(idxNullOutputFile);
	fclose(asfInputFile);
	fclose(dataOutputFile);
	fclose(tmpOutputFile);
	free(preroll_pos);
	free(strOutFileName);
	free(strPktOutFileName);
	free(strIdxOutFileName);
	free(strPktNullOutFileName);
	free(strIdxNullOutFileName);
	free(strHeaderOutFileName);
	free(strDataOutFileName);

	asf_free_packet(pkt);

	asf_close(file);
}