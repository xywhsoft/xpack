# xpkgui 重构方案总结

## 重构目标

将原有的单文件 2600+ 行代码重构为模块化架构，解决以下问题：
1. 消息循环阻塞导致界面无法操作
2. 全局变量过多导致状态管理混乱
3. 代码耦合严重难以维护
4. 缺乏清晰的模块划分

## 架构设计

### 目录结构

```
tools/xpkgui/
├── src/
│   ├── main.c                    # 程序入口
│   ├── core/
│   │   ├── app.c/h               # 应用程序核心
│   │   └── window.c/h            # 主窗口管理
│   ├── ui/
│   │   ├── dialogs/              # 对话框模块
│   │   │   └── dialog_base.c/h  # 对话框基类
│   │   ├── widgets/              # UI控件
│   │   │   ├── file_list.c/h     # 文件列表控件
│   │   │   └── status_bar.c/h    # 状态栏控件
│   │   └── menus/               # 菜单处理
│   │       └── menu_handler.c/h  # 菜单命令处理
│   ├── operations/               # 业务逻辑
│   │   ├── package_ops.c/h       # 包操作
│   │   ├── file_ops.c/h          # 文件操作
│   │   ├── extract_ops.c/h       # 解压操作
│   │   └── verify_ops.c/h        # 验证操作
│   ├── config/                  # 配置管理
│   │   ├── settings.c/h          # 设置管理
│   │   └── history.c/h           # 历史记录
│   ├── utils/                   # 工具函数
│   │   ├── string_utils.c/h      # 字符串工具
│   │   ├── format_utils.c/h      # 格式化工具
│   │   └── error_handler.c/h     # 错误处理
│   └── resources/               # 资源文件
│       ├── xpkgui.rc             # 资源脚本
│       └── resource.h            # 资源头文件
├── include/xpkgui/
│   ├── common.h                 # 公共头文件
│   ├── types.h                  # 类型定义
│   ├── constants.h              # 常量定义
│   ├── xpack.h                 # xpack 接口头文件
│   └── headers.h               # 所有函数声明
├── build.bat                    # 编译脚本
└── README.md                    # 架构文档
```

## 核心设计原则

### 1. 单一职责原则

