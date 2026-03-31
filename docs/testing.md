# xPack 测试说明

## 1. 目标

这份文档说明当前测试体系的组织方式、运行方式，以及新增测试时应遵守的约定。

当前测试体系的设计目标是：

1. 保持 Windows 下的完整顺序回归可以直接运行。
2. 让测试代码和库默认入口解耦。
3. 把大体量自测拆成按主题维护的文件。
4. 在不引入额外测试框架的前提下，逐步补齐统一 helper。

配套覆盖索引见 `docs/test_matrix.md`。

## 2. 当前架构

当前测试入口链路如下：

```text
tests/xpack_test_main.c
  -> 定义 XPACK_SELF_TEST_MAIN
  -> 包含 xpack.c
     -> 包含 tests/test_helpers.h
     -> 包含 tests/selftest_main.h
        -> 顺序包含 smoke / unit / integration 聚合块
           -> 聚合块再包含更细的专题 inc 文件
```

这意味着：

1. 整个测试程序仍然是单个翻译单元。
2. 所有测试块共享同一组局部变量和辅助函数。
3. 继续拆分时优先通过 `#include` 聚合，不急着引入新的测试框架。

## 3. 目录分层

### 3.1 顶层外壳

1. `tests/xpack_test_main.c`
   测试程序入口。
2. `tests/selftest_main.h`
   共享变量、初始化、清理和顺序聚合。
3. `tests/test_helpers.h`
   统一测试 helper 层。

### 3.2 smoke

用于验证最小主链和入口级保护：

1. `tests/smoke/runtime_open.inc.h`
2. `tests/smoke/format_guard.inc.h`
3. `tests/smoke/core_mode.inc.h`

### 3.3 unit

用于验证单一模式、单一规则或单一 lookup 保护：

1. `tests/unit/index_mode.inc.h`
2. `tests/unit/path_win32.inc.h`
3. `tests/unit/path_lookup_guard.inc.h`
4. `tests/unit/path_linux.inc.h`

其中 `index_mode.inc.h` 与 `path_win32.inc.h` 现在也覆盖了 `STORE + immediate + AddFile / UpdateFile` 的包装层回归，以及超大源文件在 `block-limit`、目的包路径 `IO_OPEN` 等边界下的错误优先级，用来验证 `Core / Index / Path` 三层都会命中新接入的文件直写路径，并在失败时保住旧数据。

### 3.4 integration

用于验证 `save / build / reopen / rollback / volume / solid` 这类跨模块主链路：

1. `tests/integration/build_compact.inc.h`
2. `tests/integration/codec_levels.inc.h`
3. `tests/integration/volume_mode.inc.h`
4. `tests/integration/replace_rollback.inc.h`
5. `tests/integration/solid_mode.inc.h`
6. `tests/integration/layout_state.inc.h`
7. `tests/integration/readonly_guard.inc.h`

其中 `tests/integration/volume_mode.inc.h` 当前还覆盖：

1. 基于新版 `xrt` 的 `64-bit seek`、超过 `4GB` 的文件定位，以及 `INT64_MAX` 以上 seek 保护回归
2. 超过 `4GB` 的大逻辑偏移分卷原始读写，以及分卷 `store` 大条目的分块直导出
3. 大尾段 `save rollback` 的临时文件回滚路径
4. 大结果导出的分块写路径、普通未压缩条目和 buffered 未压缩条目的分块直导出路径、普通 `LZ4 / LZ4HC / ZSTD / LZMA2` 条目的专用 `ReadToFile` 路径、非分卷普通压缩条目的映射式 `read / verify / verifyAll` 路径、非分卷包 `Meta / Entry Table` 尾段的映射式打开路径、非分卷大 `STORE` 与 `solid + STORE` 条目的映射式 `verify` 路径、`solid` 条目的直接切片导出路径、压缩 `solid` 下 `LZ4 / LZ4HC / ZSTD / LZMA2` 的单文件切片式 `ReadToMemory / Verify / ReadToFile` 路径、压缩 `solid` 下 `LZ4 / LZ4HC / ZSTD / LZMA2` 的按条目切片式 `verifyAll` 路径、压缩 `solid` 作为源时去掉整条 `pSolidSrc` 原始流副本的 `build` 路径、目标 `solid` 为 `STORE / LZ4 / LZ4HC / ZSTD / LZMA2` 时“临时 raw 文件 -> 直接写包体或流式压缩 -> 写包体”的低峰值 `build` 路径、并确认当前 `0-15` 映射下公开 `solid` 路径不再落回旧的通用整流 fallback、普通非 `solid` 条目的 `xpkBuild` 直搬运路径、`normal -> solid + STORE` 时源 `STORE` 条目的原始块直搬运路径、`solid + STORE -> normal` 时源切片的原始块直搬运路径、`LZ4 / LZ4HC / ZSTD / LZMA2 + immediate/buffered + file` 的流式文件输入路径、`LZ4 / LZ4HC + immediate/buffered + data` 的临时文件映射低峰值写入路径、`ZSTD / LZMA2 + immediate/buffered + data` 的临时压缩文件低峰值写入路径、`Index / Path` 包装层对 buffered 压缩 data 的持久化路径、`3MB+` 级 buffered `LZ4 / ZSTD` 与 `1MB+` 级 buffered `LZMA2` 大内存数据回归、`2MB+ / 3MB+` 级 `solid ZSTD -> solid LZMA2` 大数据 `build / verifyAll / readToFile` 回归，以及一条 12 文件多轮 `save -> reopen -> solid build -> recompress build -> unsolid build -> volume build` 的 Windows 中压测回归、5 轮 `reopen -> verify -> mode switch -> build -> verify` 的循环稳定性回归，再加一条带 `update / rename / remove / solid / unsolid / volume / reopen` 的多阶段集成压测，还有保守条件下 `solid` build 的原始压缩块直搬运路径，以及超大单文件源输入在 `AddFile / UpdateFile` 上的 `block-limit` 边界

