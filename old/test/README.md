# xPack Ver7 测试系统

## 概述

xPack Ver7 测试系统提供了一个统一的测试框架，可以全面测试 xPack 库的所有功能。系统包含 33 个测试模块，每个模块对应一个功能类别，总共包含数百个测试用例。

## 文件结构

```
test/
├── test_framework.h                    # 统一测试框架头文件
├── coverage_framework.h                # 覆盖率测试框架
├── benchmark_framework.h              # 基准测试框架
├── xpack_full_test_runner.c           # 完整测试运行器（33个模块）
├── xpack_stable_test.c                # 稳定性测试运行器
├── xpack_benchmark_runner.c           # 性能基准测试运行器
├── build_all_tests.bat                # 完整测试套件构建脚本
├── build_stable_test.bat              # 稳定性测试构建脚本
├── build_benchmark.bat                # 基准测试构建脚本
├── 01_core_basic.h                    # 核心基本功能测试
├── 02_core_operations.h              # 核心操作测试
├── 03_core_edge_cases.h              # 核心边界情况测试
├── 04_index_operations.h              # 索引模式测试
├── 05_path_operations.h               # 路径模式测试
├── 06_path_case_sensitivity.h         # 路径大小写测试
├── 07_compression_data_patterns.h     # 压缩数据模式测试
├── 08_compression_accuracy.h          # 压缩准确性测试
├── 09_compression_large_files.h       # 大文件压缩测试
├── 10_solid_compression.h             # 固实压缩测试
├── 11_error_handling.h                # 错误处理测试
├── 12_batch_operations.h              # 批量操作测试
├── 13_traverse_operations.h           # 遍历操作测试
├── 14_statistics.h                    # 统计功能测试
├── 15_verify_operations.h             # 验证操作测试
├── 16_rebuild_operations.h            # 重建操作测试
├── 17_package_properties.h            # 包属性测试
├── 18_file_type.h                     # 文件类型测试
├── 19_save_load_cycles.h              # 保存/加载周期测试
├── 20_multiple_packages.h             # 多包操作测试
├── 21_edge_large_files.h              # 大文件边界测试
├── 22_edge_many_files.h               # 多文件边界测试
├── 23_concurrent_access.h             # 并发访问测试
├── 24_corruption_recovery.h           # 损坏恢复测试
├── 25_cross_platform.h                # 跨平台测试
├── 26_memory_management.h             # 内存管理测试
├── 27_integration_real_world.h        # 真实世界集成测试
├── 28_performance_benchmark.h         # 性能基准测试
├── 29_compression_ratio.h             # 压缩比率测试
├── 30_regression_tests.h              # 回归测试
├── 31_volume_basic.h                 # Volume基础功能测试
├── 32_volume_single_file.h           # Volume单文件测试
├── 33_volume_cross.h                 # Volume跨卷操作测试
└── README.md                           # 本文档
```

## 测试类别

| 类别 | 编号 | 说明 | 测试数量 |
|------|------|------|---------|
| Core | 01-03 | 核心功能 | 16+ |
| Index | 04 | 索引操作 | 6 |
| Path | 05-06 | 路径操作 | 12 |
| Compression | 07-10 | 压缩算法 | 15 |
| Error | 11 | 错误处理 | 24 |
| Batch | 12 | 批量操作 | 8 |
| Traverse | 13 | 遍历操作 | 6 |
| Stats | 14 | 统计功能 | 5 |
| Verify | 15 | 验证操作 | 5 |
| Rebuild | 16 | 重建操作 | 4 |
| Properties | 17 | 属性操作 | 4 |
| FileType | 18 | 文件类型 | 4 |
| Cycle | 19 | 循环操作 | 3 |
| Multiple | 20 | 多包操作 | 4 |
| Edge | 21-22 | 边界情况 | 8 |
| Concurrent | 23 | 并发访问 | 4 |
| Recovery | 24 | 错误恢复 | 5 |
| Platform | 25 | 跨平台 | 4 |
| Memory | 26 | 内存管理 | 5 |
| Integration | 27 | 集成测试 | 6 |
| Performance | 28 | 性能测试 | 5 |
| Ratio | 29 | 压缩比 | 4 |
| Regression | 30 | 回归测试 | 6 |
| Volume | 31-33 | Volume功能 | 7 |

**总计**: 33个测试模块，150+ 测试用例

## 快速开始

### 1. 构建测试程序

#### 完整测试套件（推荐）

包含所有 33 个测试模块：

```batch
cd d:\Git\xPack
test\build_all_tests.bat
```

#### 稳定性测试

```batch
cd d:\Git\xPack
test\build_stable_test.bat
```

#### 基准测试

```batch
cd d:\Git\xPack
test\build_benchmark.bat
```

