/*! \file
 * \brief interface to the handler functions
 */
#if !defined(_HANDLER_H_)
#define	_HANDLER_H_

#include <zakdef.h>
#include <compilerpragmas.h>

#ifdef __cplusplus
extern "C" {
#endif

EXPORT void PCityMain(void);
EXPORT void ConfigureNewGame(void);
EXPORT void InitGameStruct(void);
EXPORT void DrawGame(Int8 full);
EXPORT void PostLoadGame(void);

#ifdef __cplusplus
}
#endif

#endif
