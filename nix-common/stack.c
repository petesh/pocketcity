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
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <appconfig.h>

typedef struct tag_dsObj {
	Int32 *bp;
	Int32 *sp;
	Int32 *se;
	Int32 sl;
} dsObj;

#define	STACK_IMPL
#include <stack.h>

/*
 * resize the stack based on the current size
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

/*
 * Create a new stack/list
 */
dsObj *
StackNew(void)
{
	dsObj *rv = calloc(sizeof (dsObj), 1);
	return (rv);
}

/*
 * Delete the stack
 */
void
StackDelete(dsObj *sp)
{
	if (sp->bp) free(sp->bp);
	free(sp);
}

/*
 * add an item to the stack
 */
void
StackPush(dsObj *sp, Int32 value)
{
	if (sp->sp >= sp->se) StackResize(sp, sp->sl ? sp->sl << 1 : 64);
	*(++sp->sp) = value;
}

/*
 * pop an item from the stack
 */
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

/*
 * is the stack empty
 */
Int8
StackIsEmpty(dsObj *sp)
{
	if (sp->sp == NULL)
		return ((Int8)1);
	return ((Int8)(sp->sp < sp->bp));
}

/*
 * empty the stack
 */
void
StackDoEmpty(dsObj *sp)
{
	if (sp->sp != NULL)
		sp->sp = sp->bp - 1;
}

/*!
 * \brief get count of number of emements in stack
 */
int
StackNElements(dsObj *sp)
{
	return ((sp->sp+1) - sp->bp);
}

/*!
 * \brief get an element form the list
 * \param sp the list data structure
 * \param index the element index to retrieve
 * \return the element in the list (or -1)
 */
Int32
ListGet(dsObj *sp, Int32 index)
{
	if (index >= (sp->sp - sp->bp))
		return (-1);
	return (sp->bp[index + 1]);
}

/*!
 * \brief set an element in the list
 * \param sp the pointer to the stack object
 * \param index the index of the item
 * \param element the value to set the item to
 */
void
ListSet(dsObj *sp, Int32 index, Int32 element)
{
	if (index < (sp->sp - sp->bp))
		sp->bp[index+1] = element;
}

/*!
 * \brief insert an element into a list
 * \param sp the list data structure
 * \param index the index to place the element
 * \param element the element to add
 */
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

/*
 * \brief remove an element from a list
 * \param sp the data structure pointer
 * \param index the element index to remove from the list
 * \return the list element that was removed (or -1)
 */
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
