# Smoke 测试

`smoke/` 负责最小主链和入口级保护，目标是尽快暴露“包打不开、格式不对、最基本的 Core 行为跑不通”这类问题。

当前文件：

1. `selftest_open_core.inc.h`
   `smoke` 聚合入口。
2. `runtime_open.inc.h`
   runtime 自检、`xpkOpen`、`createIfMissing`、目录路径、稀疏残卷入口保护。
3. `format_guard.inc.h`
   坏头、坏格式和打开保护。
4. `core_mode.inc.h`
   默认 `Core` 包、`meta`、`infoExt`、基础读写和状态保护。

适合放进 `smoke` 的测试：

1. 入口能否正常运行。
2. 基础格式是否能被正确识别。
3. 最小 `Core` 工作流是否可用。
4. 明显坏包是否会被及时拒绝。
