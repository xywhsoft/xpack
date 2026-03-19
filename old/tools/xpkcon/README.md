# xpkcon - xPack 命令行工具

xpkcon 是 xPack 文件压缩库的命令行工具，功能对标 7z，支持创建、解压、管理和查看 xPack 压缩包。

## 特性

- 支持四种包类型：Core、Index、Linux、Win32
- 支持多种压缩算法：无压缩、LZ4、LZ4-HC、ZSTD、LZMA2
- 支持 16 级压缩级别
- 支持固实压缩模式
- 支持通配符匹配
- 支持递归目录处理

## 编译

### x64 版本

#### 方法 1：静态链接（推荐）
生成独立的可执行文件，无需 DLL。

```batch
cd tools\xpkcon
build_static.bat
```

输出：`release\x64\xpkcon.exe`

#### 方法 2：动态链接
生成较小的可执行文件，但运行时需要 `xpack.dll`。

```batch
cd tools\xpkcon
build_dynamic.bat
```

前提条件：`release/x64/xpack.dll` 必须存在。如果不存在，先运行：
```batch
build_TCC_DLL_x64.bat
```

输出：`release\x64\xpkcon.exe`

### x86 版本

```batch
cd tools\xpkcon
build_x86.bat
```

输出：`release\x86\xpkcon.exe`

### 使用 GCC 编译

```batch
cd tools\xpkcon
build_gcc_x64.bat
```

输出：`release\x64\xpkcon.exe`

### 交互式菜单

Windows:
```batch
cd tools\xpkcon
build.bat
```

Linux:
```bash
cd tools/xpkcon
chmod +x build_linux.sh
./build_linux.sh
```

## 测试

xpkcon 包含完整的测试套件，可以验证所有功能是否正常工作。

### 编译测试程序

#### Windows
```batch
cd tools\xpkcon\test
build_test.bat
```

#### Linux
```bash
cd tools/xpkcon/test
chmod +x build_test.sh
./build_test.sh
```

### 运行测试

#### 基础测试
```batch
cd tools\xpkcon\test
run_test.bat
```

#### 完整测试套件（带报告）
```batch
cd tools\xpkcon\test
run_all.bat
```

生成 HTML 报告：
```batch
run_all.bat --html
```

生成 JSON 报告：
```batch
run_all.bat --json
```

同时生成两种报告：
```batch
run_all.bat --html --json
```

#### Linux
```bash
cd tools/xpkcon/test
./run_test.sh          # 基础测试
./run_all.sh            # 完整测试套件
./run_all.sh --html     # 生成 HTML 报告
./run_all.sh --json     # 生成 JSON 报告
```

### 测试覆盖

测试套件包含以下测试用例：

| 类别 | 测试 |
|------|------|
| 基本操作 | 添加单个/多个文件 |
| 提取测试 | 完整路径/无路径解压 |
| 列表和信息 | 列出内容、显示信息 |
| 测试完整性 | 验证压缩包完整性 |
| 删除和更新 | 删除文件、更新文件 |
| 压缩测试 | 所有 16 级压缩级别 |
| 固实模式 | 固实压缩测试 |
| 包类型 | 四种包类型测试 |
| 边界情况 | 空文件、二进制文件、特殊字符 |
| 高级测试 | 递归目录、大文件 |

详细测试说明请参阅 [test/README.md](test/README.md)。

## 命令参考

### 基本语法
```bash
xpkcon <命令> <压缩包> [选项] [文件...]
```

### 命令列表

| 命令 | 描述 |
|------|------|
| `a` | 添加文件到压缩包 |
| `x` | 完整路径解压文件 |
| `e` | 提取文件到当前目录 |
| `l` | 列出压缩包内容 |
| `t` | 测试压缩包完整性 |
| `d` | 从压缩包删除文件 |
| `u` | 更新压缩包中的文件 |
| `i` | 显示压缩包详细信息 |

### 选项列表

| 选项 | 描述 | 默认值 |
|------|------|--------|
| `-t<类型>` | 包类型：core/index/linux/win32 | win32 |
| `-l<级别>` | 压缩级别：0-15 | 7 |
| `-s<0|1>` | 固实模式：0=独立, 1=固实 | 0 |
| `-o<路径>` | 输出目录 | 当前目录 |
| `-y` | 自动确认所有提示 | 否 |
| `-r` | 递归处理子目录 | 否 |
| `-v` | 详细输出 | 否 |

## 压缩级别说明