## 4. 当前 helper

`tests/test_helpers.h` 当前已经提供这些基础 helper：

1. `procTestWriteBinaryFile`
   写入二进制测试夹具。
2. `procTestWriteVolumeBinaryFile`
   向某个分卷路径族写入分卷夹具文件。
3. `procTestCreateDirOccupy`
   创建目录占位，用于验证 `open / build` 的路径冲突保护。
4. `procTestDeletePathFamily`
   统一清理单文件、目录、连续分卷和稀疏残卷。
5. `procTestFileContentEquals`
   读取并比对文件内容。
6. `procTestExpectLastError`
   校验 `last error`。
7. `procTestCheckLastError`
   在需要保留两段失败编号时校验 `last error`。
8. `procTestExpectCallError`
   统一“返回码 + last error”断言。
9. `procTestCheckCallError`
   在需要保留三段失败编号时统一“返回码 + last error”断言。
10. `procTestExpectNullResultError`
   统一“空返回值 + last error”断言。
11. `procTestCheckNullResultError`
   在需要保留三段失败编号时统一“空返回值 + last error”断言。
12. `procTestExpectFalseError`
   统一“布尔假返回 + last error”断言。
13. `procTestCheckFalseError`
   在需要保留三段失败编号时统一“布尔假返回 + last error”断言。

## 5. Helper 使用规则

新增或重构测试时，优先按下面规则选 helper：

1. 默认先用 `procTestExpect*` 系列。
2. 只有必须保留“返回值错 / 错误码错 / 错误文本错”分层失败编号时，才用 `procTestCheck*`。
3. 如果测试只关心 `xpkLastError()`，不关心错误文本，就把 `sErrorText` 传 `NULL`。
4. 如果测试明确要求空字符串错误文本，才把 `sErrorText` 传 `""`。

`procTestCheck*` 的返回值约定为：

1. `1` = 返回值或结果本身错误
2. `2` = `xpkLastError()` 错误
3. `3` = 错误文本错误

## 6. 如何新增测试

推荐按下面顺序新增：

1. 先判断属于 `smoke / unit / integration` 哪一层。
2. 如果现有专题文件足够接近，就直接追加到对应文件。
3. 如果现有专题文件已经过长或主题明显不一致，再新建更细的 `*.inc.h`。
4. 在对应聚合入口里追加新的 `#include`。
5. 跑 Windows 回归。

## 7. 运行方式

### 7.1 默认完整回归

```bat
build_GCC_TEST_x64.bat
release\x64\xpack_test.exe
```

### 7.2 当前稳定可用的过滤值

当前稳定接受的过滤值有：

1. `all`
2. `smoke`
3. `smoke/open_core`
4. `unit`
5. `unit/index_path`
6. `integration`
7. `integration/large_io`
8. `integration/build_volume`
9. `integration/stress`
10. `integration/solid_readonly`
11. `unit/index_path/direct`
12. `integration/build_volume/direct`
13. `integration/stress/direct`
14. `integration/solid_readonly/direct`

也可以通过环境变量传入：

```bat
set XPACK_TEST_FILTER=smoke
release\x64\xpack_test.exe
```

### 7.3 关于 unit / integration

`unit` 和 `integration` 当前已经作为稳定过滤入口开放，但语义是阶段式累进执行，而不是彼此完全隔离。也就是说：

1. `smoke` 只跑 `smoke`。
2. `unit` 会跑 `smoke + unit`。
3. `integration/large_io` 会跑 `smoke + unit +` 显式启用的 `4GB+` 大文件与大偏移长时回归。
4. `integration/build_volume` 会跑 `smoke + unit + integration/build_volume`。
5. `integration/solid_readonly` 会跑 `smoke + unit + integration/build_volume + integration/solid_readonly`。
6. `integration` 等价于当前完整回归。
7. `integration/stress` 会跑 `smoke + unit + integration/build_volume`，并把 `integration/build_volume` 整轮重复 `5` 次。
8. 以 `/direct` 结尾的过滤值只执行当前阶段本体，不再自动带上前置阶段。

