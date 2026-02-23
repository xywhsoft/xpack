/*
 * xPack Ver7 - 主实现文件
 *
 * Core 模式完整实现
 */

#define XRT_IMPLEMENTATION
#include "xpack.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "src/base.h"
#include "src/compress.h"
#include "src/rawio.h"

/* clang-format off */

// ============================================================================
// 全局错误状态
// ============================================================================
int                 g_xpkLastError = 0;
char                g_xpkLastErrorMsg[256] = {0};
const char*         g_xpkErrorMessages[] = {
    "Success",                      // 0
    "Invalid parameter",            // 1
    "File operation failed",        // 2
    "Memory allocation failed",     // 3
    "Invalid package format",       // 4
    "Version not supported",        // 5
    "Invalid index",                // 6
    "Compression failed",           // 7
    "Decompression failed",         // 8
    "Hash verification failed",     // 9
    "Write denied in readonly",     // 10
    "Type mismatch",                // 11
    "Already exists",               // 12
    "Not found",                    // 13
    "Volume error",                 // 14
    "Solid mode restriction",       // 15
};

// ============================================================================
// 错误处理
// ============================================================================
static void xpkSetError(int code, const char* msg)
{
    g_xpkLastError = code;
    if (msg) {
        strncpy(g_xpkLastErrorMsg, msg, sizeof(g_xpkLastErrorMsg) - 1);
        g_xpkLastErrorMsg[sizeof(g_xpkLastErrorMsg) - 1] = '\0';
    } else if (code >= 0 && code <= 15) {
        strncpy(g_xpkLastErrorMsg, g_xpkErrorMessages[code], sizeof(g_xpkLastErrorMsg) - 1);
        g_xpkLastErrorMsg[sizeof(g_xpkLastErrorMsg) - 1] = '\0';
    } else {
        g_xpkLastErrorMsg[0] = '\0';
    }
}

// ============================================================================
// 工具函数（用于测试）
// ============================================================================
XPKAPI void xpkFree(void* ptr)
{
    if (ptr) free(ptr);
}

XPKAPI int xpkLastError(void)
{
    return g_xpkLastError;
}

XPKAPI const char* xpkLastErrorMsg(void)
{
    return g_xpkLastErrorMsg;
}

// ============================================================================
// 压缩测试接口（内部测试用）
// ============================================================================
XPKAPI uint64_t xpkTestCompressBound(int level, uint64_t srcSize)
{
    return _xpkCompressBound(level, srcSize);
}

XPKAPI int xpkTestCompress(int level, const void* src, uint64_t srcSize,
                            void* dst, uint64_t dstCapacity,
                            uint64_t* outSize, uint32_t* outHash)
{
    xpkCompResult result;
    int ret = _xpkCompress(level, src, srcSize, dst, dstCapacity, &result);
    if (ret == 0) {
        if (outSize) *outSize = result.compSize;
        if (outHash) *outHash = result.hash;
    }
    return ret;
}

XPKAPI int xpkTestDecompress(int level, const void* src, uint64_t srcSize,
                              void* dst, uint64_t dstSize, uint32_t expectedHash)
{
    return _xpkDecompress(level, src, srcSize, dst, dstSize, expectedHash);
}

// ============================================================================
// 分卷虚拟 I/O 实现
// ============================================================================

static int _xpkRawSeekVol(xpkObject xpk, int volIndex, uint64_t offset)
{
    if (!xpk || volIndex < 0 || volIndex >= xpk->volCount) {
        return -1;
    }
    FILE* fp = xpk->volFiles[volIndex];
    if (!fp) {
        return -1;
    }
    return fseek(fp, (long)offset, SEEK_SET);
}

static uint64_t _xpkRawGetFileSize(xpkObject xpk, int volIndex)
{
    if (!xpk || volIndex < 0 || volIndex >= xpk->volCount) {
        return 0;
    }
    FILE* fp = xpk->volFiles[volIndex];
    if (!fp) {
        return 0;
    }
    long oldPos = ftell(fp);
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, oldPos, SEEK_SET);
    return (uint64_t)size;
}

static int _xpkEnsureVolume(xpkObject xpk, int volIndex)
{
    if (!xpk || volIndex < 0) {
        return -1;
    }
    if (volIndex < xpk->volCount) {
        return 0;
    }
    if (xpk->readonly) {
        return -1;
    }
    int oldCount = xpk->volCount;
    int newCount = volIndex + 1;
    
    char** newPaths = (char**)realloc(xpk->volPaths, sizeof(char*) * newCount);
    if (!newPaths) {
        return -1;
    }
    xpk->volPaths = newPaths;
    
    FILE** newFiles = (FILE**)realloc(xpk->volFiles, sizeof(FILE*) * newCount);
    if (!newFiles) {
        return -1;
    }
    xpk->volFiles = newFiles;
    
    for (int i = oldCount; i < newCount; i++) {
        char volPath[512];
        if (i == 0) {
            snprintf(volPath, sizeof(volPath), "%s.xpk", xpk->basePath);
        } else {
            snprintf(volPath, sizeof(volPath), "%s.%03d.xpk", xpk->basePath, i);
        }
        size_t volPathLen = strlen(volPath);
        xpk->volPaths[i] = (char*)malloc(volPathLen + 1);
        if (xpk->volPaths[i]) {
            memcpy(xpk->volPaths[i], volPath, volPathLen + 1);
        }
        xpk->volFiles[i] = fopen(volPath, "r+b");
        if (!xpk->volFiles[i]) {
            xpk->volFiles[i] = fopen(volPath, "w+b");
        }
        if (!xpk->volFiles[i]) {
            xpk->volCount = i;
            return -1;
        }
    }
    xpk->volCount = newCount;
    return 0;
}

