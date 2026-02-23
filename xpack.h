/*
 * xPack Ver7 - 文件压缩包库
 *
 * 版本: 7.0
 * 许可: MIT License
 *
 * 特性:
 *   - 四种包类型: Core, Index, Linux, Win32
 *   - 压缩级别: 0-15 (LZ4 + ZSTD + LZMA2)
 *   - 依赖: xrt 库
 *   - 64位文件支持: 单文件/整包最大 16EB
 *   - 分卷支持: 虚拟 I/O 透明跨卷读写
 *   - 高效更新: 追加写入，仅重写元数据段
 *
 * 文件结构:
 *   [xpkHead 64B] [文件数据...] [Meta] [LDB]
 *
 * 版本兼容性:
 *   - 文件头: 0x116B7078 ("xpk" + 版本号 7.0)
 *   - ver6 文件头: 0x106B7078 (版本 6.0)
 */

#ifndef XPACK_H
#define XPACK_H

#include <stdint.h>
#include <xrt.h>

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */

// ============================================================================
// 版本标识
// ============================================================================
#define XPK_VERSION         0x116B7078      // "xpk" + 版本号 (7.0 = 0x11)

// ============================================================================
// 包类型
// ============================================================================
#define XPK_TYPE_CORE       0               // Core 模式：顺序位置访问
#define XPK_TYPE_INDEX      1               // Index 模式：int64 索引访问
#define XPK_TYPE_LINUX      2               // Linux 模式：路径访问 (大小写敏感)
#define XPK_TYPE_WIN32      3               // Win32 模式：路径访问 (不区分大小写)

// ============================================================================
// 压缩算法标识
// ============================================================================
#define XPK_ALG_STORE       0               // 无压缩
#define XPK_ALG_LZ4         1               // LZ4
#define XPK_ALG_LZ4HC       2               // LZ4-HC
#define XPK_ALG_ZSTD        3               // ZSTD
#define XPK_ALG_LZMA2       4               // LZMA2

// ============================================================================
// ZSTD 策略常量
// ============================================================================
#define XPK_ZSTD_FAST       1
#define XPK_ZSTD_DFAST      2
#define XPK_ZSTD_GREEDY     3
#define XPK_ZSTD_LAZY       4
#define XPK_ZSTD_LAZY2      5
#define XPK_ZSTD_BTLAZY2    6
#define XPK_ZSTD_BTOPT      7
#define XPK_ZSTD_BTULTRA    8
#define XPK_ZSTD_BTULTRA2   9

// ============================================================================
// 文件类型标识
// ============================================================================
#define XPK_FTYPE_UNKNOWN   0
#define XPK_FTYPE_BINARY    1
#define XPK_FTYPE_TEXT      2
#define XPK_FTYPE_IMAGE     3
#define XPK_FTYPE_AUDIO     4
#define XPK_FTYPE_VIDEO     5
#define XPK_FTYPE_ARCHIVE   6
#define XPK_FTYPE_FOLDER    15

// ============================================================================
// 常量定义
// ============================================================================
#define XPK_PATH_MAX        260             // 文件路径最大长度
#define XPK_COMP_DEFAULT    7               // 默认压缩级别
#define XPK_META_COMP       6               // Meta 默认压缩级别
#define XPK_LDB_COMP        8               // LDB 默认压缩级别
#define XPK_HEAD_SIZE       64              // 包头大小

// ============================================================================
// 导出宏
// ============================================================================
#ifdef XPK_BUILD_DLL
    #define XPKAPI __declspec(dllexport)
#else
    #define XPKAPI
#endif

// ############################################################################
//                              数据结构定义
// ############################################################################

