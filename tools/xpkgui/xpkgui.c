#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "resource.h"
#include "../../lib/xrt/xrt.h"
#include "../../src/xpack.h"

#pragma comment(lib, "shell32")
#pragma comment(lib, "comctl32")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define BIF_RETURNONLYFSDIRS 0x0001
#define BIF_NEWDIALOGSTYLE 0x0040

typedef void* LPITEMIDLIST;

static HMODULE hShell32 = NULL;

typedef INT_PTR (__stdcall *BFFCALLBACK)(HWND, UINT, LPARAM, LPARAM);

typedef struct _browseinfoA {
	HWND hwndOwner;
	LPCSTR pidlRoot;
	LPSTR pszDisplayName;
	LPCSTR lpszTitle;
	UINT ulFlags;
	BFFCALLBACK lpfn;
	LPARAM lParam;
	int iImage;
} BROWSEINFOA, *PBROWSEINFOA, *LPBROWSEINFOA;

typedef LPITEMIDLIST (__stdcall *PFNSHBROWSEFORFOLDERA)(PBROWSEINFOA);
typedef BOOL (__stdcall *PFNSHGETPATHFROMIDLISTA)(LPITEMIDLIST, LPSTR);
typedef void (__stdcall *PFNCoTaskMemFree)(LPVOID);

static PFNSHBROWSEFORFOLDERA pSHBrowseForFolderA = NULL;
static PFNSHGETPATHFROMIDLISTA pSHGetPathFromIDListA = NULL;
static PFNCoTaskMemFree pCoTaskMemFree = NULL;



