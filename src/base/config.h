#ifndef XPK_BASE_CONFIG_H
#define XPK_BASE_CONFIG_H

#define XPK_ERROR_TEXT_CAP 256

#if defined(__GNUC__)
    #define XPK_UNUSED __attribute__((unused))
#else
    #define XPK_UNUSED
#endif

#endif
