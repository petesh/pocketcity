#if !defined(_STACK_H_)
#define _STACK_H_
/* interface to the stack functions */

void *StackNew(void);
void StackDelete(void *);
long StackPop(void *);
void StackPush(void *, long);
int StackIsEmpty(void *);
void StackDoEmpty(void *);
int StackNElements(void *);

#endif /* _STACK_H_ */
