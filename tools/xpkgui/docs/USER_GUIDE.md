# xpkgui 用户使用说明

`xpkgui` 是 xPack 的 Windows 专用图形工具，目标是提供接近 7-Zip for Windows 的归档操作体验，同时覆盖 xPack 自身的 pack type、InfoExt、File Type、platformAttr、fileIndex 等能力。

## 基本能力

- 打开、创建、保存、另存为、重构 `.xpk`。
- 支持 `core` / `index` / `linux` / `win32` pack type。
- 支持默认压缩、meta 压缩、info 压缩、`InfoExt Size`、`Volume Size`、写入策略、solid layout。
- 支持添加文件、添加目录、拖放添加。
- 支持删除、重命名、替换、复制、移动、复制条目。
- 支持创建空条目和创建 UTF-8 文本条目。
- 支持校验选中项、校验整包。
- 支持解压选中项、解压整包，覆盖冲突时会先提示。

## 主窗口

顶部动作栏提供常用入口：

- `New`：创建新归档。
- `Open`：打开现有 `.xpk`。
- `Add`：有归档时添加文件/目录，无归档时创建新包。
- `Extract`：有选区时解压选中项，无选区时解压整包。
- `Verify`：有选区时校验选中项，无选区时校验整包。
- `Info`：显示选中项属性或归档属性。

工具栏按钮会按当前上下文启用或禁用：空归档不会启用 `Extract` / `Verify`，无选区时 `Extract` / `Verify` 会作用于整包，`Info` 可回退显示归档属性。

列表列包括：

- `Name / Path`
- `Size`
- `Packed`
- `Ratio`
- `Method`
- `File Type`
- `Modified`
- `ID`
- `Hash`
- `Attr`

列表支持按列排序、排序箭头、列宽持久化、`Reset Sort`、`Auto Size Columns` 和 `Reset Columns`。列宽和排序配置记录当前列数，列数变化时旧配置会自动回退默认。

`View -> Show Grid Lines` 和 `View -> Full Row Select` 可切换列表网格线与整行选中行为，这两个显示偏好会随用户设置持久化。

`View -> Columns...` 可选择显示/隐藏列表列。`Name / Path` 始终显示；复制或导出详细行信息时会按当前可见列和显示顺序输出。

状态栏分栏显示归档摘要、选区摘要、大小/压缩率、数据区统计和当前视图。列表在无归档、目录为空或过滤无结果时会显示空状态提示。

`Archive Properties` 会显示 pack type、压缩配置、solid/volume 设置、条目数、总大小、压缩后大小、压缩率、物理文件大小、live data、holes、metadata、entry table 和 header/table 摘要。

`Archive Settings` 修改 Pack Type、Solid Layout 或 Volume Size 时会提示风险。Pack Type 会改变条目语义；Solid / Volume 物理布局通常需要执行 `Rebuild Archive` 后才正式反映到文件。Volume Size 为 `0` 表示关闭分卷，非 `0` 值不能小于 65536 字节。

可取消的进度任务取消后会显示结果说明。已完成的文件操作不会自动回滚，未处理的条目会停止继续执行。

## 路径型包

`linux` / `win32` 包支持虚拟目录浏览：

- 双击目录进入。
- `Backspace` 返回上级。
- 顶部导航栏支持 `Back` / `Forward` / `Root` / `Up` / `Go`。
- 支持 `Flat View` 平铺查看。
- 支持 `Locate In Tree` 从平铺结果定位回目录树。

## 过滤与选择

顶部过滤框支持实时筛选当前视图：

- 多关键词 AND。
- `*` / `?` 通配符。
- 匹配名称、路径、方法、File Type、Ratio、ID、Hash、Attr 等字段。

批量选择支持：

- Select All / Deselect All / Invert Selection。
- 按模式选择或取消选择。
- 按扩展名、Hash、压缩方法、File Type、platformAttr 扩选。
- 选择重复文件、选择重复副本、删除重复副本。

## 打开与编辑

- `Open` 或双击普通文件：先解压到临时目录，再交给系统默认程序。
- `Open With...`：先解压到临时目录，再弹出系统“打开方式”。
- 包内 `.xpk` 文件：用新的 `xpkgui.exe` 窗口打开。
- `View (F3)`：小文件可在 GUI 内按 UTF-8 文本或十六进制只读预览。
- `Edit Text...`：小型 UTF-8 文本可直接在 GUI 内编辑并写回归档。
- `Edit Externally (F4)`：解压到临时目录并用记事本编辑，关闭后检测到内容变化会提示写回。
- `Show In Explorer`：先解压到临时目录，再让 Explorer 定位文件。

## xPack 特有功能

- 包 metadata 查看、编辑、导入、导出、清空。
- `core` 包单条目 InfoExt 查看、编辑、导入、导出、清空。
- 设置条目 `File Type`。
- `index` 包设置单条目 `fileIndex`。
- `linux` / `win32` 包设置 `platformAttr`。
- `linux` / `win32` 包添加内容时可指定包内根前缀。
- `index` 包添加内容时可指定起始 `fileIndex`。

## Explorer 右键菜单

安装后：

- 右击单个 `.xpk`：打开、解压、解压到同名目录、解压到当前目录、校验、属性。
- 右击普通文件/目录或多选：添加到 xPack、自动添加到 `<name>.xpk`。

在 Windows 11 上，经典 `IContextMenu` 壳扩展通常出现在“显示更多选项”中。

## 快捷键

- `Ctrl+N`：新建。
- `Ctrl+O`：打开。
- `Ctrl+S`：保存。
- `Ctrl+Shift+S`：另存为。
- `Ctrl+W`：关闭归档。
- `F3`：查看。
- `F4`：外部编辑。
- `F5`：从磁盘重新载入。
- `Ctrl+F`：聚焦过滤框。
- `Esc`：清空过滤框。
- `Ctrl+L`：聚焦路径栏。
- `Ctrl+A`：全选当前视图。
- `Ctrl+Shift+A`：取消当前视图选择。
- `Ctrl+C`：复制选中名称/路径。
- `Alt+Enter`：属性。
- `Alt+Up`：返回上级目录。
- `Alt+Home`：返回根目录。
- `Alt+Left` / `Alt+Right`：浏览历史。
- `Ctrl+Num+`：自动调整列宽。

GUI 内也可通过 `Help -> Keyboard Shortcuts` 查看快捷键。

## 命令行

`/?`、`/help`、`-help` 或 `--help` 会显示用法说明。

- `xpkgui.exe <archive.xpk>`：直接打开指定归档。
- `/shell-add <file-or-folder> [...]`：用选中项打开新建归档对话框。
- `/shell-add-auto <file-or-folder> [...]`：在选中项旁自动创建归档。
- `/extract <archive.xpk> [dest-folder]`：解压到选择或指定目录。
- `/extract-auto <archive.xpk>`：解压到归档旁的唯一同名目录。
- `/extract-here <archive.xpk>`：解压到归档所在目录。
- `/verify <archive.xpk>`：校验归档。
- `/properties <archive.xpk>`：显示归档属性。