int _xpkRawRead(xpkObject xpk, uint64_t offset, uint64_t size, void* data)
{
    if (!xpk || !data || size == 0) {
        return -1;
    }
    
    uint8_t* ptr = (uint8_t*)data;
    uint64_t remain = size;
    
    while (remain > 0) {
        int volIndex = _xpkCalcVolumeIndex(xpk, offset);
        if (volIndex >= xpk->volCount) {
            break;
        }
        uint64_t volOff = _xpkCalcVolumeOffset(xpk, offset);
        uint64_t volRem = _xpkCalcVolumeRemain(xpk, offset);
        uint64_t toRead = (remain < volRem) ? remain : volRem;
        
        if (_xpkRawSeekVol(xpk, volIndex, volOff) != 0) {
            return -1;
        }
        
        size_t readBytes = fread(ptr, 1, (size_t)toRead, xpk->volFiles[volIndex]);
        if (readBytes != toRead) {
            return -1;
        }
        
        ptr += readBytes;
        remain -= readBytes;
        offset += readBytes;
    }
    
    return 0;
}

int _xpkRawWrite(xpkObject xpk, uint64_t offset, uint64_t size, const void* data)
{
    if (!xpk || !data || size == 0) {
        return -1;
    }
    if (xpk->readonly) {
        return -1;
    }
    
    const uint8_t* ptr = (const uint8_t*)data;
    uint64_t remain = size;
    
    while (remain > 0) {
        int volIndex = _xpkCalcVolumeIndex(xpk, offset);
        if (_xpkEnsureVolume(xpk, volIndex) != 0) {
            return -1;
        }
        uint64_t volOff = _xpkCalcVolumeOffset(xpk, offset);
        uint64_t volRem = _xpkCalcVolumeRemain(xpk, offset);
        uint64_t toWrite = (remain < volRem) ? remain : volRem;
        
        if (_xpkRawSeekVol(xpk, volIndex, volOff) != 0) {
            return -1;
        }
        
        size_t written = fwrite(ptr, 1, (size_t)toWrite, xpk->volFiles[volIndex]);
        if (written != toWrite) {
            return -1;
        }
        fflush(xpk->volFiles[volIndex]);
        
        ptr += written;
        remain -= written;
        offset += written;
    }
    
    xpk->dirty = 1;
    return 0;
}

// ============================================================================
// 公开 API - 原始读写
// ============================================================================

XPKAPI int xpkRawRead(xpkObject xpk, uint64_t offset, uint64_t size, void* data)
{
    return _xpkRawRead(xpk, offset, size, data);
}

XPKAPI int xpkRawWrite(xpkObject xpk, uint64_t offset, uint64_t size, const void* data)
{
    return _xpkRawWrite(xpk, offset, size, data);
}

// ============================================================================
// 生命周期管理
// ============================================================================

