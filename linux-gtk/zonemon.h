/*!
 * \file
 * \brief interface to zone monitoring
 *
 * There's the two functions. Sad, I know.
 */

#if !defined(_ZONEMON_H)
#define	_ZONEMON_H

#if defined(__cplusplus)
extern "C" {
#endif

void hoverShow(void);
void hoverUpdate(UInt16 xpos, UInt16 ypos, int force);

#if defined(__cplusplus)
}
#endif

#endif
