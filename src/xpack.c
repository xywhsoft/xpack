/*
 * xPack Ver7 - 主实现文件
 * 
 * 包含：生命周期管理、包属性操作
 */

#include "xpack_internal.h"
#include <string.h>
#include <stdlib.h>

// ============================================================================
// 文件信息大小表
// ============================================================================
static const uint32_t xpkInfoSizes[4] = {
    sizeof(xpkFileInfo),        // Core:  20 bytes
    sizeof(xpkFileInfoIndex),   // Index: 28 bytes
    sizeof(xpkFileInfoLinux),   // Linux: 232 bytes
    sizeof(xpkFileInfoWin32)    // Win32: 236 bytes
};

// ============================================================================
// 错误状态（线程局部）
// ============================================================================
static int g_lastError = 0;
static char g_lastErrorMsg[256] = {0};

static const char* g_errorMessages[] = {
    "Success",                      // 0
    "File open failed",             // 1
    "File read failed",             // 2
    "Memory allocation failed",     // 3
    "Invalid pack format",          // 4
    "Version not supported",        // 5
    "Invalid file position",        // 6
    "Compression failed",           // 7
    "Decompression failed",         // 8
    "Hash verification failed",     // 9
    "Readonly mode write denied",   // 10
    "Pack type mismatch"            // 11
};

// ============================================================================
// 生命周期管理
// ============================================================================

XPKAPI xpkObject xpkOpen(const char* path, uint32_t offset, int readonly) {
    if (!path) {
        xpkSetError(1, "Path is NULL");
        return NULL;
    }
    
    // 分配对象
    xpkStruct* xpk = (xpkStruct*)malloc(sizeof(xpkStruct));
    if (!xpk) {
        xpkSetError(3, "Failed to allocate xpkStruct");
        return NULL;
    }
    memset(xpk, 0, sizeof(xpkStruct));
    
    xpk->baseOffset = offset;
    xpk->readonly = readonly ? 1 : 0;
    xpk->modified = 0;
    
    // 打开文件
    xpk->file = xrtOpen(path, readonly, XRT_CP_BINARY);
    
    if (xpk->file) {
        // 文件存在，尝试读取包头
        xrtSeek(xpk->file, offset, XRT_SEEK_SET);
        size_t readSize = 0;
        void* headData = xrtGet(xpk->file, sizeof(xpkHead), &readSize);
        
        if (headData && readSize == sizeof(xpkHead)) {
            memcpy(&xpk->head, headData, sizeof(xpkHead));
            free(headData);
            
            // 验证签名和版本
            if (xpk->head.signature != XPK_SIGNATURE) {
                xpkSetError(4, "Invalid signature");
                xrtClose(xpk->file);
                free(xpk);
                return NULL;
            }
            if (xpk->head.version != XPK_VERSION) {
                xpkSetError(5, "Version not supported");
                xrtClose(xpk->file);
                free(xpk);
                return NULL;
            }
            
            // 读取包头扩展数据
            if (xpk->head.headExtSize > 0) {
                xpk->headExt = xrtGet(xpk->file, xpk->head.headExtSize, &readSize);
                if (!xpk->headExt || readSize != xpk->head.headExtSize) {
                    xpkSetError(2, "Failed to read head extension");
                    xrtClose(xpk->file);
                    free(xpk);
                    return NULL;
                }
            }
            
            // 初始化 LDB 数组
            int packType = xpk->head.flag.packType & 0x03;
            xrtArrayInit(&xpk->ldb, xpkInfoSizes[packType]);
            
            // 加载 LDB
            if (xpk->head.fileCount > 0) {
                if (xpkLdbLoad(xpk) != 0) {
                    xrtArrayUnit(&xpk->ldb);
                    if (xpk->headExt) free(xpk->headExt);
                    xrtClose(xpk->file);
                    free(xpk);
                    return NULL;
                }
            }
        } else {
            // 文件为空或读取失败，初始化新包
            if (headData) free(headData);
            goto init_new_pack;
        }
    } else {
        // 文件不存在
        if (readonly) {
            xpkSetError(1, "File not found in readonly mode");
            free(xpk);
            return NULL;
        }
        
        // 创建新文件
        xpk->file = xrtOpen(path, 0, XRT_CP_BINARY);
        if (!xpk->file) {
            xpkSetError(1, "Failed to create file");
            free(xpk);
            return NULL;
        }
        
init_new_pack:
        // 初始化新包头
        memset(&xpk->head, 0, sizeof(xpkHead));
        xpk->head.signature = XPK_SIGNATURE;
        xpk->head.version = XPK_VERSION;
        xpk->head.flag.packType = XPK_TYPE_CORE;
        xpk->head.flag.ldbComp = XPK_LDB_COMP;
        xpk->head.createTime = (uint32_t)xrtToUnixTime(xrtTimeNow());
        xpk->head.modifyTime = xpk->head.createTime;
        
        // 初始化 LDB 数组（默认 Core 模式）
        xrtArrayInit(&xpk->ldb, xpkInfoSizes[XPK_TYPE_CORE]);
        
        xpk->modified = 1;
    }
    
    g_lastError = 0;
    return xpk;
}

