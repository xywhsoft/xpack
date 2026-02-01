/*
 * xPack Ver7 - Index 模式操作
 * 
 * 包含：Index 模式的添加、查找、提取、更新、删除
 */

#include "xpack_internal.h"
#include <string.h>
#include <stdlib.h>

// 外部声明
extern uint32_t xpkCompressBound(int level, uint32_t srcSize);

// ============================================================================
// 查找操作
// ============================================================================

XPKAPI uint32_t xpkIndexFind(xpkObject xpk, int32_t index) {
    if (!xpk) return UINT32_MAX;
    if (xpkType(xpk) != XPK_TYPE_INDEX) return UINT32_MAX;
    
    for (uint32_t i = 0; i < xpk->ldb.Count; i++) {
        xpkFileInfoIndex* info = (xpkFileInfoIndex*)XPK_LDB_GET(xpk, i);
        if (info && info->fileIndex == index) {
            return i;
        }
    }
    
    return UINT32_MAX;
}

// ============================================================================
// 添加操作
// ============================================================================

XPKAPI xpkFileInfoIndex* xpkIndexAppendFile(xpkObject xpk, int32_t index,
                                             const char* path, int level) {
	if (!xpk || !path) return NULL;
	if (xpk->readonly) {
		xpkSetError(10, "Cannot append in readonly mode");
		return NULL;
	}
	if (xpkType(xpk) != XPK_TYPE_INDEX) {
		xpkSetError(11, "Pack type is not Index");
		return NULL;
	}
	
	// 固实包禁止追加
	if (xpk->head.flag.solidMode) {
		xpkSetError(11, "Cannot append to solid archive pack");
		return NULL;
	}
	
	// 读取文件内容
	size_t fileSize = 0;
	void* fileData = xrtFileGetAll((str)path, &fileSize);
	if (!fileData) {
		xpkSetError(2, "Failed to read source file");
		return NULL;
	}
	
	xpkFileInfoIndex* info = xpkIndexAppendData(xpk, index, fileData,
	                                                  (uint32_t)fileSize, level);
	free(fileData);
	
	return info;
}

XPKAPI xpkFileInfoIndex* xpkIndexAppendData(xpkObject xpk, int32_t index,
                                             const void* data, uint32_t size, int level) {
	if (!xpk) return NULL;
	if (xpk->readonly) {
		xpkSetError(10, "Cannot append in readonly mode");
		return NULL;
	}
	if (xpkType(xpk) != XPK_TYPE_INDEX) {
		if (xpkTypeSet(xpk, XPK_TYPE_INDEX) != 0) {
			xpkSetError(11, "Failed to set pack type");
			return NULL;
		}
	}
	
	// 固实包禁止追加
	if (xpk->head.flag.solidMode) {
		xpkSetError(11, "Cannot append to solid archive pack");
		return NULL;
	}
	
	// 检查索引是否已存在
	if (xpkIndexFind(xpk, index) != UINT32_MAX) {
		xpkSetError(11, "Index already exists");
		return NULL;
	}
	
	// 处理空数据
	if (!data || size == 0) {
		data = "";
		size = 0;
	}
	
	// 限制压缩级别
	level = level & 0x0F;
	
	// 压缩数据
	uint32_t compSize = 0;
	void* compData = NULL;
	
	if (size == 0) {
		// 空数据不进行压缩
		compSize = 0;
		compData = NULL;
	} else {
		// 计算压缩缓冲区大小
		uint32_t compBound = xpkCompressBound(level, size);
		compData = malloc(compBound);
		if (!compData) {
			xpkSetError(3, "Failed to allocate compression buffer");
			return NULL;
		}
		
		// 压缩数据
		if (xpkCompressRouter(level, data, size, compData, compBound, &compSize) != 0) {
			free(compData);
			xpkSetError(7, "Compression failed");
			return NULL;
		}
	}
	
	// 计算文件哈希
	uint32_t fileHash = xrtHash32((ptr)data, size);
	
	// 计算数据偏移
	uint32_t dataOffset = sizeof(xpkHead) + xpk->head.headExtSize;
	if (xpk->ldb.Count > 0) {
		xpkFileInfoIndex* lastInfo = (xpkFileInfoIndex*)XPK_LDB_GET(xpk,
		                                                             xpk->ldb.Count - 1);
		if (lastInfo) {
			dataOffset = lastInfo->dataOffset + lastInfo->dataSize;
		}
	}
	
	// 写入压缩数据
	if (compSize > 0 && compData) {
		xrtSeek(xpk->file, xpk->baseOffset + dataOffset, XRT_SEEK_SET);
		if (xrtPut(xpk->file, compData, compSize) != (int)compSize) {
			free(compData);
			xpkSetError(2, "Failed to write data");
			return NULL;
		}
		free(compData);
	}
	
	// 追加文件信息
	uint32_t pos = xrtArrayAppend(&xpk->ldb, 1);
	xpkFileInfoIndex* info = (xpkFileInfoIndex*)xrtArrayGet(&xpk->ldb, pos);
	if (!info) {
		xpkSetError(3, "Failed to allocate file info");
		return NULL;
	}
	
	// 填充文件信息
	info->dataOffset = dataOffset;
	info->dataSize = compSize;
	info->fileSize = size;
	info->fileHash = fileHash;
	info->flag.value = 0;
	info->flag.compLevel = level;
	info->flag.fileType = XPK_FTYPE_UNKNOWN;
	info->fileIndex = index;
	info->userData = 0;
	
	xpk->modified = 1;
	return info;
}

