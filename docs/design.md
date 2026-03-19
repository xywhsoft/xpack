# xpk 规格与重构设计

版本: 3.1
日期: 2026-03-19
状态: Draft for Review

## 1. 范围与立场

本文档是 xpk 的正式规格草案，同时也是本轮重构的实现基线。

本轮重构采用 clean-room 立场：

1. 不以任何历史实现为兼容基线。
2. 不保留 68B 头、`ldbRawSize` 或其他未完成半成品设计。
3. 历史代码、历史文档、历史测试只作为案例来源，不作为规范来源。
4. 新实现必须服从本文档，而不是让本文档迁就旧代码。
5. xpk 对 xrt 的依赖边界，以独立的 `xrt` 工程目录为准；仓库内 `lib/xrt.h` 只视为分发形态。

本文档回答四类问题：

1. xpk 是什么，以及为什么这样设计。
2. xpk 文件格式如何编码。
3. xpk 的公开 API 应如何定义。
4. 新实现的内部模块应如何拆分。

## 2. 术语

本文档使用以下术语：

1. `Package`：一个完整的 xpk 包。
2. `Data Blocks`：包内文件数据段。
3. `Package Meta`：包级或用户自定义元数据段。
4. `Entry Table`：压缩后的文件信息段。
5. `appendPos`：当前数据区尾部的下一次数据写入位置。
6. `save`：提交当前修改，使包重新一致。
7. `build`：重构整个包，回收历史空洞，让包重新紧凑。
8. `hole`：历史删除、覆盖或重复写入后遗留下来的无效空间。
9. `normal mode`：非 solid 的普通逐文件压缩模式。
10. `solid mode`：数据段整体压缩、数据段只读的特化模式。

## 3. 产品定位

xpk 不是优先面向长期冷归档的纯归档格式，而是优先面向动态读写的运行时素材包，尤其服务于游戏素材包和微端场景。

### 3.1 主目标

1. 动态增删改时尽量少重写已有数据。
2. 运行时读取要快，尤其面向游戏素材加载。
3. 对次场景保留必要支持，例如分卷、solid、高压缩比归档。

### 3.2 核心取舍

1. 数据优先追加写入，而不是频繁整理磁盘布局。
2. `xpkSave` 负责提交一致性，不负责回收历史空间。
3. `xpkBuild` 才负责全量重构和空间回收。
4. 空间利用率不是主路径第一优先级，动态读写便利性才是。

### 3.3 压缩策略

1. `LZ4` / `LZ4-HC`：最高性能优先，面向运行时和动态写入。
2. `ZSTD`：性能与压缩率均衡，作为通用默认选择。
3. `LZMA2`：高压缩比优先，面向归档与体积敏感场景。

## 4. 总体设计原则

### 4.1 固定 64B 文件头

1. 文件头固定为 `64B`。
2. 不存在 68B 兼容模式。
3. 不存在额外可选头扩展。

### 4.2 条目结构统一扩展

1. 文件信息单元采用 `32B 基础结构 + infoExtSize 扩展` 统一模型。
2. Core、Index、Linux、Win32 的差异仅体现在扩展部分。
3. 任何模式都不另起一套完全独立的底层存储体系。

### 4.3 分卷是存储视图

1. 分卷系统只保存 `volumeSize`。
2. 分卷逻辑由 `xpkRawRead` / `xpkRawWrite` 提供虚拟连续文件视图。
3. 文件格式中不保存卷索引、卷数量或卷段表。

### 4.4 Solid 是数据段只读模式

1. Solid 不是额外包类型，而是包头中的一种模式标记。
2. Solid 禁止再修改文件数据段。
3. Solid 允许修改未进入 solid 压缩体的 `Package Meta` 和 `Entry Table`。
4. Solid 保持和普通模式相同的总体文件结构，只改变数据段压缩方式和条目偏移语义。

### 4.5 规范优先于实现

1. 先定规格，再写代码。
2. 先写状态机，再写 API。
3. 先保证主路径，再扩展次场景。

## 5. 外部能力模型

### 5.1 包类型

xpk 支持四种包类型：

1. `Core`：按顺序位置访问。
2. `Index`：按整数索引访问。
3. `Linux`：按路径访问，大小写敏感。
4. `Win32`：按路径访问，大小写不敏感。

以上四种包类型在 normal mode 下都支持完整读写。
其中 `Core` 是默认模式，不额外携带成熟场景字段，因此公开 API 不再使用 `xpkCore*` 前缀。

### 5.2 Solid 模式能力边界

Solid 模式下：

1. 允许读取文件。
2. 允许读取和修改包级元数据。
3. 允许重命名文件。
4. 允许删除文件。
5. 允许修改文件属性或其他不涉及数据段布局的条目字段。
6. 允许对这些修改执行 `xpkSave`。
7. 允许在需要压紧空间时执行 `xpkBuild`。

Solid 模式下不允许：

