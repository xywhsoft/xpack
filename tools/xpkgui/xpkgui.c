#define UNICODE
#define _UNICODE

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shlobj.h>
#include <time.h>
#include <wchar.h>
#include "resource.h"
#include "../../lib/xrt/xrt.h"
#include "../../src/xpack.h"

#pragma comment(lib, "shell32")
#pragma comment(lib, "comctl32")
#pragma comment(lib, "shlwapi")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define MAX_RECENT_FILES 10
#define MAX_HISTORY 20
#define MAX_PATH_W 32767

typedef LPITEMIDLIST (__stdcall *PFNSHBROWSEFORFOLDERW)(BROWSEINFOW*);
typedef BOOL (__stdcall *PFNSHGETPATHFROMIDLISTW)(LPITEMIDLIST, LPWSTR);
typedef void (__stdcall *PFNCoTaskMemFree)(LPVOID);

static HMODULE hShell32 = NULL;
static PFNSHBROWSEFORFOLDERW pSHBrowseForFolderW = NULL;
static PFNSHGETPATHFROMIDLISTW pSHGetPathFromIDListW = NULL;
static PFNCoTaskMemFree pCoTaskMemFree = NULL;

typedef struct {
	wchar_t path[MAX_PATH_W];
	time_t timestamp;
} HistoryItem;

typedef struct {
	int defaultCompLevel;
	int defaultPkgType;
	int solidMode;
	int volumeMode;
	uint32_t volumeSize;
	int confirmDelete;
	int overwriteFiles;
	int showStatusBar;
	int showGridLines;
	int windowWidth;
	int windowHeight;
	int windowMaximized;
} Settings;

typedef enum {
	CMD_NORMAL,
	CMD_EXTRACT,
	CMD_EXTRACT_HERE,
	CMD_ADD,
	CMD_ADD_AUTO,
	CMD_VERIFY,
	CMD_PROPERTIES
} CommandMode;

HINSTANCE g_hInstance = NULL;
HWND g_hMainWnd = NULL;
HWND g_hFileList = NULL;
HWND g_hStatusBar = NULL;
xpkObject g_xpk = NULL;
wchar_t g_xpkPath[MAX_PATH_W] = {0};
wchar_t g_currentDir[MAX_PATH_W] = {0};
CommandMode g_commandMode = CMD_NORMAL;
wchar_t g_commandPath[MAX_PATH_W] = {0};

Settings g_settings = {7, XPK_TYPE_WIN32, 0, 0, 0, 1, 0, 1, 1, 900, 600, 0};
HistoryItem g_history[MAX_RECENT_FILES] = {0};
int g_historyCount = 0;

static const wchar_t* g_compLevelDesc[] = {
	L"无压缩 (STORE)",
	L"LZ4 快速压缩",
	L"LZ4 快速压缩 (64KB)",
	L"LZ4-HC 高质量压缩",
	L"LZ4-HC 最高质量压缩",
	L"ZSTD 极速压缩",
	L"ZSTD 双倍快速压缩",
	L"ZSTD 贪婪压缩 (默认)",
	L"ZSTD 延迟压缩",
	L"ZSTD 延迟压缩2",
	L"ZSTD 二叉树延迟压缩2",
	L"ZSTD 二叉树优化压缩",
	L"ZSTD 二叉树极致压缩",
	L"ZSTD 二叉树极致压缩2",
	L"LZMA2 标准压缩",
	L"LZMA2 极致压缩"
};

static const wchar_t* g_pkgTypeDesc[] = {
	L"Win32 (推荐)  - 路径访问，不区分大小写",
	L"Linux        - 路径访问，区分大小写",
	L"Index        - 整数索引访问",
	L"Core         - 顺序位置访问"
};

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL InitMainWnd(HWND hwnd);
void UpdateStatusBar(void);
void RefreshFileList(void);
void UpdateTitle(void);
int OpenXpkFile(const wchar_t* path);
int CloseXpkFile(void);
int AddFiles(void);
int AddDirectory(void);
int ExtractFiles(void);
int ExtractAll(void);
int DeleteFiles(void);
int RenameFiles(void);
int CreateNewPackage(void);
int RebuildPackage(void);
int VerifyPackage(void);
int TestPackage(void);
int GetPackageProperties(void);
int SetDiscCode(void);
int ToggleSolidMode(void);
int ToggleVolumeMode(void);
int SetVolumeSize(void);
int SelectByPattern(void);
int NewPackageDialog(HWND hwnd, wchar_t* path, int* solidMode, int* pkgType);
int CompressLevelDialog(HWND hwnd, int* level);
int DiscCodeInputDialog(HWND hwnd, uint32_t* code);
int PatternSelectDialog(HWND hwnd, wchar_t* pattern, int* operation);
void ShowAboutDialog(void);
int BrowseForFolder(HWND hwnd, wchar_t* path, const wchar_t* title);
int BrowseForFiles(HWND hwnd, wchar_t* files, int* fileCount, const wchar_t* filter);
int BrowseForDirectory(HWND hwnd, wchar_t* path, const wchar_t* title);
int InputBox(HWND hwnd, const wchar_t* title, const wchar_t* prompt, wchar_t* buffer, int bufferSize);
void FormatSize(uint64_t size, wchar_t* buf, int bufSize);
void FormatTime(time_t t, wchar_t* buf, int bufSize);
void ErrorMsg(HWND hwnd, const wchar_t* msg);
void InfoMsg(HWND hwnd, const wchar_t* msg);
const wchar_t* GetFileTypeString(int type);

void ErrorHandler(int code, const char* message);
void ShowDetailedError(HWND hwnd, const wchar_t* context);
void LoadSettings(void);
void SaveSettings(void);
void LoadHistory(void);
void SaveHistory(const wchar_t* path);
void AddToHistory(const wchar_t* path);
int ProcessCommandLine(LPWSTR lpCmdLine);
int ExtractModeMain(const wchar_t* archivePath);
int ExtractHereModeMain(const wchar_t* archivePath);
int VerifyModeMain(const wchar_t* archivePath);
int PropertiesModeMain(const wchar_t* archivePath);

static int g_patternMatchCount = 0;
static char* g_patternMatchFiles[1000];

static int PatternMatchCallback(void* userData, uint32_t pos, void* info, void* data)
{
	if (g_patternMatchCount < 1000) {
		const char* path = xpkPathGet(g_xpk, pos);
		if (path) {
			g_patternMatchFiles[g_patternMatchCount] = _strdup(path);
			g_patternMatchCount++;
		}
	}
	return 0;
}

static wchar_t* Utf8ToWchar(const char* utf8Str) {
	if (!utf8Str) return NULL;
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, NULL, 0);
	if (len == 0) return NULL;
	wchar_t* wstr = (wchar_t*)malloc(len * sizeof(wchar_t));
	if (!wstr) return NULL;
	MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, wstr, len);
	return wstr;
}

static char* WcharToUtf8(const wchar_t* wstr) {
	if (!wstr) return NULL;
	int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	if (len == 0) return NULL;
	char* utf8Str = (char*)malloc(len);
	if (!utf8Str) return NULL;
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, utf8Str, len, NULL, NULL);
	return utf8Str;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	LPWSTR lpCmdLineW = GetCommandLineW();

	INITCOMMONCONTROLSEX icc;
	MSG msg;
	WNDCLASSEXW wc;
	BOOL bRet;

	g_hInstance = hInstance;

	LoadSettings();
	LoadHistory();

	int cmdResult = ProcessCommandLine(lpCmdLineW);
	if (cmdResult >= 0) {
		return cmdResult;
	}

	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES;
	if (!InitCommonControlsEx(&icc)) {
		MessageBoxW(NULL, L"InitCommonControlsEx failed", L"Error", MB_OK);
		return 0;
	}

	memset(&wc, 0, sizeof(WNDCLASSEXW));
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"xpkguiClass";
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassExW(&wc)) {
		MessageBoxW(NULL, L"RegisterClassExW failed", L"Error", MB_OK);
		return 0;
	}

	HMENU hMenu = LoadMenuW(hInstance, MAKEINTRESOURCEW(IDR_MAINMENU));
	if (!hMenu) {
		MessageBoxW(NULL, L"LoadMenuW failed", L"Error", MB_OK);
		return 0;
	}

	g_hMainWnd = CreateWindowExW(
		0,
		L"xpkguiClass",
		L"xpkgui - xPack 管理工具",
		WS_OVERLAPPEDWINDOW | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT,
		g_settings.windowWidth, g_settings.windowHeight,
		NULL,
		hMenu,
		hInstance,
		NULL
	);

	if (!g_hMainWnd) {
		MessageBoxW(NULL, L"CreateWindowExW failed", L"Error", MB_OK);
		return 0;
	}

	if (g_settings.windowMaximized) {
		ShowWindow(g_hMainWnd, SW_MAXIMIZE);
	} else {
		ShowWindow(g_hMainWnd, nCmdShow);
	}
	UpdateWindow(g_hMainWnd);

	GetCurrentDirectoryW(MAX_PATH_W, g_currentDir);
	xpkOnError(NULL, ErrorHandler);
	DragAcceptFiles(g_hMainWnd, TRUE);

	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if (bRet == -1) {
			break;
		} else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	SaveSettings();

	if (g_xpk != NULL) {
		xpkClose(g_xpk);
	}

	return (int)msg.wParam;
}

int ProcessCommandLine(LPWSTR lpCmdLine)
{
	int argc;
	LPWSTR* argv = CommandLineToArgvW(lpCmdLine, &argc);
	
	if (argc < 2) {
		LocalFree(argv);
		return -1;
	}

	wchar_t cmd[64];
	wchar_t path[MAX_PATH_W] = {0};

	wcscpy_s(cmd, 64, argv[1]);
	if (argc >= 3) {
		wcscpy_s(path, MAX_PATH_W, argv[2]);
	}

	if (wcscmp(cmd, L"-extract") == 0) {
		LocalFree(argv);
		return ExtractModeMain(path);
	} else if (wcscmp(cmd, L"-extract_here") == 0) {
		LocalFree(argv);
		return ExtractHereModeMain(path);
	} else if (wcscmp(cmd, L"-add") == 0) {
		LocalFree(argv);
		g_commandMode = CMD_ADD;
		wcscpy_s(g_commandPath, MAX_PATH_W, path);
		return -1;
	} else if (wcscmp(cmd, L"-add_auto") == 0) {
		LocalFree(argv);
		g_commandMode = CMD_ADD_AUTO;
		wcscpy_s(g_commandPath, MAX_PATH_W, path);
		return -1;
	} else if (wcscmp(cmd, L"-verify") == 0) {
		LocalFree(argv);
		return VerifyModeMain(path);
	} else if (wcscmp(cmd, L"-properties") == 0) {
		LocalFree(argv);
		return PropertiesModeMain(path);
	} else {
		LocalFree(argv);
		wcscpy_s(g_commandPath, MAX_PATH_W, cmd);
		return -1;
	}

	LocalFree(argv);
	return -1;
}

