# xpkgui 完整功能规格说明

## 文档版本历史

| 版本 | 日期 | 说明 |
|------|------|------|
| 1.0 | 2026-01-31 | 初始版本，包含 Shell 集成和功能补全方案 |

---

## 一、项目概述

### 1.1 目标
为 xpkgui 图形界面工具提供完整的功能规格，包括 Shell 集成、GUI 功能增强以及与 xpkcon 命令行工具的协同工作。

### 1.2 工具分工

| 工具 | 用途 | 适用场景 |
|------|------|---------|
| **xpkgui.exe** | 图形界面管理工具 | 日常文件压缩/解压、可视化操作 |
| **xpkcon.exe** | 命令行工具 | 批处理、脚本、自动化任务 |
| **xpkshext.dll** | Shell 扩展 DLL | 资源管理器右键菜单集成 |

### 1.3 架构图

```
┌─────────────────────────────────────────────────────────┐
│                   Windows Explorer                     │
└─────────────────────────────────────────────────────────┘
                           │
        ┌──────────────────┼──────────────────┐
        ▼                  ▼                  ▼
┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│ Context Menu │  │ Icon Handler │  │ Info Tip     │
│   Handler    │  │              │  │   Handler    │
└──────────────┘  └──────────────┘  └──────────────┘
        │                  │                  │
        └──────────────────┼──────────────────┘
                           ▼
                   ┌──────────────┐
                   │ xpkshext.dll │
                   │  (COM DLL)   │
                   └──────────────┘
                           │
        ┌──────────────────┼──────────────────┐
        ▼                  ▼                  ▼
┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│  xpkgui.exe  │  │ xpkcon.exe   │  │  xPack DLL   │
│    GUI       │  │   CLI        │  │   库文件     │
└──────────────┘  └──────────────┘  └──────────────┘
```

---

## 二、Shell 集成方案

### 2.1 文件关联

#### 2.1.1 注册表配置

```reg
Windows Registry Editor Version 5.00

; .xpk 文件类型定义
[HKEY_CLASSES_ROOT\.xpk]
@="xPack.File"
"PerceivedType"="compressed"
"Content Type"="application/x-xpack"

; xPack 文件类型描述
[HKEY_CLASSES_ROOT\xPack.File]
@="xPack 压缩包"
"FriendlyTypeName"="xPack 压缩包"

; 默认图标
[HKEY_CLASSES_ROOT\xPack.File\DefaultIcon]
@="\"C:\\Program Files\\xPack\\xpkgui.exe\",0"

; 打开命令（GUI）
[HKEY_CLASSES_ROOT\xPack.File\shell\open\command]
@="\"C:\\Program Files\\xPack\\xpkgui.exe\" \"%1\""

; 管理菜单项
[HKEY_CLASSES_ROOT\xPack.File\shell\manage]
@="使用 xpkgui 管理文件"
"Icon"="\"C:\\Program Files\\xPack\\xpkgui.exe\",0"

[HKEY_CLASSES_ROOT\xPack.File\shell\manage\command]
@="\"C:\\Program Files\\xPack\\xpkgui.exe\" \"%1\""
```

### 2.2 右键菜单集成

#### 2.2.1 菜单项设计

**.xpk 文件右键菜单：**
```
┌────────────────────────┐
│ 打开 xpkgui         │
│ ──────────────────── │
│ 解压到...           │
│ 解压到当前文件夹    │
│ ──────────────────── │
│ 验证压缩包         │
│ 属性               │
└────────────────────────┘
```

**普通文件/文件夹右键菜单：**
```
┌────────────────────────┐
│ 添加到 xPack...     │
│ 添加到 xPack (自命名)│
└────────────────────────┘
```

#### 2.2.2 Shell Extension DLL 设计

**文件结构：**
```
tools/xpkgui/
├── xpkshext.c           # Shell Extension 源代码
├── xpkshext.h           # 头文件
├── xpkshext.def         # DLL 导出定义
├── xpkshext.rc          # 资源文件
├── build_shext_x64.bat   # 64位编译脚本
├── build_shext_x86.bat   # 32位编译脚本
```

**核心接口：**
```c
// xpkshext.h

// CLSID 定义
// {A1B2C3D4-E5F6-7890-ABCD-EF1234567890}
static const GUID CLSID_XPKSHELLEXT = {
    0xA1B2C3D4, 0xE5F6, 0x7890, 
    {0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x90}
};

// IContextMenu 实现
typedef struct _XPKShellExt {
    IContextMenuVtbl *lpVtbl;
    IShellExtInitVtbl *lpVtblInit;
    LONG refCount;
    HWND hwnd;
    char selectedFiles[MAX_PATH * 10];
    int fileCount;
    int isXpkFile;
} XPKShellExt;

// 导出函数
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
STDAPI DllCanUnloadNow(void);
```

