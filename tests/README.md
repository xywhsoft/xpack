# xPack 测试体系

## 1. 目标

`tests/` 目录负责承载 `xPack` 的独立测试入口、共享测试外壳，以及按主题拆分的顺序回归块。

当前已经完成的拆分包括：

1. 测试入口从 `xpack.c` 分离到 `tests/xpack_test_main.c`
2. 自测主体拆成 `smoke / unit / integration`
3. `smoke / unit / integration` 都已经继续细拆成专题 `*.inc.h`
4. `tests/test_helpers.h` 已形成基础 helper 层

## 2. 当前结构

```text
tests/
|- README.md
|- test_helpers.h
|- selftest_main.h
|- xpack_test_main.c
|- smoke/
|- unit/
`- integration/
```

其中：

1. `xpack_test_main.c`
   测试程序入口。
2. `selftest_main.h`
   共享变量、初始化、清理和顺序聚合。
3. `test_helpers.h`
   文件夹具、路径家族清理和错误断言 helper。
4. `smoke / unit / integration`
   按专题拆分的测试块。

## 3. 运行方式

Windows 下：

```bat
build_GCC_TEST_x64.bat
release\x64\xpack_test.exe
```

当前稳定可用的过滤值有：

```bat
release\x64\xpack_test.exe smoke
release\x64\xpack_test.exe smoke/open_core
release\x64\xpack_test.exe unit
release\x64\xpack_test.exe unit/index_path
release\x64\xpack_test.exe integration
release\x64\xpack_test.exe integration/build_volume
release\x64\xpack_test.exe integration/solid_readonly
```

也可以通过：

```bat
set XPACK_TEST_FILTER=smoke
release\x64\xpack_test.exe
```

传入过滤值。

## 4. 结构说明

### 4.1 smoke

最小主链、入口保护、基础 `Core` 行为。

### 4.2 unit

`Index / Path` 行为、lookup 一致性、参数和错误优先级。

### 4.3 integration

`save / build / reopen / rollback / volume / solid / readonly` 等跨模块主链。

## 5. 当前限制

1. 测试程序仍然是单个顺序回归，不是多进程或按用例粒度运行。
2. 当前过滤入口采用阶段式累进执行，不是完全隔离的独立子测试进程。
3. Linux 回归和超大文件压力测试还未正式开始。

## 6. 配套文档

1. `docs/testing.md`
   测试架构、helper 约定和运行说明。
2. `docs/test_matrix.md`
   功能到测试文件的覆盖索引。
3. `docs/development.md`
   工程结构、构建方式和开发流程说明。
