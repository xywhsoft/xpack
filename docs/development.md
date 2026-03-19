# xPack 开发说明

## 1. 当前工程形态

`xPack` 当前采用“单实现入口 + 生成头文件 + 独立测试入口”的结构：

1. `xpack.c`
   唯一实现入口。负责聚合 `xrt`、压缩库和 `src/` 下的实现模块。
2. `xpack.h`
   生成产物，不手工维护。
3. `src/api/public_decl.h`
   公开声明源文件，由头文件生成器读取。
4. `tests/xpack_test_main.c`
   测试程序入口。
5. `tests/selftest_main.h`
   顺序回归主外壳，承载共享变量、初始化和统一清理。
6. `tests/test_helpers.h`
   测试辅助层，统一承载文件夹具和错误断言 helper。

默认情况下：

1. 未定义 `XPACK_SELF_TEST_MAIN` 时，`xpack.c` 只保留空 `main`。
2. 定义 `XPACK_NO_MAIN` 时，`xpack.c` 不生成任何入口，适合 DLL 或嵌入式集成。

## 2. 关键文件职责

### 2.1 实现入口

`xpack.c` 是唯一正式实现入口，负责：

1. 引入 `lib/xrt.h` 与 `lz4 / zstd / lzma`。
2. 聚合 `src/` 下全部实现模块。
3. 在非测试构建下提供默认空 `main`。

### 2.2 公开头文件

`xpack.h` 是生成产物，声明源来自：

1. `src/api/public_decl.h`

头文件生成工具位于：

1. `tools/make_header/make_header.c`

### 2.3 测试入口

`tests/xpack_test_main.c` 只负责：

1. 定义 `XPACK_SELF_TEST_MAIN`
2. 包含 `xpack.c`

真正的测试主体分为两层：

1. `tests/selftest_main.h`
   共享变量、初始化、统一清理、顺序聚合 `smoke / unit / integration`
2. `tests/test_helpers.h`
   文件写入、文件内容校验、`last error` 校验、以及组合断言 helper

## 3. 测试目录结构

当前测试目录已经按主题拆分：

```text
tests/
|- README.md
|- test_helpers.h
|- selftest_main.h
|- xpack_test_main.c
|- smoke/
|  |- README.md
|  |- selftest_open_core.inc.h
|  |- runtime_open.inc.h
|  |- format_guard.inc.h
|  `- core_mode.inc.h
|- unit/
|  |- README.md
|  |- selftest_index_path.inc.h
|  |- index_mode.inc.h
|  |- path_win32.inc.h
|  |- path_lookup_guard.inc.h
|  `- path_linux.inc.h
`- integration/
   |- README.md
   |- selftest_build_volume.inc.h
   |- build_compact.inc.h
   |- codec_levels.inc.h
   |- volume_mode.inc.h
   |- replace_rollback.inc.h
   |- selftest_solid_readonly.inc.h
   |- solid_mode.inc.h
   |- layout_state.inc.h
   `- readonly_guard.inc.h
```

这意味着：

1. 测试程序入口已经和库默认入口解耦。
2. 大块顺序自测已经拆成按主题组织的 `*.inc.h` 文件。
3. 测试 helper 已经形成单独基础层，后续继续扩展时优先在 `tests/test_helpers.h` 收口。

## 4. 构建脚本

### 4.1 Windows

共享库：

```bat
build_GCC_DLL_x64.bat
```

输出：

```text
release\x64\xpack.dll
```

测试程序：

```bat
build_GCC_TEST_x64.bat
```

输出：

```text
release\x64\xpack_test.exe
```

头文件生成：

```bat
build_header.bat
```

输出：

```text
xpack.h
```

### 4.2 Linux

当前已同步脚本，但尚未进入正式回归验证：

```text
build.sh
build_test.sh
```

## 5. 推荐开发流程

### 5.1 完整 Windows 回归

如果本轮改动涉及实现、公开头、构建脚本或测试入口，推荐按下面顺序执行：

```bat
build_header.bat
build_GCC_TEST_x64.bat
release\x64\xpack_test.exe
build_GCC_DLL_x64.bat
```

然后再补一次头文件 smoke compile。

### 5.2 轻量验证

如果本轮改动只落在测试文件或测试文档，可以先走轻量验证：

```bat
build_GCC_TEST_x64.bat
release\x64\xpack_test.exe
```

确认测试入口和行为正常后，再决定是否补跑 DLL 和头文件生成。

## 6. 当前测试覆盖

Windows 下当前顺序回归已经覆盖这些主链路：

1. `open / close / save / build`
2. `meta / infoExt / flag / stat / verify / each / eachMatch`
3. `Core / Index / Linux / Win32`
4. `read / add / update / remove / rename / attr`
5. `solid / volume / buffered / immediate`
6. 错误优先级、坏包保护、lookup 一致性、失败回滚与路径占位保护
7. 基于新版 `xrt` 的 `64-bit seek`、超过 `4GB` 的文件定位、`INT64_MAX` 以上 seek 保护，以及大逻辑偏移分卷原始读写回归
8. 大尾段 `xpkSave()` 失败回滚，已支持临时文件落盘而不是整块内存快照

当前测试程序也已经支持这些稳定过滤入口：

1. `all`
2. `smoke`
3. `smoke/open_core`
4. `unit`
5. `unit/index_path`
6. `integration`
7. `integration/build_volume`
8. `integration/solid_readonly`

## 7. 当前明确边界

目前实现和测试在 Windows 下已经比较稳定，但仍有这些边界：

1. Linux 侧还没有正式回归。
2. 接近 `4GB` 分卷上限和更大逻辑文件还缺真实压力验证。
3. 当前文件定位已经支持 `4GB+`，`xpkReadToFile` 对普通未压缩条目也已支持分块直导出，但单文件数据块、整块编解码和 `solid` 整流仍然受当前 block 模型限制。
4. 测试体系虽然已经完成结构拆分，但仍然是“单测试程序中的顺序大回归”。
5. 统一 helper 层已经成型，但坏包夹具和更多断言场景还可以继续收敛。

## 8. 当前阶段建议

如果继续往下推进，优先级建议是：

1. 继续扩展 `tests/test_helpers.h`，把重复夹具和断言继续收口。
2. 继续补 API 使用示例、构建差异说明和验证说明。
3. 等 Windows 侧完全收口后，再开始 Linux 和大文件边界验证。