// ============================================================================
// 包信息头 (64 bytes)
// ============================================================================
#pragma pack(push, 1)
typedef struct {
    // === 基础标识 (8 bytes) ===
    uint32_t    fileHead;           // 0x00: 文件头标识 "xpk" + 版本号
    uint32_t    fileCount;          // 0x04: 文件数量 (最大 4G)

    // === 位标记1 (4 bytes) ===
    uint32_t    packType    : 2;    // 包类型: 0=Core, 1=Index, 2=Linux, 3=Win32
    uint32_t    defComp     : 4;    // 默认压缩级别 (0-15)
    uint32_t    metaComp    : 4;    // Meta 压缩级别 (0-15)
    uint32_t    ldbComp     : 4;    // LDB 压缩级别 (0-15)
    uint32_t    infoExtSize : 18;   // 文件信息扩展大小 (bytes, 最大 256KB)

    // === 位标记2 (4 bytes) ===
    uint32_t    solidMode   : 1;    // 固实压缩模式
    uint32_t    volumeMode  : 1;    // 分卷模式
    uint32_t    reserved1   : 30;   // 保留

    // === 偏移信息 (12 bytes) ===
    uint64_t    dataOffset;         // 0x10: Meta 段起始位置
    uint32_t    volumeSize;         // 0x18: 分卷大小 (0=不分卷)

    // === Meta 信息 (12 bytes) ===
    uint32_t    metaRawSize;        // 0x1C: Meta 原始大小
    uint32_t    metaCompSize;       // 0x20: Meta 压缩后大小
    uint32_t    metaHash;           // 0x24: Meta 哈希值

    // === LDB 信息 (8 bytes) ===
    uint32_t    ldbCompSize;        // 0x28: LDB 压缩后大小
    uint32_t    ldbHash;            // 0x2C: LDB 哈希值

    // === 时间戳 (16 bytes) ===
    xtime       createTime;         // 0x30: 创建时间
    xtime       changeTime;         // 0x38: 修改时间
} xpkHead;
#pragma pack(pop)

// ============================================================================
// 文件信息头 - Core 模式 (32 bytes)
// ============================================================================
#pragma pack(push, 1)
typedef struct {
    uint32_t    flag;               // 标记位 (低4位=压缩级别, 4-7位=文件类型, 8位=删除标记)
    uint32_t    fileHash;           // 文件哈希值
    uint64_t    dataOffset;         // 数据偏移位置
    uint64_t    dataSize;           // 压缩后大小
    uint64_t    fileSize;           // 原始大小
} xpkFileInfo;
#pragma pack(pop)

// ============================================================================
// 文件信息头 - Index 模式 (40 bytes)
// ============================================================================
#pragma pack(push, 1)
typedef struct {
    uint32_t    flag;               // 标记位
    uint32_t    fileHash;           // 文件哈希值
    uint64_t    dataOffset;         // 数据偏移位置
    uint64_t    dataSize;           // 压缩后大小
    uint64_t    fileSize;           // 原始大小
    int64_t     fileIndex;          // 文件索引号 (用户自定义)
} xpkFileInfoIndex;
#pragma pack(pop)

// ============================================================================
// 文件信息头 - Linux 模式 (304 bytes)
// ============================================================================
#pragma pack(push, 1)
typedef struct {
    uint32_t    flag;               // 标记位
    uint32_t    fileHash;           // 文件哈希值
    uint64_t    dataOffset;         // 数据偏移位置
    uint64_t    dataSize;           // 压缩后大小
    uint64_t    fileSize;           // 原始大小
    char        filePath[XPK_PATH_MAX]; // 文件路径 (260 bytes)
    uint32_t    fileMode;           // 文件权限模式 (chmod)
    uint64_t    createTime;         // 创建时间
    uint64_t    modifyTime;         // 修改时间
    uint64_t    accessTime;         // 访问时间
} xpkFileInfoLinux;
#pragma pack(pop)

// ============================================================================
// 文件信息头 - Win32 模式 (304 bytes)
// ============================================================================
#pragma pack(push, 1)
typedef struct {
    uint32_t    flag;               // 标记位
    uint32_t    fileHash;           // 文件哈希值
    uint64_t    dataOffset;         // 数据偏移位置
    uint64_t    dataSize;           // 压缩后大小
    uint64_t    fileSize;           // 原始大小
    char        filePath[XPK_PATH_MAX]; // 文件路径 (260 bytes)
    uint32_t    fileAttr;           // 文件属性 (系统/隐藏/只读/存档)
    uint64_t    createTime;         // 创建时间
    uint64_t    modifyTime;         // 修改时间
    uint64_t    accessTime;         // 访问时间
} xpkFileInfoWin32;
#pragma pack(pop)

