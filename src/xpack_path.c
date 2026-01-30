/*
 * xPack Ver7 - 路径模式操作
 * 
 * 包含：Linux/Win32 路径模式的添加、查找、提取、更新、删除
 */

#include "xpack_internal.h"
#include <string.h>
#include <stdlib.h>

// 外部声明
extern uint32_t xpkCompressBound(int level, uint32_t srcSize);

// ============================================================================
// 路径哈希计算
// ============================================================================

// Linux: 大小写敏感
uint32_t xpkPathHashLinux(const char* path) {
    if (!path) return 0;
    return xrtHash32((ptr)path, strlen(path));
}

// Win32: 转小写后计算
uint32_t xpkPathHashWin32(const char* path) {
    if (!path) return 0;
    
    char lower[XPK_PATH_MAX];
    size_t len = strlen(path);
    if (len >= XPK_PATH_MAX) len = XPK_PATH_MAX - 1;
    
    for (size_t i = 0; i < len; i++) {
        char c = path[i];
        // 转小写，同时将反斜杠转为正斜杠
        if (c >= 'A' && c <= 'Z') {
            lower[i] = c + 32;
        } else if (c == '\\') {
            lower[i] = '/';
        } else {
            lower[i] = c;
        }
    }
    lower[len] = '\0';
    
    return xrtHash32((ptr)lower, len);
}

// ============================================================================
// 查找操作
// ============================================================================

XPKAPI uint32_t xpkPathFind(xpkObject xpk, const char* filePath) {
    if (!xpk || !filePath) return UINT32_MAX;
    
    int packType = xpkType(xpk);
    if (packType != XPK_TYPE_LINUX && packType != XPK_TYPE_WIN32) {
        return UINT32_MAX;
    }
    
    // 计算路径哈希
    uint32_t targetHash;
    if (packType == XPK_TYPE_LINUX) {
        targetHash = xpkPathHashLinux(filePath);
    } else {
        targetHash = xpkPathHashWin32(filePath);
    }
    
    // 遍历查找
    for (uint32_t i = 0; i < xpk->ldb.Count; i++) {
        if (packType == XPK_TYPE_LINUX) {
            xpkFileInfoLinux* info = (xpkFileInfoLinux*)XPK_LDB_GET(xpk, i);
            if (info && info->pathHash == targetHash) {
                // 哈希匹配，验证路径
                if (strcmp(info->filePath, filePath) == 0) {
                    return i;
                }
            }
        } else {
            xpkFileInfoWin32* info = (xpkFileInfoWin32*)XPK_LDB_GET(xpk, i);
            if (info && info->pathHash == targetHash) {
                // 哈希匹配，验证路径（不区分大小写）
                if (stricmp(info->filePath, filePath) == 0) {
                    return i;
                }
            }
        }
    }
    
    return UINT32_MAX;
}

XPKAPI int xpkPathExists(xpkObject xpk, const char* filePath) {
    return xpkPathFind(xpk, filePath) != UINT32_MAX ? 1 : 0;
}

// ============================================================================
// 添加操作
// ============================================================================

XPKAPI void* xpkPathAppendFile(xpkObject xpk, const char* filePath,
                                const char* srcPath, int level) {
	if (!xpk || !filePath || !srcPath) return NULL;
	if (xpk->readonly) {
		xpkSetError(10, "Cannot append in readonly mode");
		return NULL;
	}
	
	int packType = xpkType(xpk);
	if (packType != XPK_TYPE_LINUX && packType != XPK_TYPE_WIN32) {
		xpkSetError(11, "Pack type is not Linux or Win32");
		return NULL;
	}
	
	// 固实包禁止追加
	if (xpk->head.flag.solidMode) {
		xpkSetError(11, "Cannot append to solid archive pack");
		return NULL;
	}
	
	// 读取文件内容
	size_t fileSize = 0;
	void* fileData = xrtFileGetAll((str)srcPath, &fileSize);
	if (!fileData) {
		xpkSetError(2, "Failed to read source file");
		return NULL;
	}
	
	void* info = xpkPathAppendData(xpk, filePath, fileData, (uint32_t)fileSize, level);
	free(fileData);
	
	return info;
}

