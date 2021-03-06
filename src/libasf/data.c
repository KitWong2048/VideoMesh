
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "asf.h"
#include "asfint.h"
#include "byteio.h"
#include "data.h"
#include "parse.h"

#define GETLEN2b(bits) (((bits) == 0x03) ? 4 : bits)

#define GETVALUE2b(bits, data) \
	(((bits) != 0x03) ? ((bits) != 0x02) ? ((bits) != 0x01) ? \
	 0 : *(data) : asf_byteio_getWLE(data) : asf_byteio_getDWLE(data))

static int
asf_data_read_packet_data(asf_packet_t *packet, uint8_t flags, asf_stream_t *stream)
{
	uint8_t datalen;
	uint8_t data[18];
	uint8_t *datap;
	int tmp;

	datalen = GETLEN2b((flags >> 1) & 0x03) +
	          GETLEN2b((flags >> 3) & 0x03) +
	          GETLEN2b((flags >> 5) & 0x03) + 6;

	if ((tmp = asf_byteio_read(data, datalen, stream)) < 0) {
		return tmp;
	}

	datap = data;
	packet->length = GETVALUE2b((flags >> 5) & 0x03, datap);
	datap += GETLEN2b((flags >> 5) & 0x03);
	/* sequence value is not used */
	GETVALUE2b((flags >> 1) & 0x03, datap);
	datap += GETLEN2b((flags >> 1) & 0x03);
	packet->padding_length = GETVALUE2b((flags >> 3) & 0x03, datap);
	datap += GETLEN2b((flags >> 3) & 0x03);
	packet->send_time = asf_byteio_getDWLE(datap);
	datap += 4;
	packet->duration = asf_byteio_getWLE(datap);
	datap += 2;

	return datalen;
}

static int
asf_data_read_payload_data(asf_payload_t *payload, uint8_t flags, uint8_t *data, int size)
{
	uint8_t datalen;

	datalen = GETLEN2b(flags & 0x03) +
	          GETLEN2b((flags >> 2) & 0x03) +
	          GETLEN2b((flags >> 4) & 0x03);

	if (datalen > size) {
		return ASF_ERROR_INVALID_LENGTH;
	}

	payload->media_object_number = GETVALUE2b((flags >> 4) & 0x03, data);
	data += GETLEN2b((flags >> 4) & 0x03);
	payload->media_object_offset = GETVALUE2b((flags >> 2) & 0x03, data);
	data += GETLEN2b((flags >> 2) & 0x03);
	payload->replicated_length = GETVALUE2b(flags & 0x03, data);
	data += GETLEN2b(flags & 0x03);

	return datalen;
}

