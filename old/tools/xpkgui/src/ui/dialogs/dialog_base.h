#ifndef XPKGUI_DIALOG_BASE_H
#define XPKGUI_DIALOG_BASE_H

#include <xpkgui/define.h>

typedef LPITEMIDLIST (__stdcall *PFNSHBROWSEFORFOLDERW)(BROWSEINFOW*);
typedef BOOL (__stdcall *PFNSHGETPATHFROMIDLISTW)(LPITEMIDLIST, LPWSTR);
typedef void (__stdcall *PFNCoTaskMemFree)(LPVOID);

static HMODULE hShell32 = NULL;
static PFNSHBROWSEFORFOLDERW pSHBrowseForFolderW = NULL;
static PFNSHGETPATHFROMIDLISTW pSHGetPathFromIDListW = NULL;
static PFNCoTaskMemFree pCoTaskMemFree = NULL;

static const wchar_t* g_pkgTypeDesc[] = {
    L"Win32 (推荐)  - 路径访问，不区分大小写",
    L"Linux        - 路径访问，区分大小写",
    L"Index        - 整数索引访问",
    L"Core         - 顺序位置访问"
};

static void InitShell32Functions(void)
{
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
}

int Dialog_BrowseForFolder(HWND hwnd, wchar_t* path, const wchar_t* title)
{
    InitShell32Functions();

    if (!pSHBrowseForFolderW || !pSHGetPathFromIDListW || !pCoTaskMemFree) {
        return 0;
    }

    BROWSEINFOW bi = {0};
    LPITEMIDLIST pidl;
    wchar_t szPath[MAX_PATH_W];

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

int Dialog_BrowseForDirectory(HWND hwnd, wchar_t* path, const wchar_t* title)
{
    return Dialog_BrowseForFolder(hwnd, path, title);
}

int Dialog_BrowseForFiles(HWND hwnd, wchar_t* files, int* fileCount, const wchar_t* filter)
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

INT_PTR CALLBACK CompressLevelDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int* pLevel = NULL;

    switch (message) {
        case WM_INITDIALOG:
            pLevel = (int*)lParam;
            {
                HWND hCombo = GetDlgItem(hDlg, 1001);
                for (int i = 0; i < 16; i++) {
                    wchar_t item[64];
                    swprintf_s(item, sizeof(item) / sizeof(wchar_t), L"%d - %s", i, g_compLevelDesc[i]);
                    SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)item);
                }
                SendMessageW(hCombo, CB_SETCURSEL, *pLevel, 0);
                SetWindowTextW(GetDlgItem(hDlg, 1002), g_compLevelDesc[*pLevel]);
            }
            return TRUE;

        case WM_COMMAND:
            if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == 1001) {
                HWND hCombo = GetDlgItem(hDlg, 1001);
                int sel = (int)SendMessageW(hCombo, CB_GETCURSEL, 0, 0);
                if (sel >= 0 && sel < 16) {
                    SetWindowTextW(GetDlgItem(hDlg, 1002), g_compLevelDesc[sel]);
                }
            } else if (LOWORD(wParam) == IDOK) {
                HWND hCombo = GetDlgItem(hDlg, 1001);
                *pLevel = (int)SendMessageW(hCombo, CB_GETCURSEL, 0, 0);
                EndDialog(hDlg, IDOK);
                return TRUE;
            } else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

int Dialog_CompressLevel(HWND hwnd, int* level)
{
    return DialogBoxParamW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(2001), hwnd, CompressLevelDlgProc, (LPARAM)level);
}

typedef struct {
    wchar_t* path;
    int* solidMode;
    int* pkgType;
} NewPackageParams;

