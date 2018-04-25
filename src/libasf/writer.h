#include "asfint.h"
#include "asf.h"

int writeHeader(asf_object_header_t* header, FILE* file){
	uint8_t hdata[30];
	uint64_t ui64;
	uint32_t ui32;
	uint16_t ui16;
	int i;
	ui32 = header->guid.v1;
	for (i=0; i<4; i++)
	{	
		hdata[i] = ui32 & 0x000f;
		ui32 >>= 8;
	}

	ui16 = header->guid.v2;
	hdata[i] = ui16 & 0x0f;  // i = 4
	ui16 >>= 8;
	hdata[i+1] = ui16 &0x0f;
	i += 2;
	
	ui16 = header->guid.v3;
	ui16 >>= 8;
	hdata[i+1] = ui16 &0x0f;
	i += 2;

	for (; i<16; i++)
	{	
		hdata[i] = header->guid.v4[i-8]; // i=
	}
}