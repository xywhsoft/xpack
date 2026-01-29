# xPack Ver7 技术规范

> 版本: 7.0  
> 日期: 2026-01-29  
> 状态: 设计阶段

---

## 1. 概述

### 1.1 项目简介

xPack 是一个轻量级文件压缩包库，提供高效的文件打包、压缩、解压功能。Ver7 是第七个主要版本，在 Ver6 基础上进行了重大架构升级。

### 1.2 设计目标

- **多级压缩方案**：LZ4/ZSTD 为主力，LZMA2 作为最高压缩级别
- **压缩级别体系**：0-15 级，保证严格单调性
- **统一依赖库**：整合使用 xrt 库
- **位域结构**：使用位域替代 MASK 掩码运算
- **驼峰命名**：API 采用 `xpk` 前缀 + 驼峰命名

### 1.3 兼容性

- 文件格式签名：`xpk\0` (0x006B7078)
- 版本号：7
- 不兼容 Ver6 及更早版本的包格式

---

## 2. 压缩级别规范

### 2.1 级别定义

| 级别 | 算法 | 原生参数 | 压缩比 | 压缩速度 | 解压速度 |
|:----:|------|----------|:------:|:--------:|:--------:|
| 0 | 无压缩 | - | 1.00 | ∞ | ∞ |
| 1 | LZ4 | fast | ~2.00 | 800 MB/s | 4500 MB/s |
| 2 | LZ4 | fast (64KB) | ~2.10 | 750 MB/s | 4500 MB/s |
| 3 | LZ4-HC | level 4 | ~2.45 | 120 MB/s | 4500 MB/s |
| 4 | LZ4-HC | level 12 | ~2.72 | 40 MB/s | 4500 MB/s |
| 5 | ZSTD | fast | ~2.88 | 500 MB/s | 1400 MB/s |
| 6 | ZSTD | dfast | ~2.95 | 400 MB/s | 1380 MB/s |
| **7** | **ZSTD** | **greedy** | **~3.08** | **250 MB/s** | **1350 MB/s** |
| 8 | ZSTD | lazy | ~3.15 | 130 MB/s | 1300 MB/s |
| 9 | ZSTD | lazy2 | ~3.22 | 70 MB/s | 1260 MB/s |
| 10 | ZSTD | btlazy2 | ~3.28 | 45 MB/s | 1220 MB/s |
| 11 | ZSTD | btopt | ~3.35 | 18 MB/s | 1140 MB/s |
| 12 | ZSTD | btultra | ~3.43 | 10 MB/s | 1100 MB/s |
| 13 | ZSTD | btultra2 | ~3.50 | 4 MB/s | 1000 MB/s |
| 14 | LZMA2 | level 6 | ~3.70 | 3 MB/s | 150 MB/s |
| 15 | LZMA2 | level 9 | ~3.90 | 1.5 MB/s | 150 MB/s |

### 2.2 单调性约束

**强制要求**：级别 N+1 的压缩比必须大于级别 N，速度必须小于级别 N。

### 2.3 算法标识

```c
#define XPK_ALG_STORE   0   // 无压缩
#define XPK_ALG_LZ4     1   // LZ4
#define XPK_ALG_LZ4HC   2   // LZ4-HC
#define XPK_ALG_ZSTD    3   // ZSTD
#define XPK_ALG_LZMA2   4   // LZMA2
```

### 2.4 默认值

```c
#define XPK_COMP_DEFAULT    7   // 默认压缩级别 (ZSTD greedy)
#define XPK_LDB_COMP        8   // LDB 默认压缩级别
```

---

## 3. 包类型规范

### 3.1 类型定义

| 类型 | 值 | 说明 | 文件信息大小 |
|------|:--:|------|:------------:|
| Core | 0 | 顺序位置访问 | 20 bytes |
| Index | 1 | 整数索引访问 | 28 bytes |
| Linux | 2 | 路径访问（大小写敏感） | 232 bytes |
| Win32 | 3 | 路径访问（不区分大小写） | 236 bytes |

### 3.2 类型常量

```c
#define XPK_TYPE_CORE       0
#define XPK_TYPE_INDEX      1
#define XPK_TYPE_LINUX      2
#define XPK_TYPE_WIN32      3
```

