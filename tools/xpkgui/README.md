# xpkgui

`xpkgui` 是 xPack 的 Windows 专用图形前端，定位对标 7-Zip for Windows：

- 主程序是 `release/x64/xpkgui.exe`
- Explorer 壳扩展是 `release/x64/xpkgui_shext.dll`
- 工程源码在 `tools/xpkgui`
- 构建脚本、安装脚本都放在 `tools/xpkgui`

## 当前覆盖范围

GUI 目前已经把 xPack 的核心归档能力接进来了：

- 打开、创建、保存、重构 `.xpk`
- 文件/目录拖放添加
- 支持 `core` / `index` / `linux` / `win32` pack type
- 默认压缩、元数据压缩、信息区压缩、`infoExtSize`
- `solid`、`volume`、写入策略
- 删除、重命名、校验、属性查看
- 元数据导入 / 导出 / 清空
- 支持 GUI 内查看和编辑 UTF-8 包元数据；非 UTF-8 元数据会提供十六进制只读预览
- `core` 包在启用 `InfoExt Size` 后，可直接查看和编辑单条目的 UTF-8 `InfoExt`；非文本内容会显示十六进制只读预览，也支持按原始二进制导入 / 导出 / 清空
- 所有 pack type 都支持对当前选区批量设置条目 `File Type`，兼容旧版约定的 `Unknown / Binary / Text / Image / Audio / Video / Archive`
- Explorer 右键菜单
- 已打开归档时支持直接创建空条目；`index` 包会提示 `fileIndex`，`linux/win32` 包会提示包内路径
- 已打开归档时也支持直接创建 UTF-8 文本条目，并立即在 GUI 内编辑后写入归档
- 添加 / 创建后写入 / 解压 / 校验 / 重构 具备独立进度窗口，不再直接卡死主界面
- 进度窗口会显示当前正在处理的文件或目标路径
- `Verify` 支持智能行为：有选区时校验选中条目/目录子树，无选区时校验整包；菜单里也已拆分为 `Verify Selected` 和 `Verify All`
- 解压遇到已存在目标时会先提示，可选择全部覆盖、全部跳过或取消
- 主窗口顶部提供常用动作栏，直接放出 `New / Open / Add / Extract / Verify / Info`；其中 `Add` 在无归档时会直接创建新包，`Extract / Info` 会按当前选区智能选择作用范围
- 主列表支持按列排序，并且提供常用右键菜单
- `linux` / `win32` 包支持按虚拟目录浏览，双击进入目录，`Backspace` 返回上级
- `linux` / `win32` 包支持在目录视图和 `Flat View` 之间切换，且可将选中的平铺结果直接定位回目录树，方便全局搜索和批量操作
- `linux` / `win32` 包支持对当前选区批量设置 `platformAttr`；目录会递归作用到整个子树，输入支持十进制和 `0x` 十六进制
- 向 `index` 包添加内容时可指定起始 `fileIndex`；向 `linux` / `win32` 包添加内容时可指定包内根前缀，便于直接控制逻辑索引和入包路径
- `index` 包支持对单个文件直接 `Set FileIndex...`，可把已有条目迁移到新的 `fileIndex`
- 顶部导航栏支持 `Back / Forward / Root / Up / Go`，目录视图、`Flat View`、`Locate In Tree` 和手动路径跳转共用同一套浏览历史
- `File -> Recent Archives` 会持久化最近打开或创建的归档，下次启动仍可直接打开
- 单个文件支持直接 `Open` 或 `Open With...`，都会先解压到临时目录再交给系统处理；如果包内文件本身是 `.xpk`，则 `Open` 会直接用新的 `xpkgui` 窗口打开；`Enter` / 双击文件也走同样逻辑
- 单个文件支持 `View (F3)`：小文件可直接在 GUI 内按 UTF-8 文本或十六进制只读预览，方便快速查看内容而不跳出当前窗口
- 单个文件支持 `Edit Text...`：小型 UTF-8 文本可直接在 GUI 内编辑并写回归档；非文本内容会退化为十六进制只读预览
- `Edit` 会先解压到临时目录并用记事本打开；关闭记事本后如果检测到内容变化，会提示是否直接写回归档
- 单个文件支持 `Replace...`，可直接选一个外部文件替换当前条目，保留原有包内位置和当前 `File Type`
- 单个文件支持 `Duplicate Entry...`：可直接复制为新条目；`index` 包会提示新的 `fileIndex`，`linux/win32` 包会提示新的包内路径，并尽量保留 `File Type`、`core InfoExt` 和 `path platformAttr`
- 单个文件也支持 `Show In Explorer`，会先解压到临时目录，再让 Explorer 直接定位到该文件
- 选中目录后可以直接解压整个子树；单个目录也支持整棵子树重命名
- 选中项支持 `Properties` 查看条目或当前选区摘要；归档属性和选中项属性已区分
- 单文件属性会显示 xPack 内建的条目 `Hash`；`linux/win32` 包还会显示创建/修改/访问时间
- 主列表现在直接显示 `Hash` 列，并支持按 `Hash` 排序；过滤和按模式选择也会把 `Hash` 纳入匹配字段
- 主列表现在也直接显示 `File Type` 列，并支持按 `File Type` 排序；过滤和按模式选择也会把它纳入匹配字段
- 顶部导航栏支持 `Root / Up / Go`，路径框可直接输入包内目录并按 `Enter` 跳转
- 顶部过滤框支持实时筛选当前视图，支持多关键词 AND 和 `*` / `?` 通配符
- `Refresh / F5` 会从磁盘重新载入当前归档，并尽量保留当前目录、过滤条件、排序、平铺视图和浏览历史
- 切回 `xpkgui` 窗口时，如果当前归档已被外部程序修改，会提示是否从磁盘重新载入
- 状态栏会显示当前选中行对应的底层文件数量、原始大小和压缩后大小
- 支持把当前选中条目的名称或包内路径批量复制到剪贴板；路径型包输出完整包内路径
- 单选文件时也支持把条目的内建 `Hash` 直接复制到剪贴板
- 排序、筛选、刷新以及平铺/目录视图切换时，会尽量保留当前选中项和焦点
- 支持按当前文件扩展名、相同 `Hash + Size`、相同压缩方法、相同 `File Type` 快速扩选；`linux/win32` 包还支持按相同 `platformAttr` 扩选；也支持一键选出当前视图里的重复文件或仅选出重复副本，直接删除重复副本并默认保留每组排在最前的一个文件，以及对当前可见列表执行反选
- 支持按模式批量选择当前可见条目，匹配语义与过滤框一致，支持通配符和多关键词
- 列表支持 `F2` 重命名、`Delete` 删除、`Backspace` 返回上级目录
- 快捷键支持 `F3` 查看、`F4` 编辑、`F5` 从磁盘重新载入当前归档、`Ctrl+F` 聚焦过滤框、`Ctrl+L` 聚焦路径栏、`Ctrl+A` 全选当前视图、`Ctrl+C` 复制选中名称/路径、`Alt+Enter` 查看选中项属性、`Alt+Up` 返回上级目录、`Alt+Left / Alt+Right` 浏览历史
- 通过 Explorer / 命令行触发的解压与校验也复用同一套后台进度对话框

