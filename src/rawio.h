/*
 * xPack Ver7 - 分卷虚拟 I/O 模块
 *
 * 功能：
 *   - 透明分卷读写
 *   - 逻辑地址到物理分卷映射
 */

#ifndef XPACK_RAWIO_H
#define XPACK_RAWIO_H

#include "../xpack.h"
#include <stdio.h>

/* clang-format off */

// ============================================================================
// xpkObject 内部结构
// ============================================================================
struct xpkStruct {
    // === 原始 I/O 层 ===
    char*           basePath;       // 基础路径 (不含扩展名)
    char**          volPaths;       // 分卷文件路径列表
    FILE**          volFiles;       // 分卷文件句柄列表
    int             volCount;       // 分卷数量
    uint32_t        volSize;        // 分卷大小 (0=不分卷)
    int             readonly;       // 只读模式
    int             dirty;          // 未保存标记
    
    // === 包头 ===
    xpkHead         head;           // 包头缓存
    
    // === Meta 数据 ===
    xpkFileInfo*    files;          // 文件信息数组 (内存中)
    uint32_t        filesCapacity;  // 数组容量
    
    // === LDB 数据 ===
    void*           ldb;            // LDB 缓冲
    uint32_t        ldbSize;        // LDB 原始大小
    
    // === 运行时状态 ===
    uint64_t        writePos;       // 当前写入位置 (= head.dataOffset)
};

// ============================================================================
// 分卷路径计算
// ============================================================================
static inline uint64_t _xpkVolRawSize(xpkObject xpk)
{
    if (xpk->volSize == 0) {
        return 0;
    }
    return (uint64_t)xpk->volSize;
}

static inline int _xpkCalcVolumeIndex(xpkObject xpk, uint64_t offset)
{
    if (xpk->volSize == 0) {
        return 0;
    }
    return (int)(offset / xpk->volSize);
}

static inline uint64_t _xpkCalcVolumeOffset(xpkObject xpk, uint64_t offset)
{
    if (xpk->volSize == 0) {
        return offset;
    }
    return offset % xpk->volSize;
}

static inline uint64_t _xpkCalcVolumeRemain(xpkObject xpk, uint64_t offset)
{
    if (xpk->volSize == 0) {
        return (uint64_t)-1;
    }
    uint64_t volOff = offset % xpk->volSize;
    return xpk->volSize - volOff;
}

// ============================================================================
// 分卷虚拟 I/O 函数
// ============================================================================
int _xpkRawRead(xpkObject xpk, uint64_t offset, uint64_t size, void* data);
int _xpkRawWrite(xpkObject xpk, uint64_t offset, uint64_t size, const void* data);

#endif /* XPACK_RAWIO_H */