### 2. 运行测试

#### 完整测试

```batch
cd release\x64
xpack_full_test.exe
```

#### 稳定性测试

```batch
cd release\x64
xpack_stable_test.exe
```

#### 基准测试

```batch
cd release\x64

# 默认运行（5次，控制台输出）
xpack_benchmark.exe

# 运行10次，输出为CSV格式
xpack_benchmark.exe -r 10 -o 1

# 运行3次，输出为JSON格式，自定义文件名
xpack_benchmark.exe -r 3 -o 2 -f my_report
```

### 3. 查看报告

测试完成后会生成报告文件：
- **完整测试**：控制台输出详细结果
- **稳定性测试**：生成分类统计报告
- **基准测试**：生成CSV/JSON格式的性能报告

## 测试框架 API

### 测试宏定义

```c
#define TEST(name) void test_##name(void)
```

定义一个测试函数。

```c
#define TEST_REGISTER(name, category, description)
```

注册一个测试到测试套件。

### 断言宏

| 宏 | 说明 |
|----|------|
| `ASSERT(cond)` | 通用断言 |
| `ASSERT_EQ(a, b)` | 相等断言 |
| `ASSERT_NE(a, b)` | 不等断言 |
| `ASSERT_LT(a, b)` | 小于断言 |
| `ASSERT_LE(a, b)` | 小于等于断言 |
| `ASSERT_GT(a, b)` | 大于断言 |
| `ASSERT_GE(a, b)` | 大于等于断言 |
| `ASSERT_NULL(a)` | NULL 断言 |
| `ASSERT_NOT_NULL(a)` | 非 NULL 断言 |
| `ASSERT_STR_EQ(a, b)` | 字符串相等断言 |

### xPack API

| 函数 | 说明 |
|------|------|
| `xpkObject xpkOpen(path, ro, type)` | 打开/创建包 |
| `int xpkSave(xpk)` | 保存包 |
| `void xpkClose(xpk)` | 关闭包 |
| `uint32_t xpkAppendData(xpk, data, size, level)` | 添加数据 |
| `uint32_t xpkAppendFile(xpk, path, level)` | 添加文件 |
| `int xpkUpdateData(xpk, pos, data, size, level)` | 更新数据 |
| `int xpkUpdateFile(xpk, pos, path, level)` | 更新文件 |
| `int xpkRemove(xpk, pos)` | 删除条目 |
| `void* xpkExtractData(xpk, pos, outSize)` | 提取数据 |
| `int xpkExtractFile(xpk, pos, dstPath)` | 提取文件 |
| `uint32_t xpkCount(xpk)` | 获取条目数量 |
| `uint32_t xpkInfoSize(xpk, pos)` | 获取原始大小 |
| `uint32_t xpkInfoPacked(xpk, pos)` | 获取压缩大小 |
| `int xpkInfoLevel(xpk, pos)` | 获取压缩级别 |
| `int xpkInfoType(xpk, pos)` | 获取文件类型 |
| `int xpkInfoTypeSet(xpk, pos, type)` | 设置文件类型 |
| `uint32_t xpkInfoHash(xpk, pos)` | 获取哈希值 |
| `void* xpkInfo(xpk, pos)` | 获取完整信息 |
| `int xpkSolidMode(xpk)` | 获取固实模式 |
| `int xpkSolidModeSet(xpk, enable)` | 设置固实模式 |
| `int xpkVolumeMode(xpk)` | 获取分卷模式 |
| `int xpkVolumeModeSet(xpk, enable)` | 设置分卷模式 |
| `uint32_t xpkVolumeSize(xpk)` | 获取分卷大小 |
| `int xpkVolumeSizeSet(xpk, size)` | 设置分卷大小 |
| `int xpkVolumeSplitMode(xpk)` | 获取分卷拆分模式 |
| `int xpkVolumeSplitModeSet(xpk, mode)` | 设置分卷拆分模式 |
| `int xpkType(xpk)` | 获取包类型 |
| `int xpkLastError()` | 获取最后错误 |
| `void xpkFree(ptr)` | 释放内存 |

### Volume API

| 函数 | 说明 |
|------|------|
| `xpkVolume xpkVolumeCreate(xpk)` | 创建Volume |
| `xpkVolume xpkVolumeOpen(xpk, index)` | 打开Volume |
| `int xpkVolumeClose(vol)` | 关闭Volume |
| `uint32_t xpkVolumeGetCount(vol)` | 获取Volume条目数 |
| `uint32_t xpkVolumeGetPosition(vol, volIndex)` | 获取Volume位置 |
| `int xpkVolumeAppendData(vol, data, size, level)` | 添加数据到Volume |
| `int xpkVolumeAppendFile(vol, path, level)` | 添加文件到Volume |
| `void* xpkVolumeExtractData(vol, volIndex, outSize)` | 从Volume提取数据 |
| `int xpkVolumeExtractFile(vol, volIndex, dstPath)` | 从Volume提取文件 |

