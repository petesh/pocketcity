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
	ptrdiff_t sd = sp->sp - sp->bp;
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
	if (sp->sp >= sp->se) StackResize(sp, sp->sl ? sp->sl << 1 : 64);
	*(++sp->sp) = value;
}

Int32
StackPop(dsObj *sp)
{
	assert(sp->sp != NULL);
	if (sp->sp >= sp->bp) {
		assert(sp->sp != NULL && sp->bp != NULL);
		return (*sp->sp--);
	}
	perror("<DsObj> Object Underflow(out of elements)");
	return (-1);
}

Int8
StackIsEmpty(dsObj *sp)
{
	if (sp->sp == NULL)
		return ((Int8)1);
	return ((Int8)(sp->sp < sp->bp));
}

void
StackDoEmpty(dsObj *sp)
{
	if (sp->sp != NULL)
		sp->sp = sp->bp - 1;
}

int
StackNElements(dsObj *sp)
{
	return ((sp->sp+1) - sp->bp);
}

Int32
ListGet(dsObj *sp, Int32 index)
{
	if (index >= (sp->sp - sp->bp))
		return (-1);
	return (sp->bp[index + 1]);
}

void
ListSet(dsObj *sp, Int32 index, Int32 element)
{
	if (index < (sp->sp - sp->bp))
		sp->bp[index+1] = element;
}

void
ListInsert(dsObj *sp, Int32 index, Int32 element)
{
	if ((index - 1) >= (sp->sp - sp->bp)) {
		StackPush(sp, element);
	} else {
		StackPush(sp, 0);
		bcopy(sp->bp + (index - 1), sp->bp + index,
		    (sp->sp - (sp->bp + (index - 1))) * sizeof (Int32));
		sp->bp[index] = element;
	}
}

Int32
ListRemove(dsObj *ds, Int32 index)
{
	Int32 rv;

	if (index >= (ds->sp - ds->bp))
		return (-1);

	rv = ds->bp[index];
	bcopy(ds->bp + (index + 1), ds->bp + index,
	    (ds->sp - (ds->bp + index)) * sizeof(Int32));
	(void)StackPop(ds);

	return (rv);
}
