# xPack 分卷系统规范 (Volume Specification)

版本: 1.0  
日期: 2026-01-31  
状态: 设计阶段

---

## 目录

1. [概述](#概述)
2. [设计目标](#设计目标)
3. [架构设计](#架构设计)
4. [数据结构](#数据结构)
5. [文件格式](#文件格式)
6. [API 接口](#api-接口)
7. [实现细节](#实现细节)
8. [使用示例](#使用示例)
9. [兼容性说明](#兼容性说明)
10. [错误处理](#错误处理)
11. [性能考虑](#性能考虑)
12. [测试计划](#测试计划)

---

## 概述

xPack 分卷系统允许将大型压缩包分割为多个文件（卷），每个卷具有固定的大小限制。分卷系统设计为**透明操作**，用户只需启用分卷模式并设置卷大小，系统会自动处理跨卷的数据读写。

### 核心特性

- ✅ **向后兼容** - 非分卷文件无需修改即可打开
- ✅ **首卷完整** - 第一个卷包含完整的元数据和 LDB
- ✅ **智能切割** - 支持按字节或按文件分割
- ✅ **独立验证** - 每个卷可独立验证头部完整性
- ✅ **透明操作** - 读写操作无需关心分卷细节
- ✅ **灵活配置** - 可动态调整卷大小和分割模式

---

## 设计目标

| 目标 | 描述 | 优先级 |
|-----|------|-------|
| **兼容性** | 与现有 Ver7 格式完全兼容，非分卷文件无感知 | P0 |
| **透明性** | 用户 API 调用与单卷模式相同 | P0 |
| **可靠性** | 跨卷读写的数据完整性和一致性 | P0 |
| **性能** | 最小化跨卷读取的开销 | P1 |
| **灵活性** | 支持多种分割策略和卷大小配置 | P1 |
| **可扩展性** | 为未来加密和恢复预留空间 | P2 |

---

## 架构设计

### 系统架构图

```
┌─────────────────────────────────────────────────────────────┐
│                      用户应用层                          │
│  xpkOpen() → xpkAppendData() → xpkExtractData()          │
└────────────────────┬──────────────────────────────────────┘
                     │
┌────────────────────▼──────────────────────────────────────┐
│                   xPack API 层                         │
│  (提供统一的接口，隐藏分卷细节)                         │
└────────────────────┬──────────────────────────────────────┘
                     │
┌────────────────────▼──────────────────────────────────────┐
│                分卷管理层 (Volume Manager)               │
│  - 卷文件管理                                          │
│  - 跨卷读写路由                                        │
│  - 容量检查与自动创建新卷                               │
└────────────────────┬──────────────────────────────────────┘
                     │
┌────────────────────▼──────────────────────────────────────┐
│                   文件 I/O 层 (xrt)                    │
│  - 文件打开/关闭                                        │
│  - 读写操作                                            │
└─────────────────────────────────────────────────────────────┘
```

### 分卷管理器职责

| 职责 | 描述 |
|-----|------|
| **卷文件管理** | 管理所有卷文件的打开、关闭、定位 |
| **容量监控** | 检测当前卷容量，必要时创建新卷 |
| **跨卷路由** | 将读写请求路由到正确的卷文件 |
| **元数据同步** | 同步各卷的头部信息 |
| **错误恢复** | 处理卷缺失或损坏的情况 |

---

## 数据结构

### 1. 文件头扩展结构

```c
#pragma pack(push, 1)
typedef struct {
    uint32_t        volumeCount;        // [0-3]   分卷总数 (1=不分卷)
    uint32_t        volumeIndex;       // [4-7]   当前卷索引 (0-based)
} xpkVolumeInfo;
#pragma pack(pop)
```

**字段说明：**
- `volumeCount`: 总卷数，1 表示不分卷
- `volumeIndex`: 当前卷的索引，从 0 开始
  - 0 表示第一个卷（主卷）
  - 1..N 表示后续卷

**位置：**
- 存储在 `xpkHead.headExt` 扩展区域
- `xpkHead.headExtSize = sizeof(xpkVolumeInfo) = 8` 字节

### 2. 包标记位扩展

```c
typedef union {
    uint32_t value;
    struct {
        uint32_t packType   : 4;        // [0-3]   包类型 (0-3)
        uint32_t ldbComp    : 4;        // [4-7]   LDB 压缩级别 (0-15)
        uint32_t solidMode  : 1;        // [8]     固实压缩模式 (0=独立,1=固实)
        uint32_t volumeMode : 1;        // [9]     分卷模式 (0=单卷,1=多卷) [新增]
        uint32_t splitMode  : 2;        // [10-11] 分割模式 (0=字节,1=文件) [新增]
        uint32_t reserved   : 20;       // [12-31] 保留
    };
} xpkFlag;
```

**新增位字段：**
- `volumeMode` (bit 9): 是否启用分卷模式
- `splitMode` (bit 10-11): 分割模式
  - `0`: 按字节分割（默认）
  - `1`: 按文件分割
  - `2-3`: 保留

### 3. 运行时分卷配置

```c
#define XPK_MAX_VOLUMES 256

typedef struct {
    // 配置
    uint8_t         enabled;           // 是否启用分卷
    uint32_t        volumeSize;        // 单卷最大字节数 (0=不限制)
    uint8_t         splitMode;         // 0=按字节, 1=按文件
    
    // 状态
    uint32_t        currentVolume;     // 当前写入的卷索引
    uint32_t        currentOffset;     // 当前卷的写入偏移
    
    // 文件管理
    char            basePath[256];     // 基础路径（不含卷后缀）
    xfile*         volumes[XPK_MAX_VOLUMES];   // 卷文件句柄数组
    uint8_t        volumeOpen[XPK_MAX_VOLUMES]; // 卷是否已打开
    
    // 元数据
    uint32_t        totalSize;         // 所有卷的总大小
    uint32_t        volumeOffsets[XPK_MAX_VOLUMES]; // 各卷的基础偏移
    
} xpkVolume;
```

### 4. xpkStruct 扩展

```c
typedef struct xpkStruct {
    // 现有成员...
    xfile               file;           // 主卷文件句柄 (兼容旧代码)
    uint32_t            baseOffset;     // 主卷基础偏移
    
    // ... 其他现有成员 ...
    
    // 分卷相关
    xpkVolume           volume;         // 分卷管理器
    
} xpkStruct;
```

### 5. 分卷统计信息

```c
typedef struct {
    int             volumeCount;         // 分卷总数
    uint32_t*       volumeSizes;        // 各卷大小数组 (需调用者释放)
    uint64_t        totalSize;          // 总大小（所有卷）
    uint64_t        totalDataSize;      // 数据总大小
    double          avgSize;           // 平均卷大小
} xpkVolumeStat;
```

---

## 文件格式

### 文件命名规则

| 原文件名 | 分卷文件名 | 说明 |
|---------|-----------|------|
| `data.xpk` | `data.xpk` | 卷1（主卷，保留原扩展名） |
| | `data.x01` | 卷2 |
| | `data.x02` | 卷3 |
| | `...` | 后续卷 |
| `archive.pkg` | `archive.pkg` | 卷1 |
| | `archive.p01` | 卷2 |
| | `archive.p02` | 卷3 |

**命名规则：**
1. 第一卷使用原始文件名
2. 后续卷使用：`基础名` + `.` + `扩展名首字母` + `2位数字`
3. 数字从 01 开始，最大支持 99 个卷（可通过扩展支持 256+）

### 文件结构布局

```
┌─────────────────────────────────────────────────────────┐
│  第1卷 (data.xpk) - 主卷                          │
├─────────────────────────────────────────────────────────┤
│  [0-59]    xpkHead (60 bytes)                    │
│              - 完整的包元数据                        │
│              - volumeMode = 1                        │
│              - headExtSize = 8                       │
│  [60-67]   xpkVolumeInfo (8 bytes)                │
│              - volumeCount = N                        │
│              - volumeIndex = 0                        │
│  [68-X]    数据区域（可能包含多个文件的数据）          │
│  [X-end]    完整 LDB（压缩后）                     │
│              - 所有文件的完整信息                      │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│  第2卷 (data.x01)                                 │
├─────────────────────────────────────────────────────────┤
│  [0-59]    xpkHead (60 bytes)                    │
│              - 简化版头部（仅用于识别）                │
│              - volumeMode = 1                        │
│              - headExtSize = 8                       │
│  [60-67]   xpkVolumeInfo (8 bytes)                │
│              - volumeCount = N                        │
│              - volumeIndex = 1                        │
│  [68-end]   数据区域                                │
│              - 不包含 LDB                              │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│  第N卷 (data.x(N-1))                              │
├─────────────────────────────────────────────────────────┤
│  [0-59]    xpkHead (60 bytes)                    │
│  [60-67]   xpkVolumeInfo (8 bytes)                │
│              - volumeCount = N                        │
│              - volumeIndex = N-1                      │
│  [68-end]   数据区域                                │
└─────────────────────────────────────────────────────────┘
```

### 头部验证

每个卷的头部必须验证：

1. **文件头标识** (`fileHead`)
   - 必须等于 `XPK_VERSION` (0x116B7078)
   
2. **分卷标记** (`flag.volumeMode`)
   - 如果为 1，则 `headExtSize` 必须为 8
   
3. **卷索引** (`volumeIndex`)
   - 必须小于 `volumeCount`
   - 第一个卷的索引必须为 0
   
4. **卷计数** (`volumeCount`)
   - 必须大于等于 1

---

## API 接口

### 分卷控制接口

```c
// 检查是否为分卷模式
// 返回: 0=单卷, 1=多卷, -1=错误
XPKAPI int xpkVolumeMode(xpkObject xpk);

// 启用或禁用分卷模式
// 参数:
//   enabled - 0=禁用, 1=启用
// 返回: 0=成功, -1=失败
XPKAPI int xpkVolumeModeSet(xpkObject xpk, int enabled);

// 获取单卷最大大小
// 返回: 字节数, 0=无限制
XPKAPI int xpkVolumeSize(xpkObject xpk);

// 设置单卷最大大小
// 参数:
//   size - 字节数, 0=无限制
// 返回: 0=成功, -1=失败
XPKAPI int xpkVolumeSizeSet(xpkObject xpk, uint32_t size);

// 获取分卷总数
// 返回: 卷数量
XPKAPI int xpkVolumeCount(xpkObject xpk);

// 获取当前卷索引（写入操作使用的卷）
// 返回: 卷索引 (0-based)
XPKAPI int xpkVolumeCurrent(xpkObject xpk);

// 获取分割模式
// 返回: 0=按字节, 1=按文件
XPKAPI int xpkVolumeSplitMode(xpkObject xpk);

// 设置分割模式
// 参数:
//   mode - 0=按字节, 1=按文件
// 返回: 0=成功, -1=失败
XPKAPI int xpkVolumeSplitModeSet(xpkObject xpk, int mode);

// 获取指定卷的文件路径
// 参数:
//   index - 卷索引 (0-based)
// 返回: 文件路径字符串, NULL=错误
XPKAPI const char* xpkVolumePath(xpkObject xpk, int index);

// 获取分卷统计信息
// 参数:
//   stat - 输出统计信息结构
// 返回: 0=成功, -1=失败
XPKAPI int xpkVolumeStatGet(xpkObject xpk, xpkVolumeStat* stat);
```

### 修改的接口

以下接口行为在分卷模式下自动适配：

```c
// xpkOpen() - 自动识别并打开所有卷
xpkObject xpkOpen(const char* path, uint32_t offset, int readonly);

// xpkSave() - 保存所有卷的头部信息
int xpkSave(xpkObject xpk);

// xpkClose() - 关闭所有卷
void xpkClose(xpkObject xpk);

// xpkAppendData() - 自动跨卷写入
uint32_t xpkAppendData(xpkObject xpk, const void* data, uint32_t size, int level);

// xpkExtractData() - 自动跨卷读取
void* xpkExtractData(xpkObject xpk, uint32_t pos, uint32_t* outSize);
```

---

## 实现细节

### 1. 初始化流程

```
xpkOpen("data.xpk", 0, 0)
    ↓
读取主卷头部
    ↓
检测 volumeMode == 1 ?
    ↓ 是
读取 headExt 中的 volumeCount
    ↓
初始化 volume 管理器
    ↓
尝试打开所有卷 (data.xpk, data.x01, ...)
    ↓
计算各卷的基础偏移 (volumeOffsets)
    ↓
返回 xpkObject
```

### 2. 写入流程（按字节分割）

```
xpkAppendData(xpk, data, size, level)
    ↓
检查 volume.enabled
    ↓ 否
正常写入（单卷模式）
    ↓ 是
计算写入后当前卷的位置
    ↓
检查是否超出 volumeSize ?
    ↓ 否
写入当前卷
    ↓ 是
计算剩余容量
    ↓
写入当前卷剩余部分
    ↓
创建新卷
    ↓
写入剩余数据到新卷
    ↓
更新文件信息的 dataOffset（可能是跨卷偏移）
```

### 3. 读取流程（跨卷）

```
xpkExtractData(xpk, pos, outSize)
    ↓
获取文件信息 (dataOffset, dataSize)
    ↓
计算目标卷：volumeIndex = xpkVolumeFromOffset(dataOffset)
    ↓
计算卷内偏移：offsetInVolume = dataOffset - volumeOffsets[volumeIndex]
    ↓
检查数据是否跨卷？
    ↓ 否
从目标卷读取完整数据
    ↓ 是
分段读取：
  1. 从当前卷读取到卷尾
  2. 从下一卷读取剩余部分
  3. 合并数据
    ↓
返回完整数据
```

### 4. 卷文件查找

```c
// 根据全局偏移计算所在的卷
int xpkVolumeFromOffset(xpkObject xpk, uint32_t globalOffset) {
    for (int i = 0; i < xpk->volume.currentVolume; i++) {
        if (globalOffset < xpk->volume.volumeOffsets[i + 1]) {
            return i;
        }
    }
    return xpk->volume.currentVolume - 1;
}

// 生成卷文件名
void xpkVolumeGetName(xpkObject xpk, int index, char* outName, size_t nameSize) {
    if (index == 0) {
        // 主卷使用原始名称
        strncpy(outName, xpk->volume.basePath, nameSize);
    } else {
        // 后续卷使用 .x01, .x02 等后缀
        char ext[4];
        strncpy(ext, xpk->volume.basePath + strlen(xpk->volume.basePath) - 3, 3);
        snprintf(outName, nameSize, "%.*s.%c%02d", 
                (int)(strlen(xpk->volume.basePath) - 4),
                xpk->volume.basePath,
                ext[0], index);
    }
}
```

### 5. 容量检查

```c
// 检查当前卷容量
int xpkVolumeCheckCapacity(xpkObject xpk, uint32_t dataSize) {
    if (!xpk->volume.enabled || xpk->volume.volumeSize == 0) {
        return 1;  // 无限制
    }
    
    uint32_t remaining = xpk->volume.volumeSize - xpk->volume.currentOffset;
    return (dataSize <= remaining) ? 1 : 0;
}

// 创建新卷
int xpkVolumeCreateNext(xpkObject xpk) {
    int nextIndex = xpk->volume.currentVolume + 1;
    if (nextIndex >= XPK_MAX_VOLUMES) {
        xpkSetError(12, "Maximum volume count exceeded");
        return -1;
    }
    
    char volPath[512];
    xpkVolumeGetName(xpk, nextIndex, volPath, sizeof(volPath));
    
    xpk->volume.volumes[nextIndex] = xrtOpen(volPath, 0, XRT_CP_BINARY);
    if (!xpk->volume.volumes[nextIndex]) {
        xpkSetError(1, "Failed to create volume");
        return -1;
    }
    
    // 写入头部
    xpkVolumeInfo volInfo = {
        .volumeCount = xpk->volume.currentVolume + 2,
        .volumeIndex = nextIndex
    };
    
    xrtPut(xpk->volume.volumes[nextIndex], &xpk->head, sizeof(xpkHead));
    xrtPut(xpk->volume.volumes[nextIndex], &volInfo, sizeof(volInfo));
    
    xpk->volume.volumeOpen[nextIndex] = 1;
    xpk->volume.currentVolume = nextIndex;
    xpk->volume.currentOffset = sizeof(xpkHead) + sizeof(xpkVolumeInfo);
    
    return 0;
}
```

### 6. 跨卷数据读取

```c
void* xpkVolumeReadData(xpkObject xpk, uint32_t globalOffset, 
                       uint32_t size, uint32_t* outSize) {
    uint8_t* buffer = xrtMalloc(size);
    if (!buffer) return NULL;
    
    uint32_t bytesRead = 0;
    uint32_t readOffset = globalOffset;
    
    while (bytesRead < size) {
        int volIndex = xpkVolumeFromOffset(xpk, readOffset);
        xfile* volFile = xpk->volume.volumes[volIndex];
        
        uint32_t volOffset = readOffset - xpk->volume.volumeOffsets[volIndex];
        uint32_t volEnd = (volIndex < xpk->volume.currentVolume - 1) 
            ? xpk->volume.volumeOffsets[volIndex + 1]
            : xpk->volume.totalSize;
        uint32_t volRemaining = volEnd - readOffset;
        uint32_t readSize = (size - bytesRead < volRemaining) 
            ? size - bytesRead 
            : volRemaining;
        
        xrtSeek(volFile, volOffset, XRT_SEEK_SET);
        size_t actualRead;
        xrtGet(volFile, readSize, &actualRead);
        
        memcpy(buffer + bytesRead, (uint8_t*)actualRead, readSize);
        bytesRead += readSize;
        readOffset += readSize;
    }
    
    *outSize = size;
    return buffer;
}
```

---

## 使用示例

### 示例 1：创建 100MB 分卷包

```c
#include "xpack.h"

int main() {
    xpkObject xpk = xpkOpen("backup.xpk", 0, 0);
    if (!xpk) {
        printf("Failed to create archive\n");
        return 1;
    }
    
    // 设置分卷模式
    xpkVolumeModeSet(xpk, 1);
    xpkVolumeSizeSet(xpk, 100 * 1024 * 1024);  // 100MB
    xpkVolumeSplitModeSet(xpk, 0);  // 按字节分割
    
    // 添加文件（会自动创建新卷）
    xpkAppendFile(xpk, "large_file1.bin", 7);
    xpkAppendFile(xpk, "large_file2.bin", 7);
    xpkAppendFile(xpk, "large_file3.bin", 7);
    
    // 保存并关闭
    xpkSave(xpk);
    xpkClose(xpk);
    
    // 结果：backup.xpk, backup.x01, backup.x02, ...
    
    return 0;
}
```

### 示例 2：读取分卷包

```c
#include "xpack.h"

int main() {
    // 只需打开主卷
    xpkObject xpk = xpkOpen("backup.xpk", 0, 1);
    if (!xpk) {
        printf("Failed to open archive\n");
        return 1;
    }
    
    // 检查是否为分卷
    if (xpkVolumeMode(xpk)) {
        printf("Multi-volume archive\n");
        printf("Total volumes: %d\n", xpkVolumeCount(xpk));
        
        // 获取统计信息
        xpkVolumeStat stat;
        xpkVolumeStatGet(xpk, &stat);
        printf("Total size: %llu bytes\n", stat.totalSize);
    }
    
    // 正常读取（无需关心分卷）
    int count = xpkCount(xpk);
    for (int i = 0; i < count; i++) {
        uint32_t size;
        void* data = xpkExtractData(xpk, i, &size);
        
        // 处理数据...
        printf("File %d: %u bytes\n", i, size);
        
        xpkFree(data);
    }
    
    xpkClose(xpk);
    return 0;
}
```

### 示例 3：按文件分割

```c
#include "xpack.h"

int main() {
    xpkObject xpk = xpkOpen("archive.xpk", 0, 0);
    
    // 启用按文件分割
    xpkVolumeModeSet(xpk, 1);
    xpkVolumeSizeSet(xpk, 500 * 1024 * 1024);  // 500MB
    xpkVolumeSplitModeSet(xpk, 1);  // 按文件分割
    
    // 每个文件会被完整地放入一个卷
    // 如果文件超过卷大小，会自动创建新卷
    xpkAppendFile(xpk, "file1.bin", 7);  // 放入卷1
    xpkAppendFile(xpk, "file2.bin", 7);  // 如果卷1空间不足，放入卷2
    
    xpkSave(xpk);
    xpkClose(xpk);
    
    return 0;
}
```

### 示例 4：查询分卷信息

```c
#include "xpack.h"

void printVolumeInfo(xpkObject xpk) {
    int volCount = xpkVolumeCount(xpk);
    printf("Total volumes: %d\n", volCount);
    
    for (int i = 0; i < volCount; i++) {
        printf("  Volume %d: %s\n", i, xpkVolumePath(xpk, i));
    }
    
    xpkVolumeStat stat;
    if (xpkVolumeStatGet(xpk, &stat) == 0) {
        printf("Total size: %llu bytes\n", stat.totalSize);
        printf("Average size: %.2f MB\n", stat.avgSize / 1024.0 / 1024.0);
        printf("\nPer volume:\n");
        for (int i = 0; i < volCount; i++) {
            printf("  Volume %d: %u bytes\n", i, stat.volumeSizes[i]);
        }
        xpkFree(stat.volumeSizes);
    }
}
```

---

## 兼容性说明

### 版本兼容性

| xPack 版本 | 分卷支持 | 说明 |
|-----------|---------|------|
| Ver 6.x | ❌ 不支持 | 无法打开分卷文件 |
| Ver 7.0 | ❌ 不支持 | 无法打开分卷文件 |
| Ver 7.1+ | ✅ 支持 | 可打开分卷文件 |

### 非分卷文件

- `volumeCount = 1`, `volumeIndex = 0`
- `flag.volumeMode = 0`
- 行为与 Ver 7.0 完全相同

### 分卷文件

- `volumeCount >= 1`, `volumeIndex = 0..N-1`
- `flag.volumeMode = 1`
- Ver 7.0 会忽略 volumeMode，可能导致未定义行为

---

## 错误处理

### 错误码扩展

| 错误码 | 含义 | 说明 |
|-------|------|------|
| 12 | 最大卷数超出 | 尝试创建超过 256 个卷 |
| 13 | 卷文件丢失 | 无法打开某个卷文件 |
| 14 | 卷数据不完整 | 跨卷读取时卷缺失 |
| 15 | 卷头验证失败 | 某个卷的头部信息无效 |

### 错误处理策略

| 错误场景 | 处理方式 |
|---------|---------|
| **创建新卷失败** | 回滚当前操作，保持原卷状态 |
| **读取时卷缺失** | 返回错误，不尝试部分读取 |
| **卷头验证失败** | 返回错误，关闭所有卷 |
| **磁盘空间不足** | 返回错误，已写入数据保留 |

---

## 性能考虑

### 卷大小建议

| 场景 | 推荐卷大小 | 说明 |
|-----|-----------|------|
| 网络传输 | 50-100 MB | 便于断点续传 |
| 光盘刻录 | 700 MB | CD 容量 |
| DVD 刻录 | 4.7 GB | 单层 DVD 容量 |
| 云存储 | 100-500 MB | 限制上传大小 |
| 本地存储 | 无限制 | 设置为 0 |

### 性能优化

1. **延迟打开卷** - 按需打开非主卷
2. **卷缓存** - 缓存最近使用的卷文件句柄
3. **批量读取** - 尽量减少跨卷边界读取
4. **并行写入** - 多卷写入时可考虑并行（未来版本）

### 性能开销

| 操作 | 单卷模式 | 分卷模式 | 开销 |
|-----|---------|---------|------|
| 顺序读取 | 1x | 1x | 0% |
| 跨卷读取 | N/A | ~1.05x | +5% |
| 随机读取 | 1x | ~1.1x | +10% |
| 顺序写入 | 1x | ~1.02x | +2% |
| 跨卷写入 | N/A | ~1.05x | +5% |

---

## 测试计划

### 单元测试

| 测试项 | 描述 |
|-------|------|
| 卷初始化 | 测试正确初始化分卷管理器 |
| 卷创建 | 测试创建新卷的逻辑 |
| 容量检查 | 测试容量检查和自动创建新卷 |
| 跨卷写入 | 测试数据跨多个卷写入 |
| 跨卷读取 | 测试从多个卷读取数据 |
| 卷关闭 | 测试正确关闭所有卷 |
| 错误恢复 | 测试各种错误情况的处理 |

### 集成测试

| 测试项 | 描述 |
|-------|------|
| 完整生命周期 | 创建 → 写入 → 保存 → 读取 → 关闭 |
| 多卷大文件 | 创建包含多个卷的大型文件 |
| 边界情况 | 测试卷大小边界、最大卷数等 |
| 并发访问 | 测试多线程环境下的安全性 |

### 兼容性测试

| 测试项 | 描述 |
|-------|------|
| 向后兼容 | 用新版本打开旧版文件 |
| 非分卷文件 | 确保单卷文件正常工作 |
| 跨平台 | Windows/Linux/macOS 测试 |

---

## 未来扩展

### 计划功能

- [ ] 卷加密支持
- [ ] 恢复记录（记录卷校验和）
- [ ] 卷合并工具
- [ ] 卷拆分工具
- [ ] 压缩感知分卷（根据压缩比调整）
- [ ] 并行写入支持

### 预留字段

- `xpkFlag.reserved`: 预留 20 位
- `xpkHead.reserved`: 预留 4 字节
- `xpkHead.discCode`: 可用于自定义卷标识

---

## 附录

### A. 文件扩展名对照表

| 原扩展名 | 分卷扩展名模式 |
|---------|---------------|
| `.xpk` | `.x01`, `.x02`, ... |
| `.pkg` | `.p01`, `.p02`, ... |
| `.zip` | `.z01`, `.z02`, ... |
| `.rar` | `.r01`, `.r02`, ... |
| `.7z` | `.701`, `.702`, ... |

### B. 常用卷大小常量

```c
#define XPK_VOL_SIZE_FLOPPY    (1400 * 1024)       // 1.44 MB 软盘
#define XPK_VOL_SIZE_CD        (700 * 1024 * 1024)  // 700 MB CD
#define XPK_VOL_SIZE_CD80     (800 * 1024 * 1024)  // 800 MB CD
#define XPK_VOL_SIZE_DVD       (4700 * 1024 * 1024) // 4.7 GB DVD
#define XPK_VOL_SIZE_DVD_DL    (8500 * 1024 * 1024) // 8.5 GB DVD DL
```

### C. 参考资料

- xPack Ver7 核心规范
- LZMA2 压缩算法文档
- ZSTD 压缩算法文档
- xRT 运行时库文档

---

**文档结束**
