/*! \file
 * \brief memory allocation routines for the palm platform
 *
 * This is an implementation the commin stdlib routines realloc and calloc
 * as the palm platform does not operate in the same manner.
 */

#include <PalmTypes.h>
#include <MemoryMgr.h>

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
	MemSet(mp, size * count, 0);
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

