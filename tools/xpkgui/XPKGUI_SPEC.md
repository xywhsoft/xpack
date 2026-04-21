# xpkgui 规格与进度跟踪

本文件用于跟踪 `xpkgui` 的后续实现进度。后续开发以本 spec 为主线：每完成一项功能、验证或文档工作，都同步更新对应勾选状态。

状态说明：

- `[ ]` 未开始
- `[~]` 进行中
- `[x]` 已完成

当前目标：

- Windows-only GUI，对标 7-Zip for Windows。
- 工程保留在 `tools/xpkgui`。
- 主程序输出到 `release/x64/xpkgui.exe`。
- Explorer shell 扩展输出到 `release/x64/xpkgui_shext.dll`。
- 构建、安装、卸载、smoke 脚本保留在 `tools/xpkgui`，避免污染仓库根目录。

## 0. 开发约定

- [x] 确认本项目只支持 Windows，不做跨平台 GUI 适配。
- [x] 确认 GUI 对标 7-Zip for Windows 的核心体验。
- [x] 确认旧代码 `D:\git\xPack\old\tools\xpkgui` 仅作参考，不按旧架构兼容。
- [x] 确认不兼容旧格式包，新包格式和当前 xPack API 优先。
- [x] 确认源码目录为 `tools/xpkgui`。
- [x] 确认构建产物目录为 `release/x64`。
- [x] 确认构建脚本、安装脚本、卸载脚本、smoke 脚本放在 `tools/xpkgui`。
- [x] 确认后续每完成一项主线任务后更新本 spec。
- [~] 保持 README 与本 spec 同步，避免 README 只记录已完成能力、spec 只记录待办而产生分歧。

## 1. 已完成基线

- [x] 实现 `build_x64.bat`，可构建 `xpkgui.exe` 和 `xpkgui_shext.dll`。
- [x] 实现 `install_x64.bat` 和 `uninstall_x64.bat`。
- [x] 实现 `smoke_x64.bat`，覆盖 `core` / `index` / `linux` / `win32` 新包创建、info、list、test、extract。
- [x] GUI 主程序静态接入 xPack 核心和压缩后端，不依赖单独 `xpack.dll`。
- [x] 支持打开、创建、保存、另存为、重构 `.xpk`。
- [x] 支持 `core` / `index` / `linux` / `win32` pack type。
- [x] 支持默认压缩、meta 压缩、info 压缩、`InfoExt Size`、`Volume Size`、写入策略、solid layout。
- [x] 支持添加文件、添加目录、拖放添加。
- [x] 支持删除、重命名、替换、复制、移动、复制条目。
- [x] 支持创建空条目和创建 UTF-8 文本条目。
- [x] 支持校验选中项、校验整包。
- [x] 支持解压选中项、解压整包，覆盖冲突时提示。
- [x] 添加、创建、解压、校验、重构使用进度窗口。
- [x] 支持归档属性和选中项属性。
- [x] 支持直接打开文件、打开方式、在 Explorer 中定位临时解压文件。
- [x] 支持 `View (F3)` 内置文本/十六进制预览。
- [x] 支持 `Edit Text...` 直接编辑小型 UTF-8 文本并写回。
- [x] 支持 `Edit (F4)` 解压到临时目录、用记事本编辑、检测变更后写回。
- [x] `.xpk` 内部文件双击后启动新的 `xpkgui.exe` 打开。
- [x] `linux` / `win32` 包支持虚拟目录浏览。
- [x] `linux` / `win32` 包支持 `Flat View` 和 `Locate In Tree`。
- [x] 顶部导航栏支持 `Back` / `Forward` / `Root` / `Up` / `Go`。
- [x] 顶部过滤框支持实时筛选、多关键词 AND、`*` / `?` 通配符。
- [x] 列表支持排序、排序箭头、排序持久化和 `Reset Sort`。
- [x] 列表支持列宽持久化、`Auto Size Columns`、`Reset Columns`。
- [x] 列宽和排序持久化记录当前列数，列数变化时自动回退默认。
- [x] 列表显示 `Name / Path`、`Size`、`Packed`、`Ratio`、`Method`、`File Type`、`Modified`、`ID`、`Hash`、`Attr`。
- [x] `Ratio`、`Hash`、`File Type` 支持排序、过滤和模式选择匹配。
- [x] 支持复制/导出选中项或可见项名称、路径、详细 TSV、hash TSV。
- [x] 支持按扩展名、hash、压缩方法、file type、platform attr 扩选。
- [x] 支持选择重复文件、选择重复副本、删除重复副本。
- [x] 支持模式选择和模式取消选择。
- [x] 支持包 metadata 查看、编辑、导入、导出、清空。
- [x] 支持 `core` 包单条目 InfoExt 查看、编辑、导入、导出、清空。
- [x] 支持设置 `File Type`。
- [x] 支持 `index` 包设置单条目 `fileIndex`。
- [x] 支持 `linux` / `win32` 包设置 `platformAttr`。
- [x] 支持最近归档列表持久化。
- [x] 支持窗口位置、大小、最大化状态持久化。
- [x] 支持 `New...` 默认归档配置持久化。
- [x] 支持 `Tools -> Show Settings File`。
- [x] 支持 `Tools -> Reset UI Preferences...`。
- [x] 支持 `Help -> Keyboard Shortcuts`。
- [x] 支持 `Help -> Command Line Usage`。
- [x] 支持 `/help`、`/?`、`-help`、`--help` 命令行帮助。
- [x] Explorer 右键菜单接入添加、解压、校验、属性入口。
- [x] 命令行入口支持 `/shell-add`、`/shell-add-auto`、`/extract`、`/extract-auto`、`/extract-here`、`/verify`、`/properties`。