int ExtractModeMain(const wchar_t* archivePath)
{
	char* utf8Path = WcharToUtf8(archivePath);
	if (!utf8Path) {
		fwprintf(stderr, L"Error: Cannot convert path\n");
		return 1;
	}

	xpkObject xpk = xpkOpen(utf8Path, 0, 0);
	free(utf8Path);
	
	if (!xpk) {
		fwprintf(stderr, L"Error: Cannot open archive '%s'\n", archivePath);
		return 1;
	}

	wchar_t outDir[MAX_PATH_W] = {0};
	BROWSEINFOW bi = {0};
	LPITEMIDLIST pidl;

	if (!hShell32) {
		hShell32 = LoadLibraryW(L"shell32.dll");
	}

	if (hShell32 != NULL) {
		if (!pSHBrowseForFolderW) {
			pSHBrowseForFolderW = (PFNSHBROWSEFORFOLDERW)GetProcAddress(hShell32, "SHBrowseForFolderW");
		}
		if (!pSHGetPathFromIDListW) {
			pSHGetPathFromIDListW = (PFNSHGETPATHFROMIDLISTW)GetProcAddress(hShell32, "SHGetPathFromIDListW");
		}
		if (!pCoTaskMemFree) {
			pCoTaskMemFree = (PFNCoTaskMemFree)GetProcAddress(hShell32, "CoTaskMemFree");
		}
	}

	bi.hwndOwner = NULL;
	bi.pszDisplayName = outDir;
	bi.lpszTitle = L"选择解压目录";
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	pidl = pSHBrowseForFolderW(&bi);

	if (pidl != NULL) {
		if (pSHGetPathFromIDListW(pidl, outDir)) {
			pCoTaskMemFree(pidl);
			char* utf8OutDir = WcharToUtf8(outDir);
			if (utf8OutDir) {
				if (xpkExtractAll(xpk, utf8OutDir) == 0) {
					wprintf(L"Extracted to: %s\n", outDir);
					free(utf8OutDir);
					xpkClose(xpk);
					return 0;
				}
				free(utf8OutDir);
			}
		}
		pCoTaskMemFree(pidl);
	}

	xpkClose(xpk);
	return 1;
}

int ExtractHereModeMain(const wchar_t* archivePath)
{
	char* utf8Path = WcharToUtf8(archivePath);
	if (!utf8Path) {
		fwprintf(stderr, L"Error: Cannot convert path\n");
		return 1;
	}

	wchar_t outDir[MAX_PATH_W];
	wcscpy_s(outDir, MAX_PATH_W, archivePath);
	wchar_t* lastSlash = wcsrchr(outDir, L'\\');
	if (lastSlash) {
		*lastSlash = L'\0';
	} else {
		wcscpy_s(outDir, MAX_PATH_W, L".");
	}

	xpkObject xpk = xpkOpen(utf8Path, 0, 0);
	free(utf8Path);
	
	if (!xpk) {
		fwprintf(stderr, L"Error: Cannot open archive '%s'\n", archivePath);
		return 1;
	}

	char* utf8OutDir = WcharToUtf8(outDir);
	if (utf8OutDir) {
		if (xpkExtractAll(xpk, utf8OutDir) == 0) {
			wprintf(L"Extracted to: %s\n", outDir);
			free(utf8OutDir);
			xpkClose(xpk);
			return 0;
		}
		free(utf8OutDir);
	}

	xpkClose(xpk);
	return 1;
}

int VerifyModeMain(const wchar_t* archivePath)
{
	char* utf8Path = WcharToUtf8(archivePath);
	if (!utf8Path) {
		fwprintf(stderr, L"Error: Cannot convert path\n");
		return 1;
	}

	xpkObject xpk = xpkOpen(utf8Path, 0, 0);
	free(utf8Path);
	
	if (!xpk) {
		fwprintf(stderr, L"Error: Cannot open archive '%s'\n", archivePath);
		return 1;
	}

	int result = xpkVerifyAll(xpk);

	if (result == 0) {
		wprintf(L"All files verified successfully\n");
	} else if (result > 0) {
		wprintf(L"Verification failed: %d file(s) have errors\n", result);
	} else {
		wprintf(L"Verification error occurred\n");
	}

	xpkClose(xpk);
	return result != 0;
}

int PropertiesModeMain(const wchar_t* archivePath)
{
	char* utf8Path = WcharToUtf8(archivePath);
	if (!utf8Path) {
		fwprintf(stderr, L"Error: Cannot convert path\n");
		return 1;
	}

	xpkObject xpk = xpkOpen(utf8Path, 0, 1);
	free(utf8Path);
	
	if (!xpk) {
		fwprintf(stderr, L"Error: Cannot open archive '%s'\n", archivePath);
		return 1;
	}

	xpkHead* head = xpkGetHead(xpk);
	if (!head) {
		fwprintf(stderr, L"Error: Failed to get archive header\n");
		xpkClose(xpk);
		return 1;
	}

	xpkStat stat;
	xpkStatGet(xpk, &stat);

	wchar_t sizeBuf[64];
	wchar_t createTime[64], modifyTime[64];
	FormatSize(stat.totalSize, sizeBuf, sizeof(sizeBuf) / sizeof(wchar_t));
	FormatTime(head->createTime, createTime, sizeof(createTime) / sizeof(wchar_t));
	FormatTime(head->modifyTime, modifyTime, sizeof(modifyTime) / sizeof(wchar_t));

	const wchar_t* typeStr = L"Unknown";
	switch (xpkType(xpk)) {
		case XPK_TYPE_CORE: typeStr = L"Core"; break;
		case XPK_TYPE_INDEX: typeStr = L"Index"; break;
		case XPK_TYPE_LINUX: typeStr = L"Linux"; break;
		case XPK_TYPE_WIN32: typeStr = L"Win32"; break;
	}

	int solidMode = xpkSolidMode(xpk);

	wprintf(L"\nArchive Information:\n");
	wprintf(L"  Path:       %s\n", archivePath);
	wprintf(L"  Type:       %s\n", typeStr);
	wprintf(L"  Files:      %u\n", head->fileCount);
	wprintf(L"  Size:       %s\n", sizeBuf);
	wprintf(L"  Packed:     %llu bytes\n", stat.packedSize);
	wprintf(L"  Ratio:      %.1f%%\n", stat.ratio * 100);
	wprintf(L"  Mode:       %s\n", solidMode ? L"Solid" : L"Separate");
	wprintf(L"  Disc Code:  0x%08X\n", head->discCode);
	wprintf(L"  Created:    %s\n", createTime);
	wprintf(L"  Modified:   %s\n", modifyTime);
	wprintf(L"\n");

	xpkClose(xpk);
	return 0;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh;

	switch (message) {
		case WM_CREATE:
			if (!InitMainWnd(hwnd)) {
				return -1;
			}
			if (g_commandMode == CMD_ADD && wcslen(g_commandPath) > 0) {
				AddFiles();
			} else if (g_commandMode == CMD_ADD_AUTO && wcslen(g_commandPath) > 0) {
				AddFiles();
			} else if (wcslen(g_commandPath) > 0) {
				OpenXpkFile(g_commandPath);
			}
			break;

		case WM_SIZE:
			{
				RECT rcClient, rcStatus;

				GetClientRect(hwnd, &rcClient);
				GetWindowRect(g_hStatusBar, &rcStatus);
				int statusHeight = rcStatus.bottom - rcStatus.top;

				MoveWindow(g_hFileList, 0, 0, rcClient.right, rcClient.bottom - statusHeight, TRUE);
				MoveWindow(g_hStatusBar, 0, rcClient.bottom - statusHeight, rcClient.right, statusHeight, TRUE);

				if (wParam != SIZE_MINIMIZED) {
					g_settings.windowWidth = rcClient.right;
					g_settings.windowHeight = rcClient.bottom;
				}
			}
			break;

		case WM_SYSCOMMAND:
			if (wParam == SC_MAXIMIZE) {
				g_settings.windowMaximized = 1;
			} else if (wParam == SC_RESTORE) {
				g_settings.windowMaximized = 0;
			}
			break;

		case WM_DROPFILES:
			{
				HDROP hDrop = (HDROP)wParam;
				UINT fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);

				if (g_xpk == NULL) {
					DragFinish(hDrop);
					ErrorMsg(hwnd, L"请先打开或创建一个压缩包");
					break;
				}

				int type = xpkType(g_xpk);
				if (type != XPK_TYPE_WIN32 && type != XPK_TYPE_LINUX) {
					DragFinish(hDrop);
					ErrorMsg(hwnd, L"当前模式不支持文件路径操作");
					break;
				}

				int success = 0;
				for (UINT i = 0; i < fileCount; i++) {
					wchar_t filePath[MAX_PATH_W];
					DragQueryFileW(hDrop, i, filePath, MAX_PATH_W);

					wchar_t* slash = wcsrchr(filePath, L'\\');
					wchar_t* slash2 = wcsrchr(filePath, L'/');
					wchar_t* name = (slash2 > slash) ? slash2 : slash;
					name = name ? name + 1 : filePath;

					char* utf8FilePath = WcharToUtf8(filePath);
					char* utf8Name = WcharToUtf8(name);
					if (utf8FilePath && utf8Name) {
						if (xpkPathAppendFile(g_xpk, utf8Name, utf8FilePath, g_settings.defaultCompLevel) != NULL) {
							success++;
						}
					}
					if (utf8FilePath) free(utf8FilePath);
					if (utf8Name) free(utf8Name);
				}

				DragFinish(hDrop);
				if (success > 0) {
					RefreshFileList();
					InfoMsg(hwnd, L"添加成功");
				} else {
					ErrorMsg(hwnd, L"添加失败");
				}
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case ID_FILE_NEW:
					CreateNewPackage();
					break;

				case ID_FILE_OPEN:
					{
						wchar_t path[MAX_PATH_W] = {0};
						OPENFILENAMEW ofn = {0};
						ofn.lStructSize = sizeof(OPENFILENAMEW);
						ofn.hwndOwner = hwnd;
						ofn.lpstrFilter = L"xPack 文件 (*.xpk)\0*.xpk\0所有文件 (*.*)\0*.*\0";
						ofn.lpstrFile = path;
						ofn.nMaxFile = MAX_PATH_W;
						ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
						ofn.lpstrDefExt = L"xpk";

						if (GetOpenFileNameW(&ofn)) {
							OpenXpkFile(path);
						}
					}
					break;

				case ID_FILE_CLOSE:
					CloseXpkFile();
					break;

				case ID_FILE_SAVE:
					if (g_xpk != NULL) {
						if (xpkSave(g_xpk) == 0) {
							InfoMsg(hwnd, L"保存成功");
						} else {
							ShowDetailedError(hwnd, L"保存失败");
						}
					}
					break;

				case ID_FILE_EXIT:
					PostMessage(hwnd, WM_CLOSE, 0, 0);
					break;

				case ID_FILE_ADD:
					AddFiles();
					break;

				case ID_FILE_EXTRACT:
					ExtractFiles();
					break;

				case ID_FILE_DELETE:
					DeleteFiles();
					break;

				case ID_FILE_RENAME:
					RenameFiles();
					break;

				case ID_FILE_REBUILD:
					RebuildPackage();
					break;

				case ID_FILE_PROPERTIES:
					GetPackageProperties();
					break;

				case ID_VIEW_REFRESH:
					RefreshFileList();
					break;

				case ID_TOOLS_VERIFY:
					VerifyPackage();
					break;

				case ID_TOOLS_TEST:
					TestPackage();
					break;

				case ID_TOOLS_EXTRACTALL:
					ExtractAll();
					break;

				case ID_TOOLS_SOLIDMODE:
					ToggleSolidMode();
					break;

				case ID_TOOLS_VOLUME_MODE:
					ToggleVolumeMode();
					break;

				case ID_TOOLS_VOLUME_SIZE:
					SetVolumeSize();
					break;

				case ID_TOOLS_DISCCODE:
					SetDiscCode();
					break;

				case ID_TOOLS_PATTERN:
					SelectByPattern();
					break;

				case ID_HELP_ABOUT:
					ShowAboutDialog();
					break;
			}
			break;

		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			if (pnmh->hwndFrom == g_hFileList) {
				if (pnmh->code == NM_DBLCLK) {
					LPNMITEMACTIVATE pnmia = (LPNMITEMACTIVATE)lParam;
					if (pnmia->iItem >= 0) {
						if (g_xpk != NULL) {
							int type = xpkType(g_xpk);
							if (type == XPK_TYPE_WIN32 || type == XPK_TYPE_LINUX) {
								wchar_t filePath[MAX_PATH_W];
								LVITEMW lvi = {0};
								lvi.mask = LVIF_TEXT;
								lvi.iItem = pnmia->iItem;
								lvi.iSubItem = 0;
								lvi.pszText = filePath;
								lvi.cchTextMax = MAX_PATH_W;

								if (ListView_GetItem(g_hFileList, &lvi)) {
									wchar_t savePath[MAX_PATH_W];
									if (BrowseForFolder(hwnd, savePath, L"选择解压目录")) {
										wcscat_s(savePath, MAX_PATH_W, L"\\");
										wcscat_s(savePath, MAX_PATH_W, filePath);

										char* utf8FilePath = WcharToUtf8(filePath);
										char* utf8SavePath = WcharToUtf8(savePath);
										if (utf8FilePath && utf8SavePath) {
											if (xpkPathExtractFile(g_xpk, utf8FilePath, utf8SavePath) == 0) {
												InfoMsg(hwnd, L"解压成功");
											} else {
												ShowDetailedError(hwnd, L"解压失败");
											}
										}
										if (utf8FilePath) free(utf8FilePath);
										if (utf8SavePath) free(utf8SavePath);
									}
								}
							}
						}
					}
				}
			}
			break;

		case WM_CLOSE:
			if (g_xpk != NULL) {
				xpkClose(g_xpk);
				g_xpk = NULL;
			}
			DestroyWindow(hwnd);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return 0;
}