XPKAPI xpkObject xpkOpen(const char* path, uint64_t offset, int readonly)
{
    if (!path) {
        xpkSetError(XPK_ERR_PARAM, NULL);
        return NULL;
    }
    
    xpkObject xpk = (xpkObject)calloc(1, sizeof(struct xpkStruct));
    if (!xpk) {
        xpkSetError(XPK_ERR_MEMORY, NULL);
        return NULL;
    }
    
    xpk->readonly = readonly;
    xpk->volSize = 0;
    
    size_t pathLen = strlen(path);
    xpk->basePath = (char*)malloc(pathLen + 1);
    if (!xpk->basePath) {
        free(xpk);
        xpkSetError(XPK_ERR_MEMORY, NULL);
        return NULL;
    }
    memcpy(xpk->basePath, path, pathLen + 1);
    
    xpk->volPaths = (char**)malloc(sizeof(char*));
    xpk->volFiles = (FILE**)malloc(sizeof(FILE*));
    if (!xpk->volPaths || !xpk->volFiles) {
        free(xpk->basePath);
        free(xpk->volPaths);
        free(xpk->volFiles);
        free(xpk);
        xpkSetError(XPK_ERR_MEMORY, NULL);
        return NULL;
    }
    xpk->volCount = 0;
    
    char mainPath[512];
    snprintf(mainPath, sizeof(mainPath), "%s.xpk", path);
    size_t mainPathLen = strlen(mainPath);
    xpk->volPaths[0] = (char*)malloc(mainPathLen + 1);
    if (xpk->volPaths[0]) {
        memcpy(xpk->volPaths[0], mainPath, mainPathLen + 1);
    }
    
    const char* mode = readonly ? "rb" : "r+b";
    xpk->volFiles[0] = fopen(mainPath, mode);
    if (!xpk->volFiles[0] && !readonly) {
        xpk->volFiles[0] = fopen(mainPath, "w+b");
    }
    
    if (!xpk->volFiles[0]) {
        free(xpk->volPaths[0]);
        free(xpk->basePath);
        free(xpk->volPaths);
        free(xpk->volFiles);
        free(xpk);
        xpkSetError(XPK_ERR_FILE, "Cannot open package file");
        return NULL;
    }
    xpk->volCount = 1;
    
    uint64_t fileSize = _xpkRawGetFileSize(xpk, 0);
    if (fileSize >= XPK_HEAD_SIZE && offset == 0) {
        if (_xpkRawRead(xpk, 0, XPK_HEAD_SIZE, &xpk->head) != 0) {
            xpkClose(xpk);
            xpkSetError(XPK_ERR_FILE, "Failed to read header");
            return NULL;
        }
        
        if (xpk->head.fileHead != XPK_VERSION) {
            xpkClose(xpk);
            xpkSetError(XPK_ERR_VERSION, "Unsupported package version");
            return NULL;
        }
        
        xpk->writePos = xpk->head.dataOffset;
        
        if (xpk->head.volumeMode && xpk->head.volumeSize > 0) {
            xpk->volSize = xpk->head.volumeSize;
            
            for (int i = 1; ; i++) {
                char volPath[512];
                snprintf(volPath, sizeof(volPath), "%s.%03d.xpk", path, i);
                if (!xrtFileExists(volPath)) {
                    break;
                }
                
                int idx = xpk->volCount;
                char** newPaths = (char**)realloc(xpk->volPaths, sizeof(char*) * (idx + 1));
                FILE** newFiles = (FILE**)realloc(xpk->volFiles, sizeof(FILE*) * (idx + 1));
                if (!newPaths || !newFiles) {
                    break;
                }
                xpk->volPaths = newPaths;
                xpk->volFiles = newFiles;
                
                size_t vpLen = strlen(volPath);
                xpk->volPaths[idx] = (char*)malloc(vpLen + 1);
                if (xpk->volPaths[idx]) {
                    memcpy(xpk->volPaths[idx], volPath, vpLen + 1);
                }
                xpk->volFiles[idx] = fopen(volPath, mode);
                if (xpk->volFiles[idx]) {
                    xpk->volCount++;
                } else {
                    free(xpk->volPaths[idx]);
                    break;
                }
            }
        }
        
        if (_xpkLoadMeta(xpk) != 0) {
            xpkClose(xpk);
            xpkSetError(XPK_ERR_FORMAT, "Failed to load meta data");
            return NULL;
        }
    } else {
        memset(&xpk->head, 0, sizeof(xpkHead));
        xpk->head.fileHead = XPK_VERSION;
        xpk->head.defComp = XPK_COMP_DEFAULT;
        xpk->head.metaComp = XPK_META_COMP;
        xpk->head.ldbComp = XPK_LDB_COMP;
        xpk->head.createTime = xrtNow();
        xpk->head.changeTime = xpk->head.createTime;
        xpk->head.dataOffset = XPK_HEAD_SIZE;
        xpk->writePos = XPK_HEAD_SIZE;
        xpk->dirty = 1;
    }
    
    xpkSetError(XPK_OK, NULL);
    return xpk;
}

XPKAPI int xpkSave(xpkObject xpk)
{
    if (!xpk) {
        xpkSetError(XPK_ERR_PARAM, NULL);
        return -1;
    }
    if (xpk->readonly) {
        xpkSetError(XPK_ERR_READONLY, NULL);
        return -1;
    }
    if (!xpk->dirty) {
        return 0;
    }
    
    if (_xpkSaveMeta(xpk) != 0) {
        xpkSetError(XPK_ERR_FILE, "Failed to save meta");
        return -1;
    }
    
    xpk->head.changeTime = xrtNow();
    
    if (_xpkRawWrite(xpk, 0, XPK_HEAD_SIZE, &xpk->head) != 0) {
        xpkSetError(XPK_ERR_FILE, "Failed to write header");
        return -1;
    }
    
    for (int i = 0; i < xpk->volCount; i++) {
        if (xpk->volFiles[i]) {
            fflush(xpk->volFiles[i]);
        }
    }
    
    xpk->dirty = 0;
    xpkSetError(XPK_OK, NULL);
    return 0;
}

XPKAPI void xpkClose(xpkObject xpk)
{
    if (!xpk) {
        return;
    }
    
    if (xpk->dirty && !xpk->readonly) {
        xpkSave(xpk);
    }
    
    for (int i = 0; i < xpk->volCount; i++) {
        if (xpk->volFiles[i]) {
            fclose(xpk->volFiles[i]);
        }
        if (xpk->volPaths[i]) {
            free(xpk->volPaths[i]);
        }
    }
    
    if (xpk->volFiles) free(xpk->volFiles);
    if (xpk->volPaths) free(xpk->volPaths);
    if (xpk->basePath) free(xpk->basePath);
    if (xpk->files) free(xpk->files);
    if (xpk->ldb) free(xpk->ldb);
    
    free(xpk);
}

// ============================================================================
// 包属性操作
// ============================================================================

XPKAPI int xpkType(xpkObject xpk)
{
    if (!xpk) return -1;
    return (int)xpk->head.packType;
}

