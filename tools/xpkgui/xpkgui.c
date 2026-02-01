#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shlobj.h>
#include <time.h>
#include "resource.h"
#include "../../lib/xrt/xrt.h"
#include "../../src/xpack.h"

#pragma comment(lib, "shell32")
#pragma comment(lib, "comctl32")
#pragma comment(lib, "shlwapi")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define MAX_RECENT_FILES 10
#define MAX_HISTORY 20

typedef LPITEMIDLIST (__stdcall *PFNSHBROWSEFORFOLDERA)(BROWSEINFOA*);
typedef BOOL (__stdcall *PFNSHGETPATHFROMIDLISTA)(LPITEMIDLIST, LPSTR);
typedef void (__stdcall *PFNCoTaskMemFree)(LPVOID);

static HMODULE hShell32 = NULL;
static PFNSHBROWSEFORFOLDERA pSHBrowseForFolderA = NULL;
static PFNSHGETPATHFROMIDLISTA pSHGetPathFromIDListA = NULL;
static PFNCoTaskMemFree pCoTaskMemFree = NULL;

typedef struct {
	char path[MAX_PATH];
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
char g_xpkPath[MAX_PATH] = {0};
char g_currentDir[MAX_PATH] = {0};
CommandMode g_commandMode = CMD_NORMAL;
char g_commandPath[MAX_PATH] = {0};

Settings g_settings = {7, XPK_TYPE_WIN32, 0, 0, 0, 1, 0, 1, 1, 900, 600, 0};
HistoryItem g_history[MAX_RECENT_FILES] = {0};
int g_historyCount = 0;

static const char* g_compLevelDesc[] = {
	"无压缩 (STORE)",
	"LZ4 快速压缩",
	"LZ4 快速压缩 (64KB)",
	"LZ4-HC 高质量压缩",
	"LZ4-HC 最高质量压缩",
	"ZSTD 极速压缩",
	"ZSTD 双倍快速压缩",
	"ZSTD 贪婪压缩 (默认)",
	"ZSTD 延迟压缩",
	"ZSTD 延迟压缩2",
	"ZSTD 二叉树延迟压缩2",
	"ZSTD 二叉树优化压缩",
	"ZSTD 二叉树极致压缩",
	"ZSTD 二叉树极致压缩2",
	"LZMA2 标准压缩",
	"LZMA2 极致压缩"
};

static const char* g_pkgTypeDesc[] = {
	"Win32 (推荐)  - 路径访问，不区分大小写",
	"Linux        - 路径访问，区分大小写",
	"Index        - 整数索引访问",
	"Core         - 顺序位置访问"
};

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL InitMainWnd(HWND hwnd);
void UpdateStatusBar(void);
void RefreshFileList(void);
void UpdateTitle(void);
int OpenXpkFile(const char* path);
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
int NewPackageDialog(HWND hwnd, char* path, int* solidMode, int* pkgType);
int CompressLevelDialog(HWND hwnd, int* level);
int DiscCodeInputDialog(HWND hwnd, uint32_t* code);
int PatternSelectDialog(HWND hwnd, char* pattern, int* operation);
void ShowAboutDialog(void);
int BrowseForFolder(HWND hwnd, char* path, const char* title);
int BrowseForFiles(HWND hwnd, char* files, int* fileCount, const char* filter);
int BrowseForDirectory(HWND hwnd, char* path, const char* title);
int InputBox(HWND hwnd, const char* title, const char* prompt, char* buffer, int bufferSize);
void FormatSize(uint64_t size, char* buf, int bufSize);
void FormatTime(time_t t, char* buf, int bufSize);
void ErrorMsg(HWND hwnd, const char* msg);
void InfoMsg(HWND hwnd, const char* msg);
const char* GetFileTypeString(int type);

void ErrorHandler(int code, const char* message);
void ShowDetailedError(HWND hwnd, const char* context);
void LoadSettings(void);
void SaveSettings(void);
void LoadHistory(void);
void SaveHistory(const char* path);
void AddToHistory(const char* path);
int ProcessCommandLine(LPSTR lpCmdLine);
int ExtractModeMain(const char* archivePath);
int ExtractHereModeMain(const char* archivePath);
int VerifyModeMain(const char* archivePath);
int PropertiesModeMain(const char* archivePath);

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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	INITCOMMONCONTROLSEX icc;
	MSG msg;
	WNDCLASSEX wc;
	BOOL bRet;

	g_hInstance = hInstance;

	LoadSettings();
	LoadHistory();

	int cmdResult = ProcessCommandLine(lpCmdLine);
	if (cmdResult >= 0) {
		return cmdResult;
	}

	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES;
	InitCommonControlsEx(&icc);

	memset(&wc, 0, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
	wc.lpszClassName = "xpkguiClass";
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc)) {
		return 0;
	}

	g_hMainWnd = CreateWindowEx(
		0,
		"xpkguiClass",
		"xpkgui - xPack 管理工具",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		g_settings.windowWidth, g_settings.windowHeight,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!g_hMainWnd) {
		return 0;
	}

	if (g_settings.windowMaximized) {
		ShowWindow(g_hMainWnd, SW_MAXIMIZE);
	} else {
		ShowWindow(g_hMainWnd, nCmdShow);
	}
	UpdateWindow(g_hMainWnd);

	GetCurrentDirectory(MAX_PATH, g_currentDir);
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

int ProcessCommandLine(LPSTR lpCmdLine)
{
	if (strlen(lpCmdLine) == 0) {
		return -1;
	}

	char cmd[64];
	char path[MAX_PATH];

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

	if (strcmp(cmd, "-extract") == 0) {
		return ExtractModeMain(path);
	} else if (strcmp(cmd, "-extract_here") == 0) {
		return ExtractHereModeMain(path);
	} else if (strcmp(cmd, "-add") == 0) {
		g_commandMode = CMD_ADD;
		strcpy(g_commandPath, path);
		return -1;
	} else if (strcmp(cmd, "-add_auto") == 0) {
		g_commandMode = CMD_ADD_AUTO;
		strcpy(g_commandPath, path);
		return -1;
	} else if (strcmp(cmd, "-verify") == 0) {
		return VerifyModeMain(path);
	} else if (strcmp(cmd, "-properties") == 0) {
		return PropertiesModeMain(path);
	} else if (strcmp(cmd, "open") == 0) {
		strcpy(g_commandPath, path);
		return -1;
	}

	return -1;
}

int ExtractModeMain(const char* archivePath)
{
	xpkObject xpk = xpkOpen(archivePath, 0, 0);
	if (!xpk) {
		fprintf(stderr, "Error: Cannot open archive '%s'\n", archivePath);
		return 1;
	}

	char outDir[MAX_PATH] = {0};
	BROWSEINFOA bi = {0};
	LPITEMIDLIST pidl;

	if (!hShell32) {
		hShell32 = LoadLibraryA("shell32.dll");
	}

	if (hShell32 != NULL) {
		if (!pSHBrowseForFolderA) {
			pSHBrowseForFolderA = (PFNSHBROWSEFORFOLDERA)GetProcAddress(hShell32, "SHBrowseForFolderA");
		}
		if (!pSHGetPathFromIDListA) {
			pSHGetPathFromIDListA = (PFNSHGETPATHFROMIDLISTA)GetProcAddress(hShell32, "SHGetPathFromIDListA");
		}
		if (!pCoTaskMemFree) {
			pCoTaskMemFree = (PFNCoTaskMemFree)GetProcAddress(hShell32, "CoTaskMemFree");
		}
	}

	bi.hwndOwner = NULL;
	bi.pszDisplayName = outDir;
	bi.lpszTitle = "选择解压目录";
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	pidl = pSHBrowseForFolderA(&bi);

	if (pidl != NULL) {
		if (pSHGetPathFromIDListA(pidl, outDir)) {
			pCoTaskMemFree(pidl);
			if (xpkExtractAll(xpk, outDir) == 0) {
				printf("Extracted to: %s\n", outDir);
				xpkClose(xpk);
				return 0;
			}
		}
		pCoTaskMemFree(pidl);
	}

	xpkClose(xpk);
	return 1;
}