**菜单命令映射：**

| 菜单项 | ID | 调用命令 |
|--------|----|----------|
| 打开 xpkgui | 0 | `xpkgui.exe "%1"` |
| 解压到... | 1 | `xpkgui.exe -extract "%1"` |
| 解压到当前文件夹 | 2 | `xpkgui.exe -extract_here "%1"` |
| 验证压缩包 | 3 | `xpkgui.exe -verify "%1"` |
| 属性 | 4 | `xpkgui.exe -properties "%1"` |
| 添加到 xPack... | 10 | `xpkgui.exe -add "%1"` |
| 添加到 xPack (自命名) | 11 | `xpkgui.exe -add_auto "%1"` |

#### 2.2.3 Shell 扩展注册

```reg
Windows Registry Editor Version 5.00

; 注册 Shell 扩展
[HKEY_CLASSES_ROOT\CLSID\{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}]
@="xPack Shell Extension"

[HKEY_CLASSES_ROOT\CLSID\{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}\InprocServer32]
@="C:\\Program Files\\xPack\\xpkshext.dll"
"ThreadingModel"="Apartment"

; .xpk 文件右键菜单
[HKEY_CLASSES_ROOT\.xpk\shellex\ContextMenuHandlers\XPKShell]
@="{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}"

; * (所有文件) 右键菜单
[HKEY_CLASSES_ROOT\*\shellex\ContextMenuHandlers\XPKShell]
@="{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}"

; Directory (文件夹) 右键菜单
[HKEY_CLASSES_ROOT\Directory\shellex\ContextMenuHandlers\XPKShell]
@="{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}"

; Folder (文件夹背景) 右键菜单
[HKEY_CLASSES_ROOT\Folder\shellex\ContextMenuHandlers\XPKShell]
@="{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}"
```

### 2.3 图标叠加（Icon Handler）

```c
// IExtractIcon 接口实现
HRESULT STDMETHODCALLTYPE GetIconLocation(IExtractIcon* This, 
    UINT uFlags, LPSTR szIconFile, UINT cchMax, int* piIndex, 
    UINT* pwFlags)
{
    strcpy(szIconFile, "C:\\Program Files\\xPack\\xpkgui.exe");
    *piIndex = 0;
    *pwFlags = GIL_NOTFILENAME | GIL_PERINSTANCE;
    return S_OK;
}
```

### 2.4 信息提示（Info Tip Handler）

```c
// IQueryInfo 接口实现
HRESULT STDMETHODCALLTYPE GetInfoTip(IQueryInfo* This,
    DWORD dwFlags, LPWSTR* ppwszTip)
{
    XPKInfoTipHandler* pHandler = (XPKInfoTipHandler*)This;
    
    xpkObject xpk = xpkOpen(pHandler->filePath, 0, 1);
    if (xpk) {
        xpkStat stat;
        xpkStatGet(xpk, &stat);
        
        wchar_t tip[512];
        swprintf(tip, 
            L"xPack 压缩包\n文件数: %u\n大小: %llu bytes\n压缩率: %.1f%%",
            stat.fileCount, stat.totalSize, stat.ratio * 100);
        
        *ppwszTip = (LPWSTR)CoTaskMemAlloc((wcslen(tip) + 1) * sizeof(wchar_t));
        wcscpy(*ppwszTip, tip);
        
        xpkClose(xpk);
        return S_OK;
    }
    
    return E_FAIL;
}
```

---

## 三、xpkgui GUI 功能补全

### 3.1 压缩级别选择功能

#### 3.1.1 功能描述
在添加文件/重命名文件时提供压缩级别选择对话框。

#### 3.1.2 级别描述映射表

| 级别 | 算法 | 描述 | 原生级别 |
|------|------|------|---------|
| 0 | STORE | 无压缩 | 0 |
| 1 | LZ4 | LZ4 快速压缩 | 1 |
| 2 | LZ4 | LZ4 快速压缩 (64KB) | 2 |
| 3 | LZ4-HC | LZ4-HC 高质量压缩 | 4 |
| 4 | LZ4-HC | LZ4-HC 最高质量压缩 | 12 |
| 5 | ZSTD | ZSTD 极速压缩 | FAST |
| 6 | ZSTD | ZSTD 双倍快速压缩 | DFAST |
| 7 | ZSTD | ZSTD 贪婪压缩 (默认) | GREEDY |
| 8 | ZSTD | ZSTD 延迟压缩 | LAZY |
| 9 | ZSTD | ZSTD 延迟压缩2 | LAZY2 |
| 10 | ZSTD | ZSTD 二叉树延迟压缩2 | BTLAZY2 |
| 11 | ZSTD | ZSTD 二叉树优化压缩 | BTOPT |
| 12 | ZSTD | ZSTD 二叉树极致压缩 | BTULTRA |
| 13 | ZSTD | ZSTD 二叉树极致压缩2 | BTULTRA2 |
| 14 | LZMA2 | LZMA2 标准压缩 | 6 |
| 15 | LZMA2 | LZMA2 极致压缩 | 9 |

