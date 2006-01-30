/*!
 * \file
 * \brief Platform level config file - l(u)n(i)x
 *
 * This file contains all the #define commands for the lunix platform.
 * That's linux/unix to y'all
 */
#if !defined(_CONFIG_H_)
#define	_CONFIG_H_

#define	LUNIX
#include <assert.h>
#include <stdint.h>

/*!
 * This is the Byte data type. assume ILP32 ... terrible, I know
 */
typedef uint8_t Byte;

/*! \brief Int8 data type to correspond to palm data type */
typedef int8_t Int8;

/*! \brief Char data type to correspond to palm data type */
typedef char Char;

/*! \brief UInt8 data type to correspond to palm data type */
typedef uint8_t UInt8;
/*! \brief Int16 data type to correspond to palm data type */
typedef int16_t Int16;
/*! \brief UInt16 data type to correspond to palm data type */
typedef uint16_t UInt16;
/*! \brief Int32 data type to correspond to palm data type */
typedef int32_t Int32;
/*! \brief UInt32 data type to correspond to palm data type */
typedef uint32_t UInt32;

/*! \brief the MemHandle data type - it's a pointer, nothing magic about that */
typedef void *MemHandle;
/*! \brief the MemPtr data type - it's a pointer, nothing more */
typedef void *MemPtr;


#endif /* _CONFIG_H_ */
