/*
 * xPack Ver7 - 内部头文件
 * 
 * 此文件仅供库内部使用，不对外公开
 */

#ifndef XPACK_INTERNAL_H
#define XPACK_INTERNAL_H

#include <xrt/xrt.h>
#include "xpack.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// xPack 对象内部结构
// ============================================================================
typedef struct xpkStruct {
	// 文件信息
	xfile               file;           // xrt 文件句柄
	uint32_t            baseOffset;     // 包在文件中的基础偏移
	
	// 状态标记
	uint8_t             readonly;       // 只读模式
	uint8_t             modified;       // 已修改标记
	uint8_t             reserved[2];    // 保留对齐
	
	// 包信息
	xpkHead             head;           // 包头信息
	void*               headExt;        // 包头扩展数据
	
	// 文件列表 (LDB)
	xarray_struct       ldb;            // 文件信息列表（使用 xrt 的 xarray）
	
	// 回调
	xpkErrorProc        onError;        // 错误回调
	
	// 固实压缩相关（仅创建时使用）
	uint8_t             solidMode;      // 固实模式标记
	uint8_t             solidCompLevel; // 固实块压缩级别
	uint8_t             reserved2[2];
	
	// 固实缓冲区（创建时暂存）
	xbuffer             solidBuffer;    // 固实数据缓冲区
	uint32_t            solidBufferSize;// 固实缓冲区大小
	
	// 固实块缓存（读取时使用）
	void*               solidDecompressed;  // 解压后的固实块数据
	uint32_t            solidDecompSize;    // 解压后的固实块大小
	uint8_t             solidCached;        // 固实块是否已缓存
	
} xpkStruct;

// ============================================================================
// 内部辅助函数
// ============================================================================

// xrt 数组是 1-based 的，这些宏用于转换
// XPK_LDB_GET: 用 0-based 索引获取元素 (传入 0 返回第一个元素)
// XPK_LDB_POS: 将 xrtArrayAppend 返回的 1-based 位置转为 0-based
#define XPK_LDB_GET(xpk, pos0) xrtArrayGet(&(xpk)->ldb, (pos0) + 1)
#define XPK_LDB_POS(pos1) ((pos1) - 1)
#define XPK_LDB_REMOVE(xpk, pos0, count) xrtArrayRemove(&(xpk)->ldb, (pos0) + 1, count)

// 压缩/解压路由
int xpkCompressRouter(int level, const void* src, uint32_t srcSize, 
                      void* dst, uint32_t dstCapacity, uint32_t* outSize);
int xpkDecompressRouter(int level, const void* src, uint32_t srcSize,
                        void* dst, uint32_t dstSize);

// LDB 操作
int xpkLdbLoad(xpkObject xpk);
int xpkLdbSave(xpkObject xpk);

// 文件信息操作
void* xpkInfoAlloc(xpkObject xpk);
void  xpkInfoFree(xpkObject xpk, uint32_t pos);

// 路径哈希计算
uint32_t xpkPathHashLinux(const char* path);   // 大小写敏感
uint32_t xpkPathHashWin32(const char* path);   // 不区分大小写

// 错误处理
void xpkSetError(int code, const char* msg);

// 压缩辅助函数
uint32_t xpkCompressBound(int level, uint32_t srcSize);

// 固实压缩内部函数
int xpkSolidAppendData(xpkObject xpk, const void* data, uint32_t size, int level);
int xpkSolidSave(xpkObject xpk);
void* xpkSolidExtractData(xpkObject xpk, uint32_t pos, xpkFileInfo* info, uint32_t* outSize);
int xpkSolidDecompressBlock(xpkObject xpk);
uint32_t xpkGetSolidOffset(xpkObject xpk, uint32_t pos);

#ifdef __cplusplus
}
#endif

#endif /* XPACK_INTERNAL_H */
