#if !defined(_STACK_H_)
#define _STACK_H_
/* interface to the stack functions */

#ifdef __cplusplus
extern "C" {
#endif

void *StackNew(void);
void StackDelete(void *);
long StackPop(void *);
void StackPush(void *, long);
int StackIsEmpty(void *);
void StackDoEmpty(void *);
int StackNElements(void *);

#ifdef __cplusplus
}
#endif

#endif /* _STACK_H_ */