static int
asf_data_read_payloads(asf_packet_t *packet,
                       uint64_t preroll,
                       uint8_t multiple,
                       uint8_t type,
                       uint8_t flags,
                       uint8_t *data,
                       uint32_t datalen)
{
	asf_payload_t pl;
	int i, tmp, skip;

	skip = 0;
	for (i=0; i<packet->payload_count; i++) {
		uint8_t pts_delta = 0;
		int compressed = 0;

		/* FIXME: mark keyframe? */
		pl.stream_number = data[skip] & 0x7f;
		skip++;

		tmp = asf_data_read_payload_data(&pl, flags, data + skip, datalen - skip);
		if (tmp < 0) {
			return tmp;
		}
		skip += tmp;

		if (pl.replicated_length > 1) {
			if (pl.replicated_length < 8 || pl.replicated_length + skip > datalen) {
				/* not enough data */
				return ASF_ERROR_INVALID_LENGTH;
			}
			pl.replicated_data = data + skip;
			skip += pl.replicated_length;

			pl.pts = asf_byteio_getDWLE(pl.replicated_data + 4);
		} else if (pl.replicated_length == 1) {
			if (skip >= datalen) {
				/* not enough data */
				return ASF_ERROR_INVALID_LENGTH;
			}

			/* in compressed payload object offset is actually pts */
			pl.pts = pl.media_object_offset;
			pl.media_object_offset = 0;

			pl.replicated_length = 0;
			pl.replicated_data = NULL;

			pts_delta = data[skip];
			skip++;
			compressed = 1;
		} else {
			pl.pts = packet->send_time;
			pl.replicated_data = NULL;
		}

		/* substract preroll value from pts since it's counted in */
		pl.pts -= preroll;
		/* FIXME: check that pts is positive */

		if (multiple) {
			tmp = GETLEN2b(type);

			if (tmp != 2) {
				/* in multiple payloads datalen should be a word */
				return ASF_ERROR_INVALID_VALUE;
			}
			if (skip + tmp > datalen) {
				/* not enough data */
				return ASF_ERROR_INVALID_LENGTH;
			}

			pl.datalen = GETVALUE2b(type, data + skip);
			skip += tmp;
		} else {
			pl.datalen = datalen - skip;
		}

		if (compressed) {
			int i, used = 0;

			for (i=0; used < pl.datalen; i++)
				used += 1 + data[skip + used];

			if (used != pl.datalen) {
				/* invalid compressed data size */
				return ASF_ERROR_INVALID_LENGTH;
			}

			packet->payload_count += i;
			if (packet->payload_count > packet->payloads_size) {
				void *tempptr;

				tempptr = realloc(packet->payloads,
				                  packet->payload_count * sizeof(asf_payload_t));
				if (!tempptr) {
					return ASF_ERROR_OUTOFMEM;
				}
				packet->payloads = tempptr;
				packet->payloads_size = packet->payload_count;
			}

			while (skip < datalen) {
				pl.datalen = data[skip];
				skip++;

				pl.data = data + skip;
				skip += pl.datalen;

				pl.pts += pts_delta;
				memcpy(&packet->payloads[i], &pl, sizeof(asf_payload_t));
				i++;
			}
			i--;
		} else {
			pl.data = data + skip;
			memcpy(&packet->payloads[i], &pl, sizeof(asf_payload_t));
		}
		skip += pl.datalen;

#ifdef DEBUG
		printf("payload(%d/%d) stream: %d, object: %d, offset: %d, pts: %d, datalen: %d\n",
		       i+1, packet->payload_count, pl.stream_number, pl.media_object_number,
		       pl.media_object_offset, pl.pts, pl.datalen);
#endif
	}

	return skip;
}

void
asf_data_init_packet(asf_packet_t *packet)
{
	packet->ec_length = 0;
	packet->ec_data = NULL;
	packet->ec_data_size = 0;

	packet->length = 0;
	packet->padding_length = 0;
	packet->send_time = 0;
	packet->duration = 0;

	packet->payload_count = 0;
	packet->payloads = NULL;
	packet->payloads_size = 0;

	packet->datalen = 0;
	packet->payload_data = NULL;
	packet->payload_data_size = 0;
}