int ExtractHereModeMain(const char* archivePath)
{
	char outDir[MAX_PATH];
	strcpy(outDir, archivePath);
	char* lastSlash = strrchr(outDir, '\\');
	if (lastSlash) {
		*lastSlash = '\0';
	} else {
		strcpy(outDir, ".");
	}

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

int VerifyModeMain(const char* archivePath)
{
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

int PropertiesModeMain(const char* archivePath)
{
	xpkObject xpk = xpkOpen(archivePath, 0, 1);
	if (!xpk) {
		fprintf(stderr, "Error: Cannot open archive '%s'\n", archivePath);
		return 1;
	}

	xpkHead* head = xpkGetHead(xpk);
	if (!head) {
		fprintf(stderr, "Error: Failed to get archive header\n");
		xpkClose(xpk);
		return 1;
	}

	xpkStat stat;
	xpkStatGet(xpk, &stat);

	char sizeBuf[64];
	char createTime[64], modifyTime[64];
	FormatSize(stat.totalSize, sizeBuf, sizeof(sizeBuf));
	FormatTime(head->createTime, createTime, sizeof(createTime));
	FormatTime(head->modifyTime, modifyTime, sizeof(modifyTime));

	const char* typeStr = "Unknown";
	switch (xpkType(xpk)) {
		case XPK_TYPE_CORE: typeStr = "Core"; break;
		case XPK_TYPE_INDEX: typeStr = "Index"; break;
		case XPK_TYPE_LINUX: typeStr = "Linux"; break;
		case XPK_TYPE_WIN32: typeStr = "Win32"; break;
	}

	int solidMode = xpkSolidMode(xpk);

	printf("\nArchive Information:\n");
	printf("  Path:       %s\n", archivePath);
	printf("  Type:       %s\n", typeStr);
	printf("  Files:      %u\n", head->fileCount);
	printf("  Size:       %s\n", sizeBuf);
	printf("  Packed:     %llu bytes\n", stat.packedSize);
	printf("  Ratio:      %.1f%%\n", stat.ratio * 100);
	printf("  Mode:       %s\n", solidMode ? "Solid" : "Separate");
	printf("  Disc Code:  0x%08X\n", head->discCode);
	printf("  Created:    %s\n", createTime);
	printf("  Modified:   %s\n", modifyTime);
	printf("\n");

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
			if (g_commandMode == CMD_ADD && strlen(g_commandPath) > 0) {
				AddFiles();
			} else if (g_commandMode == CMD_ADD_AUTO && strlen(g_commandPath) > 0) {
				AddFiles();
			} else if (strlen(g_commandPath) > 0) {
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
				UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

				if (g_xpk == NULL) {
					DragFinish(hDrop);
					ErrorMsg(hwnd, "请先打开或创建一个压缩包");
					break;
				}

				int type = xpkType(g_xpk);
				if (type != XPK_TYPE_WIN32 && type != XPK_TYPE_LINUX) {
					DragFinish(hDrop);
					ErrorMsg(hwnd, "当前模式不支持文件路径操作");
					break;
				}

				int success = 0;
				for (UINT i = 0; i < fileCount; i++) {
					char filePath[MAX_PATH];
					DragQueryFile(hDrop, i, filePath, MAX_PATH);

					char* slash = strrchr(filePath, '\\');
					char* slash2 = strrchr(filePath, '/');
					char* name = (slash2 > slash) ? slash2 : slash;
					name = name ? name + 1 : filePath;

					if (xpkPathAppendFile(g_xpk, name, filePath, g_settings.defaultCompLevel) != NULL) {
						success++;
					}
				}

				DragFinish(hDrop);
				if (success > 0) {
					RefreshFileList();
					InfoMsg(hwnd, "添加成功");
				} else {
					ErrorMsg(hwnd, "添加失败");
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
						char path[MAX_PATH] = {0};
						OPENFILENAME ofn = {0};
						ofn.lStructSize = sizeof(OPENFILENAME);
						ofn.hwndOwner = hwnd;
						ofn.lpstrFilter = "xPack 文件 (*.xpk)\0*.xpk\0所有文件 (*.*)\0*.*\0";
						ofn.lpstrFile = path;
						ofn.nMaxFile = MAX_PATH;
						ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
						ofn.lpstrDefExt = "xpk";

						if (GetOpenFileName(&ofn)) {
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
							InfoMsg(hwnd, "保存成功");
						} else {
							ShowDetailedError(hwnd, "保存失败");
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
								char filePath[MAX_PATH];
								LVITEM lvi = {0};
								lvi.mask = LVIF_TEXT;
								lvi.iItem = pnmia->iItem;
								lvi.iSubItem = 0;
								lvi.pszText = filePath;
								lvi.cchTextMax = MAX_PATH;

								if (ListView_GetItem(g_hFileList, &lvi)) {
									char savePath[MAX_PATH];
									if (BrowseForFolder(hwnd, savePath, "选择解压目录")) {
										strcat(savePath, "\\");
										strcat(savePath, filePath);

										if (xpkPathExtractFile(g_xpk, filePath, savePath) == 0) {
											InfoMsg(hwnd, "解压成功");
										} else {
											ShowDetailedError(hwnd, "解压失败");
										}
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

	g_hFileList = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		WC_LISTVIEW,
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

	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

	lvc.pszText = "文件名";
	lvc.cx = 300;
	lvc.iSubItem = 0;
	ListView_InsertColumn(g_hFileList, 0, &lvc);

	lvc.pszText = "大小";
	lvc.cx = 100;
	lvc.iSubItem = 1;
	ListView_InsertColumn(g_hFileList, 1, &lvc);

	lvc.pszText = "压缩后";
	lvc.cx = 100;
	lvc.iSubItem = 2;
	ListView_InsertColumn(g_hFileList, 2, &lvc);

	lvc.pszText = "压缩比";
	lvc.cx = 80;
	lvc.iSubItem = 3;
	ListView_InsertColumn(g_hFileList, 3, &lvc);

	lvc.pszText = "算法";
	lvc.cx = 60;
	lvc.iSubItem = 4;
	ListView_InsertColumn(g_hFileList, 4, &lvc);

	lvc.pszText = "类型";
	lvc.cx = 80;
	lvc.iSubItem = 5;
	ListView_InsertColumn(g_hFileList, 5, &lvc);

	lvc.pszText = "哈希";
	lvc.cx = 100;
	lvc.iSubItem = 6;
	ListView_InsertColumn(g_hFileList, 6, &lvc);

	g_hStatusBar = CreateWindowEx(
		0,
		STATUSCLASSNAME,
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
	char buf[512] = {0};

	if (g_xpk != NULL) {
		xpkStat stat;
		if (xpkStatGet(g_xpk, &stat) == 0) {
			char sizeBuf[64];
			FormatSize(stat.totalSize, sizeBuf, sizeof(sizeBuf));
			int solidMode = xpkSolidMode(g_xpk);
			int volumeMode = xpkVolumeMode(g_xpk);
			uint32_t volumeSize = xpkVolumeSize(g_xpk);
			char volumeInfo[128] = {0};

			if (volumeMode) {
				char vsizeBuf[64];
				FormatSize(volumeSize, vsizeBuf, sizeof(vsizeBuf));
				sprintf_s(volumeInfo, sizeof(volumeInfo), " | 分卷: %s", vsizeBuf);
			}

			sprintf_s(buf, sizeof(buf), "文件: %u | 总大小: %s | 压缩率: %.1f%% | %s%s",
				stat.fileCount, sizeBuf, stat.ratio * 100, solidMode ? "固实" : "独立", volumeInfo);
		}
	} else {
		strcpy(buf, "未打开压缩包");
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
		char name[MAX_PATH] = {0};
		char size[64] = {0};
		char packed[64] = {0};
		char ratio[32] = {0};
		char algo[32] = {0};
		char fileType[32] = {0};
		char hashStr[64] = {0};

		uint32_t fileSize = xpkInfoSize(g_xpk, i);
		uint32_t packedSize = xpkInfoPacked(g_xpk, i);
		int level = xpkInfoLevel(g_xpk, i);
		int ftype = xpkInfoType(g_xpk, i);

		FormatSize(fileSize, size, sizeof(size));
		FormatSize(packedSize, packed, sizeof(packed));

		if (packedSize > 0) {
			sprintf_s(ratio, sizeof(ratio), "%.1f%%", (double)packedSize / fileSize * 100);
		} else {
			strcpy(ratio, "N/A");
		}

		if (level == 0) {
			strcpy(algo, "无");
		} else if (level <= 4) {
			strcpy(algo, "LZ4");
		} else if (level <= 13) {
			strcpy(algo, "ZSTD");
		} else {
			strcpy(algo, "LZMA2");
		}

		strcpy(fileType, GetFileTypeString(ftype));

		uint32_t hash = xpkInfoHash(g_xpk, i);
		sprintf_s(hashStr, sizeof(hashStr), "%08X", hash);

		if (type == XPK_TYPE_WIN32 || type == XPK_TYPE_LINUX) {
			const char* path = xpkPathGet(g_xpk, i);
			if (path != NULL) {
				strcpy(name, path);
			}
		} else {
			sprintf_s(name, MAX_PATH, "文件_%u", i);
		}

		LVITEM lvi = {0};
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
	char title[MAX_PATH + 64];

	if (g_xpk != NULL) {
		sprintf_s(title, sizeof(title), "xpkgui - %s", g_xpkPath);
	} else {
		strcpy(title, "xpkgui - xPack 管理工具");
	}

	SetWindowText(g_hMainWnd, title);
}

int OpenXpkFile(const char* path)
{
	if (g_xpk != NULL) {
		xpkClose(g_xpk);
		g_xpk = NULL;
	}

	g_xpk = xpkOpen(path, 0, 0);

	if (g_xpk == NULL) {
		ShowDetailedError(g_hMainWnd, "无法打开压缩包文件");
		return -1;
	}

	strcpy(g_xpkPath, path);
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
		ErrorMsg(g_hMainWnd, "请先打开或创建一个压缩包");
		return -1;
	}

	int type = xpkType(g_xpk);
	if (type != XPK_TYPE_WIN32 && type != XPK_TYPE_LINUX) {
		ErrorMsg(g_hMainWnd, "当前模式不支持文件路径操作");
		return -1;
	}

	char files[4096] = {0};
	int fileCount = 0;

	if (!BrowseForFiles(g_hMainWnd, files, &fileCount, "所有文件 (*.*)\0*.*\0")) {
		return -1;
	}

	int level = g_settings.defaultCompLevel;
	if (CompressLevelDialog(g_hMainWnd, &level) != IDOK) {
		return -1;
	}

	char* p = files;
	int success = 0;

	while (*p != '\0') {
		char fileName[MAX_PATH];
		strcpy(fileName, p);

		const char* slash = strrchr(fileName, '\\');
		const char* slash2 = strrchr(fileName, '/');
		const char* name = (slash2 > slash) ? slash2 : slash;
		name = name ? name + 1 : fileName;

		if (xpkPathAppendFile(g_xpk, name, fileName, level) != NULL) {
			success++;
		}

		p += strlen(p) + 1;
	}

	if (success > 0) {
		RefreshFileList();
		InfoMsg(g_hMainWnd, "添加成功");
	} else {
		ErrorMsg(g_hMainWnd, "添加失败");
	}

	return 0;
}

int AddDirectory(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, "请先打开或创建一个压缩包");
		return -1;
	}

	int type = xpkType(g_xpk);
	if (type != XPK_TYPE_WIN32 && type != XPK_TYPE_LINUX) {
		ErrorMsg(g_hMainWnd, "当前模式不支持文件路径操作");
		return -1;
	}

	char dirPath[MAX_PATH] = {0};
	if (!BrowseForDirectory(g_hMainWnd, dirPath, "选择要添加的目录")) {
		return -1;
	}

	int level = g_settings.defaultCompLevel;
	if (CompressLevelDialog(g_hMainWnd, &level) != IDOK) {
		return -1;
	}

	char cmd[MAX_PATH * 4];
	sprintf_s(cmd, sizeof(cmd), "xpkcon a \"%s\" -r -l%d \"%s\"", g_xpkPath, level, dirPath);

	STARTUPINFO si = {sizeof(si)};
	PROCESS_INFORMATION pi;

	if (CreateProcess(NULL, cmd, NULL, NULL, FALSE,
		CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
		WaitForSingleObject(pi.hProcess, INFINITE);
		DWORD exitCode;
		GetExitCodeProcess(pi.hProcess, &exitCode);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		if (exitCode == 0) {
			RefreshFileList();
			InfoMsg(g_hMainWnd, "添加目录成功");
			return 0;
		}
	}

	ShowDetailedError(g_hMainWnd, "添加目录失败");
	return -1;
}

int ExtractFiles(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, "请先打开一个压缩包");
		return -1;
	}

	int selected = ListView_GetNextItem(g_hFileList, -1, LVNI_SELECTED);
	if (selected < 0) {
		ErrorMsg(g_hMainWnd, "请选择要解压的文件");
		return -1;
	}

	int type = xpkType(g_xpk);
	if (type != XPK_TYPE_WIN32 && type != XPK_TYPE_LINUX) {
		ErrorMsg(g_hMainWnd, "当前模式不支持文件路径操作");
		return -1;
	}

	char savePath[MAX_PATH];
	if (!BrowseForFolder(g_hMainWnd, savePath, "选择解压目录")) {
		return -1;
	}

	char filePath[MAX_PATH];
	LVITEM lvi = {0};
	lvi.mask = LVIF_TEXT;
	lvi.iItem = selected;
	lvi.iSubItem = 0;
	lvi.pszText = filePath;
	lvi.cchTextMax = MAX_PATH;

	if (!ListView_GetItem(g_hFileList, &lvi)) {
		return -1;
	}

	strcat(savePath, "\\");
	strcat(savePath, filePath);

	if (xpkPathExtractFile(g_xpk, filePath, savePath) == 0) {
		InfoMsg(g_hMainWnd, "解压成功");
		return 0;
	} else {
		ShowDetailedError(g_hMainWnd, "解压失败");
		return -1;
	}
}

int ExtractAll(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, "请先打开一个压缩包");
		return -1;
	}

	char savePath[MAX_PATH];
	if (!BrowseForFolder(g_hMainWnd, savePath, "选择解压目录")) {
		return -1;
	}

	if (xpkExtractAll(g_xpk, savePath) == 0) {
		InfoMsg(g_hMainWnd, "全部解压成功");
		return 0;
	} else {
		ShowDetailedError(g_hMainWnd, "解压失败");
		return -1;
	}
}

int DeleteFiles(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, "请先打开一个压缩包");
		return -1;
	}

	int selected = ListView_GetNextItem(g_hFileList, -1, LVNI_SELECTED);
	if (selected < 0) {
		ErrorMsg(g_hMainWnd, "请选择要删除的文件");
		return -1;
	}

	int type = xpkType(g_xpk);
	if (type != XPK_TYPE_WIN32 && type != XPK_TYPE_LINUX) {
		ErrorMsg(g_hMainWnd, "当前模式不支持文件路径操作");
		return -1;
	}

	char filePath[MAX_PATH];
	LVITEM lvi = {0};
	lvi.mask = LVIF_TEXT;
	lvi.iItem = selected;
	lvi.iSubItem = 0;
	lvi.pszText = filePath;
	lvi.cchTextMax = MAX_PATH;

	if (!ListView_GetItem(g_hFileList, &lvi)) {
		return -1;
	}

	if (g_settings.confirmDelete) {
		if (MessageBox(g_hMainWnd, "确定要删除选中的文件吗?", "确认删除",
			MB_YESNO | MB_ICONQUESTION) != IDYES) {
			return -1;
		}
	}

	if (xpkPathRemove(g_xpk, filePath) == 0) {
		RefreshFileList();
		InfoMsg(g_hMainWnd, "删除成功");
		return 0;
	} else {
		ShowDetailedError(g_hMainWnd, "删除失败");
		return -1;
	}
}

int RenameFiles(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, "请先打开一个压缩包");
		return -1;
	}

	int selected = ListView_GetNextItem(g_hFileList, -1, LVNI_SELECTED);
	if (selected < 0) {
		ErrorMsg(g_hMainWnd, "请选择要重命名的文件");
		return -1;
	}

	int type = xpkType(g_xpk);
	if (type != XPK_TYPE_WIN32 && type != XPK_TYPE_LINUX) {
		ErrorMsg(g_hMainWnd, "当前模式不支持文件路径操作");
		return -1;
	}

	char oldPath[MAX_PATH];
	LVITEM lvi = {0};
	lvi.mask = LVIF_TEXT;
	lvi.iItem = selected;
	lvi.iSubItem = 0;
	lvi.pszText = oldPath;
	lvi.cchTextMax = MAX_PATH;

	if (!ListView_GetItem(g_hFileList, &lvi)) {
		return -1;
	}

	char newPath[MAX_PATH] = {0};
	strcpy(newPath, oldPath);

	if (InputBox(g_hMainWnd, "重命名", "输入新的文件名:", newPath, MAX_PATH) == IDOK) {
		int level = g_settings.defaultCompLevel;
		if (CompressLevelDialog(g_hMainWnd, &level) == IDOK) {
			if (xpkPathUpdateFile(g_xpk, oldPath, newPath, level) == 0) {
				RefreshFileList();
				InfoMsg(g_hMainWnd, "重命名成功");
				return 0;
			}
		}
	}

	ShowDetailedError(g_hMainWnd, "重命名失败");
	return -1;
}

