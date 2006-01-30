
#include <mem_compat.h>
#include <stddef.h>
#include <config.h>

/*!
 * \brief the stack data structure
 * This is the data structure that contains the stack
 */
typedef struct _dsobj {
	Int32 *bp; /*!< base pointer */
	Int32 *sp; /*!< stack pointer */
	Int32 *se; /*!< stack end */
	Int32 sl; /*!< stack length */
} lsObj_t;

#define STACK_IMPL
#include <stack.h>

/*!
 * \brief resize the stack based on the current size
 * \param sp the stack object
 * \param newSize the new size of the stack
 */
static void
StackResize(lsObj_t *sp, Int32 newSize)
{
	ptrdiff_t sd;

	assert(sp);
	sd = sp->sp - sp->bp;
	sp->sl = newSize;
	sp->bp = gRealloc(sp->bp, sp->sl * sizeof (Int32));
	sp->se = sp->bp + (sp->sl - 1);
	sp->sp = sp->bp + sd;
}

lsObj_t *
StackNew(void)
{
	lsObj_t *rv = gCalloc(sizeof (lsObj_t), 1);
	return (rv);
}

void
StackDelete(lsObj_t *sp)
{
	if (sp == NULL)
		return;
	if (sp->bp) gFree(sp->bp);
	gFree(sp);
}

void
StackPush(lsObj_t *sp, Int32 value)
{
	assert(sp);
	if (sp->sp >= sp->se)
		StackResize(sp, sp->sl ? sp->sl << 1 : 64);
	*(sp->sp++) = value;
}

Int32
StackPop(lsObj_t *sp)
{
	assert(sp);
	assert(sp->sp != NULL);
	assert(sp->sp > sp->bp);
	assert(sp->bp != NULL);
	sp->sp--;
	return (*sp->sp);
}

Int8
StackIsEmpty(lsObj_t *sp)
{
	assert(sp);
	return ((Int8)(sp->sp == sp->bp));
}

void
StackDoEmpty(lsObj_t *sp)
{
	assert(sp);
	sp->sp = sp->bp;
}

Int32
StackNElements(lsObj_t *sp)
{
	assert(sp);
	return (sp->sp - sp->bp);
}

Int32
ListGet(lsObj_t *sp, Int32 index)
{
	assert(sp);
	assert (index < (sp->sp - sp->bp));
	return (sp->bp[index]);
}

void
ListSet(lsObj_t *sp, Int32 index, Int32 element)
{
	assert(sp);
	assert(index < (sp->sp - sp->bp));
	sp->bp[index] = element;
}

void
ListInsert(lsObj_t *sp, Int32 index, Int32 element)
{
	assert(sp);
	if ((index) >= (sp->sp - sp->bp)) {
		StackPush(sp, element);
	} else {
		StackPush(sp, 0);
		gMemMove(sp->bp + index + 1, sp->bp + index,
		    (sp->sp - (sp->bp + index)) * sizeof (Int32));
		sp->bp[index] = element;
	}
}

Int32
ListRemove(lsObj_t *ds, Int32 index)
{
	Int32 rv;

	assert(ds);
	assert(index < (ds->sp - ds->bp));
	rv = ds->bp[index];
	gMemMove(ds->bp + index, ds->bp + (index + 1),
	    (ds->sp - (ds->bp + index)) * sizeof (Int32));
	(void)StackPop(ds);

	return (rv);
}
