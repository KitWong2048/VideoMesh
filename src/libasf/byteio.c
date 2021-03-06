
#include <string.h>
#include <stdio.h>
#include "byteio.h"

uint16_t
asf_byteio_getWLE(uint8_t *data)
{
	uint16_t ret;
	int i;

	for (i=1, ret=0; i>=0; i--) {
		ret <<= 8;
		ret |= data[i];
	}

	return ret;
}

uint32_t
asf_byteio_getDWLE(uint8_t *data)
{
	uint32_t ret;
	int i;

	for (i=3, ret=0; i>=0; i--) {
		ret <<= 8;
		ret |= data[i];
	}

	return ret;
}

uint64_t
asf_byteio_getQWLE(uint8_t *data)
{
	uint64_t ret;
	int i;

	for (i=7, ret=0; i>=0; i--) {
		ret <<= 8;
		ret |= data[i];
	}

	return ret;
}

void
asf_byteio_getGUID(guid_t *guid, uint8_t *data)
{
	guid->v1 = asf_byteio_getDWLE(data);
	guid->v2 = asf_byteio_getWLE(data + 4);
	guid->v3 = asf_byteio_getWLE(data + 6);
	memcpy(guid->v4, data + 8, 8);
}

void
asf_byteio_get_string(uint16_t *string, uint16_t strlen, uint8_t *data)
{
	int i;

	for (i=0; i<strlen; i++) {
		string[i] = asf_byteio_getWLE(data + i*2);
	}
}

int
asf_byteio_readbyte(asf_stream_t *stream)
{
	uint8_t byte;
	int ret;

	if ((ret = asf_byteio_read(&byte, 1, stream)) <= 0) {
		printf("ret: %d\n",ret);
		return (ret == 0) ? ASF_ERROR_EOF : ASF_ERROR_IO;
	}

	return byte;
}

int
asf_byteio_read(uint8_t *data, int size, asf_stream_t *stream)
{
	int read = 0, tmp;
//	long length=0, cur_pos=0;

	if (!stream->read) {
		return ASF_ERROR_INTERNAL;
	}

//	printf("size: %d\n",size);
//	printf("current pos: %ld\n",ftell(stream->opaque));
//	cur_pos = ftell(stream->opaque);

//	fseek(stream->opaque,0,SEEK_END);
//	length = ftell(stream->opaque);
//	fseek(stream->opaque,cur_pos,SEEK_SET);
//	printf("file size: %ld\n",length);
//	tmp = stream->read(stream->opaque, data, size);
//	tmp = fread(data, 1, size, stream->opaque);
//	printf("current pos: %ld\n",ftell(stream->opaque));
//	printf("tmp: %d\n",tmp);
//	return size;
	while ((tmp = stream->read(stream->opaque, data+read, size-read)) > 0) {
		read += tmp;

//		printf("tmp: %d\n",tmp);
		if (read == size) {
			return read;
		}
	}
//	printf("tmp: %d\n",tmp);
	return (tmp == 0) ? ASF_ERROR_EOF : ASF_ERROR_IO;
}