// ============================================================================
// 提取操作
// ============================================================================

XPKAPI int xpkIndexExtractFile(xpkObject xpk, int32_t index, const char* path) {
    if (!xpk || !path) return -1;
    if (xpkType(xpk) != XPK_TYPE_INDEX) return -1;
    
    uint32_t pos = xpkIndexFind(xpk, index);
    if (pos == UINT32_MAX) {
        xpkSetError(6, "Index not found");
        return -1;
    }
    
    return xpkExtractFile(xpk, pos, path);
}

XPKAPI void* xpkIndexExtractData(xpkObject xpk, int32_t index, uint32_t* outSize) {
    if (!xpk) return NULL;
    if (xpkType(xpk) != XPK_TYPE_INDEX) return NULL;
    
    uint32_t pos = xpkIndexFind(xpk, index);
    if (pos == UINT32_MAX) {
        xpkSetError(6, "Index not found");
        return NULL;
    }
    
    return xpkExtractData(xpk, pos, outSize);
}

// ============================================================================
// 更新操作
// ============================================================================

XPKAPI int xpkIndexUpdateFile(xpkObject xpk, int32_t index, const char* path, int level) {
    if (!xpk || !path) return -1;
    if (xpkType(xpk) != XPK_TYPE_INDEX) return -1;
    
    uint32_t pos = xpkIndexFind(xpk, index);
    if (pos == UINT32_MAX) {
        xpkSetError(6, "Index not found");
        return -1;
    }
    
    return xpkUpdateFile(xpk, pos, path, level);
}

XPKAPI int xpkIndexUpdateData(xpkObject xpk, int32_t index, const void* data, 
                               uint32_t size, int level) {
    if (!xpk) return -1;
    if (xpkType(xpk) != XPK_TYPE_INDEX) return -1;
    
    uint32_t pos = xpkIndexFind(xpk, index);
    if (pos == UINT32_MAX) {
        xpkSetError(6, "Index not found");
        return -1;
    }
    
    return xpkUpdateData(xpk, pos, data, size, level);
}

// ============================================================================
// 删除操作
// ============================================================================

XPKAPI int xpkIndexRemove(xpkObject xpk, int32_t index) {
    if (!xpk) return -1;
    if (xpkType(xpk) != XPK_TYPE_INDEX) return -1;
    
    uint32_t pos = xpkIndexFind(xpk, index);
    if (pos == UINT32_MAX) {
        xpkSetError(6, "Index not found");
        return -1;
    }
    
    return xpkRemove(xpk, pos);
}

// ============================================================================
// 用户数据操作
// ============================================================================

XPKAPI int32_t xpkIndexUserData(xpkObject xpk, int32_t index) {
    if (!xpk) return 0;
    if (xpkType(xpk) != XPK_TYPE_INDEX) return 0;
    
    uint32_t pos = xpkIndexFind(xpk, index);
    if (pos == UINT32_MAX) return 0;
    
    xpkFileInfoIndex* info = (xpkFileInfoIndex*)XPK_LDB_GET(xpk, pos);
    return info ? info->userData : 0;
}

XPKAPI int xpkIndexUserDataSet(xpkObject xpk, int32_t index, int32_t value) {
    if (!xpk) return -1;
    if (xpk->readonly) {
        xpkSetError(10, "Cannot modify in readonly mode");
        return -1;
    }
    if (xpkType(xpk) != XPK_TYPE_INDEX) return -1;
    
    uint32_t pos = xpkIndexFind(xpk, index);
    if (pos == UINT32_MAX) {
        xpkSetError(6, "Index not found");
        return -1;
    }
    
    xpkFileInfoIndex* info = (xpkFileInfoIndex*)XPK_LDB_GET(xpk, pos);
    if (!info) return -1;
    
    info->userData = value;
    xpk->modified = 1;
    return 0;
}
