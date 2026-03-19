# Integration 测试

`integration/` 负责覆盖跨模块主链路，重点验证磁盘结果、回滚安全和布局切换。

当前文件：

1. `selftest_build_volume.inc.h`
   `build / codec / volume / replace` 聚合入口。
2. `build_compact.inc.h`
   `xpkBuild`、空间回收、临时路径占位和构建清理。
3. `codec_levels.inc.h`
   压缩级别映射、编解码回归和压缩回退。
4. `volume_mode.inc.h`
   分卷虚拟 I/O、跨卷读写、分卷 `save / build` 和残卷处理。
5. `replace_rollback.inc.h`
   替换原包、失败回滚、备份路径冲突和半提交清理。
6. `selftest_solid_readonly.inc.h`
   `solid / readonly / layout` 聚合入口。
7. `solid_mode.inc.h`
   `solid` 建包、读取、删除、元数据和重构。
8. `layout_state.inc.h`
   布局状态机、`solid / volume` 切换和尺寸断言。
9. `readonly_guard.inc.h`
   只读保护、内部坏状态、错误优先级和收尾回归。

适合放进 `integration` 的测试：

1. 涉及 `save / build / reopen / rollback` 的主链路。
2. 涉及 `volume` 或 `solid` 的磁盘行为。
3. 涉及替换原包、备份路径、失败清理或布局切换。
4. 涉及多个模块协同、最终以磁盘结果和状态一致性为判断标准的问题。
