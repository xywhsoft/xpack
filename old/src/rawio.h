/*
 * xPack Ver7 - 分卷虚拟 I/O 模块
 *
 * 功能：
 *   - 透明分卷读写
 *   - 逻辑地址到物理分卷映射
 *   - 脏数据管理
 *   - 缓冲写入模式
 */

#ifndef XPACK_RAWIO_H
#define XPACK_RAWIO_H

#include "../xpack.h"
#include <stdio.h>

/* clang-format off */

// ============================================================================
// 脏数据标志
// ============================================================================
typedef enum {
    XPK_DIRTY_NONE   = 0,
    XPK_DIRTY_HEAD   = 1 << 0,   // 包头已修改
    XPK_DIRTY_META   = 1 << 1,   // Meta (文件列表) 已修改
    XPK_DIRTY_LDB    = 1 << 2,   // LDB (用户 Meta) 已修改
    XPK_DIRTY_DATA   = 1 << 3,   // 文件数据已修改 (缓冲模式)
} XpkDirtyFlags;

// ============================================================================
// 待写入数据块
// ============================================================================
typedef struct xpkWriteBlock {
    uint64_t            offset;     // 写入偏移
    uint64_t            size;       // 数据大小
    uint8_t*            data;       // 数据内容
    struct xpkWriteBlock* next;     // 下一个块
} xpkWriteBlock;

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
    int             dirtyFlags;     // 脏数据标志 (XpkDirtyFlags)
    
    // === 缓冲写入模式 ===
    int             buffered;       // 缓冲模式: 1=不立即写入磁盘, 0=立即写入
    xpkWriteBlock*  writeBlocks;    // 待写入数据块链表
    int             writeBlockCount;// 待写入块数量
    
    // === 包头 ===
    xpkHead         head;           // 包头缓存
    
    // === Meta 数据 ===
    xpkFileInfo*        files;          // Core 模式文件信息数组
    xpkFileInfoIndex*   indexFiles;     // Index 模式文件信息数组
    xpkFileInfoLinux*   pathFiles;      // Path 模式文件信息数组 (Linux/Win32)
    uint32_t            filesCapacity;  // 数组容量
    
    // === LDB 数据 (用户 Meta) ===
    void*           ldb;            // LDB 缓冲 (用户自定义 Meta)
    uint32_t        ldbSize;        // LDB 原始大小
    int             ldbLevel;       // LDB 压缩级别
    
    // === 已保存的状态 (用于判断是否需要重写 LDB) ===
    uint32_t        savedMetaCompSize;  // 已保存的 Meta 压缩大小
    
    // === 运行时状态 ===
    uint64_t        writePos;       // 当前写入位置 (= head.dataOffset)
};

// ============================================================================
// 内联辅助函数
// ============================================================================
static inline int _xpkIsDirty(xpkObject xpk, int flag) {
    return (xpk->dirtyFlags & flag) != 0;
}

static inline void _xpkSetDirty(xpkObject xpk, int flag) {
    xpk->dirtyFlags |= flag;
}

static inline void _xpkClearDirty(xpkObject xpk, int flag) {
    xpk->dirtyFlags &= ~flag;
}

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