// ============================================================================
// 文件信息标记位操作
// ============================================================================
#define XPK_FLAG_COMP_LEVEL(f)      ((f) & 0x0F)            // 压缩级别
#define XPK_FLAG_FILE_TYPE(f)       (((f) >> 4) & 0x0F)     // 文件类型
#define XPK_FLAG_DELETED(f)         (((f) >> 8) & 0x01)     // 删除标记

#define XPK_FLAG_SET_COMP(l)        ((l) & 0x0F)
#define XPK_FLAG_SET_TYPE(t)        (((t) & 0x0F) << 4)
#define XPK_FLAG_SET_DELETED(d)     (((d) & 0x01) << 8)

// ============================================================================
// 压缩级别映射结构
// ============================================================================
typedef struct {
    uint8_t algorithm;              // 算法类型
    uint8_t nativeLevel;            // 原生级别参数
} xpkCompMap;

// ============================================================================
// 压缩级别映射表
// ============================================================================
static const xpkCompMap xpkCompTable[16] = {
    { XPK_ALG_STORE,  0 },                  // 0:  无压缩
    { XPK_ALG_LZ4,    1 },                  // 1:  LZ4 fast
    { XPK_ALG_LZ4,    2 },                  // 2:  LZ4 fast (64KB)
    { XPK_ALG_LZ4HC,  4 },                  // 3:  LZ4-HC level 4
    { XPK_ALG_LZ4HC,  12 },                 // 4:  LZ4-HC level 12
    { XPK_ALG_ZSTD,  XPK_ZSTD_FAST },       // 5:  ZSTD fast
    { XPK_ALG_ZSTD,  XPK_ZSTD_DFAST },      // 6:  ZSTD dfast
    { XPK_ALG_ZSTD,  XPK_ZSTD_GREEDY },     // 7:  ZSTD greedy [DEFAULT]
    { XPK_ALG_ZSTD,  XPK_ZSTD_LAZY },       // 8:  ZSTD lazy
    { XPK_ALG_ZSTD,  XPK_ZSTD_LAZY2 },      // 9:  ZSTD lazy2
    { XPK_ALG_ZSTD,  XPK_ZSTD_BTLAZY2 },    // 10: ZSTD btlazy2
    { XPK_ALG_ZSTD,  XPK_ZSTD_BTOPT },      // 11: ZSTD btopt
    { XPK_ALG_ZSTD,  XPK_ZSTD_BTULTRA },    // 12: ZSTD btultra
    { XPK_ALG_ZSTD,  XPK_ZSTD_BTULTRA2 },   // 13: ZSTD btultra2
    { XPK_ALG_LZMA2, 6 },                   // 14: LZMA2 level 6
    { XPK_ALG_LZMA2, 9 },                   // 15: LZMA2 level 9
};

// ============================================================================
// 错误回调函数类型
// ============================================================================
typedef void (*xpkErrorProc)(int code, const char* message);

// ============================================================================
// 遍历回调函数类型
// ============================================================================
typedef int (*xpkEachCallback)(void* xpk, uint32_t pos, void* info, void* userData);

// ============================================================================
// 统计信息结构
// ============================================================================
typedef struct {
    uint32_t    fileCount;         // 文件数量
    uint64_t    totalSize;         // 原始总大小
    uint64_t    packedSize;        // 压缩后总大小
    double      ratio;             // 压缩比
} xpkStat;

// ============================================================================
// 分卷统计信息结构
// ============================================================================
typedef struct {
    int         volumeCount;       // 分卷总数
    uint64_t    totalSize;         // 总大小 (所有卷)
    uint64_t    dataOffset;        // 数据偏移 (Meta 起始)
    uint64_t    volumeSize;        // 单卷大小
} xpkVolumeStat;

