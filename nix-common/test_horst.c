#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <inttypes.h>
#include <strings.h>
#include <stringsearch.h>

int
main(int argc, char **argv)
{
	size_t haylen;
	size_t needlen;
	char *fnd;

	if (argc < 3) {
		printf("usage: %s <haystack> <needle>\n", argv[0]);
		return (1);
	}
	haylen = strlen(argv[1]);
	needlen = strlen(argv[2]);

	if (haylen < needlen) {
		printf("Needle is bigger than haystack!\n");
		return (3);
	}
	fnd = inMem(argv[1], haylen, argv[2], needlen);
	if (fnd != NULL) {
		printf("found: %ld\n", (long)(fnd - argv[1]));
	}
	return (0);
}