### 3.3 路径规范

```c
#define XPK_PATH_MAX        200   // 文件路径最大长度
```

---

## 4. 文件格式规范

### 4.1 文件布局

```
┌────────────────────────┐  偏移 0
│      包信息头          │  48 bytes
├────────────────────────┤  偏移 48
│    包头扩展数据        │  可选，headExtSize bytes
├────────────────────────┤
│                        │
│      文件数据区        │  压缩后的文件内容
│                        │
├────────────────────────┤  偏移 ldbOffset
│      LDB 数据块        │  压缩后的文件信息列表
└────────────────────────┘
```

### 4.2 包信息头 (48 bytes)

| 偏移 | 大小 | 字段 | 说明 |
|:----:|:----:|------|------|
| 0 | 4 | signature | 文件签名 "xpk\0" (0x006B7078) |
| 4 | 1 | version | 版本号 (7) |
| 5 | 1 | reserved0 | 保留 |
| 6 | 2 | infoExtSize | 文件信息扩展大小 |
| 8 | 4 | flag | 包标记位域 |
| 12 | 4 | fileCount | 文件数量 |
| 16 | 4 | headExtSize | 包头扩展数据大小 |
| 20 | 4 | discCode | 识别代码 |
| 24 | 4 | ldbOffset | LDB 偏移位置 |
| 28 | 4 | ldbSize | LDB 压缩后大小 |
| 32 | 4 | ldbRawSize | LDB 原始大小 |
| 36 | 4 | ldbHash | LDB 哈希值 |
| 40 | 4 | createTime | 创建时间 |
| 44 | 4 | modifyTime | 修改时间 |

### 4.3 包标记位域

```c
typedef union {
    uint32_t value;
    struct {
        uint32_t packType   : 4;    // [0-3]   包类型
        uint32_t ldbComp    : 4;    // [4-7]   LDB 压缩级别
        uint32_t reserved1  : 8;    // [8-15]  保留
        uint32_t reserved2  : 16;   // [16-31] 保留
    };
} xpkFlag;
```

### 4.4 文件标记位域

```c
typedef union {
    uint32_t value;
    struct {
        uint32_t compLevel  : 4;    // [0-3]   压缩级别
        uint32_t fileType   : 4;    // [4-7]   文件类型
        uint32_t encrypted  : 1;    // [8]     加密标记
        uint32_t reserved   : 23;   // [9-31]  保留
    };
} xpkFileFlag;
```

---

## 5. 数据结构规范

### 5.1 文件信息 - Core (20 bytes)

| 偏移 | 大小 | 字段 | 说明 |
|:----:|:----:|------|------|
| 0 | 4 | dataOffset | 数据偏移位置 |
| 4 | 4 | dataSize | 压缩后大小 |
| 8 | 4 | fileSize | 原始大小 |
| 12 | 4 | fileHash | 文件哈希值 |
| 16 | 4 | flag | 文件标记位域 |

### 5.2 文件信息 - Index (28 bytes)

| 偏移 | 大小 | 字段 | 说明 |
|:----:|:----:|------|------|
| 0-19 | 20 | (Core) | 基础信息 |
| 20 | 4 | fileIndex | 文件索引号 |
| 24 | 4 | userData | 用户自定义数据 |

### 5.3 文件信息 - Linux (232 bytes)

| 偏移 | 大小 | 字段 | 说明 |
|:----:|:----:|------|------|
| 0-19 | 20 | (Core) | 基础信息 |
| 20 | 200 | filePath | 文件路径 |
| 220 | 4 | pathHash | 路径哈希值（大小写敏感） |
| 224 | 4 | fileAttr | 文件属性 |
| 228 | 4 | modifyTime | 修改时间 |

### 5.4 文件信息 - Win32 (236 bytes)

| 偏移 | 大小 | 字段 | 说明 |
|:----:|:----:|------|------|
| 0-19 | 20 | (Core) | 基础信息 |
| 20 | 200 | filePath | 文件路径 |
| 220 | 4 | pathHash | 路径哈希值（转小写） |
| 224 | 4 | fileAttr | 文件属性 |
| 228 | 4 | createTime | 创建时间 |
| 232 | 4 | modifyTime | 修改时间 |

---

