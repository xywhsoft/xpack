# xPack Ver7 设计文档

## 一、概述

### 1.1 项目简介

xPack 是一个轻量级文件压缩包库，支持多种包类型和压缩算法。Ver7 是该库的第七个主要版本，在 Ver6 基础上进行了重大升级。

### 1.2 版本演进

| 版本 | 特性 |
|------|------|
| Ver5 | FreeBASIC 实现，基础功能 |
| Ver6 | C 语言重写，四种包类型，自定义压缩回调 |
| **Ver7** | 纯 ZSTD 方案，压缩级别体系，xrt 库整合 |

### 1.3 Ver7 主要变更

1. **压缩算法简化**：移除 LZMA2/XZ，采用纯 ZSTD 方案
2. **压缩级别体系**：0-15 级，严格单调递增
3. **依赖库整合**：统一使用 xrt 库（整合 xCore、xFile、mmu、hash）
4. **位域结构**：使用位域替代 MASK 掩码运算
5. **API 命名规范**：驼峰命名，`xpk` 前缀

---

## 二、压缩级别设计

### 2.1 设计原则

- **严格单调性**：级别提升 → 压缩比提高 → 速度降低
- **算法分层**：LZ4 系列（极速）+ ZSTD 系列（平衡/高压缩）
- **4bit 存储**：支持 0-15 共 16 个级别

### 2.2 级别定义

| 级别 | 算法 | 原生参数 | 压缩比 | 压缩速度 | 解压速度 | 适用场景 |
|:----:|------|----------|:------:|:--------:|:--------:|----------|
| 0 | 无压缩 | - | 1.00 | ∞ | ∞ | 已压缩数据 |
| 1 | LZ4 | default | ~2.10 | 780 MB/s | 4500 MB/s | 实时加载 |
| 2 | LZ4-HC | level 4 | ~2.45 | 120 MB/s | 4500 MB/s | 极速解压 |
| 3 | LZ4-HC | level 9 | ~2.72 | 40 MB/s | 4500 MB/s | LZ4 极限 |
| 4 | ZSTD | level 1 | ~2.88 | 500 MB/s | 1400 MB/s | 快速压缩 |
| 5 | ZSTD | level 2 | ~2.95 | 400 MB/s | 1380 MB/s | 较快压缩 |
| **6** | **ZSTD** | **level 4** | **~3.08** | **250 MB/s** | **1350 MB/s** | **通用默认** |
| 7 | ZSTD | level 6 | ~3.15 | 130 MB/s | 1300 MB/s | 标准压缩 |
| 8 | ZSTD | level 8 | ~3.22 | 70 MB/s | 1260 MB/s | 较高压缩 |
| 9 | ZSTD | level 10 | ~3.28 | 45 MB/s | 1220 MB/s | 高压缩 |
| 10 | ZSTD | level 12 | ~3.33 | 30 MB/s | 1180 MB/s | 更高压缩 |
| 11 | ZSTD | level 14 | ~3.38 | 18 MB/s | 1140 MB/s | 高压缩比 |
| 12 | ZSTD | level 16 | ~3.43 | 10 MB/s | 1100 MB/s | 很高压缩比 |
| 13 | ZSTD | level 18 | ~3.47 | 6 MB/s | 1050 MB/s | 超高压缩比 |
| 14 | ZSTD | level 20 | ~3.50 | 4 MB/s | 1000 MB/s | 接近极限 |
| 15 | ZSTD | level 22 | ~3.52 | 2 MB/s | 950 MB/s | ZSTD 极限 |

### 2.3 使用建议

| 场景 | 推荐级别 | 说明 |
|------|:--------:|------|
| 游戏资源实时加载 | 1-3 | LZ4 极速解压 |
| 一般应用资源包 | 6 | 默认平衡 |
| 软件分发包 | 8-10 | 较高压缩比 |
| 数据归档存储 | 12-15 | 追求压缩比 |
| 已压缩文件 (jpg/mp3) | 0 | 避免重复压缩 |

### 2.4 映射表实现

