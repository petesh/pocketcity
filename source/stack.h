/*! \file
 * \brief interface to the stack functions
 *
 * These functions are indended to to provide a simplified set of stack
 * routines that mey be implemented without reallocating them.
 */

#if !defined(_STACK_H_)
#define	_STACK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <appconfig.h>

#if !defined(STACK_IMPL)
typedef void *dsObj;
#endif

dsObj *StackNew(void);
void StackDelete(dsObj *);
Int32 StackPop(dsObj *);
void StackPush(dsObj *, Int32);
Int8 StackIsEmpty(dsObj *);
void StackDoEmpty(dsObj *);
Int32 StackNElements(dsObj *);

#define ListNew	StackNew
#define ListDelete StackDelete
#define ListAdd	StackPush
#define ListNElements StackNElements

Int32 ListGet(dsObj *, Int32);
void ListSet(dsObj *, Int32, Int32);
void ListInsert(dsObj *, Int32, Int32);
Int32 ListRemove(dsObj *, Int32);

#ifdef __cplusplus
}
#endif

#endif /* _STACK_H_ */