XPKAPI void* xpkPathAppendData(xpkObject xpk, const char* filePath,
                                const void* data, uint32_t size, int level) {
	if (!xpk || !filePath) return NULL;
	if (xpk->readonly) {
		xpkSetError(10, "Cannot append in readonly mode");
		return NULL;
	}
	
	int packType = xpkType(xpk);
	if (packType != XPK_TYPE_LINUX && packType != XPK_TYPE_WIN32) {
		xpkSetError(11, "Pack type is not Linux or Win32");
		return NULL;
	}
	
	// 固实包禁止追加
	if (xpk->head.flag.solidMode) {
		xpkSetError(11, "Cannot append to solid archive pack");
		return NULL;
	}
	
	// 检查路径是否已存在
	if (xpkPathFind(xpk, filePath) != UINT32_MAX) {
		xpkSetError(11, "Path already exists");
		return NULL;
	}
	
	// 检查路径长度
	size_t pathLen = strlen(filePath);
	if (pathLen >= XPK_PATH_MAX) {
		xpkSetError(11, "Path too long");
		return NULL;
	}
	
	// 处理空数据
	if (!data || size == 0) {
		data = "";
		size = 0;
	}
	
	// 限制压缩级别
	level = level & 0x0F;
	
	// 计算压缩缓冲区大小
	uint32_t compBound = xpkCompressBound(level, size);
	void* compData = malloc(compBound);
	if (!compData) {
		xpkSetError(3, "Failed to allocate compression buffer");
		return NULL;
	}
	
	// 压缩数据
	uint32_t compSize = 0;
	if (xpkCompressRouter(level, data, size, compData, compBound, &compSize) != 0) {
		free(compData);
		xpkSetError(7, "Compression failed");
		return NULL;
	}
	
	// 计算文件哈希
	uint32_t fileHash = xrtHash32((ptr)data, size);
	
	// 计算数据偏移
	uint32_t dataOffset = sizeof(xpkHead) + xpk->head.headExtSize;
	if (xpk->ldb.Count > 0) {
		void* lastInfo = XPK_LDB_GET(xpk, xpk->ldb.Count - 1);
		if (lastInfo) {
			xpkFileInfo* base = (xpkFileInfo*)lastInfo;
			dataOffset = base->dataOffset + base->dataSize;
		}
	}
	
	// 写入压缩数据
	xrtSeek(xpk->file, xpk->baseOffset + dataOffset, XRT_SEEK_SET);
	if (xrtPut(xpk->file, compData, compSize) != (int)compSize) {
		free(compData);
		xpkSetError(2, "Failed to write data");
		return NULL;
	}
	free(compData);
	
	// 追加文件信息
	uint32_t pos1 = xrtArrayAppend(&xpk->ldb, 1);
	void* info = xrtArrayGet(&xpk->ldb, pos1);
	if (!info) {
		xpkSetError(3, "Failed to allocate file info");
		return NULL;
	}
	
	// 获取当前时间
	uint32_t nowTime = (uint32_t)xrtToUnixTime(xrtNow());
	
	if (packType == XPK_TYPE_LINUX) {
		xpkFileInfoLinux* linuxInfo = (xpkFileInfoLinux*)info;
		linuxInfo->dataOffset = dataOffset;
		linuxInfo->dataSize = compSize;
		linuxInfo->fileSize = size;
		linuxInfo->fileHash = fileHash;
		linuxInfo->flag.value = 0;
		linuxInfo->flag.compLevel = level;
		linuxInfo->flag.fileType = XPK_FTYPE_UNKNOWN;
		memset(linuxInfo->filePath, 0, XPK_PATH_MAX);
		strncpy(linuxInfo->filePath, filePath, XPK_PATH_MAX - 1);
		linuxInfo->pathHash = xpkPathHashLinux(filePath);
		linuxInfo->fileAttr = 0;
		linuxInfo->modifyTime = nowTime;
	} else {
		xpkFileInfoWin32* win32Info = (xpkFileInfoWin32*)info;
		win32Info->dataOffset = dataOffset;
		win32Info->dataSize = compSize;
		win32Info->fileSize = size;
		win32Info->fileHash = fileHash;
		win32Info->flag.value = 0;
		win32Info->flag.compLevel = level;
		win32Info->flag.fileType = XPK_FTYPE_UNKNOWN;
		memset(win32Info->filePath, 0, XPK_PATH_MAX);
		strncpy(win32Info->filePath, filePath, XPK_PATH_MAX - 1);
		win32Info->pathHash = xpkPathHashWin32(filePath);
		win32Info->fileAttr = 0;
		win32Info->createTime = nowTime;
		win32Info->modifyTime = nowTime;
	}
	
	xpk->modified = 1;
	return info;
}

