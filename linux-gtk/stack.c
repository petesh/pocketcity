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
#include <assert.h>

typedef struct _stacky {
    long *bp;
    long *sp;
    long *se;
    long sl;
} Stacky;

/*
 * resize the stack based on the current size
 */
static void
StackResize(Stacky *sp)
{
    long sd = sp->sp - sp->bp;
    if (sp->sl == 0) sp->sl = 256;
    sp->sl <<= 1; /* Double the size every time */
    sp->bp = realloc(sp->bp, sp->sl * sizeof (long));
    sp->se = sp->bp + (sp->sl - 1);
    sp->sp = sp->bp + sd;
}

/*
 * Create a new stack
 */
Stacky *
StackNew(void)
{
    Stacky *rv = calloc(sizeof (Stacky), 1);
    return (rv);
}

/*
 * Delete the stack
 */
void
StackDelete(Stacky *sp)
{
    if (sp->bp) free(sp->bp);
    free(sp);
}

/*
 * add an item to the stack
 */
void
StackPush(Stacky *sp, long value)
{
    if (sp->sp >= sp->se) StackResize(sp);
    *(++sp->sp) = value;
}

/*
 * pop an item from the stack
 */
long
StackPop(Stacky *sp)
{
    if (sp->sp >= sp->bp) {
        assert(sp->sp != NULL && sp->bp != NULL);
        return (*sp->sp--);
    }
    perror("<MyStack> Stack Underflow");
    return (-1);
}

/*
 * is the stack empty
 */
int
StackIsEmpty(Stacky *sp)
{
    if (sp->sp == NULL)
        return (1);
    return (sp->sp < sp->bp);
}

/*
 * empty the stack
 */
void
StackDoEmpty(Stacky *sp)
{
    sp->sp = sp->bp - 1;
}

/*
 * get count of number of emements in stack
 */
int
StackNElements(Stacky *sp)
{
    return ((sp->sp+1) - sp->bp);
}
