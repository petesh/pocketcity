#ifndef _PALMUTILS_H_
#define _PALMUTILS_H_

#include <PalmTypes.h>
Boolean canColor(UInt16 nbits);
Err changeDepthRes(UInt32 ncolors);
Err restoreDepthRes(void);
UInt32 getDepth(void);
UInt32 GetCreatorID(void);
Char **FillStringList(UInt16 resID, UInt16 *length);
void FreeStringList(Char **list);

#endif /* _PALMUTILS_H_ */