int CreateNewPackage(void)
{
	char path[MAX_PATH] = {0};
	int solidMode = 0;
	int pkgType = XPK_TYPE_WIN32;

	if (NewPackageDialog(g_hMainWnd, path, &solidMode, &pkgType) != IDOK) {
		return -1;
	}

	if (g_xpk != NULL) {
		xpkClose(g_xpk);
		g_xpk = NULL;
	}

	g_xpk = xpkOpen(path, 0, 0);

	if (g_xpk == NULL) {
		ShowDetailedError(g_hMainWnd, "无法创建压缩包文件");
		return -1;
	}

	if (xpkTypeSet(g_xpk, pkgType) != 0) {
		ErrorMsg(g_hMainWnd, "设置包类型失败");
		xpkClose(g_xpk);
		g_xpk = NULL;
		return -1;
	}

	if (solidMode && xpkSolidModeSet(g_xpk, 1) != 0) {
		ErrorMsg(g_hMainWnd, "设置固实模式失败");
		xpkClose(g_xpk);
		g_xpk = NULL;
		return -1;
	}

	strcpy(g_xpkPath, path);
	g_settings.defaultPkgType = pkgType;
	g_settings.solidMode = solidMode;
	RefreshFileList();
	UpdateTitle();
	AddToHistory(path);
	InfoMsg(g_hMainWnd, "新建压缩包成功");
	return 0;
}