#### 3.1.3 对话框设计

```
┌──────────────────────────────────────────┐
│  选择压缩级别                            │
├──────────────────────────────────────────┤
│                                          │
│  压缩级别: [ZSTD 贪婪压缩 (默认) ▼]       │
│                                          │
│  级别说明:                                │
│  ZSTD 贪婪模式提供良好的压缩比和速度平衡   │
│                                          │
│  [确定] [取消]                           │
└──────────────────────────────────────────┘
```

#### 3.1.4 新增/修改函数

```c
int CompressLevelDialog(HWND hwnd, int* level);

// 修改 AddFiles() - 调用压缩级别对话框
int AddFiles(void) {
    // ...
    int level = 7;
    if (CompressLevelDialog(g_hMainWnd, &level) == IDOK) {
        // 使用选择的级别添加文件
    }
}

// 修改 RenameFiles() - 支持选择压缩级别
int RenameFiles(void) {
    // ...
    int level = 7;
    if (CompressLevelDialog(g_hMainWnd, &level) == IDOK) {
        // 使用选择的级别更新文件
    }
}
```

### 3.2 包类型选择功能

#### 3.2.1 功能描述
在新建压缩包时允许用户选择包类型。

#### 3.2.2 对话框设计

```
┌──────────────────────────────────────────┐
│  新建 xPack 压缩包                      │
├──────────────────────────────────────────┤
│                                          │
│  压缩包路径: [_______________] [浏览...]  │
│                                          │
│  包类型:                                  │
│  ◉ Win32 (推荐)  - 路径访问，不区分大小写 │
│  ○ Linux        - 路径访问，区分大小写    │
│  ○ Index        - 整数索引访问             │
│  ○ Core         - 顺序位置访问             │
│                                          │
│  压缩模式: □ 启用固实压缩                 │
│                                          │
│  [确定] [取消]                           │
└──────────────────────────────────────────┘
```

#### 3.2.3 修改函数

```c
int NewPackageDialog(HWND hwnd, char* path, int* solidMode, int* pkgType);
```

### 3.3 批量添加目录功能

#### 3.3.1 功能描述
支持选择目录并递归添加其中的所有文件。

#### 3.3.2 UI 设计

- 文件菜单新增"添加目录..."菜单项 (ID_FILE_ADDDIR)
- 目录浏览对话框
- 进度显示

#### 3.3.3 新增函数

```c
int AddDirectory(void);
int BrowseForDirectory(HWND hwnd, char* path, const char* title);
```

#### 3.3.4 调用 xpkcon 的替代方案

对于复杂的批量操作，可以调用 xpkcon：

```c
int AddDirectoryViaCLI(const char* dirPath) {
    char cmd[MAX_PATH * 2];
    sprintf(cmd, "xpkcon a \"%s\" -r -l7 \"%s\"", g_xpkPath, dirPath);
    return system(cmd);
}
```

### 3.4 文件信息增强显示

#### 3.4.1 UI 修改

- 文件列表新增列：哈希值、文件类型
- 属性对话框显示详细信息

#### 3.4.2 文件类型映射

| 类型值 | 名称 | 说明 |
|--------|------|------|
| 0 | 未知 | 通用文件 |
| 1 | 二进制 | 可执行文件等 |
| 2 | 文本 | 文本文件 |
| 3 | 图像 | 图片文件 |
| 4 | 音频 | 音频文件 |
| 5 | 视频 | 视频文件 |
| 6 | 归档 | 其他压缩包 |
| 15 | 目录 | 目录标记 |

#### 3.4.3 修改函数

```c
const char* GetFileTypeString(int type);

void RefreshFileList(void) {
    // 添加哈希值和文件类型列
    // ...
}

int GetPackageProperties(void) {
    // 显示更多信息
    // ...
}
```

### 3.5 模式匹配操作功能

#### 3.5.1 功能描述
支持按通配符模式选择多个文件进行批量操作。

#### 3.5.2 UI 设计