XPKAPI int xpkSave(xpkObject xpk) {
    if (!xpk) return -1;
    if (xpk->readonly) {
        xpkSetError(10, "Cannot save in readonly mode");
        return -1;
    }
    if (!xpk->modified) return 0;  // 无修改
    
    // 更新修改时间
    xpk->head.modifyTime = (uint32_t)xrtToUnixTime(xrtTimeNow());
    xpk->head.fileCount = xpk->ldb.Count;
    
    // 移动到文件开始位置
    xrtSeek(xpk->file, xpk->baseOffset, XRT_SEEK_SET);
    
    // 计算数据区偏移
    uint32_t dataOffset = sizeof(xpkHead) + xpk->head.headExtSize;
    
    // 写入包头（暂时）
    xrtPut(xpk->file, &xpk->head, sizeof(xpkHead));
    
    // 写入包头扩展数据
    if (xpk->head.headExtSize > 0 && xpk->headExt) {
        xrtPut(xpk->file, xpk->headExt, xpk->head.headExtSize);
    }
    
    // 保存 LDB
    if (xpkLdbSave(xpk) != 0) {
        return -1;
    }
    
    // 重新写入包头（更新 LDB 信息）
    xrtSeek(xpk->file, xpk->baseOffset, XRT_SEEK_SET);
    xrtPut(xpk->file, &xpk->head, sizeof(xpkHead));
    
    xpk->modified = 0;
    return 0;
}

XPKAPI void xpkClose(xpkObject xpk) {
    if (!xpk) return;
    
    // 释放 LDB
    xrtArrayUnit(&xpk->ldb);
    
    // 释放包头扩展数据
    if (xpk->headExt) {
        free(xpk->headExt);
        xpk->headExt = NULL;
    }
    
    // 关闭文件
    if (xpk->file) {
        xrtClose(xpk->file);
        xpk->file = NULL;
    }
    
    free(xpk);
}

// ============================================================================
// 包属性操作
// ============================================================================

XPKAPI int xpkType(xpkObject xpk) {
    if (!xpk) return -1;
    return xpk->head.flag.packType & 0x03;
}

XPKAPI int xpkTypeSet(xpkObject xpk, int type) {
    if (!xpk) return -1;
    if (xpk->readonly) {
        xpkSetError(10, "Cannot modify in readonly mode");
        return -1;
    }
    if (xpk->ldb.Count > 0) {
        xpkSetError(11, "Cannot change type after adding files");
        return -1;
    }
    if (type < 0 || type > 3) return -1;
    
    // 更新类型
    xpk->head.flag.packType = type;
    
    // 重新初始化 LDB 数组（调整元素大小）
    xrtArrayUnit(&xpk->ldb);
    xrtArrayInit(&xpk->ldb, xpkInfoSizes[type]);
    
    xpk->modified = 1;
    return 0;
}

XPKAPI uint32_t xpkCount(xpkObject xpk) {
    if (!xpk) return 0;
    return xpk->ldb.Count;
}

XPKAPI uint32_t xpkDiscCode(xpkObject xpk) {
    if (!xpk) return 0;
    return xpk->head.discCode;
}

XPKAPI int xpkDiscCodeSet(xpkObject xpk, uint32_t code) {
    if (!xpk) return -1;
    if (xpk->readonly) {
        xpkSetError(10, "Cannot modify in readonly mode");
        return -1;
    }
    xpk->head.discCode = code;
    xpk->modified = 1;
    return 0;
}

XPKAPI void xpkOnError(xpkObject xpk, xpkErrorProc callback) {
    if (!xpk) return;
    xpk->onError = callback;
}

XPKAPI xpkHead* xpkGetHead(xpkObject xpk) {
    if (!xpk) return NULL;
    return &xpk->head;
}

// ============================================================================
// 错误处理
// ============================================================================

void xpkSetError(int code, const char* msg) {
    g_lastError = code;
    if (msg) {
        strncpy(g_lastErrorMsg, msg, sizeof(g_lastErrorMsg) - 1);
        g_lastErrorMsg[sizeof(g_lastErrorMsg) - 1] = '\0';
    } else if (code >= 0 && code < (int)(sizeof(g_errorMessages)/sizeof(g_errorMessages[0]))) {
        strncpy(g_lastErrorMsg, g_errorMessages[code], sizeof(g_lastErrorMsg) - 1);
    } else {
        g_lastErrorMsg[0] = '\0';
    }
}

XPKAPI int xpkLastError(void) {
    return g_lastError;
}

XPKAPI const char* xpkLastErrorMsg(void) {
    return g_lastErrorMsg;
}
