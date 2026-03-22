# xPack Windows 状态

## 1. 当前结论

截至 `2026-03-21`，`xPack` 在 Windows 下已经进入“核心能力可用于受控生产场景”的状态。

当前已经稳定打通这些主链路：

1. `open / close / save / build`
2. `meta / infoExt / flag / stat / verify / each / eachMatch`
3. `Core / Index / Linux / Win32`
4. `read / add / update / remove / rename / attr`
5. `solid / volume / buffered / immediate`
6. `4GB+` 文件定位、超大逻辑偏移分卷原始读写
7. 普通条目、`solid` 条目、分卷条目在 Windows 下的主要回归链路


## 2. 已完成的关键能力

### 2.1 文件定位与大偏移

1. 文件定位已经覆盖 `4GB+`
2. 分卷原始 I/O 已验证超过 `4GB` 的逻辑偏移
3. 大尾段 `xpkSave()` 失败回滚已改成“小尾段内存快照 + 大尾段临时文件回滚”


### 2.2 普通包低峰值路径

1. 普通未压缩条目与 buffered 未压缩条目导出已支持分块直通
2. 普通 `LZ4 / LZ4HC / ZSTD / LZMA2` 条目的 `ReadToFile` 已有专用路径
3. 非分卷普通压缩条目的 `read / verify / verifyAll` 已支持“映射压缩块 -> 直接解码”
4. 非分卷包 `Meta / Entry Table` 打开已支持映射式尾段解码
5. 普通非 `solid` 条目的 `xpkBuild` 已支持直接搬运现成压缩块


### 2.3 文件输入写入路径

1. `STORE + immediate + file` 已改成分块直写包尾
2. `LZ4 / LZ4HC + immediate/buffered + file` 已改成“源文件分块写临时 raw 文件，再压缩/回退”
3. `ZSTD / LZMA2 + immediate/buffered + file` 已改成“源文件分块压缩到临时文件，再择优写入包尾或写入队列”
4. `LZ4 / LZ4HC + immediate/buffered + data` 已改成“临时文件可写映射直接生成压缩块，再写入包尾或精确读回压缩块写入队列”
5. `ZSTD / LZMA2 + immediate/buffered + data` 已改成“内存分块流式压缩到临时文件，再写入包尾或精确读回压缩块写入队列”
6. `Index / Path` 包装层也已覆盖 buffered 压缩 data 的 `save / reopen / verifyAll / readToFile`
7. 已补 `3MB+` 级 buffered `LZ4 / ZSTD` 与 `1MB+` 级 buffered `LZMA2` 大内存数据回归
8. `Core / Index / Path` 三层都已覆盖超大源文件拒绝边界


### 2.4 Solid 低峰值路径

1. `solid + STORE` 的单文件 `ReadToMemory / Verify / ReadToFile / VerifyAll` 已有低峰值路径
2. 压缩 `solid` 下 `LZ4 / LZ4HC / ZSTD / LZMA2` 的单文件 `ReadToMemory / Verify / ReadToFile` 已改成按目标切片处理
3. 压缩 `solid` 下 `LZ4 / LZ4HC / ZSTD / LZMA2` 的 `verifyAll` 已改成按条目切片校验
4. `solid + STORE -> normal` 已支持原始切片直搬运
5. 目标 `solid` 为 `STORE / LZ4 / LZ4HC / ZSTD / LZMA2` 时，`build` 已改成“逐条目写临时 raw 文件 -> 直接写包体或流式压缩 -> 写包体”
6. 压缩 `solid` 作为源时，`build -> normal` 和 `build -> solid` 的主路径已经去掉整条 `pSolidSrc` 原始流副本


## 3. Windows 验证命令

最近一轮通过的命令：

```bat
build_header.bat
build_GCC_TEST_x64.bat
build_GCC_DLL_x64.bat
release\x64\xpack_test.exe smoke/open_core
release\x64\xpack_test.exe unit/index_path
release\x64\xpack_test.exe integration/build_volume
release\x64\xpack_test.exe integration/build_volume/direct
release\x64\xpack_test.exe integration/solid_readonly
release\x64\xpack_test.exe integration/solid_readonly/direct
release\x64\xpack_test.exe all
```

当前也已经补了独立的 Windows 压测入口：

```bat
build_GCC_STRESS_x64.bat
```

它会先重建测试程序，再执行多轮 `integration/stress/direct`，并把日志写到 `release\x64\xpack_stress.log`。


## 4. 当前剩余硬边界

当前剩余的实现难点，已经主要收敛到一类主边界和两类工程收尾：

1. 单文件数据块与整块编解码仍然受当前 block 模型限制
2. Windows 下还没有做发布级长时间压力测试
3. Linux 侧主回归已通过，但 Linux 压测还没有正式开始

更具体地说：

1. `LZ4 / LZ4HC / ZSTD / LZMA2 + immediate/buffered + file` 的输入侧已经流式化，`LZ4 / LZ4HC / ZSTD / LZMA2 + immediate/buffered + data` 也都已去掉整块压缩输出堆缓冲或整块压缩输出堆副本，但单文件数据块本身与编解码输入大小仍然受当前 block 模型约束
2. 当前 `0-15` 压缩级别映射下，公开支持的 `solid` 路径已经都落到专用低峰值分支，旧的通用整流 fallback 已经清理掉；剩下的限制主要还是单数据块和编解码本身，而不是额外的 `solid` 通用兜底路径
3. Windows 下虽然功能回归已经很强，但还没有做足够长时间的发布级压力测试；当前已新增一条 12 文件多轮 `save / reopen / solid build / recompress build / unsolid build / volume build` 的中压测回归、额外 5 轮 `reopen / verify / mode switch / build` 的循环稳定性回归、一条带 `update / rename / remove / solid / unsolid / volume / reopen` 的多阶段集成压测，并额外提供了 `build_GCC_STRESS_x64.bat` 作为独立压测入口；截至当前版本，已实测跑通 `build_GCC_STRESS_x64.bat 3 all 6`、`build_GCC_STRESS_x64.bat 2 all 8`、`build_GCC_STRESS_x64.bat 3 all 10` 与 `build_GCC_STRESS_x64.bat 5 all 12`，并且压测脚本已支持时间戳日志、latest 副本、history 记录和耗时摘要
4. Linux 侧已经完成 Debian 13 主回归，剩余工作主要是 Linux 压测与更大压力边界验证


## 5. 后续建议

如果继续按当前顺序推进，建议优先级如下：

1. 继续压低单数据块与编解码本身的 block 峰值
2. 做 Windows 大包压力测试
3. 在 Windows 长压继续推进的同时，补 Linux 压测与更大压力边界验证