int RebuildPackage(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, "请先打开一个压缩包");
		return -1;
	}

	if (MessageBox(g_hMainWnd, "重建压缩包可能需要较长时间,确定要继续吗?",
		"确认重建", MB_YESNO | MB_ICONQUESTION) != IDYES) {
		return -1;
	}

	if (xpkRebuild(g_xpk) == 0) {
		RefreshFileList();
		InfoMsg(g_hMainWnd, "重建成功");
		return 0;
	} else {
		ShowDetailedError(g_hMainWnd, "重建失败");
		return -1;
	}
}

int VerifyPackage(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, "请先打开一个压缩包");
		return -1;
	}

	int result = xpkVerifyAll(g_xpk);

	if (result == 0) {
		InfoMsg(g_hMainWnd, "所有文件验证通过");
	} else if (result > 0) {
		char msg[256];
		sprintf_s(msg, sizeof(msg), "验证失败: %d 个文件有问题", result);
		ErrorMsg(g_hMainWnd, msg);
	} else {
		ShowDetailedError(g_hMainWnd, "验证过程出错");
	}

	return 0;
}

int TestPackage(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, "请先打开一个压缩包");
		return -1;
	}

	uint32_t count = xpkCount(g_xpk);
	char msg[256];
	sprintf_s(msg, sizeof(msg), "测试压缩包...\n共 %u 个文件", count);

	HWND hProgress = CreateWindowEx(
		0, PROGRESS_CLASS, "",
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
		InfoMsg(g_hMainWnd, "所有文件测试通过");
	} else if (failed > 0) {
		char msg[256];
		sprintf_s(msg, sizeof(msg), "测试完成: %d 个文件有问题", failed);
		ErrorMsg(g_hMainWnd, msg);
	} else {
		ShowDetailedError(g_hMainWnd, "测试过程出错");
	}

	return 0;
}

