# xpkcon 编译指南

## Windows 编译

### 快速开始

1. 进入 `tools\xpkcon` 目录
2. 运行 `build.bat` 选择编译配置

### 编译选项

| 脚本 | 编译器 | 架构 | 链接方式 | 输出 |
|------|--------|------|----------|------|
| build_tcc_static_x64.bat | TCC | x64 | 静态 | release/x64/xpkcon.exe |
| build_tcc_dynamic_x64.bat | TCC | x64 | 动态 | release/o64/xpkcon.exe |
| build_gcc_static_x64.bat | GCC | x64 | 静态 | release/o64/xpkcon.exe |
| build_gcc_dynamic_x64.bat | GCC | x64 | 动态 | release/o64/xpkcon.exe |
| build_tcc_static_x86.bat | TCC | x86 | 静态 | release/o86/xpkcon.exe |
| build_tcc_dynamic_x86.bat | TCC | x86 | 动态 | release/o86/xpkcon.exe |
| build_gcc_static_x86.bat | GCC | x86 | 静态 | release/o86/xpkcon.exe |
| build_gcc_dynamic_x86.bat | GCC | x86 | 动态 | release/o86/xpkcon.exe |

### 快捷脚本

| 脚本 | 等同于 | 说明 |
|------|--------|------|
| build_x64.bat | build_tcc_static_x64.bat | TCC x64 静态 |
| build_x86.bat | build_tcc_static_x86.bat | TCC x86 静态（自动检查 DLL） |
| build_static.bat | build_tcc_static_x64.bat | TCC x64 静态 |
| build_dynamic.bat | build_tcc_dynamic_x64.bat | TCC x64 动态 |
| build_gcc_x64.bat | build_gcc_static_x64.bat | GCC x64 静态 |

### 使用方法

#### 方法 1：交互式菜单（推荐）
```batch
cd tools\xpkcon
build.bat
```

#### 方法 2：直接运行编译脚本
```batch
cd tools\xpkcon

# TCC x64 静态（独立运行）
build_tcc_static_x64.bat

# TCC x64 动态（需要 xpack.dll）
build_tcc_dynamic_x64.bat

# GCC x64 静态（独立运行）
build_gcc_static_x64.bat

# GCC x64 动态（需要 xpack.dll）
build_gcc_dynamic_x64.bat
```

### 动态链接前置条件

使用动态链接前，需要先编译 xpack.dll：

```batch
# x64 DLL
build_TCC_DLL_x64.bat

# x86 DLL
build_TCC_DLL_x86.bat
```

生成的文件位置：
- x64: `release/x64/xpack.dll`
- x86: `release/x86/xpack.dll`

## Linux 编译

### 编译选项

| 脚本 | 编译器 | 链接方式 | 输出 |
|------|--------|----------|------|
| build_linux_static.sh | GCC | 静态 | release/linux/xpkcon |
| build_linux_dynamic.sh | GCC | 动态 | release/linux/xpkcon |

### 使用方法

#### 方法 1：交互式菜单（推荐）
```bash
cd tools/xpkcon
chmod +x build_linux.sh
./build_linux.sh
```

#### 方法 2：直接运行编译脚本
```bash
cd tools/xpkcon
chmod +x build_linux_static.sh build_linux_dynamic.sh

# 静态链接（独立运行）
./build_linux_static.sh

# 动态链接（需要 libxpack.so）
./build_linux_dynamic.sh
```

### 动态链接前置条件

使用动态链接前，需要先编译 libxpack.so：

```bash
./build_TCC_DLL_linux.sh
```

生成的文件位置：`release/linux/libxpack.so`

## 编译器选择

### TCC（Tiny C Compiler）

**优点：**
- 编译速度极快
- 可执行文件较小
- 适合快速迭代

**缺点：**
- 优化不如 GCC
- 某些 C99/C11 特性支持有限

**适用场景：**
- 开发调试
- 快速构建
- 可执行文件大小敏感

### GCC

**优点：**
- 优化效果好
- 完整的 C 标准支持
- 更好的错误诊断

**缺点：**
- 编译速度较慢
- 可执行文件较大

**适用场景：**
- 发布版本
- 性能敏感的应用

## 静态链接 vs 动态链接

### 静态链接

**优点：**
- 独立运行，无需 DLL
- 部署简单
- 启动速度快

**缺点：**
- 可执行文件较大
- 更新库需要重新编译

**适用场景：**
- 单机使用
- 需要独立部署
- 减少依赖

### 动态链接

**优点：**
- 可执行文件较小
- 共享库可以多个程序使用
- 更新库无需重新编译程序

**缺点：**
- 需要对应的 DLL/so 文件
- 部署时需要携带库文件
- 启动需要加载库

**适用场景：**
- 多个程序使用同一库
- 需要减少磁盘占用
- 库频繁更新

## 输出目录

所有编译输出统一放在 `release` 目录下：

```
release/
├── x64/           # Windows 64-bit
│   ├── xpkcon.exe
│   ├── xpack.dll (需要时)
│   └── libxpack.dll.a (需要时)
├── x86/           # Windows 32-bit
│   ├── xpkcon.exe
│   ├── xpack.dll (需要时)
│   └── libxpack.dll.a (需要时)
└── linux/         # Linux
    ├── xpkcon
    ├── libxpack.so (需要时)
    └── libxpack.so.a (需要时)
```

## 常见问题

### Q: 编译时提示找不到 xpack.dll

**A:** 动态链接需要先编译 xpack.dll：
```batch
cd D:\Git\xPack
build_TCC_DLL_x64.bat
```

### Q: 如何选择编译选项？

**A:** 根据使用场景选择：
- 快速测试：TCC 静态
- 生产环境：GCC 静态
- 多程序共享：GCC 动态

### Q: Linux 下如何添加执行权限？

**A:**
```bash
chmod +x tools/xpkcon/*.sh
```

### Q: 编译失败怎么办？

**A:** 检查：
1. 编译器是否正确安装
2. 所有源文件路径是否正确
3. 动态链接时 DLL 是否存在
4. Windows 下是否使用了正确的命令提示符（CMD）

## 推荐编译流程

### 开发阶段
```batch
# 使用 TCC 快速编译
cd tools\xpkcon
build_tcc_static_x64.bat
```

### 测试阶段
```batch
# 使用 GCC 优化编译
cd tools\xpkcon
build_gcc_static_x64.bat
```

### 发布阶段
```batch
# 根据需求选择配置
cd tools\xpkcon
build.bat
# 选择 [1] TCC x64 Static 或 [5] GCC x64 Static
```

## 完整示例

### Windows x64 静态编译
```batch
cd D:\Git\xPack\tools\xpkcon
build_tcc_static_x64.bat
```

### Windows x86 动态编译
```batch
# 先编译 DLL
cd D:\Git\xPack
build_TCC_DLL_x86.bat

# 再编译 xpkcon
cd tools\xpkcon
build_tcc_dynamic_x86.bat
```

### Linux 静态编译
```bash
cd /path/to/xPack/tools/xpkcon
chmod +x build_linux_static.sh
./build_linux_static.sh
```

### Linux 动态编译
```bash
# 先编译共享库
cd /path/to/xPack
./build_TCC_DLL_linux.sh

# 再编译 xpkcon
cd tools/xpkcon
chmod +x build_linux_dynamic.sh
./build_linux_dynamic.sh
```
