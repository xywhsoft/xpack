# xpkcon 命令行工具

<cite>
**本文档中引用的文件**
- [xpkcon.c](file://tools/xpkcon/xpkcon.c)
- [README.md](file://tools/xpkcon/README.md)
- [BUILD_INSTRUCTIONS.md](file://tools/xpkcon/BUILD_INSTRUCTIONS.md)
- [build.bat](file://tools/xpkcon/build.bat)
- [build_static.bat](file://tools/xpkcon/build_static.bat)
- [build_dynamic.bat](file://tools/xpkcon/build_dynamic.bat)
- [test/README.md](file://tools/xpkcon/test/README.md)
- [test_xpk_api.c](file://tools/xpkcon/test/test_xpk_api.c)
- [test_xpkcon.c](file://tools/xpkcon/test/test_xpkcon.c)
- [design.md](file://docs/design.md)
- [spec.md](file://docs/spec.md)
</cite>

## 目录
1. [简介](#简介)
2. [项目结构](#项目结构)
3. [核心组件](#核心组件)
4. [架构概览](#架构概览)
5. [详细组件分析](#详细组件分析)
6. [依赖关系分析](#依赖关系分析)
7. [性能考虑](#性能考虑)
8. [故障排除指南](#故障排除指南)
9. [结论](#结论)
10. [附录](#附录)

## 简介

xpkcon 是 xPack 文件压缩库的命令行工具，功能对标 7z，支持创建、解压、管理和查看 xPack 压缩包。该工具提供了丰富的压缩算法支持、多种包类型选择以及灵活的配置选项。

### 主要特性

- **多算法支持**：支持无压缩、LZ4、LZ4-HC、ZSTD、LZMA2 等多种压缩算法
- **多包类型**：Core、Index、Linux、Win32 四种包类型
- **十六级压缩**：支持 0-15 级压缩级别，满足不同性能需求
- **固实压缩**：支持固实压缩模式以获得更高的压缩比
- **通配符匹配**：支持文件名通配符过滤
- **递归处理**：支持目录递归处理
- **分卷压缩**：支持按大小或文件数量进行分卷压缩

## 项目结构

xpkcon 项目采用模块化设计，主要包含以下结构：

```mermaid
graph TB
subgraph "工具目录 (tools/xpkcon)"
A[xpkcon.c - 主程序]
B[README.md - 用户文档]
C[BUILD_INSTRUCTIONS.md - 构建说明]
D[build.bat - 交互式构建脚本]
E[build_static.bat - 静态链接构建]
F[build_dynamic.bat - 动态链接构建]
subgraph "测试目录 (test/)"
G[test_xpk_api.c - API测试]
H[test_xpkcon.c - 命令行测试]
I[README.md - 测试文档]
end
end
subgraph "库依赖"
J[xpack.h - xPack库头文件]
K[xrt/xrt.h - xRT库]
L[lz4/lz4.h - LZ4库]
M[zstd/zstd.h - ZSTD库]
N[lzma/Lzma.h - LZMA库]
end
A --> J
A --> K
A --> L
A --> M
A --> N
```

**图表来源**
- [xpkcon.c](file://tools/xpkcon/xpkcon.c#L1-L10)
- [README.md](file://tools/xpkcon/README.md#L1-L50)

**章节来源**
- [xpkcon.c](file://tools/xpkcon/xpkcon.c#L1-L128)
- [README.md](file://tools/xpkcon/README.md#L1-L150)

## 核心组件

### 命令行参数解析器

命令行参数解析器负责处理用户输入的各种选项和参数：

```mermaid
flowchart TD
A[开始解析] --> B{检查参数数量}
B --> |参数不足| C[打印帮助信息]
B --> |参数充足| D[检查帮助选项]
D --> |是帮助| E[打印帮助]
D --> |非帮助| F[解析选项]
F --> G{解析包类型}
G --> H{解析压缩级别}
H --> I{解析固实模式}
I --> J{解析分卷大小}
J --> K{解析输出目录}
K --> L{解析其他选项}
L --> M[提取命令和文件列表]
M --> N[验证必需参数]
N --> O[返回解析结果]
```

**图表来源**
- [xpkcon.c](file://tools/xpkcon/xpkcon.c#L207-L337)

### 命令处理器

工具支持八种主要命令，每种命令都有专门的处理函数：

| 命令 | 功能 | 对应函数 |
|------|------|----------|
| `a` | 添加文件到压缩包 | `cmd_add()` |
| `x` | 完整路径解压文件 | `cmd_extract()` |
| `e` | 提取文件到当前目录 | `cmd_extract_simple()` |
| `l` | 列出压缩包内容 | `cmd_list()` |
| `t` | 测试压缩包完整性 | `cmd_test()` |
| `d` | 从压缩包删除文件 | `cmd_delete()` |
| `u` | 更新压缩包中的文件 | `cmd_update()` |
| `i` | 显示压缩包详细信息 | `cmd_info()` |

**章节来源**
- [xpkcon.c](file://tools/xpkcon/xpkcon.c#L99-L118)
- [xpkcon.c](file://tools/xpkcon/xpkcon.c#L339-L726)

## 架构概览

xpkcon 采用分层架构设计，从上到下分为应用层、业务逻辑层和数据访问层：

```mermaid
graph TB
subgraph "应用层 (Application Layer)"
A[xpkcon 主程序]
B[命令处理器]
C[参数解析器]
end
subgraph "业务逻辑层 (Business Logic Layer)"
D[xPack API封装]
E[文件系统操作]
F[压缩算法选择]
end
subgraph "数据访问层 (Data Access Layer)"
G[xPack库]
H[压缩算法库]
I[文件系统]
end
A --> B
B --> C
B --> D
C --> D
D --> E
D --> F
E --> I
F --> H
F --> G
```

**图表来源**
- [xpkcon.c](file://tools/xpkcon/xpkcon.c#L61-L128)
- [design.md](file://docs/design.md#L307-L448)

## 详细组件分析

### 主程序入口点

主程序负责初始化、参数解析、命令执行和资源清理：

```mermaid
sequenceDiagram
participant U as 用户
participant M as 主程序
participant P as 参数解析器
participant X as xPack库
participant C as 命令处理器
U->>M : 启动xpkcon
M->>M : 检查参数数量
M->>P : 解析命令行参数
P-->>M : 返回解析结果
M->>X : 打开压缩包
X-->>M : 返回xpk对象
M->>C : 执行对应命令
C->>X : 调用xPack API
X-->>C : 返回操作结果
C-->>M : 返回命令执行结果
M->>X : 保存并关闭压缩包
M-->>U : 输出结果
```

**图表来源**
- [xpkcon.c](file://tools/xpkcon/xpkcon.c#L61-L128)

### 压缩级别映射

xpkcon 支持 16 级压缩级别，每级对应不同的压缩算法：

```mermaid
flowchart LR
subgraph "压缩级别映射"
A[级别 0] --> B[无压缩]
C[级别 1-2] --> D[LZ4]
E[级别 3-4] --> F[LZ4-HC]
G[级别 5] --> H[ZSTD Fast]
I[级别 6] --> J[ZSTD DFast]
K[级别 7] --> L[ZSTD Greedy]
M[级别 8] --> N[ZSTD Lazy]
O[级别 9] --> P[ZSTD Lazy2]
Q[级别 10] --> R[ZSTD BTLazy2]
S[级别 11] --> T[ZSTD BTOpt]
U[级别 12] --> V[ZSTD BTUltra]
W[级别 13] --> X[ZSTD BTUltra2]
Y[级别 14] --> Z[LZMA2 L6]
AA[级别 15] --> BB[LZMA2 L9]
end
```

**图表来源**
- [README.md](file://tools/xpkcon/README.md#L184-L202)

### 分卷压缩机制

分卷压缩功能允许将大压缩包分割成多个小文件：

```mermaid
flowchart TD
A[启用分卷模式] --> B{设置分卷大小}
B --> C{设置拆分模式}
C --> D[计算文件数量]
D --> E[创建第一个卷]
E --> F[添加文件到当前卷]
F --> G{卷容量检查}
G --> |未满| F
G --> |已满| H[创建新卷]
H --> F
F --> I[完成所有文件]
I --> J[输出卷信息]
```

**图表来源**
- [xpkcon.c](file://tools/xpkcon/xpkcon.c#L428-L442)

**章节来源**
- [xpkcon.c](file://tools/xpkcon/xpkcon.c#L177-L205)
- [xpkcon.c](file://tools/xpkcon/xpkcon.c#L428-L442)

## 依赖关系分析

### 外部库依赖

xpkcon 依赖于多个外部库来提供完整的功能：

```mermaid
graph TB
subgraph "xpkcon 主程序"
A[xpkcon.c]
end
subgraph "xPack 库"
B[xpack.h]
C[xpack.c]
D[xpack_compress.c]
end
subgraph "压缩算法库"
E[lz4.h]
F[zstd.h]
G[LzmaDec.h]
end
subgraph "系统库"
H[kernel32.lib]
I[user32.lib]
J[advapi32.lib]
end
A --> B
A --> E
A --> F
A --> G
A --> H
A --> I
A --> J
B --> C
B --> D
```

**图表来源**
- [BUILD_INSTRUCTIONS.md](file://tools/xpkcon/BUILD_INSTRUCTIONS.md#L37-L79)
- [design.md](file://docs/design.md#L392-L410)

### 内部模块依赖

内部模块之间的依赖关系相对简单，主要是主程序依赖于各个功能模块：

```mermaid
graph LR
A[xpkcon.c] --> B[命令处理函数]
A --> C[文件系统操作]
A --> D[格式化工具]
A --> E[进度跟踪]
B --> F[xPack API调用]
C --> G[目录遍历]
C --> H[文件检测]
D --> I[大小格式化]
D --> J[时间格式化]
```

**图表来源**
- [xpkcon.c](file://tools/xpkcon/xpkcon.c#L14-L60)

**章节来源**
- [BUILD_INSTRUCTIONS.md](file://tools/xpkcon/BUILD_INSTRUCTIONS.md#L1-L115)
- [design.md](file://docs/design.md#L392-L410)

## 性能考虑

### 压缩算法性能对比

不同压缩级别的性能特征：

| 级别范围 | 算法 | 压缩速度 | 解压速度 | 压缩比 | 内存使用 |
|----------|------|----------|----------|--------|----------|
| 0 | 无压缩 | 最快 | 最快 | 最低 | 最低 |
| 1-4 | LZ4/LZ4-HC | 快速 | 极快 | 中等 | 低 |
| 5-13 | ZSTD | 中等 | 快速 | 高 | 中等 |
| 14-15 | LZMA2 | 慢 | 中等 | 很高 | 高 |

### 内存管理策略

xpkcon 采用渐进式内存管理策略：

1. **文件预读**：对于大文件，采用分块读取策略
2. **动态分配**：根据文件数量动态分配内存
3. **及时释放**：处理完文件后立即释放相关内存
4. **错误恢复**：在内存不足时提供错误处理机制

### 并发处理能力

虽然当前版本主要采用单线程处理，但架构设计支持未来的并发扩展：

- **文件系统操作**：支持异步文件读写
- **压缩算法**：部分算法支持多线程
- **进度报告**：支持实时进度更新

## 故障排除指南

### 常见构建问题

| 问题 | 原因 | 解决方案 |
|------|------|----------|
| 找不到 xpack.dll | DLL 文件缺失 | 运行 `build_TCC_DLL_x64.bat` 生成DLL |
| 找不到 tcc 命令 | TCC 编译器未安装 | 安装 TCC 并添加到 PATH |
| 编译错误 | 路径或参数错误 | 检查源文件路径和编译参数 |
| 链接失败 | 缺少系统库 | 确保 kernel32、user32、advapi32 存在 |

### 运行时错误诊断

```mermaid
flowchart TD
A[程序崩溃] --> B{检查输入参数}
B --> |参数错误| C[打印帮助信息]
B --> |参数正确| D{检查压缩包}
D --> |压缩包损坏| E[提示修复建议]
D --> |压缩包正常| F{检查权限}
F --> |权限不足| G[提升权限]
F --> |权限正常| H[检查磁盘空间]
H --> |空间不足| I[清理空间]
H --> |空间充足| J[查看日志]
```

**图表来源**
- [xpkcon.c](file://tools/xpkcon/xpkcon.c#L86-L90)

### 性能优化建议

1. **选择合适的压缩级别**：根据文件类型选择最佳压缩级别
2. **合理使用固实模式**：对相似文件使用固实模式
3. **分卷压缩**：大文件使用分卷压缩便于传输
4. **批量操作**：尽量使用批量添加和提取操作

**章节来源**
- [BUILD_INSTRUCTIONS.md](file://tools/xpkcon/BUILD_INSTRUCTIONS.md#L102-L115)
- [README.md](file://tools/xpkcon/README.md#L327-L342)

## 结论

xpkcon 是一个功能完整、设计良好的命令行压缩工具，具有以下特点：

### 优势

- **功能丰富**：支持多种压缩算法和包类型
- **易于使用**：简洁的命令行界面，类似 7z 的使用体验
- **性能优秀**：针对不同场景提供最优的压缩策略
- **可扩展性强**：模块化设计便于功能扩展

### 发展方向

1. **增强通配符支持**：完善文件名匹配功能
2. **改进进度显示**：提供更详细的进度信息
3. **增加网络支持**：支持远程文件操作
4. **优化内存使用**：进一步减少内存占用

xpkcon 为 xPack 生态系统提供了重要的命令行工具，满足了开发者和用户的多样化需求。

## 附录

### 命令参考表

| 命令 | 语法 | 描述 |
|------|------|------|
| `a` | `xpkcon a <archive> [files...]` | 添加文件到压缩包 |
| `x` | `xpkcon x <archive> [files...]` | 完整路径解压文件 |
| `e` | `xpkcon e <archive> [files...]` | 提取文件到当前目录 |
| `l` | `xpkcon l <archive>` | 列出压缩包内容 |
| `t` | `xpkcon t <archive>` | 测试压缩包完整性 |
| `d` | `xpkcon d <archive> [files...]` | 从压缩包删除文件 |
| `u` | `xpkcon u <archive> [files...]` | 更新压缩包中的文件 |
| `i` | `xpkcon i <archive>` | 显示压缩包详细信息 |

### 选项参考表

| 选项 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `-t<type>` | 字符串 | `win32` | 包类型：core/index/linux/win32 |
| `-l<level>` | 整数 | `7` | 压缩级别：0-15 |
| `-s<0|1>` | 整数 | `0` | 固实模式：0=独立, 1=固实 |
| `-V<size>` | 字符串 | `0` | 分卷大小（支持K/M/G后缀） |
| `-o<path>` | 字符串 | `.` | 输出目录 |
| `-y` | 标志 | `false` | 自动确认所有提示 |
| `-r` | 标志 | `false` | 递归处理子目录 |
| `-v` | 标志 | `false` | 详细输出 |

### 构建配置

xpkcon 支持多种构建配置：

```mermaid
graph TB
A[构建配置] --> B[静态链接]
A --> C[动态链接]
B --> D[TCC x64 静态]
B --> E[TCC x86 静态]
B --> F[GCC x64 静态]
B --> G[GCC x86 静态]
C --> H[TCC x64 动态]
C --> I[TCC x86 动态]
C --> J[GCC x64 动态]
C --> K[GCC x86 动态]
```

**图表来源**
- [build.bat](file://tools/xpkcon/build.bat#L8-L15)