BOOL InitMainWnd(HWND hwnd)
{
	RECT rc;

	GetClientRect(hwnd, &rc);

	g_hFileList = CreateWindowExW(
		WS_EX_CLIENTEDGE,
		WC_LISTVIEWW,
		NULL,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
		LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
		0, 0,
		rc.right, rc.bottom - 24,
		hwnd,
		(HMENU)IDC_FILELIST,
		g_hInstance,
		NULL
	);

	if (!g_hFileList) {
		return FALSE;
	}

	DWORD exStyle = LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	if (g_settings.showGridLines) {
		exStyle |= LVS_EX_GRIDLINES;
	}
	ListView_SetExtendedListViewStyle(g_hFileList, exStyle);

	LVCOLUMNW lvc;
	lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

	lvc.pszText = L"文件名";
	lvc.cx = 300;
	lvc.iSubItem = 0;
	ListView_InsertColumn(g_hFileList, 0, &lvc);

	lvc.pszText = L"大小";
	lvc.cx = 100;
	lvc.iSubItem = 1;
	ListView_InsertColumn(g_hFileList, 1, &lvc);

	lvc.pszText = L"压缩后";
	lvc.cx = 100;
	lvc.iSubItem = 2;
	ListView_InsertColumn(g_hFileList, 2, &lvc);

	lvc.pszText = L"压缩比";
	lvc.cx = 80;
	lvc.iSubItem = 3;
	ListView_InsertColumn(g_hFileList, 3, &lvc);

	lvc.pszText = L"算法";
	lvc.cx = 60;
	lvc.iSubItem = 4;
	ListView_InsertColumn(g_hFileList, 4, &lvc);

	lvc.pszText = L"类型";
	lvc.cx = 80;
	lvc.iSubItem = 5;
	ListView_InsertColumn(g_hFileList, 5, &lvc);

	lvc.pszText = L"哈希";
	lvc.cx = 100;
	lvc.iSubItem = 6;
	ListView_InsertColumn(g_hFileList, 6, &lvc);

	g_hStatusBar = CreateWindowExW(
		0,
		STATUSCLASSNAMEW,
		NULL,
		WS_CHILD | WS_VISIBLE,
		0, rc.bottom - 24,
		rc.right, 24,
		hwnd,
		(HMENU)IDC_STATUSBAR,
		g_hInstance,
		NULL
	);

	if (!g_settings.showStatusBar) {
		ShowWindow(g_hStatusBar, SW_HIDE);
	}

	UpdateStatusBar();

	return TRUE;
}

void UpdateStatusBar(void)
{
	wchar_t buf[512] = {0};

	if (g_xpk != NULL) {
		xpkStat stat;
		if (xpkStatGet(g_xpk, &stat) == 0) {
			wchar_t sizeBuf[64];
			FormatSize(stat.totalSize, sizeBuf, sizeof(sizeBuf) / sizeof(wchar_t));
			int solidMode = xpkSolidMode(g_xpk);
			int volumeMode = xpkVolumeMode(g_xpk);
			uint32_t volumeSize = xpkVolumeSize(g_xpk);
			wchar_t volumeInfo[128] = {0};

			if (volumeMode) {
				wchar_t vsizeBuf[64];
				FormatSize(volumeSize, vsizeBuf, sizeof(vsizeBuf) / sizeof(wchar_t));
				swprintf_s(volumeInfo, sizeof(volumeInfo) / sizeof(wchar_t), L" | 分卷: %s", vsizeBuf);
			}

			swprintf_s(buf, sizeof(buf) / sizeof(wchar_t), L"文件: %u | 总大小: %s | 压缩率: %.1f%% | %s%s",
				stat.fileCount, sizeBuf, stat.ratio * 100, solidMode ? L"固实" : L"独立", volumeInfo);
		}
	} else {
		wcscpy_s(buf, sizeof(buf) / sizeof(wchar_t), L"未打开压缩包");
	}

	SendMessage(g_hStatusBar, WM_SETTEXT, 0, (LPARAM)buf);
}

void RefreshFileList(void)
{
	if (g_xpk == NULL) {
		return;
	}

	ListView_DeleteAllItems(g_hFileList);

	int type = xpkType(g_xpk);
	uint32_t count = xpkCount(g_xpk);

	for (uint32_t i = 0; i < count; i++) {
		wchar_t name[MAX_PATH_W] = {0};
		wchar_t size[64] = {0};
		wchar_t packed[64] = {0};
		wchar_t ratio[32] = {0};
		wchar_t algo[32] = {0};
		wchar_t fileType[32] = {0};
		wchar_t hashStr[64] = {0};

		uint32_t fileSize = xpkInfoSize(g_xpk, i);
		uint32_t packedSize = xpkInfoPacked(g_xpk, i);
		int level = xpkInfoLevel(g_xpk, i);
		int ftype = xpkInfoType(g_xpk, i);

		FormatSize(fileSize, size, 64);
		FormatSize(packedSize, packed, 64);

		if (packedSize > 0) {
			swprintf_s(ratio, 32, L"%.1f%%", (double)packedSize / fileSize * 100);
		} else {
			wcscpy_s(ratio, 32, L"N/A");
		}

		if (level == 0) {
			wcscpy_s(algo, 32, L"无");
		} else if (level <= 4) {
			wcscpy_s(algo, 32, L"LZ4");
		} else if (level <= 13) {
			wcscpy_s(algo, 32, L"ZSTD");
		} else {
			wcscpy_s(algo, 32, L"LZMA2");
		}

		wcscpy_s(fileType, 32, GetFileTypeString(ftype));

		uint32_t hash = xpkInfoHash(g_xpk, i);
		swprintf_s(hashStr, 64, L"%08X", hash);

		if (type == XPK_TYPE_WIN32 || type == XPK_TYPE_LINUX) {
			const char* path = xpkPathGet(g_xpk, i);
			if (path != NULL) {
				wchar_t* wpath = Utf8ToWchar(path);
				if (wpath) {
					wcscpy_s(name, MAX_PATH_W, wpath);
					free(wpath);
				}
			}
		} else {
			swprintf_s(name, MAX_PATH_W, L"文件_%u", i);
		}

		LVITEMW lvi = {0};
		lvi.mask = LVIF_TEXT;
		lvi.iItem = (int)i;
		lvi.iSubItem = 0;
		lvi.pszText = name;

		int pos = ListView_InsertItem(g_hFileList, &lvi);

		ListView_SetItemText(g_hFileList, pos, 1, size);
		ListView_SetItemText(g_hFileList, pos, 2, packed);
		ListView_SetItemText(g_hFileList, pos, 3, ratio);
		ListView_SetItemText(g_hFileList, pos, 4, algo);
		ListView_SetItemText(g_hFileList, pos, 5, fileType);
		ListView_SetItemText(g_hFileList, pos, 6, hashStr);
	}

	UpdateStatusBar();
}

void UpdateTitle(void)
{
	wchar_t title[MAX_PATH_W + 64];

	if (g_xpk != NULL) {
		swprintf_s(title, sizeof(title) / sizeof(wchar_t), L"xpkgui - %s", g_xpkPath);
	} else {
		wcscpy_s(title, sizeof(title) / sizeof(wchar_t), L"xpkgui - xPack 管理工具");
	}

	SetWindowTextW(g_hMainWnd, title);
}