// ============================================================================
// xPack 对象 (不透明类型)
// ============================================================================
typedef struct xpkStruct* xpkObject;

/* clang-format on */

// ############################################################################
//                              API 接口定义
// ############################################################################

// ============================================================================
// 生命周期管理
// ============================================================================
XPKAPI xpkObject    xpkOpen(const char* path, uint64_t offset, int readonly);
XPKAPI int          xpkSave(xpkObject xpk);
XPKAPI void         xpkClose(xpkObject xpk);

// ============================================================================
// 包属性操作
// ============================================================================
XPKAPI int          xpkType(xpkObject xpk);
XPKAPI int          xpkTypeSet(xpkObject xpk, int type);
XPKAPI uint32_t     xpkCount(xpkObject xpk);
XPKAPI xpkHead*     xpkGetHead(xpkObject xpk);

// ============================================================================
// 包元数据操作
// ============================================================================
XPKAPI void*        xpkMetaGet(xpkObject xpk, uint32_t* outSize);
XPKAPI int          xpkMetaSet(xpkObject xpk, const void* data, uint32_t size, int level);

// ============================================================================
// 固实压缩控制
// ============================================================================
XPKAPI int          xpkSolidMode(xpkObject xpk);
XPKAPI int          xpkSolidModeSet(xpkObject xpk, int enabled);

// ============================================================================
// 分卷控制
// ============================================================================
XPKAPI int          xpkVolumeMode(xpkObject xpk);
XPKAPI int          xpkVolumeModeSet(xpkObject xpk, int enabled);
XPKAPI uint32_t     xpkVolumeSize(xpkObject xpk);
XPKAPI int          xpkVolumeSizeSet(xpkObject xpk, uint32_t size);
XPKAPI int          xpkVolumeCount(xpkObject xpk);
XPKAPI const char*  xpkVolumePath(xpkObject xpk, int index);
XPKAPI int          xpkVolumeStatGet(xpkObject xpk, xpkVolumeStat* stat);

// ============================================================================
// 文件操作 - Core 模式 (按位置)
// ============================================================================
XPKAPI uint32_t     xpkAppendFile(xpkObject xpk, const char* path, int level);
XPKAPI uint32_t     xpkAppendData(xpkObject xpk, const void* data, uint64_t size, int level);
XPKAPI int          xpkExtractFile(xpkObject xpk, uint32_t pos, const char* path);
XPKAPI void*        xpkExtractData(xpkObject xpk, uint32_t pos, uint64_t* outSize);
XPKAPI int          xpkUpdateFile(xpkObject xpk, uint32_t pos, const char* path, int level);
XPKAPI int          xpkUpdateData(xpkObject xpk, uint32_t pos, const void* data, uint64_t size, int level);
XPKAPI int          xpkRemove(xpkObject xpk, uint32_t pos);

// ============================================================================
// 文件信息获取
// ============================================================================
XPKAPI void*        xpkInfo(xpkObject xpk, uint32_t pos);
XPKAPI uint64_t     xpkInfoSize(xpkObject xpk, uint32_t pos);
XPKAPI uint64_t     xpkInfoPacked(xpkObject xpk, uint32_t pos);
XPKAPI uint32_t     xpkInfoHash(xpkObject xpk, uint32_t pos);
XPKAPI int          xpkInfoLevel(xpkObject xpk, uint32_t pos);
XPKAPI int          xpkInfoType(xpkObject xpk, uint32_t pos);
XPKAPI int          xpkInfoTypeSet(xpkObject xpk, uint32_t pos, int type);
XPKAPI int          xpkInfoDeleted(xpkObject xpk, uint32_t pos);