```
┌──────────────────────────────────────────┐
│  模式匹配操作                           │
├──────────────────────────────────────────┤
│                                          │
│  文件模式: [*.txt ▼]                     │
│                                          │
│  操作:                                   │
│  ◉ 解压到...                             │
│  ○ 删除                                 │
│  ○ 重命名...                             │
│                                          │
│  [执行] [取消]                           │
└──────────────────────────────────────────┘
```

#### 3.5.3 新增函数

```c
int SelectByPattern(void);
int DeleteSelectedPattern(const char* pattern);
int ExtractSelectedPattern(const char* pattern, const char* dstDir);
```

### 3.6 识别代码管理功能

#### 3.6.1 功能描述
允许用户查看和设置压缩包的识别代码。

#### 3.6.2 UI 设计

- 工具菜单新增"设置识别代码..."菜单项 (ID_TOOLS_DISCCODE)
- 属性对话框显示识别代码

#### 3.6.3 新增函数

```c
int SetDiscCode(void);
```

### 3.7 错误处理增强

#### 3.7.1 功能描述
使用 xPack 的错误处理 API 提供更详细的错误信息。

#### 3.7.2 实现方式

```c
void ErrorHandler(int code, const char* message) {
    char buf[512];
    sprintf(buf, "错误代码: %d\n错误信息: %s", code, message);
    ErrorMsg(g_hMainWnd, buf);
}

void ShowDetailedError(HWND hwnd, const char* context) {
    int lastError = xpkLastError();
    const char* lastErrorMsg = xpkLastErrorMsg();
    
    char msg[512];
    sprintf(msg, "%s\n\n错误代码: %d\n错误信息: %s", 
        context, lastError, lastErrorMsg);
    MessageBox(hwnd, msg, "错误", MB_OK | MB_ICONERROR);
}
```

### 3.8 拖放支持

#### 3.8.1 功能描述
支持拖拽文件到 xpkgui 窗口进行压缩。

#### 3.8.2 实现

```c
// 在 MainWndProc 中添加
case WM_DROPFILES:
    {
        HDROP hDrop = (HDROP)wParam;
        UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
        
        for (UINT i = 0; i < fileCount; i++) {
            char filePath[MAX_PATH];
            DragQueryFile(hDrop, i, filePath, MAX_PATH);
            
            if (g_xpk != NULL) {
                char fileName[MAX_PATH];
                const char* slash = strrchr(filePath, '\\');
                strcpy(fileName, slash ? slash + 1 : filePath);
                
                xpkPathAppendFile(g_xpk, fileName, filePath, 7);
            }
        }
        
        DragFinish(hDrop);
        RefreshFileList();
    }
    break;

// 初始化时启用拖放
DragAcceptFiles(g_hMainWnd, TRUE);
```

### 3.9 快捷键支持

#### 3.9.1 快捷键映射

| 快捷键 | 功能 |
|--------|------|
| Ctrl+N | 新建压缩包 |
| Ctrl+O | 打开压缩包 |
| Ctrl+S | 保存压缩包 |
| Ctrl+A | 添加文件 |
| Ctrl+D | 添加目录 |
| Ctrl+E | 解压文件 |
| Ctrl+F | 查找文件 |
| Del | 删除文件 |
| F2 | 重命名文件 |
| F5 | 刷新 |
| Ctrl+Q | 退出 |

### 3.10 收藏夹/最近文件

#### 3.10.1 功能描述
保存最近打开的压缩包历史。

#### 3.10.2 配置文件

```
; %APPDATA%\xPack\history.ini
[History]
Count=5
Path1=C:\Data\archive1.xpk
Path2=D:\Backup\backup.xpk
Path3=C:\Downloads\files.xpk
Path4=E:\Projects\data.xpk
Path5=C:\Temp\test.xpk
```

#### 3.10.3 新增函数

```c
void LoadHistory(void);
void SaveHistory(const char* path);
void ShowHistoryMenu(void);
```

### 3.11 配置管理

#### 3.11.1 功能描述
保存用户偏好设置。

#### 3.11.2 配置文件

```
; %APPDATA%\xPack\settings.ini
[Settings]
DefaultCompLevel=7
DefaultPkgType=3
SolidMode=0
ConfirmDelete=1
OverwriteFiles=0
ShowStatusBar=1
ShowGridLines=1
[Window]
Width=900
Height=600
Maximized=0
```

#### 3.11.3 新增函数

```c
void LoadSettings(void);
void SaveSettings(void);
```

### 3.12 压缩测试功能

#### 3.12.1 功能描述
在不解压的情况下测试压缩包完整性。

#### 3.12.2 新增菜单项和函数

