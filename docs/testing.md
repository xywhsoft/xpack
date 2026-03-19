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

### 3.4 integration

用于验证 `save / build / reopen / rollback / volume / solid` 这类跨模块主链路：

1. `tests/integration/build_compact.inc.h`
2. `tests/integration/codec_levels.inc.h`
3. `tests/integration/volume_mode.inc.h`
4. `tests/integration/replace_rollback.inc.h`
5. `tests/integration/solid_mode.inc.h`
6. `tests/integration/layout_state.inc.h`
7. `tests/integration/readonly_guard.inc.h`

## 4. 当前 helper

`tests/test_helpers.h` 当前已经提供这些基础 helper：

1. `procTestWriteBinaryFile`
   写入二进制测试夹具。
2. `procTestFileContentEquals`
   读取并比对文件内容。
3. `procTestExpectLastError`
   校验 `last error`。
4. `procTestCheckLastError`
   在需要保留两段失败编号时校验 `last error`。
5. `procTestExpectCallError`
   统一“返回码 + last error”断言。
6. `procTestCheckCallError`
   在需要保留三段失败编号时统一“返回码 + last error”断言。
7. `procTestExpectNullResultError`
   统一“空返回值 + last error”断言。
8. `procTestCheckNullResultError`
   在需要保留三段失败编号时统一“空返回值 + last error”断言。
9. `procTestExpectFalseError`
   统一“布尔假返回 + last error”断言。
10. `procTestCheckFalseError`
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

当前稳定接受的过滤值只有：

1. `all`
2. `smoke`
3. `smoke/open_core`

也可以通过环境变量传入：

```bat
set XPACK_TEST_FILTER=smoke
release\x64\xpack_test.exe
```

### 7.3 关于 unit / integration

虽然测试目录已经按 `unit` 和 `integration` 拆分完成，但当前命令行过滤器还没有把它们作为稳定独立入口开放。也就是说：

1. `unit` 和 `integration` 当前是内部组织层次，不是稳定 CLI 过滤值。
2. 如果改动落在 `unit` 或 `integration`，当前仍建议直接跑完整回归。

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
2. Linux 侧还没有正式进入回归。
3. 超大文件和接近 `4GB` 分卷上限还缺压力测试。
4. helper 层已经成型，但目录夹具、坏包夹具和更多断言场景仍可以继续收口。

## 10. 后续建议

当前最值得继续做的三件事：

1. 扩展 `tests/test_helpers.h`，补路径占位、坏包样本和目录夹具 helper。
2. 继续压缩重复错误断言样板。
3. 在这套结构稳定后，再开始 Linux 和大文件边界验证。
