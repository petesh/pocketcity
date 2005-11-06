/*!
 * \file
 * \brief interface to the stack functions
 *
 * These functions are indended to to provide a simplified set of stack
 * routines that mey be implemented without reallocating them.
 */

#if !defined(_STACK_H_)
#define	_STACK_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <appconfig.h>

#if !defined(STACK_IMPL)
typedef void dsObj;
#endif

/*!
 * \brief Create a new stack
 * \return the new stack object
 */
dsObj *StackNew(void);
/*!
 * \brief delete the stack object
 * \param obj the object to delete
 */
void StackDelete(dsObj *obj);
/*!
 * \brief pop the first item from the stack
 * \param obj the stack object
 * \return the top item from the stack
 */
Int32 StackPop(dsObj *obj);
/*!
 * \brief push an item onto the stack
 * \param obj the stack
 * \param value the value to push
 */
void StackPush(dsObj *obj, Int32 value);
/*!
 * \brief check is the stack empty
 * \param obj the stack
 * \return true if the stack is empty, false otherwise
 */
Int8 StackIsEmpty(dsObj *obj);
/*!
 * \brief empty the stack
 * \param obj the stack to empty
 */
void StackDoEmpty(dsObj *obj);
/*!
 * \brief get the number of elements on the stack
 * \param obj the stack
 * \return the number of objects on the stack
 */
Int32 StackNElements(dsObj *obj);

/*! \brief create a new list */
#define	ListNew	StackNew
/*! \brief delete a list */
#define	ListDelete StackDelete
/*! \brief add an item to the list */
#define	ListAdd	StackPush
/*! \brief get number of elements on a list */
#define	ListNElements StackNElements

/*!
 * \brief get the specific item from the list
 * \param obj the list
 * \param index the index into the list to get
 * \return the value or -1 if not found
 */
Int32 ListGet(dsObj *obj, Int32 index);
/*!
 * \brief set the value at a certain index into the list to the value
 * \param obj the list
 * \param index the index into the list
 * \param value the value of that point
 */
void ListSet(dsObj *obj, Int32 index, Int32 value);
/*!
 * \brief insert an item into the list at a certain location
 *
 * It is inserted at the location shuffling the old item down one on the
 * list.
 * \param obj the list object
 * \param index the index into the list
 * \param value the new value
 */
void ListInsert(dsObj *obj, Int32 index, Int32 value);
/*!
 * \brief remove an item from the list
 * \param obj the list object
 * \param index the index from which to remove the item
 * \return the value that was stored there
 */
Int32 ListRemove(dsObj *obj, Int32 index);

#if defined(__cplusplus)
}
#endif

#endif /* _STACK_H_ */
