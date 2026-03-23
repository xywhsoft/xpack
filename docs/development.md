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

Windows 压测入口：

```bat
build_GCC_STRESS_x64.bat
```

默认会先重建 `release\x64\xpack_test.exe`，再运行 `5` 轮 `integration/stress/direct`，并把输出写到：

```text
release\x64\xpack_stress.log
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

当前已完成 Debian 13 正式主回归，并已进入连续压测阶段：

```text
build.sh
build_test.sh
build_stress.sh
./build_test.sh
release/x64/xpack_test integration/build_volume
release/x64/xpack_test integration/solid_readonly
release/x64/xpack_test all
./build_stress.sh 1 integration/stress/direct 2
./build_stress.sh 1 all 2
./build_stress.sh 1 all 3
./build_stress.sh 2 all 3
./build_stress.sh 2 all 5
./build_stress.sh 2 all 8
./build_stress.sh 2 all 10
./build_stress.sh 3 all 6
./build_stress.sh 3 all 8
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
8. `integration/stress`
9. `integration/solid_readonly`
10. `unit/index_path/direct`
11. `integration/build_volume/direct`
12. `integration/stress/direct`
13. `integration/solid_readonly/direct`

## 7. 当前明确边界

目前实现和测试在 Windows 下已经比较稳定，但仍有这些边界：

1. Debian 13 侧已经完成正式主回归，并已实测跑通 `./build_test.sh`、`release/x64/xpack_test integration/build_volume`、`release/x64/xpack_test integration/solid_readonly`、`release/x64/xpack_test all`、`./build_stress.sh 1 integration/stress/direct 2`、`./build_stress.sh 1 all 2`、`./build_stress.sh 1 all 3`、`./build_stress.sh 2 all 3`、`./build_stress.sh 2 all 5`、`./build_stress.sh 2 all 8`、`./build_stress.sh 2 all 10`、`./build_stress.sh 3 all 6` 与 `./build_stress.sh 3 all 8`。
2. Linux 压测阶段暴露出的 `xrtFileCopy()` 成功路径 FD 泄漏已经修复；`xpkBuild()` 在 Debian 13 下的多轮 replace/copy 不再持续抬升文件描述符。
3. 接近 `4GB` 分卷上限和更大逻辑文件还缺真实压力验证。
4. 当前文件定位已经支持 `4GB+`，`xpkOpen()` 在非分卷包上也已支持直接映射压缩 `Meta / Entry Table` 尾段再解码；`xpkReadToFile` 对普通未压缩条目、buffered 的未压缩条目、普通 `LZ4 / LZ4HC / ZSTD / LZMA2` 条目，以及 `solid` 单文件导出都已接通专用路径，非分卷普通 `STORE` 条目与 `solid + STORE` 条目的单文件 `verify / verifyAll` 现在也都已支持映射式路径，普通非 `solid` 压缩条目的 `xpkReadToMemory / xpkVerify / xpkVerifyAll` 也已经接通“映射压缩块 -> 直接解码”路径；对压缩 `solid`，`LZ4 / LZ4HC / ZSTD / LZMA2` 的单文件 `ReadToMemory / Verify / ReadToFile` 已经改成只保留目标切片，不再先展开整条 solid 原始流，`verifyAll` 也已经降到按条目切片校验，而“压缩 `solid` 作为源 -> build 到普通包 / 重新 build 到 solid 包”这两条主路径，也已经去掉了整条 `pSolidSrc` 原始流的额外内存副本。`xpkAddFile / xpkUpdateFile` 的 `STORE + immediate` 已经在 `Core / Index / Path` 三层接通直写文件路径，`LZ4 / LZ4HC / ZSTD / LZMA2 + immediate/buffered + file` 现在也都已改成“源文件分块写入临时 raw 或临时压缩文件，再择优写入包尾或写入队列”的路径，不再依赖整块源文件映射；其中 `LZ4 / LZ4HC + immediate/buffered + data` 已经改成“临时文件可写映射直接生成压缩块，再写入包尾或精确读回写入队列”，`ZSTD / LZMA2 + immediate/buffered + data` 也都已经接通“内存分块流式压缩到临时文件，再写入包尾或精确读回写入队列”的路径，不再额外保留整块压缩输出缓冲。目标 `solid` 为 `STORE / LZ4 / LZ4HC / ZSTD / LZMA2` 的 `build` 现在也已经改成“逐条目写临时 raw 文件，再直接写包体或流式压缩到临时文件后写入包体”的路径，不再需要整块 `pSolidRaw`，其中 `ZSTD / LZMA2` 目标也都不再需要整块压缩输出缓冲；同时当前 `0-15` 压缩级别映射下，旧的通用 `solid` 整流 fallback 已经清理掉，公开支持路径都会直接落到这些专用分支。`Core / Index / Path` 的 `AddFile / UpdateFile` 也都已覆盖“超大源文件 -> block-limit 拒绝且旧状态保持不变”的回归，`xpkBuild` 对普通非 `solid` 条目也已支持直接搬运现成压缩块，`normal -> solid + STORE` 在源条目本来就是 `STORE` 时也会优先直搬运原始块，`solid + STORE -> normal` 现在也已支持直接按切片搬运原始块；在满足“已是 `solid`、无删除洞、压缩级别不变”时，`solid` build 也会直接搬原始压缩块，但单文件数据块和整块编解码本身仍然受当前 block 模型限制。
5. 测试体系虽然已经完成结构拆分，但仍然是“单测试程序中的顺序大回归”。
6. 统一 helper 层已经成型，但坏包夹具和更多断言场景还可以继续收敛。

## 8. 当前阶段建议

如果继续往下推进，优先级建议是：

1. 继续扩展 `tests/test_helpers.h`，把重复夹具和断言继续收口。
2. 继续补 API 使用示例、构建差异说明和验证说明。
3. 在 Windows 与 Linux 主回归都已经跑通的基础上，继续补 Linux 压测和更大文件边界验证。
## 9. 2026-03-20 补充

1. 单卷包下，`STORE + immediate` 的 `xpkAddFile / xpkUpdateFile` 现在已经支持超大源文件分块直写到包尾，再对包内范围计算 hash，不再依赖整文件映射源文件。
2. 这条能力已经覆盖 `Core / Index / Path` 三层公开 API；超大单卷 `STORE + immediate` 的 `AddFile / UpdateFile / Verify / VerifyAll / ReadToFile` 都已有 Windows 回归保护。
3. 当前真正还没流式化的主边界，已经进一步收敛到“单文件数据块与整块编解码本身仍然主要依赖当前 block 模型”，而公开支持的 `solid` 压缩级别路径已经不再保留旧的通用整流 fallback。
## 10. 2026-03-21 补充

1. 单卷包下，`Core / Index / Path` 三层的超大 `STORE + immediate` `AddFile / UpdateFile` 现在都已经有成功路径回归。
2. 这些回归不只验证写入成功，还会继续验证 `Verify / VerifyAll / ReadToFile` 的结果一致性。