// ============================================================================
// 提取操作
// ============================================================================

XPKAPI int xpkPathExtractFile(xpkObject xpk, const char* filePath, const char* dstPath) {
    if (!xpk || !filePath || !dstPath) return -1;
    
    uint32_t pos = xpkPathFind(xpk, filePath);
    if (pos == UINT32_MAX) {
        xpkSetError(6, "Path not found");
        return -1;
    }
    
    return xpkExtractFile(xpk, pos, dstPath);
}

XPKAPI void* xpkPathExtractData(xpkObject xpk, const char* filePath, uint32_t* outSize) {
    if (!xpk || !filePath) return NULL;
    
    uint32_t pos = xpkPathFind(xpk, filePath);
    if (pos == UINT32_MAX) {
        xpkSetError(6, "Path not found");
        return NULL;
    }
    
    return xpkExtractData(xpk, pos, outSize);
}

// ============================================================================
// 更新操作
// ============================================================================

XPKAPI int xpkPathUpdateFile(xpkObject xpk, const char* filePath, 
                              const char* srcPath, int level) {
    if (!xpk || !filePath || !srcPath) return -1;
    
    uint32_t pos = xpkPathFind(xpk, filePath);
    if (pos == UINT32_MAX) {
        xpkSetError(6, "Path not found");
        return -1;
    }
    
    return xpkUpdateFile(xpk, pos, srcPath, level);
}

XPKAPI int xpkPathUpdateData(xpkObject xpk, const char* filePath, 
                              const void* data, uint32_t size, int level) {
    if (!xpk || !filePath) return -1;
    
    uint32_t pos = xpkPathFind(xpk, filePath);
    if (pos == UINT32_MAX) {
        xpkSetError(6, "Path not found");
        return -1;
    }
    
    return xpkUpdateData(xpk, pos, data, size, level);
}

// ============================================================================
// 删除操作
// ============================================================================

XPKAPI int xpkPathRemove(xpkObject xpk, const char* filePath) {
    if (!xpk || !filePath) return -1;
    
    uint32_t pos = xpkPathFind(xpk, filePath);
    if (pos == UINT32_MAX) {
        xpkSetError(6, "Path not found");
        return -1;
    }
    
    return xpkRemove(xpk, pos);
}

// ============================================================================
// 路径获取
// ============================================================================

XPKAPI const char* xpkPathGet(xpkObject xpk, uint32_t pos) {
    if (!xpk || pos >= xpk->ldb.Count) return NULL;
    
    int packType = xpkType(xpk);
    if (packType == XPK_TYPE_LINUX) {
        xpkFileInfoLinux* info = (xpkFileInfoLinux*)XPK_LDB_GET(xpk, pos);
        return info ? info->filePath : NULL;
    } else if (packType == XPK_TYPE_WIN32) {
        xpkFileInfoWin32* info = (xpkFileInfoWin32*)XPK_LDB_GET(xpk, pos);
        return info ? info->filePath : NULL;
    }
    
    return NULL;
}
