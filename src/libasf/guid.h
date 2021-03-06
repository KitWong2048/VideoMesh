
#ifndef GUID_H
#define GUID_H

#include "asf.h"

struct guid_s {
	uint32_t v1;
	uint32_t v2;
	uint16_t v3;
	uint8_t  v4[8];
};
typedef struct guid_s guid_t;

enum guid_type_e {
	GUID_UNKNOWN,

	GUID_HEADER,
	GUID_DATA,
	GUID_INDEX,

	GUID_FILE_PROPERTIES,
	GUID_STREAM_PROPERTIES,
	GUID_CONTENT_DESCRIPTION,
	GUID_HEADER_EXTENSION,
	GUID_MARKER,
	GUID_CODEC_LIST,
	GUID_STREAM_BITRATE_PROPERTIES,
	GUID_PADDING,
	GUID_EXTENDED_CONTENT_DESCRIPTION,

	GUID_METADATA,
	GUID_LANGUAGE_LIST,
	GUID_EXTENDED_STREAM_PROPERTIES,
	GUID_ADVANCED_MUTUAL_EXCLUSION,
	GUID_STREAM_PRIORITIZATION,

	GUID_STREAM_TYPE_AUDIO,
	GUID_STREAM_TYPE_VIDEO,
	GUID_STREAM_TYPE_COMMAND
};
typedef enum guid_type_e guid_type_t;


int asf_guid_match(const guid_t *guid1, const guid_t *guid2);
guid_type_t asf_guid_get_object_type(const guid_t *guid);
guid_type_t asf_guid_get_stream_type(const guid_t *guid);
guid_type_t asf_guid_get_type(const guid_t *guid);

#endif