| 级别 | 算法 | 描述 |
|------|------|------|
| 0 | 无压缩 | 仅打包不压缩 |
| 1-2 | LZ4 | 快速压缩 |
| 3-4 | LZ4-HC | LZ4 高压缩比 |
| 5 | ZSTD Fast | ZSTD 快速模式 |
| 6 | ZSTD DFast | ZSTD 双向快速 |
| 7 | ZSTD Greedy | ZSTD 贪婪模式（默认） |
| 8 | ZSTD Lazy | ZSTD 懒惰模式 |
| 9 | ZSTD Lazy2 | ZSTD 懒惰模式2 |
| 10 | ZSTD BTLazy2 | ZSTD 双树懒惰2 |
| 11 | ZSTD BTOpt | ZSTD 双树优化 |
| 12 | ZSTD BTUltra | ZSTD 双树超高速 |
| 13 | ZSTD BTUltra2 | ZSTD 双树超高速2 |
| 14 | LZMA2 L6 | LZMA2 级别6 |
| 15 | LZMA2 L9 | LZMA2 级别9 |

## 使用示例

### 创建压缩包
```bash
# 创建 Win32 类型压缩包，默认压缩级别
xpkcon a archive.xpk file1.txt file2.txt

# 使用 ZSTD 快速模式
xpkcon a archive.xpk -l5 file1.txt file2.txt

# 启用固实压缩模式
xpkcon a archive.xpk -s1 file1.txt file2.txt

# 递归添加整个目录
xpkcon a archive.xpk -r C:\mydata
```

### 解压文件
```bash
# 解压所有文件到指定目录
xpkcon x archive.xpk -o C:\output

# 解压指定文件
xpkcon x archive.xpk file1.txt file2.txt

# 提取文件到当前目录（不带路径）
xpkcon e archive.xpk file1.txt
```

### 列出内容
```bash
# 列出压缩包所有内容
xpkcon l archive.xpk
```

### 测试压缩包
```bash
# 测试压缩包完整性
xpkcon t archive.xpk
```

### 删除文件
```bash
# 删除文件（会提示确认）
xpkcon d archive.xpk oldfile.txt

# 自动确认，不提示
xpkcon d archive.xpk -y oldfile.txt
```

### 更新文件
```bash
# 更新压缩包中的文件
xpkcon u archive.xpk -l7 file1.txt
```

### 查看信息
```bash
# 显示压缩包详细信息
xpkcon i archive.xpk
```

## 输出格式

### 列表命令输出
```
  Filename                                  Size       Packed      Ratio     Level
  --------                                  ----       ------      -----     -----
  file1.txt                                1.23 KB    456 B       37.4%         7
  file2.txt                                5.67 MB    1.23 MB     21.7%         7

Total: 2 files
```

### 信息命令输出
```
Archive Information:
  Path:       archive.xpk
  Type:       Win32
  Files:      2
  Size:       5.68 MB
  Packed:     1234567 bytes
  Ratio:      21.2%
  Mode:       Solid
  Disc Code:  0x00000000
  Created:    2026-01-31 10:30:45
  Modified:   2026-01-31 10:30:45
```

## 包类型说明

### Core 模式
- 按位置顺序访问
- 最小的包头结构
- 适合批量顺序读取

### Index 模式
- 按整数索引访问
- 支持用户自定义索引
- 适合数据库等场景

### Linux 模式
- 按路径访问
- 路径大小写敏感
- 适合 Linux/Unix 系统

### Win32 模式
- 按路径访问
- 路径不区分大小写
- 适合 Windows 系统

## 固实压缩模式

### 独立模式（默认）
- 每个文件独立压缩
- 解压速度快
- 随机访问方便

### 固实模式
- 所有文件作为一个整体压缩
- 压缩比更高
- 解压时需要解压整个块
- 适合相似类型文件

## 错误代码

| 代码 | 描述 |
|------|------|
| 0 | 成功 |
| 1 | 参数错误或操作失败 |
| -1 | 底层库错误 |

## 注意事项

1. 固实模式只能在空压缩包中设置
2. 不同包类型之间不兼容
3. 压缩级别越高，压缩时间越长
4. LZMA2 压缩比最高但速度最慢
5. 通配符匹配支持 `*` 和 `?`

## 与 7z 的兼容性

xpkcon 命令设计参考了 7z 的命令格式，但不是完全兼容：

| 7z 命令 | xpkcon 命令 |
|---------|-------------|
| 7z a | xpkcon a |
| 7z x | xpkcon x |
| 7z e | xpkcon e |
| 7z l | xpkcon l |
| 7z t | xpkcon t |
| 7z d | xpkcon d |
| 7z u | xpkcon u |

主要差异：
- xpkcon 默认使用 Win32 模式
- 压缩级别映射不同
- 文件格式不兼容

## 许可证

MIT License

## 版本历史

- v1.0.0 - 初始版本
  - 支持基本命令：a, x, e, l, t, d, u, i
  - 支持 16 级压缩级别
  - 支持四种包类型
  - 支持固实压缩模式
  - 完整的测试套件