每个模块只负责一个功能领域：
- **core/**: 应用程序核心和窗口管理
- **ui/**: 用户界面组件
- **operations/**: 业务逻辑操作
- **config/**: 配置和历史记录
- **utils/**: 通用工具函数

### 2. 依赖注入

通过 `AppContext` 结构体传递应用状态，避免全局变量：

```c
typedef struct {
    HINSTANCE hInstance;
    HWND hMainWnd;
    HWND hFileList;
    HWND hStatusBar;
    
    xpkObject xpk;
    wchar_t xpkPath[MAX_PATH_W];
    wchar_t currentDir[MAX_PATH_W];
    
    CommandMode commandMode;
    wchar_t commandPath[MAX_PATH_W];
    
    Settings settings;
    HistoryItem history[MAX_RECENT_FILES];
    int historyCount;
    
    HMODULE hShell32;
    int initialized;
} AppContext;
```

### 3. 事件驱动

使用 Windows 标准消息处理机制，避免自定义消息循环阻塞：

```c
// 主消息循环
while (GetMessage(&msg, NULL, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
}

// 对话框使用标准 API
DialogBoxParamW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(2001), 
               hwnd, CompressLevelDlgProc, (LPARAM)level);
```

### 4. 模块化对话框

使用 `DialogBoxParam` 创建模态对话框，不阻塞主消息循环：

```c
INT_PTR CALLBACK CompressLevelDlgProc(HWND hDlg, UINT message, 
                                      WPARAM wParam, LPARAM lParam)
{
    static int* pLevel = NULL;
    
    switch (message) {
        case WM_INITDIALOG:
            pLevel = (int*)lParam;
            return TRUE;
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                EndDialog(hDlg, IDOK);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

int Dialog_CompressLevel(HWND hwnd, int* level)
{
    return DialogBoxParamW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(2001), 
                       hwnd, CompressLevelDlgProc, (LPARAM)level);
}
```

## 关键技术改进

### 1. 消息循环优化

**问题**：原有代码中每个对话框都有自己的 `GetMessage` 循环，阻塞主窗口。

**解决方案**：使用 Windows 标准对话框 API (`DialogBoxParam`)，由系统管理对话框消息循环。

### 2. 消除全局变量

**问题**：原有代码使用大量全局变量，导致状态管理混乱。

**解决方案**：创建 `AppContext` 结构体，通过参数传递上下文。

### 3. 模块化设计

**问题**：原有代码所有功能耦合在一个文件中。

**解决方案**：按功能领域拆分为多个模块，每个模块提供清晰的公共接口。

## 模块说明

### Core 模块

- **app.c**: 应用程序初始化、运行、清理
- **window.c**: 主窗口创建、消息处理

### UI 模块

- **dialogs/dialog_base.c**: 对话框基类和通用对话框
- **widgets/file_list.c**: 文件列表控件
- **widgets/status_bar.c**: 状态栏控件
- **menus/menu_handler.c**: 菜单命令处理

### Operations 模块

- **package_ops.c**: 包操作（打开、关闭、保存、创建等）
- **file_ops.c**: 文件操作（删除、重命名）
- **extract_ops.c**: 解压操作
- **verify_ops.c**: 验证操作

### Config 模块

- **settings.c**: 设置管理
- **history.c**: 历史记录管理

### Utils 模块

- **string_utils.c**: 字符串转换工具
- **format_utils.c**: 格式化工具
- **error_handler.c**: 错误处理

## 编译说明

### 编译脚本

```batch
@echo off
setlocal

set SRC_DIR=src
set OBJ_DIR=obj
set BIN_DIR=bin
set INC_DIR=include
set XRT_DIR=../../lib/xrt
set XPACK_DIR=../../src

set CFLAGS=-I%INC_DIR% -I%XRT_DIR% -I%XPACK_DIR% -DXRT_HEADER_PATH="\"%XRT_DIR%/xrt.h\"" -Wall -O2
set LDFLAGS=-mwindows -lshell32 -lcomctl32 -lshlwapi

if not exist %OBJ_DIR% mkdir %OBJ_DIR%
if not exist %BIN_DIR% mkdir %BIN_DIR%

echo Compiling xrt...
gcc %CFLAGS% -c %XRT_DIR%/xrt.c -o %OBJ_DIR%/xrt.o

echo Compiling xpack...
gcc %CFLAGS% -c %XPACK_DIR%/xpack.c -o %OBJ_DIR%/xpack.o
gcc %CFLAGS% -c %XPACK_DIR%/xpack_core.c -o %OBJ_DIR%/xpack_core.o
gcc %CFLAGS% -c %XPACK_DIR%/xpack_index.c -o %OBJ_DIR%/xpack_index.o
gcc %CFLAGS% -c %XPACK_DIR%/xpack_path.c -o %OBJ_DIR%/xpack_path.o
gcc %CFLAGS% -c %XPACK_DIR%/xpack_util.c -o %OBJ_DIR%/xpack_util.o
gcc %CFLAGS% -c %XPACK_DIR%/xpack_compress.c -o %OBJ_DIR%/xpack_compress.o
gcc %CFLAGS% -c %XPACK_DIR%/xpack_ldb.c -o %OBJ_DIR%/xpack_ldb.o
gcc %CFLAGS% -c %XPACK_DIR%/xpack_volume.c -o %OBJ_DIR%/xpack_volume.o

echo Compiling xpkgui...
windres -i src/resources/xpkgui.rc -o obj/xpkgui_res.o -O coff
gcc %CFLAGS% -c %SRC_DIR%/main.c -o %OBJ_DIR%/main.o
gcc %CFLAGS% -c %SRC_DIR%/core/app.c -o %OBJ_DIR%/app.o
gcc %CFLAGS% -c %SRC_DIR%/core/window.c -o %OBJ_DIR%/window.o
gcc %CFLAGS% -c %SRC_DIR%/ui/dialogs/dialog_base.c -o %OBJ_DIR%/dialog_base.o
gcc %CFLAGS% -c %SRC_DIR%/ui/widgets/file_list.c -o %OBJ_DIR%/file_list.o
gcc %CFLAGS% -c %SRC_DIR%/ui/widgets/status_bar.c -o %OBJ_DIR%/status_bar.o
gcc %CFLAGS% -c %SRC_DIR%/ui/menus/menu_handler.c -o %OBJ_DIR%/menu_handler.o
gcc %CFLAGS% -c %SRC_DIR%/operations/package_ops.c -o %OBJ_DIR%/package_ops.o
gcc %CFLAGS% -c %SRC_DIR%/operations/file_ops.c -o %OBJ_DIR%/file_ops.o
gcc %CFLAGS% -c %SRC_DIR%/operations/extract_ops.c -o %OBJ_DIR%/extract_ops.o
gcc %CFLAGS% -c %SRC_DIR%/operations/verify_ops.c -o %OBJ_DIR%/verify_ops.o
gcc %CFLAGS% -c %SRC_DIR%/config/settings.c -o %OBJ_DIR%/settings.o
gcc %CFLAGS% -c %SRC_DIR%/config/history.c -o %OBJ_DIR%/history.o
gcc %CFLAGS% -c %SRC_DIR%/utils/string_utils.c -o %OBJ_DIR%/string_utils.o
gcc %CFLAGS% -c %SRC_DIR%/utils/format_utils.c -o %OBJ_DIR%/format_utils.o
gcc %CFLAGS% -c %SRC_DIR%/utils/error_handler.c -o %OBJ_DIR%/error_handler.o

echo Linking...
gcc %OBJ_DIR%/xrt.o %OBJ_DIR%/xpack.o %OBJ_DIR%/xpack_core.o %OBJ_DIR%/xpack_index.o %OBJ_DIR%/xpack_path.o %OBJ_DIR%/xpack_util.o %OBJ_DIR%/xpack_compress.o %OBJ_DIR%/xpack_ldb.o %OBJ_DIR%/xpack_volume.o %OBJ_DIR%/xpkgui_res.o %OBJ_DIR%/main.o %OBJ_DIR%/app.o %OBJ_DIR%/window.o %OBJ_DIR%/dialog_base.o %OBJ_DIR%/file_list.o %OBJ_DIR%/status_bar.o %OBJ_DIR%/menu_handler.o %OBJ_DIR%/package_ops.o %OBJ_DIR%/file_ops.o %OBJ_DIR%/extract_ops.o %OBJ_DIR%/verify_ops.o %OBJ_DIR%/settings.o %OBJ_DIR%/history.o %OBJ_DIR%/string_utils.o %OBJ_DIR%/format_utils.o %OBJ_DIR%/error_handler.o %LDFLAGS% -o %BIN_DIR%/xpkgui.exe

if exist %BIN_DIR%/xpkgui.exe (
    echo Build successful: %BIN_DIR%/xpkgui.exe
) else (
    echo Build failed!
)
endlocal
```

### 编译命令

```bash
cd tools/xpkgui
build.bat
```

## 使用说明

### 运行程序

```bash
# 正常启动
xpkgui.exe

# 打开指定文件
xpkgui.exe archive.xpk

# 解压模式
xpkgui.exe -extract archive.xpk

# 添加文件模式
xpkgui.exe -add file.txt
```

### 功能特性

1. **文件管理**：打开、关闭、保存压缩包
2. **文件操作**：添加、删除、重命名文件
3. **解压功能**：解压单个文件或全部文件
4. **验证功能**：验证压缩包完整性
5. **配置管理**：保存用户设置和历史记录
6. **拖放支持**：支持拖放文件到窗口

## 扩展开发

### 添加新对话框

1. 在 `src/ui/dialogs/` 创建新文件
2. 实现对话框过程函数
3. 在 `dialog_base.h` 中声明接口
4. 在需要的地方调用对话框

### 添加新操作

1. 在 `src/operations/` 创建新文件
2. 实现操作函数
3. 在 `operations.h` 中声明接口
4. 在菜单处理器中添加命令处理

## 注意事项

1. **线程安全**：当前实现为单线程，避免在回调中执行耗时操作
2. **内存管理**：注意释放动态分配的内存
3. **错误处理**：使用统一的错误处理机制
4. **编码转换**：使用 UTF-8 和 Unicode 转换工具

## 未来改进

1. **异步操作**：添加后台线程支持，避免界面卡顿
2. **进度显示**：为耗时操作添加进度条
3. **多语言支持**：添加国际化支持
4. **主题支持**：添加自定义主题功能
5. **插件系统**：支持第三方插件扩展

## 总结

通过模块化重构，xpkgui 的代码结构更加清晰，易于维护和扩展。新的架构解决了原有代码的主要问题，提供了更好的用户体验和开发体验。
