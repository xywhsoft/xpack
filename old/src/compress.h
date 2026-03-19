/*
 * xPack Ver7 - 压缩模块
 *
 * 包含：压缩/解压路由、算法映射、哈希校验
 * 
 * 压缩级别说明：
 *   0:  无压缩 (STORE)
 *   1-2:  LZ4 快速压缩
 *   3-4:  LZ4-HC 高压缩
 *   5-13: ZSTD (fast -> btultra2)
 *   14-15: LZMA2
 * 
 * 规则：
 *   1. 压缩后如果变大，则回退为不压缩
 *   2. 压缩时计算原始数据的哈希值
 *   3. 解压时验证哈希值，不匹配则返回失败
 */

#include "base.h"

// 压缩库头文件
#include <lz4.h>
#include <lz4hc.h>
#include <zstd.h>
#include <Lzma2Enc.h>
#include <Lzma2Dec.h>
#include <Alloc.h>

/* clang-format off */

// ============================================================================
// 压缩上界计算
// ============================================================================

uint64_t _xpkCompressBound(int level, uint64_t srcSize)
{
    level = level & 0x0F;
    const xpkCompMap* map = &xpkCompTable[level];
    
    switch (map->algorithm) {
        case XPK_ALG_STORE:
            return srcSize;
            
        case XPK_ALG_LZ4:
        case XPK_ALG_LZ4HC:
            return (uint64_t)LZ4_compressBound((int)srcSize);
            
        case XPK_ALG_ZSTD:
            return (uint64_t)ZSTD_compressBound(srcSize);
            
        case XPK_ALG_LZMA2:
            return srcSize + (srcSize / 100) + 1024 + 1;
            
        default:
            return srcSize;
    }
}

// ============================================================================
// 压缩实现
// 
// 参数：
//   level       - 压缩级别 (0-15)
//   src         - 原始数据
//   srcSize     - 原始数据大小
//   dst         - 压缩数据输出缓冲区
//   dstCapacity - 输出缓冲区容量
//   result      - 压缩结果（包含 compSize, hash, algorithm）
// 
// 返回：
//   0  - 成功
//   -1 - 参数错误或压缩失败
// ============================================================================