```c
// 菜单项: ID_TOOLS_TEST
// 新增函数
int TestPackage(void) {
    if (g_xpk == NULL) {
        ErrorMsg(g_hMainWnd, "请先打开一个压缩包");
        return -1;
    }
    
    int failed = xpkVerifyAll(g_xpk);
    
    if (failed == 0) {
        InfoMsg(g_hMainWnd, "所有文件测试通过");
    } else if (failed > 0) {
        char msg[256];
        sprintf(msg, "测试完成: %d 个文件有问题", failed);
        ErrorMsg(g_hMainWnd, msg);
    } else {
        ErrorMsg(g_hMainWnd, "测试过程出错");
    }
    
    return 0;
}
```

---

## 四、xpkgui 命令行参数支持

### 4.1 命令行参数设计

xpkgui 支持以下命令行参数模式，便于与 Shell Extension 和脚本集成：

| 参数 | 功能 |
|------|------|
| `<文件名>` | 打开指定的 .xpk 文件 |
| `-extract <文件>` | 提取模式：显示提取对话框 |
| `-extract_here <文件>` | 提取到当前文件夹 |
| `-add <文件>` | 添加模式：显示添加对话框 |
| `-add_auto <文件>` | 添加模式（自动命名） |
| `-verify <文件>` | 验证压缩包并显示结果 |
| `-properties <文件>` | 显示压缩包属性 |

### 4.2 命令行参数实现

```c
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
    LPSTR lpCmdLine, int nCmdShow)
{
    if (strlen(lpCmdLine) > 0) {
        char cmd[64];
        char path[MAX_PATH];
        
        // 解析命令行
        if (lpCmdLine[0] == '-') {
            sscanf(lpCmdLine, "%s \"%[^\"]\"", cmd, path);
        } else {
            strcpy(cmd, "open");
            strcpy(path, lpCmdLine);
            if (path[0] == '"') {
                strcpy(path, path + 1);
                path[strlen(path) - 1] = '\0';
            }
        }
        
        // 根据命令执行相应操作
        if (strcmp(cmd, "-extract") == 0) {
            return ExtractModeMain(path);
        } else if (strcmp(cmd, "-extract_here") == 0) {
            return ExtractHereModeMain(path);
        } else if (strcmp(cmd, "-add") == 0) {
            return AddModeMain(path);
        } else if (strcmp(cmd, "-add_auto") == 0) {
            return AddAutoModeMain(path);
        } else if (strcmp(cmd, "-verify") == 0) {
            return VerifyModeMain(path);
        } else if (strcmp(cmd, "-properties") == 0) {
            return PropertiesModeMain(path);
        } else if (strcmp(cmd, "open") == 0) {
            g_hInstance = hInstance;
            InitMainWindow();
            OpenXpkFile(path);
        }
    } else {
        // 正常 GUI 启动
        g_hInstance = hInstance;
        InitMainWindow();
    }
    
    return MessageLoop();
}
```

### 4.3 模式函数实现

```c
int ExtractModeMain(const char* archivePath) {
    xpkObject xpk = xpkOpen(archivePath, 0, 0);
    if (!xpk) {
        fprintf(stderr, "Error: Cannot open archive '%s'\n", archivePath);
        return 1;
    }
    
    char outDir[MAX_PATH];
    if (BrowseForFolder(NULL, outDir, "选择解压目录")) {
        if (xpkExtractAll(xpk, outDir) == 0) {
            printf("Extracted to: %s\n", outDir);
            xpkClose(xpk);
            return 0;
        }
    }
    
    xpkClose(xpk);
    return 1;
}

int ExtractHereModeMain(const char* archivePath) {
    char outDir[MAX_PATH];
    strcpy(outDir, archivePath);
    PathRemoveFileSpec(outDir);
    
    xpkObject xpk = xpkOpen(archivePath, 0, 0);
    if (!xpk) {
        fprintf(stderr, "Error: Cannot open archive '%s'\n", archivePath);
        return 1;
    }
    
    if (xpkExtractAll(xpk, outDir) == 0) {
        printf("Extracted to: %s\n", outDir);
        xpkClose(xpk);
        return 0;
    }
    
    xpkClose(xpk);
    return 1;
}

int VerifyModeMain(const char* archivePath) {
    xpkObject xpk = xpkOpen(archivePath, 0, 0);
    if (!xpk) {
        fprintf(stderr, "Error: Cannot open archive '%s'\n", archivePath);
        return 1;
    }
    
    int result = xpkVerifyAll(xpk);
    
    if (result == 0) {
        printf("All files verified successfully\n");
    } else if (result > 0) {
        printf("Verification failed: %d file(s) have errors\n", result);
    } else {
        printf("Verification error occurred\n");
    }
    
    xpkClose(xpk);
    return result != 0;
}
```

