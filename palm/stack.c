/*
 * A stack implementation that uses a growing memptr.
 * Designed to hold unsigned longs...
 * I live in an ILP32 / LP64 world.
 * long == pointer in 32 bit and 64 bit. If not then just forgive me for
 * this laziness.
 */
#include <MemoryMgr.h>
#include <ErrorMgr.h>
#include <FeatureMgr.h>

#include "resCompat.h"

typedef struct _stacky {
    MemHandle   sh; /* Start of stack... handle */
    long       *ss; /* Start of stack */
    long       *sp; /* Stack pointer */
    long       *se; /* Stack pointer */
    long        sl; /* Stack Len */
} Stacky;

/*
 * preallocates 128 elements to the stack
 */
void *
StackNew(void)
{
    Stacky *s = MemPtrNew(sizeof (Stacky));
    if (s == NULL)
        return (NULL);
    s->sl = 128;
    if ((NULL == (s->sh = MemHandleNew(s->sl * sizeof (long)))) ||
      (NULL == (s->ss = MemHandleLock(s->sh)))) {
        if (s->sh) MemHandleFree(s->sh);
        MemPtrFree(s);
        return (NULL);
    }
    s->se = s->ss + (s->sl - 1);
    s->sp = s->ss - 1;
    return (s);
}

/*
 * delete the contents of the stack
 */
void
StackDelete(Stacky *sp)
{
    if (sp->sh) {
        MemHandleUnlock(sp->sh);
        MemHandleFree(sp->sh);
    }
    MemPtrFree(sp);
}

/*
 * remove the top most item from the stack
 */
long
StackPop(Stacky *sp)
{
    if (sp->sp >= sp->ss) {
        return (*sp->sp--);
    }
    ErrFatalDisplayIf(1, "myStack Underflow");
    return (-1);
}

/*
 * add an element to the stack
 */
void
StackPush(Stacky *sp, long elt)
{
    if (sp->sp >= sp->se) {
        long sd = sp->sp - sp->ss;
        long sn;
        sp->sl <<= 1;
        sn = sp->sl * sizeof (long);
        MemHandleUnlock(sp->sh);
        if (errNone != MemHandleResize(sp->sh, sn)) {
            ErrFatalDisplayIf(1, "Resize of myStack Chunk Failed");
        }
        sp->ss = MemHandleLock(sp->sh);
        sp->se = sp->ss + (sp->sl - 1);
        sp->sp = sp->ss + sd;
    }
    *(++sp->sp) = elt;
}

/*
 * check if the stack is empty
 */
int
StackIsEmpty(Stacky *sp)
{
    return (sp->sp < sp->ss);
}

/*
 * 'empty' the stack.
 */
void
StackDoEmpty(Stacky *sp)
{
    sp->sp = sp->ss - 1;
}

/*
 * get the count of the number of elements on the stack
 */
int
StackNElements(Stacky *sp)
{
    return ((sp->sp+1) - sp->ss);
}