## 添加新测试

### 1. 创建测试函数

```c
TEST(my_new_test) {
    xpkObject xpk = xpkOpen("test_my_new.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    xpkAppendData(xpk, "Test data", 9, 6);
    
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);
}
```

### 2. 注册测试

```c
void register_my_tests(void) {
    TEST_REGISTER(my_new_test, CAT_CORE, "Test my new feature");
}
```

### 3. 在测试运行器中包含

```c
#include "my_tests.h"

int main(int argc, char* argv[]) {
    // ... 初始化代码 ...
    
    register_my_tests();
    
    // ... 运行测试 ...
}
```

## 测试报告格式

### 完整测试报告

```
=================================================
  xPack Ver7 - Full Test Suite
=================================================

Running tests from: 01_core_basic.h
Running tests from: 02_core_operations.h
...
Running tests from: 33_volume_cross.h

Summary:
  Total Tests: 150
  Passed:      145
  Failed:      5
  Duration:    45.2 seconds

Results by Category:
  Core                : 20 passed,  0 failed (total: 20)
  Index               :  6 passed,  0 failed (total:  6)
  Path                : 12 passed,  1 failed (total: 13)
  Compression         : 15 passed,  2 failed (total: 17)
  Error               : 24 passed,  0 failed (total: 24)
  ...
  Volume              :  7 passed,  0 failed (total:  7)

FAILED - Some tests did not pass!
=================================================
```

### 稳定性测试报告

```
=================================================
  xPack Ver7 - Stability Test Suite
=================================================

Summary:
  Total Tests: 50
  Passed:      48
  Failed:      2
  Duration:    20.1 seconds

Results by Category:
  Core                : 10 passed,  0 failed (total: 10)
  Index               :  6 passed,  0 failed (total:  6)
  Path                : 12 passed,  0 failed (total: 12)
  Volume              :  7 passed,  2 failed (total:  9)
  ...
=================================================
```

### 基准测试报告

#### 控制台输出

```
===================================
xPack Ver7 - Performance Benchmark
===================================

Benchmark: LZ4 Compression
  Run 1: 150MB in 120ms (1250 MB/s)
  Run 2: 150MB in 118ms (1271 MB/s)
  ...
  Average: 150MB in 119ms (1260 MB/s)
  Min: 118ms, Max: 125ms, Median: 119ms
```

#### CSV格式

```csv
Benchmark,Run,Size_MB,Time_ms,Speed_MB/s
LZ4 Compression,1,150,120,1250
LZ4 Compression,2,150,118,1271
...
```

#### JSON格式

```json
{
  "benchmark": "LZ4 Compression",
  "runs": [
    {"run": 1, "size_mb": 150, "time_ms": 120, "speed_mbps": 1250},
    {"run": 2, "size_mb": 150, "time_ms": 118, "speed_mbps": 1271}
  ],
  "stats": {
    "average_time_ms": 119,
    "min_time_ms": 118,
    "max_time_ms": 125,
    "median_time_ms": 119
  }
}
```

## 基准测试说明

基准测试程序 `xpack_benchmark_runner.c` 提供以下功能：

### 支持的基准测试

| 基准测试 | 说明 | 数据量 |
|----------|------|--------|
| **LZ4压缩** | 测试LZ4算法压缩性能 | 100MB |
| **ZSTD压缩** | 测试ZSTD算法压缩性能 | 100MB |
| **LZMA2压缩** | 测试LZMA2算法压缩性能 | 100MB |
| **解压缩** | 测试解压缩性能 | 100MB |
| **文件操作** | 测试批量文件操作性能 | 1000个文件 |

### 命令行选项

```
xpack_benchmark_runner [options]

  -r <count>     每个基准测试运行次数（默认：5）
  -o <format>    输出格式：0=控制台, 1=CSV, 2=JSON（默认：0）
  -f <filename>   输出文件前缀（默认：benchmark_report）
  -h             显示帮助信息
```

### 使用示例

```batch
# 默认运行（5次，控制台输出）
release\x64\xpack_benchmark.exe

# 运行10次，输出为CSV格式
release\x64\xpack_benchmark.exe -r 10 -o 1

# 运行3次，输出为JSON格式，自定义文件名
release\x64\xpack_benchmark.exe -r 3 -o 2 -f my_report
```

## 编译警告修复

本测试系统已修复所有编译警告，包括：

### 修复的警告类型

