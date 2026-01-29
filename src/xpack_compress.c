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
#include <lzma/Lzma2Enc.h>
#include <lzma/Lzma2Dec.h>
#include <lzma/Alloc.h>

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
            // nativeLevel: 1=fast(默认), 2=fast(64KB块/更高加速)
            int acceleration = (map->nativeLevel >= 2) ? 2 : 1;
            int compSize = LZ4_compress_fast((const char*)src, (char*)dst, 
                                              (int)srcSize, (int)dstCapacity,
                                              acceleration);
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
            // ZSTD 压缩（按策略）
            ZSTD_CCtx* cctx = ZSTD_createCCtx();
            if (!cctx) return -1;
            
            // 禁用 checksum（xPack 使用 xrtHash32 代替）
            ZSTD_CCtx_setParameter(cctx, ZSTD_c_checksumFlag, 0);
            // 设置压缩策略（nativeLevel = ZSTD_strategy 枚举值）
            ZSTD_CCtx_setParameter(cctx, ZSTD_c_strategy, map->nativeLevel);
            
            size_t compSize = ZSTD_compress2(cctx, dst, dstCapacity, src, srcSize);
            ZSTD_freeCCtx(cctx);
            
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
            
        case XPK_ALG_LZMA2: {
            // LZMA2 压缩
            CLzma2EncHandle enc = Lzma2Enc_Create(&g_Alloc, &g_BigAlloc);
            if (!enc) return -1;
            
            // 设置压缩属性
            CLzma2EncProps props;
            Lzma2EncProps_Init(&props);
            props.lzmaProps.level = map->nativeLevel;
            
            SRes res = Lzma2Enc_SetProps(enc, &props);
            if (res != SZ_OK) {
                Lzma2Enc_Destroy(enc);
                return -1;
            }
            
            // 获取属性字节（解压时需要）
            Byte propByte = Lzma2Enc_WriteProperties(enc);
            
            // 输出格式: [propByte(1)] + [compressed data]
            if (dstCapacity < 1) {
                Lzma2Enc_Destroy(enc);
                return -1;
            }
            
            ((Byte*)dst)[0] = propByte;
            size_t destLen = dstCapacity - 1;
            
            // 执行压缩
            res = Lzma2Enc_Encode2(enc, 
                NULL, (Byte*)dst + 1, &destLen,
                NULL, (const Byte*)src, srcSize,
                NULL);
            
            Lzma2Enc_Destroy(enc);
            
            if (res != SZ_OK) {
                // 压缩失败，回退到无压缩
                if (dstCapacity < srcSize) return -1;
                memcpy(dst, src, srcSize);
                *outSize = srcSize;
                return 0;
            }
            
            *outSize = (uint32_t)(destLen + 1);  // +1 for propByte
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
            
        case XPK_ALG_LZMA2: {
            // LZMA2 解压
            // 输入格式: [propByte(1)] + [compressed data]
            if (srcSize < 1) return -1;
            
            Byte propByte = ((const Byte*)src)[0];
            SizeT destLen = dstSize;
            SizeT srcLen = srcSize - 1;
            ELzmaStatus status;
            
            SRes res = Lzma2Decode(
                (Byte*)dst, &destLen,
                (const Byte*)src + 1, &srcLen,
                propByte, LZMA_FINISH_END, &status, &g_Alloc);
            
            if (res != SZ_OK || destLen != dstSize) {
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
            
        case XPK_ALG_LZMA2:
            // LZMA2 最坏情况: 原始大小 + 1 (propByte) + 少量开销
            // LZMA2 块头最大约 5 字节/64KB，保守估计增加 1%
            return srcSize + (srcSize / 100) + 1024 + 1;
            
        default:
            return srcSize;
    }
}
