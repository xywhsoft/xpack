# xPack 压测说明

## 1. 目标

这份文档只说明 `xPack` 的压力测试入口、日志、历史记录和当前实测结果，不重复介绍普通功能回归。

当前压测主要覆盖：

1. `save -> reopen -> verifyAll -> readToFile`
2. `solid / unsolid / recompress / volume build`
3. `buffered / immediate`
4. `STORE / LZ4 / LZ4HC / ZSTD / LZMA2`
5. 多轮重复 `UpdateData / UpdateFile`


## 2. 直接入口

测试程序入口：

```bat
release\x64\xpack_test.exe integration/stress/direct
```

这个入口只跑 `stress` 阶段本身，不会重复执行 `smoke` 和 `unit`。


## 3. stress 轮次

`xpack_test` 现在支持第二个参数控制 `stress` 内层轮次：

```bat
release\x64\xpack_test.exe integration/stress/direct 2
release\x64\xpack_test.exe all 2
```

规则：

1. 未提供时默认 `5`
2. 也可以使用环境变量 `XPACK_TEST_STRESS_REPEAT`
3. 合法范围为 `1-1000`


## 4. Windows 压测脚本

推荐入口：

```bat
build_GCC_STRESS_x64.bat [outer_rounds] [filter] [inner_stress_repeat]
```

默认值：

```bat
outer_rounds = 5
filter = integration/stress/direct
inner_stress_repeat = 跟随 xpack_test 默认值 5
```

常用例子：

```bat
build_GCC_STRESS_x64.bat
build_GCC_STRESS_x64.bat 3
build_GCC_STRESS_x64.bat 2 integration/stress/direct 6
build_GCC_STRESS_x64.bat 1 all 2
```

日志文件：

```bat
release\x64\xpack_stress.log
release\x64\xpack_stress_YYYYMMDD_HHMMSS.log
```

锁文件：

```bat
release\x64\xpack_stress.lock
```

历史记录：

```bat
release\x64\xpack_stress_history.tsv
```

当前脚本行为：

1. 运行期间创建锁文件，阻止并发执行
2. 每次运行保留一份时间戳日志
3. 最后一次结果复制到 `release\x64\xpack_stress.log`
4. 追加一行历史记录到 `release\x64\xpack_stress_history.tsv`
5. 日志当前包含 `start_unix / end_unix / elapsed_seconds`


## 5. Linux 压测入口

Linux 侧现在也已经补了同结构脚本：

```sh
./build_stress.sh [outer_rounds] [filter] [inner_stress_repeat]
```

它和 Windows 脚本保持同样的参数、锁文件、时间戳日志和 history 结构。

当前状态：

1. `build_stress.sh` 已经写好
2. `bash -n build_stress.sh` 已通过
3. Debian 13 主回归已经完成，下一步是 Linux 压测


## 6. 当前建议

日常验证：

```bat
build_GCC_STRESS_x64.bat 1 integration/stress/direct 2
```

中等强度：

```bat
build_GCC_STRESS_x64.bat 2 integration/stress/direct 6
```

发布前长压：

```bat
build_GCC_STRESS_x64.bat 5 integration/stress/direct 10
```

如果要把 `smoke + unit + integration` 一起纳入长压，可以使用：

```bat
build_GCC_STRESS_x64.bat 2 all 3
build_GCC_STRESS_x64.bat 3 all 6
build_GCC_STRESS_x64.bat 2 all 8
build_GCC_STRESS_x64.bat 3 all 10
build_GCC_STRESS_x64.bat 4 all 12
build_GCC_STRESS_x64.bat 5 all 12
```


## 7. 当前状态

截至当前版本，已经实际跑通：

```bat
build_GCC_STRESS_x64.bat 1 integration/stress/direct 2
build_GCC_STRESS_x64.bat 2 integration/stress/direct 6
build_GCC_STRESS_x64.bat 3 integration/stress/direct 8
build_GCC_STRESS_x64.bat 1 all 3
build_GCC_STRESS_x64.bat 2 all 5
build_GCC_STRESS_x64.bat 3 all 6
build_GCC_STRESS_x64.bat 2 all 8
build_GCC_STRESS_x64.bat 3 all 10
build_GCC_STRESS_x64.bat 4 all 12
```

其中当前更重的一条已跑通记录是：

```bat
build_GCC_STRESS_x64.bat 5 all 12
```

最近一次这条命令的 `elapsed_seconds` 为 `1140`。

锁文件保护也已经验证过：

1. 手工创建 `release\x64\xpack_stress.lock`
2. 再启动 `build_GCC_STRESS_x64.bat`
3. 脚本会直接拒绝执行
4. 当前锁冲突返回码为 `3`
5. `history.tsv` 现在也会保留真实失败码，例如 `FAILED_2`


## 8. 还没完成的事

1. 还没有做更长时间的发布级连续压测记录
2. 还没有在 Linux 下完成正式压测回归
3. 单数据块与编解码本身仍然受当前 `block` 模型限制