Explorer 右键菜单复用了 `xpkgui.exe` 的命令行入口：

- `/shell-add`
- `/shell-add-auto`
- `/extract`
- `/extract-auto`
- `/extract-here`
- `/verify`
- `/properties`

## 构建

在仓库根目录或任意位置执行都可以：

```bat
tools\xpkgui\build_x64.bat
```

脚本会优先直接使用 PATH 中的 `clang-cl` / `rc`。如果当前 shell 不是 VS 开发环境，它会尝试通过 `vswhere + VsDevCmd.bat` 自动加载 x64 工具链。

构建产物输出到：

- `release/x64/xpkgui.exe`
- `release/x64/xpkgui_shext.dll`

说明：

- 这里默认使用 `clang-cl`，不是 `cl`。原因是当前 `xrt/xpack` 源码里仍有一部分对 MSVC C 前端不友好的实现，`clang-cl` 更稳。
- GUI 主体会直接静态编进 `xpack.c` 和压缩后端，不依赖单独的 `xpack.dll`。

## 安装与卸载

注册当前构建结果到当前用户：

```bat
tools\xpkgui\install_x64.bat
```

卸载当前用户的关联和壳扩展：

```bat
tools\xpkgui\uninstall_x64.bat
```

安装脚本会做两件事：

- 调用 `regsvr32` 注册 `xpkgui_shext.dll`
- 在 `HKCU\Software\Classes` 下注册 `.xpk` 的文件关联和常用 verb

这样不需要管理员权限，也不会污染系统级 `HKLM/HKCR`。

## 壳扩展行为

当前 Explorer 壳扩展是经典 `IContextMenu + IShellExtInit` 实现：

- 右击单个 `.xpk` 时提供打开、解压、解压到同名目录、解压到当前目录、校验、属性
- 右击普通文件/目录或多选时提供“添加到 xPack...”和“添加到 `<name>.xpk`”

在 Windows 11 上，这类经典壳扩展通常会出现在“显示更多选项”中；`.xpk` 的普通 shell verb 则可以直接通过文件关联使用。
