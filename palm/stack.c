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

#include <resCompat.h>

typedef struct tag_dsobj {
	MemHandle   sh; /* Start of stack... handle */
	Int32	  *ss; /* Start of stack */
	Int32	  *sp; /* Stack pointer */
	Int32	  *se; /* Stack pointer */
	Int32	   sl; /* Stack Len */
} dsObj;

#define STACK_IMPL
#include <stack.h>

/*
 * preallocates 128 elements to the stack
 */
dsObj *
StackNew(void)
{
	dsObj *s = (dsObj *)MemPtrNew(sizeof (dsObj));
	if (s == NULL)
		return (NULL);
	s->sl = 128;
	if ((NULL == (s->sh = MemHandleNew(s->sl * sizeof (Int32)))) ||
	    (NULL == (s->ss = (Int32 *)MemHandleLock(s->sh)))) {
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
StackDelete(dsObj *sp)
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
Int32
StackPop(dsObj *sp)
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
StackPush(dsObj *sp, Int32 elt)
{
	if (sp->sp >= sp->se) {
		Int32 sd = sp->sp - sp->ss;
		Int32 sn;
		sp->sl <<= 1;
		sn = sp->sl * sizeof (Int32);
		MemHandleUnlock(sp->sh);
		if (errNone != MemHandleResize(sp->sh, sn)) {
			ErrFatalDisplayIf(1, "Resize of myStack Chunk Failed");
		}
		sp->ss = (Int32 *)MemHandleLock(sp->sh);
		sp->se = sp->ss + (sp->sl - 1);
		sp->sp = sp->ss + sd;
	}
	*(++sp->sp) = elt;
}

/*
 * check if the stack is empty
 */
Int8
StackIsEmpty(dsObj *sp)
{
	return (sp->sp < sp->ss);
}

/*
 * 'empty' the stack.
 */
void
StackDoEmpty(dsObj *sp)
{
	sp->sp = sp->ss - 1;
}

/*
 * get the count of the number of elements on the stack
 */
Int32
StackNElements(dsObj *sp)
{
	return ((sp->sp+1) - sp->ss);
}

/*!
 * \brief get an item from the list
 * \param sp The pointer to the data object
 * \param index the index of the object
 * \return the item at the location, or -1 on error
 */
Int32
ListGet(dsObj *sp, Int32 index)
{
	if (index >= (sp->sp - sp->ss))
		return (-1);
	return (sp->ss[index + 1]);
}

/*!
 * \brief set an item in the list
 * \param sp the pointer to the data object
 * \param index the index of the item
 * \param element the value for the new item
 */
void
ListSet(dsObj *sp, Int32 index, Int32 element)
{
	if (index < (sp->sp - sp->ss))
		sp->ss[index + 1] = element;
}
