# xpkgui 开发与验证说明

## 目录约定

- 源码目录：`tools/xpkgui/src`
- 资源目录：`tools/xpkgui/src/resources`
- shell 扩展源码：`tools/xpkgui/src/shell`
- 构建脚本：`tools/xpkgui/build_x64.bat`
- 打包脚本：`tools/xpkgui/package_x64.bat`
- 安装脚本：`tools/xpkgui/install_x64.bat`
- 卸载脚本：`tools/xpkgui/uninstall_x64.bat`
- release 审计脚本：`tools/xpkgui/check_release_x64.bat`
- smoke 脚本：`tools/xpkgui/smoke_x64.bat`
- GUI 命令超时包装器：`tools/xpkgui/run_with_timeout.ps1`
- 发布包清单生成器：`tools/xpkgui/write_package_manifest.ps1`
- 发布包清单校验器：`tools/xpkgui/verify_package_manifest.ps1`
- spec 进度统计：`tools/xpkgui/show_spec_status.ps1`
- release 审计报告：`tools/xpkgui/write_release_audit_report.ps1`
- 进度跟踪：`tools/xpkgui/XPKGUI_SPEC.md`
- xPack API 覆盖说明：`tools/xpkgui/docs/API_COVERAGE.md`
- 功能评估结论：`tools/xpkgui/docs/FEATURE_EVALUATION.md`
- 错误处理审查：`tools/xpkgui/docs/ERROR_HANDLING_REVIEW.md`
- `return FALSE` 审查：`tools/xpkgui/docs/RETURN_FALSE_AUDIT.md`
- release 目录清单：`tools/xpkgui/docs/RELEASE_MANIFEST.md`
- 手工回归测试：`tools/xpkgui/docs/MANUAL_REGRESSION.md`

## 构建

```bat
tools\xpkgui\build_x64.bat
```

脚本优先使用 PATH 中的 `clang-cl` / `rc`。如果当前 shell 不是 VS 开发环境，会尝试通过 `vswhere + VsDevCmd.bat` 自动加载 x64 工具链。

产物：

- `release/x64/xpkgui.exe`
- `release/x64/xpkgui_shext.dll`

GUI 主体静态编进 `xpack.c` 和压缩后端，不依赖单独 `xpack.dll`。

## 命令超时包装器

`tools\xpkgui\run_with_timeout.ps1` 用于 smoke 中启动 GUI 命令行入口。它会在超时后终止子进程并返回非零退出码，避免自动化验证被意外弹窗或死锁长期阻塞。

## Smoke

```bat
tools\xpkgui\smoke_x64.bat
```

当前 smoke 覆盖：

- 构建 `xpkgui.exe` / `xpkgui_shext.dll`。
- 静态检查安装脚本、构建产物、列布局和配置迁移关键代码。
- 用 `xpkcon.exe` 生成 `core` / `index` / `linux` / `win32` 四类新包。
- 对四类包执行 `info`、`list`、`test`、`extract`。
- 设置 `XPKGUI_SMOKE=1`，以非交互方式验证 `xpkgui.exe` 的 `/verify`、`/properties`、`/extract`、`/extract-auto`、`/extract-here`、`/shell-add-auto` 和未知参数返回码。
- GUI 命令行验证通过 `run_with_timeout.ps1` 执行，单条命令超时按失败处理。

临时输入、测试包和解压结果位于：

- `tools/xpkgui/obj/smoke`

`release/x64` 发布载荷审计可单独运行：

```bat
tools\xpkgui\check_release_x64.bat
tools\xpkgui\check_release_x64.bat /strict
```

默认模式只输出 warning；`/strict` 用于真正发布前，发现非预期载荷会返回失败。

干净的 xpkgui 交付目录可通过打包脚本生成：

```bat
tools\xpkgui\package_x64.bat
```

输出目录：

- `tools/xpkgui/obj/package/x64`

该目录只复制 `xpkgui.exe`、`xpkgui_shext.dll`、安装/卸载脚本和用户可见文档，不复制 `release/x64` 中的历史 probe/smoke 文件。

`package_x64.bat` 会同时生成 `PACKAGE_MANIFEST.txt`，记录交付文件的相对路径、大小和 SHA-256，用于发布前核对与后续问题排查。

清单可单独复核：

```bat
powershell -NoProfile -ExecutionPolicy Bypass -File tools\xpkgui\verify_package_manifest.ps1 tools\xpkgui\obj\package\x64
```

## 非交互 smoke 模式

`XPKGUI_SMOKE=1` 用于自动化验证：

- 命令行即时任务不弹最终成功/失败 MessageBox。
- 成功返回 `0`。
- 失败返回非 `0`。
- `/properties` 在 smoke 模式下只验证读取流程，不显示属性弹窗。

不要把 `XPKGUI_SMOKE=1` 作为普通用户运行模式，它只用于测试脚本。

## 安装与卸载

```bat
tools\xpkgui\install_x64.bat
tools\xpkgui\uninstall_x64.bat
```

安装脚本会：

- 调用 `regsvr32` 注册 `release/x64/xpkgui_shext.dll`。
- 在 `HKCU\Software\Classes` 下注册 `.xpk` 文件关联和常用 verb。
- 如果 `regsvr32` 失败，输出退出码和可见错误对话框重试命令。

卸载脚本会反注册 shell 扩展并清理当前用户关联。卸载后会检查常见 HKCU 残留项，并在仍发现 `xPack.File`、`.xpk` 或 shell 扩展 CLSID 时输出排查提示。

## 变更流程

- 开发前查看 `XPKGUI_SPEC.md`，选择未完成任务。
- 如果发现新任务，先加入 spec，再实现。
- 完成实现后运行 smoke。
- smoke 通过后更新 spec 状态。
- 可运行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\xpkgui\show_spec_status.ps1` 查看 spec 剩余任务。
- README 只保留入口和快速命令，用户细节写入 `docs/USER_GUIDE.md`，开发细节写入本文件。
- `xpack.h` 新增公开 API 时，先更新 `docs/API_COVERAGE.md`，再决定是否加入 spec 和 GUI。
- 受核心 API 限制的功能先写入 `docs/FEATURE_EVALUATION.md`，不要用重写数据的方式伪装 metadata-only 功能。
- 新增失败路径时先按 `docs/ERROR_HANDLING_REVIEW.md` 判断是否需要用户可见诊断；不要把用户取消当作错误。
- 新增 `return FALSE` 路径时按 `docs/RETURN_FALSE_AUDIT.md` 分类，确认是用户取消、能力守卫、内部探测、调用方已报错，还是必须新增诊断。
- 修改 release 输出策略时同步更新 `docs/RELEASE_MANIFEST.md`，不要在普通开发任务中删除共享 release 目录里的历史文件。

## Explorer 锁定 DLL

Explorer 可能锁定 `xpkgui_shext.dll`，导致重新链接失败。处理方式：

```powershell
Stop-Process -Name explorer -Force -ErrorAction SilentlyContinue
Start-Sleep -Seconds 2
cmd /c D:\git\xPack\tools\xpkgui\smoke_x64.bat
if (-not (Get-Process -Name explorer -ErrorAction SilentlyContinue)) { Start-Process explorer.exe }
```

只有在实际遇到 DLL 锁定时才需要这样处理。