XPKAPI int xpkTypeSet(xpkObject xpk, int type)
{
    if (!xpk || type < 0 || type > 3) {
        xpkSetError(XPK_ERR_PARAM, NULL);
        return -1;
    }
    if (xpk->readonly) {
        xpkSetError(XPK_ERR_READONLY, NULL);
        return -1;
    }
    xpk->head.packType = (uint32_t)type;
    xpk->dirty = 1;
    return 0;
}

XPKAPI uint32_t xpkCount(xpkObject xpk)
{
    if (!xpk) return 0;
    return xpk->head.fileCount;
}

XPKAPI xpkHead* xpkGetHead(xpkObject xpk)
{
    if (!xpk) return NULL;
    return &xpk->head;
}

// ============================================================================
// 分卷控制
// ============================================================================

XPKAPI int xpkVolumeMode(xpkObject xpk)
{
    if (!xpk) return 0;
    return (int)xpk->head.volumeMode;
}

XPKAPI int xpkVolumeModeSet(xpkObject xpk, int enabled)
{
    if (!xpk) {
        xpkSetError(XPK_ERR_PARAM, NULL);
        return -1;
    }
    if (xpk->readonly) {
        xpkSetError(XPK_ERR_READONLY, NULL);
        return -1;
    }
    xpk->head.volumeMode = enabled ? 1 : 0;
    xpk->dirty = 1;
    return 0;
}

XPKAPI uint32_t xpkVolumeSize(xpkObject xpk)
{
    if (!xpk) return 0;
    return xpk->head.volumeSize;
}

XPKAPI int xpkVolumeSizeSet(xpkObject xpk, uint32_t size)
{
    if (!xpk) {
        xpkSetError(XPK_ERR_PARAM, NULL);
        return -1;
    }
    if (xpk->readonly) {
        xpkSetError(XPK_ERR_READONLY, NULL);
        return -1;
    }
    if (xpk->head.fileCount > 0) {
        xpkSetError(XPK_ERR_SOLID, "Cannot change volume size after files added");
        return -1;
    }
    xpk->head.volumeSize = size;
    xpk->volSize = size;
    xpk->dirty = 1;
    return 0;
}

XPKAPI int xpkVolumeCount(xpkObject xpk)
{
    if (!xpk) return 0;
    return xpk->volCount;
}

XPKAPI const char* xpkVolumePath(xpkObject xpk, int index)
{
    if (!xpk || index < 0 || index >= xpk->volCount) return NULL;
    return xpk->volPaths[index];
}

// ============================================================================
// Core 模式内部函数
// ============================================================================

int _xpkEnsureFilesCapacity(xpkObject xpk, uint32_t needed)
{
    if (!xpk) return -1;
    
    if (needed <= xpk->filesCapacity) {
        return 0;
    }
    
    uint32_t newCap = xpk->filesCapacity;
    if (newCap == 0) newCap = 16;
    while (newCap < needed) newCap *= 2;
    
    xpkFileInfo* newFiles = (xpkFileInfo*)realloc(xpk->files, sizeof(xpkFileInfo) * newCap);
    if (!newFiles) {
        return -1;
    }
    
    xpk->files = newFiles;
    xpk->filesCapacity = newCap;
    return 0;
}

int _xpkLoadMeta(xpkObject xpk)
{
    if (!xpk) return -1;
    
    if (xpk->head.metaCompSize == 0) {
        xpk->files = NULL;
        xpk->filesCapacity = 0;
        return 0;
    }
    
    uint8_t* compMeta = (uint8_t*)malloc(xpk->head.metaCompSize);
    if (!compMeta) {
        return -1;
    }
    
    uint64_t metaOffset = xpk->head.dataOffset;
    
    if (_xpkRawRead(xpk, metaOffset, xpk->head.metaCompSize, compMeta) != 0) {
        free(compMeta);
        return -1;
    }
    
    uint32_t rawSize = xpk->head.metaRawSize;
    uint8_t* rawMeta = (uint8_t*)malloc(rawSize);
    if (!rawMeta) {
        free(compMeta);
        return -1;
    }
    
    if (_xpkDecompress(xpk->head.metaComp, compMeta, xpk->head.metaCompSize,
                       rawMeta, rawSize, xpk->head.metaHash) != 0) {
        free(compMeta);
        free(rawMeta);
        return -1;
    }
    
    free(compMeta);
    
    xpkMetaHead* metaHead = (xpkMetaHead*)rawMeta;
    uint32_t fileCount = metaHead->fileCount;
    
    if (_xpkEnsureFilesCapacity(xpk, fileCount) != 0) {
        free(rawMeta);
        return -1;
    }
    
    xpkFileInfo* srcFiles = (xpkFileInfo*)(rawMeta + XPK_META_HEAD_SIZE);
    memcpy(xpk->files, srcFiles, sizeof(xpkFileInfo) * fileCount);
    xpk->head.fileCount = fileCount;
    
    free(rawMeta);
    return 0;
}

