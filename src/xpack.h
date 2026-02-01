/*
 * xPack Ver7 - 文件压缩包库
 *
 * 版本: 7.0
 * 许可: MIT License
 *
 * 特性:
 *   - 四种包类型: Core, Index, Linux, Win32
 *   - 压缩级别: 0-15 (LZ4 + ZSTD)
 *   - 依赖: xrt 库
 *
 * 版本兼容性:
 *   - 文件头前4字节为 "xpk" + 版本号 (0x116B7078 = 0x706B7078 | 0x11000000)
 *   - 此格式与 ver6 兼容，旧版本可以通过检查文件头识别版本
 *   - ver6 文件头: 0x106B7078 (版本 6.0)
 *   - ver7 文件头: 0x116B7078 (版本 7.0)
 */

#ifndef XPACK_H
#define XPACK_H

#include <stdint.h>
#include <xrt/xrt.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// 版本标识
// ============================================================================
#define XPK_VERSION         0x116B7078      // "xpk" + 版本号 (7.0 = 0x11)
                                            // 版本号存储在文件头前4字节，与 ver6 兼容

// ============================================================================
// 包类型
// ============================================================================
#define XPK_TYPE_CORE       0               // Core 模式：顺序位置访问
#define XPK_TYPE_INDEX      1               // Index 模式：整数索引访问
#define XPK_TYPE_LINUX      2               // Linux 模式：路径访问(大小写敏感)
#define XPK_TYPE_WIN32      3               // Win32 模式：路径访问(不区分大小写)

// ============================================================================
// 压缩算法标识（内部使用）
// ============================================================================
#define XPK_ALG_STORE       0               // 无压缩
#define XPK_ALG_LZ4         1               // LZ4
#define XPK_ALG_LZ4HC       2               // LZ4-HC
#define XPK_ALG_ZSTD        3               // ZSTD
#define XPK_ALG_LZMA2       4               // LZMA2

// ============================================================================
// ZSTD 策略常量（与 ZSTD_strategy 枚举对应）
// ============================================================================
#define XPK_ZSTD_FAST       1               // ZSTD_fast
#define XPK_ZSTD_DFAST      2               // ZSTD_dfast
#define XPK_ZSTD_GREEDY     3               // ZSTD_greedy
#define XPK_ZSTD_LAZY       4               // ZSTD_lazy
#define XPK_ZSTD_LAZY2      5               // ZSTD_lazy2
#define XPK_ZSTD_BTLAZY2    6               // ZSTD_btlazy2
#define XPK_ZSTD_BTOPT      7               // ZSTD_btopt
#define XPK_ZSTD_BTULTRA    8               // ZSTD_btultra
#define XPK_ZSTD_BTULTRA2   9               // ZSTD_btultra2

// ============================================================================
// 文件类型标识（可选，用户自定义）
// ============================================================================
#define XPK_FTYPE_UNKNOWN   0               // 未知/通用
#define XPK_FTYPE_BINARY    1               // 二进制数据
#define XPK_FTYPE_TEXT      2               // 文本文件
#define XPK_FTYPE_IMAGE     3               // 图像文件
#define XPK_FTYPE_AUDIO     4               // 音频文件
#define XPK_FTYPE_VIDEO     5               // 视频文件
#define XPK_FTYPE_ARCHIVE   6               // 归档文件
#define XPK_FTYPE_FOLDER    15              // 目录标记

// ============================================================================
// 路径最大长度
// ============================================================================
#define XPK_PATH_MAX        200             // 文件路径最大长度

// ============================================================================
// 默认值
// ============================================================================
#define XPK_COMP_DEFAULT    7               // 默认压缩级别 (ZSTD greedy)
#define XPK_LDB_COMP        8               // LDB 默认压缩级别

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
// 包标记位域 (32 bit)
// ============================================================================
typedef union {
    uint32_t value;
    struct {
        uint32_t packType   : 4;        // [0-3]   包类型 (0-3)
        uint32_t ldbComp    : 4;        // [4-7]   LDB 压缩级别 (0-15)
        uint32_t solidMode  : 1;        // [8]     固实压缩模式 (0=独立,1=固实)
        uint32_t volumeMode : 1;        // [9]     分卷模式 (0=单卷,1=多卷)
        uint32_t splitMode  : 2;        // [10-11] 分割模式 (0=字节,1=文件)
        uint32_t reserved   : 20;       // [12-31] 保留
    };
} xpkFlag;

