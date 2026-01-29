/*
 * xPack Ver7 - 压缩模块
 * 
 * 包含：压缩/解压路由
 */

#include "xpack_internal.h"
#include <string.h>
#include <lz4/lz4.h>
#include <lz4/lz4hc.h>
#include <zstd/zstd.h>

// ============================================================================
// 压缩路由
// ============================================================================

int xpkCompressRouter(int level, const void* src, uint32_t srcSize, 
                      void* dst, uint32_t dstCapacity, uint32_t* outSize) {
    if (!src || !dst || !outSize || srcSize == 0) {
        return -1;
    }
    
    // 限制级别范围
    level = level & 0x0F;
    const xpkCompMap* map = &xpkCompTable[level];
    
    switch (map->algorithm) {
        case XPK_ALG_STORE:
            // 无压缩：直接复制
            if (dstCapacity < srcSize) return -1;
            memcpy(dst, src, srcSize);
            *outSize = srcSize;
            return 0;
            
        case XPK_ALG_LZ4: {
            // LZ4 快速压缩
            int compSize = LZ4_compress_default((const char*)src, (char*)dst, 
                                                 (int)srcSize, (int)dstCapacity);
            if (compSize <= 0) {
                // 压缩失败，回退到无压缩
                if (dstCapacity < srcSize) return -1;
                memcpy(dst, src, srcSize);
                *outSize = srcSize;
                return 0;
            }
            *outSize = (uint32_t)compSize;
            return 0;
        }
            
        case XPK_ALG_LZ4HC: {
            // LZ4-HC 高压缩
            int compSize = LZ4_compress_HC((const char*)src, (char*)dst, 
                                           (int)srcSize, (int)dstCapacity, 
                                           map->nativeLevel);
            if (compSize <= 0) {
                // 压缩失败，回退到无压缩
                if (dstCapacity < srcSize) return -1;
                memcpy(dst, src, srcSize);
                *outSize = srcSize;
                return 0;
            }
            *outSize = (uint32_t)compSize;
            return 0;
        }
            
        case XPK_ALG_ZSTD: {
            // ZSTD 压缩
            size_t compSize = ZSTD_compress(dst, dstCapacity, src, srcSize, 
                                            map->nativeLevel);
            if (ZSTD_isError(compSize)) {
                // 压缩失败，回退到无压缩
                if (dstCapacity < srcSize) return -1;
                memcpy(dst, src, srcSize);
                *outSize = srcSize;
                return 0;
            }
            *outSize = (uint32_t)compSize;
            return 0;
        }
            
        default:
            return -1;
    }
}

// ============================================================================
// 解压路由
// ============================================================================

int xpkDecompressRouter(int level, const void* src, uint32_t srcSize,
                        void* dst, uint32_t dstSize) {
    if (!src || !dst || srcSize == 0 || dstSize == 0) {
        return -1;
    }
    
    // 如果压缩后大小等于原始大小，说明未压缩
    if (srcSize == dstSize) {
        memcpy(dst, src, srcSize);
        return 0;
    }
    
    // 限制级别范围
    level = level & 0x0F;
    const xpkCompMap* map = &xpkCompTable[level];
    
    switch (map->algorithm) {
        case XPK_ALG_STORE:
            // 无压缩：直接复制
            if (srcSize != dstSize) return -1;
            memcpy(dst, src, srcSize);
            return 0;
            
        case XPK_ALG_LZ4:
        case XPK_ALG_LZ4HC: {
            // LZ4 解压（LZ4 和 LZ4-HC 使用相同的解压函数）
            int decompSize = LZ4_decompress_safe((const char*)src, (char*)dst, 
                                                  (int)srcSize, (int)dstSize);
            if (decompSize < 0 || (uint32_t)decompSize != dstSize) {
                return -1;
            }
            return 0;
        }
            
        case XPK_ALG_ZSTD: {
            // ZSTD 解压
            size_t decompSize = ZSTD_decompress(dst, dstSize, src, srcSize);
            if (ZSTD_isError(decompSize) || decompSize != dstSize) {
                return -1;
            }
            return 0;
        }
            
        default:
            return -1;
    }
}

// ============================================================================
// 压缩辅助函数
// ============================================================================

// 获取压缩后最大可能大小
uint32_t xpkCompressBound(int level, uint32_t srcSize) {
    level = level & 0x0F;
    const xpkCompMap* map = &xpkCompTable[level];
    
    switch (map->algorithm) {
        case XPK_ALG_STORE:
            return srcSize;
            
        case XPK_ALG_LZ4:
        case XPK_ALG_LZ4HC:
            return (uint32_t)LZ4_compressBound((int)srcSize);
            
        case XPK_ALG_ZSTD:
            return (uint32_t)ZSTD_compressBound(srcSize);
            
        default:
            return srcSize;
    }
}
