/*
 * Part of the pocketcity application.
 */

/*
 * This provides the stack functions for a unix/linux box. It's almost
 * identical to the palm one except it uses malloc/realloc normally.
 * realloc on palm has problems (fragmented heap)
 */

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct _stacky {
    long *bp;
    long *sp;
    long *se;
    long sl;
} Stacky;

static void
StackResize(Stacky *sp)
{
    ptrdiff_t sd = sp->sp - sp->bp;
    if (sp->sl == 0) sp->sl = 64;
    sp->sl <<= 1; // Double the size every time
    sp->bp = realloc(sp->bp, sp->sl * sizeof (long));
    sp->se = sp->bp + (sp->sl - 1);
    sp->sp = sp->bp + sd;
}

Stacky *
StackNew(void)
{
    Stacky *rv = calloc(sizeof (Stacky), 1);
    return (rv);
}

void
StackDelete(Stacky *sp)
{
    if (sp->bp) free(sp->bp);
    free(sp);
}

void
StackPush(Stacky *sp, long value)
{
    if (sp->sp >= sp->se) StackResize(sp);
    *(++sp->sp) = value;
}

long
StackPop(Stacky *sp)
{
    if (sp->sp >= sp->bp) {
        return (*sp->sp--);
    }
    perror("<MyStack> Stack Underflow");
    return (-1);
}

int
StackIsEmpty(Stacky *sp)
{
    return (sp->sp < sp->bp);
}

void
StackDoEmpty(Stacky *sp)
{
    sp->sp = sp->bp - 1;
}

int
StackNElements(Stacky *sp)
{
    return ((sp->sp+1) - sp->bp);
}
