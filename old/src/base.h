/*
 * xPack Ver7 - 内部基础模块
 *
 * 当前实现：压缩功能所需的类型定义
 */

#ifndef XPACK_BASE_H
#define XPACK_BASE_H

#include "../xpack.h"
#include <stdlib.h>
#include <string.h>

/* clang-format off */

// ============================================================================
// 全局错误状态声明
// ============================================================================
extern int          g_xpkLastError;
extern char         g_xpkLastErrorMsg[256];
extern const char*  g_xpkErrorMessages[];

// 错误码定义
enum {
    XPK_OK              = 0,
    XPK_ERR_PARAM       = 1,
    XPK_ERR_FILE        = 2,
    XPK_ERR_MEMORY      = 3,
    XPK_ERR_FORMAT      = 4,
    XPK_ERR_VERSION     = 5,
    XPK_ERR_INDEX       = 6,
    XPK_ERR_COMPRESS    = 7,
    XPK_ERR_DECOMPRESS  = 8,
    XPK_ERR_HASH        = 9,
    XPK_ERR_READONLY    = 10,
    XPK_ERR_TYPE        = 11,
    XPK_ERR_EXISTS      = 12,
    XPK_ERR_NOTFOUND    = 13,
    XPK_ERR_VOLUME      = 14,
    XPK_ERR_SOLID       = 15,
};

// ============================================================================
// 压缩结果结构
// ============================================================================
typedef struct {
    uint64_t    compSize;        // 压缩后大小
    uint32_t    hash;            // 原始数据哈希值 (使用 xrtHash32)
    int         algorithm;       // 实际使用的算法
} xpkCompResult;

// ============================================================================
// Meta 段结构
// ============================================================================
#pragma pack(push, 1)
typedef struct {
    uint32_t    fileCount;      // 文件数量
    uint32_t    reserved[3];    // 保留 (对齐到 16 字节)
} xpkMetaHead;
#pragma pack(pop)

#define XPK_META_HEAD_SIZE  16

// ============================================================================
// 内部函数声明（压缩模块）
// ============================================================================
uint64_t    _xpkCompressBound(int level, uint64_t srcSize);
int         _xpkCompress(int level, const void* src, uint64_t srcSize, 
                          void* dst, uint64_t dstCapacity, xpkCompResult* result);
int         _xpkDecompress(int level, const void* src, uint64_t srcSize,
                            void* dst, uint64_t dstSize, uint32_t expectedHash);

// ============================================================================
// 内部函数声明（Core 模块）
// ============================================================================
int         _xpkLoadMeta(xpkObject xpk);
int         _xpkSaveMeta(xpkObject xpk);
int         _xpkEnsureFilesCapacity(xpkObject xpk, uint32_t needed);

#endif /* XPACK_BASE_H */
