#if !defined(_HANDLER_H_)
#define _HANDLER_H_

#include <zakdef.h>

#ifdef __cplusplus
extern "C" {
#endif

void PCityMain(void);
void SetupNewGame(void);
void DrawGame(int full);
void PostLoadGame(void);

#ifdef __cplusplus
}
#endif

#endif
