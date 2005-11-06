/*!
 * \file
 * \brief memory allocation routines for the palm platform
 *
 * This is an implementation the commin stdlib routines realloc and calloc
 * as the palm platform does not operate in the same manner.
 */

#include <PalmTypes.h>
#include <MemoryMgr.h>

#include <mem_compat.h>

#if defined(MEM_DEBUG)
MemPtr
_MemPtrNew(UInt32 size, char *file, int line)
{
	MemPtr mp = MemPtrNew(size);
	WriteLog("%lx = MemPtrNew(%lx) [%s:%d]\n", mp, size, file, line);
	return (mp);
}

MemPtr
_MemHandleLock(MemHandle mh, char *file, int line)
{
	MemPtr mp = MemHandleLock(mh);
	WriteLog("%lx = MemHandleLock(%lx) [%s:%d]\n", mp, mh, file, line);
	return (mp);
}

Err
_MemHandleUnlock(MemHandle mh, char *file, int line)
{
	Err err = MemHandleUnlock(mh);
	WriteLog("%x = MemHandleUnlock(%lx) [%s:%d]\n", err, mh, file, line);
	return (err);
}

MemHandle
_MemHandleNew(UInt32 size, char *file, int line)
{
	MemHandle mh = MemHandleNew(size);
	WriteLog("%lx = MemHandleNew(%lx) [%s:%d]\n", mh, size, file, line);
	return (mh);
}

MemHandle
_MemPtrRecoverHandle(MemPtr mp, char *file, int line)
{
	MemHandle mh = MemPtrRecoverHandle(mp);
	WriteLog("%lx = MemPtrRecoverHandle(%lx) [%s:%d]\n", mh, mp, file,
	    line);
	return (mh);
}

Err
_MemHandleResize(MemHandle mh, UInt32 size, char *file, int line)
{
	Err err = MemHandleResize(mh, size);
	WriteLog("%d = MemHandleResize(%lx, %lx) [%s:%d]\n", err, mh,
	    size, file, line);
	return (err);
}

Err
_MemHandleFree(MemHandle mh, char *file, int line)
{
	Err err = MemHandleFree(mh);
	WriteLog("%d = MemHandleFree(%lx) [%s:%d]\n", err, mh, file, line);
	return (err);
}

Err
_MemPtrFree(MemPtr mp, char *file, int line)
{
	Err err = MemPtrFree(mp);
	WriteLog("%d = MemPtrFree(%lx) [%s:%d]\n", err, mp, file, line);
	return (err);
}
#endif
#include <mem_compat.h>

/*!
 * \brief parallels the realloc function
 * \returns the new memory pointer
 * \param old old pointer to the memory
 * \param new_size the new size of the structure
 */
MemPtr
palm_realloc(MemPtr old, UInt32 new_size)
{
	MemHandle mh;

	if (old == NULL) {
		mh = MemHandleNew(new_size);
		if (mh == NULL)
			return (mh);
		return (MemHandleLock(mh));
	}

	mh = MemPtrRecoverHandle(old);
	MemPtrUnlock(old);
	MemHandleResize(mh, new_size);
	return (MemHandleLock(mh));
}

MemPtr
palm_calloc(UInt32 size, UInt32 count)
{
	MemHandle mh;
	MemPtr mp;

	mh = MemHandleNew(size * count);
	if (mh == NULL)
		return (mh);
	mp = MemHandleLock(mh);
	MemSet(mp, (Int32)(size * count), 0);
	return (mp);
}

MemPtr
palm_malloc(UInt32 size)
{
	MemHandle mh = MemHandleNew(size);
	return (MemHandleLock(mh));
}

void
palm_free(MemPtr mem)
{
	MemHandle mh = MemPtrRecoverHandle(mem);
	MemHandleUnlock(mh);
	MemHandleFree(mh);
}
