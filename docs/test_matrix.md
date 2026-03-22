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
   `open / close`、空包创建、占位路径、目录路径、稀疏残卷入口保护，以及非分卷包 `Meta / Entry Table` 的映射式打开路径。
2. `tests/smoke/format_guard.inc.h`
   坏头、坏 Meta、坏 Entry Table、格式尺寸保护、坏格式打开失败。
3. `tests/smoke/core_mode.inc.h`
   默认 `Core` 模式、`infoExtSize`、`meta`、基础读写和基础查询。

## 4. Unit

1. `tests/unit/index_mode.inc.h`
   `Index` 模式增删改查、`fileIndex` 查找、重复索引保护、索引映射一致性，以及 `STORE + immediate + AddFile / UpdateFile` 包装层回归、超大源文件边界与 `IO_OPEN` 优先级。
2. `tests/unit/path_win32.inc.h`
   `Win32` 路径模式、大小写不敏感、斜杠规范化、重命名和属性修改，以及 `STORE + immediate + AddFile / UpdateFile` 包装层回归、超大源文件边界与 `IO_OPEN` 优先级。
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
   分卷虚拟 I/O、跨卷读写、分卷尾段、分卷 `save / build`、残卷处理，以及 `64-bit seek` / 超过 `4GB` 的大逻辑偏移分卷原始 I/O、分卷 `store` 大条目的分块直导出，以及分卷 `solid + STORE` 的 `verify / read / build` 回归。
4. `tests/integration/replace_rollback.inc.h`
   替换原包、`.replace.bak`、替换失败回滚、半提交清理、备份路径冲突，以及大尾段 `save rollback` 的临时文件回滚。
5. `tests/integration/solid_mode.inc.h`
   `solid` 数据读取、`solid <-> normal` build、`normal -> solid + STORE` 混合源条目重构、`solid + STORE -> normal` 原始切片直搬运、压缩 `solid` 下 `LZ4 / LZ4HC / ZSTD / LZMA2` 的单文件切片式 `ReadToMemory / Verify / ReadToFile` 与 `verifyAll`、压缩 `solid` 源 `build` 时去掉整条 `pSolidSrc` 原始流副本、`solid + STORE` 超大单文件校验，以及 solid 统计和 `verify`。
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

1. Linux 侧已经完成 Debian 13 主回归执行，但还没有做正式压测。
2. 接近 `4GB` 分卷上限和更大逻辑文件还缺真实压力测试。
3. 文件定位已覆盖 `4GB+`，普通未压缩条目和 buffered 未压缩条目导出也已支持分块直通，普通 `LZ4 / LZ4HC / ZSTD / LZMA2` 条目的 `ReadToFile` 也都已有专用路径，非分卷包 `Meta / Entry Table` 打开、普通非 `solid` 压缩条目的 `read / verify / verifyAll`、以及非分卷普通 `STORE` 条目与 `solid + STORE` 条目的单文件 `verify / verifyAll` 都已支持映射式路径，压缩 `solid` 下 `LZ4 / LZ4HC / ZSTD / LZMA2` 的单文件 `ReadToMemory / Verify / ReadToFile` 与 `verifyAll` 也已改成目标切片路径，`LZ4 / LZ4HC / ZSTD / LZMA2 + immediate/buffered + file` 也已改成流式文件输入，`LZ4 / LZ4HC + immediate/buffered + data` 已改成“临时文件可写映射直接生成压缩块，再写入包尾或精确读回压缩块写入队列”，`ZSTD / LZMA2 + immediate/buffered + data` 也已改成“内存分块流式压缩到临时文件，再写入包尾或精确读回压缩块写入队列”，并已补上 `Index / Path` 包装层的 buffered 压缩 data 持久化回归、`3MB+` 级 buffered `LZ4 / ZSTD` 与 `1MB+` 级 buffered `LZMA2` 大内存数据回归，以及 `2MB+ / 3MB+` 级 `solid ZSTD -> solid LZMA2` 大数据 `build / verifyAll / readToFile` 回归、12 文件多轮 `save / reopen / solid build / recompress build / unsolid build / volume build` 的 Windows 中压测回归、额外 5 轮 `reopen / verify / mode switch / build` 的循环稳定性回归，以及一条带 `update / rename / remove / solid / unsolid / volume / reopen` 的多阶段集成压测；目标 `solid` 为 `STORE / LZ4 / LZ4HC / ZSTD / LZMA2` 时的 `build` 也已改成“临时 raw 文件 -> 直接写包体或流式压缩 -> 写包体”，并且当前 `0-15` 映射下公开 `solid` 路径已经不再保留旧的通用整流 fallback；`AddFile / UpdateFile` 的超大源文件拒绝边界也已覆盖到 `Core / Index / Path`，普通非 solid 条目的 `xpkBuild` 也已支持直搬运现成压缩块，`solid + STORE -> normal` 的切片直搬运也已覆盖，但单块编解码和单文件数据块本身仍然是当前实现边界。
4. 当前仍是顺序大回归，没有按测试名粒度过滤执行的能力。

## 9. 过滤说明

当前稳定接受的过滤值有：

1. `all`
2. `smoke`
3. `smoke/open_core`
4. `unit`
5. `unit/index_path`
6. `integration`
7. `integration/build_volume`
8. `integration/solid_readonly`
9. `unit/index_path/direct`
10. `integration/build_volume/direct`
11. `integration/solid_readonly/direct`

这些过滤值采用阶段式累进执行：

1. `smoke` 只跑 `smoke`
2. `unit` 会跑 `smoke + unit`
3. `integration/build_volume` 会跑 `smoke + unit + integration/build_volume`
4. `integration/solid_readonly` 会跑 `smoke + unit + integration/build_volume + integration/solid_readonly`
5. `integration` 等价于当前完整回归
6. 以 `/direct` 结尾的过滤值只执行当前阶段本体
## 10. 2026-03-20 补充矩阵

1. `smoke/open_core`
新增覆盖“单卷包 + 超大 `STORE + immediate` + `xpkUpdateFile` 成功写入 + `Verify / VerifyAll / ReadToFile` 成功”。
2. `unit/index_path`
新增覆盖 `Index` 与 `Win32 Path` 两层下，超大单卷 `STORE + immediate` 的成功写入、状态校验与导出大小校验。
## 11. 2026-03-21 补充矩阵

1. `smoke/open_core`
新增覆盖“单卷包 + 超大 `STORE + immediate` + `xpkAddFile / xpkUpdateFile` 成功写入 + `Verify / VerifyAll / ReadToFile` 成功”。
2. `unit/index_path`
新增覆盖 `Index` 与 `Win32 Path` 两层下，超大单卷 `STORE + immediate` 的 `AddFile / UpdateFile` 成功写入、状态校验与导出大小校验。
## 12. 2026-03-21 追加矩阵

1. `smoke/open_core`
新增覆盖“单卷包 + `compLevel 6 + immediate + file` + 大文件 `xpkAddFile / xpkUpdateFile / Save / Reopen / VerifyAll / ReadToFile` 成功”。