---

## 五、xpkcon 命令行工具集成

### 5.1 与 xpkgui 的分工

| 操作类型 | xpkgui | xpkcon |
|---------|--------|--------|
| 日常文件压缩/解压 | ✅ 主要 | ✅ 辅助 |
| 批量处理 | ⚠️ 有限 | ✅ 主要 |
| 脚本自动化 | ❌ 不支持 | ✅ 主要 |
| 递归目录操作 | ⚠️ 有限 | ✅ 主要 |
| 可视化操作 | ✅ 主要 | ❌ 不支持 |
| 通配符匹配 | ⚠️ 有限 | ✅ 主要 |

### 5.2 xpkcon 调用方式

#### 5.2.1 在 xpkgui 中调用 xpkcon

对于复杂的批量操作，xpkgui 可以调用 xpkcon：

```c
int CallXpkcon(const char* cmdLine) {
    STARTUPINFO si = {sizeof(si)};
    PROCESS_INFORMATION pi;
    
    char fullCmd[MAX_PATH * 2];
    sprintf(fullCmd, "xpkcon %s", cmdLine);
    
    if (CreateProcess(NULL, fullCmd, NULL, NULL, FALSE, 
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return exitCode;
    }
    
    return -1;
}
```

#### 5.2.2 使用场景示例

**批量添加目录：**
```c
int AddDirectoryViaXpkcon(const char* dirPath) {
    char cmd[MAX_PATH * 2];
    sprintf(cmd, "a \"%s\" -r -l7 \"%s\"", g_xpkPath, dirPath);
    return CallXpkcon(cmd);
}
```

**模式匹配提取：**
```c
int ExtractPatternViaXpkcon(const char* pattern, const char* dstDir) {
    char cmd[MAX_PATH * 2];
    sprintf(cmd, "x \"%s\" -o\"%s\" %s", g_xpkPath, dstDir, pattern);
    return CallXpkcon(cmd);
}
```

### 5.3 xpkcon 功能总结

xpkcon 已实现以下完整功能（参考 [xpkcon/README.md](../xpkcon/README.md)）：

| 命令 | 功能 |
|------|------|
| `a` | 添加文件到压缩包 |
| `x` | 完整路径解压文件 |
| `e` | 提取文件到当前目录 |
| `l` | 列出压缩包内容 |
| `t` | 测试压缩包完整性 |
| `d` | 从压缩包删除文件 |
| `u` | 更新压缩包中的文件 |
| `i` | 显示压缩包详细信息 |

---

## 六、安装和部署

### 6.1 安装脚本 (install.bat)

```batch
@echo off
setlocal

set INSTALL_DIR=%ProgramFiles%\xPack
set EXE_PATH=%INSTALL_DIR%\xpkgui.exe
set CLI_PATH=%INSTALL_DIR%\xpkcon.exe
set SHELL_DLL=%INSTALL_DIR%\xpkshext.dll

echo Installing xPack...

; 创建安装目录
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"

; 复制文件
copy xpkgui.exe "%EXE_PATH%" /Y
copy xpkcon.exe "%CLI_PATH%" /Y
copy xpkshext.dll "%SHELL_DLL%" /Y
copy xPack.dll "%INSTALL_DIR%\xPack.dll" /Y

; 添加到 PATH
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path /t REG_EXPAND_SZ /d "%PATH%;%INSTALL_DIR%" /f

; 注册文件关联
reg add HKCR\.xpk /ve /d xPack.File /f
reg add HKCR\xPack.File /ve /d "xPack 压缩包" /f
reg add HKCR\xPack.File\DefaultIcon /ve /d "\"%EXE_PATH%\",0" /f
reg add HKCR\xPack.File\shell\open\command /ve /d "\"%EXE_PATH%\" \"%%1\"" /f

; 注册 Shell 扩展
regsvr32 /s "%SHELL_DLL%"

; 刷新图标缓存
ie4uinit.exe -show

echo xPack 安装完成！
echo 请注销并重新登录以使 PATH 环境变量生效。
pause
```

### 6.2 卸载脚本 (uninstall.bat)

