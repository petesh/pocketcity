#if !defined(_APPCONFIG_H)
#define _APPCONFIG_H

#if defined(__GNUC__) && defined(__UNIX__)
	#pragma pack(2)
#endif

#if !defined(LP64)
/* assume ILP32 ... terrible, I know */
typedef char Int8;
typedef char Char;
typedef unsigned char UInt8;
typedef signed short Int16;
typedef unsigned short UInt16;
typedef signed int Int32;
typedef unsigned int UInt32;
#endif


typedef struct _NixAppConfig_01 {
    int         window_x;
    int         window_y;
    int         window_width;
    int         window_height;
} UnixAppConfig_01_t;

typedef UnixAppConfig_01_t PlatformAppConfig;

#define DEFAULT_APPCONFIG       { \
    100, 100, 400, 400 \
}

#endif /* _APPCONFIG_H */