## 2. 当前开发主线

- [x] 建立可跟踪的后续开发 spec，并把后续任务迁移到本文件。
- [~] 按本 spec 逐项推进剩余开发。
- [~] 每轮完成后把对应任务状态从 `[ ]` 或 `[~]` 更新为 `[x]`。
- [ ] 当发现新需求或 bug 时，先加入本 spec，再实现。

## 3. 自动化验证

- [x] 添加 `smoke_x64.bat`。
- [x] smoke 覆盖四种 pack type 的新包创建。
- [x] smoke 覆盖 `info`、`list`、`test`、`extract`。
- [x] smoke 增加 `/verify <archive.xpk>` 命令行入口验证。
- [x] smoke 增加 `/properties <archive.xpk>` 命令行入口验证。
- [x] smoke 增加 `/extract <archive.xpk> <dest>` 命令行入口验证。
- [x] smoke 增加 `/extract-auto <archive.xpk>` 命令行入口验证。
- [x] smoke 增加 `/extract-here <archive.xpk>` 命令行入口验证。
- [x] smoke 增加 `/shell-add-auto <file-or-folder> [...]` 的非交互创建验证。
- [x] smoke 增加安装脚本静态检查，确认 `install_x64.bat` / `uninstall_x64.bat` 引用的文件存在。
- [x] smoke 增加构建产物完整性检查，确认 `xpkgui.exe`、`xpkgui_shext.dll`、所需运行依赖均存在。
- [x] smoke 增加 GUI 命令行入口超时保护，避免弹窗或死锁导致自动化验证无限挂起。
- [x] smoke 增加新列布局检查，防止 `XPKGUI_ARCHIVE_COLUMN_COUNT` 与实际渲染/导出列数不一致。
- [x] smoke 增加命令行未知参数返回码检查。
- [x] smoke 增加旧 `xpkgui.ini` 列宽/排序迁移场景检查。
- [x] smoke 验证 `package_x64.bat` 生成的发布包清单和核心文件校验记录。
- [x] smoke 校验 `PACKAGE_MANIFEST.txt` 中记录的文件大小和 SHA-256。
- [x] 增加 `show_spec_status.ps1`，可从 spec 自动统计已完成、进行中和未完成任务。
- [x] 增加 `write_release_audit_report.ps1`，生成 `release/x64` 发布目录审计报告。

## 4. 手工回归验证

