/*!
 * \file
 * \brief non locking based implementation of 'locking'
 *
 * We use a shadow 'handle' for the locking. The locking of a zone involves
 * copying the shadow handle into the memory pointer. Unlocking simply
 * zeroes it out.
 */

#include <locking.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

int
mem_alloc(lockmem_t *mem, UInt32 size)
{
	int rv;

	rv = mem_resize(mem, size);
	if (rv)
		bzero(mem->handle, size);
	return (rv);
}

void
mem_lock(lockmem_t *mem)
{
	if (mem->lock_count++ == 0) {
		if (mem->pptr != NULL)
			*mem->pptr = mem->handle;
		mem->ptr = mem->handle;
	}
}

void
mem_unlock(lockmem_t *mem)
{
	if (mem->lock_count-- == 1) {
		if (mem->pptr != NULL)
			*mem->pptr = NULL;
		mem->ptr = NULL;
	}
	assert(mem->lock_count >= 0);
}

int
mem_resize(lockmem_t *mem, UInt32 size)
{
	if (size == 0) {
		mem_release(mem);
		return (1);
	} else {
		if (mem->handle) {
			mem->handle = realloc(mem->handle, size);
			if (mem->handle) {
				memset(mem->handle, size, 0);
				mem->size = size;
			}
		} else {
			mem->handle = calloc(size, 1);
			if (mem->handle)
				mem->size = size;
		}
		if (*mem->pptr != NULL)
			*mem->pptr = 0;
		mem->lock_count = 0;
		return ((NULL == mem->handle) ? 1 : 0);
	}
}

void
mem_release(lockmem_t *mem)
{
	if (mem->pptr != NULL)
		*mem->pptr = NULL;
	mem->lock_count = 0;
	free(mem->handle);
	mem->handle = NULL;
	mem->size = 0;
}