1. 添加新文件数据。
2. 更新已有文件数据。
3. 执行任何会重写、重排或重新切片 solid 数据段的操作。

### 5.3 分卷模式能力边界

分卷模式用于：

1. 存储在小容量设备上。
2. 适配 FAT32 等单文件大小受限文件系统。

分卷大小由 `volumeSize` 指定，并遵循以下约束：

1. `volumeSize == 0` 表示不启用分卷。
2. API 不允许设置小于 `64KB` 的分卷大小，以避免过多的分卷计算和 I/O 读写。
3. 上限受 `uint32_t volumeSize` 字段约束。

## 6. 文件格式规范

### 6.1 常量

```c
#define XPK_FILE_HEAD      0x116B7078u
#define XPK_HEAD_SIZE      64u
#define XPK_VOLUME_MIN     0x00010000u
#define XPK_VOLUME_MAX     0xFFFFFFFFu
#define XPK_PATH_BYTES     260u
#define XPK_PATH_ENTRY_SIZE 320u

#define XPK_TYPE_CORE      0u
#define XPK_TYPE_INDEX     1u
#define XPK_TYPE_LINUX     2u
#define XPK_TYPE_WIN32     3u

#define XPK_ALG_STORE      0u
#define XPK_ALG_LZ4        1u
#define XPK_ALG_LZ4HC      2u
#define XPK_ALG_ZSTD       3u
#define XPK_ALG_LZMA2      4u
```

### 6.2 文件布局

标准布局如下：

```text
[xpkHead 64B] [Data Blocks] [Package Meta (compressed)] [Entry Table (compressed)]
```

字段含义如下：

1. `xpkHead` 固定为 64 字节。
2. `Data Blocks` 是文件数据段。
3. `Package Meta` 是包级或用户自定义元数据段。
4. `Entry Table` 是压缩后的文件信息段。
5. `head.dataOffset` 指向 `Package Meta` 起始位置。
6. `Data Blocks` 长度为 `head.dataOffset - XPK_HEAD_SIZE`。

### 6.3 文件头

#### 6.3.1 结构

```c
#pragma pack(push, 1)
typedef struct {
    uint32_t    fileHead;           // 0x00
    uint32_t    fileCount;          // 0x04

    uint32_t    packType    : 2;    // 0x08
    uint32_t    defComp     : 4;    // 默认压缩级别，仅作为新写入文件的缺省值
    uint32_t    metaComp    : 4;
    uint32_t    infoComp    : 4;
    uint32_t    infoExtSize : 18;

    uint32_t    solidMode   : 1;    // 0x0C
    uint32_t    volumeMode  : 1;
    uint32_t    reserved1   : 30;

    uint64_t    dataOffset;         // 0x10
    uint32_t    volumeSize;         // 0x18

    uint32_t    metaRawSize;        // 0x1C
    uint32_t    metaCompSize;       // 0x20
    uint32_t    metaHash;           // 0x24

    uint32_t    infoCompSize;       // 0x28
    uint32_t    infoHash;           // 0x2C

    xtime       createTime;         // 0x30
    xtime       changeTime;         // 0x38
} xpkHead;
#pragma pack(pop)
```

#### 6.3.2 约束

1. `sizeof(xpkHead) == 64`。
2. `fileHead` 必须等于 `XPK_FILE_HEAD`。
3. `infoExtSize` 决定条目总大小为 `32 + infoExtSize`。
4. `volumeSize == 0` 表示非分卷模式。
5. 启用分卷时，`volumeSize` 必须满足 `XPK_VOLUME_MIN <= volumeSize <= XPK_VOLUME_MAX`。
6. `solidMode == 1` 表示数据段整体压缩。
7. `changeTime` 在每次成功落盘后更新。

#### 6.3.3 说明

`Entry Table` 的原始总大小可由 `fileCount * (32 + infoExtSize)` 计算，因此头中不保留额外 raw-size 字段。

### 6.4 条目标记位

`flag` 字段定义如下：

```c
#define XPK_FLAG_COMP_MASK      0x0000000F
#define XPK_FLAG_TYPE_MASK      0x000000F0
#define XPK_FLAG_DELETED_MASK   0x00000100
```

位语义：

1. bit `[0..3]`：压缩级别。
2. bit `[4..7]`：文件类型。
3. bit `8`：删除标记。
4. 其他位保留给未来扩展。

### 6.5 条目结构

#### 6.5.1 Core 基础结构

```c
#pragma pack(push, 1)
typedef struct {
    uint32_t    flag;
    uint32_t    fileHash;
    uint64_t    dataOffset;
    uint64_t    dataSize;
    uint64_t    fileSize;
} xpkFileInfo;
#pragma pack(pop)
```

大小固定为 `32B`。

#### 6.5.2 Index 扩展结构

```c
#pragma pack(push, 1)
typedef struct {
    uint32_t    flag;
    uint32_t    fileHash;
    uint64_t    dataOffset;
    uint64_t    dataSize;
    uint64_t    fileSize;
    int64_t     fileIndex;
} xpkFileInfoIndex;
#pragma pack(pop)
```