int _xpkCompress(int level, const void* src, uint64_t srcSize, 
                  void* dst, uint64_t dstCapacity, xpkCompResult* result)
{
    // 参数检查
    if (!src || !dst || !result) {
        return -1;
    }
    
    // 初始化结果
    result->compSize = 0;
    result->hash = 0;
    result->algorithm = XPK_ALG_STORE;
    
    // 空数据处理
    if (srcSize == 0) {
        result->compSize = 0;
        result->hash = 0;
        result->algorithm = XPK_ALG_STORE;
        return 0;
    }
    
    // 计算原始数据哈希值 (使用 xrtHash32)
    result->hash = xrtHash32((ptr)src, (size_t)srcSize);
    
    level = level & 0x0F;
    const xpkCompMap* map = &xpkCompTable[level];
    
    // ------------------------------------------------------------
    // STORE: 无压缩
    // ------------------------------------------------------------
    if (map->algorithm == XPK_ALG_STORE) {
        if (dstCapacity < srcSize) {
            return -1;
        }
        memcpy(dst, src, (size_t)srcSize);
        result->compSize = srcSize;
        result->algorithm = XPK_ALG_STORE;
        return 0;
    }
    
    // ------------------------------------------------------------
    // LZ4 快速压缩 (level 1-2)
    // ------------------------------------------------------------
    if (map->algorithm == XPK_ALG_LZ4) {
        int acceleration = (map->nativeLevel >= 2) ? 2 : 1;
        int compSize = LZ4_compress_fast(
            (const char*)src, (char*)dst,
            (int)srcSize, (int)dstCapacity,
            acceleration
        );
        
        if (compSize > 0 && (uint64_t)compSize < srcSize) {
            result->compSize = (uint64_t)compSize;
            result->algorithm = XPK_ALG_LZ4;
            return 0;
        }
        
        // 回退到 STORE
        if (dstCapacity < srcSize) {
            return -1;
        }
        memcpy(dst, src, (size_t)srcSize);
        result->compSize = srcSize;
        result->algorithm = XPK_ALG_STORE;
        return 0;
    }
    
    // ------------------------------------------------------------
    // LZ4-HC 高压缩 (level 3-4)
    // ------------------------------------------------------------
    if (map->algorithm == XPK_ALG_LZ4HC) {
        int compSize = LZ4_compress_HC(
            (const char*)src, (char*)dst,
            (int)srcSize, (int)dstCapacity,
            map->nativeLevel
        );
        
        if (compSize > 0 && (uint64_t)compSize < srcSize) {
            result->compSize = (uint64_t)compSize;
            result->algorithm = XPK_ALG_LZ4HC;
            return 0;
        }
        
        // 回退到 STORE
        if (dstCapacity < srcSize) {
            return -1;
        }
        memcpy(dst, src, (size_t)srcSize);
        result->compSize = srcSize;
        result->algorithm = XPK_ALG_STORE;
        return 0;
    }
    
    // ------------------------------------------------------------
    // ZSTD 压缩 (level 5-13)
    // ------------------------------------------------------------
    if (map->algorithm == XPK_ALG_ZSTD) {
        ZSTD_CCtx* cctx = ZSTD_createCCtx();
        if (!cctx) {
            return -1;
        }
        
        ZSTD_CCtx_setParameter(cctx, ZSTD_c_checksumFlag, 0);
        ZSTD_CCtx_setParameter(cctx, ZSTD_c_strategy, map->nativeLevel);
        
        size_t compSize = ZSTD_compress2(cctx, dst, dstCapacity, src, srcSize);
        ZSTD_freeCCtx(cctx);
        
        if (!ZSTD_isError(compSize) && compSize < srcSize) {
            result->compSize = (uint64_t)compSize;
            result->algorithm = XPK_ALG_ZSTD;
            return 0;
        }
        
        // 回退到 STORE
        if (dstCapacity < srcSize) {
            return -1;
        }
        memcpy(dst, src, (size_t)srcSize);
        result->compSize = srcSize;
        result->algorithm = XPK_ALG_STORE;
        return 0;
    }
    
    // ------------------------------------------------------------
    // LZMA2 压缩 (level 14-15)
    // ------------------------------------------------------------
    if (map->algorithm == XPK_ALG_LZMA2) {
        CLzma2EncHandle enc = Lzma2Enc_Create(&g_Alloc, &g_BigAlloc);
        if (!enc) {
            return -1;
        }
        
        CLzma2EncProps props;
        Lzma2EncProps_Init(&props);
        props.lzmaProps.level = map->nativeLevel;
        
        SRes res = Lzma2Enc_SetProps(enc, &props);
        if (res != SZ_OK) {
            Lzma2Enc_Destroy(enc);
            return -1;
        }
        
        Byte propByte = Lzma2Enc_WriteProperties(enc);
        
        if (dstCapacity < 1) {
            Lzma2Enc_Destroy(enc);
            return -1;
        }
        
        ((Byte*)dst)[0] = propByte;
        size_t destLen = dstCapacity - 1;
        
        res = Lzma2Enc_Encode2(
            enc,
            NULL, (Byte*)dst + 1, &destLen,
            NULL, (const Byte*)src, srcSize,
            NULL
        );
        
        Lzma2Enc_Destroy(enc);
        
        if (res == SZ_OK && (destLen + 1) < srcSize) {
            result->compSize = (uint64_t)(destLen + 1);
            result->algorithm = XPK_ALG_LZMA2;
            return 0;
        }
        
        // 回退到 STORE
        if (dstCapacity < srcSize) {
            return -1;
        }
        memcpy(dst, src, (size_t)srcSize);
        result->compSize = srcSize;
        result->algorithm = XPK_ALG_STORE;
        return 0;
    }
    
    return -1;
}

