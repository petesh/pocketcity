/*! \file
 * \brief Memory compatibility functions
 * 
 * Needed because the annoying palm platform does not supply compatible
 * calls for malloc, free etc.
 */

#if !defined(_MEM_COMPAT_H)
#define _MEM_COMPAT_H
 
#include <MemoryMgr.h>

MemPtr palm_realloc(MemPtr old, UInt32 new_size);
MemPtr palm_calloc(UInt32 size, UInt32 count);
void palm_free(MemPtr mem);
MemPtr palm_malloc(UInt32 size);

#define gMalloc	palm_malloc
#define gFree palm_free
#define gRealloc palm_realloc
#define gCalloc palm_calloc
#define gMemSet(P,L,C) MemSet(P,L,C)

#endif /* _MEM_COMPAT_H */
