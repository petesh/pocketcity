/*!
 * \file
 * \brief A stack implementation that uses a growing memptr.
 *
 * Designed to hold unsigned longs...
 * I live in an ILP32 / LP64 world.
 * long == pointer in 32 bit and 64 bit. If not then just forgive me for
 * this laziness. It's just that this implementation is for the palmOS
 * platform.
 */
#include <MemoryMgr.h>
#include <ErrorMgr.h>
#include <FeatureMgr.h>

#include <ui.h>
#include <mem_compat.h>

/*! \brief stack object */
typedef struct tag_dsobj {
	MemHandle   sh; /*!< Start of stack... handle */
	Int32	  *ss; /*!< Start of stack */
	Int32	  *sp; /*!< Stack pointer */
	Int32	  *se; /*!< Stack end pointer */
	Int32	   sl; /*<! Stack Len */
} dsObj;

#define STACK_IMPL
#include <stack.h>

/*!
 * preallocates 128 elements to the stack
 */
dsObj *
StackNew(void)
{
	dsObj *s = (dsObj *)MemPtrNew(sizeof (dsObj));
	if (s == NULL)
		return (NULL);
	s->sl = 4;
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

void
StackDelete(dsObj *sp)
{
	if (sp->sh) {
		MemHandleUnlock(sp->sh);
		MemHandleFree(sp->sh);
	}
	MemPtrFree(sp);
}

static void
StackResize(dsObj *sp, Int32 newsize)
{
	Int32 sd = sp->sp - sp->ss;
	Int32 sn;
	sp->sl = newsize;
	sn = sp->sl * sizeof (Int32);
	MemHandleUnlock(sp->sh);
	if (errNone != MemHandleResize(sp->sh, sn)) {
		ErrFatalDisplayIf(1, "Resize of myStack Chunk Failed");
	}
	sp->ss = (Int32 *)MemHandleLock(sp->sh);
	sp->se = sp->ss + (sp->sl - 1);
	sp->sp = sp->ss + sd;
}

Int32
StackPop(dsObj *sp)
{
	if (sp->sp >= sp->ss) {
		Int32 rv = *sp->sp--;
		if (((sp->sl >> 2) > (sp->sp - sp->ss)) && (sp->sl > 4))
			StackResize(sp, sp->sl >> 1);
		return (rv);
	}
	ErrFatalDisplayIf(1, "myStack Underflow");
	return (-1);
}

void
StackPush(dsObj *sp, Int32 elt)
{
	if (sp->sp >= sp->se)
		StackResize(sp, sp->sl << 1);
	*(++sp->sp) = elt;
}

Int8
StackIsEmpty(dsObj *sp)
{
	return (sp->sp < sp->ss);
}

void
StackDoEmpty(dsObj *sp)
{
	sp->sp = sp->ss - 1;
}

Int32
StackNElements(dsObj *sp)
{
	return ((sp->sp+1) - sp->ss);
}

Int32
ListGet(dsObj *sp, Int32 index)
{
	if (index >= (sp->sp - sp->ss))
		return (-1);
	return (sp->ss[index + 1]);
}

void
ListSet(dsObj *sp, Int32 index, Int32 element)
{
	if (index < (sp->sp - sp->ss))
		sp->ss[index + 1] = element;
}