int OpenXpkFile(const wchar_t* path)
{
	if (g_xpk != NULL) {
		xpkClose(g_xpk);
		g_xpk = NULL;
	}

	char* utf8Path = WcharToUtf8(path);
	if (!utf8Path) {
		ShowDetailedError(g_hMainWnd, L"无法转换路径编码");
		return -1;
	}

	g_xpk = xpkOpen(utf8Path, 0, 0);
	free(utf8Path);

	if (g_xpk == NULL) {
		ShowDetailedError(g_hMainWnd, L"无法打开压缩包文件");
		return -1;
	}

	wcscpy_s(g_xpkPath, MAX_PATH_W, path);
	AddToHistory(path);
	RefreshFileList();
	UpdateTitle();
	return 0;
}

int CloseXpkFile(void)
{
	if (g_xpk != NULL) {
		xpkClose(g_xpk);
		g_xpk = NULL;
		g_xpkPath[0] = '\0';
		ListView_DeleteAllItems(g_hFileList);
		UpdateStatusBar();
		UpdateTitle();
		return 0;
	}
	return -1;
}

int AddFiles(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, L"请先打开或创建一个压缩包");
		return -1;
	}

	int type = xpkType(g_xpk);
	if (type != XPK_TYPE_WIN32 && type != XPK_TYPE_LINUX) {
		ErrorMsg(g_hMainWnd, L"当前模式不支持文件路径操作");
		return -1;
	}

	wchar_t files[4096] = {0};
	int fileCount = 0;

	if (!BrowseForFiles(g_hMainWnd, files, &fileCount, L"所有文件 (*.*)\0*.*\0")) {
		return -1;
	}

	int level = g_settings.defaultCompLevel;
	if (CompressLevelDialog(g_hMainWnd, &level) != IDOK) {
		return -1;
	}

	wchar_t* p = files;
	int success = 0;

	while (*p != L'\0') {
		wchar_t fileName[MAX_PATH_W];
		wcscpy_s(fileName, MAX_PATH_W, p);

		const wchar_t* slash = wcsrchr(fileName, L'\\');
		const wchar_t* slash2 = wcsrchr(fileName, L'/');
		const wchar_t* name = (slash2 > slash) ? slash2 : slash;
		name = name ? name + 1 : fileName;

		char* utf8FileName = WcharToUtf8(fileName);
		char* utf8Name = WcharToUtf8(name);
		if (utf8FileName && utf8Name) {
			if (xpkPathAppendFile(g_xpk, utf8Name, utf8FileName, level) != NULL) {
				success++;
			}
		}
		if (utf8FileName) free(utf8FileName);
		if (utf8Name) free(utf8Name);

		p += wcslen(p) + 1;
	}

	if (success > 0) {
		RefreshFileList();
		InfoMsg(g_hMainWnd, L"添加成功");
	} else {
		ErrorMsg(g_hMainWnd, L"添加失败");
	}

	return 0;
}

int AddDirectory(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, L"请先打开或创建一个压缩包");
		return -1;
	}

	int type = xpkType(g_xpk);
	if (type != XPK_TYPE_WIN32 && type != XPK_TYPE_LINUX) {
		ErrorMsg(g_hMainWnd, L"当前模式不支持文件路径操作");
		return -1;
	}

	wchar_t dirPath[MAX_PATH_W] = {0};
	if (!BrowseForDirectory(g_hMainWnd, dirPath, L"选择要添加的目录")) {
		return -1;
	}

	int level = g_settings.defaultCompLevel;
	if (CompressLevelDialog(g_hMainWnd, &level) != IDOK) {
		return -1;
	}

	wchar_t cmd[MAX_PATH_W * 4];
	swprintf_s(cmd, MAX_PATH_W * 4, L"xpkcon a \"%s\" -r -l%d \"%s\"", g_xpkPath, level, dirPath);

	STARTUPINFOW si = {sizeof(si)};
	PROCESS_INFORMATION pi;

	if (CreateProcessW(NULL, cmd, NULL, NULL, FALSE,
		CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
		WaitForSingleObject(pi.hProcess, INFINITE);
		DWORD exitCode;
		GetExitCodeProcess(pi.hProcess, &exitCode);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		if (exitCode == 0) {
			RefreshFileList();
			InfoMsg(g_hMainWnd, L"添加目录成功");
			return 0;
		}
	}

	ShowDetailedError(g_hMainWnd, L"添加目录失败");
	return -1;
}

int ExtractFiles(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, L"请先打开一个压缩包");
		return -1;
	}

	int selected = ListView_GetNextItem(g_hFileList, -1, LVNI_SELECTED);
	if (selected < 0) {
		ErrorMsg(g_hMainWnd, L"请选择要解压的文件");
		return -1;
	}

	int type = xpkType(g_xpk);
	if (type != XPK_TYPE_WIN32 && type != XPK_TYPE_LINUX) {
		ErrorMsg(g_hMainWnd, L"当前模式不支持文件路径操作");
		return -1;
	}

	wchar_t savePath[MAX_PATH_W];
	if (!BrowseForFolder(g_hMainWnd, savePath, L"选择解压目录")) {
		return -1;
	}

	wchar_t filePath[MAX_PATH_W];
	LVITEMW lvi = {0};
	lvi.mask = LVIF_TEXT;
	lvi.iItem = selected;
	lvi.iSubItem = 0;
	lvi.pszText = filePath;
	lvi.cchTextMax = MAX_PATH_W;

	if (!ListView_GetItem(g_hFileList, &lvi)) {
		return -1;
	}

	wcscat_s(savePath, MAX_PATH_W, L"\\");
	wcscat_s(savePath, MAX_PATH_W, filePath);

	char* utf8FilePath = WcharToUtf8(filePath);
	char* utf8SavePath = WcharToUtf8(savePath);
	if (utf8FilePath && utf8SavePath) {
		if (xpkPathExtractFile(g_xpk, utf8FilePath, utf8SavePath) == 0) {
			InfoMsg(g_hMainWnd, L"解压成功");
			if (utf8FilePath) free(utf8FilePath);
			if (utf8SavePath) free(utf8SavePath);
			return 0;
		}
	}
	if (utf8FilePath) free(utf8FilePath);
	if (utf8SavePath) free(utf8SavePath);
	ShowDetailedError(g_hMainWnd, L"解压失败");
	return -1;
}

int ExtractAll(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, L"请先打开一个压缩包");
		return -1;
	}

	wchar_t savePath[MAX_PATH_W];
	if (!BrowseForFolder(g_hMainWnd, savePath, L"选择解压目录")) {
		return -1;
	}

	char* utf8SavePath = WcharToUtf8(savePath);
	if (utf8SavePath) {
		if (xpkExtractAll(g_xpk, utf8SavePath) == 0) {
			InfoMsg(g_hMainWnd, L"全部解压成功");
			free(utf8SavePath);
			return 0;
		}
		free(utf8SavePath);
	}
	ShowDetailedError(g_hMainWnd, L"解压失败");
	return -1;
}

int DeleteFiles(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, L"请先打开一个压缩包");
		return -1;
	}

	int selected = ListView_GetNextItem(g_hFileList, -1, LVNI_SELECTED);
	if (selected < 0) {
		ErrorMsg(g_hMainWnd, L"请选择要删除的文件");
		return -1;
	}

	int type = xpkType(g_xpk);
	if (type != XPK_TYPE_WIN32 && type != XPK_TYPE_LINUX) {
		ErrorMsg(g_hMainWnd, L"当前模式不支持文件路径操作");
		return -1;
	}

	wchar_t filePath[MAX_PATH_W];
	LVITEMW lvi = {0};
	lvi.mask = LVIF_TEXT;
	lvi.iItem = selected;
	lvi.iSubItem = 0;
	lvi.pszText = filePath;
	lvi.cchTextMax = MAX_PATH_W;

	if (!ListView_GetItem(g_hFileList, &lvi)) {
		return -1;
	}

	char* utf8FilePath = WcharToUtf8(filePath);
	if (utf8FilePath) {
		if (g_settings.confirmDelete) {
			if (MessageBoxW(g_hMainWnd, L"确定要删除选中的文件吗?", L"确认删除",
				MB_YESNO | MB_ICONQUESTION) != IDYES) {
				free(utf8FilePath);
				return -1;
			}
		}

		if (xpkPathRemove(g_xpk, utf8FilePath) == 0) {
			RefreshFileList();
			InfoMsg(g_hMainWnd, L"删除成功");
			free(utf8FilePath);
			return 0;
		}
		free(utf8FilePath);
	}
	ShowDetailedError(g_hMainWnd, L"删除失败");
	return -1;
}

int RenameFiles(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, L"请先打开一个压缩包");
		return -1;
	}

	int selected = ListView_GetNextItem(g_hFileList, -1, LVNI_SELECTED);
	if (selected < 0) {
		ErrorMsg(g_hMainWnd, L"请选择要重命名的文件");
		return -1;
	}

	int type = xpkType(g_xpk);
	if (type != XPK_TYPE_WIN32 && type != XPK_TYPE_LINUX) {
		ErrorMsg(g_hMainWnd, L"当前模式不支持文件路径操作");
		return -1;
	}

	wchar_t oldPath[MAX_PATH_W];
	LVITEMW lvi = {0};
	lvi.mask = LVIF_TEXT;
	lvi.iItem = selected;
	lvi.iSubItem = 0;
	lvi.pszText = oldPath;
	lvi.cchTextMax = MAX_PATH_W;

	if (!ListView_GetItem(g_hFileList, &lvi)) {
		return -1;
	}

	wchar_t newPath[MAX_PATH_W] = {0};
	wcscpy_s(newPath, MAX_PATH_W, oldPath);

	if (InputBox(g_hMainWnd, L"重命名", L"输入新的文件名:", newPath, MAX_PATH_W) == IDOK) {
		int level = g_settings.defaultCompLevel;
		if (CompressLevelDialog(g_hMainWnd, &level) == IDOK) {
			char* utf8OldPath = WcharToUtf8(oldPath);
			char* utf8NewPath = WcharToUtf8(newPath);
			if (utf8OldPath && utf8NewPath) {
				if (xpkPathUpdateFile(g_xpk, utf8OldPath, utf8NewPath, level) == 0) {
					RefreshFileList();
					InfoMsg(g_hMainWnd, L"重命名成功");
					free(utf8OldPath);
					free(utf8NewPath);
					return 0;
				}
			}
			if (utf8OldPath) free(utf8OldPath);
			if (utf8NewPath) free(utf8NewPath);
		}
	}
	ShowDetailedError(g_hMainWnd, L"重命名失败");
	return -1;
}

