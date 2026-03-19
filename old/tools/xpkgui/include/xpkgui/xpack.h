#ifndef XPKGUI_XPACK_H
#define XPKGUI_XPACK_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* xpkObject;

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t fileCount;
    uint32_t flags;
    uint32_t discCode;
    uint64_t createTime;
    uint64_t modifyTime;
    uint64_t dataOffset;
    uint64_t indexOffset;
    uint64_t dataLength;
    uint64_t indexLength;
} xpkHead;

typedef struct {
    uint32_t fileCount;
    uint64_t totalSize;
    uint64_t packedSize;
    double ratio;
} xpkStat;

#define XPK_TYPE_CORE 0
#define XPK_TYPE_INDEX 1
#define XPK_TYPE_LINUX 2
#define XPK_TYPE_WIN32 3

xpkObject xpkOpen(const char* path, int mode, int flags);
int xpkClose(xpkObject xpk);
int xpkSave(xpkObject xpk);

int xpkType(xpkObject xpk);
int xpkTypeSet(xpkObject xpk, int type);

uint32_t xpkCount(xpkObject xpk);

uint32_t xpkInfoSize(xpkObject xpk, uint32_t index);
uint32_t xpkInfoPacked(xpkObject xpk, uint32_t index);
int xpkInfoLevel(xpkObject xpk, uint32_t index);
int xpkInfoType(xpkObject xpk, uint32_t index);
uint32_t xpkInfoHash(xpkObject xpk, uint32_t index);

const char* xpkPathGet(xpkObject xpk, uint32_t index);
int xpkPathAppendFile(xpkObject xpk, const char* name, const char* path, int level);
int xpkPathRemove(xpkObject xpk, const char* path);
int xpkPathUpdateFile(xpkObject xpk, const char* oldPath, const char* newPath, int level);
int xpkPathExtractFile(xpkObject xpk, const char* path, const char* savePath);

int xpkStatGet(xpkObject xpk, xpkStat* stat);
int xpkVerify(xpkObject xpk, uint32_t index);
int xpkVerifyAll(xpkObject xpk);
int xpkExtractAll(xpkObject xpk, const char* path);
int xpkRebuild(xpkObject xpk);

xpkHead* xpkGetHead(xpkObject xpk);

int xpkSolidMode(xpkObject xpk);
int xpkSolidModeSet(xpkObject xpk, int mode);

int xpkVolumeMode(xpkObject xpk);
int xpkVolumeModeSet(xpkObject xpk, int mode);
uint32_t xpkVolumeSize(xpkObject xpk);
int xpkVolumeSizeSet(xpkObject xpk, uint32_t size);

uint32_t xpkDiscCode(xpkObject xpk);
int xpkDiscCodeSet(xpkObject xpk, uint32_t code);

void xpkSolidBlockInfo(xpkObject xpk, uint32_t* offset, uint32_t* size);

int xpkLastError(void);
const char* xpkLastErrorMsg(void);
void xpkOnError(void (*handler)(int code, const char* msg));

#ifdef __cplusplus
}
#endif

#endif
