/*!
 * \file
 * \brief test code for the buffer packing code
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <inttypes.h>
#include <strings.h>
#include <globals.h>

#define ANDUPTO(X) \
	((Char)((1<<((X)-1)) | (1<<((X)-2)) | (1<<((X)-3)) | (1<<((X)-4)) | \
	(1<<((X)-5)) | (1<<((X)-6)) | (1<<((X)-7)) | (1<<((X)-8))))

void
DumpChar(Char value)
{
	int i;
	for (i = 7; i >= 0; i--) {
		printf("%c", (value & (1<<i)) == (1<<i) ? '1' : '0');
	}
}

void
DumpBuffer(Char *buffer, UInt32 len)
{
	int ap = 0;
	printf("**\n");
	while(len--) {
		if (ap == 4) {
			ap = 0;
			printf("\n");
		}
		printf("[");
		DumpChar(*buffer);
		printf("]");
		buffer++;
		ap++;
	}
	printf("\n**\n");
}

int
main(int argc, char **argv)
{
	char *sbuf;
	char *dbuf2 = NULL;
	char *dbuf = NULL;
	int len = 32;
	int nbits;
	int asb;

	sbuf = malloc(len);
	dbuf2 = malloc(len);
	for (nbits = 7; nbits >= 1; nbits--) {
		printf("And == %x\n", ANDUPTO(nbits));
		for (asb = 0; asb < len; asb++) {
			sbuf[asb] = asb & ANDUPTO(nbits);
		}
		if (dbuf) {
			free(dbuf);
		}

		bzero(dbuf2, len);
		dbuf = calloc(1, len / (8 - nbits));
		printf("\nUnpacked\n");
		DumpBuffer(sbuf, len);

		PackBits(sbuf, dbuf, nbits, len);
		printf("\nPacked\n");
		DumpBuffer(dbuf, len / (8 - nbits));
		UnpackBits(dbuf, dbuf2, nbits, len);
		printf("\nUnpacked\n");
		DumpBuffer(dbuf2, len);
	}
	free(sbuf);
	free(dbuf2);
	if (dbuf) free(dbuf);
	return (0);
}
