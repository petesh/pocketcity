/*!
 * \file
 * \brief interface to some utility functions for the palm paltform
 *
 * These routines are considered useful enough to be stuffed into the same
 * file.
 */
#if !defined(_PALMUTILS_H_)
#define	_PALMUTILS_H_

#include <PalmTypes.h>
#include <Form.h>
#include <sections.h>

/*!
 * \brief can this device perform color operations at the depth requested
 * \param nbits the number of bits (1, 2, 4, 8, 16)
 * \return true if this depth is available
 */
Boolean canColor(UInt16 nbits);

/*!
 * \brief Change the depth
 *
 * This will consequently set the resolution of the screen
 * \param ncolors the depth to try to set the screen resolution to
 * \param tryHigh try to put the screen in high resolution mode.
 * \return errNone if nothing untoward happens, otherwise an error
 */
Err changeDepthRes(UInt32 ncolors, Boolean tryHigh);

/*!
 * \brief restore the screen to the original depth and resolution
 * \return error if it can't restore the depth and resolution
 */
Err restoreDepthRes(void);

/*!
 * \brief Return the depth in bits per pixel
 * \return the screen depth.
 */
UInt32 getDepth(void);

/*!
 * \brief get the applications' creator ID
 * \return the creatorID. Machine will crash otherwise
 */
UInt32 GetCreatorID(void);

/*!
 * \brief is the device an old Zire (palmos 4)
 * \return true if the item is an original zire.
 */
Boolean isZireOld(void);

/*!
 * \brief is this device a HandEra machine
 * \return true of this is a handera machine.
 */
Boolean isHandEra(void);

#if defined(PALM_FIVE)
/*!
 * \brief does this device have a 5-way navigator
 * \return true if this device has a 5-way navigator
 */
Boolean hasFiveWayNav(void);
#endif

/*!
 * \brief build a string list from all the string list items from resID
 * \param resID the resource to get the strings from
 * \param length (out) the # of items in the list
 * \return the char array containing all the strings
 */
Char **FillStringList(UInt16 resID, UInt16 *length) LARD_SECTION;

/*!
 * \brief free the contents of a string list
 *
 * The list wil have been obtained from the FillStringList function
 * \param list the list to free
 */
void FreeStringList(Char **list) LARD_SECTION;

/*!
 * \brief get an object pointer from an item index
 * \param form the form to obtain the pointer from
 * \param index the index of the item on the form
 * \return the pointer
 */
void *GetObjectPtr(FormType *form, UInt16 index);

/*!
 * \brief rearrange the location of a bitmap on screen
 * \param form the form that the bitmap resides on
 * \param oID the id of the bitmap item on the form
 * \param offsetX the offset to move the bitmap by on the X axis
 * \param offsetY the offset to move the bitmap on the y axis.
 */
void RearrangeBitmap(FormPtr form, UInt16 oID, Int16 offsetX, Int16 offsetY);

/*!
 * \brief rearrange/move/resize an object on the screen
 * \param form the form who's item we wish to rearrange is on.
 * \param oID object's ientifier
 * \param offsetX x offset to move object by
 * \param offsetY y offset to move object by
 * \param resizeX amount to resize X axis by
 * \param resizeY amount to resize Y axis by
 */
void RearrangeObjectOnly(FormPtr form, UInt16 oID, Int16 offsetX,
    Int16 offsetY, Int16 resizeX, Int16 resizeY);

/*!
 * \brief get a bitmap's dimensions
 *
 * It's for compatibly with pre-PalmOS4 machines.
 * \param pBmp pointer to the bitmap
 * \param pWidth pointer to the width
 * \param pHeight pointer to the height
 * \param pRowBytes pointer to # of bytes in a  row
 */
void compatBmpGetDimensions(BitmapPtr pBmp, Coord *pWidth, Coord *pHeight,
    UInt16 *pRowBytes);

#if defined(DEBUG)
/*!
 * \brief display a warning in the program at a certain file and line
 * \param information the informational message to display
 * \param file the name of the file the message came from
 * \param line the line that the error occurred
 */
void DangerWillRobinson(char *information, char *file, int line);
/*!
 * \brief Display a warning dialog
 *
 * The use of this function can seriously lead to code bloat (strings)
 * \param mesg the message to display
 */
#define	Warning(mesg) DangerWillRobinson(mesg, __FILE__, __LINE__)

#else
#define	Warning(mesg)
#endif

#endif /* _PALMUTILS_H_ */