// ============================================================================
// 包信息头 (60 bytes)
// ============================================================================
#pragma pack(push, 1)
typedef struct {
    // 基础标识 (4 bytes)
    uint32_t        fileHead;           // [0-3]   文件头标识 "xpk" + 版本号

    // 包标记 (4 bytes)
    xpkFlag         flag;               // [4-7]   包标记位域

    // 文件信息 (12 bytes)
    uint32_t        fileCount;          // [8-11]  文件数量
    uint32_t        headExtSize;        // [12-15] 包头扩展数据大小
    uint32_t        discCode;           // [16-19] 识别代码(用户自定义)

    // 文件信息扩展 (8 bytes)
    uint32_t        infoExtSize;        // [20-23] 文件信息扩展大小
    uint32_t        reserved;           // [24-27] 保留

    // LDB 信息 (16 bytes)
    uint32_t        ldbOffset;          // [28-31] LDB 偏移位置
    uint32_t        ldbSize;            // [32-35] LDB 压缩后大小
    uint32_t        ldbRawSize;         // [36-39] LDB 原始大小
    uint32_t        ldbHash;            // [40-43] LDB 哈希值

    // 时间戳 (16 bytes)
	xtime           createTime;         // [44-51] 创建时间
	xtime           modifyTime;         // [52-59] 修改时间

} xpkHead;
#pragma pack(pop)

// ============================================================================
// 文件标记位域 (32 bit)
// ============================================================================
typedef union {
    uint32_t value;
    struct {
        uint32_t compLevel  : 4;        // [0-3]   压缩级别 (0-15)
        uint32_t fileType   : 4;        // [4-7]   文件类型 (0-15)
        uint32_t encrypted  : 1;        // [8]     加密标记 (保留)
        uint32_t reserved   : 23;       // [9-31]  保留
    };
} xpkFileFlag;

// ============================================================================
// 文件信息头 - Core 模式 (20 bytes)
// ============================================================================
#pragma pack(push, 1)
typedef struct {
    uint32_t        dataOffset;         // [0-3]   数据偏移位置
    uint32_t        dataSize;           // [4-7]   压缩后大小
    uint32_t        fileSize;           // [8-11]  原始大小
    uint32_t        fileHash;           // [12-15] 文件哈希值
    xpkFileFlag     flag;               // [16-19] 文件标记位域
} xpkFileInfo;
#pragma pack(pop)

// ============================================================================
// 文件信息头 - Index 模式 (28 bytes)
// ============================================================================
#pragma pack(push, 1)
typedef struct {
    // 基础信息 (20 bytes)
    uint32_t        dataOffset;         // [0-3]   数据偏移位置
    uint32_t        dataSize;           // [4-7]   压缩后大小
    uint32_t        fileSize;           // [8-11]  原始大小
    uint32_t        fileHash;           // [12-15] 文件哈希值
    xpkFileFlag     flag;               // [16-19] 文件标记位域
    
    // 扩展信息 (8 bytes)
    int32_t         fileIndex;          // [20-23] 文件索引号
    int32_t         userData;           // [24-27] 用户自定义数据
} xpkFileInfoIndex;
#pragma pack(pop)

// ============================================================================
// 文件信息头 - Linux 模式 (232 bytes)
// ============================================================================
#pragma pack(push, 1)
typedef struct {
    // 基础信息 (20 bytes)
    uint32_t        dataOffset;         // [0-3]   数据偏移位置
    uint32_t        dataSize;           // [4-7]   压缩后大小
    uint32_t        fileSize;           // [8-11]  原始大小
    uint32_t        fileHash;           // [12-15] 文件哈希值
    xpkFileFlag     flag;               // [16-19] 文件标记位域
    
    // 路径信息 (208 bytes)
    char            filePath[XPK_PATH_MAX];  // [20-219]  文件路径
    uint32_t        pathHash;           // [220-223] 路径哈希值(大小写敏感)
    
    // 文件属性 (8 bytes)
    uint32_t        fileAttr;           // [224-227] 文件属性(权限等)
    uint32_t        modifyTime;         // [228-231] 修改时间
} xpkFileInfoLinux;
#pragma pack(pop)

// ============================================================================
// 文件信息头 - Win32 模式 (236 bytes)
// ============================================================================
#pragma pack(push, 1)
typedef struct {
	// 基础信息 (20 bytes)
	uint32_t        dataOffset;         // [0-3]   数据偏移位置
	uint32_t        dataSize;           // [4-7]   压缩后大小
	uint32_t        fileSize;           // [8-11]  原始大小
	uint32_t        fileHash;           // [12-15] 文件哈希值
	xpkFileFlag     flag;               // [16-19] 文件标记位域
	
	// 路径信息 (208 bytes)
	char            filePath[XPK_PATH_MAX];  // [20-219]  文件路径
	uint32_t        pathHash;           // [220-223] 路径哈希值(转小写)
	
	// 文件属性 (12 bytes)
	uint32_t        fileAttr;           // [224-227] 文件属性
	uint32_t        createTime;         // [228-231] 创建时间
	uint32_t        modifyTime;         // [232-235] 修改时间
} xpkFileInfoWin32;
#pragma pack(pop)