```c
#define XPK_ALG_STORE   0   // 无压缩
#define XPK_ALG_LZ4     1   // LZ4
#define XPK_ALG_LZ4HC   2   // LZ4-HC
#define XPK_ALG_ZSTD    3   // ZSTD

static const xpkCompMap xpkCompTable[16] = {
    { XPK_ALG_STORE,  0 },  // 0:  无压缩
    { XPK_ALG_LZ4,    1 },  // 1:  LZ4 default
    { XPK_ALG_LZ4HC,  4 },  // 2:  LZ4-HC level 4
    { XPK_ALG_LZ4HC,  9 },  // 3:  LZ4-HC level 9
    { XPK_ALG_ZSTD,   1 },  // 4:  ZSTD level 1
    { XPK_ALG_ZSTD,   2 },  // 5:  ZSTD level 2
    { XPK_ALG_ZSTD,   4 },  // 6:  ZSTD level 4  [DEFAULT]
    { XPK_ALG_ZSTD,   6 },  // 7:  ZSTD level 6
    { XPK_ALG_ZSTD,   8 },  // 8:  ZSTD level 8
    { XPK_ALG_ZSTD,  10 },  // 9:  ZSTD level 10
    { XPK_ALG_ZSTD,  12 },  // 10: ZSTD level 12
    { XPK_ALG_ZSTD,  14 },  // 11: ZSTD level 14
    { XPK_ALG_ZSTD,  16 },  // 12: ZSTD level 16
    { XPK_ALG_ZSTD,  18 },  // 13: ZSTD level 18
    { XPK_ALG_ZSTD,  20 },  // 14: ZSTD level 20
    { XPK_ALG_ZSTD,  22 },  // 15: ZSTD level 22
};
```

---

## 三、包类型设计

### 3.1 四种包类型

| 类型 | 值 | 说明 | 访问方式 |
|------|:--:|------|----------|
| Core | 0 | 核心模式 | 按位置访问 (1-based) |
| Index | 1 | 索引模式 | 按整数索引访问 |
| Linux | 2 | Linux 模式 | 按路径访问（大小写敏感） |
| Win32 | 3 | Windows 模式 | 按路径访问（不区分大小写） |

### 3.2 模式选择指南

```
选择 Core 模式：
  - 文件数量固定
  - 按顺序访问
  - 最小存储开销

选择 Index 模式：
  - 需要用整数 ID 标识文件
  - 文件顺序无关
  - 支持用户自定义数据

选择 Linux/Win32 模式：
  - 需要保留目录结构
  - 按文件名/路径访问
  - 需要保留文件属性
```

---

## 四、数据结构设计

### 4.1 文件格式布局

```
+------------------+
|    包信息头      |  48 bytes (xpkHead)
+------------------+
|  包头扩展数据    |  可选，用户自定义
+------------------+
|                  |
|    文件数据区    |  压缩后的文件内容
|                  |
+------------------+
|   LDB 数据块     |  压缩后的文件信息列表
+------------------+
```

### 4.2 包信息头 (48 bytes)

```c
typedef struct {
    // 基础标识 (8 bytes)
    uint32_t    signature;      // 文件签名 "xpk\0" (0x006B7078)
    uint8_t     version;        // 版本号 (7)
    uint8_t     reserved0;      // 保留
    uint16_t    infoExtSize;    // 文件信息扩展大小
    
    // 包标记 (4 bytes)
    xpkFlag     flag;           // 包标记位域
    
    // 文件信息 (12 bytes)
    uint32_t    fileCount;      // 文件数量
    uint32_t    headExtSize;    // 包头扩展数据大小
    uint32_t    discCode;       // 识别代码(用户自定义)
    
    // LDB 信息 (16 bytes)
    uint32_t    ldbOffset;      // LDB 偏移位置
    uint32_t    ldbSize;        // LDB 压缩后大小
    uint32_t    ldbRawSize;     // LDB 原始大小
    uint32_t    ldbHash;        // LDB 哈希值
    
    // 时间戳 (8 bytes)
    uint32_t    createTime;     // 创建时间
    uint32_t    modifyTime;     // 修改时间
} xpkHead;
```

### 4.3 包标记位域

```c
typedef union {
    uint32_t value;
    struct {
        uint32_t packType   : 4;    // [0-3]   包类型 (0-3)
        uint32_t ldbComp    : 4;    // [4-7]   LDB 压缩级别 (0-15)
        uint32_t reserved1  : 8;    // [8-15]  保留
        uint32_t reserved2  : 16;   // [16-31] 保留
    };
} xpkFlag;
```

### 4.4 文件信息结构

#### Core 模式 (20 bytes)

```c
typedef struct {
    uint32_t    dataOffset;     // 数据偏移位置
    uint32_t    dataSize;       // 压缩后大小
    uint32_t    fileSize;       // 原始大小
    uint32_t    fileHash;       // 文件哈希值
    xpkFileFlag flag;           // 文件标记位域
} xpkFileInfo;
```

#### Index 模式 (28 bytes)

```c
typedef struct {
    // 基础信息 (20 bytes)
    uint32_t    dataOffset;
    uint32_t    dataSize;
    uint32_t    fileSize;
    uint32_t    fileHash;
    xpkFileFlag flag;
    // 扩展信息 (8 bytes)
    int32_t     fileIndex;      // 文件索引号
    int32_t     userData;       // 用户自定义数据
} xpkFileInfoIndex;
```

