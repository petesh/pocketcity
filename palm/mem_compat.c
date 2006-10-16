/*!
 * \file
 * \brief memory allocation routines for the palm platform
 *
 * This is an implementation the commin stdlib routines realloc and calloc
 * as the palm platform does not operate in the same manner.
 */

#include <PalmTypes.h>
#include <MemoryMgr.h>
#include <logging.h>

#if defined(MEM_DEBUG)

MemPtr
_MemPtrNew(UInt32 size, char *file, int line)
{
	MemPtr mp = MemPtrNew(size);
	WriteLog("MemPtrNew(%lx) = %lx [%s:%d]\n", size, mp, file, line);
	return (mp);
}

MemPtr
_MemHandleLock(MemHandle mh, char *file, int line)
{
	MemPtr mp = MemHandleLock(mh);
	WriteLog("MemHandleLock(%lx) = %lx [%s:%d]\n", mh, mp, file, line);
	return (mp);
}

Err
_MemHandleUnlock(MemHandle mh, char *file, int line)
{
	Err err = MemHandleUnlock(mh);
	WriteLog("MemHandleUnlock(%lx) = %x [%s:%d]\n", mh, err, file, line);
	return (err);
}

MemHandle
_MemHandleNew(UInt32 size, char *file, int line)
{
	MemHandle mh = MemHandleNew(size);
	WriteLog("MemHandleNew(%lx) = %lx [%s:%d]\n", size, mh, file, line);
	return (mh);
}

MemHandle
_MemPtrRecoverHandle(MemPtr mp, char *file, int line)
{
	MemHandle mh = MemPtrRecoverHandle(mp);
	WriteLog("MemPtrRecoverHandle(%lx) = %lx [%s:%d]\n", mp, mh, file,
	    line);
	return (mh);
}

Err
_MemHandleResize(MemHandle mh, UInt32 size, char *file, int line)
{
	Err err = MemHandleResize(mh, size);
	WriteLog("MemHandleResize(%lx, %lx) = %x [%s:%d]\n", mh, size, err,
	    file, line);
	return (err);
}

Err
_MemHandleFree(MemHandle mh, char *file, int line)
{
	Err err = MemHandleFree(mh);
	WriteLog("MemHandleFree(%lx) = %x [%s:%d]\n", mh, err, file, line);
	return (err);
}

Err
_MemPtrFree(MemPtr mp, char *file, int line)
{
	Err err = MemPtrFree(mp);
	WriteLog("MemPtrFree(%lx) = %x [%s:%d]\n", mp, err, file, line);
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
	MemPtr rv;

	if (old == NULL) {
		mh = MemHandleNew(new_size);
		if (mh == NULL)
			return (mh);
		rv = MemHandleLock(mh);
		return (rv);
	}

	mh = MemPtrRecoverHandle(old);
	MemPtrUnlock(old);
	MemHandleResize(mh, new_size);
	rv = MemHandleLock(mh);
	return (rv);
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
	MemPtr mp;
	MemHandle mh = MemHandleNew(size);
	mp = MemHandleLock(mh);
	return (mp);
}

void
palm_free(MemPtr mem)
{
	MemHandle mh = MemPtrRecoverHandle(mem);
	MemHandleUnlock(mh);
	MemHandleFree(mh);
}