1. `infoExtSize = 8`
2. 条目总大小为 `40B`

#### 6.5.3 Path 模式结构

本规格固定 Linux / Win32 条目总大小为 `320B`，固定其语义字段集合为：

1. 公共 32B 基础头
2. 路径字节区
3. 平台属性字段
4. 创建时间
5. 修改时间
6. 访问时间

推荐结构如下：

```c
#pragma pack(push, 1)
typedef struct {
    uint32_t    flag;
    uint32_t    fileHash;
    uint64_t    dataOffset;
    uint64_t    dataSize;
    uint64_t    fileSize;
    char        pathBytes[260];
    uint32_t    platformAttr;
    uint64_t    createTime;
    uint64_t    modifyTime;
    uint64_t    accessTime;
} xpkFileInfoPath;
#pragma pack(pop)
```

说明：

1. 该布局满足 `32 + 288 = 320`。
2. `platformAttr` 在 Linux 中可表达 `mode`，在 Win32 中可表达文件属性。
3. `pathBytes` 的确切编码策略在 path 模块章节定义。
4. 写入该结构前应先整体清零；未使用字段和未占满的路径字节保持 `0`，以提升 `Entry Table` 压缩率，并避免泄漏无意义内存内容。

### 6.6 条目偏移语义

`dataOffset` 的解释取决于 `solidMode`：

1. normal mode：保存压缩后数据在包中的物理偏移。
2. solid mode：保存文件在压缩前逻辑数据流中的逻辑偏移。

读取路径必须先检查 `head.solidMode`，再解释条目偏移。

### 6.7 dataSize 语义

`dataSize` 的语义如下：

1. normal mode：表示该文件压缩后数据大小。
2. solid mode：不作为物理压缩块定位依据。
3. solid mode 下的逻辑切片范围由 `dataOffset + fileSize` 决定。

### 6.8 Package Meta 与 Entry Table

#### 6.8.1 Package Meta

`Package Meta` 是包级或用户自定义元数据段：

1. 由 `metaComp` 指定压缩级别。
2. 由 `metaRawSize`、`metaCompSize`、`metaHash` 描述。
3. 可以为空（默认为空）。

#### 6.8.2 Entry Table

`Entry Table` 是压缩后的文件信息段：

1. 由 `infoComp` 指定压缩级别。
2. 原始大小可由 `fileCount * (32 + infoExtSize)` 推导。
3. 由 `infoCompSize` 和 `infoHash` 描述其压缩后大小和压缩前哈希。

### 6.9 normal mode 数据段

在普通模式下：

1. 每个文件独立压缩。
2. 文件删除或更新不会回收旧数据空间。
3. 新数据总是追加到当前数据区尾部。

### 6.10 solid mode 数据段

在 Solid 模式下：

1. 文件数据先按逻辑顺序拼成一个连续未压缩数据流。
2. 整个逻辑流整体压缩后写入 `Data Blocks`。
3. `Package Meta` 和 `Entry Table` 不进入 solid 数据流。
4. 删除、重命名、属性修改不会触碰 solid 数据流本身。

## 7. 运行时对象模型

### 7.1 包对象

建议内部对象如下：

```c
typedef struct xpkEntry {
    uint32_t    flag;
    uint32_t    fileHash;
    uint64_t    dataOffset;
    uint64_t    dataSize;
    uint64_t    fileSize;

    int64_t     fileIndex;

    char*       path;
    uint32_t    platformAttr;
    uint64_t    createTime;
    uint64_t    modifyTime;
    uint64_t    accessTime;
} xpkEntry;

typedef struct xpkPackageState {
    xpkHead     head;

    uint8_t     readonly;
    uint8_t     dirtyHead;
    uint8_t     dirtyPackageMeta;
    uint8_t     dirtyEntryTable;
    uint8_t     dirtyData;

    uint64_t    appendPos;

    void*       packageMeta;
    uint32_t    packageMetaSize;

    xarray      coreEntries;    // packType == CORE
    xlist       indexEntries;   // packType == INDEX
    xdict       pathEntries;    // packType == LINUX / WIN32
    uint32_t    entryCount;

    xpkWriteQueue writeQueue;
    xpkStorage* storage;
    xpkErrorState error;
} xpkPackageState;
```

实现约束：

1. 默认模式使用 xrt 的 `xarray` 维护顺序条目集合。
2. Index 模式使用 xrt 的 `xlist` 维护 `fileIndex -> entry` 映射。
3. Linux / Win32 模式使用 xrt 的 `xdict` 维护 `normalizedPath -> entry` 映射。
4. `entryCount` 始终表示逻辑可见条目数量，不依赖底层容器的内部容量。
5. 新实现应参考独立 `xrt` 工程目录中的容器实现与文档，而不是只参考单头文件快照。

### 7.2 脏标记