int
asf_data_get_packet(asf_packet_t *packet, asf_file_t *file)
{
	asf_stream_t *stream;
	uint32_t read = 0;
	int packet_flags, packet_property, payload_length_type;
	void *tmpptr;
	int tmp;

	long length=0;

	stream = &file->stream;
	if ((tmp = asf_byteio_readbyte(stream)) < 0) {
		return ASF_ERROR_EOF;
	}
	read = 1;

	if (tmp & 0x80) {
		uint8_t opaque_data, ec_length_type;

		packet->ec_length = tmp & 0x0f;
		opaque_data = (tmp >> 4) & 0x01;
		ec_length_type = (tmp >> 5) & 0x03;

		if (ec_length_type != 0x00 ||
		    opaque_data != 0 ||
		    packet->ec_length != 0x02) {
			/* incorrect error correction flags */
			return ASF_ERROR_INVALID_VALUE;
		}

		if (packet->ec_length > packet->ec_data_size) {
			tmpptr = realloc(packet->ec_data, packet->ec_length);
			if (!tmpptr) {
				return ASF_ERROR_OUTOFMEM;
			}
			packet->ec_data = tmpptr;
			packet->ec_data_size = packet->ec_length;
		}

		if ((tmp = asf_byteio_read(packet->ec_data,
		                           packet->ec_length,
		                           stream)) < 0) {
			return tmp;
		}
		read += packet->ec_length;
	} else {
		packet->ec_length = 0;
	}

	if ((packet_flags = asf_byteio_readbyte(stream)) < 0 ||
	    (packet_property = asf_byteio_readbyte(stream)) < 0) {
		return ASF_ERROR_IO;
	}
	read += 2;

	tmp = asf_data_read_packet_data(packet, packet_flags, stream);
	if (tmp < 0) {
		return tmp;
	}
	read += tmp;

	/* this is really idiotic, packet length can (and often will) be
	 * undefined and we just have to use the header packet size as the size
	 * value */
	if (!((packet_flags >> 5) & 0x03)) {
		packet->length = file->packet_size;
	}
	
	/* this is also really idiotic, if packet length is smaller than packet
	 * size, we need to manually add the additional bytes into padding length
	 */
	if (packet->length < file->packet_size) {
		packet->padding_length += file->packet_size - packet->length;
		packet->length = file->packet_size;
	}

	if (packet->length > file->packet_size) {
		/* packet with too big length value */
		return ASF_ERROR_INVALID_LENGTH;
	}

	/* check if we have multiple payloads */
	if (packet_flags & 0x01) {
		if ((tmp = asf_byteio_readbyte(stream)) < 0) {
			return tmp;
		}
		read++;

		packet->payload_count = tmp & 0x3f;
		payload_length_type = (tmp >> 6) & 0x03;
	} else {
		packet->payload_count = 1;
		payload_length_type = 0x02; /* not used */
	}

	if (packet->length < read) {
		/* header exceeded packet size, invalid file */
		/* FIXME: should this be checked earlier? */
		return ASF_ERROR_INVALID_LENGTH;
	}

	packet->datalen = packet->length - read;

	if (packet->payload_count > packet->payloads_size) {
		tmpptr = realloc(packet->payloads,
		                 packet->payload_count * sizeof(asf_payload_t));
		if (!tmpptr) {
			return ASF_ERROR_OUTOFMEM;
		}
		packet->payloads = tmpptr;
		packet->payloads_size = packet->payload_count;
	}
	if (packet->datalen > packet->payload_data_size) {
		tmpptr = realloc(packet->payload_data,
		                 packet->datalen);
		if (!tmpptr) {
			return ASF_ERROR_OUTOFMEM;
		}
		packet->payload_data = tmpptr;
		packet->payload_data_size = packet->payload_count;
	}

	length = ftell(stream->opaque);
//	printf("Payload offset: %ld\n",length);

	if ((tmp = asf_byteio_read(packet->payload_data, packet->datalen, stream)) < 0) {
		return tmp;
	}

	tmp = asf_data_read_payloads(packet, file->preroll, packet_flags & 0x01,
	                             payload_length_type, packet_property,
	                             packet->payload_data,
	                             packet->datalen - packet->padding_length);
	if (tmp < 0) {
		return tmp;
	}
	read += tmp;

#ifdef DEBUG
	printf("packet read, eclen: %d, length: %d, padding: %d, time %d, duration: %d, payloads: %d\n",
	       packet->ec_length, packet->length, packet->padding_length, packet->send_time,
	       packet->duration, packet->payload_count);
#endif

	return read;
}

void
asf_data_free_packet(asf_packet_t *packet)
{
	if (!packet)
		return;

	free(packet->ec_data);
	free(packet->payloads);
	free(packet->payload_data);

	packet->ec_data = NULL;
	packet->payloads = NULL;
	packet->payload_data = NULL;
}
