# xpkgui

`xpkgui` 是 xPack 的 Windows 专用图形前端，定位对标 7-Zip for Windows。

- 主程序：`release/x64/xpkgui.exe`
- Explorer 壳扩展：`release/x64/xpkgui_shext.dll`
- 工程源码：`tools/xpkgui`
- 构建、安装、卸载、smoke 脚本：`tools/xpkgui`
- 后续开发进度跟踪：`tools/xpkgui/XPKGUI_SPEC.md`

## 文档入口

- 用户使用说明：`docs/USER_GUIDE.md`
- 开发与验证说明：`docs/DEV_GUIDE.md`
- 常见问题：`docs/FAQ.md`
- 手工回归测试：`docs/MANUAL_REGRESSION.md`
- 发布前检查清单：`docs/RELEASE_CHECKLIST.md`

## 快速构建

在仓库根目录或任意位置执行：

```bat
tools\xpkgui\build_x64.bat
```

构建产物输出到：

- `release/x64/xpkgui.exe`
- `release/x64/xpkgui_shext.dll`

## 快速验证

```bat
tools\xpkgui\smoke_x64.bat
```

脚本会构建 GUI 和 shell 扩展，生成 `core` / `index` / `linux` / `win32` 四类新包，并验证 `xpkcon` 和 `xpkgui.exe` 的主要命令行入口。

查看 spec 当前进度：

```bat
powershell -NoProfile -ExecutionPolicy Bypass -File tools\xpkgui\show_spec_status.ps1
```

## 快速打包

```bat
tools\xpkgui\package_x64.bat
```

干净交付目录输出到 `tools/xpkgui/obj/package/x64`，只包含 xpkgui 发布载荷。

发布包会包含 `PACKAGE_MANIFEST.txt`，可用以下命令复核大小和 SHA-256：

```bat
powershell -NoProfile -ExecutionPolicy Bypass -File tools\xpkgui\verify_package_manifest.ps1 tools\xpkgui\obj\package\x64
```

如果需要审计共享的 `release/x64` 目录：

```bat
tools\xpkgui\check_release_x64.bat
powershell -NoProfile -ExecutionPolicy Bypass -File tools\xpkgui\write_release_audit_report.ps1
```

## 安装与卸载

注册当前构建结果到当前用户：

```bat
tools\xpkgui\install_x64.bat
```

卸载当前用户的关联和壳扩展：

```bat
tools\xpkgui\uninstall_x64.bat
```

安装脚本使用 `HKCU\Software\Classes`，不需要管理员权限。