脏标记规则如下：

1. 修改文件头相关属性时，标记 `dirtyHead`。
2. 修改 `Package Meta` 时，标记 `dirtyPackageMeta` 和 `dirtyHead`。
3. 添加、删除、重命名、属性修改、索引修改等条目变化时，标记 `dirtyEntryTable` 和 `dirtyHead`。
4. 写入或计划写入文件数据时，标记 `dirtyData`、`dirtyEntryTable` 和 `dirtyHead`。
5. Solid 模式下允许设置 `dirtyPackageMeta`、`dirtyEntryTable` 和 `dirtyHead`，但禁止设置由文件数据写入触发的 `dirtyData`。

### 7.3 生命周期状态

建议状态机如下：

1. `Clean`：对象与磁盘一致。
2. `DirtyMeta`：仅包级元数据有变更。
3. `DirtyEntry`：文件信息段有变更。
4. `DirtyDataBuffered`：存在尚未刷入的数据队列。
5. `DirtyDataAppended`：已有数据写入磁盘但尚未保存尾部段。

状态转移原则：

1. `save` 把任意 dirty 状态转回 `Clean`。
2. `build` 会构造一个新的紧凑布局，然后回到 `Clean`。

### 7.4 两种数据写入策略

#### 7.4.1 立即写入

1. 直接把新文件数据写到当前 `appendPos`。
2. 立即推进 `appendPos` 和逻辑上的 `dataOffset`。
3. 设置 `dirtyData`、`dirtyEntryTable`、`dirtyHead`。

风险：

1. 原尾部段会被新数据覆盖。
2. 如果之后不执行 `save`，包会不一致。

#### 7.4.2 缓存写入

1. 数据先进入内存写队列。
2. 磁盘布局保持原状。
3. 设置 `dirtyData`、`dirtyEntryTable`、`dirtyHead`。

风险：

1. 如果之后不执行 `save`，修改会丢失。
2. 但原包不会被损坏。

### 7.5 空间浪费模型

xpk 明确接受“为了动态写入便利性而产生历史空洞”的取向。

规则如下：

1. 删除文件不会回收旧数据空间。
2. 删除后再次添加同名文件，新数据仍然追加到数据区尾部。
3. 更新文件数据时，新数据也写到数据区尾部。
4. 重命名、属性修改、元数据修改不新增数据空洞。

## 8. save 与 build

### 8.1 xpkSave

`xpkSave` 的职责是提交当前状态，不负责空间整理。

推荐流程：

1. 刷入缓存写队列中的数据。
2. 重新编码并压缩 `Package Meta`。
3. 重新编码并压缩 `Entry Table`。
4. 更新 `head.dataOffset`、`fileCount`、哈希和时间戳。
5. 最后回写前 64B 文件头。
6. 清空脏标记。

约束：

1. 文件头必须最后写入。
2. 只有 `save` 成功返回后，修改才视为正式提交。
3. Solid 模式下 `save` 只允许提交 `Package Meta`、`Entry Table` 和头部修改。

### 8.2 xpkBuild

`xpkBuild` 是显式的全量重构动作。

命名约定：

1. 新规范统一使用 `xpkBuild`。
2. 历史命名 `xpkReBuild` 不进入新设计。

职责如下：

1. 只保留当前仍然有效的文件、元数据和文件信息。
2. 释放历史删除、覆盖和重复写入产生的空洞。
3. 重新生成紧凑的数据段。
4. 重新生成 `Package Meta` 和 `Entry Table`。
5. 更新全部条目偏移、头信息和时间戳。

推荐实现：

1. 通过临时输出包构建新布局。
2. 成功后再替换原包。
3. 失败时不破坏原包。

## 9. 公开 API 规格

### 9.1 API 设计规则

1. 所有公开函数使用 `xpk` 前缀。
2. 公开函数返回 `0` 表示成功，负值表示失败。
3. 内存返回值的释放统一使用 `xpkFree()`。
4. 结构化参数优先采用 options 结构体，便于未来扩展。

### 9.2 基础类型

```c
typedef struct xpkStruct* xpkObject;

typedef enum xpkPackType {
    XPK_PACK_CORE  = 0,
    XPK_PACK_INDEX = 1,
    XPK_PACK_LINUX = 2,
    XPK_PACK_WIN32 = 3
} xpkPackType;

typedef enum xpkWritePolicy {
    XPK_WRITE_BUFFERED = 0,
    XPK_WRITE_IMMEDIATE = 1
} xpkWritePolicy;

typedef enum xpkErrorCode {
    XPK_OK = 0,
    XPK_ERR_PARAM,
    XPK_ERR_STATE,
    XPK_ERR_MEMORY,
    XPK_ERR_IO,
    XPK_ERR_FORMAT,
    XPK_ERR_HASH,
    XPK_ERR_NOT_FOUND,
    XPK_ERR_EXISTS,
    XPK_ERR_SOLID_DATA_WRITE,
    XPK_ERR_READONLY,
    XPK_ERR_UNSUPPORTED
} xpkErrorCode;

typedef struct xpkOpenOptions {
    uint8_t readonly;
    uint8_t createIfMissing;
    uint8_t bufferedDefault;
    uint8_t reserved0;
} xpkOpenOptions;

typedef struct xpkWriteOptions {
    uint8_t compLevel;      // 0xFF 表示使用 head.defComp
    uint8_t writePolicy;    // xpkWritePolicy
    uint8_t fileType;       // 自定义文件类型
    uint8_t reserved0;
} xpkWriteOptions;

typedef struct xpkBuildOptions {
    const char* tempPath;   // 可选
    uint8_t replaceOriginal;
    uint8_t reserved0[7];
} xpkBuildOptions;
```

