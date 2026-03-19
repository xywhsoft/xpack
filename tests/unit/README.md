# Unit 测试

`unit/` 负责验证单一模式、单一规则或单一 lookup 保护，重点是把模式级行为和参数优先级讲清楚。

当前文件：

1. `selftest_index_path.inc.h`
   `unit` 聚合入口。
2. `index_mode.inc.h`
   `Index` 模式增删改查、`fileIndex` 查找、索引映射一致性。
3. `path_win32.inc.h`
   `Win32 Path` 模式、大小写不敏感、分隔符规范化、重命名和属性修改。
4. `path_lookup_guard.inc.h`
   `Path / Index` lookup 损坏、坏路径条目、坏格式保护和重建行为。
5. `path_linux.inc.h`
   `Linux Path` 模式和路径匹配差异。

适合放进 `unit` 的测试：

1. 某一种包类型的独立行为。
2. 单一路径规则或单一 lookup 保护。
3. 参数优先级或错误优先级。
4. 不依赖 `save / build / reopen` 的模式级逻辑。
