/*
 * xPack Ver7 - Core 模式操作
 * 
 * 包含：添加、提取、更新、删除、信息获取
 */

#include "xpack_internal.h"
#include <string.h>
#include <stdlib.h>

// 外部声明
extern uint32_t xpkCompressBound(int level, uint32_t srcSize);

// ============================================================================
// 添加操作
// ============================================================================

XPKAPI uint32_t xpkAppendFile(xpkObject xpk, const char* path, int level) {
    if (!xpk || !path) return UINT32_MAX;
    if (xpk->readonly) {
        xpkSetError(10, "Cannot append in readonly mode");
        return UINT32_MAX;
    }
    
    // 读取文件内容
    size_t fileSize = 0;
    void* fileData = xrtFileGetAll((str)path, &fileSize);
    if (!fileData) {
        xpkSetError(2, "Failed to read source file");
        return UINT32_MAX;
    }
    
    uint32_t pos = xpkAppendData(xpk, fileData, (uint32_t)fileSize, level);
    free(fileData);
    
    return pos;
}

XPKAPI uint32_t xpkAppendData(xpkObject xpk, const void* data, uint32_t size, int level) {
	if (!xpk) return UINT32_MAX;
	if (xpk->readonly) {
		xpkSetError(10, "Cannot append in readonly mode");
		return UINT32_MAX;
	}
	
	// 固实模式：使用固实压缩函数
	if (xpk->head.flag.solidMode) {
		return xpkSolidAppendData(xpk, data, size, level);
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
			return UINT32_MAX;
		}
		
		// 压缩数据
		if (xpkCompressRouter(level, data, size, compData, compBound, &compSize) != 0) {
			free(compData);
			xpkSetError(7, "Compression failed");
			return UINT32_MAX;
		}
	}
	
	// 计算文件哈希
	uint32_t fileHash = xrtHash32((ptr)data, size);
	
	// 计算数据偏移
	uint32_t dataOffset = xpk->baseOffset + sizeof(xpkHead) + xpk->head.headExtSize;
	
	// 如果有现有文件，计算下一个数据偏移
	if (xpk->ldb.Count > 0) {
		xpkFileInfo* lastInfo = (xpkFileInfo*)XPK_LDB_GET(xpk, xpk->ldb.Count - 1);
		if (lastInfo) {
			dataOffset = lastInfo->dataOffset + lastInfo->dataSize;
		}
	}
	
	// 写入压缩数据到文件
	xrtSeek(xpk->file, dataOffset, XRT_SEEK_SET);
	if (compSize > 0 && compData) {
		if (xpkVolumeWriteData(xpk, compData, compSize) != 0) {
			free(compData);
			return UINT32_MAX;
		}
		free(compData);
	}
	
	// 追加文件信息到 LDB
	uint32_t pos1 = xrtArrayAppend(&xpk->ldb, 1);  // 1-based position
	xpkFileInfo* info = (xpkFileInfo*)xrtArrayGet(&xpk->ldb, pos1);
	if (!info) {
		xpkSetError(3, "Failed to allocate file info");
		return UINT32_MAX;
	}
	
	// 填充文件信息
	info->dataOffset = dataOffset;
	info->dataSize = compSize;
	info->fileSize = size;
	info->fileHash = fileHash;
	info->flag.value = 0;
	info->flag.compLevel = level;
	info->flag.fileType = XPK_FTYPE_UNKNOWN;
	
	xpk->modified = 1;
	return pos1 - 1;  // Return 0-based position
}

// ============================================================================
// 提取操作
// ============================================================================

XPKAPI int xpkExtractFile(xpkObject xpk, uint32_t pos, const char* path) {
    if (!xpk || !path) return -1;
    
    uint32_t outSize = 0;
    void* data = xpkExtractData(xpk, pos, &outSize);
    if (!data) return -1;
    
    int result = xrtFilePutAll((str)path, data, outSize);
    free(data);
    
    return (result == (int)outSize) ? 0 : -1;
}

XPKAPI void* xpkExtractData(xpkObject xpk, uint32_t pos, uint32_t* outSize) {
	if (!xpk || pos >= xpk->ldb.Count) {
		xpkSetError(6, "Invalid file position");
		return NULL;
	}
	
	xpkFileInfo* info = (xpkFileInfo*)XPK_LDB_GET(xpk, pos);
	if (!info) {
		xpkSetError(6, "Failed to get file info");
		return NULL;
	}
	
	// 固实模式：使用固实解压
	if (xpk->head.flag.solidMode) {
		return xpkSolidExtractData(xpk, pos, info, outSize);
	}
	
	// 独立模式：标准解压
	// 处理空文件
	if (info->fileSize == 0 && info->dataSize == 0) {
		void* rawData = malloc(1);
		if (!rawData) {
			xpkSetError(3, "Failed to allocate buffer");
			return NULL;
		}
		if (outSize) *outSize = 0;
		return rawData;
	}
	
	// 读取压缩数据
	uint32_t readSize = 0;
	void* compData = xpkVolumeReadData(xpk, info->dataOffset, info->dataSize, &readSize);
	if (!compData || readSize != info->dataSize) {
		if (compData) free(compData);
		xpkSetError(2, "Failed to read compressed data");
		return NULL;
	}
	
	// 分配解压缓冲区
	void* rawData = malloc(info->fileSize > 0 ? info->fileSize : 1);
	if (!rawData) {
		free(compData);
		xpkSetError(3, "Failed to allocate decompression buffer");
		return NULL;
	}
	
	// 解压数据
	if (info->fileSize > 0) {
		if (xpkDecompressRouter(info->flag.compLevel, compData, info->dataSize,
		                     rawData, info->fileSize) != 0) {
			free(compData);
			free(rawData);
			xpkSetError(8, "Decompression failed");
			return NULL;
		}
	}
	free(compData);
	
	if (outSize) *outSize = info->fileSize;
	return rawData;
}

