# xpkgui FAQ

## 打开包提示 `invalid xpk package format`

优先确认正在打开的是当前版本 xPack 创建的新包。旧包不作为兼容目标。

如果是刚创建的新包仍然报错：

- 先运行 `tools\xpkgui\smoke_x64.bat` 确认基础读写链路正常。
- 确认不是把临时解压出的普通文件当成归档打开。
- 记录归档路径、操作步骤和错误弹窗文本。

## 右键“添加到 xPack”失败

先确认已运行：

```bat
tools\xpkgui\install_x64.bat
```

如果 Explorer 未刷新：

- 关闭并重新打开 Explorer 窗口。
- 必要时重启 Explorer。
- 重新运行 `install_x64.bat`。

## Windows 11 看不到右键菜单

当前 shell 扩展是经典 `IContextMenu` 实现，Windows 11 通常会把它放在“显示更多选项”里。

`.xpk` 的普通打开、解压等 verb 仍通过文件关联注册到当前用户。

## 重新构建时 `xpkgui_shext.dll` 被占用

Explorer 会加载 shell 扩展 DLL，导致链接覆盖失败。处理方式见 `docs/DEV_GUIDE.md` 的“Explorer 锁定 DLL”。

## 安装不需要管理员权限吗

不需要。安装脚本写入 `HKCU\Software\Classes`，只影响当前用户。

## 卸载后菜单仍然存在

可能是 Explorer 缓存。尝试：

- 重新打开 Explorer 窗口。
- 重启 Explorer。
- 重新运行 `tools\xpkgui\uninstall_x64.bat`。

## 双击包内文件如何处理

- 普通文件：先解压到临时目录，再交给系统默认程序。
- `.xpk` 文件：先解压到临时目录，再启动新的 `xpkgui.exe` 打开。
- `Open With...`：弹出系统打开方式。
- `Edit Externally (F4)`：用记事本编辑，关闭后如检测到变更会提示写回。

## 如何查看或重置 UI 配置

- `Tools -> Show Settings File`：定位当前用户的 `xpkgui.ini`。
- `Tools -> Reset UI Preferences...`：清除窗口布局、列宽、排序、列表显示偏好和 `New...` 默认归档配置。
- `View -> Columns...`：恢复默认 UI 偏好后会回到全部列可见。
- 最近归档列表不会被 `Reset UI Preferences...` 清空。

## Smoke 会产生哪些临时文件

主要位于：

- `tools/xpkgui/obj/smoke`

这个目录由 `tools/xpkgui/.gitignore` 忽略。

历史上 `release/x64` 下存在一些 probe/smoke 文件，其中部分已经被 Git 跟踪，不能在普通开发过程中直接删除。发布前应按 `docs/RELEASE_CHECKLIST.md` 单独处理。