### 7.4 Windows 压测脚本

如果需要做一轮独立的 Windows 压测，可以直接运行：

```bat
build_GCC_STRESS_x64.bat
```

默认行为是：

1. 先执行 `build_GCC_TEST_x64.bat`
2. 再运行 `5` 轮 `integration/stress/direct`
3. 把完整输出记录到 `release\x64\xpack_stress.log`

也可以手工指定轮数或过滤值：

```bat
build_GCC_STRESS_x64.bat 3 integration/stress/direct
```

## 8. Windows 验证清单

如果改动涉及实现、公开头、构建脚本或测试入口，建议至少执行：

```bat
build_header.bat
build_GCC_TEST_x64.bat
release\x64\xpack_test.exe
build_GCC_DLL_x64.bat
```

然后再补一次头文件 smoke compile。

如果本轮只改测试和文档，最小验证集可以缩成：

```bat
build_GCC_TEST_x64.bat
release\x64\xpack_test.exe
```

## 9. 当前边界

当前测试体系已经完成结构拆分，但仍保留这些边界：

1. 仍然是顺序大回归，不支持按用例粒度过滤执行。
2. Linux 侧已经完成 Debian 13 主回归，并且已经开始正式压测；当前已实测跑通到 `./build_stress.sh 2 all 10` 与 `./build_stress.sh 3 all 8`，同时 `xrtFileCopy()` 在 Linux 成功路径的 FD 泄漏也已修复，但还没有做更长时间的连续压测。
3. 超大文件和接近 `4GB` 分卷上限还缺压力测试。
4. 文件定位已覆盖 `4GB+`，普通未压缩条目和 buffered 未压缩条目导出也已支持分块直通，非分卷普通压缩条目的 `read / verify / verifyAll` 与非分卷包 `Meta / Entry Table` 打开也已接通映射式路径，非分卷普通 `STORE` 条目与 `solid + STORE` 条目的单文件 `verify / verifyAll` 也已支持映射式路径，压缩 `solid` 下 `LZ4 / LZ4HC / ZSTD / LZMA2` 的单文件 `ReadToMemory / Verify / ReadToFile` 与 `verifyAll` 也已改成按目标切片路径，`LZ4 / LZ4HC / ZSTD / LZMA2 + immediate/buffered + file` 也已经改成流式文件输入，`LZ4 / LZ4HC + immediate/buffered + data` 已改成“临时文件可写映射直接生成压缩块，再写入包尾或精确读回写入队列”，`ZSTD / LZMA2 + immediate/buffered + data` 也已改成“内存分块流式压缩到临时文件，再写入包尾或精确读回写入队列”，目标 `solid` 为 `STORE / LZ4 / LZ4HC / ZSTD / LZMA2` 时的 `build` 也已改成“临时 raw 文件 -> 直接写包体或流式压缩 -> 写包体”，并且当前公开支持的 `solid` 压缩级别路径已经不再保留旧的通用整流 fallback；`AddFile / UpdateFile` 在 `Core / Index / Path` 三层的超大源文件拒绝边界也已覆盖，普通非 solid 条目的 `xpkBuild` 也已支持直搬运现成压缩块，`solid + STORE -> normal` 的原始切片直搬运也已补齐，但单块编解码和单文件数据块本身仍然是当前实现边界。
5. helper 层已经成型，但目录夹具、坏包夹具和更多断言场景仍可以继续收口。

## 10. 后续建议

当前最值得继续做的三件事：

1. 扩展 `tests/test_helpers.h`，补路径占位、坏包样本和目录夹具 helper。
2. 继续压缩重复错误断言样板。
3. 在这套结构稳定后，继续补 Linux 压测和更大文件边界验证。
## 11. 2026-03-20 新增回归

1. `tests/smoke/core_mode.inc.h`
普通单卷包下，超大 `STORE + immediate` 的 `xpkUpdateFile` 现在会验证成功写入、`xpkVerify / xpkVerifyAll` 成功，以及 `xpkReadToFile` 导出后的目标文件大小正确。
2. `tests/unit/index_mode.inc.h`
`Index` 模式下，超大单卷 `STORE + immediate` 的 `xpkIndexUpdateFile` 已改成成功路径回归，并验证 `ReadToFile` 导出大小。
3. `tests/unit/path_win32.inc.h`
`Win32 Path` 模式下，超大单卷 `STORE + immediate` 的 `xpkPathAddFile / xpkPathUpdateFile` 已改成成功路径回归，并验证 `PathReadToFile` 导出大小。
## 12. 2026-03-21 补充

1. `tests/smoke/core_mode.inc.h`
现在同时覆盖 `Core` 下超大单卷 `STORE + immediate` 的 `xpkAddFile` 与 `xpkUpdateFile` 成功路径。
2. `tests/unit/index_mode.inc.h`
现在同时覆盖 `Index` 下超大单卷 `STORE + immediate` 的 `xpkIndexAddFile` 与 `xpkIndexUpdateFile` 成功路径。
