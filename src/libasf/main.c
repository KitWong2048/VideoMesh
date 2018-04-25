
#include <stdio.h>
#include "asf.h"
#include "asfint.h"
#include "libasf.h"
#define MAXPACKETSIZE 100000
#define _LIB_BUILD_
struct test{
	char a;
	char b;
	char *c;
};

static void
print_metadata(asf_metadata_t *metadata) {
	int i;

	printf("----- PRINT METADATA -----\n");
	if (metadata->title)
		printf("title: %s\n", metadata->title);
	if (metadata->artist)
		printf("artist: %s\n", metadata->artist);
	if (metadata->copyright)
		printf("copyright: %s\n", metadata->copyright);
	if (metadata->description)
		printf("description: %s\n", metadata->description);
	if (metadata->rating)
		printf("rating: %s\n", metadata->rating);
	for (i=0; i<metadata->extended_count; i++) {
		printf("\"%s\" - \"%s\"\n", metadata->extended[i].key, metadata->extended[i].value);
	}
	printf("-----  END METADATA -----\n");
}


#ifndef _LIB_BUILD_

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Incorrect number of arguments\n");
		return -1;
	}
	packetize(argv[1], argv[1], 0);	
	return 0;
}
#endif
