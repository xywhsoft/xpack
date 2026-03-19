#ifndef XPK_MODEL_DIRTY_H
#define XPK_MODEL_DIRTY_H

typedef enum xpkDirtyBits {
    XPK_DIRTY_NONE = 0,
    XPK_DIRTY_HEAD = 1 << 0,
    XPK_DIRTY_PACKAGE_META = 1 << 1,
    XPK_DIRTY_ENTRY_TABLE = 1 << 2,
    XPK_DIRTY_DATA = 1 << 3
} xpkDirtyBits;

#endif