int _xpkSaveMeta(xpkObject xpk)
{
    if (!xpk || xpk->readonly) return -1;
    
    uint32_t fileCount = xpk->head.fileCount;
    uint32_t rawSize = XPK_META_HEAD_SIZE + sizeof(xpkFileInfo) * fileCount;
    
    uint8_t* rawMeta = (uint8_t*)malloc(rawSize);
    if (!rawMeta) {
        return -1;
    }
    
    xpkMetaHead* metaHead = (xpkMetaHead*)rawMeta;
    metaHead->fileCount = fileCount;
    metaHead->reserved[0] = 0;
    metaHead->reserved[1] = 0;
    metaHead->reserved[2] = 0;
    
    if (fileCount > 0 && xpk->files) {
        memcpy(rawMeta + XPK_META_HEAD_SIZE, xpk->files, sizeof(xpkFileInfo) * fileCount);
    }
    
    uint64_t bound = _xpkCompressBound(xpk->head.metaComp, rawSize);
    uint8_t* compMeta = (uint8_t*)malloc((size_t)bound);
    if (!compMeta) {
        free(rawMeta);
        return -1;
    }
    
    xpkCompResult result;
    if (_xpkCompress(xpk->head.metaComp, rawMeta, rawSize, compMeta, bound, &result) != 0) {
        free(rawMeta);
        free(compMeta);
        return -1;
    }
    
    xpk->head.metaRawSize = rawSize;
    xpk->head.metaCompSize = (uint32_t)result.compSize;
    xpk->head.metaHash = result.hash;
    
    uint64_t metaOffset = xpk->head.dataOffset;
    if (_xpkRawWrite(xpk, metaOffset, result.compSize, compMeta) != 0) {
        free(rawMeta);
        free(compMeta);
        return -1;
    }
    
    free(rawMeta);
    free(compMeta);
    return 0;
}

// ============================================================================
// Core 模式文件操作
// ============================================================================

XPKAPI uint32_t xpkAppendData(xpkObject xpk, const void* data, uint64_t size, int level)
{
    if (!xpk || !data || size == 0) {
        xpkSetError(XPK_ERR_PARAM, NULL);
        return (uint32_t)-1;
    }
    if (xpk->readonly) {
        xpkSetError(XPK_ERR_READONLY, NULL);
        return (uint32_t)-1;
    }
    if (level < 0) level = xpk->head.defComp;
    if (level > 15) level = 15;
    
    uint64_t bound = _xpkCompressBound(level, size);
    uint8_t* compData = (uint8_t*)malloc((size_t)bound);
    if (!compData) {
        xpkSetError(XPK_ERR_MEMORY, NULL);
        return (uint32_t)-1;
    }
    
    xpkCompResult result;
    if (_xpkCompress(level, data, size, compData, bound, &result) != 0) {
        free(compData);
        xpkSetError(XPK_ERR_COMPRESS, NULL);
        return (uint32_t)-1;
    }
    
    if (_xpkEnsureFilesCapacity(xpk, xpk->head.fileCount + 1) != 0) {
        free(compData);
        xpkSetError(XPK_ERR_MEMORY, NULL);
        return (uint32_t)-1;
    }
    
    uint64_t writeOffset = xpk->head.dataOffset;
    
    if (_xpkRawWrite(xpk, writeOffset, result.compSize, compData) != 0) {
        free(compData);
        xpkSetError(XPK_ERR_FILE, NULL);
        return (uint32_t)-1;
    }
    
    free(compData);
    
    uint32_t pos = xpk->head.fileCount;
    xpkFileInfo* info = &xpk->files[pos];
    info->flag = XPK_FLAG_SET_COMP(level) | XPK_FLAG_SET_TYPE(XPK_FTYPE_BINARY);
    info->fileHash = result.hash;
    info->dataOffset = writeOffset;
    info->dataSize = result.compSize;
    info->fileSize = size;
    
    xpk->head.fileCount++;
    xpk->head.dataOffset = writeOffset + result.compSize;
    xpk->dirty = 1;
    
    xpkSetError(XPK_OK, NULL);
    return pos;
}

XPKAPI uint32_t xpkAppendFile(xpkObject xpk, const char* path, int level)
{
    if (!xpk || !path) {
        xpkSetError(XPK_ERR_PARAM, NULL);
        return (uint32_t)-1;
    }
    
    size_t size = 0;
    void* data = xrtFileGetAll(path, &size);
    if (!data) {
        xpkSetError(XPK_ERR_FILE, "Cannot read source file");
        return (uint32_t)-1;
    }
    
    uint32_t pos = xpkAppendData(xpk, data, (uint64_t)size, level);
    free(data);
    return pos;
}

XPKAPI void* xpkExtractData(xpkObject xpk, uint32_t pos, uint64_t* outSize)
{
    if (!xpk || pos >= xpk->head.fileCount) {
        xpkSetError(XPK_ERR_INDEX, NULL);
        return NULL;
    }
    
    xpkFileInfo* info = &xpk->files[pos];
    
    if (XPK_FLAG_DELETED(info->flag)) {
        xpkSetError(XPK_ERR_NOTFOUND, "File deleted");
        return NULL;
    }
    
    uint8_t* compData = (uint8_t*)malloc((size_t)info->dataSize);
    if (!compData) {
        xpkSetError(XPK_ERR_MEMORY, NULL);
        return NULL;
    }
    
    if (_xpkRawRead(xpk, info->dataOffset, info->dataSize, compData) != 0) {
        free(compData);
        xpkSetError(XPK_ERR_FILE, NULL);
        return NULL;
    }
    
    uint8_t* rawData = (uint8_t*)malloc((size_t)info->fileSize);
    if (!rawData) {
        free(compData);
        xpkSetError(XPK_ERR_MEMORY, NULL);
        return NULL;
    }
    
    int level = XPK_FLAG_COMP_LEVEL(info->flag);
    if (_xpkDecompress(level, compData, info->dataSize, rawData, info->fileSize, info->fileHash) != 0) {
        free(compData);
        free(rawData);
        xpkSetError(XPK_ERR_HASH, "Hash verification failed");
        return NULL;
    }
    
    free(compData);
    
    if (outSize) *outSize = info->fileSize;
    xpkSetError(XPK_OK, NULL);
    return rawData;
}