int CreateNewPackage(void)
{
	wchar_t path[MAX_PATH_W] = {0};
	int solidMode = 0;
	int pkgType = XPK_TYPE_WIN32;

	if (NewPackageDialog(g_hMainWnd, path, &solidMode, &pkgType) != IDOK) {
		return -1;
	}

	if (g_xpk != NULL) {
		xpkClose(g_xpk);
		g_xpk = NULL;
	}

	char* utf8Path = WcharToUtf8(path);
	if (!utf8Path) {
		ShowDetailedError(g_hMainWnd, L"无法转换路径编码");
		return -1;
	}

	g_xpk = xpkOpen(utf8Path, 0, 0);
	free(utf8Path);

	if (g_xpk == NULL) {
		ShowDetailedError(g_hMainWnd, L"无法创建压缩包文件");
		return -1;
	}

	if (xpkTypeSet(g_xpk, pkgType) != 0) {
		ErrorMsg(g_hMainWnd, L"设置包类型失败");
		xpkClose(g_xpk);
		g_xpk = NULL;
		return -1;
	}

	if (solidMode && xpkSolidModeSet(g_xpk, 1) != 0) {
		ErrorMsg(g_hMainWnd, L"设置固实模式失败");
		xpkClose(g_xpk);
		g_xpk = NULL;
		return -1;
	}

	wcscpy_s(g_xpkPath, MAX_PATH_W, path);
	g_settings.defaultPkgType = pkgType;
	g_settings.solidMode = solidMode;
	RefreshFileList();
	UpdateTitle();
	AddToHistory(path);
	InfoMsg(g_hMainWnd, L"新建压缩包成功");
	return 0;
}

int RebuildPackage(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, L"请先打开一个压缩包");
		return -1;
	}

	if (MessageBoxW(g_hMainWnd, L"重建压缩包可能需要较长时间,确定要继续吗?",
		L"确认重建", MB_YESNO | MB_ICONQUESTION) != IDYES) {
		return -1;
	}

	if (xpkRebuild(g_xpk) == 0) {
		RefreshFileList();
		InfoMsg(g_hMainWnd, L"重建成功");
		return 0;
	} else {
		ShowDetailedError(g_hMainWnd, L"重建失败");
		return -1;
	}
}

int VerifyPackage(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, L"请先打开一个压缩包");
		return -1;
	}

	int result = xpkVerifyAll(g_xpk);

	if (result == 0) {
		InfoMsg(g_hMainWnd, L"所有文件验证通过");
	} else if (result > 0) {
		wchar_t msg[256];
		swprintf_s(msg, 256, L"验证失败: %d 个文件有问题", result);
		ErrorMsg(g_hMainWnd, msg);
	} else {
		ShowDetailedError(g_hMainWnd, L"验证过程出错");
	}

	return 0;
}

int TestPackage(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, L"请先打开一个压缩包");
		return -1;
	}

	uint32_t count = xpkCount(g_xpk);
	wchar_t msg[256];
	swprintf_s(msg, 256, L"测试压缩包...\n共 %u 个文件", count);

	HWND hProgress = CreateWindowExW(
		0, PROGRESS_CLASSW, L"",
		WS_CHILD | WS_VISIBLE,
		100, 100, 300, 20,
		g_hMainWnd, NULL, g_hInstance, NULL
	);

	if (hProgress) {
		SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, count));
		SendMessage(hProgress, PBM_SETPOS, 0, 0);
	}

	int failed = 0;
	for (uint32_t i = 0; i < count; i++) {
		if (xpkVerify(g_xpk, i) != 0) {
			failed++;
		}
		if (hProgress) {
			SendMessage(hProgress, PBM_SETPOS, i + 1, 0);
		}
	}

	if (hProgress) {
		DestroyWindow(hProgress);
	}

	if (failed == 0) {
		InfoMsg(g_hMainWnd, L"所有文件测试通过");
	} else if (failed > 0) {
		wchar_t msg[256];
		swprintf_s(msg, 256, L"测试完成: %d 个文件有问题", failed);
		ErrorMsg(g_hMainWnd, msg);
	} else {
		ShowDetailedError(g_hMainWnd, L"测试过程出错");
	}

	return 0;
}

int ToggleSolidMode(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, L"请先打开一个压缩包");
		return -1;
	}

	int currentMode = xpkSolidMode(g_xpk);
	int newMode = currentMode ? 0 : 1;

	if (xpkCount(g_xpk) > 0) {
		ErrorMsg(g_hMainWnd, L"只能在空压缩包中切换固实模式");
		return -1;
	}

	wchar_t msg[256];
	swprintf_s(msg, sizeof(msg) / sizeof(wchar_t), L"确定要%s固实压缩模式吗?\n\n固实模式会将所有文件作为一个整体压缩,\n可以获得更好的压缩比，但解压时需要解压整个块。",
		newMode ? L"启用" : L"禁用");

	if (MessageBoxW(g_hMainWnd, msg, L"确认切换固实模式", MB_YESNO | MB_ICONQUESTION) != IDYES) {
		return -1;
	}

	if (xpkSolidModeSet(g_xpk, newMode) == 0) {
		g_settings.solidMode = newMode;
		UpdateStatusBar();
		InfoMsg(g_hMainWnd, newMode ? L"已启用固实压缩模式" : L"已禁用固实压缩模式");
		return 0;
	} else {
		ShowDetailedError(g_hMainWnd, L"切换固实模式失败");
		return -1;
	}
}

int ToggleVolumeMode(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, L"请先打开一个压缩包");
		return -1;
	}

	int currentMode = xpkVolumeMode(g_xpk);
	int newMode = currentMode ? 0 : 1;

	if (xpkCount(g_xpk) > 0) {
		ErrorMsg(g_hMainWnd, L"只能在空压缩包中切换分卷模式");
		return -1;
	}

	wchar_t msg[256];
	swprintf_s(msg, sizeof(msg) / sizeof(wchar_t), L"确定要%s分卷模式吗?\n\n分卷模式会将压缩包分割成多个文件,\n适合大文件存储和传输。",
		newMode ? L"启用" : L"禁用");

	if (MessageBoxW(g_hMainWnd, msg, L"确认切换分卷模式", MB_YESNO | MB_ICONQUESTION) != IDYES) {
		return -1;
	}

	if (xpkVolumeModeSet(g_xpk, newMode) == 0) {
		g_settings.volumeMode = newMode;
		UpdateStatusBar();
		InfoMsg(g_hMainWnd, newMode ? L"已启用分卷模式" : L"已禁用分卷模式");
		return 0;
	} else {
		ShowDetailedError(g_hMainWnd, L"切换分卷模式失败");
		return -1;
	}
}

int SetVolumeSize(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, L"请先打开一个压缩包");
		return -1;
	}

	HWND hDlg, hEditSize, hComboUnit, hPrompt, hPrompt2, hOK, hCancel;
	MSG msg;
	BOOL bRet;
	int result = IDCANCEL;
	uint32_t currentSize = xpkVolumeSize(g_xpk);
	wchar_t sizeStr[32];
	swprintf_s(sizeStr, 32, L"%u", currentSize);

	const wchar_t* units[] = {L"字节", L"KB", L"MB", L"GB"};
	int selectedUnit = 0;
	uint32_t displaySize = currentSize;
	if (displaySize >= 1024 * 1024 * 1024) {
		displaySize /= 1024 * 1024 * 1024;
		selectedUnit = 3;
	} else if (displaySize >= 1024 * 1024) {
		displaySize /= 1024 * 1024;
		selectedUnit = 2;
	} else if (displaySize >= 1024) {
		displaySize /= 1024;
		selectedUnit = 1;
	}
	swprintf_s(sizeStr, 32, L"%u", displaySize);

	hDlg = CreateWindowExW(
		WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
		MAKEINTRESOURCEW(0x8002),
		L"设置分卷大小",
		WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT,
		300, 180,
		g_hMainWnd, NULL, g_hInstance, NULL
	);

	if (!hDlg) {
		return IDCANCEL;
	}

	hPrompt = CreateWindowExW(0, L"STATIC", L"分卷大小:",
		WS_CHILD | WS_VISIBLE, 10, 10, 280, 20, hDlg, NULL, g_hInstance, NULL);

	hEditSize = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", sizeStr,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER,
		10, 35, 150, 20, hDlg, NULL, g_hInstance, NULL);

	hComboUnit = CreateWindowExW(WS_EX_CLIENTEDGE, L"COMBOBOX", L"",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
		170, 35, 110, 200, hDlg, (HMENU)1001, g_hInstance, NULL);

	hPrompt2 = CreateWindowExW(0, L"STATIC", L"输入 0 表示不限制分卷大小",
		WS_CHILD | WS_VISIBLE, 10, 65, 280, 20, hDlg, NULL, g_hInstance, NULL);

	hOK = CreateWindowExW(0, L"BUTTON", L"确定",
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		50, 100, 80, 25, hDlg, (HMENU)IDOK, g_hInstance, NULL);

	hCancel = CreateWindowExW(0, L"BUTTON", L"取消",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		170, 100, 80, 25, hDlg, (HMENU)IDCANCEL, g_hInstance, NULL);

	for (int i = 0; i < 4; i++) {
		SendMessageW(hComboUnit, CB_ADDSTRING, 0, (LPARAM)units[i]);
	}
	SendMessageW(hComboUnit, CB_SETCURSEL, selectedUnit, 0);

	SetFocus(hEditSize);
	ShowWindow(hDlg, SW_SHOW);
	EnableWindow(g_hMainWnd, FALSE);

	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if (bRet == -1) {
			break;
		} else if (!IsDialogMessage(hDlg, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_COMMAND) {
			if (LOWORD(msg.wParam) == IDOK) {
				GetWindowTextW(hEditSize, sizeStr, 32);
				uint32_t value = _wtoi(sizeStr);
				int unit = SendMessageW(hComboUnit, CB_GETCURSEL, 0, 0);

				switch (unit) {
					case 1: value *= 1024; break;
					case 2: value *= 1024 * 1024; break;
					case 3: value *= 1024 * 1024 * 1024; break;
				}

				if (xpkVolumeSizeSet(g_xpk, value) == 0) {
					g_settings.volumeSize = value;
					wchar_t msg[128];
					swprintf_s(msg, 128, L"分卷大小已设置为: %u 字节", value);
					InfoMsg(g_hMainWnd, msg);
					result = IDOK;
				} else {
					ShowDetailedError(g_hMainWnd, L"设置分卷大小失败");
				}
				break;
			} else if (LOWORD(msg.wParam) == IDCANCEL) {
				result = IDCANCEL;
				break;
			}
		}
	}

	DestroyWindow(hDlg);
	EnableWindow(g_hMainWnd, TRUE);
	SetForegroundWindow(g_hMainWnd);

	return result;
}

