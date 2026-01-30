/*
 * xPack Ver7 - LDB 操作
 * 
 * 包含：LDB 加载/保存
 */

#include "xpack_internal.h"
#include <string.h>
#include <stdlib.h>

// 外部声明
extern uint32_t xpkCompressBound(int level, uint32_t srcSize);

// 文件信息大小表
static const uint32_t xpkInfoSizes[4] = {
    sizeof(xpkFileInfo),        // Core:  20 bytes
    sizeof(xpkFileInfoIndex),   // Index: 28 bytes
    sizeof(xpkFileInfoLinux),   // Linux: 232 bytes
    sizeof(xpkFileInfoWin32)    // Win32: 236 bytes
};

// ============================================================================
// LDB 加载
// ============================================================================

int xpkLdbLoad(xpkObject xpk) {
    if (!xpk) return -1;
    if (xpk->head.fileCount == 0) return 0;
    
    int packType = xpk->head.flag.packType & 0x03;
    uint32_t infoSize = xpkInfoSizes[packType];
    
    // 加上扩展大小
    uint32_t totalInfoSize = infoSize + xpk->head.infoExtSize;
    uint32_t ldbRawSize = xpk->head.fileCount * totalInfoSize;
    
    // 读取压缩的 LDB 数据
    xrtSeek(xpk->file, xpk->baseOffset + xpk->head.ldbOffset, XRT_SEEK_SET);
    size_t readSize = 0;
    void* compData = xrtGet(xpk->file, xpk->head.ldbSize, &readSize);
    
    if (!compData || readSize != xpk->head.ldbSize) {
        if (compData) free(compData);
        xpkSetError(2, "Failed to read LDB data");
        return -1;
    }
    
    // 验证 LDB 哈希
    uint32_t hash = xrtHash32(compData, xpk->head.ldbSize);
    if (hash != xpk->head.ldbHash) {
        free(compData);
        xpkSetError(9, "LDB hash verification failed");
        return -1;
    }
    
    // 分配解压缓冲区
    void* rawData = malloc(xpk->head.ldbRawSize);
    if (!rawData) {
        free(compData);
        xpkSetError(3, "Failed to allocate LDB buffer");
        return -1;
    }
    
    // 解压 LDB 数据
    int ldbLevel = xpk->head.flag.ldbComp;
    if (xpkDecompressRouter(ldbLevel, compData, xpk->head.ldbSize,
                             rawData, xpk->head.ldbRawSize) != 0) {
        free(compData);
        free(rawData);
        xpkSetError(8, "LDB decompression failed");
        return -1;
    }
    free(compData);
    
    // 分配 LDB 数组空间
    if (!xrtArrayAlloc(&xpk->ldb, xpk->head.fileCount)) {
        free(rawData);
        xpkSetError(3, "Failed to allocate LDB array");
        return -1;
    }
    
    // 复制文件信息到数组
    // 注意：这里需要处理扩展数据的情况
    uint8_t* srcPtr = (uint8_t*)rawData;
    for (uint32_t i = 0; i < xpk->head.fileCount; i++) {
        xrtArrayAppend(&xpk->ldb, 1);
        void* dstInfo = XPK_LDB_GET(xpk, i);
        if (dstInfo) {
            memcpy(dstInfo, srcPtr, infoSize);
        }
        srcPtr += totalInfoSize;
    }
    
    free(rawData);
    return 0;
}

// ============================================================================
// LDB 保存
// ============================================================================

int xpkLdbSave(xpkObject xpk) {
	if (!xpk) return -1;
	if (xpk->readonly) return -1;
	
	if (xpk->ldb.Count == 0) {
		// 无文件，清空 LDB 信息
		xpk->head.ldbOffset = sizeof(xpkHead) + xpk->head.headExtSize;
		xpk->head.ldbSize = 0;
		xpk->head.ldbRawSize = 0;
		xpk->head.ldbHash = 0;
		return 0;
	}
	
	int packType = xpk->head.flag.packType & 0x03;
	uint32_t infoSize = xpkInfoSizes[packType];
	uint32_t totalInfoSize = infoSize + xpk->head.infoExtSize;
	uint32_t ldbRawSize = xpk->ldb.Count * totalInfoSize;
	
	// 分配原始数据缓冲区
	void* rawData = malloc(ldbRawSize);
	if (!rawData) {
		xpkSetError(3, "Failed to allocate LDB buffer");
		return -1;
	}
	memset(rawData, 0, ldbRawSize);
	
	// 复制文件信息到缓冲区
	uint8_t* dstPtr = (uint8_t*)rawData;
	for (uint32_t i = 0; i < xpk->ldb.Count; i++) {
		void* srcInfo = XPK_LDB_GET(xpk, i);
		if (srcInfo) {
			memcpy(dstPtr, srcInfo, infoSize);
		}
		dstPtr += totalInfoSize;
	}
	
	// 计算 LDB 偏移（固实模式下已在 xpkSolidSave 中设置）
	uint32_t ldbOffset = xpk->head.ldbOffset;
	if (!xpk->head.flag.solidMode) {
		// 独立模式：计算在最后一个文件数据之后
		ldbOffset = sizeof(xpkHead) + xpk->head.headExtSize;
		if (xpk->ldb.Count > 0) {
			xpkFileInfo* lastInfo = (xpkFileInfo*)XPK_LDB_GET(xpk, xpk->ldb.Count - 1);
			if (lastInfo) {
				ldbOffset = lastInfo->dataOffset + lastInfo->dataSize;
			}
		}
	}
	
	// 压缩 LDB 数据
	int ldbLevel = xpk->head.flag.ldbComp;
	uint32_t compBound = xpkCompressBound(ldbLevel, ldbRawSize);
	void* compData = malloc(compBound);
	if (!compData) {
		free(rawData);
		xpkSetError(3, "Failed to allocate compression buffer");
		return -1;
	}
	
	uint32_t compSize = 0;
	if (xpkCompressRouter(ldbLevel, rawData, ldbRawSize, compData, compBound, &compSize) != 0) {
		free(rawData);
		free(compData);
		xpkSetError(7, "LDB compression failed");
		return -1;
	}
	free(rawData);
	
	// 计算 LDB 哈希
	uint32_t ldbHash = xrtHash32(compData, compSize);
	
	// 写入 LDB 数据
	xrtSeek(xpk->file, xpk->baseOffset + ldbOffset, XRT_SEEK_SET);
	if (xrtPut(xpk->file, compData, compSize) != (int)compSize) {
		free(compData);
		xpkSetError(2, "Failed to write LDB data");
		return -1;
	}
	free(compData);
	
	// 设置文件结束位置
	xrtSetEOF(xpk->file);
	
	// 更新包头中的 LDB 信息（固实模式下不更新 ldbOffset）
	xpk->head.ldbSize = compSize;
	xpk->head.ldbRawSize = ldbRawSize;
	xpk->head.ldbHash = ldbHash;
	if (!xpk->head.flag.solidMode) {
		xpk->head.ldbOffset = ldbOffset;
	}
	
	return 0;
}