int ToggleSolidMode(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, "请先打开一个压缩包");
		return -1;
	}

	int currentMode = xpkSolidMode(g_xpk);
	int newMode = currentMode ? 0 : 1;

	if (xpkCount(g_xpk) > 0) {
		ErrorMsg(g_hMainWnd, "只能在空压缩包中切换固实模式");
		return -1;
	}

	char msg[256];
	sprintf_s(msg, sizeof(msg), "确定要%s固实压缩模式吗?\n\n固实模式会将所有文件作为一个整体压缩,\n可以获得更好的压缩比，但解压时需要解压整个块。",
		newMode ? "启用" : "禁用");

	if (MessageBox(g_hMainWnd, msg, "确认切换固实模式", MB_YESNO | MB_ICONQUESTION) != IDYES) {
		return -1;
	}

	if (xpkSolidModeSet(g_xpk, newMode) == 0) {
		g_settings.solidMode = newMode;
		UpdateStatusBar();
		InfoMsg(g_hMainWnd, newMode ? "已启用固实压缩模式" : "已禁用固实压缩模式");
		return 0;
	} else {
		ShowDetailedError(g_hMainWnd, "切换固实模式失败");
		return -1;
	}
}

int ToggleVolumeMode(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, "请先打开一个压缩包");
		return -1;
	}

	int currentMode = xpkVolumeMode(g_xpk);
	int newMode = currentMode ? 0 : 1;

	if (xpkCount(g_xpk) > 0) {
		ErrorMsg(g_hMainWnd, "只能在空压缩包中切换分卷模式");
		return -1;
	}

	char msg[256];
	sprintf_s(msg, sizeof(msg), "确定要%s分卷模式吗?\n\n分卷模式会将压缩包分割成多个文件,\n适合大文件存储和传输。",
		newMode ? "启用" : "禁用");

	if (MessageBox(g_hMainWnd, msg, "确认切换分卷模式", MB_YESNO | MB_ICONQUESTION) != IDYES) {
		return -1;
	}

	if (xpkVolumeModeSet(g_xpk, newMode) == 0) {
		g_settings.volumeMode = newMode;
		UpdateStatusBar();
		InfoMsg(g_hMainWnd, newMode ? "已启用分卷模式" : "已禁用分卷模式");
		return 0;
	} else {
		ShowDetailedError(g_hMainWnd, "切换分卷模式失败");
		return -1;
	}
}

int SetVolumeSize(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, "请先打开一个压缩包");
		return -1;
	}

	HWND hDlg, hEditSize, hComboUnit, hPrompt, hPrompt2, hOK, hCancel;
	MSG msg;
	BOOL bRet;
	int result = IDCANCEL;
	uint32_t currentSize = xpkVolumeSize(g_xpk);
	char sizeStr[32];
	sprintf_s(sizeStr, sizeof(sizeStr), "%u", currentSize);

	const char* units[] = {"字节", "KB", "MB", "GB"};
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
	sprintf_s(sizeStr, sizeof(sizeStr), "%u", displaySize);

	hDlg = CreateWindowEx(
		WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
		WC_DIALOG,
		"设置分卷大小",
		WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT,
		300, 180,
		g_hMainWnd, NULL, g_hInstance, NULL
	);

	if (!hDlg) {
		return IDCANCEL;
	}

	hPrompt = CreateWindowEx(0, "STATIC", "分卷大小:",
		WS_CHILD | WS_VISIBLE, 10, 10, 280, 20, hDlg, NULL, g_hInstance, NULL);

	hEditSize = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", sizeStr,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER,
		10, 35, 150, 20, hDlg, NULL, g_hInstance, NULL);

	hComboUnit = CreateWindowEx(WS_EX_CLIENTEDGE, "COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
		170, 35, 110, 200, hDlg, (HMENU)1001, g_hInstance, NULL);

	hPrompt2 = CreateWindowEx(0, "STATIC", "输入 0 表示不限制分卷大小",
		WS_CHILD | WS_VISIBLE, 10, 65, 280, 20, hDlg, NULL, g_hInstance, NULL);

	hOK = CreateWindowEx(0, "BUTTON", "确定",
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		50, 100, 80, 25, hDlg, (HMENU)IDOK, g_hInstance, NULL);

	hCancel = CreateWindowEx(0, "BUTTON", "取消",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		170, 100, 80, 25, hDlg, (HMENU)IDCANCEL, g_hInstance, NULL);

	for (int i = 0; i < 4; i++) {
		SendMessage(hComboUnit, CB_ADDSTRING, 0, (LPARAM)units[i]);
	}
	SendMessage(hComboUnit, CB_SETCURSEL, selectedUnit, 0);

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
				GetWindowText(hEditSize, sizeStr, sizeof(sizeStr));
				uint32_t value = atoi(sizeStr);
				int unit = SendMessage(hComboUnit, CB_GETCURSEL, 0, 0);

				switch (unit) {
					case 1: value *= 1024; break;
					case 2: value *= 1024 * 1024; break;
					case 3: value *= 1024 * 1024 * 1024; break;
				}

				if (xpkVolumeSizeSet(g_xpk, value) == 0) {
					g_settings.volumeSize = value;
					char msg[128];
					sprintf_s(msg, sizeof(msg), "分卷大小已设置为: %u 字节", value);
					InfoMsg(g_hMainWnd, msg);
					result = IDOK;
				} else {
					ShowDetailedError(g_hMainWnd, "设置分卷大小失败");
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
		ErrorMsg(g_hMainWnd, "请先打开一个压缩包");
		return -1;
	}

	xpkHead* head = xpkGetHead(g_xpk);
	if (head == NULL) {
		ErrorMsg(g_hMainWnd, "获取包信息失败");
		return -1;
	}

	xpkStat stat;
	xpkStatGet(g_xpk, &stat);

	char sizeBuf[64], createTime[64], modifyTime[64];
	FormatSize(stat.totalSize, sizeBuf, sizeof(sizeBuf));
	FormatTime(head->createTime, createTime, sizeof(createTime));
	FormatTime(head->modifyTime, modifyTime, sizeof(modifyTime));

	char msg[1024];
	const char* typeStr = "Unknown";
	int solidMode = xpkSolidMode(g_xpk);

	switch (xpkType(g_xpk)) {
		case XPK_TYPE_CORE:
			typeStr = "Core";
			break;
		case XPK_TYPE_INDEX:
			typeStr = "Index";
			break;
		case XPK_TYPE_LINUX:
			typeStr = "Linux";
			break;
		case XPK_TYPE_WIN32:
			typeStr = "Win32";
			break;
	}

	uint32_t solidOffset, solidSize;
	xpkSolidBlockInfo(g_xpk, &solidOffset, &solidSize);

	sprintf_s(msg, sizeof(msg),
		"压缩包信息:\n\n"
		"类型:       %s\n"
		"文件数:     %u\n"
		"总大小:     %s\n"
		"压缩后:     %llu 字节\n"
		"压缩率:     %.1f%%\n"
		"压缩模式:   %s\n"
		"识别代码:   0x%08X\n"
		"固实偏移:   %llu\n"
		"固实大小:   %llu\n"
		"创建时间:   %s\n"
		"修改时间:   %s\n",
		typeStr,
		head->fileCount,
		sizeBuf,
		stat.packedSize,
		stat.ratio * 100,
		solidMode ? "固实模式" : "独立模式",
		head->discCode,
		solidOffset,
		solidSize,
		createTime,
		modifyTime
	);

	MessageBox(g_hMainWnd, msg, "压缩包属性", MB_OK | MB_ICONINFORMATION);

	return 0;
}