#### Linux 模式 (232 bytes)

```c
typedef struct {
    // 基础信息 (20 bytes)
    uint32_t    dataOffset;
    uint32_t    dataSize;
    uint32_t    fileSize;
    uint32_t    fileHash;
    xpkFileFlag flag;
    // 路径信息 (208 bytes)
    char        filePath[200];  // 文件路径
    uint32_t    pathHash;       // 路径哈希值(大小写敏感)
    // 文件属性 (8 bytes)
    uint32_t    fileAttr;       // 文件属性(权限等)
    uint32_t    modifyTime;     // 修改时间
} xpkFileInfoLinux;
```

#### Win32 模式 (236 bytes)

```c
typedef struct {
    // 基础信息 (20 bytes)
    uint32_t    dataOffset;
    uint32_t    dataSize;
    uint32_t    fileSize;
    uint32_t    fileHash;
    xpkFileFlag flag;
    // 路径信息 (208 bytes)
    char        filePath[200];  // 文件路径
    uint32_t    pathHash;       // 路径哈希值(转小写)
    // 文件属性 (12 bytes)
    uint32_t    fileAttr;       // 文件属性
    uint32_t    createTime;     // 创建时间
    uint32_t    modifyTime;     // 修改时间
} xpkFileInfoWin32;
```

### 4.5 文件标记位域

```c
typedef union {
    uint32_t value;
    struct {
        uint32_t compLevel  : 4;    // [0-3]   压缩级别 (0-15)
        uint32_t fileType   : 4;    // [4-7]   文件类型 (0-15)
        uint32_t encrypted  : 1;    // [8]     加密标记 (保留)
        uint32_t reserved   : 23;   // [9-31]  保留
    };
} xpkFileFlag;
```

---

## 五、API 接口设计

### 5.1 命名规范

- **前缀**：`xpk`
- **风格**：小驼峰命名 (lowerCamelCase)
- **示例**：`xpkOpen`, `xpkAppendFile`, `xpkPathExtractData`

### 5.2 接口分类

#### 生命周期管理

```c
xpkObject   xpkOpen(const char* path, uint32_t offset, int readonly);
int         xpkSave(xpkObject xpk);
void        xpkClose(xpkObject xpk);
```

#### 包属性操作

```c
int         xpkType(xpkObject xpk);
int         xpkTypeSet(xpkObject xpk, int type);
uint32_t    xpkCount(xpkObject xpk);
uint32_t    xpkDiscCode(xpkObject xpk);
int         xpkDiscCodeSet(xpkObject xpk, uint32_t code);
void        xpkOnError(xpkObject xpk, xpkErrorProc callback);
xpkHead*    xpkGetHead(xpkObject xpk);
```

#### Core 模式操作

```c
uint32_t    xpkAppendFile(xpkObject xpk, const char* path, int level);
uint32_t    xpkAppendData(xpkObject xpk, const void* data, uint32_t size, int level);
int         xpkExtractFile(xpkObject xpk, uint32_t pos, const char* path);
void*       xpkExtractData(xpkObject xpk, uint32_t pos, uint32_t* outSize);
int         xpkUpdateFile(xpkObject xpk, uint32_t pos, const char* path, int level);
int         xpkUpdateData(xpkObject xpk, uint32_t pos, const void* data, uint32_t size, int level);
int         xpkRemove(xpkObject xpk, uint32_t pos);
```

#### 文件信息获取

```c
void*       xpkInfo(xpkObject xpk, uint32_t pos);
uint32_t    xpkInfoSize(xpkObject xpk, uint32_t pos);
uint32_t    xpkInfoPacked(xpkObject xpk, uint32_t pos);
uint32_t    xpkInfoHash(xpkObject xpk, uint32_t pos);
int         xpkInfoLevel(xpkObject xpk, uint32_t pos);
int         xpkInfoType(xpkObject xpk, uint32_t pos);
int         xpkInfoTypeSet(xpkObject xpk, uint32_t pos, int type);
```

#### Index 模式操作

