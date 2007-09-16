/*!
 * \file
 * \brief implementation of a memory searching algorithm
 *
 * It's the horspool string search algorithm, except used for searching
 * for memory patterns.
 */
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <stringsearch.h>

/*!
 * \brief fill up an array with offset values used to advance the pointer
 * \param srch the search string
 * \param srch_len the length of the search string
 * \param bml the alphabet array to occupy
 * \param bm_len the length of the alphabet
 */
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
	size_t bmbc[ASIZE];
	unsigned char *aj, *maxj;
	unsigned char c;
	unsigned char lic;

	if ((space_len < item_len))
		return (NULL);

	fillBM(item, item_len, bmbc, ASIZE);

	aj = (unsigned char *)space;
	maxj  = (unsigned char *)(space + (space_len - item_len));

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

