/*!
 * \file
 * \brief test the list code.
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stack.h>

int
main(int argc, char **argv)
{
	dsObj *obj = ListNew();
	int i;

	for (i = 0; i < 8192; i++) {
		ListAdd(obj, i);
	}
	for (i = 8190; i < 8194; i++)
		printf("%ld ", (long)ListGet(obj, i));
	printf("\n");

	for (i = 195; i < 205; i++)
		printf("%ld ", (long)ListGet(obj, i));
	printf("\n%ld\n", (long)ListRemove(obj, 200));
	for (i = 195; i < 205; i++)
		printf("%ld ", (long)ListGet(obj, i));
	printf("\n");
	for (i = 8190; i < 8194; i++)
		printf("%ld ", (long)ListGet(obj, i));
	printf("\n");
	ListInsert(obj, 200, 100);
	for (i = 195; i < 205; i++)
		printf("%ld ", (long)ListGet(obj, i));
	printf("\n");
	for (i = 8190; i < 8194; i++)
		printf("%ld ", (long)ListGet(obj, i));
	printf("\n");
	ListInsert(obj, 9000, 100);
	for (i = 8190; i < 8194; i++)
		printf("%ld ", (long)ListGet(obj, i));
	printf("\n");
	ListInsert(obj, 8194, 200);
	for (i = 8190; i < 8194; i++)
		printf("%ld ", (long)ListGet(obj, i));
	printf("\n");

	return (0);
}