XPKAPI int xpkExtractFile(xpkObject xpk, uint32_t pos, const char* path)
{
    if (!xpk || !path) {
        xpkSetError(XPK_ERR_PARAM, NULL);
        return -1;
    }
    
    uint64_t size = 0;
    void* data = xpkExtractData(xpk, pos, &size);
    if (!data) {
        return -1;
    }
    
    int ret = xrtFilePutAll(path, data, (size_t)size);
    free(data);
    
    if (ret != 0) {
        xpkSetError(XPK_ERR_FILE, "Cannot write output file");
        return -1;
    }
    
    xpkSetError(XPK_OK, NULL);
    return 0;
}

XPKAPI int xpkUpdateData(xpkObject xpk, uint32_t pos, const void* data, uint64_t size, int level)
{
    if (!xpk || !data || size == 0) {
        xpkSetError(XPK_ERR_PARAM, NULL);
        return -1;
    }
    if (xpk->readonly) {
        xpkSetError(XPK_ERR_READONLY, NULL);
        return -1;
    }
    if (pos >= xpk->head.fileCount) {
        xpkSetError(XPK_ERR_INDEX, NULL);
        return -1;
    }
    
    uint32_t newPos = xpkAppendData(xpk, data, size, level);
    if (newPos == (uint32_t)-1) {
        return -1;
    }
    
    xpk->files[pos].flag |= XPK_FLAG_SET_DELETED(1);
    xpk->dirty = 1;
    
    xpkSetError(XPK_OK, NULL);
    return 0;
}

XPKAPI int xpkUpdateFile(xpkObject xpk, uint32_t pos, const char* path, int level)
{
    if (!xpk || !path) {
        xpkSetError(XPK_ERR_PARAM, NULL);
        return -1;
    }
    
    size_t size = 0;
    void* data = xrtFileGetAll(path, &size);
    if (!data) {
        xpkSetError(XPK_ERR_FILE, "Cannot read source file");
        return -1;
    }
    
    int ret = xpkUpdateData(xpk, pos, data, (uint64_t)size, level);
    free(data);
    return ret;
}

XPKAPI int xpkRemove(xpkObject xpk, uint32_t pos)
{
    if (!xpk) {
        xpkSetError(XPK_ERR_PARAM, NULL);
        return -1;
    }
    if (xpk->readonly) {
        xpkSetError(XPK_ERR_READONLY, NULL);
        return -1;
    }
    if (pos >= xpk->head.fileCount) {
        xpkSetError(XPK_ERR_INDEX, NULL);
        return -1;
    }
    
    xpk->files[pos].flag |= XPK_FLAG_SET_DELETED(1);
    xpk->dirty = 1;
    
    xpkSetError(XPK_OK, NULL);
    return 0;
}

// ============================================================================
// 文件信息获取
// ============================================================================

XPKAPI void* xpkInfo(xpkObject xpk, uint32_t pos)
{
    if (!xpk || pos >= xpk->head.fileCount) {
        return NULL;
    }
    return &xpk->files[pos];
}

XPKAPI uint64_t xpkInfoSize(xpkObject xpk, uint32_t pos)
{
    if (!xpk || pos >= xpk->head.fileCount) return 0;
    return xpk->files[pos].fileSize;
}

XPKAPI uint64_t xpkInfoPacked(xpkObject xpk, uint32_t pos)
{
    if (!xpk || pos >= xpk->head.fileCount) return 0;
    return xpk->files[pos].dataSize;
}

XPKAPI uint32_t xpkInfoHash(xpkObject xpk, uint32_t pos)
{
    if (!xpk || pos >= xpk->head.fileCount) return 0;
    return xpk->files[pos].fileHash;
}

XPKAPI int xpkInfoLevel(xpkObject xpk, uint32_t pos)
{
    if (!xpk || pos >= xpk->head.fileCount) return -1;
    return XPK_FLAG_COMP_LEVEL(xpk->files[pos].flag);
}

XPKAPI int xpkInfoType(xpkObject xpk, uint32_t pos)
{
    if (!xpk || pos >= xpk->head.fileCount) return -1;
    return XPK_FLAG_FILE_TYPE(xpk->files[pos].flag);
}