```c
uint32_t            xpkIndexFind(xpkObject xpk, int32_t index);
xpkFileInfoIndex*   xpkIndexAppendFile(xpkObject xpk, int32_t index, const char* path, int level);
xpkFileInfoIndex*   xpkIndexAppendData(xpkObject xpk, int32_t index, const void* data, uint32_t size, int level);
int                 xpkIndexExtractFile(xpkObject xpk, int32_t index, const char* path);
void*               xpkIndexExtractData(xpkObject xpk, int32_t index, uint32_t* outSize);
int                 xpkIndexUpdateFile(xpkObject xpk, int32_t index, const char* path, int level);
int                 xpkIndexUpdateData(xpkObject xpk, int32_t index, const void* data, uint32_t size, int level);
int                 xpkIndexRemove(xpkObject xpk, int32_t index);
int32_t             xpkIndexUserData(xpkObject xpk, int32_t index);
int                 xpkIndexUserDataSet(xpkObject xpk, int32_t index, int32_t value);
```

#### 路径模式操作

```c
uint32_t    xpkPathFind(xpkObject xpk, const char* filePath);
int         xpkPathExists(xpkObject xpk, const char* filePath);
void*       xpkPathAppendFile(xpkObject xpk, const char* filePath, const char* srcPath, int level);
void*       xpkPathAppendData(xpkObject xpk, const char* filePath, const void* data, uint32_t size, int level);
int         xpkPathExtractFile(xpkObject xpk, const char* filePath, const char* dstPath);
void*       xpkPathExtractData(xpkObject xpk, const char* filePath, uint32_t* outSize);
int         xpkPathUpdateFile(xpkObject xpk, const char* filePath, const char* srcPath, int level);
int         xpkPathUpdateData(xpkObject xpk, const char* filePath, const void* data, uint32_t size, int level);
int         xpkPathRemove(xpkObject xpk, const char* filePath);
const char* xpkPathGet(xpkObject xpk, uint32_t pos);
```

#### 遍历接口

```c
typedef int (*xpkEachCallback)(void* xpk, uint32_t pos, void* info, void* userData);

int         xpkEach(xpkObject xpk, xpkEachCallback callback, void* userData);
int         xpkEachMatch(xpkObject xpk, const char* pattern, xpkEachCallback callback, void* userData);
```

#### 批量操作

```c
int         xpkExtractAll(xpkObject xpk, const char* dir);
int         xpkAppendDir(xpkObject xpk, const char* dir, const char* pattern, int level, int recursive);
```

#### 工具函数

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

## 六、依赖库

### 6.1 外部依赖

| 库 | 用途 | 来源 |
|----|------|------|
| xrt | 文件操作、内存管理、哈希 | 自研 |
| lz4 | LZ4/LZ4-HC 压缩 | lib/lz4 |
| zstd | ZSTD 压缩 | lib/zstd |

### 6.2 头文件引用

```c
#include <xrt/xrt.h>    // xrt 统一库
#include <lz4/lz4.h>    // LZ4 压缩
#include <lz4/lz4hc.h>  // LZ4-HC 高压缩
#include <zstd/zstd.h>  // ZSTD 压缩
```

---

## 七、使用示例

### 7.1 创建压缩包

```c
// 创建新包
xpkObject xpk = xpkOpen("data.xpk", 0, 0);

// 设置包类型（仅空包可设置）
xpkTypeSet(xpk, XPK_TYPE_WIN32);

// 添加文件
xpkPathAppendFile(xpk, "images/logo.png", "C:/assets/logo.png", 6);
xpkPathAppendFile(xpk, "sounds/bgm.mp3", "C:/assets/bgm.mp3", 0);  // 已压缩，不再压缩

// 保存并关闭
xpkClose(xpk);
```

### 7.2 读取压缩包

```c
// 只读打开
xpkObject xpk = xpkOpen("data.xpk", 0, 1);

// 解压到文件
xpkPathExtractFile(xpk, "images/logo.png", "C:/output/logo.png");

// 解压到内存
uint32_t size;
void* data = xpkPathExtractData(xpk, "sounds/bgm.mp3", &size);
// 使用 data...
xpkFree(data);

xpkClose(xpk);
```

### 7.3 遍历文件

```c
int listCallback(void* xpk, uint32_t pos, void* info, void* userData) {
    xpkFileInfoWin32* file = (xpkFileInfoWin32*)info;
    printf("%d: %s (%u bytes)\n", pos, file->filePath, file->fileSize);
    return 0;  // 继续遍历
}

xpkObject xpk = xpkOpen("data.xpk", 0, 1);
xpkEach(xpk, listCallback, NULL);
xpkClose(xpk);
```

---

## 八、目录结构

```
ver7/
├── docs/
│   └── design.md           # 设计文档
└── src/
    ├── xpack.h             # 公开头文件
    ├── xpack_internal.h    # 内部头文件
    └── xpack.c             # 实现文件 (待开发)
```

---

## 九、版本历史

| 日期 | 版本 | 变更 |
|------|------|------|
| 2026-01-29 | 7.0 | 初始设计完成 |