int GetPackageProperties(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, L"请先打开一个压缩包");
		return -1;
	}

	xpkHead* head = xpkGetHead(g_xpk);
	if (head == NULL) {
		ErrorMsg(g_hMainWnd, L"获取包信息失败");
		return -1;
	}

	xpkStat stat;
	xpkStatGet(g_xpk, &stat);

	wchar_t sizeBuf[64], createTime[64], modifyTime[64];
	FormatSize(stat.totalSize, sizeBuf, sizeof(sizeBuf) / sizeof(wchar_t));
	FormatTime(head->createTime, createTime, sizeof(createTime) / sizeof(wchar_t));
	FormatTime(head->modifyTime, modifyTime, sizeof(modifyTime) / sizeof(wchar_t));

	wchar_t msg[1024];
	const wchar_t* typeStr = L"Unknown";
	int solidMode = xpkSolidMode(g_xpk);

	switch (xpkType(g_xpk)) {
		case XPK_TYPE_CORE:
			typeStr = L"Core";
			break;
		case XPK_TYPE_INDEX:
			typeStr = L"Index";
			break;
		case XPK_TYPE_LINUX:
			typeStr = L"Linux";
			break;
		case XPK_TYPE_WIN32:
			typeStr = L"Win32";
			break;
	}

	uint32_t solidOffset, solidSize;
	xpkSolidBlockInfo(g_xpk, &solidOffset, &solidSize);

	swprintf_s(msg, sizeof(msg) / sizeof(wchar_t),
		L"压缩包信息:\n\n"
		L"类型:       %s\n"
		L"文件数:     %u\n"
		L"总大小:     %s\n"
		L"压缩后:     %llu 字节\n"
		L"压缩率:     %.1f%%\n"
		L"压缩模式:   %s\n"
		L"识别代码:   0x%08X\n"
		L"固实偏移:   %llu\n"
		L"固实大小:   %llu\n"
		L"创建时间:   %s\n"
		L"修改时间:   %s\n",
		typeStr,
		head->fileCount,
		sizeBuf,
		stat.packedSize,
		stat.ratio * 100,
		solidMode ? L"固实模式" : L"独立模式",
		head->discCode,
		solidOffset,
		solidSize,
		createTime,
		modifyTime
	);

	MessageBoxW(g_hMainWnd, msg, L"压缩包属性", MB_OK | MB_ICONINFORMATION);

	return 0;
}

int SetDiscCode(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, L"请先打开一个压缩包");
		return -1;
	}

	uint32_t currentCode = xpkDiscCode(g_xpk);
	uint32_t newCode = currentCode;

	if (DiscCodeInputDialog(g_hMainWnd, &newCode) == IDOK) {
		if (xpkDiscCodeSet(g_xpk, newCode) == 0) {
			wchar_t msg[128];
			swprintf_s(msg, 128, L"识别代码已设置为: 0x%08X", newCode);
			InfoMsg(g_hMainWnd, msg);
			return 0;
		} else {
			ShowDetailedError(g_hMainWnd, L"设置识别代码失败");
		}
	}

	return -1;
}

int SelectByPattern(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, L"请先打开一个压缩包");
		return -1;
	}

	wchar_t pattern[MAX_PATH_W] = L"*";
	int operation = 0;

	if (PatternSelectDialog(g_hMainWnd, pattern, &operation) != IDOK) {
		return -1;
	}

	switch (operation) {
		case 0:
			{
				wchar_t savePath[MAX_PATH_W];
				if (BrowseForFolder(g_hMainWnd, savePath, L"选择解压目录")) {
					char* utf8SavePath = WcharToUtf8(savePath);
					char* utf8Pattern = WcharToUtf8(pattern);
					if (utf8SavePath && utf8Pattern) {
						int result = xpkEachMatch(g_xpk, utf8Pattern, (void*)utf8SavePath, NULL);
						if (result >= 0) {
							wchar_t msg[128];
							swprintf_s(msg, sizeof(msg) / sizeof(wchar_t), L"成功解压 %d 个文件", result);
							InfoMsg(g_hMainWnd, msg);
							free(utf8SavePath);
							free(utf8Pattern);
							return 0;
						}
						free(utf8SavePath);
						free(utf8Pattern);
					}
					ShowDetailedError(g_hMainWnd, L"解压失败");
				}
			}
			break;
		case 1:
			{
				g_patternMatchCount = 0;
				char* utf8Pattern = WcharToUtf8(pattern);
				if (utf8Pattern) {
					xpkEachMatch(g_xpk, utf8Pattern, PatternMatchCallback, NULL);
					free(utf8Pattern);
				}
				if (g_patternMatchCount > 0 && MessageBoxW(g_hMainWnd, L"确定要删除匹配的文件吗?", L"确认删除",
					MB_YESNO | MB_ICONQUESTION) == IDYES) {
					for (int i = 0; i < g_patternMatchCount; i++) {
						xpkPathRemove(g_xpk, g_patternMatchFiles[i]);
						free(g_patternMatchFiles[i]);
					}
					RefreshFileList();
					InfoMsg(g_hMainWnd, L"删除成功");
					return 0;
				}
			}
			break;
	}

	return -1;
}

void ShowAboutDialog(void)
{
	MessageBoxW(g_hMainWnd,
		L"xpkgui - xPack 管理工具\n\n"
		L"版本: 2.0\n"
		L"基于 xPack 文件压缩库\n"
		L"支持多种压缩算法和包模式\n"
		L"支持命令行操作和 Shell 集成",
		L"关于 xpkgui",
		MB_OK | MB_ICONINFORMATION
	);
}

int NewPackageDialog(HWND hwnd, wchar_t* path, int* solidMode, int* pkgType)
{
	HWND hDlg, hEditPath, hBtnBrowse, hCheckSolid;
	HWND hRadioWin32, hRadioLinux, hRadioIndex, hRadioCore;
	HWND hOK, hCancel, hPrompt, hPrompt2, hPrompt3;
	MSG msg;
	BOOL bRet;
	int result = IDCANCEL;
	wchar_t filePath[MAX_PATH_W] = {0};

	hDlg = CreateWindowExW(
		WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
		MAKEINTRESOURCEW(0x8002),
		L"新建 xPack 压缩包",
		WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT,
		420, 280,
		hwnd, NULL, g_hInstance, NULL
	);

	if (!hDlg) {
		return IDCANCEL;
	}

	hPrompt = CreateWindowExW(0, L"STATIC", L"请输入压缩包文件路径:",
		WS_CHILD | WS_VISIBLE, 10, 10, 400, 20, hDlg, NULL, g_hInstance, NULL);

	hEditPath = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", filePath,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		10, 35, 300, 20, hDlg, NULL, g_hInstance, NULL);

	hBtnBrowse = CreateWindowExW(0, L"BUTTON", L"浏览...",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		320, 35, 70, 20, hDlg, (HMENU)1001, g_hInstance, NULL);

	hPrompt2 = CreateWindowExW(0, L"STATIC", L"包类型:",
		WS_CHILD | WS_VISIBLE, 10, 70, 400, 20, hDlg, NULL, g_hInstance, NULL);

	hRadioWin32 = CreateWindowExW(0, L"BUTTON", g_pkgTypeDesc[0],
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
		10, 95, 400, 20, hDlg, (HMENU)2000, g_hInstance, NULL);

	hRadioLinux = CreateWindowExW(0, L"BUTTON", g_pkgTypeDesc[1],
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
		10, 120, 400, 20, hDlg, (HMENU)2001, g_hInstance, NULL);

	hRadioIndex = CreateWindowExW(0, L"BUTTON", g_pkgTypeDesc[2],
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
		10, 145, 400, 20, hDlg, (HMENU)2002, g_hInstance, NULL);

	hRadioCore = CreateWindowExW(0, L"BUTTON", g_pkgTypeDesc[3],
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
		10, 170, 400, 20, hDlg, (HMENU)2003, g_hInstance, NULL);

	hPrompt3 = CreateWindowExW(0, L"STATIC", L"压缩模式:",
		WS_CHILD | WS_VISIBLE, 10, 200, 400, 20, hDlg, NULL, g_hInstance, NULL);

	hCheckSolid = CreateWindowExW(0, L"BUTTON", L"启用固实压缩 (更好的压缩比，但解压整个块)",
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		10, 220, 400, 20, hDlg, (HMENU)1002, g_hInstance, NULL);

	hOK = CreateWindowExW(0, L"BUTTON", L"确定",
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		110, 250, 80, 25, hDlg, (HMENU)IDOK, g_hInstance, NULL);

	hCancel = CreateWindowExW(0, L"BUTTON", L"取消",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		230, 250, 80, 25, hDlg, (HMENU)IDCANCEL, g_hInstance, NULL);

	switch (g_settings.defaultPkgType) {
		case XPK_TYPE_WIN32: SendMessage(hRadioWin32, BM_SETCHECK, BST_CHECKED, 0); break;
		case XPK_TYPE_LINUX: SendMessage(hRadioLinux, BM_SETCHECK, BST_CHECKED, 0); break;
		case XPK_TYPE_INDEX: SendMessage(hRadioIndex, BM_SETCHECK, BST_CHECKED, 0); break;
		case XPK_TYPE_CORE: SendMessage(hRadioCore, BM_SETCHECK, BST_CHECKED, 0); break;
	}

	if (g_settings.solidMode) {
		SendMessage(hCheckSolid, BM_SETCHECK, BST_CHECKED, 0);
	}

	SetFocus(hEditPath);
	ShowWindow(hDlg, SW_SHOW);
	EnableWindow(hwnd, FALSE);

	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if (bRet == -1) {
			break;
		} else if (!IsDialogMessage(hDlg, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_COMMAND) {
			if (LOWORD(msg.wParam) == 1001) {
				OPENFILENAMEW ofn = {0};
				wchar_t savePath[MAX_PATH_W] = {0};
				ofn.lStructSize = sizeof(OPENFILENAMEW);
				ofn.hwndOwner = hDlg;
				ofn.lpstrFilter = L"xPack 文件 (*.xpk)\0*.xpk\0所有文件 (*.*)\0*.*\0";
				ofn.lpstrFile = savePath;
				ofn.nMaxFile = MAX_PATH_W;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
				ofn.lpstrDefExt = L"xpk";

				if (GetSaveFileNameW(&ofn)) {
					SetWindowTextW(hEditPath, savePath);
				}
			} else if (LOWORD(msg.wParam) == IDOK) {
				GetWindowTextW(hEditPath, path, MAX_PATH_W);
				*solidMode = SendMessage(hCheckSolid, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0;
				if (SendMessage(hRadioWin32, BM_GETCHECK, 0, 0)) *pkgType = XPK_TYPE_WIN32;
				else if (SendMessage(hRadioLinux, BM_GETCHECK, 0, 0)) *pkgType = XPK_TYPE_LINUX;
				else if (SendMessage(hRadioIndex, BM_GETCHECK, 0, 0)) *pkgType = XPK_TYPE_INDEX;
				else *pkgType = XPK_TYPE_CORE;
				result = IDOK;
				break;
			} else if (LOWORD(msg.wParam) == IDCANCEL) {
				result = IDCANCEL;
				break;
			}
		}
	}

	DestroyWindow(hDlg);
	EnableWindow(hwnd, TRUE);
	SetForegroundWindow(hwnd);

	return result;
}

int CompressLevelDialog(HWND hwnd, int* level)
{
	HWND hDlg, hComboLevel, hStaticDesc, hOK, hCancel, hPrompt;
	MSG msg;
	BOOL bRet;
	int result = IDCANCEL;
	int selectedLevel = *level;

	hDlg = CreateWindowExW(
		WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
		MAKEINTRESOURCEW(0x8002),
		L"选择压缩级别",
		WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT,
		380, 180,
		hwnd, NULL, g_hInstance, NULL
	);

	if (!hDlg) {
		return IDCANCEL;
	}

	hPrompt = CreateWindowExW(0, L"STATIC", L"压缩级别:",
		WS_CHILD | WS_VISIBLE, 10, 10, 360, 20, hDlg, NULL, g_hInstance, NULL);

	hComboLevel = CreateWindowExW(WS_EX_CLIENTEDGE, L"COMBOBOX", L"",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
		10, 35, 360, 200, hDlg, (HMENU)1001, g_hInstance, NULL);

	hStaticDesc = CreateWindowExW(0, L"STATIC", g_compLevelDesc[selectedLevel],
		WS_CHILD | WS_VISIBLE, 10, 65, 360, 40, hDlg, NULL, g_hInstance, NULL);

	hOK = CreateWindowExW(0, L"BUTTON", L"确定",
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		100, 120, 80, 25, hDlg, (HMENU)IDOK, g_hInstance, NULL);

	hCancel = CreateWindowExW(0, L"BUTTON", L"取消",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		200, 120, 80, 25, hDlg, (HMENU)IDCANCEL, g_hInstance, NULL);

	for (int i = 0; i < 16; i++) {
		wchar_t item[64];
		swprintf_s(item, sizeof(item) / sizeof(wchar_t), L"%d - %s", i, g_compLevelDesc[i]);
		SendMessage(hComboLevel, CB_ADDSTRING, 0, (LPARAM)item);
	}

	SendMessage(hComboLevel, CB_SETCURSEL, selectedLevel, 0);
	SetFocus(hComboLevel);
	ShowWindow(hDlg, SW_SHOW);
	EnableWindow(hwnd, FALSE);

	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if (bRet == -1) {
			break;
		} else if (!IsDialogMessage(hDlg, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_COMMAND) {
			if (HIWORD(msg.wParam) == CBN_SELCHANGE && LOWORD(msg.wParam) == 1001) {
				int sel = SendMessage(hComboLevel, CB_GETCURSEL, 0, 0);
				if (sel >= 0 && sel < 16) {
					SetWindowTextW(hStaticDesc, g_compLevelDesc[sel]);
				}
			} else if (LOWORD(msg.wParam) == IDOK) {
				*level = SendMessage(hComboLevel, CB_GETCURSEL, 0, 0);
				if (*level < 0) *level = g_settings.defaultCompLevel;
				result = IDOK;
				break;
			} else if (LOWORD(msg.wParam) == IDCANCEL) {
				result = IDCANCEL;
				break;
			}
		}
	}

	DestroyWindow(hDlg);
	EnableWindow(hwnd, TRUE);
	SetForegroundWindow(hwnd);

	return result;
}

int DiscCodeInputDialog(HWND hwnd, uint32_t* code)
{
	HWND hDlg, hEditCode, hPrompt, hOK, hCancel;
	MSG msg;
	BOOL bRet;
	int result = IDCANCEL;
	wchar_t codeStr[32];
	swprintf_s(codeStr, sizeof(codeStr) / sizeof(wchar_t), L"%08X", *code);

	hDlg = CreateWindowExW(
		WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
		MAKEINTRESOURCEW(0x8002),
		L"设置识别代码",
		WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT,
		300, 140,
		hwnd, NULL, g_hInstance, NULL
	);

	if (!hDlg) {
		return IDCANCEL;
	}

	hPrompt = CreateWindowExW(0, L"STATIC", L"请输入识别代码 (十六进制):",
		WS_CHILD | WS_VISIBLE, 10, 10, 280, 20, hDlg, NULL, g_hInstance, NULL);

	hEditCode = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", codeStr,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_UPPERCASE,
		10, 35, 280, 20, hDlg, NULL, g_hInstance, NULL);

	hOK = CreateWindowExW(0, L"BUTTON", L"确定",
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		50, 70, 80, 25, hDlg, (HMENU)IDOK, g_hInstance, NULL);

	hCancel = CreateWindowExW(0, L"BUTTON", L"取消",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		170, 70, 80, 25, hDlg, (HMENU)IDCANCEL, g_hInstance, NULL);

	SetFocus(hEditCode);
	ShowWindow(hDlg, SW_SHOW);
	EnableWindow(hwnd, FALSE);

	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if (bRet == -1) {
			break;
		} else if (!IsDialogMessage(hDlg, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_COMMAND) {
			if (LOWORD(msg.wParam) == IDOK) {
				GetWindowTextW(hEditCode, codeStr, sizeof(codeStr) / sizeof(wchar_t));
				swscanf(codeStr, L"%X", code);
				result = IDOK;
				break;
			} else if (LOWORD(msg.wParam) == IDCANCEL) {
				result = IDCANCEL;
				break;
			}
		}
	}

	DestroyWindow(hDlg);
	EnableWindow(hwnd, TRUE);
	SetForegroundWindow(hwnd);

	return result;
}