HINSTANCE g_hInstance = NULL;
HWND g_hMainWnd = NULL;
HWND g_hFileList = NULL;
HWND g_hStatusBar = NULL;
xpkObject g_xpk = NULL;
char g_xpkPath[MAX_PATH] = {0};
char g_currentDir[MAX_PATH] = {0};

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL InitMainWnd(HWND hwnd);
void UpdateStatusBar(void);
void RefreshFileList(void);
void UpdateTitle(void);
int OpenXpkFile(const char* path);
int CloseXpkFile(void);
int AddFiles(void);
int ExtractFiles(void);
int ExtractAll(void);
int DeleteFiles(void);
int RenameFiles(void);
int CreateNewPackage(void);
int RebuildPackage(void);
int VerifyPackage(void);
int GetPackageProperties(void);
void ShowAboutDialog(void);
int BrowseForFolder(HWND hwnd, char* path, const char* title);
int BrowseForFiles(HWND hwnd, char* files, int* fileCount, const char* filter);
int InputBox(HWND hwnd, const char* title, const char* prompt, char* buffer, int bufferSize);
void FormatSize(uint64_t size, char* buf, int bufSize);
void ErrorMsg(HWND hwnd, const char* msg);
void InfoMsg(HWND hwnd, const char* msg);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	INITCOMMONCONTROLSEX icc;
	MSG msg;
	WNDCLASSEX wc;
	BOOL bRet;

	g_hInstance = hInstance;

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

	if ( !RegisterClassEx(&wc) ) {
		return 0;
	}

	g_hMainWnd = CreateWindowEx(
		0,
		"xpkguiClass",
		"xpkgui - xPack 管理工具",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		900, 600,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if ( !g_hMainWnd ) {
		return 0;
	}

	ShowWindow(g_hMainWnd, nCmdShow);
	UpdateWindow(g_hMainWnd);

	GetCurrentDirectory(MAX_PATH, g_currentDir);

	while ( (bRet = GetMessage(&msg, NULL, 0, 0)) != 0 ) {
		if ( bRet == -1 ) {
			break;
		} else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	if ( g_xpk != NULL ) {
		xpkClose(g_xpk);
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh;

	switch ( message ) {
		case WM_CREATE:
			if ( !InitMainWnd(hwnd) ) {
				return -1;
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
			}
			break;

		case WM_COMMAND:
			switch ( LOWORD(wParam) ) {
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

						if ( GetOpenFileName(&ofn) ) {
							OpenXpkFile(path);
						}
					}
					break;

				case ID_FILE_CLOSE:
					CloseXpkFile();
					break;

				case ID_FILE_SAVE:
					if ( g_xpk != NULL ) {
						if ( xpkSave(g_xpk) == 0 ) {
							InfoMsg(hwnd, "保存成功");
						} else {
							ErrorMsg(hwnd, "保存失败");
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

				case ID_TOOLS_EXTRACTALL:
					ExtractAll();
					break;

				case ID_HELP_ABOUT:
					ShowAboutDialog();
					break;
			}
			break;

		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			if ( pnmh->hwndFrom == g_hFileList ) {
				if ( pnmh->code == NM_DBLCLK ) {
					LPNMITEMACTIVATE pnmia = (LPNMITEMACTIVATE)lParam;
					if ( pnmia->iItem >= 0 ) {
						if ( g_xpk != NULL ) {
							int type = xpkType(g_xpk);
							if ( type == XPK_TYPE_WIN32 || type == XPK_TYPE_LINUX ) {
								char filePath[MAX_PATH];
								LVITEM lvi = {0};
								lvi.mask = LVIF_TEXT;
								lvi.iItem = pnmia->iItem;
								lvi.iSubItem = 0;
								lvi.pszText = filePath;
								lvi.cchTextMax = MAX_PATH;

								if ( ListView_GetItem(g_hFileList, &lvi) ) {
									char savePath[MAX_PATH];
									if ( BrowseForFolder(hwnd, savePath, "选择解压目录") ) {
										strcat(savePath, "\\");
										strcat(savePath, filePath);

										if ( xpkPathExtractFile(g_xpk, filePath, savePath) == 0 ) {
											InfoMsg(hwnd, "解压成功");
										} else {
											ErrorMsg(hwnd, "解压失败");
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
			if ( g_xpk != NULL ) {
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
	HIMAGELIST hImgList;

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

	if ( !g_hFileList ) {
		return FALSE;
	}

	ListView_SetExtendedListViewStyle(g_hFileList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

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

	UpdateStatusBar();

	return TRUE;
}

void UpdateStatusBar(void)
{
	char buf[256] = {0};

	if ( g_xpk != NULL ) {
		xpkStat stat;
		if ( xpkStatGet(g_xpk, &stat) == 0 ) {
			char sizeBuf[64];
			FormatSize(stat.totalSize, sizeBuf, sizeof(sizeBuf));
			sprintf_s(buf, sizeof(buf), "文件: %u | 总大小: %s | 压缩率: %.1f%%",
				stat.fileCount, sizeBuf, stat.ratio * 100);
		}
	} else {
		strcpy(buf, "未打开压缩包");
	}

	SendMessage(g_hStatusBar, WM_SETTEXT, 0, (LPARAM)buf);
}

void RefreshFileList(void)
{
	if ( g_xpk == NULL ) {
		return;
	}

	ListView_DeleteAllItems(g_hFileList);

	int type = xpkType(g_xpk);
	uint32_t count = xpkCount(g_xpk);

	for ( uint32_t i = 0; i < count; i++ ) {
		char name[MAX_PATH] = {0};
		char size[64] = {0};
		char packed[64] = {0};
		char ratio[32] = {0};
		char algo[32] = {0};

		uint32_t fileSize = xpkInfoSize(g_xpk, i);
		uint32_t packedSize = xpkInfoPacked(g_xpk, i);
		int level = xpkInfoLevel(g_xpk, i);

		FormatSize(fileSize, size, sizeof(size));
		FormatSize(packedSize, packed, sizeof(packed));

		if ( packedSize > 0 ) {
			sprintf_s(ratio, sizeof(ratio), "%.1f%%", (double)packedSize / fileSize * 100);
		} else {
			strcpy(ratio, "N/A");
		}

		if ( level == 0 ) {
			strcpy(algo, "无");
		} else if ( level <= 4 ) {
			strcpy(algo, "LZ4");
		} else if ( level <= 13 ) {
			strcpy(algo, "ZSTD");
		} else {
			strcpy(algo, "LZMA2");
		}

		if ( type == XPK_TYPE_WIN32 || type == XPK_TYPE_LINUX ) {
			const char* path = xpkPathGet(g_xpk, i);
			if ( path != NULL ) {
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
	}

	UpdateStatusBar();
}

void UpdateTitle(void)
{
	char title[MAX_PATH + 64];

	if ( g_xpk != NULL ) {
		sprintf_s(title, sizeof(title), "xpkgui - %s", g_xpkPath);
	} else {
		strcpy(title, "xpkgui - xPack 管理工具");
	}

	SetWindowText(g_hMainWnd, title);
}

int OpenXpkFile(const char* path)
{
	if ( g_xpk != NULL ) {
		xpkClose(g_xpk);
		g_xpk = NULL;
	}

	g_xpk = xpkOpen(path, 0, 0);

	if ( g_xpk == NULL ) {
		ErrorMsg(g_hMainWnd, "无法打开压缩包文件");
		return -1;
	}

	strcpy(g_xpkPath, path);
	RefreshFileList();
	UpdateTitle();
	return 0;
}

int CloseXpkFile(void)
{
	if ( g_xpk != NULL ) {
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
	if ( g_xpk == NULL ) {
		ErrorMsg(g_hMainWnd, "请先打开或创建一个压缩包");
		return -1;
	}

	int type = xpkType(g_xpk);
	if ( type != XPK_TYPE_WIN32 && type != XPK_TYPE_LINUX ) {
		ErrorMsg(g_hMainWnd, "当前模式不支持文件路径操作");
		return -1;
	}

	char files[4096] = {0};
	int fileCount = 0;

	if ( !BrowseForFiles(g_hMainWnd, files, &fileCount, "所有文件 (*.*)\0*.*\0") ) {
		return -1;
	}

	char* p = files;
	int success = 0;

	while ( *p != '\0' ) {
		char fileName[MAX_PATH];
		strcpy(fileName, p);

		const char* slash = strrchr(fileName, '\\');
		const char* slash2 = strrchr(fileName, '/');
		const char* name = (slash2 > slash) ? slash2 : slash;
		name = name ? name + 1 : fileName;

		if ( xpkPathAppendFile(g_xpk, name, fileName, 7) != NULL ) {
			success++;
		}

		p += strlen(p) + 1;
	}

	if ( success > 0 ) {
		RefreshFileList();
		InfoMsg(g_hMainWnd, "添加成功");
	} else {
		ErrorMsg(g_hMainWnd, "添加失败");
	}

	return 0;
}

int ExtractFiles(void)
{
	if ( g_xpk == NULL ) {
		ErrorMsg(g_hMainWnd, "请先打开一个压缩包");
		return -1;
	}

	int selected = ListView_GetNextItem(g_hFileList, -1, LVNI_SELECTED);
	if ( selected < 0 ) {
		ErrorMsg(g_hMainWnd, "请选择要解压的文件");
		return -1;
	}

	int type = xpkType(g_xpk);
	if ( type != XPK_TYPE_WIN32 && type != XPK_TYPE_LINUX ) {
		ErrorMsg(g_hMainWnd, "当前模式不支持文件路径操作");
		return -1;
	}

	char savePath[MAX_PATH];
	if ( !BrowseForFolder(g_hMainWnd, savePath, "选择解压目录") ) {
		return -1;
	}

	char filePath[MAX_PATH];
	LVITEM lvi = {0};
	lvi.mask = LVIF_TEXT;
	lvi.iItem = selected;
	lvi.iSubItem = 0;
	lvi.pszText = filePath;
	lvi.cchTextMax = MAX_PATH;

	if ( !ListView_GetItem(g_hFileList, &lvi) ) {
		return -1;
	}

	strcat(savePath, "\\");
	strcat(savePath, filePath);

	if ( xpkPathExtractFile(g_xpk, filePath, savePath) == 0 ) {
		InfoMsg(g_hMainWnd, "解压成功");
		return 0;
	} else {
		ErrorMsg(g_hMainWnd, "解压失败");
		return -1;
	}
}

int ExtractAll(void)
{
	if ( g_xpk == NULL ) {
		ErrorMsg(g_hMainWnd, "请先打开一个压缩包");
		return -1;
	}

	char savePath[MAX_PATH];
	if ( !BrowseForFolder(g_hMainWnd, savePath, "选择解压目录") ) {
		return -1;
	}

	if ( xpkExtractAll(g_xpk, savePath) == 0 ) {
		InfoMsg(g_hMainWnd, "全部解压成功");
		return 0;
	} else {
		ErrorMsg(g_hMainWnd, "解压失败");
		return -1;
	}
}

int DeleteFiles(void)
{
	if ( g_xpk == NULL ) {
		ErrorMsg(g_hMainWnd, "请先打开一个压缩包");
		return -1;
	}

	int selected = ListView_GetNextItem(g_hFileList, -1, LVNI_SELECTED);
	if ( selected < 0 ) {
		ErrorMsg(g_hMainWnd, "请选择要删除的文件");
		return -1;
	}

	int type = xpkType(g_xpk);
	if ( type != XPK_TYPE_WIN32 && type != XPK_TYPE_LINUX ) {
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

	if ( !ListView_GetItem(g_hFileList, &lvi) ) {
		return -1;
	}

	if ( MessageBox(g_hMainWnd, "确定要删除选中的文件吗?", "确认删除",
		MB_YESNO | MB_ICONQUESTION) == IDYES ) {

		if ( xpkPathRemove(g_xpk, filePath) == 0 ) {
			RefreshFileList();
			InfoMsg(g_hMainWnd, "删除成功");
			return 0;
		} else {
			ErrorMsg(g_hMainWnd, "删除失败");
			return -1;
		}
	}

	return -1;
}

int RenameFiles(void)
{
	if ( g_xpk == NULL ) {
		ErrorMsg(g_hMainWnd, "请先打开一个压缩包");
		return -1;
	}

	int selected = ListView_GetNextItem(g_hFileList, -1, LVNI_SELECTED);
	if ( selected < 0 ) {
		ErrorMsg(g_hMainWnd, "请选择要重命名的文件");
		return -1;
	}

	int type = xpkType(g_xpk);
	if ( type != XPK_TYPE_WIN32 && type != XPK_TYPE_LINUX ) {
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

	if ( !ListView_GetItem(g_hFileList, &lvi) ) {
		return -1;
	}

	char newPath[MAX_PATH] = {0};
	strcpy(newPath, oldPath);

	if ( InputBox(g_hMainWnd, "重命名", "输入新的文件名:", newPath, MAX_PATH) == IDOK ) {
		if ( xpkPathUpdateFile(g_xpk, oldPath, newPath, 7) == 0 ) {
			RefreshFileList();
			InfoMsg(g_hMainWnd, "重命名成功");
			return 0;
		} else {
			ErrorMsg(g_hMainWnd, "重命名失败");
			return -1;
		}
	}

	return -1;
}

int CreateNewPackage(void)
{
	char path[MAX_PATH] = {0};
	OPENFILENAME ofn = {0};
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = g_hMainWnd;
	ofn.lpstrFilter = "xPack 文件 (*.xpk)\0*.xpk\0所有文件 (*.*)\0*.*\0";
	ofn.lpstrFile = path;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = "xpk";

	if ( !GetSaveFileName(&ofn) ) {
		return -1;
	}

	if ( g_xpk != NULL ) {
		xpkClose(g_xpk);
		g_xpk = NULL;
	}

	g_xpk = xpkOpen(path, 0, 0);

	if ( g_xpk == NULL ) {
		ErrorMsg(g_hMainWnd, "无法创建压缩包文件");
		return -1;
	}

	if ( xpkTypeSet(g_xpk, XPK_TYPE_WIN32) != 0 ) {
		ErrorMsg(g_hMainWnd, "设置包类型失败");
		xpkClose(g_xpk);
		g_xpk = NULL;
		return -1;
	}

	strcpy(g_xpkPath, path);
	RefreshFileList();
	UpdateTitle();
	InfoMsg(g_hMainWnd, "新建压缩包成功");
	return 0;
}

int RebuildPackage(void)
{
	if ( g_xpk == NULL ) {
		ErrorMsg(g_hMainWnd, "请先打开一个压缩包");
		return -1;
	}

	if ( MessageBox(g_hMainWnd, "重建压缩包可能需要较长时间,确定要继续吗?",
		"确认重建", MB_YESNO | MB_ICONQUESTION) == IDYES ) {

		if ( xpkRebuild(g_xpk) == 0 ) {
			RefreshFileList();
			InfoMsg(g_hMainWnd, "重建成功");
			return 0;
		} else {
			ErrorMsg(g_hMainWnd, "重建失败");
			return -1;
		}
	}

	return -1;
}

int VerifyPackage(void)
{
	if ( g_xpk == NULL ) {
		ErrorMsg(g_hMainWnd, "请先打开一个压缩包");
		return -1;
	}

	int result = xpkVerifyAll(g_xpk);

	if ( result == 0 ) {
		InfoMsg(g_hMainWnd, "所有文件验证通过");
	} else if ( result > 0 ) {
		char msg[256];
		sprintf_s(msg, sizeof(msg), "验证失败: %d 个文件有问题", result);
		ErrorMsg(g_hMainWnd, msg);
	} else {
		ErrorMsg(g_hMainWnd, "验证过程出错");
	}

	return 0;
}

int GetPackageProperties(void)
{
	if ( g_xpk == NULL ) {
		ErrorMsg(g_hMainWnd, "请先打开一个压缩包");
		return -1;
	}

	xpkHead* head = xpkGetHead(g_xpk);
	if ( head == NULL ) {
		ErrorMsg(g_hMainWnd, "获取包信息失败");
		return -1;
	}

	xpkStat stat;
	xpkStatGet(g_xpk, &stat);

	char sizeBuf[64];
	FormatSize(stat.totalSize, sizeBuf, sizeof(sizeBuf));

	char msg[512];
	const char* typeStr = "Unknown";

	switch ( xpkType(g_xpk) ) {
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

	sprintf_s(msg, sizeof(msg),
		"压缩包信息:\n\n"
		"类型: %s\n"
		"文件数: %u\n"
		"总大小: %s\n"
		"压缩后: %llu 字节\n"
		"压缩率: %.1f%%\n"
		"识别代码: 0x%08X\n",
		typeStr,
		head->fileCount,
		sizeBuf,
		stat.packedSize,
		stat.ratio * 100,
		head->discCode
	);

	MessageBox(g_hMainWnd, msg, "压缩包属性", MB_OK | MB_ICONINFORMATION);

	return 0;
}

void ShowAboutDialog(void)
{
	MessageBox(g_hMainWnd,
		"xpkgui - xPack 管理工具\n\n"
		"版本: 1.0\n"
		"基于 xPack 文件压缩库\n"
		"支持多种压缩算法和包模式",
		"关于 xpkgui",
		MB_OK | MB_ICONINFORMATION
	);
}

int BrowseForFolder(HWND hwnd, char* path, const char* title)
{
	BROWSEINFOA bi = {0};
	LPITEMIDLIST pidl;
	char szPath[MAX_PATH];

	if ( !hShell32 ) {
		hShell32 = LoadLibraryA("shell32.dll");
	}

	if ( hShell32 != NULL ) {
		if ( !pSHBrowseForFolderA ) {
			pSHBrowseForFolderA = (PFNSHBROWSEFORFOLDERA)GetProcAddress(hShell32, "SHBrowseForFolderA");
		}
		if ( !pSHGetPathFromIDListA ) {
			pSHGetPathFromIDListA = (PFNSHGETPATHFROMIDLISTA)GetProcAddress(hShell32, "SHGetPathFromIDListA");
		}
		if ( !pCoTaskMemFree ) {
			pCoTaskMemFree = (PFNCoTaskMemFree)GetProcAddress(hShell32, "CoTaskMemFree");
		}
	}

	if ( !pSHBrowseForFolderA || !pSHGetPathFromIDListA || !pCoTaskMemFree ) {
		return 0;
	}

	bi.hwndOwner = hwnd;
	bi.pszDisplayName = szPath;
	bi.lpszTitle = title;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	pidl = pSHBrowseForFolderA(&bi);

	if ( pidl != NULL ) {
		if ( pSHGetPathFromIDListA(pidl, path) ) {
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

	if ( GetOpenFileName(&ofn) ) {
		strcpy(files, fileBuf);

		char* p = fileBuf;
		*fileCount = 0;

		if ( *(p + strlen(p) + 1) == '\0' ) {
			*fileCount = 1;
		} else {
			p += strlen(p) + 1;
			while ( *p != '\0' ) {
				(*fileCount)++;
				p += strlen(p) + 1;
			}
		}

		return 1;
	}

	return 0;
}

void FormatSize(uint64_t size, char* buf, int bufSize)
{
	if ( size < 1024 ) {
		sprintf_s(buf, bufSize, "%llu B", size);
	} else if ( size < 1024 * 1024 ) {
		sprintf_s(buf, bufSize, "%.2f KB", size / 1024.0);
	} else if ( size < 1024 * 1024 * 1024 ) {
		sprintf_s(buf, bufSize, "%.2f MB", size / (1024.0 * 1024.0));
	} else {
		sprintf_s(buf, bufSize, "%.2f GB", size / (1024.0 * 1024.0 * 1024.0));
	}
}

void ErrorMsg(HWND hwnd, const char* msg)
{
	MessageBox(hwnd, msg, "错误", MB_OK | MB_ICONERROR);
}

void InfoMsg(HWND hwnd, const char* msg)
{
	MessageBox(hwnd, msg, "信息", MB_OK | MB_ICONINFORMATION);
}

int InputBox(HWND hwnd, const char* title, const char* prompt, char* buffer, int bufferSize)
{
	HWND hDlg;
	HWND hEdit;
	HWND hPrompt;
	HWND hOK, hCancel;
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

	if ( !hDlg ) {
		return IDCANCEL;
	}

	RECT rc;
	GetClientRect(hDlg, &rc);
	MoveWindow(hDlg, 0, 0, 300, 140, TRUE);

	hPrompt = CreateWindowEx(
		0, "STATIC", prompt,
		WS_CHILD | WS_VISIBLE,
		10, 10, 280, 20,
		hDlg, NULL, g_hInstance, NULL
	);

	hEdit = CreateWindowEx(
		WS_EX_CLIENTEDGE, "EDIT", buffer,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		10, 35, 280, 20,
		hDlg, NULL, g_hInstance, NULL
	);

	hOK = CreateWindowEx(
		0, "BUTTON", "确定",
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		50, 70, 80, 25,
		hDlg, (HMENU)IDOK, g_hInstance, NULL
	);

	hCancel = CreateWindowEx(
		0, "BUTTON", "取消",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		170, 70, 80, 25,
		hDlg, (HMENU)IDCANCEL, g_hInstance, NULL
	);

	SetFocus(hEdit);
	ShowWindow(hDlg, SW_SHOW);
	EnableWindow(hwnd, FALSE);

	while ( (bRet = GetMessage(&msg, NULL, 0, 0)) != 0 ) {
		if ( bRet == -1 ) {
			break;
		} else if ( !IsDialogMessage(hDlg, &msg) ) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if ( msg.message == WM_COMMAND ) {
			if ( LOWORD(msg.wParam) == IDOK ) {
				GetWindowText(hEdit, buffer, bufferSize);
				result = IDOK;
				break;
			} else if ( LOWORD(msg.wParam) == IDCANCEL ) {
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
