/*! \file
 * \brief interface to the handler functions
 */
#if !defined(_HANDLER_H_)
#define	_HANDLER_H_

#include <zakdef.h>

#ifdef __cplusplus
extern "C" {
#endif

void PCityMain(void);
void SetupNewGame(void);
void DrawGame(Int8 full);
void PostLoadGame(void);

#ifdef __cplusplus
}
#endif

#endif