// ============================================================================
// 更新操作
// ============================================================================

XPKAPI int xpkUpdateFile(xpkObject xpk, uint32_t pos, const char* path, int level) {
    if (!xpk || !path) return -1;
    if (xpk->readonly) {
        xpkSetError(10, "Cannot update in readonly mode");
        return -1;
    }
    
    // 读取文件内容
    size_t fileSize = 0;
    void* fileData = xrtFileGetAll((str)path, &fileSize);
    if (!fileData) {
        xpkSetError(2, "Failed to read source file");
        return -1;
    }
    
    int result = xpkUpdateData(xpk, pos, fileData, (uint32_t)fileSize, level);
    free(fileData);
    
    return result;
}

XPKAPI int xpkUpdateData(xpkObject xpk, uint32_t pos, const void* data, uint32_t size, int level) {
	if (!xpk) return -1;
	if (xpk->readonly) {
		xpkSetError(10, "Cannot update in readonly mode");
		return -1;
	}
	
	// 固实包禁止更新
	if (xpk->head.flag.solidMode) {
		xpkSetError(11, "Cannot update solid archive pack");
		return -1;
	}
	
	if (pos >= xpk->ldb.Count) {
		xpkSetError(6, "Invalid file position");
		return -1;
	}
	
	xpkFileInfo* info = (xpkFileInfo*)XPK_LDB_GET(xpk, pos);
	if (!info) {
		xpkSetError(6, "Failed to get file info");
		return -1;
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
			return -1;
		}
		
		// 压缩数据
		if (xpkCompressRouter(level, data, size, compData, compBound, &compSize) != 0) {
			free(compData);
			xpkSetError(7, "Compression failed");
			return -1;
		}
	}
	
	// 计算文件哈希
	uint32_t fileHash = xrtHash32((ptr)data, size);
	
	// 如果新数据大小不超过原数据大小，可以原地更新
	// 否则需要追加到文件末尾（留下空洞，需要 rebuild 优化）
	uint32_t dataOffset;
	if (compSize <= info->dataSize) {
		dataOffset = info->dataOffset;
	} else {
		// 追加到文件末尾
		xrtSeek(xpk->file, 0, XRT_SEEK_END);
		dataOffset = (uint32_t)xrtTell(xpk->file) - xpk->baseOffset;
	}
	
	// 写入压缩数据
	if (compSize > 0 && compData) {
		xrtSeek(xpk->file, xpk->baseOffset + dataOffset, XRT_SEEK_SET);
		if (xrtPut(xpk->file, compData, compSize) != (int)compSize) {
			free(compData);
			xpkSetError(2, "Failed to write data");
			return -1;
		}
		free(compData);
	}
	
	// 更新文件信息
	info->dataOffset = dataOffset;
	info->dataSize = compSize;
	info->fileSize = size;
	info->fileHash = fileHash;
	info->flag.compLevel = level;
	
	xpk->modified = 1;
	return 0;
}

// ============================================================================
// 删除操作
// ============================================================================

XPKAPI int xpkRemove(xpkObject xpk, uint32_t pos) {
	if (!xpk) return -1;
	if (xpk->readonly) {
		xpkSetError(10, "Cannot remove in readonly mode");
		return -1;
	}
	
	// 固实包禁止删除
	if (xpk->head.flag.solidMode) {
		xpkSetError(11, "Cannot remove from solid archive pack");
		return -1;
	}
	
	if (pos >= xpk->ldb.Count) {
		xpkSetError(6, "Invalid file position");
		return -1;
	}
	
	// 从 LDB 中删除
	if (!XPK_LDB_REMOVE(xpk, pos, 1)) {
		xpkSetError(3, "Failed to remove from LDB");
		return -1;
	}
	
	xpk->modified = 1;
	return 0;
}

// ============================================================================
// 信息获取
// ============================================================================

XPKAPI void* xpkInfo(xpkObject xpk, uint32_t pos) {
    if (!xpk || pos >= xpk->ldb.Count) return NULL;
    return XPK_LDB_GET(xpk, pos);
}

XPKAPI uint32_t xpkInfoSize(xpkObject xpk, uint32_t pos) {
    xpkFileInfo* info = (xpkFileInfo*)xpkInfo(xpk, pos);
    return info ? info->fileSize : 0;
}

XPKAPI uint32_t xpkInfoPacked(xpkObject xpk, uint32_t pos) {
    xpkFileInfo* info = (xpkFileInfo*)xpkInfo(xpk, pos);
    return info ? info->dataSize : 0;
}

XPKAPI uint32_t xpkInfoHash(xpkObject xpk, uint32_t pos) {
    xpkFileInfo* info = (xpkFileInfo*)xpkInfo(xpk, pos);
    return info ? info->fileHash : 0;
}

XPKAPI int xpkInfoLevel(xpkObject xpk, uint32_t pos) {
    xpkFileInfo* info = (xpkFileInfo*)xpkInfo(xpk, pos);
    return info ? info->flag.compLevel : -1;
}

XPKAPI int xpkInfoType(xpkObject xpk, uint32_t pos) {
    xpkFileInfo* info = (xpkFileInfo*)xpkInfo(xpk, pos);
    return info ? info->flag.fileType : -1;
}

XPKAPI int xpkInfoTypeSet(xpkObject xpk, uint32_t pos, int type) {
    if (!xpk || xpk->readonly) return -1;
    xpkFileInfo* info = (xpkFileInfo*)xpkInfo(xpk, pos);
    if (!info) return -1;
    
    info->flag.fileType = type & 0x0F;
    xpk->modified = 1;
    return 0;
}