### 9.3 生命周期 API

```c
xpkObject xpkOpen(const char* packagePath, const xpkOpenOptions* options);
int xpkClose(xpkObject xpk);
int xpkSave(xpkObject xpk);
int xpkBuild(xpkObject xpk, const xpkBuildOptions* options);
```

语义：

1. `xpkOpen` 打开或创建包对象。
2. `xpkClose` 释放对象，不隐式调用 `save`。
3. `xpkSave` 提交修改，但不整理空洞。
4. `xpkBuild` 重构整个包，回收空洞。

### 9.4 包配置 API

```c
int xpkGetPackType(xpkObject xpk, xpkPackType* outType);
int xpkSetPackType(xpkObject xpk, xpkPackType type);

int xpkGetDefaultComp(xpkObject xpk, uint8_t* outLevel);
int xpkSetDefaultComp(xpkObject xpk, uint8_t level);

int xpkGetMetaComp(xpkObject xpk, uint8_t* outLevel);
int xpkSetMetaComp(xpkObject xpk, uint8_t level);

int xpkGetInfoComp(xpkObject xpk, uint8_t* outLevel);
int xpkSetInfoComp(xpkObject xpk, uint8_t level);

int xpkGetVolumeSize(xpkObject xpk, uint32_t* outSize);
int xpkSetVolumeSize(xpkObject xpk, uint32_t size);

int xpkGetSolidMode(xpkObject xpk, int* outEnabled);
int xpkSetSolidMode(xpkObject xpk, int enabled);
```

设计约束：

1. `packType` 只能在空包上切换。
2. `volumeSize` 和 `solidMode` 如果会改变数据布局，则必须通过 `xpkBuild` 生效。
3. `defComp`、`metaComp`、`infoComp` 可以在非空包上修改，并影响未来写入或下次保存。

### 9.5 Package Meta API

```c
void* xpkMetaGet(xpkObject xpk, uint32_t* outSize);
int xpkMetaSet(xpkObject xpk, const void* data, uint32_t size, uint8_t compLevel);
int xpkMetaClear(xpkObject xpk);
```

语义：

1. `xpkMetaGet` 返回包级元数据副本。
2. `xpkMetaSet` 会更新 `Package Meta` 并设置相应脏标记。
3. `xpkMetaClear` 清空包级元数据。
4. Solid 模式下允许这些操作。

### 9.6 默认模式 API

```c
uint32_t xpkCount(xpkObject xpk);
int xpkGetInfo(xpkObject xpk, uint32_t pos, xpkFileInfo* outInfo);

int xpkAddFile(xpkObject xpk, const char* srcPath,
               const xpkWriteOptions* options, uint32_t* outPos);
int xpkAddData(xpkObject xpk, const void* data, uint64_t size,
               const xpkWriteOptions* options, uint32_t* outPos);

int xpkReadToFile(xpkObject xpk, uint32_t pos, const char* dstPath);
void* xpkReadToMemory(xpkObject xpk, uint32_t pos, uint64_t* outSize);

int xpkUpdateFile(xpkObject xpk, uint32_t pos, const char* srcPath,
                  const xpkWriteOptions* options);
int xpkUpdateData(xpkObject xpk, uint32_t pos, const void* data, uint64_t size,
                  const xpkWriteOptions* options);

int xpkRemove(xpkObject xpk, uint32_t pos);
int xpkSetFlag(xpkObject xpk, uint32_t pos, uint32_t mask, uint32_t value);
```

命名规则：

1. 默认模式不使用 `Core` 前缀。
2. `Index`、`Path` 系列 API 保留模式前缀，因为它们对应成熟的专用访问模型。

### 9.7 Index 模式 API

