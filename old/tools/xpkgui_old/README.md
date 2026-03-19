# xpkgui - xPack 图形管理工具

## 概述

xpkgui 是 xPack 压缩库的图形界面管理工具，提供直观的用户界面来创建、打开、编辑和管理 xPack 压缩包。

## 主要功能

### 基础功能
- **创建压缩包** - 支持选择包类型（Core/Index/Linux/Win32）和压缩级别
- **打开/保存** - 打开已有压缩包或保存修改
- **添加文件** - 添加单个或多个文件到压缩包
- **添加目录** - 递归添加整个目录
- **解压文件** - 解压单个文件或全部文件
- **删除/重命名** - 管理压缩包中的文件
- **重建压缩包** - 优化压缩包存储

### 高级功能
- **压缩级别选择** - 支持 0-15 级压缩级别（STORE/LZ4/ZSTD/LZMA2）
- **包类型选择** - 支持 Core/Index/Linux/Win32 四种包类型
- **固实压缩模式** - 可选择启用或禁用固实压缩
- **分卷压缩模式** - 将压缩包分割成多个文件，适合大文件存储和传输
- **分卷大小设置** - 支持字节/KB/MB/GB 单位设置分卷大小限制
- **模式匹配操作** - 按通配符模式批量操作文件
- **识别代码管理** - 设置和查看压缩包识别代码
- **压缩测试** - 测试压缩包完整性
- **文件信息增强** - 显示文件类型、哈希值等详细信息

### 用户体验
- **拖放支持** - 拖拽文件到窗口进行压缩
- **错误处理增强** - 显示详细的错误信息
- **配置管理** - 保存用户偏好设置
- **最近文件** - 记录最近打开的压缩包历史
- **快捷键支持** - 快速访问常用功能

### Shell 集成
- **文件关联** - 双击 .xpk 文件自动打开
- **右键菜单** - 在资源管理器中右键菜单快速操作
- **命令行支持** - 支持多种命令行参数用于脚本集成

## 编译

### 编译 xpkgui.exe

```batch
build_x64.bat    # 64位版本
build_x86.bat    # 32位版本
```

### 编译 xpkshext.dll

```batch
build_shext_x64.bat    # 64位版本
build_shext_x86.bat    # 32位版本
```

## 安装

运行安装脚本：

```batch
install.bat
```

安装脚本会：
1. 复制文件到 `%ProgramFiles%\xPack\`
2. 注册 .xpk 文件类型关联
3. 注册 Shell 扩展（右键菜单）
4. 刷新图标缓存

## 卸载

运行卸载脚本：

```batch
uninstall.bat
```

## 命令行参数

xpkgui 支持以下命令行参数：

| 参数 | 功能 |
|------|------|
| `<文件名>` | 打开指定的 .xpk 文件 |
| `-extract <文件>` | 提取模式：显示提取对话框 |
| `-extract_here <文件>` | 提取到当前文件夹 |
| `-add <文件>` | 添加模式：显示添加对话框 |
| `-add_auto <文件>` | 添加模式（自动命名） |
| `-verify <文件>` | 验证压缩包并显示结果 |
| `-properties <文件>` | 显示压缩包属性 |

### 示例

```batch
# 打开压缩包
xpkgui.exe archive.xpk

# 提取到指定目录
xpkgui.exe -extract archive.xpk

# 提取到当前目录
xpkgui.exe -extract_here archive.xpk

# 验证压缩包
xpkgui.exe -verify archive.xpk

