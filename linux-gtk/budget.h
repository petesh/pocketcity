/*!
 * \file
 * \brief the externally exposed budget routines
 */

#if !defined(_GTK_BUDGET_H_)
#define	_GTK_BUDGET_H_
#include <gtk/gtk.h>

#if defined(__cplusplus)
extern "C" {
#endif

void budget_show(void);
void UIUpdateBudget(void);

#if defined(__cplusplus)
}
#endif

#endif /* _GTK_BUDGET_H_ */
