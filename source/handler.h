/*! \file
 * \brief interface to the handler functions
 */
#if !defined(_HANDLER_H_)
#define	_HANDLER_H_

#include <zakdef.h>
#include <compilerpragmas.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * \brief Application level initialization routines
 */
EXPORT void PCityMain(void);

/*
 * \brief clean up any state that may be lying around
 */
EXPORT void PCityShutdown(void);
EXPORT void ConfigureNewGame(void);
EXPORT void InitGameStruct(void);
EXPORT void DrawGame(Int8 full);
EXPORT void PostLoadGame(void);

/*!
 * \brief set the map size
 *
 * Does not allocate any extra memory.
 * \param X the new map X size
 * \param Y the new map Y size
 */
EXPORT void setMapSize(UInt8 X, UInt8 Y);

#if defined(__cplusplus)
}
#endif

#endif