- [x] 编写手工回归测试清单，覆盖 GUI 和 Explorer 场景。
- [ ] 手工验证新建 `core` 包并打开。
- [ ] 手工验证新建 `index` 包并打开。
- [ ] 手工验证新建 `linux` 包并打开。
- [ ] 手工验证新建 `win32` 包并打开。
- [ ] 手工验证右键“添加到 xPack”默认选项可成功创建新包。
- [ ] 手工验证右键解压到指定目录。
- [ ] 手工验证右键解压到同名目录。
- [ ] 手工验证右键解压到当前目录。
- [ ] 手工验证右键校验。
- [ ] 手工验证右键属性。
- [ ] 手工验证双击包内普通文件交给系统默认程序。
- [ ] 手工验证双击包内 `.xpk` 会启动新的 `xpkgui.exe`。
- [ ] 手工验证 `Edit (F4)` 修改后写回归档。
- [ ] 手工验证 `Edit Text...` 修改后写回归档。
- [ ] 手工验证删除、重命名、替换、复制、移动、Duplicate Entry。
- [ ] 手工验证 `Flat View` / `Locate In Tree` / 浏览历史。
- [ ] 手工验证 `Refresh (F5)` 外部变更提示和重新载入。
- [ ] 手工验证列宽、排序、窗口位置、最近归档、New 默认配置持久化。
- [ ] 手工验证 `Reset UI Preferences...` 不会清空最近归档。
- [ ] 手工验证旧 `xpkgui.ini` 遇到 10 列布局时自动回退默认列宽和排序。

## 5. Explorer Shell 扩展

- [x] 实现 Explorer 右键菜单 DLL。
- [x] 实现当前用户安装和卸载脚本。
- [x] 支持 `.xpk` 文件打开、解压、校验、属性。
- [x] 支持普通文件/目录添加到 xPack。
- [ ] 确认 Windows 10 Explorer 下菜单文字、图标、排序和可用状态。
- [ ] 确认 Windows 11 新右键菜单和“显示更多选项”路径。
- [ ] 确认多选文件和多选目录的右键添加行为。
- [ ] 确认多选中包含 `.xpk` 与普通文件时的菜单策略。
- [x] 确认 DLL 被 Explorer 锁定时的重新构建提示或处理文档。
- [x] 增加 shell 扩展注册失败时的诊断输出。
- [x] 增加 shell 扩展卸载后的残留注册表检查说明。

## 6. 7-Zip 对标能力补齐

- [x] 主窗口文件列表、工具栏、菜单、右键菜单具备 7-Zip 类基础形态。
- [x] 支持打开、添加、解压、测试、属性、删除、重命名。
- [x] 支持常用快捷键。
- [x] 支持最近归档。
- [x] 支持列排序和列宽持久化。
- [x] 增加列显示/隐藏配置。
- [x] 增加 `View -> Columns...` 或等价列配置入口。
- [x] 增加 `View -> Show Grid Lines` 等列表显示偏好。
- [x] 增加 `View -> Full Row Select` 等列表显示偏好。
- [x] 增加复制完整行信息时的列顺序与可见列策略。
- [x] 增加更接近 7-Zip 的归档属性摘要，例如压缩率、物理大小、头部/表大小、空洞占比。
- [x] 增加工具栏按钮的 enable/disable 状态细化。
- [x] 增加菜单项分组和名称最终审查，避免用户混淆 `Edit` 与 `Edit Text...`。

## 7. xPack 特有能力补齐

- [x] 支持 pack type。
- [x] 支持 `InfoExt Size`。
- [x] 支持 `File Type`。
- [x] 支持 `platformAttr`。
- [x] 支持 `fileIndex`。
- [x] 支持 `solid` 和 `volume` 相关配置入口。
- [x] 梳理 `xpack.h` 当前公开 API，确认 GUI 是否遗漏新 API。
- [ ] 如果 xPack 后续新增 API，先更新本节再接入 GUI。
- [x] 增强 `volume` 模式的 GUI 提示，明确分卷输出和限制。
- [x] 增强 `solid` 模式的 GUI 提示，明确部分操作限制或成本。
- [x] 增加 pack type 转换的风险提示，避免用户误把归档类型切换导致数据语义变化。
- [x] 增加 `index` 包批量 fileIndex 调整或重新编号能力评估。
- [x] 增加 `linux` / `win32` 包时间戳编辑能力评估。

