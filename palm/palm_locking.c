/*!
 * \file
 *
 * \brief Locking routines for zones.
 */
#include <PalmOS.h>
#include <StringMgr.h>

#include <globals.h>
#include <locking.h>
#include <mem_compat.h>

int
mem_alloc(lockmem_t *str, UInt32 size)
{
	int rv;

	rv = mem_resize(str, size);
	if (rv) {
		mem_lock(str);
		MemSet(str->ptr, size, 0);
		mem_unlock(str);
	}
	return (rv);
}

void
mem_lock(lockmem_t *mem)
{
	mem->lock_count += 1;
	if (mem->lock_count == 1) {
		if (mem->handle) {
			mem->ptr = MemHandleLock(mem->handle);
			if (mem->pptr)
				*(mem->pptr) = mem->ptr;
		}
	}
}

void
mem_unlock(lockmem_t *mem)
{
	mem->lock_count -= 1;

	if (mem->lock_count == 0) {
		if (mem->handle != NULL)
			MemHandleUnlock(mem->handle);
		if (mem->pptr != NULL)
			*mem->pptr = NULL;
		mem->ptr = NULL;
	}

	ErrFatalDisplayIf(mem->lock_count < 0, "Too many unlock calls");
}

int
mem_resize(lockmem_t *mem, UInt32 size)
{
	if (size == 0) {
		mem_release(mem);
		return (1);
	} else {
		Err e = 0;
		if (mem->ptr != NULL) {
			MemHandleUnlock(mem->handle);
			mem->ptr = NULL;
			mem->lock_count = 0;
			if (mem->pptr != NULL)
				*mem->pptr = NULL;
		}
		if (mem->handle != NULL) {
			e = MemHandleResize(mem->handle, size);
			if (0 == e)
				mem->size = size;
		} else {
			mem->handle = MemHandleNew(size);
			if (NULL == mem->handle)
				e = memErrNotEnoughSpace;
			else
				mem->size = size;
		}
		return (0 == e);
	}
}

void
mem_release(lockmem_t *mem)
{
	if (mem->ptr != NULL)
		MemHandleUnlock(mem->handle);
	MemHandleFree(mem->handle);
	mem->handle = NULL;
	if (mem->pptr != NULL)
		*mem->pptr = NULL;
	mem->ptr = NULL;
	mem->lock_count = 0;
	mem->size = 0;
}

