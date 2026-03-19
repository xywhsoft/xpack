# xPack GUI 工具编译指南

## 概述

本项目提供完整的编译脚本系统，支持多种编译器、架构和链接方式：

- **编译器**: TCC, GCC
- **架构**: x64, x86
- **链接方式**: 动态链接 (DLL), 静态链接 (Standalone)
- **平台**: Windows, Linux

## 编译脚本列表

### xpkgui (GUI 工具)

| 脚本名称 | 编译器 | 架构 | 链接方式 | 说明 |
|---------|--------|------|---------|------|
| `build_tcc_dynamic_x64.bat` | TCC | x64 | 动态 | 依赖 xpack.dll |
| `build_tcc_static_x64.bat` | TCC | x64 | 静态 | 独立可执行文件 |
| `build_tcc_dynamic_x86.bat` | TCC | x86 | 动态 | 依赖 xpack.dll |
| `build_tcc_static_x86.bat` | TCC | x86 | 静态 | 独立可执行文件 |
| `build_gcc_dynamic_x64.bat` | GCC | x64 | 动态 | 依赖 xpack.dll |
| `build_gcc_static_x64.bat` | GCC | x64 | 静态 | 独立可执行文件 |
| `build_gcc_dynamic_x86.bat` | GCC | x86 | 动态 | 依赖 xpack.dll |
| `build_gcc_static_x86.bat` | GCC | x86 | 静态 | 独立可执行文件 |

### xpkcon (命令行工具)

| 脚本名称 | 编译器 | 架构 | 链接方式 | 说明 |
|---------|--------|------|---------|------|
| `build_tcc_dynamic_x64.bat` | TCC | x64 | 动态 | 依赖 xpack.dll |
| `build_tcc_static_x64.bat` | TCC | x64 | 静态 | 独立可执行文件 |
| `build_tcc_dynamic_x86.bat` | TCC | x86 | 动态 | 依赖 xpack.dll |
| `build_tcc_static_x86.bat` | TCC | x86 | 静态 | 独立可执行文件 |
| `build_gcc_dynamic_x64.bat` | GCC | x64 | 动态 | 依赖 xpack.dll |
| `build_gcc_static_x64.bat` | GCC | x64 | 静态 | 独立可执行文件 |
| `build_gcc_dynamic_x86.bat` | GCC | x86 | 动态 | 依赖 xpack.dll |
| `build_gcc_static_x86.bat` | GCC | x86 | 静态 | 独立可执行文件 |

### xpkshext.dll (Shell 扩展)

| 脚本名称 | 编译器 | 架构 | 说明 |
|---------|--------|------|------|
| `build_shext_tcc_x64.bat` | TCC | x64 | Windows Shell 扩展 |
| `build_shext_tcc_x86.bat` | TCC | x86 | Windows Shell 扩展 |
| `build_shext_gcc_x64.bat` | GCC | x64 | Windows Shell 扩展 |
| `build_shext_gcc_x86.bat` | GCC | x86 | Windows Shell 扩展 |

### Linux 编译

| 脚本名称 | 工具 | 链接方式 | 说明 |
|---------|-----|---------|------|
| `xpkcon/build_linux.sh` | xpkcon | 静态 | Linux 独立可执行文件 |
| `xpkcon/build_linux_dynamic.sh` | xpkcon | 动态 | 依赖 libxpack.so |
| `xpkgui/build_linux.sh` | xpkgui | 静态 | Linux GUI (需要 GTK3) |
| `xpkgui/build_linux_dynamic.sh` | xpkgui | 动态 | 依赖 libxpack.so 和 GTK3 |

## 快速开始

### 方式 1: 使用主构建脚本 (推荐)

```batch
# 构建所有组件
build_all.bat

# 构建特定组件
build_all.bat dll      # 仅构建 xpack.dll
build_all.bat xpkgui   # 仅构建 xpkgui
build_all.bat xpkcon   # 仅构建 xpkcon
build_all.bat shext    # 仅构建 Shell 扩展

# 清理构建输出
build_all.bat clean
```

或使用 PowerShell:

```powershell
# 构建所有组件
.\build_all.ps1

# 构建特定组件
.\build_all.ps1 -Target dll
.\build_all.ps1 -Target xpkgui
.\build_all.ps1 -Target xpkcon
.\build_all.ps1 -Target shext

# 清理构建输出
.\build_all.ps1 -Target clean
```