int PatternSelectDialog(HWND hwnd, wchar_t* pattern, int* operation)
{
	HWND hDlg, hEditPattern, hComboOp, hPrompt, hPrompt2, hOK, hCancel;
	MSG msg;
	BOOL bRet;
	int result = IDCANCEL;

	hDlg = CreateWindowExW(
		WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
		MAKEINTRESOURCEW(0x8002),
		L"模式匹配操作",
		WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT,
		350, 160,
		hwnd, NULL, g_hInstance, NULL
	);

	if (!hDlg) {
		return IDCANCEL;
	}

	hPrompt = CreateWindowExW(0, L"STATIC", L"文件模式 (通配符):",
		WS_CHILD | WS_VISIBLE, 10, 10, 330, 20, hDlg, NULL, g_hInstance, NULL);

	hEditPattern = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", pattern,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		10, 35, 330, 20, hDlg, NULL, g_hInstance, NULL);

	hPrompt2 = CreateWindowExW(0, L"STATIC", L"操作:",
		WS_CHILD | WS_VISIBLE, 10, 65, 330, 20, hDlg, NULL, g_hInstance, NULL);

	hComboOp = CreateWindowExW(WS_EX_CLIENTEDGE, L"COMBOBOX", L"",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
		10, 90, 330, 200, hDlg, (HMENU)1001, g_hInstance, NULL);

	hOK = CreateWindowExW(0, L"BUTTON", L"执行",
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		80, 120, 80, 25, hDlg, (HMENU)IDOK, g_hInstance, NULL);

	hCancel = CreateWindowExW(0, L"BUTTON", L"取消",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		190, 120, 80, 25, hDlg, (HMENU)IDCANCEL, g_hInstance, NULL);

	SendMessage(hComboOp, CB_ADDSTRING, 0, (LPARAM)L"解压到目录");
	SendMessage(hComboOp, CB_ADDSTRING, 0, (LPARAM)L"删除文件");
	SendMessage(hComboOp, CB_SETCURSEL, 0, 0);

	SetFocus(hEditPattern);
	ShowWindow(hDlg, SW_SHOW);
	EnableWindow(hwnd, FALSE);

	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if (bRet == -1) {
			break;
		} else if (!IsDialogMessage(hDlg, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_COMMAND) {
			if (LOWORD(msg.wParam) == IDOK) {
				GetWindowTextW(hEditPattern, pattern, MAX_PATH_W);
				*operation = SendMessage(hComboOp, CB_GETCURSEL, 0, 0);
				result = IDOK;
				break;
			} else if (LOWORD(msg.wParam) == IDCANCEL) {
				result = IDCANCEL;
				break;
			}
		}
	}

	DestroyWindow(hDlg);
	EnableWindow(hwnd, TRUE);
	SetForegroundWindow(hwnd);

	return result;
}

int BrowseForFolder(HWND hwnd, wchar_t* path, const wchar_t* title)
{
	BROWSEINFOW bi = {0};
	LPITEMIDLIST pidl;
	wchar_t szPath[MAX_PATH_W];

	if (!hShell32) {
		hShell32 = LoadLibraryW(L"shell32.dll");
	}

	if (hShell32 != NULL) {
		if (!pSHBrowseForFolderW) {
			pSHBrowseForFolderW = (PFNSHBROWSEFORFOLDERW)GetProcAddress(hShell32, "SHBrowseForFolderW");
		}
		if (!pSHGetPathFromIDListW) {
			pSHGetPathFromIDListW = (PFNSHGETPATHFROMIDLISTW)GetProcAddress(hShell32, "SHGetPathFromIDListW");
		}
		if (!pCoTaskMemFree) {
			pCoTaskMemFree = (PFNCoTaskMemFree)GetProcAddress(hShell32, "CoTaskMemFree");
		}
	}

	if (!pSHBrowseForFolderW || !pSHGetPathFromIDListW || !pCoTaskMemFree) {
		return 0;
	}

	bi.hwndOwner = hwnd;
	bi.pszDisplayName = szPath;
	bi.lpszTitle = title;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	pidl = pSHBrowseForFolderW(&bi);

	if (pidl != NULL) {
		if (pSHGetPathFromIDListW(pidl, path)) {
			pCoTaskMemFree(pidl);
			return 1;
		}
		pCoTaskMemFree(pidl);
	}

	return 0;
}

int BrowseForFiles(HWND hwnd, wchar_t* files, int* fileCount, const wchar_t* filter)
{
	OPENFILENAMEW ofn = {0};
	wchar_t fileBuf[4096] = {0};

	ofn.lStructSize = sizeof(OPENFILENAMEW);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = fileBuf;
	ofn.nMaxFile = sizeof(fileBuf) / sizeof(wchar_t);
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

	if (GetOpenFileNameW(&ofn)) {
		wcscpy_s(files, 4096, fileBuf);

		wchar_t* p = fileBuf;
		*fileCount = 0;

		if (*(p + wcslen(p) + 1) == L'\0') {
			*fileCount = 1;
		} else {
			p += wcslen(p) + 1;
			while (*p != L'\0') {
				(*fileCount)++;
				p += wcslen(p) + 1;
			}
		}

		return 1;
	}

	return 0;
}

int BrowseForDirectory(HWND hwnd, wchar_t* path, const wchar_t* title)
{
	return BrowseForFolder(hwnd, path, title);
}