INT_PTR CALLBACK NewPackageDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static NewPackageParams* pParams = NULL;

    switch (message) {
        case WM_INITDIALOG:
            pParams = (NewPackageParams*)lParam;
            {
                HWND hRadioWin32 = GetDlgItem(hDlg, 2000);
                HWND hRadioLinux = GetDlgItem(hDlg, 2001);
                HWND hRadioIndex = GetDlgItem(hDlg, 2002);
                HWND hRadioCore = GetDlgItem(hDlg, 2003);
                HWND hCheckSolid = GetDlgItem(hDlg, 1007);

                switch (*pParams->pkgType) {
                    case XPK_TYPE_WIN32:
                        SendMessageW(hRadioWin32, BM_SETCHECK, BST_CHECKED, 0);
                        break;
                    case XPK_TYPE_LINUX:
                        SendMessageW(hRadioLinux, BM_SETCHECK, BST_CHECKED, 0);
                        break;
                    case XPK_TYPE_INDEX:
                        SendMessageW(hRadioIndex, BM_SETCHECK, BST_CHECKED, 0);
                        break;
                    case XPK_TYPE_CORE:
                        SendMessageW(hRadioCore, BM_SETCHECK, BST_CHECKED, 0);
                        break;
                }

                if (*pParams->solidMode) {
                    SendMessageW(hCheckSolid, BM_SETCHECK, BST_CHECKED, 0);
                }
            }
            return TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == 1004) {
                OPENFILENAMEW ofn = {0};
                wchar_t savePath[MAX_PATH_W] = {0};
                ofn.lStructSize = sizeof(OPENFILENAMEW);
                ofn.hwndOwner = hDlg;
                ofn.lpstrFilter = L"xPack 文件 (*.xpk)\0*.xpk\0所有文件 (*.*)\0*.*\0";
                ofn.lpstrFile = savePath;
                ofn.nMaxFile = MAX_PATH_W;
                ofn.Flags = OFN_OVERWRITEPROMPT;
                ofn.lpstrDefExt = L"xpk";

                if (GetSaveFileNameW(&ofn)) {
                    SetWindowTextW(GetDlgItem(hDlg, 1001), savePath);
                }
            } else if (LOWORD(wParam) == IDOK) {
                GetWindowTextW(GetDlgItem(hDlg, 1001), pParams->path, MAX_PATH_W);
                *pParams->solidMode = SendMessageW(GetDlgItem(hDlg, 1007), BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0;
                if (SendMessageW(GetDlgItem(hDlg, 2000), BM_GETCHECK, 0, 0)) *pParams->pkgType = XPK_TYPE_WIN32;
                else if (SendMessageW(GetDlgItem(hDlg, 2001), BM_GETCHECK, 0, 0)) *pParams->pkgType = XPK_TYPE_LINUX;
                else if (SendMessageW(GetDlgItem(hDlg, 2002), BM_GETCHECK, 0, 0)) *pParams->pkgType = XPK_TYPE_INDEX;
                else *pParams->pkgType = XPK_TYPE_CORE;
                EndDialog(hDlg, IDOK);
                return TRUE;
            } else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

int Dialog_NewPackage(HWND hwnd, wchar_t* path, int* solidMode, int* pkgType)
{
    NewPackageParams params = { path, solidMode, pkgType };
    return DialogBoxParamW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(3001), hwnd, NewPackageDlgProc, (LPARAM)&params);
}

INT_PTR CALLBACK InputBoxDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static wchar_t* pBuffer = NULL;
    static int bufferSize = 0;

    switch (message) {
        case WM_INITDIALOG:
            {
                struct { const wchar_t* prompt; wchar_t* buffer; int size; }* params = (void*)lParam;
                SetWindowTextW(GetDlgItem(hDlg, 1001), params->prompt);
                SetWindowTextW(GetDlgItem(hDlg, 1002), params->buffer);
                pBuffer = params->buffer;
                bufferSize = params->size;
            }
            return TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                GetWindowTextW(GetDlgItem(hDlg, 1002), pBuffer, bufferSize);
                EndDialog(hDlg, IDOK);
                return TRUE;
            } else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

int Dialog_InputBox(HWND hwnd, const wchar_t* title, const wchar_t* prompt, wchar_t* buffer, int bufferSize)
{
    struct { const wchar_t* prompt; wchar_t* buffer; int size; } params = { prompt, buffer, bufferSize };
    return DialogBoxParamW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(2002), hwnd, InputBoxDlgProc, (LPARAM)&params);
}

typedef struct {
    wchar_t* pattern;
    int* operation;
} PatternSelectParams;

INT_PTR CALLBACK PatternSelectDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static PatternSelectParams* pParams = NULL;

    switch (message) {
        case WM_INITDIALOG:
            pParams = (PatternSelectParams*)lParam;
            {
                HWND hCombo = GetDlgItem(hDlg, 1001);
                SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"解压到目录");
                SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"删除文件");
                SendMessageW(hCombo, CB_SETCURSEL, 0, 0);
                SetWindowTextW(GetDlgItem(hDlg, 1002), pParams->pattern);
            }
            return TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                GetWindowTextW(GetDlgItem(hDlg, 1002), pParams->pattern, MAX_PATH_W);
                HWND hCombo = GetDlgItem(hDlg, 1001);
                *pParams->operation = (int)SendMessageW(hCombo, CB_GETCURSEL, 0, 0);
                EndDialog(hDlg, IDOK);
                return TRUE;
            } else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

int Dialog_PatternSelect(HWND hwnd, wchar_t* pattern, int* operation)
{
    PatternSelectParams params = { pattern, operation };
    return DialogBoxParamW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(3002), hwnd, PatternSelectDlgProc, (LPARAM)&params);
}

#endif