```c
int xpkIndexFind(xpkObject xpk, int64_t fileIndex, uint32_t* outPos);
int xpkIndexGetInfo(xpkObject xpk, int64_t fileIndex, xpkFileInfoIndex* outInfo);

int xpkIndexAddFile(xpkObject xpk, int64_t fileIndex, const char* srcPath,
                    const xpkWriteOptions* options);
int xpkIndexAddData(xpkObject xpk, int64_t fileIndex, const void* data, uint64_t size,
                    const xpkWriteOptions* options);

int xpkIndexReadToFile(xpkObject xpk, int64_t fileIndex, const char* dstPath);
void* xpkIndexReadToMemory(xpkObject xpk, int64_t fileIndex, uint64_t* outSize);

int xpkIndexUpdateFile(xpkObject xpk, int64_t fileIndex, const char* srcPath,
                       const xpkWriteOptions* options);
int xpkIndexUpdateData(xpkObject xpk, int64_t fileIndex, const void* data, uint64_t size,
                       const xpkWriteOptions* options);

int xpkIndexRemove(xpkObject xpk, int64_t fileIndex);
int xpkIndexSetFlag(xpkObject xpk, int64_t fileIndex, uint32_t mask, uint32_t value);
```

### 9.8 Path 模式 API

```c
int xpkPathExists(xpkObject xpk, const char* packagePath);
int xpkPathGetInfo(xpkObject xpk, const char* packagePath, xpkFileInfoPath* outInfo);

int xpkPathAddFile(xpkObject xpk, const char* packagePath, const char* srcPath,
                   const xpkWriteOptions* options);
int xpkPathAddData(xpkObject xpk, const char* packagePath, const void* data, uint64_t size,
                   const xpkWriteOptions* options);

int xpkPathReadToFile(xpkObject xpk, const char* packagePath, const char* dstPath);
void* xpkPathReadToMemory(xpkObject xpk, const char* packagePath, uint64_t* outSize);

int xpkPathUpdateFile(xpkObject xpk, const char* packagePath, const char* srcPath,
                      const xpkWriteOptions* options);
int xpkPathUpdateData(xpkObject xpk, const char* packagePath, const void* data, uint64_t size,
                      const xpkWriteOptions* options);

int xpkPathRename(xpkObject xpk, const char* oldPath, const char* newPath);
int xpkPathRemove(xpkObject xpk, const char* packagePath);
int xpkPathSetAttr(xpkObject xpk, const char* packagePath, uint32_t platformAttr);
```

路径模式规则：

1. Linux 模式大小写敏感。
2. Win32 模式大小写不敏感。
3. Solid 模式下允许 `xpkPathRename`、`xpkPathRemove`、`xpkPathSetAttr`。
4. Solid 模式下禁止 `xpkPathAdd*` 和 `xpkPathUpdate*`。

### 9.9 遍历与查询 API

```c
typedef int (*xpkEachProc)(xpkObject xpk, uint32_t pos, const void* info, void* userData);

int xpkEach(xpkObject xpk, xpkEachProc proc, void* userData);
int xpkEachMatch(xpkObject xpk, const char* pattern, xpkEachProc proc, void* userData);
```

### 9.10 维护与诊断 API

```c
typedef struct xpkStat {
    uint32_t fileCount;
    uint64_t liveDataBytes;
    uint64_t holeBytes;
    uint64_t metaBytes;
    uint64_t entryTableBytes;
} xpkStat;

int xpkVerify(xpkObject xpk, uint32_t pos);
int xpkVerifyAll(xpkObject xpk);
int xpkStatGet(xpkObject xpk, xpkStat* outStat);
void xpkFree(void* ptr);
uint32_t xpkHash32(const void* data, uint64_t size);
```

### 9.11 错误 API

```c
xpkErrorCode xpkLastError(xpkObject xpk);
const char* xpkLastErrorMessage(xpkObject xpk);
```

错误模型规则：

1. 每个对象保存自身错误。
2. 不使用进程级全局错误状态作为唯一来源。
3. 公开 API 发生错误时必须设置对象错误码和消息。

## 10. 内部模块设计

### 10.1 推荐目录

```text
xPack/
├─ include/
│  └─ xpk/
│     └─ xpk.h
├─ src/
│  ├─ api/
│  ├─ service/
│  ├─ mode/
│  ├─ format/
│  ├─ storage/
│  ├─ codec/
│  ├─ runtime/
│  └─ internal/
├─ tests/
│  ├─ spec/
│  ├─ unit/
│  ├─ integration/
│  └─ regression/
├─ tools/
│  ├─ xpkcon/
│  └─ xpkgui/
└─ old/
```

### 10.2 API 层

职责：

1. 参数校验。
2. 调用 service。
3. 把内部错误映射到公开错误。

禁止：

1. 直接读写磁盘。
2. 直接压缩或解压。
3. 直接解释磁盘结构。

### 10.3 Service 层

建议拆分：

1. `package_service.c`
2. `save_service.c`
3. `build_service.c`
4. `meta_service.c`
5. `entry_service.c`
6. `solid_service.c`
7. `verify_service.c`
8. `xrt_bridge.c`

职责：

1. 维护脏标记。
2. 维护 `appendPos`。
3. 执行 `save` 和 `build`。
4. 管理 Solid 的数据段写保护与元数据可写边界。
5. 协调 xrt 容器、文件句柄、时间与错误通道。