// ============================================================================
// 解压实现
// 
// 参数：
//   level        - 压缩级别 (0-15)，用于确定算法
//   src          - 压缩数据
//   srcSize      - 压缩数据大小
//   dst          - 解压数据输出缓冲区
//   dstSize      - 解压数据大小（已知）
//   expectedHash - 期望的哈希值
// 
// 返回：
//   0  - 成功且哈希匹配
//   -1 - 参数错误或解压失败
//   -2 - 哈希校验失败
// ============================================================================

int _xpkDecompress(int level, const void* src, uint64_t srcSize,
                    void* dst, uint64_t dstSize, uint32_t expectedHash)
{
    // 参数检查
    if (!src || !dst) {
        return -1;
    }
    
    // 空数据处理
    if (srcSize == 0 && dstSize == 0) {
        // 空数据哈希应为 0
        if (expectedHash != 0) {
            return -2;  // 哈希不匹配
        }
        return 0;
    }
    
    // 如果压缩后大小等于原始大小，说明是 STORE
    if (srcSize == dstSize) {
        memcpy(dst, src, (size_t)srcSize);
        // 验证哈希
        uint32_t actualHash = xrtHash32((ptr)dst, (size_t)dstSize);
        if (actualHash != expectedHash) {
            return -2;  // 哈希不匹配
        }
        return 0;
    }
    
    level = level & 0x0F;
    const xpkCompMap* map = &xpkCompTable[level];
    
    // ------------------------------------------------------------
    // STORE: 直接复制
    // ------------------------------------------------------------
    if (map->algorithm == XPK_ALG_STORE) {
        if (srcSize != dstSize) {
            return -1;
        }
        memcpy(dst, src, (size_t)srcSize);
        uint32_t actualHash = xrtHash32((ptr)dst, (size_t)dstSize);
        if (actualHash != expectedHash) {
            return -2;
        }
        return 0;
    }
    
    // ------------------------------------------------------------
    // LZ4 / LZ4-HC 解压
    // ------------------------------------------------------------
    if (map->algorithm == XPK_ALG_LZ4 || map->algorithm == XPK_ALG_LZ4HC) {
        int decompSize = LZ4_decompress_safe(
            (const char*)src, (char*)dst,
            (int)srcSize, (int)dstSize
        );
        
        if (decompSize < 0 || (uint64_t)decompSize != dstSize) {
            return -1;
        }
        
        // 验证哈希
        uint32_t actualHash = xrtHash32((ptr)dst, (size_t)dstSize);
        if (actualHash != expectedHash) {
            return -2;
        }
        return 0;
    }
    
    // ------------------------------------------------------------
    // ZSTD 解压
    // ------------------------------------------------------------
    if (map->algorithm == XPK_ALG_ZSTD) {
        size_t decompSize = ZSTD_decompress(dst, dstSize, src, srcSize);
        
        if (ZSTD_isError(decompSize) || decompSize != dstSize) {
            return -1;
        }
        
        // 验证哈希
        uint32_t actualHash = xrtHash32((ptr)dst, (size_t)dstSize);
        if (actualHash != expectedHash) {
            return -2;
        }
        return 0;
    }
    
    // ------------------------------------------------------------
    // LZMA2 解压
    // ------------------------------------------------------------
    if (map->algorithm == XPK_ALG_LZMA2) {
        if (srcSize < 1) {
            return -1;
        }
        
        Byte propByte = ((const Byte*)src)[0];
        SizeT destLen = (SizeT)dstSize;
        SizeT srcLen = (SizeT)(srcSize - 1);
        ELzmaStatus status;
        
        SRes res = Lzma2Decode(
            (Byte*)dst, &destLen,
            (const Byte*)src + 1, &srcLen,
            propByte, LZMA_FINISH_END, &status, &g_Alloc
        );
        
        if (res != SZ_OK || destLen != dstSize) {
            return -1;
        }
        
        // 验证哈希
        uint32_t actualHash = xrtHash32((ptr)dst, (size_t)dstSize);
        if (actualHash != expectedHash) {
            return -2;
        }
        return 0;
    }
    
    return -1;
}

/* clang-format on */