int SetDiscCode(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, "请先打开一个压缩包");
		return -1;
	}

	uint32_t currentCode = xpkDiscCode(g_xpk);
	uint32_t newCode = currentCode;

	if (DiscCodeInputDialog(g_hMainWnd, &newCode) == IDOK) {
		if (xpkDiscCodeSet(g_xpk, newCode) == 0) {
			char msg[128];
			sprintf_s(msg, sizeof(msg), "识别代码已设置为: 0x%08X", newCode);
			InfoMsg(g_hMainWnd, msg);
			return 0;
		} else {
			ShowDetailedError(g_hMainWnd, "设置识别代码失败");
		}
	}

	return -1;
}

int SelectByPattern(void)
{
	if (g_xpk == NULL) {
		ErrorMsg(g_hMainWnd, "请先打开一个压缩包");
		return -1;
	}

	char pattern[MAX_PATH] = "*";
	int operation = 0;

	if (PatternSelectDialog(g_hMainWnd, pattern, &operation) != IDOK) {
		return -1;
	}

	switch (operation) {
		case 0:
			{
				char savePath[MAX_PATH];
				if (BrowseForFolder(g_hMainWnd, savePath, "选择解压目录")) {
					int result = xpkEachMatch(g_xpk, pattern, (void*)savePath, NULL);
					if (result >= 0) {
						char msg[128];
						sprintf_s(msg, sizeof(msg), "成功解压 %d 个文件", result);
						InfoMsg(g_hMainWnd, msg);
						return 0;
					}
					ShowDetailedError(g_hMainWnd, "解压失败");
				}
			}
			break;
		case 1:
			{
				g_patternMatchCount = 0;
				xpkEachMatch(g_xpk, pattern, PatternMatchCallback, NULL);
				if (g_patternMatchCount > 0 && MessageBox(g_hMainWnd, "确定要删除匹配的文件吗?", "确认删除",
					MB_YESNO | MB_ICONQUESTION) == IDYES) {
					for (int i = 0; i < g_patternMatchCount; i++) {
						xpkPathRemove(g_xpk, g_patternMatchFiles[i]);
						free(g_patternMatchFiles[i]);
					}
					RefreshFileList();
					InfoMsg(g_hMainWnd, "删除成功");
					return 0;
				}
			}
			break;
	}

	return -1;
}

void ShowAboutDialog(void)
{
	MessageBox(g_hMainWnd,
		"xpkgui - xPack 管理工具\n\n"
		"版本: 2.0\n"
		"基于 xPack 文件压缩库\n"
		"支持多种压缩算法和包模式\n"
		"支持命令行操作和 Shell 集成",
		"关于 xpkgui",
		MB_OK | MB_ICONINFORMATION
	);
}