### 10.4 Mode 层

建议拆分：

1. `mode_default.c`
2. `mode_index.c`
3. `mode_path.c`
4. `path_normalize.c`

职责：

1. 默认模式的位置访问规则。
2. Index 的索引查找规则。
3. Linux / Win32 的路径哈希、规范化和大小写规则。
4. 分别绑定 `xarray`、`xlist`、`xdict` 三类 xrt 容器。

### 10.5 Format 层

建议拆分：

1. `head_codec.c`
2. `entry_codec.c`
3. `meta_codec.c`
4. `layout.c`

职责：

1. 编码和解码 `xpkHead`。
2. 编码和解码不同模式的条目。
3. 计算 `Entry Table` 原始大小。
4. 生成和解析 `Package Meta` 与 `Entry Table`。
5. 解释 `solidMode` 下的偏移语义。

建议内部接口：

```c
int xpkFormatReadHead(xpkStorage* storage, xpkHead* outHead);
int xpkFormatWriteHead(xpkStorage* storage, const xpkHead* head);
uint64_t xpkFormatEntryRawSize(const xpkHead* head);
int xpkFormatEncodeEntryTable(const xpkHead* head, const xpkEntry* entries,
                              uint32_t count, void** outBuf, uint32_t* outSize, uint32_t* outHash);
int xpkFormatDecodeEntryTable(const xpkHead* head, const void* buf, uint32_t size,
                              xpkEntry** outEntries, uint32_t* outCount);
```

### 10.6 Storage 层

建议拆分：

1. `rawio.c`
2. `storage_single.c`
3. `storage_volume.c`
4. `write_queue.c`

职责：

1. 提供逻辑连续文件视图。
2. 提供分卷偏移映射。
3. 管理缓存写队列。
4. 管理实际文件句柄和 flush。
5. 基于 xrt 的文件与路径能力实现跨平台读写。

建议内部接口：

```c
typedef struct xpkStorageOps {
    int  (*read)(void* self, uint64_t offset, uint64_t size, void* out);
    int  (*write)(void* self, uint64_t offset, uint64_t size, const void* data);
    int  (*flush)(void* self);
    int  (*close)(void* self);
    uint64_t (*size)(void* self);
} xpkStorageOps;

int xpkRawRead(xpkStorage* storage, uint64_t offset, uint64_t size, void* out);
int xpkRawWrite(xpkStorage* storage, uint64_t offset, uint64_t size, const void* data);
```

### 10.7 Codec 层

建议拆分：

1. `codec_router.c`
2. `codec_store.c`
3. `codec_lz4.c`
4. `codec_zstd.c`
5. `codec_lzma2.c`

职责：

1. 依据压缩级别选择算法。
2. 计算压缩上界。
3. 执行压缩和解压。
4. 在需要时回退到 `STORE`。

建议内部接口：

```c
typedef struct xpkCodecResult {
    uint32_t compSize;
    uint32_t hash;
    uint8_t  algorithm;
} xpkCodecResult;

int xpkCodecCompress(uint8_t level, const void* src, uint32_t srcSize,
                     void* dst, uint32_t dstCap, xpkCodecResult* outResult);
int xpkCodecDecompress(uint8_t level, const void* src, uint32_t srcSize,
                       void* dst, uint32_t dstSize, uint32_t expectedHash);
```

### 10.8 Runtime 层

建议拆分：

1. `error.c`
2. `memory.c`
3. `time.c`
4. `hash.c`
5. `path.c`

职责：

1. 错误对象。
2. 时间与时间戳转换。
3. 内存分配包装。
4. 哈希函数。
5. 路径规范化工具。
6. 对 xrt 的错误、时间、文件、路径和内存能力做薄封装。

实现基线：

1. 文件读写、seek、size、flush 依赖 xrt 的文件接口。
2. 时间戳获取与转换依赖 xrt 的 `xtime` / 时间接口。
3. 默认模式条目集合依赖 xrt 的 `xarray`。
4. Index 模式条目集合依赖 xrt 的 `xlist`。
5. Linux / Win32 模式条目集合依赖 xrt 的 `xdict`。
6. xrt 的独立工程目录是模块设计与使用方式的参考来源。

### 10.9 模块依赖规则

依赖方向固定如下：

1. `api -> service`
2. `service -> mode`
3. `service -> format`
4. `service -> storage`
5. `format -> codec`
6. `storage -> runtime`
7. `mode -> runtime`
8. `runtime -> xrt`
9. `storage -> xrt`
10. `mode -> xrt`

禁止：

1. `api -> format`
2. `api -> storage`
3. `mode -> storage`
4. `codec -> service`
5. `service -> xrt` 直接越过 runtime 封装

## 11. 核心流程设计

### 11.1 normal mode 添加或更新文件

流程：

