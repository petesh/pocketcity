#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <stringsearch.h>

static void
fillBM(const char *srch, size_t srch_len, size_t *bml, size_t bm_len)
{
	size_t i;
	for (i = 0; i < bm_len; i++)
		bml[i] = srch_len;
	for (i = 0; i < srch_len - 1; i++, srch++)
		bml[(size_t)*srch] = srch_len - i - 1;
}

#define ASIZE	256

void *
inMem(const char *space, size_t space_len, const char *item,
    size_t item_len)
{
	/* const char *srch_end = srch_space + srch_len;
	const char *item_end = srch_item + item_len; */
	size_t bmbc[ASIZE];
	unsigned char *aj, *maxj;
	unsigned char c;
	unsigned char lic;

	if ((space_len < item_len))
		return (NULL);

	fillBM(item, item_len, bmbc, ASIZE);

	/*
	for (j = 0; j < ASIZE; j++)
		if (bmbc[j] != item_len)
			if (j < 32 || j > 127)
				printf("bmbc['%d'] == %ld\n", j, (long)bmbc[j]);
			else
				printf("bmbc['%c'] == %ld\n", j, (long)bmbc[j]);

	*/

	aj = (char *)space;
	maxj  = (char *)(space + (space_len - item_len));

	lic = item[item_len - 1];

	while (aj <= maxj) {
		c = aj[item_len - 1];
		if (lic == c) {
			if (memcmp(item,
			    aj, item_len - 1) == 0) {
			return ((void *)(aj));
		}
		}
		aj += bmbc[(size_t)c];
	}
	return (NULL);
}

