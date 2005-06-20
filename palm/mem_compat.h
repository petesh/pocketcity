/*! \file
 * \brief Memory compatibility functions
 * 
 * Needed because the annoying palm platform does not supply compatible
 * calls for malloc, free etc.
 */

#if !defined(_MEM_COMPAT_H_)
#define _MEM_COMPAT_H_
 
#include <MemoryMgr.h>
#include <SysUtils.h>

MemPtr palm_realloc(MemPtr old, UInt32 new_size);
MemPtr palm_calloc(UInt32 size, UInt32 count);
void palm_free(MemPtr mem);
MemPtr palm_malloc(UInt32 size);

#define gMalloc	palm_malloc
#define gFree palm_free
#define gRealloc palm_realloc
#define gCalloc palm_calloc
#define gMemSet(P,L,C) MemSet(P,L,C)

#define QSort(a,b,c,d)	SysQSort(a, (UInt16)b, (Int16)c, d, (Int32)1)

#if defined(MEM_DEBUG)
#define MemHandleResize(M,S) _MemHandleResize(M, S, __FILE__, __LINE__)
#define MemPtrRecoverHandle(M) _MemPtrRecoverHandle(M, __FILE__, __LINE__)
#define MemHandleNew(S) _MemHandleNew(S, __FILE__, __LINE__)
#define MemHandleLock(M) _MemHandleLock(M, __FILE__, __LINE__)
#define MemPtrNew(S) _MemPtrNew(S, __FILE__, __LINE__)
/* Memchunkfree is here it's an alias of memptrfree */
#define MemChunkFree(P) _MemPtrFree(P, __FILE__, __LINE__)
#define MemHandleFree(P) _MemHandleFree(P, __FILE__, __LINE__)
#define MemHandleUnlock(H) _MemHandleUnlock(H, __FILE__, __LINE__)
MemPtr _MemPtrNew(UInt32 size, char *file, int line);
MemPtr _MemHandleLock(MemHandle mh, char *file, int line);
MemHandle _MemHandleNew(UInt32 size, char *file, int line);
MemHandle _MemPtrRecoverHandle(MemPtr mp, char *file, int line);
Err _MemHandleResize(MemHandle mh, UInt32 size, char *file, int line);
Err _MemPtrFree(MemPtr mp, char *file, int line);
Err _MemHandleFree(MemHandle mh, char *file, int line);
Err _MemHandleUnlock(MemHandle mh, char *file, int line);
#endif

#endif /* _MEM_COMPAT_H_ */