int NewPackageDialog(HWND hwnd, char* path, int* solidMode, int* pkgType)
{
	HWND hDlg, hEditPath, hBtnBrowse, hCheckSolid;
	HWND hRadioWin32, hRadioLinux, hRadioIndex, hRadioCore;
	HWND hOK, hCancel, hPrompt, hPrompt2, hPrompt3;
	MSG msg;
	BOOL bRet;
	int result = IDCANCEL;
	char filePath[MAX_PATH] = {0};

	hDlg = CreateWindowEx(
		WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
		WC_DIALOG,
		"新建 xPack 压缩包",
		WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT,
		420, 280,
		hwnd, NULL, g_hInstance, NULL
	);

	if (!hDlg) {
		return IDCANCEL;
	}

	hPrompt = CreateWindowEx(0, "STATIC", "请输入压缩包文件路径:",
		WS_CHILD | WS_VISIBLE, 10, 10, 400, 20, hDlg, NULL, g_hInstance, NULL);

	hEditPath = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", filePath,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		10, 35, 300, 20, hDlg, NULL, g_hInstance, NULL);

	hBtnBrowse = CreateWindowEx(0, "BUTTON", "浏览...",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		320, 35, 70, 20, hDlg, (HMENU)1001, g_hInstance, NULL);

	hPrompt2 = CreateWindowEx(0, "STATIC", "包类型:",
		WS_CHILD | WS_VISIBLE, 10, 70, 400, 20, hDlg, NULL, g_hInstance, NULL);

	hRadioWin32 = CreateWindowEx(0, "BUTTON", g_pkgTypeDesc[0],
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
		10, 95, 400, 20, hDlg, (HMENU)2000, g_hInstance, NULL);

	hRadioLinux = CreateWindowEx(0, "BUTTON", g_pkgTypeDesc[1],
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
		10, 120, 400, 20, hDlg, (HMENU)2001, g_hInstance, NULL);

	hRadioIndex = CreateWindowEx(0, "BUTTON", g_pkgTypeDesc[2],
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
		10, 145, 400, 20, hDlg, (HMENU)2002, g_hInstance, NULL);

	hRadioCore = CreateWindowEx(0, "BUTTON", g_pkgTypeDesc[3],
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
		10, 170, 400, 20, hDlg, (HMENU)2003, g_hInstance, NULL);

	hPrompt3 = CreateWindowEx(0, "STATIC", "压缩模式:",
		WS_CHILD | WS_VISIBLE, 10, 200, 400, 20, hDlg, NULL, g_hInstance, NULL);

	hCheckSolid = CreateWindowEx(0, "BUTTON", "启用固实压缩 (更好的压缩比，但解压整个块)",
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		10, 220, 400, 20, hDlg, (HMENU)1002, g_hInstance, NULL);

	hOK = CreateWindowEx(0, "BUTTON", "确定",
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		110, 250, 80, 25, hDlg, (HMENU)IDOK, g_hInstance, NULL);

	hCancel = CreateWindowEx(0, "BUTTON", "取消",
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
				OPENFILENAME ofn = {0};
				char savePath[MAX_PATH] = {0};
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = hDlg;
				ofn.lpstrFilter = "xPack 文件 (*.xpk)\0*.xpk\0所有文件 (*.*)\0*.*\0";
				ofn.lpstrFile = savePath;
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
				ofn.lpstrDefExt = "xpk";

				if (GetSaveFileName(&ofn)) {
					SetWindowText(hEditPath, savePath);
				}
			} else if (LOWORD(msg.wParam) == IDOK) {
				GetWindowText(hEditPath, path, MAX_PATH);
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

	hDlg = CreateWindowEx(
		WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
		WC_DIALOG,
		"选择压缩级别",
		WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT,
		380, 180,
		hwnd, NULL, g_hInstance, NULL
	);

	if (!hDlg) {
		return IDCANCEL;
	}

	hPrompt = CreateWindowEx(0, "STATIC", "压缩级别:",
		WS_CHILD | WS_VISIBLE, 10, 10, 360, 20, hDlg, NULL, g_hInstance, NULL);

	hComboLevel = CreateWindowEx(WS_EX_CLIENTEDGE, "COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
		10, 35, 360, 200, hDlg, (HMENU)1001, g_hInstance, NULL);

	hStaticDesc = CreateWindowEx(0, "STATIC", g_compLevelDesc[selectedLevel],
		WS_CHILD | WS_VISIBLE, 10, 65, 360, 40, hDlg, NULL, g_hInstance, NULL);

	hOK = CreateWindowEx(0, "BUTTON", "确定",
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		100, 120, 80, 25, hDlg, (HMENU)IDOK, g_hInstance, NULL);

	hCancel = CreateWindowEx(0, "BUTTON", "取消",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		200, 120, 80, 25, hDlg, (HMENU)IDCANCEL, g_hInstance, NULL);

	for (int i = 0; i < 16; i++) {
		char item[64];
		sprintf_s(item, sizeof(item), "%d - %s", i, g_compLevelDesc[i]);
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
					SetWindowText(hStaticDesc, g_compLevelDesc[sel]);
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
	char codeStr[32];
	sprintf_s(codeStr, sizeof(codeStr), "%08X", *code);

	hDlg = CreateWindowEx(
		WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
		WC_DIALOG,
		"设置识别代码",
		WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT,
		300, 140,
		hwnd, NULL, g_hInstance, NULL
	);

	if (!hDlg) {
		return IDCANCEL;
	}

	hPrompt = CreateWindowEx(0, "STATIC", "请输入识别代码 (十六进制):",
		WS_CHILD | WS_VISIBLE, 10, 10, 280, 20, hDlg, NULL, g_hInstance, NULL);

	hEditCode = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", codeStr,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_UPPERCASE,
		10, 35, 280, 20, hDlg, NULL, g_hInstance, NULL);

	hOK = CreateWindowEx(0, "BUTTON", "确定",
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		50, 70, 80, 25, hDlg, (HMENU)IDOK, g_hInstance, NULL);

	hCancel = CreateWindowEx(0, "BUTTON", "取消",
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
				GetWindowText(hEditCode, codeStr, sizeof(codeStr));
				sscanf(codeStr, "%X", code);
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

int PatternSelectDialog(HWND hwnd, char* pattern, int* operation)
{
	HWND hDlg, hEditPattern, hComboOp, hPrompt, hPrompt2, hOK, hCancel;
	MSG msg;
	BOOL bRet;
	int result = IDCANCEL;

	hDlg = CreateWindowEx(
		WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
		WC_DIALOG,
		"模式匹配操作",
		WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT,
		350, 160,
		hwnd, NULL, g_hInstance, NULL
	);

	if (!hDlg) {
		return IDCANCEL;
	}

	hPrompt = CreateWindowEx(0, "STATIC", "文件模式 (通配符):",
		WS_CHILD | WS_VISIBLE, 10, 10, 330, 20, hDlg, NULL, g_hInstance, NULL);

	hEditPattern = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", pattern,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		10, 35, 330, 20, hDlg, NULL, g_hInstance, NULL);

	hPrompt2 = CreateWindowEx(0, "STATIC", "操作:",
		WS_CHILD | WS_VISIBLE, 10, 65, 330, 20, hDlg, NULL, g_hInstance, NULL);

	hComboOp = CreateWindowEx(WS_EX_CLIENTEDGE, "COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
		10, 90, 330, 200, hDlg, (HMENU)1001, g_hInstance, NULL);

	hOK = CreateWindowEx(0, "BUTTON", "执行",
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		80, 120, 80, 25, hDlg, (HMENU)IDOK, g_hInstance, NULL);

	hCancel = CreateWindowEx(0, "BUTTON", "取消",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		190, 120, 80, 25, hDlg, (HMENU)IDCANCEL, g_hInstance, NULL);

	SendMessage(hComboOp, CB_ADDSTRING, 0, (LPARAM)"解压到目录");
	SendMessage(hComboOp, CB_ADDSTRING, 0, (LPARAM)"删除文件");
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
				GetWindowText(hEditPattern, pattern, MAX_PATH);
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

int BrowseForFolder(HWND hwnd, char* path, const char* title)
{
	BROWSEINFOA bi = {0};
	LPITEMIDLIST pidl;
	char szPath[MAX_PATH];

	if (!hShell32) {
		hShell32 = LoadLibraryA("shell32.dll");
	}

	if (hShell32 != NULL) {
		if (!pSHBrowseForFolderA) {
			pSHBrowseForFolderA = (PFNSHBROWSEFORFOLDERA)GetProcAddress(hShell32, "SHBrowseForFolderA");
		}
		if (!pSHGetPathFromIDListA) {
			pSHGetPathFromIDListA = (PFNSHGETPATHFROMIDLISTA)GetProcAddress(hShell32, "SHGetPathFromIDListA");
		}
		if (!pCoTaskMemFree) {
			pCoTaskMemFree = (PFNCoTaskMemFree)GetProcAddress(hShell32, "CoTaskMemFree");
		}
	}

	if (!pSHBrowseForFolderA || !pSHGetPathFromIDListA || !pCoTaskMemFree) {
		return 0;
	}

	bi.hwndOwner = hwnd;
	bi.pszDisplayName = szPath;
	bi.lpszTitle = title;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	pidl = pSHBrowseForFolderA(&bi);

	if (pidl != NULL) {
		if (pSHGetPathFromIDListA(pidl, path)) {
			pCoTaskMemFree(pidl);
			return 1;
		}
		pCoTaskMemFree(pidl);
	}

	return 0;
}

int BrowseForFiles(HWND hwnd, char* files, int* fileCount, const char* filter)
{
	OPENFILENAME ofn = {0};
	char fileBuf[4096] = {0};

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = fileBuf;
	ofn.nMaxFile = sizeof(fileBuf);
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

	if (GetOpenFileName(&ofn)) {
		strcpy(files, fileBuf);

		char* p = fileBuf;
		*fileCount = 0;

		if (*(p + strlen(p) + 1) == '\0') {
			*fileCount = 1;
		} else {
			p += strlen(p) + 1;
			while (*p != '\0') {
				(*fileCount)++;
				p += strlen(p) + 1;
			}
		}

		return 1;
	}

	return 0;
}

int BrowseForDirectory(HWND hwnd, char* path, const char* title)
{
	return BrowseForFolder(hwnd, path, title);
}

int InputBox(HWND hwnd, const char* title, const char* prompt, char* buffer, int bufferSize)
{
	HWND hDlg, hEdit, hPrompt, hOK, hCancel;
	MSG msg;
	BOOL bRet;
	int result = IDCANCEL;

	hDlg = CreateWindowEx(
		WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
		WC_DIALOG,
		title,
		WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT,
		300, 140,
		hwnd, NULL, g_hInstance, NULL
	);

	if (!hDlg) {
		return IDCANCEL;
	}

	hPrompt = CreateWindowEx(0, "STATIC", prompt,
		WS_CHILD | WS_VISIBLE,
		10, 10, 280, 20,
		hDlg, NULL, g_hInstance, NULL);

	hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", buffer,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		10, 35, 280, 20,
		hDlg, NULL, g_hInstance, NULL
	);

	hOK = CreateWindowEx(0, "BUTTON", "确定",
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		50, 70, 80, 25,
		hDlg, (HMENU)IDOK, g_hInstance, NULL
	);

	hCancel = CreateWindowEx(0, "BUTTON", "取消",
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
				GetWindowText(hEdit, buffer, bufferSize);
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

void FormatSize(uint64_t size, char* buf, int bufSize)
{
	if (size < 1024) {
		sprintf_s(buf, bufSize, "%llu B", size);
	} else if (size < 1024 * 1024) {
		sprintf_s(buf, bufSize, "%.2f KB", size / 1024.0);
	} else if (size < 1024 * 1024 * 1024) {
		sprintf_s(buf, bufSize, "%.2f MB", size / (1024.0 * 1024.0));
	} else {
		sprintf_s(buf, bufSize, "%.2f GB", size / (1024.0 * 1024.0 * 1024.0));
	}
}

void FormatTime(time_t t, char* buf, int bufSize)
{
	if (t == 0) {
		strcpy(buf, "N/A");
		return;
	}
	struct tm* tm = localtime(&t);
	strftime(buf, bufSize, "%Y-%m-%d %H:%M:%S", tm);
}

void ErrorMsg(HWND hwnd, const char* msg)
{
	MessageBox(hwnd, msg, "错误", MB_OK | MB_ICONERROR);
}

void InfoMsg(HWND hwnd, const char* msg)
{
	MessageBox(hwnd, msg, "信息", MB_OK | MB_ICONINFORMATION);
}

const char* GetFileTypeString(int type)
{
	switch (type) {
		case 0: return "未知";
		case 1: return "二进制";
		case 2: return "文本";
		case 3: return "图像";
		case 4: return "音频";
		case 5: return "视频";
		case 6: return "归档";
		case 15: return "目录";
		default: return "未知";
	}
}

void ErrorHandler(int code, const char* message)
{
}

void ShowDetailedError(HWND hwnd, const char* context)
{
	int lastError = xpkLastError();
	const char* lastErrorMsg = xpkLastErrorMsg();

	char msg[512];
	if (lastErrorMsg && strlen(lastErrorMsg) > 0) {
		sprintf_s(msg, sizeof(msg), "%s\n\n错误代码: %d\n错误信息: %s",
			context, lastError, lastErrorMsg);
	} else {
		sprintf_s(msg, sizeof(msg), "%s\n\n错误代码: %d", context, lastError);
	}
	MessageBox(hwnd, msg, "错误", MB_OK | MB_ICONERROR);
}

void LoadSettings(void)
{
	char path[MAX_PATH];
	SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path);
	strcat(path, "\\xPack\\settings.ini");

	FILE* f = fopen(path, "r");
	if (f) {
		char line[256];
		while (fgets(line, sizeof(line), f)) {
			if (strstr(line, "DefaultCompLevel=")) {
				sscanf(line, "DefaultCompLevel=%d", &g_settings.defaultCompLevel);
			} else if (strstr(line, "DefaultPkgType=")) {
				sscanf(line, "DefaultPkgType=%d", &g_settings.defaultPkgType);
			} else if (strstr(line, "SolidMode=")) {
				sscanf(line, "SolidMode=%d", &g_settings.solidMode);
			} else if (strstr(line, "VolumeMode=")) {
				sscanf(line, "VolumeMode=%d", &g_settings.volumeMode);
			} else if (strstr(line, "VolumeSize=")) {
				sscanf(line, "VolumeSize=%u", &g_settings.volumeSize);
			} else if (strstr(line, "ConfirmDelete=")) {
				sscanf(line, "ConfirmDelete=%d", &g_settings.confirmDelete);
			} else if (strstr(line, "OverwriteFiles=")) {
				sscanf(line, "OverwriteFiles=%d", &g_settings.overwriteFiles);
			} else if (strstr(line, "ShowStatusBar=")) {
				sscanf(line, "ShowStatusBar=%d", &g_settings.showStatusBar);
			} else if (strstr(line, "ShowGridLines=")) {
				sscanf(line, "ShowGridLines=%d", &g_settings.showGridLines);
			} else if (strstr(line, "WindowWidth=")) {
				sscanf(line, "WindowWidth=%d", &g_settings.windowWidth);
			} else if (strstr(line, "WindowHeight=")) {
				sscanf(line, "WindowHeight=%d", &g_settings.windowHeight);
			} else if (strstr(line, "WindowMaximized=")) {
				sscanf(line, "WindowMaximized=%d", &g_settings.windowMaximized);
			}
		}
		fclose(f);
	}
}

void SaveSettings(void)
{
	char path[MAX_PATH];
	SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path);
	strcat(path, "\\xPack");

	CreateDirectoryA(path, NULL);
	strcat(path, "\\settings.ini");

	FILE* f = fopen(path, "w");
	if (f) {
		fprintf(f, "[Settings]\n");
		fprintf(f, "DefaultCompLevel=%d\n", g_settings.defaultCompLevel);
		fprintf(f, "DefaultPkgType=%d\n", g_settings.defaultPkgType);
		fprintf(f, "SolidMode=%d\n", g_settings.solidMode);
		fprintf(f, "VolumeMode=%d\n", g_settings.volumeMode);
		fprintf(f, "VolumeSize=%u\n", g_settings.volumeSize);
		fprintf(f, "ConfirmDelete=%d\n", g_settings.confirmDelete);
		fprintf(f, "OverwriteFiles=%d\n", g_settings.overwriteFiles);
		fprintf(f, "ShowStatusBar=%d\n", g_settings.showStatusBar);
		fprintf(f, "ShowGridLines=%d\n", g_settings.showGridLines);
		fprintf(f, "[Window]\n");
		fprintf(f, "WindowWidth=%d\n", g_settings.windowWidth);
		fprintf(f, "WindowHeight=%d\n", g_settings.windowHeight);
		fprintf(f, "WindowMaximized=%d\n", g_settings.windowMaximized);
		fclose(f);
	}
}

void LoadHistory(void)
{
	char path[MAX_PATH];
	SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path);
	strcat(path, "\\xPack\\history.ini");

	FILE* f = fopen(path, "r");
	if (f) {
		char line[MAX_PATH];
		while (fgets(line, sizeof(line), f)) {
			if (strstr(line, "Path") == line) {
				int index;
				sscanf(line, "Path%d=%s", &index, g_history[index].path);
			} else if (strstr(line, "Count=")) {
				sscanf(line, "Count=%d", &g_historyCount);
			}
		}
		fclose(f);
	}
}

void SaveHistory(const char* path)
{
	if (!path || strlen(path) == 0) {
		return;
	}

	for (int i = 0; i < g_historyCount; i++) {
		if (_stricmp(g_history[i].path, path) == 0) {
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

	strcpy(g_history[g_historyCount].path, path);
	g_history[g_historyCount].timestamp = time(NULL);
	g_historyCount++;

	char settingsPath[MAX_PATH];
	SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, settingsPath);
	strcat(settingsPath, "\\xPack");
	CreateDirectoryA(settingsPath, NULL);
	strcat(settingsPath, "\\history.ini");

	FILE* f = fopen(settingsPath, "w");
	if (f) {
		fprintf(f, "[History]\n");
		fprintf(f, "Count=%d\n", g_historyCount);
		for (int i = 0; i < g_historyCount; i++) {
			fprintf(f, "Path%d=%s\n", i, g_history[i].path);
		}
		fclose(f);
	}
}

void AddToHistory(const char* path)
{
	SaveHistory(path);
}