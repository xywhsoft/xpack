# xpkgui 发布前检查清单

发布前按本清单逐项检查。完成后同步更新 `tools/xpkgui/XPKGUI_SPEC.md`。

## 构建与验证

- [ ] 运行 `tools\xpkgui\smoke_x64.bat` 并确认通过。
- [ ] 确认 `release/x64/xpkgui.exe` 存在。
- [ ] 确认 `release/x64/xpkgui_shext.dll` 存在。
- [ ] 确认 `release/x64/xpkcon.exe` 可用于验证新包。
- [ ] 确认 `xpkgui.exe /?` 可显示命令行帮助。

## 安装与卸载

- [ ] 运行 `tools\xpkgui\install_x64.bat`。
- [ ] 确认双击 `.xpk` 能打开 `xpkgui.exe`。
- [ ] 确认右键 `.xpk` 能看到打开、解压、校验、属性入口。
- [ ] 确认右键普通文件/目录能看到添加到 xPack 入口。
- [ ] 运行 `tools\xpkgui\uninstall_x64.bat`。
- [ ] 确认卸载后当前用户文件关联和 shell 扩展清理符合预期。

## 文件清理

- [ ] 确认 `tools/xpkgui/obj` 不进入发布包。
- [ ] 确认 `tools/xpkgui/obj/smoke` 不进入发布包。
- [ ] 运行 `tools\xpkgui\package_x64.bat` 并确认 `tools/xpkgui/obj/package/x64` 只包含 xpkgui 发布载荷。
- [ ] 确认 `tools/xpkgui/obj/package/x64/PACKAGE_MANIFEST.txt` 存在，并包含 `xpkgui.exe` 与 `xpkgui_shext.dll` 的 SHA-256。
- [ ] 运行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\xpkgui\verify_package_manifest.ps1 tools\xpkgui\obj\package\x64` 并确认通过。
- [ ] 运行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\xpkgui\write_release_audit_report.ps1`，按报告确认 unexpected 文件是否可删。
- [ ] 确认 `release/x64` 中只保留预期发布文件。
- [ ] 单独评估并清理历史 probe/smoke 文件，例如 `smoke_inputs`、`extract_all`、`shell_add_probe.txt`、`xpack_test.exe`。
- [ ] 如果历史 probe/smoke 文件已被 Git 跟踪，先确认是否仍需要保留，再通过正常变更删除。

## 文档

- [ ] 确认 `README.md` 入口准确。
- [ ] 确认 `docs/USER_GUIDE.md` 覆盖用户主要操作。
- [ ] 确认 `docs/DEV_GUIDE.md` 覆盖构建、验证、安装和 DLL 锁定处理。
- [ ] 确认 `docs/FAQ.md` 覆盖常见错误。
- [ ] 确认 `XPKGUI_SPEC.md` 状态与实际开发进度一致。

## 手工回归

手工回归按 `tools/xpkgui/docs/MANUAL_REGRESSION.md` 执行。本清单只保留发布阶段入口，不在普通开发轮次强制执行。

- [ ] 完成 `docs/MANUAL_REGRESSION.md` 的 GUI 基线包场景。
- [ ] 完成 `docs/MANUAL_REGRESSION.md` 的 Explorer 集成场景。
- [ ] 完成 `docs/MANUAL_REGRESSION.md` 的发布检查场景。