1. **数组越界警告** - `xpack_stable_test.c`
   - 修改 `MAX_CATEGORIES` 从 10 到 11
   - 确保数组访问在有效范围内

2. **指针与整数比较警告** - `05_path_operations.h`, `06_path_case_sensitivity.h`, `11_error_handling.h`
   - 使用 `ASSERT_NULL` / `ASSERT_NOT_NULL` 替代 `ASSERT_EQ(..., 0/-1)`
   - 正确处理API返回的指针类型

3. **字符串溢出警告** - `11_error_handling.h`
   - 修正数组边界计算，预留足够空间给字符串

## 测试模块详情

### 模块 01-03: 核心功能

- **01_core_basic.h**: 基础创建、添加、保存、加载操作
- **02_core_operations.h**: 更新、删除、提取操作
- **03_core_edge_cases.h**: 边界情况和异常处理

### 模块 04-06: 索引和路径

- **04_index_operations.h**: 索引模式的添加、查找、更新、删除
- **05_path_operations.h**: 路径模式的Win32/Linux路径处理
- **06_path_case_sensitivity.h**: 路径大小写敏感性测试

### 模块 07-10: 压缩功能

- **07_compression_data_patterns.h**: 不同数据模式的压缩测试
- **08_compression_accuracy.h**: 压缩解压缩准确性验证
- **09_compression_large_files.h**: 大文件压缩测试
- **10_solid_compression.h**: 固实压缩模式测试

### 模块 11: 错误处理

- **11_error_handling.h**: 24种错误场景测试，包括：
  - 无效参数
  - 文件不存在
  - 重复条目
  - 路径过长
  - 错误的包类型
  - 无效的操作

### 模块 12-20: 高级功能

- **12_batch_operations.h**: 批量添加、删除、提取、更新
- **13_traverse_operations.h**: 遍历包内容
- **14_statistics.h**: 统计信息获取
- **15_verify_operations.h**: 包完整性验证
- **16_rebuild_operations.h**: 索引重建
- **17_package_properties.h**: 包属性获取和设置
- **18_file_type.h**: 文件类型处理
- **19_save_load_cycles.h**: 多次保存加载周期
- **20_multiple_packages.h**: 多包同时操作

### 模块 21-23: 边界和并发

- **21_edge_large_files.h**: 大文件边界测试
- **22_edge_many_files.h**: 多文件边界测试
- **23_concurrent_access.h**: 并发访问测试

### 模块 24-27: 恢复和集成

- **24_corruption_recovery.h**: 损坏包恢复
- **25_cross_platform.h**: 跨平台兼容性
- **26_memory_management.h**: 内存泄漏检测
- **27_integration_real_world.h**: 真实场景集成测试

### 模块 28-30: 性能和回归

- **28_performance_benchmark.h**: 性能基准测试
- **29_compression_ratio.h**: 压缩比率测试
- **30_regression_tests.h**: 已知问题回归测试

### 模块 31-33: Volume功能

- **31_volume_basic.h**: Volume基础操作
- **32_volume_single_file.h**: 单文件Volume操作
- **33_volume_cross.h**: 跨Volume操作

## 已知问题

1. **部分API未完全实现**: 某些测试模块使用的API可能尚未完全实现，需要在 `xpack.h` 中添加或完善。

2. **并发测试**: 模块23的并发访问测试依赖于线程库，在某些平台上可能不可用。

3. **跨平台测试**: 模块25的跨平台测试需要在不同操作系统上运行以验证兼容性。

## 框架对比

### 旧测试框架 (backup/)

- 使用自定义的C测试宏
- 每个测试为独立的.c文件
- 缺乏统一的组织和报告

### 新测试框架 (test/)

- 统一的 `TEST()` 和 `TEST_REGISTER()` 宏
- 所有测试为.h文件，通过测试运行器包含
- 分类统计和详细报告
- 支持覆盖率测试和基准测试

## 转换说明

原有的 01-30 个测试文件已从 `backup/` 目录转换到 `test/` 目录：

- 所有测试使用统一的 `TEST` 宏定义
- 所有测试使用统一的断言宏
- 所有测试通过 `TEST_REGISTER` 注册到测试套件
- 测试按类别组织，便于统计和报告
- 新增了31-33的Volume测试模块

## 构建选项

### 完整测试套件

包含所有 33 个测试模块：

```batch
test\build_all_tests.bat
```

输出: `release\x64\xpack_full_test.exe`

### 稳定性测试

```batch
test\build_stable_test.bat
```

输出: `release\x64\xpack_stable_test.exe`

### 基准测试

```batch
test\build_benchmark.bat
```

输出: `release\x64\xpack_benchmark.exe`

## 许可证

本测试系统遵循 xPack Ver7 的许可证。
