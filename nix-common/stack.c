/*!
 * \file
 * Part of the pocketcity application.
 *
 * \brief This provides the stack functions for a unix/linux box.
 *
 * It's almost
 * identical to the palm one except it uses malloc/realloc normally.
 * realloc on palm has problems (fragmented heap)
 */

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <appconfig.h>
#include <malloc.h>

/*!
 * \brief the data structure for the stack object
 */
typedef struct tag_dsobj_nix {
	Int32 *bp; /*!< base pointer */
	Int32 *sp; /*!< stack pointer */
	Int32 *se; /*!< stack end */
	Int32 sl; /*!< stack length */
} dsObj;

#define	STACK_IMPL
#include <stack.h>

/*!
 * \brief resize the stack based on the current size
 * \param sp the stack object
 * \param newSize the new size of the stack
 */
static void
StackResize(dsObj *sp, Int32 newSize)
{
	ptrdiff_t sd;

	assert(sp);
	sd = sp->sp - sp->bp;
	sp->sl = newSize;
	sp->bp = realloc(sp->bp, sp->sl * sizeof (Int32));
	sp->se = sp->bp + (sp->sl - 1);
	sp->sp = sp->bp + sd;
}

dsObj *
StackNew(void)
{
	dsObj *rv = calloc(sizeof (dsObj), 1);
	return (rv);
}

void
StackDelete(dsObj *sp)
{
	if (sp->bp) free(sp->bp);
	free(sp);
}

void
StackPush(dsObj *sp, Int32 value)
{
	assert(sp);
	if (sp->sp >= sp->se)
		StackResize(sp, sp->sl ? sp->sl << 1 : 64);
	*(sp->sp++) = value;
}

Int32
StackPop(dsObj *sp)
{
	assert(sp);
	assert(sp->sp != NULL);
	assert (sp->sp > sp->bp);
	assert(sp->bp != NULL);
	sp->sp--;
	return (*sp->sp);
}

Int8
StackIsEmpty(dsObj *sp)
{
	assert(sp);
	return ((Int8)(sp->sp == sp->bp));
}

void
StackDoEmpty(dsObj *sp)
{
	assert(sp);
	sp->sp = sp->bp;
}

int
StackNElements(dsObj *sp)
{
	assert(sp);
	return (sp->sp - sp->bp);
}

Int32
ListGet(dsObj *sp, Int32 index)
{
	assert(sp);
	assert (index < (sp->sp - sp->bp));
	return (sp->bp[index]);
}

void
ListSet(dsObj *sp, Int32 index, Int32 element)
{
	assert (index < (sp->sp - sp->bp));
	sp->bp[index] = element;
}

void
ListInsert(dsObj *sp, Int32 index, Int32 element)
{
	if ((index) >= (sp->sp - sp->bp)) {
		StackPush(sp, element);
	} else {
		StackPush(sp, 0);
		bcopy(sp->bp + index, sp->bp + index,
		    (sp->sp - (sp->bp + index)) * sizeof (Int32));
		sp->bp[index] = element;
	}
}

Int32
ListRemove(dsObj *ds, Int32 index)
{
	Int32 rv;

	assert (index < (ds->sp - ds->bp));
	rv = ds->bp[index];
	bcopy(ds->bp + (index + 1), ds->bp + index,
	    (ds->sp - (ds->bp + index)) * sizeof(Int32));
	(void)StackPop(ds);

	return (rv);
}
