/* \file
 * \brief Memory compatibility functions
 * 
 * Needed because the annoying palm platform does not supply compatible
 * calls for malloc, free etc.
 */

#if !defined(_MEM_COMPAT_H)
#define _MEM_COMPAT_H
 
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define gMalloc	malloc
#define gFree free
#define gRealloc realloc
#define gCalloc calloc
/*! \brief swap arguments for character and length */
#define gMemSet(P,L,C) memset((P),(C),(L))
#define	QSort(a,b,c,d)	qsort(a, b, c, \
    (int (*)(const void *, const void *))d)

#endif /* _MEM_COMPAT_H */
