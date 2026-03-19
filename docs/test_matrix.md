# xPack 测试覆盖矩阵

## 1. 目的

这份文档回答两个问题：

1. 某条能力当前主要由哪个测试块覆盖。
2. 新增回归时应该优先追加到哪个测试文件。

## 2. 顶层入口

1. `tests/xpack_test_main.c`
   测试程序入口。
2. `tests/selftest_main.h`
   顺序聚合 `smoke / unit / integration`。
3. `tests/test_helpers.h`
   公共 helper 层。

## 3. Smoke

1. `tests/smoke/runtime_open.inc.h`
   `open / close`、空包创建、占位路径、目录路径、稀疏残卷入口保护。
2. `tests/smoke/format_guard.inc.h`
   坏头、坏 Meta、坏 Entry Table、格式尺寸保护、坏格式打开失败。
3. `tests/smoke/core_mode.inc.h`
   默认 `Core` 模式、`infoExtSize`、`meta`、基础读写和基础查询。

## 4. Unit

1. `tests/unit/index_mode.inc.h`
   `Index` 模式增删改查、`fileIndex` 查找、重复索引保护、索引映射一致性。
2. `tests/unit/path_win32.inc.h`
   `Win32` 路径模式、大小写不敏感、斜杠规范化、重命名和属性修改。
3. `tests/unit/path_lookup_guard.inc.h`
   `Path / Index` lookup 一致性、坏 lookup、坏路径条目、查询层坏格式保护。
4. `tests/unit/path_linux.inc.h`
   `Linux` 路径模式、大小写敏感和路径规范化差异。

## 5. Integration

1. `tests/integration/build_compact.inc.h`
   `xpkBuild`、空间回收、默认与自定义 `tempPath`、占位文件或目录、临时构建清理。
2. `tests/integration/codec_levels.inc.h`
   压缩级别映射、`STORE / LZ4 / LZ4HC / ZSTD / LZMA2`、压缩回退到 `STORE`。
3. `tests/integration/volume_mode.inc.h`
   分卷虚拟 I/O、跨卷读写、分卷尾段、分卷 `save / build`、残卷处理。
4. `tests/integration/replace_rollback.inc.h`
   替换原包、`.replace.bak`、替换失败回滚、半提交清理、备份路径冲突。
5. `tests/integration/solid_mode.inc.h`
   `solid` 数据读取、`solid <-> normal` build、solid 统计和 `verify`。
6. `tests/integration/layout_state.inc.h`
   `solid / volume` 目标布局与已应用布局状态机、`save / build` 限制。
7. `tests/integration/readonly_guard.inc.h`
   只读保护、错误优先级、参数优先级、`packType mismatch` 和只读下的 `build / save` 限制。

## 6. 按功能反查

1. `open / close`：先看 `tests/smoke/runtime_open.inc.h`
2. 坏格式打开：先看 `tests/smoke/format_guard.inc.h`
3. `Core` 与 `infoExt`：先看 `tests/smoke/core_mode.inc.h`
4. `Index` 行为：先看 `tests/unit/index_mode.inc.h`
5. `Win32 / Linux Path` 差异：先看 `tests/unit/path_win32.inc.h` 和 `tests/unit/path_linux.inc.h`
6. lookup 坏状态保护：先看 `tests/unit/path_lookup_guard.inc.h`
7. `save / build` 紧凑化：先看 `tests/integration/build_compact.inc.h`
8. 编解码：先看 `tests/integration/codec_levels.inc.h`
9. 分卷：先看 `tests/integration/volume_mode.inc.h`
10. 替换与回滚：先看 `tests/integration/replace_rollback.inc.h`
11. `solid`：先看 `tests/integration/solid_mode.inc.h`
12. 布局状态机：先看 `tests/integration/layout_state.inc.h`
13. 只读与错误优先级：先看 `tests/integration/readonly_guard.inc.h`

## 7. 新增测试时的放置规则

1. 只验证入口是否能跑通，放 `smoke`。
2. 只验证某一种模式、lookup 或某组参数优先级，放 `unit`。
3. 涉及 `save / build / reopen / rollback / volume / solid / disk replace`，放 `integration`。
4. 如果问题跨越多个模块，但最终关注的是磁盘结果和回滚安全，也优先放 `integration`。

## 8. 当前已知空白

1. Linux 侧还没有正式回归执行。
2. 接近 `4GB` 分卷上限和更大逻辑文件还缺真实压力测试。
3. 当前仍是顺序大回归，没有按测试名粒度过滤执行的能力。

## 9. 过滤说明

当前稳定接受的过滤值只有：

1. `all`
2. `smoke`
3. `smoke/open_core`

`unit` 和 `integration` 当前已经完成目录拆分，但仍然只作为覆盖索引和内部组织层，不作为稳定独立入口对外承诺。