// ============================================================================
// 固实文件信息扩展 (固实压缩模式下使用)
// ============================================================================
#pragma pack(push, 1)
typedef struct {
	uint32_t        dataOffsetInBlock;  // [0-3]   在固实块中的数据偏移
} xpkFileInfoSolid;
#pragma pack(pop)

// ============================================================================
// 分卷信息头 (8 bytes) - 存储在 xpkHead.headExt
// ============================================================================
#pragma pack(push, 1)
typedef struct {
    uint32_t        volumeCount;        // [0-3]   分卷总数 (1=不分卷)
    uint32_t        volumeIndex;       // [4-7]   当前卷索引 (0-based)
} xpkVolumeInfo;
#pragma pack(pop)

// ============================================================================
// 压缩级别映射结构
// ============================================================================
typedef struct {
    uint8_t algorithm;                  // 算法类型
    uint8_t nativeLevel;                // 原生级别参数
} xpkCompMap;

// ============================================================================
// 压缩级别映射表
// ============================================================================
static const xpkCompMap xpkCompTable[16] = {
    { XPK_ALG_STORE,  0 },                  // 0:  无压缩
    { XPK_ALG_LZ4,    1 },                  // 1:  LZ4 fast
    { XPK_ALG_LZ4,    2 },                  // 2:  LZ4 fast (64KB)
    { XPK_ALG_LZ4HC,  4 },                  // 3:  LZ4-HC level 4
    { XPK_ALG_LZ4HC, 12 },                  // 4:  LZ4-HC level 12
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
    uint32_t fileCount;                 // 文件数量
    uint64_t totalSize;                 // 原始总大小
    uint64_t packedSize;                // 压缩后总大小
    double   ratio;                     // 压缩比
} xpkStat;

// ============================================================================
// 分卷统计信息结构
// ============================================================================
typedef struct {
    int             volumeCount;         // 分卷总数
    uint32_t*       volumeSizes;        // 各卷大小数组 (需调用者释放)
    uint64_t        totalSize;          // 总大小（所有卷）
    uint64_t        totalDataSize;      // 数据总大小
    double          avgSize;           // 平均卷大小
} xpkVolumeStat;

// ============================================================================
// xPack 对象（不透明类型）
// ============================================================================
typedef struct xpkStruct* xpkObject;


// ############################################################################
//                              API 接口定义
// ############################################################################

// ============================================================================
// 生命周期管理
// ============================================================================
XPKAPI xpkObject    xpkOpen(const char* path, uint32_t offset, int readonly);
XPKAPI int          xpkSave(xpkObject xpk);
XPKAPI void         xpkClose(xpkObject xpk);

// ============================================================================
// 包属性操作
// ============================================================================
XPKAPI int          xpkType(xpkObject xpk);
XPKAPI int          xpkTypeSet(xpkObject xpk, int type);
XPKAPI uint32_t     xpkCount(xpkObject xpk);
XPKAPI uint32_t     xpkDiscCode(xpkObject xpk);
XPKAPI int          xpkDiscCodeSet(xpkObject xpk, uint32_t code);
XPKAPI void         xpkOnError(xpkObject xpk, xpkErrorProc callback);
XPKAPI xpkHead*     xpkGetHead(xpkObject xpk);

// ============================================================================
// 固实压缩控制接口
// ============================================================================
XPKAPI int          xpkSolidMode(xpkObject xpk);
XPKAPI int          xpkSolidModeSet(xpkObject xpk, int enabled);
XPKAPI int          xpkSolidBlockInfo(xpkObject xpk, uint32_t* offset, uint32_t* size);

// ============================================================================
// 分卷控制接口
// ============================================================================
XPKAPI int          xpkVolumeMode(xpkObject xpk);
XPKAPI int          xpkVolumeModeSet(xpkObject xpk, int enabled);
XPKAPI int          xpkVolumeSize(xpkObject xpk);
XPKAPI int          xpkVolumeSizeSet(xpkObject xpk, uint32_t size);
XPKAPI int          xpkVolumeCount(xpkObject xpk);
XPKAPI int          xpkVolumeCurrent(xpkObject xpk);
XPKAPI int          xpkVolumeSplitMode(xpkObject xpk);
XPKAPI int          xpkVolumeSplitModeSet(xpkObject xpk, int mode);
XPKAPI const char*  xpkVolumePath(xpkObject xpk, int index);
XPKAPI int          xpkVolumeStatGet(xpkObject xpk, xpkVolumeStat* stat);