### 方式 2: 手动构建

```batch
# 1. 先构建 xpack.dll (动态链接需要)
cd d:\Git\xPack
build_TCC_DLL_x64.bat

# 2. 构建 xpkgui
cd tools\xpkgui
build_tcc_dynamic_x64.bat

# 3. 构建 xpkcon
cd ..\xpkcon
build_tcc_dynamic_x64.bat

# 4. 构建 Shell 扩展
cd ..\xpkgui
build_shext_tcc_x64.bat
```

### 方式 3: 选择编译器

根据系统安装的编译器选择：

```batch
# 使用 TCC (编译速度快，适合开发)
build_tcc_dynamic_x64.bat

# 使用 GCC (优化更好，适合发布)
build_gcc_dynamic_x64.bat
```

## 输出目录

所有编译输出统一存放在 `release` 目录下：

```
d:\Git\xPack\release\
├── x64\              # 64 位 Windows 输出
│   ├── xpack.dll
│   ├── xpkgui.exe
│   ├── xpkgui_static.exe
│   ├── xpkcon.exe
│   ├── xpkcon_static.exe
│   └── xpkshext.dll
└── x86\              # 32 位 Windows 输出
    ├── xpack.dll
    ├── xpkgui.exe
    ├── xpkgui_static.exe
    ├── xpkcon.exe
    ├── xpkcon_static.exe
    └── xpkshext.dll
```

Linux 输出:

```
d:\Git\xPack\release\linux\
├── xpkgui
├── xpkcon
└── libxpack.so
```

## 安装

构建完成后，运行安装脚本：

```batch
cd tools\xpkgui
install.bat
```

安装程序会：
- 检测系统架构并选择正确的输出目录
- 复制文件到 `%ProgramFiles%\xPack\`
- 注册 .xpk 文件关联
- 注册 Shell 扩展（右键菜单）
- 刷新图标缓存

## 卸载

```batch
cd tools\xpkgui
uninstall.bat
```

## 编译器安装

### TCC (Tiny C Compiler)

下载地址: https://bellard.org/tcc/

安装后，确保 `tcc.exe` 在系统 PATH 中。

### GCC (MinGW-w64)

下载地址: https://www.mingw-w64.org/

安装后，确保 `gcc.exe` 在系统 PATH 中。

## 常见问题

### Q: 编译提示找不到 xpack.dll

A: 动态链接需要先构建 xpack.dll。运行：
```batch
cd d:\Git\xPack
build_TCC_DLL_x64.bat
```

### Q: 资源文件编译失败

A: 确保安装了 windres 工具（MinGW 或 TCC 自带）。

### Q: Shell 扩展注册失败

A: 需要管理员权限运行 install.bat。右键点击 "以管理员身份运行"。

### Q: Linux 编译缺少 GTK3

A: 安装 GTK3 开发库：
```bash
# Ubuntu/Debian
sudo apt-get install libgtk-3-dev

# Fedora/RHEL
sudo dnf install gtk3-devel
```

### Q: TCC 编译链接错误

A: TCC 对某些复杂 C 代码支持有限，建议使用 GCC 进行生产构建。

## 功能特性

### xpkgui

- 16 级压缩级别选择
- 4 种包模式选择
- 命令行参数支持 (7 种模式)
- 拖放文件支持
- 设置和历史持久化
- 7 列文件列表显示
- Shell 集成对话框

### xpkcon

- 完整的命令行接口
- 支持所有 xPack API 操作
- 批量文件处理
- 脚本友好

### Shell 扩展

- .xpk 文件右键菜单
- 文件/文件夹右键菜单（添加到 xPack）
- 自动检测系统架构

## 技术说明

### 动态链接 vs 静态链接

| 特性 | 动态链接 | 静态链接 |
|-----|---------|---------|
| 文件大小 | 小 | 大 |
| 启动速度 | 快 | 稍慢 |
| 依赖 | 需要 xpack.dll | 无依赖 |
| 更新 | 只更新 DLL | 需重新编译 |
| 内存占用 | 低（多进程共享） | 高（各进程独立） |

### 编译器选择

| 编译器 | 优点 | 缺点 | 适用场景 |
|-------|------|------|---------|
| TCC | 编译极快，轻量 | 优化较少 | 快速开发测试 |
| GCC | 优化好，稳定 | 编译较慢 | 生产环境发布 |

## 许可证

请参考项目根目录的 LICENSE 文件。