XPKAPI int xpkInfoTypeSet(xpkObject xpk, uint32_t pos, int type)
{
    if (!xpk || pos >= xpk->head.fileCount) {
        xpkSetError(XPK_ERR_INDEX, NULL);
        return -1;
    }
    if (xpk->readonly) {
        xpkSetError(XPK_ERR_READONLY, NULL);
        return -1;
    }
    
    uint32_t flag = xpk->files[pos].flag;
    flag = (flag & ~0x000000F0) | XPK_FLAG_SET_TYPE(type);
    xpk->files[pos].flag = flag;
    xpk->dirty = 1;
    
    return 0;
}

XPKAPI int xpkInfoDeleted(xpkObject xpk, uint32_t pos)
{
    if (!xpk || pos >= xpk->head.fileCount) return 0;
    return XPK_FLAG_DELETED(xpk->files[pos].flag) ? 1 : 0;
}

// ============================================================================
// 工具函数
// ============================================================================

XPKAPI uint32_t xpkHash(const void* data, uint64_t size)
{
    if (!data || size == 0) return 0;
    return xrtHash32((const ptr)data, (const size_t)size);
}

XPKAPI int xpkVerify(xpkObject xpk, uint32_t pos)
{
    if (!xpk || pos >= xpk->head.fileCount) {
        xpkSetError(XPK_ERR_INDEX, NULL);
        return -1;
    }
    
    void* data = xpkExtractData(xpk, pos, NULL);
    if (!data) {
        return -1;
    }
    
    free(data);
    xpkSetError(XPK_OK, NULL);
    return 0;
}

XPKAPI int xpkVerifyAll(xpkObject xpk)
{
    if (!xpk) {
        xpkSetError(XPK_ERR_PARAM, NULL);
        return -1;
    }
    
    for (uint32_t i = 0; i < xpk->head.fileCount; i++) {
        if (XPK_FLAG_DELETED(xpk->files[i].flag)) continue;
        if (xpkVerify(xpk, i) != 0) {
            return -1;
        }
    }
    
    xpkSetError(XPK_OK, NULL);
    return 0;
}

XPKAPI int xpkStatGet(xpkObject xpk, xpkStat* stat)
{
    if (!xpk || !stat) {
        xpkSetError(XPK_ERR_PARAM, NULL);
        return -1;
    }
    
    memset(stat, 0, sizeof(xpkStat));
    
    for (uint32_t i = 0; i < xpk->head.fileCount; i++) {
        if (XPK_FLAG_DELETED(xpk->files[i].flag)) continue;
        stat->fileCount++;
        stat->totalSize += xpk->files[i].fileSize;
        stat->packedSize += xpk->files[i].dataSize;
    }
    
    if (stat->totalSize > 0) {
        stat->ratio = (double)stat->packedSize / (double)stat->totalSize;
    }
    
    xpkSetError(XPK_OK, NULL);
    return 0;
}

