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

void *StackNew(void);
void StackDelete(void *);
Int32 StackPop(void *);
void StackPush(void *, Int32);
Int16 StackIsEmpty(void *);
void StackDoEmpty(void *);
Int16 StackNElements(void *);

#ifdef __cplusplus
}
#endif

#endif /* _STACK_H_ */
