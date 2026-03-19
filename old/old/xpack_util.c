/*
 * xPack Ver7 - 工具函数
 * 
 * 包含：遍历、批量操作、工具函数
 */

#include "xpack_internal.h"
#include <string.h>
#include <stdlib.h>

// 外部声明
extern uint32_t xpkCompressBound(int level, uint32_t srcSize);

// ============================================================================
// 内存管理
// ============================================================================

XPKAPI void xpkFree(void* ptr) {
    if (ptr) free(ptr);
}

// ============================================================================
// 哈希计算
// ============================================================================

XPKAPI uint32_t xpkHash(const void* data, uint32_t size) {
    if (!data || size == 0) return 0;
    return xrtHash32((ptr)data, size);
}

// ============================================================================
// 校验功能
// ============================================================================

XPKAPI int xpkVerify(xpkObject xpk, uint32_t pos) {
    if (!xpk || pos >= xpk->ldb.Count) return -1;
    
    // 提取数据
    uint32_t outSize = 0;
    void* data = xpkExtractData(xpk, pos, &outSize);
    if (!data) return -1;
    
    // 计算哈希
    uint32_t hash = xrtHash32(data, outSize);
    free(data);
    
    // 比较哈希
    xpkFileInfo* info = (xpkFileInfo*)xpkInfo(xpk, pos);
    if (!info) return -1;
    
    return (hash == info->fileHash) ? 0 : -1;
}

XPKAPI int xpkVerifyAll(xpkObject xpk) {
    if (!xpk) return -1;
    
    for (uint32_t i = 0; i < xpk->ldb.Count; i++) {
        if (xpkVerify(xpk, i) != 0) {
            return (int)i;  // 返回失败的位置
        }
    }
    
    return 0;  // 全部通过
}

// ============================================================================
// 统计信息
// ============================================================================

XPKAPI int xpkStatGet(xpkObject xpk, xpkStat* stat) {
    if (!xpk || !stat) return -1;
    
    memset(stat, 0, sizeof(xpkStat));
    stat->fileCount = xpk->ldb.Count;
    
    for (uint32_t i = 0; i < xpk->ldb.Count; i++) {
        xpkFileInfo* info = (xpkFileInfo*)XPK_LDB_GET(xpk, i);
        if (info) {
            stat->totalSize += info->fileSize;
            stat->packedSize += info->dataSize;
        }
    }
    
    if (stat->totalSize > 0) {
        stat->ratio = (double)stat->totalSize / (double)stat->packedSize;
    } else {
        stat->ratio = 1.0;
    }
    
    return 0;
}

// ============================================================================
// 遍历功能
// ============================================================================

XPKAPI int xpkEach(xpkObject xpk, xpkEachCallback callback, void* userData) {
    if (!xpk || !callback) return -1;
    
    for (uint32_t i = 0; i < xpk->ldb.Count; i++) {
        void* info = XPK_LDB_GET(xpk, i);
        int result = callback(xpk, i, info, userData);
        if (result != 0) {
            return result;  // 回调返回非0，停止遍历
        }
    }
    
    return 0;
}

// 简单的通配符匹配
static int simpleMatch(const char* pattern, const char* str) {
    if (!pattern || !str) return 0;
    
    while (*pattern && *str) {
        if (*pattern == '*') {
            pattern++;
            if (*pattern == '\0') return 1;  // * 在末尾，匹配所有
            while (*str) {
                if (simpleMatch(pattern, str)) return 1;
                str++;
            }
            return 0;
        } else if (*pattern == '?' || *pattern == *str) {
            pattern++;
            str++;
        } else {
            return 0;
        }
    }
    
    while (*pattern == '*') pattern++;
    return (*pattern == '\0' && *str == '\0');
}

XPKAPI int xpkEachMatch(xpkObject xpk, const char* pattern, 
                         xpkEachCallback callback, void* userData) {
    if (!xpk || !callback) return -1;
    
    int packType = xpkType(xpk);
    
    for (uint32_t i = 0; i < xpk->ldb.Count; i++) {
        void* info = XPK_LDB_GET(xpk, i);
        
        // 获取路径进行匹配
        const char* path = NULL;
        if (packType == XPK_TYPE_LINUX) {
            xpkFileInfoLinux* linuxInfo = (xpkFileInfoLinux*)info;
            path = linuxInfo->filePath;
        } else if (packType == XPK_TYPE_WIN32) {
            xpkFileInfoWin32* win32Info = (xpkFileInfoWin32*)info;
            path = win32Info->filePath;
        }
        
        // 如果有路径且匹配模式
        if (!pattern || (path && simpleMatch(pattern, path))) {
            int result = callback(xpk, i, info, userData);
            if (result != 0) {
                return result;
            }
        }
    }
    
    return 0;
}

// ============================================================================
// 批量操作
// ============================================================================

// 提取回调上下文
typedef struct {
    const char* dir;
    int success;
    int failed;
} ExtractContext;

