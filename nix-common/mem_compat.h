/*! \file
 * \brief Memory compatibility functions
 * 
 * Needed because the palm platform does not supply compatible
 * calls for malloc, free etc.
 */

#if !defined(_MEM_COMPAT_H)
#define _MEM_COMPAT_H
 
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define gmalloc	malloc
#define gfree free
#define grealloc realloc
#define gcalloc calloc
/*! \brief swap arguments for character and length */
#define gmemset(P,L,C) memset((P),(C),(L))
#define gmemmove(D, S, L) memmove((D), (S), (L))
#define	gqsort(a,b,c,d)	qsort(a, b, c, \
    (int (*)(const void *, const void *))d)

#endif /* _MEM_COMPAT_H */