1. 读取输入文件或输入内存块。
2. 依据 `xpkWriteOptions` 决定压缩级别和写入策略。
3. 压缩后数据写入 `appendPos` 或进入写队列。
4. 更新条目中的 `dataOffset`、`dataSize`、`fileSize`、`flag`。
5. 标记 `dirtyData`、`dirtyEntryTable`、`dirtyHead`。

### 11.2 重命名、删除、属性修改

流程：

1. 定位条目。
2. 修改路径、删除标记或平台属性。
3. 不触碰 `Data Blocks`。
4. 标记 `dirtyEntryTable` 和 `dirtyHead`。

说明：

1. 这套规则在 normal mode 与 solid mode 都成立。
2. 删除只影响逻辑可见性，不立即回收物理空间。

### 11.3 normal mode 读取

流程：

1. 根据条目找到物理 `dataOffset`。
2. 读取 `dataSize` 字节压缩块。
3. 解压得到 `fileSize` 字节数据。
4. 校验哈希或必要的完整性信息。

### 11.4 solid mode 读取

流程：

1. 读取整个 solid 压缩体或其可缓存视图。
2. 解压得到逻辑连续数据流。
3. 按条目的逻辑 `dataOffset` 和 `fileSize` 切片。
4. 返回目标文件内容。

### 11.5 save

流程：

1. 若存在缓存数据，按顺序刷入数据区。
2. 根据内存中的 `packageMeta` 重新生成 `Package Meta` 压缩段。
3. 根据条目数组重新生成 `Entry Table` 压缩段。
4. 将两段写入当前尾部。
5. 更新 `head.dataOffset`、`fileCount`、哈希、时间戳。
6. 最后回写 `xpkHead`。

### 11.6 build

流程：

1. 枚举当前仍然有效的条目。
2. 新建临时包。
3. 按当前模式重新写入活跃数据。
4. 重新写入 `Package Meta` 和 `Entry Table`。
5. 校验成功后替换原包。

## 12. 测试设计

### 12.1 测试层级

1. `tests/spec/`：验证格式和状态机规则。
2. `tests/unit/`：验证 format、codec、storage、mode 的纯模块行为。
3. `tests/integration/`：验证完整读写流程。
4. `tests/regression/`：只吸收和新规范一致的历史案例。

### 12.2 必测项

1. 文件头固定为 `64B`。
2. Core / Index / Path 条目大小与 `infoExtSize` 正确。
3. `dataOffset` 与尾部布局正确。
4. `Package Meta` 与 `Entry Table` 的压缩和校验正确。
5. normal mode 下 `dataOffset` 表示压缩后物理偏移。
6. solid mode 下 `dataOffset` 表示压缩前逻辑偏移。
7. 分卷跨边界读写正确。
8. 立即写入后未 `save` 的损坏风险符合预期。
9. 缓存写入后未 `save` 不损坏原包但会丢失修改。
10. `save` 后脏标记清零且包重新一致。
11. Solid 模式下允许修改包级元数据并成功 `save`。
12. Solid 模式下允许修改属性、重命名、删除并成功 `save`。
13. Solid 模式下文件数据相关写接口全部拒绝。
14. 删除后再次添加时，新数据仍追加到尾部，不复用历史空洞。
15. `xpkBuild` 后可以回收删除和覆盖产生的空洞。

## 13. 实现路线

### Phase 1：格式与状态机定稿

1. 固定 64B 头结构。
2. 固定条目结构和 `infoExtSize` 规则。
3. 固定 dirty / save / build 状态机。

### Phase 2：format / codec / storage 落地

1. 完成头结构和条目编码。
2. 完成 `Package Meta` 与 `Entry Table` 压缩处理。
3. 完成单卷和分卷原始 I/O。

### Phase 3：normal mode 读写落地

1. 完成 Core / Index / Linux / Win32 的完整读写。
2. 完成立即写入和缓存写入两种策略。
3. 完成 `save` 路径。

### Phase 4：solid 数据只读路径落地

1. 完成 solid 数据读取。
2. 完成 solid 偏移解释。
3. 完成 solid 下元数据可写、数据段不可写的边界控制。
4. 完成 solid 下删除、重命名、属性修改的 entry-table 路径。

### Phase 5：build 与空间整理落地

1. 完成无效空间识别。
2. 完成 `xpkBuild` 重构路径。
3. 完成临时包替换策略。

### Phase 6：工具与工程收口

1. 接入 CLI。
2. 再决定 GUI 接入方式。
3. 完成构建系统与测试资产整理。

## 14. 待作者审阅点

以下内容已经细化为可实现设计，但仍建议作者重点审阅：

1. `dataSize` 在 solid mode 下保留但不参与物理块定位，这一语义是否接受。
2. `xpkSetSolidMode` 与 `xpkSetVolumeSize` 在非空包上的最终生效策略，是否统一要求通过 `xpkBuild`。
3. `xpkBuild` 的公开签名是否保留 options 结构，还是简化为无参数形式。
4. 错误文本是否复用 xrt 的线程级错误描述机制，还是由 xpk 独立维护对象级错误缓冲。
