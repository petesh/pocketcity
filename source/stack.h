#if !defined(_STACK_H_)
#define _STACK_H_
/* interface to the stack functions */

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