XPKAPI int xpkRebuild(xpkObject xpk)
{
    if (!xpk) {
        xpkSetError(XPK_ERR_PARAM, NULL);
        return -1;
    }
    if (xpk->readonly) {
        xpkSetError(XPK_ERR_READONLY, NULL);
        return -1;
    }
    
    size_t basePathLen = strlen(xpk->basePath);
    char* basePath = (char*)malloc(basePathLen + 1);
    if (!basePath) {
        xpkSetError(XPK_ERR_MEMORY, NULL);
        return -1;
    }
    memcpy(basePath, xpk->basePath, basePathLen + 1);
    
    int wasVolumeMode = xpk->head.volumeMode;
    uint32_t wasVolumeSize = xpk->head.volumeSize;
    int wasPackType = (int)xpk->head.packType;
    
    char tmpPath[512];
    snprintf(tmpPath, sizeof(tmpPath), "%s_tmp", basePath);
    
    char delPath[512];
    snprintf(delPath, sizeof(delPath), "%s.xpk", tmpPath);
    xrtFileDelete(delPath);
    for (int v = 1; v <= 100; v++) {
        snprintf(delPath, sizeof(delPath), "%s.%03d.xpk", tmpPath, v);
        xrtFileDelete(delPath);
    }
    
    xpkObject tmp = xpkOpen(tmpPath, 0, 0);
    if (!tmp) {
        free(basePath);
        xpkSetError(XPK_ERR_FILE, "Failed to create temp package");
        return -1;
    }
    
    if (wasVolumeMode) {
        xpkVolumeSizeSet(tmp, wasVolumeSize);
        xpkVolumeModeSet(tmp, 1);
    }
    xpkTypeSet(tmp, wasPackType);
    
    uint32_t copiedCount = 0;
    uint64_t copiedSize = 0;
    
    for (uint32_t i = 0; i < xpk->head.fileCount; i++) {
        if (XPK_FLAG_DELETED(xpk->files[i].flag)) continue;
        
        uint64_t size = 0;
        void* data = xpkExtractData(xpk, i, &size);
        if (!data) {
            xpkClose(tmp);
            free(basePath);
            
            char delPath[512];
            snprintf(delPath, sizeof(delPath), "%s.xpk", tmpPath);
            xrtFileDelete(delPath);
            for (int v = 1; v <= 100; v++) {
                snprintf(delPath, sizeof(delPath), "%s.%03d.xpk", tmpPath, v);
                xrtFileDelete(delPath);
            }
            
            xpkSetError(XPK_ERR_DECOMPRESS, "Failed to extract file for rebuild");
            return -1;
        }
        
        int level = XPK_FLAG_COMP_LEVEL(xpk->files[i].flag);
        uint32_t newPos = xpkAppendData(tmp, data, size, level);
        free(data);
        
        if (newPos == (uint32_t)-1) {
            xpkClose(tmp);
            free(basePath);
            
            char delPath[512];
            snprintf(delPath, sizeof(delPath), "%s.xpk", tmpPath);
            xrtFileDelete(delPath);
            for (int v = 1; v <= 100; v++) {
                snprintf(delPath, sizeof(delPath), "%s.%03d.xpk", tmpPath, v);
                xrtFileDelete(delPath);
            }
            
            xpkSetError(XPK_ERR_COMPRESS, "Failed to append file during rebuild");
            return -1;
        }
        
        int fileType = XPK_FLAG_FILE_TYPE(xpk->files[i].flag);
        xpkInfoTypeSet(tmp, newPos, fileType);
        
        copiedCount++;
        copiedSize += size;
    }
    
    xpkSave(tmp);
    xpkClose(tmp);
    
    char pathOld[512];
    char pathNew[512];
    
    for (int v = 1; v <= 100; v++) {
        snprintf(pathOld, sizeof(pathOld), "%s.%03d.xpk", basePath, v);
        xrtFileDelete(pathOld);
    }
    
    snprintf(pathOld, sizeof(pathOld), "%s.xpk", basePath);
    xrtFileDelete(pathOld);
    
    snprintf(pathOld, sizeof(pathOld), "%s.xpk", tmpPath);
    snprintf(pathNew, sizeof(pathNew), "%s.xpk", basePath);
    
    if (rename(pathOld, pathNew) != 0) {
        FILE* src = fopen(pathOld, "rb");
        FILE* dst = fopen(pathNew, "wb");
        if (src && dst) {
            char buf[4096];
            size_t n;
            while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
                fwrite(buf, 1, n, dst);
            }
            fclose(src);
            fclose(dst);
            xrtFileDelete(pathOld);
        } else {
            if (src) fclose(src);
            if (dst) fclose(dst);
        }
    }
    
    if (wasVolumeMode) {
        for (int v = 1; v <= 100; v++) {
            snprintf(pathOld, sizeof(pathOld), "%s.%03d.xpk", tmpPath, v);
            snprintf(pathNew, sizeof(pathNew), "%s.%03d.xpk", basePath, v);
            if (xrtFileExists(pathOld)) {
                rename(pathOld, pathNew);
            }
        }
    }
    
    for (int i = 0; i < xpk->volCount; i++) {
        if (xpk->volFiles[i]) {
            fclose(xpk->volFiles[i]);
            xpk->volFiles[i] = NULL;
        }
        if (xpk->volPaths[i]) {
            free(xpk->volPaths[i]);
            xpk->volPaths[i] = NULL;
        }
    }
    
    free(xpk->basePath);
    xpk->basePath = basePath;
    
    snprintf(pathNew, sizeof(pathNew), "%s.xpk", basePath);
    xpk->volFiles[0] = fopen(pathNew, "r+b");
    xpk->volCount = 1;
    
    if (xpk->files) {
        free(xpk->files);
        xpk->files = NULL;
        xpk->filesCapacity = 0;
    }
    
    if (_xpkRawRead(xpk, 0, XPK_HEAD_SIZE, &xpk->head) != 0) {
        xpkSetError(XPK_ERR_FILE, "Failed to read header after rebuild");
        return -1;
    }
    
    xpk->writePos = xpk->head.dataOffset;
    
    if (xpk->head.volumeMode && xpk->head.volumeSize > 0) {
        xpk->volSize = xpk->head.volumeSize;
        
        for (int v = 1; ; v++) {
            snprintf(pathNew, sizeof(pathNew), "%s.%03d.xpk", basePath, v);
            if (!xrtFileExists(pathNew)) break;
            
            if (v >= xpk->volCount) {
                char** newPaths = (char**)realloc(xpk->volPaths, sizeof(char*) * (v + 1));
                FILE** newFiles = (FILE**)realloc(xpk->volFiles, sizeof(FILE*) * (v + 1));
                if (!newPaths || !newFiles) break;
                xpk->volPaths = newPaths;
                xpk->volFiles = newFiles;
                xpk->volPaths[v] = NULL;
                xpk->volFiles[v] = NULL;
            }
            
            if (xpk->volPaths[v]) free(xpk->volPaths[v]);
            size_t vpLen = strlen(pathNew);
            xpk->volPaths[v] = (char*)malloc(vpLen + 1);
            if (xpk->volPaths[v]) {
                memcpy(xpk->volPaths[v], pathNew, vpLen + 1);
            }
            
            xpk->volFiles[v] = fopen(pathNew, "rb");
            if (xpk->volFiles[v]) {
                xpk->volCount = v + 1;
            }
        }
    }
    
    if (_xpkLoadMeta(xpk) != 0) {
        xpkSetError(XPK_ERR_FORMAT, "Failed to load meta after rebuild");
        return -1;
    }
    
    xpk->readonly = 0;
    xpk->dirty = 0;
    
    (void)copiedCount;
    (void)copiedSize;
    
    xpkSetError(XPK_OK, NULL);
    return 0;
}

/* clang-format on */