// ============================================================================
// 文件操作 - Core 模式（按位置）
// ============================================================================
XPKAPI uint32_t     xpkAppendFile(xpkObject xpk, const char* path, int level);
XPKAPI uint32_t     xpkAppendData(xpkObject xpk, const void* data, uint32_t size, int level);
XPKAPI int          xpkExtractFile(xpkObject xpk, uint32_t pos, const char* path);
XPKAPI void*        xpkExtractData(xpkObject xpk, uint32_t pos, uint32_t* outSize);
XPKAPI int          xpkUpdateFile(xpkObject xpk, uint32_t pos, const char* path, int level);
XPKAPI int          xpkUpdateData(xpkObject xpk, uint32_t pos, const void* data, uint32_t size, int level);
XPKAPI int          xpkRemove(xpkObject xpk, uint32_t pos);

// ============================================================================
// 文件信息获取
// ============================================================================
XPKAPI void*        xpkInfo(xpkObject xpk, uint32_t pos);
XPKAPI uint32_t     xpkInfoSize(xpkObject xpk, uint32_t pos);
XPKAPI uint32_t     xpkInfoPacked(xpkObject xpk, uint32_t pos);
XPKAPI uint32_t     xpkInfoHash(xpkObject xpk, uint32_t pos);
XPKAPI int          xpkInfoLevel(xpkObject xpk, uint32_t pos);
XPKAPI int          xpkInfoType(xpkObject xpk, uint32_t pos);
XPKAPI int          xpkInfoTypeSet(xpkObject xpk, uint32_t pos, int type);

// ============================================================================
// Index 模式专用接口
// ============================================================================
XPKAPI uint32_t             xpkIndexFind(xpkObject xpk, int32_t index);
XPKAPI xpkFileInfoIndex*    xpkIndexAppendFile(xpkObject xpk, int32_t index, const char* path, int level);
XPKAPI xpkFileInfoIndex*    xpkIndexAppendData(xpkObject xpk, int32_t index, const void* data, uint32_t size, int level);
XPKAPI int                  xpkIndexExtractFile(xpkObject xpk, int32_t index, const char* path);
XPKAPI void*                xpkIndexExtractData(xpkObject xpk, int32_t index, uint32_t* outSize);
XPKAPI int                  xpkIndexUpdateFile(xpkObject xpk, int32_t index, const char* path, int level);
XPKAPI int                  xpkIndexUpdateData(xpkObject xpk, int32_t index, const void* data, uint32_t size, int level);
XPKAPI int                  xpkIndexRemove(xpkObject xpk, int32_t index);
XPKAPI int32_t              xpkIndexUserData(xpkObject xpk, int32_t index);
XPKAPI int                  xpkIndexUserDataSet(xpkObject xpk, int32_t index, int32_t value);

// ============================================================================
// 路径模式专用接口 (Linux/Win32)
// ============================================================================
XPKAPI uint32_t     xpkPathFind(xpkObject xpk, const char* filePath);
XPKAPI int          xpkPathExists(xpkObject xpk, const char* filePath);
XPKAPI void*        xpkPathAppendFile(xpkObject xpk, const char* filePath, const char* srcPath, int level);
XPKAPI uint32_t     xpkPathAppendData(xpkObject xpk, const char* filePath, const void* data, uint32_t size, int level);
XPKAPI int          xpkPathExtractFile(xpkObject xpk, const char* filePath, const char* dstPath);
XPKAPI void*        xpkPathExtractData(xpkObject xpk, const char* filePath, uint32_t* outSize);
XPKAPI int          xpkPathUpdateFile(xpkObject xpk, const char* filePath, const char* srcPath, int level);
XPKAPI int          xpkPathUpdateData(xpkObject xpk, const char* filePath, const void* data, uint32_t size, int level);
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
XPKAPI uint32_t     xpkHash(const void* data, uint32_t size);
XPKAPI int          xpkVerify(xpkObject xpk, uint32_t pos);
XPKAPI int          xpkVerifyAll(xpkObject xpk);
XPKAPI int          xpkStatGet(xpkObject xpk, xpkStat* stat);
XPKAPI int          xpkRebuild(xpkObject xpk);
XPKAPI int          xpkLastError(void);
XPKAPI const char*  xpkLastErrorMsg(void);

#ifdef __cplusplus
}
#endif

#endif /* XPACK_H */