static int extractCallback(void* xpk, uint32_t pos, void* info, void* userData) {
    ExtractContext* ctx = (ExtractContext*)userData;
    xpkObject pack = (xpkObject)xpk;
    
    int packType = xpkType(pack);
    const char* filePath = NULL;
    
    if (packType == XPK_TYPE_LINUX) {
        filePath = ((xpkFileInfoLinux*)info)->filePath;
    } else if (packType == XPK_TYPE_WIN32) {
        filePath = ((xpkFileInfoWin32*)info)->filePath;
    }
    
    if (filePath) {
        // 构建目标路径
        char dstPath[512];
        snprintf(dstPath, sizeof(dstPath), "%s/%s", ctx->dir, filePath);
        
        // 确保目录存在
        char dirPath[512];
        strncpy(dirPath, dstPath, sizeof(dirPath) - 1);
        char* lastSlash = strrchr(dirPath, '/');
        if (!lastSlash) lastSlash = strrchr(dirPath, '\\');
        if (lastSlash) {
            *lastSlash = '\0';
            xrtDirCreateAll((str)dirPath);
        }
        
        // 提取文件
        if (xpkExtractFile(pack, pos, dstPath) == 0) {
            ctx->success++;
        } else {
            ctx->failed++;
        }
    } else {
        // Core/Index 模式：按位置命名
        char dstPath[512];
        snprintf(dstPath, sizeof(dstPath), "%s/%u.dat", ctx->dir, pos);
        
        if (xpkExtractFile(pack, pos, dstPath) == 0) {
            ctx->success++;
        } else {
            ctx->failed++;
        }
    }
    
    return 0;  // 继续遍历
}

XPKAPI int xpkExtractAll(xpkObject xpk, const char* dir) {
    if (!xpk || !dir) return -1;
    
    // 创建目标目录
    xrtDirCreateAll((str)dir);
    
    ExtractContext ctx = { dir, 0, 0 };
    xpkEach(xpk, extractCallback, &ctx);
    
    return ctx.failed > 0 ? -1 : 0;
}

// 目录扫描回调上下文
typedef struct {
    xpkObject xpk;
    const char* baseDir;
    size_t baseDirLen;
    const char* pattern;
    int level;
    int success;
    int failed;
} AppendDirContext;

static int appendDirCallback(str sPath, size_t iSize, int bDir, ptr pData, ptr Param) {
    AppendDirContext* ctx = (AppendDirContext*)Param;
    
    if (bDir == 0) {  // 是文件
        // 计算相对路径
        const char* relativePath = sPath;
        if (strncmp(sPath, ctx->baseDir, ctx->baseDirLen) == 0) {
            relativePath = sPath + ctx->baseDirLen;
            if (*relativePath == '/' || *relativePath == '\\') {
                relativePath++;
            }
        }
        
        // 检查模式匹配
        if (ctx->pattern && !simpleMatch(ctx->pattern, relativePath)) {
            return 0;  // 不匹配，跳过
        }
        
        // 添加文件
        void* info = xpkPathAppendFile(ctx->xpk, relativePath, sPath, ctx->level);
        if (info) {
            ctx->success++;
        } else {
            ctx->failed++;
        }
    }
    
    return 0;  // 继续扫描
}

XPKAPI int xpkAppendDir(xpkObject xpk, const char* dir, const char* pattern, 
                         int level, int recursive) {
    if (!xpk || !dir) return -1;
    if (xpk->readonly) {
        xpkSetError(10, "Cannot append in readonly mode");
        return -1;
    }
    
    int packType = xpkType(xpk);
    if (packType != XPK_TYPE_LINUX && packType != XPK_TYPE_WIN32) {
        xpkSetError(11, "Pack type must be Linux or Win32 for directory append");
        return -1;
    }
    
    AppendDirContext ctx;
    ctx.xpk = xpk;
    ctx.baseDir = dir;
    ctx.baseDirLen = strlen(dir);
    ctx.pattern = pattern;
    ctx.level = level;
    ctx.success = 0;
    ctx.failed = 0;
    
    xrtDirScan((str)dir, recursive, (ptr)appendDirCallback, &ctx);
    
    return ctx.failed > 0 ? -1 : ctx.success;
}

// ============================================================================
// 重建功能
// ============================================================================

XPKAPI int xpkRebuild(xpkObject xpk) {
    if (!xpk) return -1;
    if (xpk->readonly) {
        xpkSetError(10, "Cannot rebuild in readonly mode");
        return -1;
    }
    if (xpk->ldb.Count == 0) return 0;  // 无需重建
    
    // 计算新的数据偏移
    uint32_t newOffset = sizeof(xpkHead) + xpk->head.headExtSize;
    
    // 遍历所有文件，重新排列数据
    for (uint32_t i = 0; i < xpk->ldb.Count; i++) {
        xpkFileInfo* info = (xpkFileInfo*)XPK_LDB_GET(xpk, i);
        if (!info) continue;
        
        // 如果数据位置已经是最优的，跳过
        if (info->dataOffset == newOffset) {
            newOffset += info->dataSize;
            continue;
        }
        
        // 读取原数据
        xrtSeek(xpk->file, xpk->baseOffset + info->dataOffset, XRT_SEEK_SET);
        size_t readSize = 0;
        void* data = xrtGet(xpk->file, info->dataSize, &readSize);
        if (!data || readSize != info->dataSize) {
            xrtFree(data);
            xpkSetError(2, "Failed to read data during rebuild");
            return -1;
        }
        
        // 写入新位置
        xrtSeek(xpk->file, xpk->baseOffset + newOffset, XRT_SEEK_SET);
        if (xrtPut(xpk->file, data, info->dataSize) != (int)info->dataSize) {
            xrtFree(data);
            xpkSetError(2, "Failed to write data during rebuild");
            return -1;
        }
        xrtFree(data);
        
        // 更新偏移
        info->dataOffset = newOffset;
        newOffset += info->dataSize;
    }
    
    xpk->modified = 1;
    return 0;
}