## 6. API 规范

### 6.1 命名规范

- **前缀**：`xpk`
- **风格**：小驼峰 (lowerCamelCase)
- **示例**：`xpkOpen`, `xpkAppendFile`, `xpkPathExtractData`

### 6.2 生命周期

```c
xpkObject   xpkOpen(const char* path, uint32_t offset, int readonly);
int         xpkSave(xpkObject xpk);
void        xpkClose(xpkObject xpk);
```

### 6.3 包属性

```c
int         xpkType(xpkObject xpk);
int         xpkTypeSet(xpkObject xpk, int type);
uint32_t    xpkCount(xpkObject xpk);
uint32_t    xpkDiscCode(xpkObject xpk);
int         xpkDiscCodeSet(xpkObject xpk, uint32_t code);
xpkHead*    xpkGetHead(xpkObject xpk);
```

### 6.4 Core 模式

```c
uint32_t    xpkAppendFile(xpkObject xpk, const char* path, int level);
uint32_t    xpkAppendData(xpkObject xpk, const void* data, uint32_t size, int level);
int         xpkExtractFile(xpkObject xpk, uint32_t pos, const char* path);
void*       xpkExtractData(xpkObject xpk, uint32_t pos, uint32_t* outSize);
int         xpkUpdateFile(xpkObject xpk, uint32_t pos, const char* path, int level);
int         xpkUpdateData(xpkObject xpk, uint32_t pos, const void* data, uint32_t size, int level);
int         xpkRemove(xpkObject xpk, uint32_t pos);
```

### 6.5 文件信息

```c
void*       xpkInfo(xpkObject xpk, uint32_t pos);
uint32_t    xpkInfoSize(xpkObject xpk, uint32_t pos);
uint32_t    xpkInfoPacked(xpkObject xpk, uint32_t pos);
uint32_t    xpkInfoHash(xpkObject xpk, uint32_t pos);
int         xpkInfoLevel(xpkObject xpk, uint32_t pos);
int         xpkInfoType(xpkObject xpk, uint32_t pos);
```

### 6.6 Index 模式

```c
uint32_t            xpkIndexFind(xpkObject xpk, int32_t index);
xpkFileInfoIndex*   xpkIndexAppendFile(xpkObject xpk, int32_t index, const char* path, int level);
xpkFileInfoIndex*   xpkIndexAppendData(xpkObject xpk, int32_t index, const void* data, uint32_t size, int level);
int                 xpkIndexExtractFile(xpkObject xpk, int32_t index, const char* path);
void*               xpkIndexExtractData(xpkObject xpk, int32_t index, uint32_t* outSize);
int                 xpkIndexRemove(xpkObject xpk, int32_t index);
```

### 6.7 路径模式

```c
uint32_t    xpkPathFind(xpkObject xpk, const char* filePath);
int         xpkPathExists(xpkObject xpk, const char* filePath);
void*       xpkPathAppendFile(xpkObject xpk, const char* filePath, const char* srcPath, int level);
void*       xpkPathAppendData(xpkObject xpk, const char* filePath, const void* data, uint32_t size, int level);
int         xpkPathExtractFile(xpkObject xpk, const char* filePath, const char* dstPath);
void*       xpkPathExtractData(xpkObject xpk, const char* filePath, uint32_t* outSize);
int         xpkPathRemove(xpkObject xpk, const char* filePath);
const char* xpkPathGet(xpkObject xpk, uint32_t pos);
```

### 6.8 遍历与批量

```c
typedef int (*xpkEachCallback)(void* xpk, uint32_t pos, void* info, void* userData);

int         xpkEach(xpkObject xpk, xpkEachCallback callback, void* userData);
int         xpkEachMatch(xpkObject xpk, const char* pattern, xpkEachCallback callback, void* userData);
int         xpkExtractAll(xpkObject xpk, const char* dir);
int         xpkAppendDir(xpkObject xpk, const char* dir, const char* pattern, int level, int recursive);
```

### 6.9 工具函数

```c
void        xpkFree(void* ptr);
uint32_t    xpkHash(const void* data, uint32_t size);
int         xpkVerify(xpkObject xpk, uint32_t pos);
int         xpkVerifyAll(xpkObject xpk);
int         xpkStatGet(xpkObject xpk, xpkStat* stat);
int         xpkRebuild(xpkObject xpk);
int         xpkLastError(void);
const char* xpkLastErrorMsg(void);
```