```batch
@echo off
setlocal

set INSTALL_DIR=%ProgramFiles%\xPack
set EXE_PATH=%INSTALL_DIR%\xpkgui.exe
set CLI_PATH=%INSTALL_DIR%\xpkcon.exe
set SHELL_DLL=%INSTALL_DIR%\xpkshext.dll

echo Uninstalling xPack...

; 注销 Shell 扩展
regsvr32 /u /s "%SHELL_DLL%"

; 删除文件关联
reg delete HKCR\.xpk /f
reg delete HKCR\xPack.File /f

; 删除文件
del /F /Q "%EXE_PATH%"
del /F /Q "%CLI_PATH%"
del /F /Q "%SHELL_DLL%"
del /F /Q "%INSTALL_DIR%\xPack.dll"

; 删除安装目录（如果为空）
rd "%INSTALL_DIR%" 2>nul

; 刷新图标缓存
ie4uinit.exe -show

echo xPack 卸载完成！
pause
```

---

## 七、功能优先级和实施计划

### 7.1 Phase 1: Shell 集成 (P0 - 核心功能)

| 任务 | 优先级 | 预计工时 |
|------|--------|---------|
| 实现 xpkshext.dll Shell Extension | P0 | 8h |
| 注册表配置和安装脚本 | P0 | 2h |
| xpkgui 命令行参数支持 | P0 | 4h |
| 文件关联和图标 | P0 | 2h |
| **小计** | | **16h** |

### 7.2 Phase 2: 基础功能增强 (P0 - 核心功能)

| 任务 | 优先级 | 预计工时 |
|------|--------|---------|
| 压缩级别选择对话框 | P0 | 3h |
| 包类型选择对话框 | P0 | 2h |
| 批量添加目录 | P0 | 3h |
| 错误处理增强 | P0 | 2h |
| 拖放支持 | P0 | 2h |
| **小计** | | **12h** |

### 7.3 Phase 3: 用户体验提升 (P1 - 重要功能)

| 任务 | 优先级 | 预计工时 |
|------|--------|---------|
| 收藏夹/最近文件 | P1 | 4h |
| 配置管理 | P1 | 3h |
| 快捷键支持 | P1 | 2h |
| 压缩测试功能 | P1 | 2h |
| 图标叠加和 Info Tip | P1 | 4h |
| **小计** | | **15h** |

### 7.4 Phase 4: 高级功能 (P2 - 可选功能)

| 任务 | 优先级 | 预计工时 |
|------|--------|---------|
| 模式匹配操作 | P2 | 4h |
| 识别代码管理 | P2 | 2h |
| 文件信息增强显示 | P2 | 3h |
| 分卷压缩 | P2 | 6h |
| 主题支持 | P2 | 4h |
| 视图模式切换 | P2 | 2h |
| 文件搜索 | P2 | 3h |
| **小计** | | **24h** |

### 7.5 总计

| 阶段 | 预计工时 |
|------|---------|
| Phase 1 | 16h |
| Phase 2 | 12h |
| Phase 3 | 15h |
| Phase 4 | 24h |
| **总计** | **67h** |

---

## 八、新增文件清单

| 文件路径 | 用途 |
|---------|------|
| `tools/xpkgui/xpkshext.c` | Shell Extension 源代码 |
| `tools/xpkgui/xpkshext.h` | Shell Extension 头文件 |
| `tools/xpkgui/xpkshext.def` | DLL 导出定义 |
| `tools/xpkgui/xpkshext.rc` | Shell Extension 资源 |
| `tools/xpkgui/build_shext_x64.bat` | Shell Extension 64位编译脚本 |
| `tools/xpkgui/build_shext_x86.bat` | Shell Extension 32位编译脚本 |
| `tools/xpkgui/xpkgui.reg` | 文件关联注册表 |
| `tools/xpkgui/xpkshext.reg` | Shell 扩展注册表 |
| `tools/xpkgui/install.bat` | 安装脚本 |
| `tools/xpkgui/uninstall.bat` | 卸载脚本 |
| `tools/xpkgui/xpkgui.ico` | 程序图标 |

---

## 九、与 7-Zip 功能对比

| 功能 | 7-Zip | xpkgui (当前) | xpkgui (目标) |
|------|-------|--------------|--------------|
| 基础压缩/解压 | ✅ | ✅ | ✅ |
| 多压缩算法 | ✅ | ✅ | ✅ |
| 包类型选择 | ❌ | ❌ | ✅ |
| Shell 扩展 | ✅ | ❌ | ✅ |
| 右键菜单 | ✅ | ❌ | ✅ |
| 文件关联 | ✅ | ❌ | ✅ |
| 拖放支持 | ✅ | ❌ | ✅ |
| 信息提示 | ✅ | ❌ | ✅ |
| 分卷压缩 | ✅ | ❌ | ✅ (P2) |
| 压缩测试 | ✅ | ❌ | ✅ |
| 收藏夹 | ✅ | ❌ | ✅ |
| 配置管理 | ✅ | ❌ | ✅ |
| 主题支持 | ❌ | ❌ | ✅ (P2) |
| 固实压缩 | ✅ | ✅ | ✅ |
| 批量操作 | ✅ | 部分 | ✅ |
| 命令行工具 | ✅ | ✅ | ✅ (xpkcon) |

