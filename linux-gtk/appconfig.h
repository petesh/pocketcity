#if !defined(_APPCONFIG_H)
#define _APPCONFIG_H

typedef struct _PalmAppConfig_01 {
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
