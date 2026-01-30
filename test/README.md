# xPack Ver7 - 测试目录

## 目录结构

```
test/
├── test_framework.h          # 测试框架头文件
├── test_framework.c          # 测试框架实现
├── test_main.c              # 主测试程序（包含所有测试用例）
├── run_tests.bat            # Windows 测试编译和运行脚本
├── run_tests.sh             # Linux 测试编译和运行脚本
└── 01_core_basic.c          # (保留的测试文件，用于参考)
```

## 快速开始

### Windows 系统

在项目根目录执行：

```bash
cd test
run_tests.bat
```

### Linux 系统

在项目根目录执行：

```bash
cd test
chmod +x run_tests.sh
./run_tests.sh
```

## 测试覆盖范围

当前测试套件包含以下测试类别：

### 1. Core 模式测试（3个测试）
- `core_create_empty` - 创建空包测试
- `core_append_data` - 追加数据测试
- `core_multiple_files` - 多文件操作测试

### 2. 压缩测试（1个测试）
- `compression_levels` - 所有压缩级别测试

### 3. Index 模式测试（1个测试）
- `index_mode` - 索引模式完整功能测试

### 4. Path 模式测试（2个测试）
- `path_mode_win32` - Windows 路径模式测试
- `path_mode_linux` - Linux 路径模式测试

### 5. 固实压缩测试（6个测试）
- `solid_mode_enable_disable` - 固实模式启用/禁用测试
- `solid_mode_multiple_files` - 多文件固实压缩测试
- `solid_mode_empty_files` - 空文件固实压缩测试
- `solid_mode_compression_ratio` - 压缩率对比测试
- `solid_mode_block_info` - 固实块信息测试
- `solid_mode_cannot_set_after_files` - 错误条件测试

### 6. 统计和校验测试（1个测试）
- `stat_and_verify` - 统计信息获取和校验测试

**总计：14个测试用例**

## 测试结果

所有测试应该通过，输出类似如下：

```
=================================================
  xPack Ver7 Test Suite
=================================================

[Core Mode Tests]
  Testing core_create_empty... PASSED
  Testing core_append_data... PASSED
  Testing core_multiple_files... PASSED

[Compression Tests]
  Testing compression_levels... PASSED

[Index Mode Tests]
  Testing index_mode... PASSED

[Path Mode Tests]
  Testing path_mode_win32... PASSED
  Testing path_mode_linux... PASSED

[Solid Compression Tests]
  Testing solid_mode_enable_disable... PASSED
  Testing solid_mode_multiple_files... PASSED
  Testing solid_mode_empty_files... PASSED
  Testing solid_mode_compression_ratio... PASSED
  Testing solid_mode_block_info... PASSED
  Testing solid_mode_cannot_set_after_files... PASSED

[Utility Tests]
  Testing stat_and_verify... PASSED

=================================================
  Results: 14 passed, 0 failed
=================================================

============================================================
  All tests passed!
============================================================
```

## 测试框架

测试框架提供了以下宏：

- `TEST(name)` - 定义测试函数
- `RUN_TEST(name)` - 运行测试
- `ASSERT(cond)` - 断言条件
- `ASSERT_EQ(a, b)` - 断言相等
- `ASSERT_NE(a, b)` - 断言不等
- `ASSERT_NULL(a)` - 断言为空
- `ASSERT_NOT_NULL(a)` - 断言非空
- `ASSERT_STR_EQ(a, b)` - 断言字符串相等

## 编译参数

测试使用以下编译参数：

- `-m64` - 64位编译
- `-O2` - 优化级别 2
- `-s` - 去除符号表
- `-DZ7_ST` - 静态链接 LZMA
- `-DDEBUG_TRACE` - 启用调试跟踪
- `-ffunction-sections -fdata-sections -Wl,--gc-sections` - 移除未使用的代码

## 故障排除

### 编译失败
确保 GCC 已安装并添加到 PATH 环境变量中。

### 运行时错误
确保所有依赖库（lib/xrt, lib/lz4, lib/zstd, lib/lzma）都存在。

### 测试失败
检查 `release/x64` 目录下的 `.xpk` 文件是否被清理干净。

## 未来扩展

可以根据需要在 `test_main.c` 中添加更多测试用例。遵循以下命名规范：

- 使用 `TEST(name)` 宏定义测试函数
- 使用 `RUN_TEST(name)` 宏运行测试
- 使用各种 `ASSERT_*` 宏进行断言

测试函数命名规范：`<category>_<description>`