---

## 十、开发注意事项

### 10.1 COM 接口实现要点

1. Shell Extension DLL 必须实现以下接口：
   - `IUnknown` (基类)
   - `IShellExtInit` (初始化)
   - `IContextMenu` (右键菜单)

2. 线程模型必须使用 `Apartment` (STA)

3. DLL 必须导出 `DllGetClassObject` 和 `DllCanUnloadNow`

### 10.2 安全考虑

1. 所有文件操作应检查文件权限
2. 解压时防止路径遍历攻击
3. 处理大文件时应有进度提示
4. 错误信息不应泄露敏感信息

### 10.3 兼容性考虑

1. 支持 Windows 7 及以上版本
2. 同时提供 32 位和 64 位版本
3. 考虑高 DPI 显示支持
4. 支持非 ASCII 路径（Unicode）

### 10.4 性能优化

1. 大量文件操作使用异步处理
2. 文件列表使用虚拟化（超过 1000 项）
3. 缓存压缩包统计信息
4. 延迟加载文件详情

---

## 十一、测试计划

### 11.1 单元测试

| 模块 | 测试内容 |
|------|---------|
| Shell Extension | COM 接口、菜单项、命令执行 |
| 压缩级别对话框 | 级别选择、UI 响应 |
| 包类型选择 | 类型切换、模式限制 |
| 拖放操作 | 单文件、多文件、目录 |

### 11.2 集成测试

| 测试场景 | 测试内容 |
|---------|---------|
| 文件关联 | 双击打开、图标显示 |
| 右键菜单 | 各菜单项功能 |
| 命令行参数 | 各参数模式 |
| xpkcon 调用 | 批量操作、错误处理 |

### 11.3 兼容性测试

| 测试环境 | 测试内容 |
|---------|---------|
| Windows 7 | 基础功能 |
| Windows 10 | 完整功能 |
| Windows 11 | 完整功能 |
| 32 位系统 | 32 位 DLL |
| 64 位系统 | 64 位 DLL |

---

## 十二、文档和帮助

### 12.1 用户文档

1. **用户手册** (user_manual.md)
   - 基本操作指南
   - 功能说明
   - 快捷键列表
   - 常见问题

2. **API 文档** (api_reference.md)
   - Shell Extension 接口
   - 命令行参数
   - 配置文件格式

### 12.2 开发文档

1. **开发指南** (development_guide.md)
   - 编译说明
   - 代码结构
   - 扩展开发

2. **发布说明** (CHANGELOG.md)
   - 版本历史
   - 新增功能
   - 已知问题

---

## 附录

### A. 压缩级别详细说明

| 级别 | 算法 | 原生级别 | 速度 | 压缩比 | 推荐场景 |
|------|------|---------|------|--------|---------|
| 0 | STORE | 0 | 极快 | 无 | 已压缩文件 |
| 1-2 | LZ4 | 1-2 | 极快 | 低 | 实时压缩 |
| 3-4 | LZ4-HC | 4,12 | 快 | 中 | 日志文件 |
| 5-13 | ZSTD | 1-9 | 中-慢 | 高 | 通用压缩 |
| 14-15 | LZMA2 | 6,9 | 慢 | 极高 | 归档存储 |

### B. 包类型选择指南

| 包类型 | 特点 | 适用场景 | 不适用场景 |
|--------|------|---------|-----------|
| Win32 | 路径不区分大小写 | Windows 平台 | Linux 系统 |
| Linux | 路径区分大小写 | Linux/Unix | Windows 平台 |
| Index | 整数索引访问 | 数据库、缓存 | 通用文件 |
| Core | 顺序位置访问 | 流式处理 | 随机访问 |

### C. 常见问题 (FAQ)

**Q: 为什么 Shell Extension 不工作？**
A: 需要使用 `regsvr32` 注册 DLL，并重启资源管理器。

**Q: 如何禁用右键菜单？**
A: 运行 `regsvr32 /u xpkshext.dll` 注销 Shell Extension。

**Q: xpkgui 和 xpkcon 有什么区别？**
A: xpkgui 是图形界面工具，适合日常使用；xpkcon 是命令行工具，适合批处理和脚本。

**Q: 支持哪些压缩格式？**
A: xpkgui 专注于 xPack 格式。其他格式需要先转换。

**Q: 如何恢复默认设置？**
A: 删除 `%APPDATA%\xPack\settings.ini` 文件。

---

**文档结束**
