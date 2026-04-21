# xpkgui xPack API 覆盖说明

本文按 `xpack.h` 的公开 API 分组记录 GUI 当前覆盖情况，用于后续新增 API 时快速检查是否需要接入 GUI。

## 已接入

- 包对象管理：`xpkOpen`、`xpkClose`、`xpkSave`、`xpkBuild`。
- 包级配置：`xpkGetPackType` / `xpkSetPackType`、`xpkGetDefaultComp` / `xpkSetDefaultComp`、`xpkGetMetaComp` / `xpkSetMetaComp`、`xpkGetInfoComp` / `xpkSetInfoComp`、`xpkGetInfoExtSize` / `xpkSetInfoExtSize`、`xpkGetVolumeSize` / `xpkSetVolumeSize`、`xpkGetSolidMode` / `xpkSetSolidMode`。
- 包元数据：`xpkMetaGet`、`xpkMetaSet`、`xpkMetaClear`。
- 位置访问：`xpkCount`、`xpkGetInfo`、`xpkGetInfoExt`、`xpkSetInfoExt`、`xpkAddFile`、`xpkAddData`、`xpkReadToFile`、`xpkReadToMemory`、`xpkUpdateFile`、`xpkUpdateData`、`xpkRemove`、`xpkSetFlag`。
- Index 访问：`xpkIndexFind`、`xpkIndexAddFile`、`xpkIndexAddData`、`xpkIndexReadToFile`、`xpkIndexReadToMemory`、`xpkIndexUpdateFile`、`xpkIndexUpdateData`、`xpkIndexRemove`。
- Path 访问：`xpkPathExists`、`xpkPathAddFile`、`xpkPathAddData`、`xpkPathReadToFile`、`xpkPathReadToMemory`、`xpkPathUpdateFile`、`xpkPathUpdateData`、`xpkPathRename`、`xpkPathRemove`、`xpkPathSetAttr`。
- 遍历、校验、统计：`xpkEach`、`xpkVerify`、`xpkVerifyAll`、`xpkStatGet`。
- 错误状态：`xpkLastError`、`xpkLastErrorMessage`。
- 内存释放：`xpkFree`。

## 未单独接入或暂不需要 GUI 入口

- `xpkIndexGetInfo`：当前列表和属性读取通过 `xpkEach` 枚举获得 Index 信息，暂不需要单独入口。
- `xpkIndexSetFlag`：当前 File Type 修改通过条目位置调用 `xpkSetFlag`，无需按 fileIndex 再包装一层。
- `xpkPathGetInfo`：当前列表和属性读取通过 `xpkEach` 枚举获得 Path 信息，暂不需要单独入口。
- `xpkEachMatch`：GUI 已实现更适合界面的多关键词和通配符过滤，暂不直接使用底层匹配枚举。
- `xpkHash32`：GUI 使用归档条目已有 hash；如后续增加“计算外部文件 hash”工具，再接入该 API。

## 功能驱动的 API 缺口

- 批量重新编号 `index` 包的 `fileIndex` 需要 metadata-only 的 `xpkIndexSetFileIndex`、`xpkIndexRename` 或事务式批量 reorder API；当前 GUI 只保留单条目 `Set FileIndex...`。
- 编辑 `linux` / `win32` 包时间戳需要 `xpkPathSetTimes` 或通用 path metadata setter；当前 GUI 只读取和显示时间戳。
- 详细评估见 `docs/FEATURE_EVALUATION.md`。

## 后续检查规则

- `xpack.h` 新增公开 API 时，先在本文件补充覆盖判断。
- 如果 API 有直接用户价值，再在 `XPKGUI_SPEC.md` 增加可跟踪任务。
- 如果 API 只是底层等价入口，记录“不单独接入”的理由即可。
