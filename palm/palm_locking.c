#include <PalmOS.h>
#include <StringMgr.h>
#include <globals.h>
#include <ui.h>
#include <mem_compat.h>

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

	lock->lockcount += 1;
	if (lock->lockcount == 1) {
		if (lock->handle) {
			MemPtr mp = MemHandleLock(lock->handle);
			*(lock->destVar) = mp;
		}
		lock->handle = NULL;
	}
}

void
ReleaseZone(lockZone zone)
{
	struct tag_lockers *lock = lockZones + zone;

	LockZone(zone);
	if (*lock->destVar != NULL) {
		MemPtrFree(*lock->destVar);
	}
	lock->handle = NULL;
	lock->destVar = NULL;
	lock->lockcount = 0;
}

void
UnlockZone(lockZone zone)
{
	struct tag_lockers *lock = lockZones + zone;
	MemPtr mp = *lock->destVar;

	if (mp != NULL) {
		lock->handle = MemPtrRecoverHandle(mp);
	}
	lock->lockcount -= 1;
	if (lock->lockcount == 0) {
		if (mp != NULL)
			MemHandleUnlock(lock->handle);
		*lock->destVar = NULL;
	}

	ErrFatalDisplayIf(lock->lockcount < 0,
	    "Too many unlock calls");
}
