#if !defined(_PALMUTILS_H_)
#define	_PALMUTILS_H_

#include <PalmTypes.h>
Boolean canColor(UInt16 nbits);
Err changeDepthRes(UInt32 ncolors, Boolean tryHigh);
Err restoreDepthRes(void);
UInt32 getDepth(void);
UInt32 GetCreatorID(void);
Boolean isZireOld(void);
Boolean isHandEra(void);
Char **FillStringList(UInt16 resID, UInt16 *length);
void FreeStringList(Char **list);
void RearrangeBitmap(FormPtr form, UInt16 oID, Int16 offsetX, Int16 offsetY);
void RearrangeObjectOnly(FormPtr form, UInt16 oID, Int16 offsetX,
    Int16 offsetY, Int16 resizeX, Int16 resizeY);

#if defined(DEBUG)
void DangerWillRobinson(char *, char *, int);
/*!
 * \brief Display a warning dialog
 *
 * The use of this function can seriously lead to code bloat (strings)
 * \param mesg the message to display
 */
#define	Warning(mesg) DangerWillRobinson(mesg, __FILE__, __LINE__)

#else
#define Warning(mesg)
#endif

#endif /* _PALMUTILS_H_ */