# 查看属性
xpkgui.exe -properties archive.xpk
```

## 快捷键

| 快捷键 | 功能 |
|--------|------|
| Ctrl+N | 新建压缩包 |
| Ctrl+O | 打开压缩包 |
| Ctrl+S | 保存压缩包 |
| Ctrl+A | 添加文件 |
| Ctrl+D | 添加目录 |
| Ctrl+E | 解压文件 |
| F5 | 刷新 |
| Del | 删除文件 |
| F2 | 重命名文件 |

## 压缩级别说明

| 级别 | 算法 | 描述 | 原生级别 |
|------|------|------|---------|
| 0 | STORE | 无压缩 | 0 |
| 1-2 | LZ4 | 快速压缩 | 1-2 |
| 3-4 | LZ4-HC | 高质量压缩 | 4,12 |
| 5-13 | ZSTD | 标准压缩 | 1-9 |
| 14-15 | LZMA2 | 高压缩率 | 6,9 |

## 包类型说明

| 包类型 | 特点 | 适用场景 |
|--------|------|---------|
| Win32 | 路径不区分大小写 | Windows 平台 |
| Linux | 路径区分大小写 | Linux/Unix |
| Index | 整数索引访问 | 数据库、缓存 |
| Core | 顺序位置访问 | 流式处理 |

## 分卷压缩功能

### 概述

分卷压缩功能允许将大型压缩包分割成多个较小体积的文件，适用于以下场景：
- 大文件存储（如光盘、U盘容量限制）
- 网络传输（邮件附件大小限制）
- 多卷备份（每卷单独存储）

### 使用方法

#### 1. 启用分卷模式

在工具菜单中选择：
- **工具 → 分卷模式** - 切换分卷模式启用/禁用
- **工具 → 设置分卷大小** - 设置单个卷的大小限制

#### 2. 设置分卷大小

分卷大小支持以下单位：
- **字节 (B)** - 精确控制
- **千字节 (KB)** - 小文件分卷
- **兆字节 (MB)** - 常用单位（推荐）
- **吉字节 (GB)** - 大文件分卷

常用分卷大小参考：
- 1.44MB - 标准软盘
- 700MB - CD-R
- 4.7GB - DVD-R
- 25GB - 蓝光盘

#### 3. 分卷文件命名

启用分卷模式后，压缩包会自动生成多个文件：
```
archive.xpk
archive.xpk.001
archive.xpk.002
archive.xpk.003
...
```

第一个文件 `.xpk` 是主文件，包含索引信息。后续文件 `.xpk.001`, `.xpk.002` 等是数据卷。

### 限制和注意事项

1. **只能在空压缩包中切换分卷模式**
   - 已包含文件的压缩包需要先清空才能切换模式

2. **分卷大小设置**
   - 设置为 0 表示不限制（单卷模式）
   - 最小值建议不低于 1MB

3. **解压要求**
   - 所有分卷文件必须在同一目录
   - 缺少任何分卷会导致解压失败

4. **性能影响**
   - 分卷模式会略微增加压缩时间
   - 解压时需要按顺序读取所有分卷

### 与 xpkcon 配合使用

命令行工具 xpkcon 也支持分卷功能：

```batch
# 创建分卷压缩包（每卷 100MB）
xpkcon a archive.xpk -vs100000000 file1 file2

# 查看分卷信息
xpkcon l archive.xpk
```

## 配置文件

配置文件位置：`%APPDATA%\xPack\settings.ini`

```ini
[Settings]
DefaultCompLevel=7
DefaultPkgType=3
SolidMode=0
VolumeMode=0
VolumeSize=0
ConfirmDelete=1
OverwriteFiles=0
ShowStatusBar=1
ShowGridLines=1
[Window]
Width=900
Height=600
Maximized=0
```

**分卷模式说明**：
- `VolumeMode=1` 启用分卷模式
- `VolumeSize=0` 表示不限制分卷大小（单卷模式）
- `VolumeSize=104857600` 表示每卷 100MB
- 支持的单位：字节、KB、MB、GB

## 最近文件

历史文件位置：`%APPDATA%\xPack\history.ini`

```ini
[History]
Count=5
Path0=C:\Data\archive1.xpk
Path1=D:\Backup\backup.xpk
Path2=C:\Downloads\files.xpk
Path3=E:\Projects\data.xpk
Path4=C:\Temp\test.xpk
```

## 与 xpkcon 集成

对于复杂的批量操作，xpkgui 会自动调用 xpkcon 命令行工具：

- 添加目录时调用 `xpkcon a -r -lN`
- 模式匹配提取时调用 `xpkcon x -oDIR pattern`

## Shell 扩展

Shell 扩展 (xpkshext.dll) 提供右键菜单集成：

### .xpk 文件右键菜单
- 打开 xpkgui
- 解压到...
- 解压到当前文件夹
- 验证压缩包
- 属性

### 普通文件/文件夹右键菜单
- 添加到 xPack...
- 添加到 xPack (自命名)

## 文件结构

```
tools/xpkgui/
├── xpkgui.c                 # 主程序源代码
├── xpkgui.rc                # 资源文件
├── resource.h               # 资源ID定义
├── xpkshext.c              # Shell Extension 源代码
├── xpkshext.h              # Shell Extension 头文件
├── xpkshext.def            # DLL 导出定义
├── build_x64.bat           # 64位编译脚本
├── build_x86.bat           # 32位编译脚本
├── build_shext_x64.bat     # Shell Extension 64位编译脚本
├── build_shext_x86.bat     # Shell Extension 32位编译脚本
├── install.bat             # 安装脚本
├── uninstall.bat           # 卸载脚本
├── xpkgui.reg             # 文件关联注册表
└── xpkshext.reg           # Shell 扩展注册表
```

## 系统要求

- Windows 7 或更高版本
- Visual C++ 运行时库

## 许可证

请参考项目根目录的许可证文件。

## 贡献

欢迎提交问题报告和拉取请求。
