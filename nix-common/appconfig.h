/*! \file
 * \brief the linux application configuration for state
 *
 * This contains the data types for the linux gtk environment.
 *
 * The name 'linux' is actually a misnomer, as it will aslo work
 * on Solaris and under the cygwin envronment under windows.
 */

#if !defined(_APPCONFIG_H)
#define _APPCONFIG_H

#if defined(__GNUC__) && defined(__UNIX__)
	#pragma pack(2)
#endif

#if !defined(LP64)
/*!
 * This is the Byte data type. assume ILP32 ... terrible, I know
 */
typedef unsigned char Byte;

/*! \brief Int8 data type to correspond to palm data type */
typedef signed char Int8;
/*! \brief Char data type to correspond to palm data type */
typedef signed char Char;
/*! \brief UInt8 data type to correspond to palm data type */
typedef unsigned char UInt8;
/*! \brief Int16 data type to correspond to palm data type */
typedef signed short Int16;
/*! \brief UInt16 data type to correspond to palm data type */
typedef unsigned short UInt16;
/*! \brief Int32 data type to correspond to palm data type */
typedef signed int Int32;
/*! \brief UInt32 data type to correspond to palm data type */
typedef unsigned int UInt32;
#endif

/*! \brief the unix application configuration */
typedef struct _NixAppConfig_01 {
    int         window_x;	/*!< horizontal window size */
    int         window_y;	/*!< Vertical window size */
    int         window_width;	/*!< Width of window */
    int         window_height;	/*!< Height of window */
} UnixAppConfig_01_t;

/*! \brief unix specific application configuration */
typedef UnixAppConfig_01_t PlatformAppConfig;

/*! \brief the default appconfig (filled in at initialization time) */
#define DEFAULT_APPCONFIG       { \
    100, 100, 400, 400 \
}

#endif /* _APPCONFIG_H */
