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
    xpk->file = xrtOpen((str)path, readonly, XRT_CP_BINARY);
    
    if (xpk->file) {
        // 文件存在，尝试读取包头
        xrtSeek(xpk->file, offset, XRT_SEEK_SET);
        size_t readSize = 0;
        void* headData = xrtGet(xpk->file, sizeof(xpkHead), &readSize);
        
        if (headData && readSize == sizeof(xpkHead)) {
            memcpy(&xpk->head, headData, sizeof(xpkHead));
            free(headData);

            // 验证版本（文件头包含 "xpk" + 版本号，与 ver6 兼容）
            if (xpk->head.fileHead != XPK_VERSION) {
                xpkSetError(4, "Invalid version or signature");
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
            
            // 初始化固实相关字段
            xpk->solidMode = xpk->head.flag.solidMode ? 1 : 0;
            xpk->solidCompLevel = XPK_COMP_DEFAULT;
            xpk->solidBufferSize = 0;
            xpk->solidCached = 0;
            xpk->solidDecompressed = NULL;
            xpk->solidBuffer = NULL;
            
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
        xpk->file = xrtOpen((str)path, 0, XRT_CP_BINARY);
        if (!xpk->file) {
            xpkSetError(1, "Failed to create file");
            free(xpk);
            return NULL;
        }
        
init_new_pack:
        // 初始化新包头
        memset(&xpk->head, 0, sizeof(xpkHead));
        xpk->head.fileHead = XPK_VERSION;
        xpk->head.flag.packType = XPK_TYPE_CORE;
        xpk->head.flag.ldbComp = XPK_LDB_COMP;
        xpk->head.ldbOffset = sizeof(xpkHead);
        xpk->head.createTime = xrtNow();
        xpk->head.modifyTime = xpk->head.createTime;

        // 初始化 LDB 数组（默认 Core 模式）
        xrtArrayInit(&xpk->ldb, xpkInfoSizes[XPK_TYPE_CORE]);

        // 初始化固实相关字段
        xpk->solidMode = xpk->head.flag.solidMode ? 1 : 0;
        xpk->solidCompLevel = XPK_COMP_DEFAULT;
        xpk->solidBufferSize = 0;
        xpk->solidCached = 0;
        xpk->solidDecompressed = NULL;
        xpk->solidBuffer = NULL;

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
	if (!xpk->modified) return 0;
	
	// 更新修改时间
	xpk->head.modifyTime = xrtNow();
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
	
	// 固实模式：保存固实块
	if (xpk->head.flag.solidMode) {
		if (xpkSolidSave(xpk) != 0) {
			return -1;
		}
	} else {
		// 独立模式：保存 LDB
		if (xpkLdbSave(xpk) != 0) {
			return -1;
		}
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
	
	// 释放固实缓冲区（创建时）
	xrtBufferDestroy(xpk->solidBuffer);
	xpk->solidBuffer = NULL;
	
	// 释放固实块缓存（读取时）
	if (xpk->solidDecompressed) {
		free(xpk->solidDecompressed);
		xpk->solidDecompressed = NULL;
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
// 固实压缩控制接口
// ============================================================================

XPKAPI int xpkSolidMode(xpkObject xpk) {
	if (!xpk) return -1;
	return xpk->head.flag.solidMode ? 1 : 0;
}

XPKAPI int xpkSolidModeSet(xpkObject xpk, int enabled) {
	if (!xpk) return -1;
	
	// 只允许在空包时设置
	if (xpk->ldb.Count > 0) {
		xpkSetError(11, "Cannot set solid mode on non-empty pack");
		return -1;
	}
	
	// 只允许在写入模式设置
	if (xpk->readonly) {
		xpkSetError(10, "Cannot set solid mode in readonly");
		return -1;
	}
	
	// 设置固实模式
	xpk->head.flag.solidMode = enabled ? 1 : 0;
	xpk->solidMode = enabled ? 1 : 0;
	
	if (enabled) {
		// 初始化固实缓冲区
		xpk->solidBuffer = xrtBufferCreate(1024 * 1024);
		xpk->solidBufferSize = 0;
		xpk->solidCompLevel = XPK_COMP_DEFAULT;
		xpk->solidCached = 0;
		xpk->solidDecompressed = NULL;
	} else {
		// 清理固实缓冲区
		if (xpk->solidBuffer) {
			xrtBufferDestroy(xpk->solidBuffer);
			xpk->solidBuffer = NULL;
		}
		xpk->solidBufferSize = 0;
	}
	
	xpk->modified = 1;
	return 0;
}

XPKAPI int xpkSolidBlockInfo(xpkObject xpk, uint32_t* offset, uint32_t* size) {
	if (!xpk) return -1;
	if (!xpk->head.flag.solidMode) return -1;
	
	uint32_t dataOffset = sizeof(xpkHead) + xpk->head.headExtSize;
	uint32_t solidSize = xpk->head.ldbOffset - dataOffset;
	
	if (offset) *offset = dataOffset;
	if (size) *size = solidSize;
	
	return 0;
}

// ============================================================================
// 固实压缩内部函数
// ============================================================================

int xpkSolidAppendData(xpkObject xpk, const void* data, uint32_t size, int level) {
	// 处理空数据
	if (!data || size == 0) {
		data = "";
		size = 0;
	}
	
	// 计算文件哈希
	uint32_t fileHash = xrtHash32((ptr)data, size);
	
	// 计算在固实块中的偏移
	uint32_t offsetInBlock = xpk->solidBufferSize;
	
	// 添加到固实缓冲区（跳过空数据）
	if (size > 0) {
		if (!xrtBufferAppend(xpk->solidBuffer, (ptr)data, size, XBUF_BINARY)) {
			xpkSetError(3, "Failed to append to solid buffer");
			return UINT32_MAX;
		}
	}
	xpk->solidBufferSize += size;
	
	// 保存压缩级别（使用第一个文件的级别）
	if (xpk->ldb.Count == 0) {
		xpk->solidCompLevel = level;
	}
	
	// 追加文件信息
	uint32_t pos1 = xrtArrayAppend(&xpk->ldb, 1);
	if (pos1 == 0) {
		xpkSetError(3, "Failed to allocate file info");
		return UINT32_MAX;
	}
	
	xpkFileInfo* info = (xpkFileInfo*)xrtArrayGet(&xpk->ldb, pos1);
	if (!info) {
		xpkSetError(3, "Failed to allocate file info");
		return UINT32_MAX;
	}
	
	// 填充文件信息
	info->dataOffset = 0;
	info->dataSize = size;
	info->fileSize = size;
	info->fileHash = fileHash;
	info->flag.value = 0;
	info->flag.compLevel = level;
	info->flag.fileType = XPK_FTYPE_UNKNOWN;
	
	xpk->modified = 1;
	return pos1 - 1;
}

int xpkSolidSave(xpkObject xpk) {
	if (!xpk) return -1;
	if (!xpk->head.flag.solidMode) return 0;
	
	// 压缩固实缓冲区
	uint32_t compBound = xpkCompressBound(xpk->solidCompLevel, xpk->solidBufferSize);
	void* compData = malloc(compBound);
	if (!compData) {
		xpkSetError(3, "Failed to allocate compression buffer");
		return -1;
	}
	
	uint32_t compSize = 0;
	if (xpkCompressRouter(xpk->solidCompLevel, xpk->solidBuffer->Buffer, xpk->solidBufferSize,
	                      compData, compBound, &compSize) != 0) {
		free(compData);
		xpkSetError(7, "Solid compression failed");
		return -1;
	}
	
	// 计算固实块偏移
	uint32_t dataOffset = sizeof(xpkHead) + xpk->head.headExtSize;
	
	// 写入固实块
	xrtSeek(xpk->file, xpk->baseOffset + dataOffset, XRT_SEEK_SET);
	if (xrtPut(xpk->file, compData, compSize) != (int)compSize) {
		free(compData);
		xpkSetError(2, "Failed to write solid block");
		return -1;
	}
	free(compData);
	
	// 更新 LDB 偏移
	xpk->head.ldbOffset = dataOffset + compSize;
	
	// 保存 LDB（包含固实偏移信息）
	if (xpkLdbSave(xpk) != 0) {
		return -1;
	}
	
	// 清理固实缓冲区
	xrtBufferUnit(xpk->solidBuffer);
	xpk->solidBufferSize = 0;
	
	return 0;
}

void* xpkSolidExtractData(xpkObject xpk, uint32_t pos, xpkFileInfo* info, uint32_t* outSize) {
	// 检查缓存
	if (!xpk->solidCached) {
		if (xpkSolidDecompressBlock(xpk) != 0) {
			return NULL;
		}
	}
	
	// 获取固实偏移信息
	uint32_t offsetInBlock = xpkGetSolidOffset(xpk, pos);
	
	// 检查偏移和大小有效性
	if (offsetInBlock + info->fileSize > xpk->solidDecompSize) {
		xpkSetError(9, "Solid data offset out of range");
		return NULL;
	}
	
	// 分配文件数据缓冲区
	void* fileData = malloc(info->fileSize);
	if (!fileData) {
		xpkSetError(3, "Failed to allocate file buffer");
		return NULL;
	}
	
	// 从固实块中复制文件数据
	memcpy(fileData, (uint8_t*)xpk->solidDecompressed + offsetInBlock, info->fileSize);
	
	if (outSize) *outSize = info->fileSize;
	return fileData;
}

int xpkSolidDecompressBlock(xpkObject xpk) {
	// 计算固实块信息
	uint32_t dataOffset = sizeof(xpkHead) + xpk->head.headExtSize;
	uint32_t solidSize = xpk->head.ldbOffset - dataOffset;
	
	// 读取压缩数据
	xrtSeek(xpk->file, xpk->baseOffset + dataOffset, XRT_SEEK_SET);
	size_t readSize = 0;
	void* compData = xrtGet(xpk->file, solidSize, &readSize);
	
	if (!compData || readSize != solidSize) {
		if (compData) free(compData);
		xpkSetError(2, "Failed to read solid block");
		return -1;
	}
	
	// 计算原始大小（通过累加文件大小）
	uint32_t totalRawSize = 0;
	for (uint32_t i = 0; i < xpk->ldb.Count; i++) {
		xpkFileInfo* info = (xpkFileInfo*)XPK_LDB_GET(xpk, i);
		totalRawSize += info->fileSize;
	}
	
	// 分配解压缓冲区
	void* rawData = malloc(totalRawSize);
	if (!rawData) {
		free(compData);
		xpkSetError(3, "Failed to allocate solid buffer");
		return -1;
	}
	
	// 解压固实块
	int solidLevel = xpk->head.flag.ldbComp;
	if (xpkDecompressRouter(solidLevel, compData, solidSize,
	                     rawData, totalRawSize) != 0) {
		free(compData);
		free(rawData);
		xpkSetError(8, "Solid decompression failed");
		return -1;
	}
	free(compData);
	
	// 保存到缓存
	xpk->solidDecompressed = rawData;
	xpk->solidDecompSize = totalRawSize;
	xpk->solidCached = 1;
	
	return 0;
}

uint32_t xpkGetSolidOffset(xpkObject xpk, uint32_t pos) {
	// 计算固实偏移（累加前面所有文件的大小）
	uint32_t offset = 0;
	for (uint32_t i = 0; i < pos; i++) {
		xpkFileInfo* info = (xpkFileInfo*)XPK_LDB_GET(xpk, i);
		offset += info->fileSize;
	}
	return offset;
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