// ============================================================================
// Index 模式专用接口
// ============================================================================
XPKAPI uint32_t             xpkIndexFind(xpkObject xpk, int64_t index);
XPKAPI xpkFileInfoIndex*    xpkIndexAppendFile(xpkObject xpk, int64_t index, const char* path, int level);
XPKAPI xpkFileInfoIndex*    xpkIndexAppendData(xpkObject xpk, int64_t index, const void* data, uint64_t size, int level);
XPKAPI int                  xpkIndexExtractFile(xpkObject xpk, int64_t index, const char* path);
XPKAPI void*                xpkIndexExtractData(xpkObject xpk, int64_t index, uint64_t* outSize);
XPKAPI int                  xpkIndexUpdateFile(xpkObject xpk, int64_t index, const char* path, int level);
XPKAPI int                  xpkIndexUpdateData(xpkObject xpk, int64_t index, const void* data, uint64_t size, int level);
XPKAPI int                  xpkIndexRemove(xpkObject xpk, int64_t index);

// ============================================================================
// 路径模式专用接口 (Linux/Win32)
// ============================================================================
XPKAPI uint32_t     xpkPathFind(xpkObject xpk, const char* filePath);
XPKAPI int          xpkPathExists(xpkObject xpk, const char* filePath);
XPKAPI void*        xpkPathAppendFile(xpkObject xpk, const char* filePath, const char* srcPath, int level);
XPKAPI uint32_t     xpkPathAppendData(xpkObject xpk, const char* filePath, const void* data, uint64_t size, int level);
XPKAPI int          xpkPathExtractFile(xpkObject xpk, const char* filePath, const char* dstPath);
XPKAPI void*        xpkPathExtractData(xpkObject xpk, const char* filePath, uint64_t* outSize);
XPKAPI int          xpkPathUpdateFile(xpkObject xpk, const char* filePath, const char* srcPath, int level);
XPKAPI int          xpkPathUpdateData(xpkObject xpk, const char* filePath, const void* data, uint64_t size, int level);
XPKAPI int          xpkPathRemove(xpkObject xpk, const char* filePath);
XPKAPI const char*  xpkPathGet(xpkObject xpk, uint32_t pos);

// ============================================================================
// 遍历接口
// ============================================================================
XPKAPI int          xpkEach(xpkObject xpk, xpkEachCallback callback, void* userData);
XPKAPI int          xpkEachMatch(xpkObject xpk, const char* pattern, xpkEachCallback callback, void* userData);

// ============================================================================
// 批量操作
// ============================================================================
XPKAPI int          xpkExtractAll(xpkObject xpk, const char* dir);
XPKAPI int          xpkAppendDir(xpkObject xpk, const char* dir, const char* pattern, int level, int recursive);

// ============================================================================
// 工具函数
// ============================================================================
XPKAPI void         xpkFree(void* ptr);
XPKAPI uint32_t     xpkHash(const void* data, uint64_t size);
XPKAPI int          xpkVerify(xpkObject xpk, uint32_t pos);
XPKAPI int          xpkVerifyAll(xpkObject xpk);
XPKAPI int          xpkStatGet(xpkObject xpk, xpkStat* stat);
XPKAPI int          xpkRebuild(xpkObject xpk);
XPKAPI int          xpkLastError(void);
XPKAPI const char*  xpkLastErrorMsg(void);

// ============================================================================
// 原始读写（分卷虚拟 I/O 层）
// ============================================================================
XPKAPI int      xpkRawRead(xpkObject xpk, uint64_t offset, uint64_t size, void* data);
XPKAPI int      xpkRawWrite(xpkObject xpk, uint64_t offset, uint64_t size, const void* data);

// ============================================================================
// 测试接口（内部测试用）
// ============================================================================
XPKAPI uint64_t     xpkTestCompressBound(int level, uint64_t srcSize);
XPKAPI int          xpkTestCompress(int level, const void* src, uint64_t srcSize,
                                    void* dst, uint64_t dstCapacity, 
                                    uint64_t* outSize, uint32_t* outHash);
XPKAPI int          xpkTestDecompress(int level, const void* src, uint64_t srcSize,
                                       void* dst, uint64_t dstSize, uint32_t expectedHash);

#ifdef __cplusplus
}
#endif

#endif /* XPACK_H */