int InputBox(HWND hwnd, const wchar_t* title, const wchar_t* prompt, wchar_t* buffer, int bufferSize)
{
	HWND hDlg, hEdit, hPrompt, hOK, hCancel;
	MSG msg;
	BOOL bRet;
	int result = IDCANCEL;

	hDlg = CreateWindowExW(
		WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
		MAKEINTRESOURCEW(0x8002),
		title,
		WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT,
		300, 140,
		hwnd, NULL, g_hInstance, NULL
	);

	if (!hDlg) {
		return IDCANCEL;
	}

	hPrompt = CreateWindowExW(0, L"STATIC", prompt,
		WS_CHILD | WS_VISIBLE,
		10, 10, 280, 20,
		hDlg, NULL, g_hInstance, NULL
	);

	hEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", buffer,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		10, 35, 280, 20,
		hDlg, NULL, g_hInstance, NULL
	);

	hOK = CreateWindowExW(0, L"BUTTON", L"确定",
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		50, 70, 80, 25,
		hDlg, (HMENU)IDOK, g_hInstance, NULL
	);

	hCancel = CreateWindowExW(0, L"BUTTON", L"取消",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		170, 70, 80, 25,
		hDlg, (HMENU)IDCANCEL, g_hInstance, NULL
	);

	SetFocus(hEdit);
	ShowWindow(hDlg, SW_SHOW);
	EnableWindow(hwnd, FALSE);

	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if (bRet == -1) {
			break;
		} else if (!IsDialogMessage(hDlg, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_COMMAND) {
			if (LOWORD(msg.wParam) == IDOK) {
				GetWindowTextW(hEdit, buffer, bufferSize);
				result = IDOK;
				break;
			} else if (LOWORD(msg.wParam) == IDCANCEL) {
				result = IDCANCEL;
				break;
			}
		}
	}

	DestroyWindow(hDlg);
	EnableWindow(hwnd, TRUE);
	SetForegroundWindow(hwnd);

	return result;
}

void FormatSize(uint64_t size, wchar_t* buf, int bufSize)
{
	if (size < 1024) {
		swprintf_s(buf, bufSize, L"%llu B", size);
	} else if (size < 1024 * 1024) {
		swprintf_s(buf, bufSize, L"%.2f KB", size / 1024.0);
	} else if (size < 1024 * 1024 * 1024) {
		swprintf_s(buf, bufSize, L"%.2f MB", size / (1024.0 * 1024.0));
	} else {
		swprintf_s(buf, bufSize, L"%.2f GB", size / (1024.0 * 1024.0 * 1024.0));
	}
}

void FormatTime(time_t t, wchar_t* buf, int bufSize)
{
	if (t == 0) {
		wcscpy_s(buf, bufSize, L"N/A");
		return;
	}
	struct tm* tm = localtime(&t);
	wchar_t timeStr[64];
	wcsftime(timeStr, bufSize, L"%Y-%m-%d %H:%M:%S", tm);
	wcscpy_s(buf, bufSize, timeStr);
}

void ErrorMsg(HWND hwnd, const wchar_t* msg)
{
	MessageBoxW(hwnd, msg, L"错误", MB_OK | MB_ICONERROR);
}

void InfoMsg(HWND hwnd, const wchar_t* msg)
{
	MessageBoxW(hwnd, msg, L"信息", MB_OK | MB_ICONINFORMATION);
}

const wchar_t* GetFileTypeString(int type)
{
	static const wchar_t* typeStrings[] = {
		L"未知",
		L"二进制",
		L"文本",
		L"图像",
		L"音频",
		L"视频",
		L"归档",
		L"未知",
		L"未知",
		L"未知",
		L"未知",
		L"未知",
		L"未知",
		L"未知",
		L"未知",
		L"目录"
	};
	if (type >= 0 && type < 16) {
		return typeStrings[type];
	}
	return L"未知";
}

void ErrorHandler(int code, const char* message)
{
}

void ShowDetailedError(HWND hwnd, const wchar_t* context)
{
	int lastError = xpkLastError();
	const char* lastErrorMsg = xpkLastErrorMsg();

	wchar_t msg[512];
	if (lastErrorMsg && strlen(lastErrorMsg) > 0) {
		wchar_t* wErrorMsg = Utf8ToWchar(lastErrorMsg);
		if (wErrorMsg) {
			swprintf_s(msg, sizeof(msg) / sizeof(wchar_t), L"%s\n\n错误代码: %d\n错误信息: %s",
				context, lastError, wErrorMsg);
			free(wErrorMsg);
		} else {
			swprintf_s(msg, sizeof(msg) / sizeof(wchar_t), L"%s\n\n错误代码: %d", context, lastError);
		}
	} else {
		swprintf_s(msg, sizeof(msg) / sizeof(wchar_t), L"%s\n\n错误代码: %d", context, lastError);
	}
	MessageBoxW(hwnd, msg, L"错误", MB_OK | MB_ICONERROR);
}

void LoadSettings(void)
{
	wchar_t path[MAX_PATH_W];
	SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, path);
	wcscat_s(path, MAX_PATH_W, L"\\xPack\\settings.ini");

	FILE* f = _wfopen(path, L"r");
	if (f) {
		wchar_t line[256];
		while (fgetws(line, sizeof(line) / sizeof(wchar_t), f)) {
			if (wcsstr(line, L"DefaultCompLevel=")) {
				swscanf(line, L"DefaultCompLevel=%d", &g_settings.defaultCompLevel);
			} else if (wcsstr(line, L"DefaultPkgType=")) {
				swscanf(line, L"DefaultPkgType=%d", &g_settings.defaultPkgType);
			} else if (wcsstr(line, L"SolidMode=")) {
				swscanf(line, L"SolidMode=%d", &g_settings.solidMode);
			} else if (wcsstr(line, L"VolumeMode=")) {
				swscanf(line, L"VolumeMode=%d", &g_settings.volumeMode);
			} else if (wcsstr(line, L"VolumeSize=")) {
				swscanf(line, L"VolumeSize=%u", &g_settings.volumeSize);
			} else if (wcsstr(line, L"ConfirmDelete=")) {
				swscanf(line, L"ConfirmDelete=%d", &g_settings.confirmDelete);
			} else if (wcsstr(line, L"OverwriteFiles=")) {
				swscanf(line, L"OverwriteFiles=%d", &g_settings.overwriteFiles);
			} else if (wcsstr(line, L"ShowStatusBar=")) {
				swscanf(line, L"ShowStatusBar=%d", &g_settings.showStatusBar);
			} else if (wcsstr(line, L"ShowGridLines=")) {
				swscanf(line, L"ShowGridLines=%d", &g_settings.showGridLines);
			} else if (wcsstr(line, L"WindowWidth=")) {
				swscanf(line, L"WindowWidth=%d", &g_settings.windowWidth);
			} else if (wcsstr(line, L"WindowHeight=")) {
				swscanf(line, L"WindowHeight=%d", &g_settings.windowHeight);
			} else if (wcsstr(line, L"WindowMaximized=")) {
				swscanf(line, L"WindowMaximized=%d", &g_settings.windowMaximized);
			}
		}
		fclose(f);
	}
}

void SaveSettings(void)
{
	wchar_t path[MAX_PATH_W];
	SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, path);
	wcscat_s(path, MAX_PATH_W, L"\\xPack");

	CreateDirectoryW(path, NULL);
	wcscat_s(path, MAX_PATH_W, L"\\settings.ini");

	FILE* f = _wfopen(path, L"w");
	if (f) {
		fwprintf(f, L"[Settings]\n");
		fwprintf(f, L"DefaultCompLevel=%d\n", g_settings.defaultCompLevel);
		fwprintf(f, L"DefaultPkgType=%d\n", g_settings.defaultPkgType);
		fwprintf(f, L"SolidMode=%d\n", g_settings.solidMode);
		fwprintf(f, L"VolumeMode=%d\n", g_settings.volumeMode);
		fwprintf(f, L"VolumeSize=%u\n", g_settings.volumeSize);
		fwprintf(f, L"ConfirmDelete=%d\n", g_settings.confirmDelete);
		fwprintf(f, L"OverwriteFiles=%d\n", g_settings.overwriteFiles);
		fwprintf(f, L"ShowStatusBar=%d\n", g_settings.showStatusBar);
		fwprintf(f, L"ShowGridLines=%d\n", g_settings.showGridLines);
		fwprintf(f, L"[Window]\n");
		fwprintf(f, L"WindowWidth=%d\n", g_settings.windowWidth);
		fwprintf(f, L"WindowHeight=%d\n", g_settings.windowHeight);
		fwprintf(f, L"WindowMaximized=%d\n", g_settings.windowMaximized);
		fclose(f);
	}
}

void LoadHistory(void)
{
	wchar_t path[MAX_PATH_W];
	SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, path);
	wcscat_s(path, MAX_PATH_W, L"\\xPack\\history.ini");

	FILE* f = _wfopen(path, L"r");
	if (f) {
		wchar_t line[MAX_PATH_W];
		while (fgetws(line, sizeof(line) / sizeof(wchar_t), f)) {
			if (wcsstr(line, L"Path") == line) {
				int index;
				swscanf(line, L"Path%d=%s", &index, g_history[index].path);
			} else if (wcsstr(line, L"Count=")) {
				swscanf(line, L"Count=%d", &g_historyCount);
			}
		}
		fclose(f);
	}
}

void SaveHistory(const wchar_t* path)
{
	if (!path || wcslen(path) == 0) {
		return;
	}

	for (int i = 0; i < g_historyCount; i++) {
		if (_wcsicmp(g_history[i].path, path) == 0) {
			memmove(&g_history[i], &g_history[i + 1],
				(g_historyCount - i - 1) * sizeof(HistoryItem));
			g_historyCount--;
			break;
		}
	}

	if (g_historyCount >= MAX_RECENT_FILES) {
		memmove(&g_history[0], &g_history[1],
			(g_historyCount - 1) * sizeof(HistoryItem));
		g_historyCount--;
	}

	wcscpy_s(g_history[g_historyCount].path, MAX_PATH_W, path);
	g_history[g_historyCount].timestamp = time(NULL);
	g_historyCount++;

	wchar_t settingsPath[MAX_PATH_W];
	SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, settingsPath);
	wcscat_s(settingsPath, MAX_PATH_W, L"\\xPack");

	CreateDirectoryW(settingsPath, NULL);
	wcscat_s(settingsPath, MAX_PATH_W, L"\\history.ini");

	FILE* f = _wfopen(settingsPath, L"w");
	if (f) {
		fwprintf(f, L"[History]\n");
		fwprintf(f, L"Count=%d\n", g_historyCount);
		for (int i = 0; i < g_historyCount; i++) {
			fwprintf(f, L"Path%d=%s\n", i, g_history[i].path);
		}
		fclose(f);
	}
}

void AddToHistory(const wchar_t* path)
{
	SaveHistory(path);
}