---

## 7. 依赖库规范

### 7.1 必需依赖

| 库 | 版本 | 用途 |
|----|------|------|
| xrt | latest | 文件操作、内存管理、哈希、数组 |
| lz4 | 1.9+ | LZ4/LZ4-HC 压缩 |
| zstd | 1.5+ | ZSTD 压缩 |
| lzma | 2501+ | LZMA2 压缩 (LZMA SDK) |

### 7.2 头文件引用

```c
#include <xrt/xrt.h>
#include <lz4/lz4.h>
#include <lz4/lz4hc.h>
#include <zstd/zstd.h>
#include <lzma/Lzma2Enc.h>
#include <lzma/Lzma2Dec.h>
```

### 7.3 xrt 功能映射

| 功能 | xrt 函数/类型 |
|------|---------------|
| 文件操作 | xfile, xrtOpen, xrtClose, xrtRead, xrtWrite |
| 结构体数组 | xarray, xrtArrayCreate, xrtArrayGet |
| 哈希计算 | xrtHash32 |
| 内存管理 | xrtMalloc, xrtFree |
| 路径操作 | xrtPathGetName, xrtPathJoin |
| 时间操作 | xtime, xrtToUnixTime, xrtFromUnixTime |

---

## 8. 文件类型标识

```c
#define XPK_FTYPE_UNKNOWN   0   // 未知/通用
#define XPK_FTYPE_BINARY    1   // 二进制数据
#define XPK_FTYPE_TEXT      2   // 文本文件
#define XPK_FTYPE_IMAGE     3   // 图像文件
#define XPK_FTYPE_AUDIO     4   // 音频文件
#define XPK_FTYPE_VIDEO     5   // 视频文件
#define XPK_FTYPE_ARCHIVE   6   // 归档文件
#define XPK_FTYPE_FOLDER    15  // 目录标记
```

---

## 9. 错误码规范

| 代码 | 说明 |
|:----:|------|
| 0 | 成功 |
| 1 | 文件打开失败 |
| 2 | 文件读取失败 |
| 3 | 内存分配失败 |
| 4 | 无效的包格式 |
| 5 | 版本不支持 |
| 6 | 文件位置无效 |
| 7 | 压缩失败 |
| 8 | 解压失败 |
| 9 | 哈希校验失败 |
| 10 | 只读模式禁止写入 |
| 11 | 包类型不匹配 |

---

## 10. 使用场景指南

| 场景 | 推荐级别 | 推荐模式 |
|------|:--------:|:--------:|
| 游戏资源实时加载 | 1-3 | Core/Win32 |
| 一般应用资源包 | 6 | Win32 |
| 软件分发包 | 8-10 | Win32 |
| 数据归档存储 | 12-15 | Linux |
| 已压缩文件 | 0 | 任意 |
| 整数 ID 索引资源 | 6 | Index |

---

## 附录 A: 压缩级别映射表

```c
// ZSTD 策略常量
#define XPK_ZSTD_FAST       1
#define XPK_ZSTD_DFAST      2
#define XPK_ZSTD_GREEDY     3
#define XPK_ZSTD_LAZY       4
#define XPK_ZSTD_LAZY2      5
#define XPK_ZSTD_BTLAZY2    6
#define XPK_ZSTD_BTOPT      7
#define XPK_ZSTD_BTULTRA    8
#define XPK_ZSTD_BTULTRA2   9

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
```

---

## 附录 B: 目录结构

```
d:\git\xPack\
├── src/
│   ├── xpack.h             # 公开头文件
│   ├── xpack_internal.h    # 内部头文件
│   ├── xpack.c             # 主实现
│   ├── xpack_compress.c    # 压缩模块
│   ├── xpack_index.c       # Index 模式
│   ├── xpack_path.c        # 路径模式
│   └── xpack_util.c        # 工具函数
├── docs/
│   ├── design.md           # 设计文档
│   └── spec.md             # 技术规范
├── test/
│   └── ...                 # 测试文件
└── lib/
    ├── lz4/
    ├── zstd/
    └── xrt/
```