## 8. 错误处理与稳定性

- [x] 修复新包打开时 `invalid xpk package format` 的已知路径问题。
- [x] 修复右键默认添加时报 `pack type does not match api` 的已知问题。
- [x] 修复双击包内文件报 `invalid xpk package format` 的已知问题。
- [x] 简化双击包内文件逻辑，普通文件交给系统默认程序，`.xpk` 启动新 `xpkgui`。
- [x] 统一错误弹窗文本，区分用户取消、格式错误、IO 错误、API 状态错误。
- [x] 对 `invalid xpk package format` 增加更明确上下文，例如当前打开的是归档还是临时解压文件。
- [x] 对临时目录创建失败、外部程序启动失败提供可复制诊断信息。
- [x] 对外部编辑后写回失败提供保留临时文件路径的提示。
- [x] 对只读文件、只读归档、权限不足提供更明确提示。
- [x] 审查所有 `return FALSE` 路径，补齐必要错误提示。
- [x] 审查所有临时文件/目录生命周期，避免长期堆积或过早删除。

## 9. UI 与可用性

- [x] 增加顶部常用动作栏。
- [x] 增加路径栏和过滤栏。
- [x] 增加状态栏选区摘要。
- [x] 增加快捷键帮助。
- [x] 增加命令行帮助。
- [x] 增加状态栏分栏显示，避免单行文本过长时难以阅读。
- [x] 增加大归档加载时的忙碌提示或后台加载评估。
- [x] 增加列表空状态提示，例如无归档、目录为空、过滤无结果。
- [x] 增加过滤框 placeholder 或旁边说明，提示支持通配符和多关键词。
- [x] 增加进度对话框取消后的结果说明。
- [x] 增加常用按钮 tooltip。
- [x] 增加高 DPI 和不同缩放比例下的布局检查。
- [x] 增加窗口最小尺寸限制。
- [x] 增加深色模式评估，暂不作为第一版必需项。

## 10. 文档

- [x] README 记录当前覆盖范围。
- [x] README 记录构建、smoke、安装、卸载。
- [x] README 记录命令行入口。
- [x] README 记录主要快捷键和 GUI 功能。
- [x] 新增本 spec 用于后续进度跟踪。
- [x] 将 README 拆分为用户使用说明和开发说明，降低单文件长度。
- [x] 增加 Explorer 右键菜单安装、卸载、重启 Explorer 的排错说明。
- [x] 增加常见错误 FAQ。
- [x] 增加手工回归测试说明。
- [x] 增加发布前检查清单。

## 11. 发布前检查

- [x] 清理或忽略 `tools/xpkgui/obj` 下临时构建产物。
- [~] 清理或忽略 `release/x64` 下 smoke 临时输出。
- [x] 确认 `.gitignore` 覆盖 xpkgui 构建中间文件。
- [~] 确认 release 输出目录只保留预期发布文件。
- [x] 增加独立 `package_x64.bat`，生成干净 xpkgui 交付目录。
- [x] 发布包生成 `PACKAGE_MANIFEST.txt`，记录交付文件路径、大小和 SHA-256。
- [x] 发布包清单可通过 `verify_package_manifest.ps1` 自动复核。
- [x] `release/x64` unexpected 文件可生成 Markdown 审计报告，包含 Git 跟踪状态和处理建议。
- [ ] 确认 `xpkgui.exe` 可在干净环境启动。
- [ ] 确认 `xpkgui_shext.dll` 可在干净环境注册。
- [ ] 确认安装后双击 `.xpk` 打开 `xpkgui.exe`。
- [ ] 确认卸载后 `.xpk` 文件关联和 shell 扩展清理符合预期。
- [x] 确认 README 和本 spec 的状态一致。
- [x] 确认 smoke 通过。
- [ ] 确认关键 GUI 场景手工回归通过。
