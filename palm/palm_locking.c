#include <PalmOS.h>
#include <StringMgr.h>
#include <globals.h>
#include <ui.h>

/* Locking routines */
static struct tag_lockers {
	MemHandle	handle;
	int		lockcount;
	void		**destVar;
} lockZones[lz_end] = {
	{ NULL, 0, &worldPtr },
	{ NULL, 0, &growablePtr }
};

void
LockZone(lockZone zone)
{
	struct tag_lockers *lock = lockZones + zone;

	if (lock->lockcount++ == 1) {
		if (lock->handle != NULL) {
			*lock->destVar = MemHandleLock(lock->handle);
		}
	}
}

void
UnlockZone(lockZone zone)
{
	struct tag_lockers *lock = lockZones + zone;

	if (lock->handle == NULL) {
		lock->handle = MemPtrRecoverHandle(*lock->destVar);
	}
	if (--lock->lockcount == 0) {
		MemPtrUnlock(*lock->destVar);
		*lock->destVar = NULL;
	}

	ErrFatalDisplayIf(lock->lockcount < 0,
	    "Too many unlock calls");
}

