#ifndef XPKGUI_FILE_LIST_H
#define XPKGUI_FILE_LIST_H

#include <xpkgui/define.h>

HWND FileList_Create(AppContext* ctx)
{
    RECT rc;
    GetClientRect(ctx->hMainWnd, &rc);

    HWND hList = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        WC_LISTVIEWW,
        NULL,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
        LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
        0, 0,
        rc.right, rc.bottom - 24,
        ctx->hMainWnd,
        (HMENU)IDC_FILELIST,
        ctx->hInstance,
        NULL
    );

    if (!hList) {
        return NULL;
    }

    DWORD exStyle = LVS_EX_FULLROWSELECT;
    if (ctx->settings.showGridLines) {
        exStyle |= LVS_EX_GRIDLINES;
    }
    ListView_SetExtendedListViewStyle(hList, exStyle);

    LVCOLUMNW lvc;
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    lvc.pszText = L"文件名";
    lvc.cx = 300;
    lvc.iSubItem = 0;
    ListView_InsertColumn(hList, 0, &lvc);

    lvc.pszText = L"大小";
    lvc.cx = 100;
    lvc.iSubItem = 1;
    ListView_InsertColumn(hList, 1, &lvc);

    lvc.pszText = L"压缩后";
    lvc.cx = 100;
    lvc.iSubItem = 2;
    ListView_InsertColumn(hList, 2, &lvc);

    lvc.pszText = L"压缩比";
    lvc.cx = 80;
    lvc.iSubItem = 3;
    ListView_InsertColumn(hList, 3, &lvc);

    lvc.pszText = L"算法";
    lvc.cx = 60;
    lvc.iSubItem = 4;
    ListView_InsertColumn(hList, 4, &lvc);

    lvc.pszText = L"类型";
    lvc.cx = 80;
    lvc.iSubItem = 5;
    ListView_InsertColumn(hList, 5, &lvc);

    lvc.pszText = L"哈希";
    lvc.cx = 100;
    lvc.iSubItem = 6;
    ListView_InsertColumn(hList, 6, &lvc);

    return hList;
}

void FileList_Refresh(AppContext* ctx)
{
    if (ctx->xpk == NULL) {
        return;
    }

    ListView_DeleteAllItems(ctx->hFileList);

    int type = xpkType(ctx->xpk);
    uint32_t count = xpkCount(ctx->xpk);

    for (uint32_t i = 0; i < count; i++) {
        wchar_t name[MAX_PATH_W] = {0};
        wchar_t size[64] = {0};
        wchar_t packed[64] = {0};
        wchar_t ratio[32] = {0};
        wchar_t algo[32] = {0};
        wchar_t fileType[32] = {0};
        wchar_t hashStr[64] = {0};

        uint32_t fileSize = xpkInfoSize(ctx->xpk, i);
        uint32_t packedSize = xpkInfoPacked(ctx->xpk, i);
        int level = xpkInfoLevel(ctx->xpk, i);
        int ftype = xpkInfoType(ctx->xpk, i);

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

        wcscpy_s(fileType, 32, FormatUtils_GetFileTypeString(ftype));

        uint32_t hash = xpkInfoHash(ctx->xpk, i);
        swprintf_s(hashStr, 64, L"%08X", hash);

        if (type == XPK_TYPE_WIN32 || type == XPK_TYPE_LINUX) {
            const char* path = xpkPathGet(ctx->xpk, i);
            if (path != NULL) {
                wchar_t* wpath = String_Utf8ToWchar(path);
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

        int pos = ListView_InsertItem(ctx->hFileList, &lvi);

        LVITEMW lviText = {0};
        lviText.mask = LVIF_TEXT;
        lviText.iItem = pos;
        lviText.iSubItem = 1;
        lviText.pszText = size;
        SendMessageW(ctx->hFileList, LVM_SETITEMW, 0, (LPARAM)&lviText);

        lviText.iSubItem = 2;
        lviText.pszText = packed;
        SendMessageW(ctx->hFileList, LVM_SETITEMW, 0, (LPARAM)&lviText);

        lviText.iSubItem = 3;
        lviText.pszText = ratio;
        SendMessageW(ctx->hFileList, LVM_SETITEMW, 0, (LPARAM)&lviText);

        lviText.iSubItem = 4;
        lviText.pszText = algo;
        SendMessageW(ctx->hFileList, LVM_SETITEMW, 0, (LPARAM)&lviText);

        lviText.iSubItem = 5;
        lviText.pszText = fileType;
        SendMessageW(ctx->hFileList, LVM_SETITEMW, 0, (LPARAM)&lviText);

        lviText.iSubItem = 6;
        lviText.pszText = hashStr;
        SendMessageW(ctx->hFileList, LVM_SETITEMW, 0, (LPARAM)&lviText);
    }

    Window_UpdateStatusBar(ctx);
}

int FileList_GetSelectedIndex(AppContext* ctx)
{
    return ListView_GetNextItem(ctx->hFileList, -1, LVNI_SELECTED);
}

int FileList_GetSelectedPath(AppContext* ctx, wchar_t* path, int maxLen)
{
    int selected = FileList_GetSelectedIndex(ctx);
    if (selected < 0) {
        return -1;
    }

    LVITEMW lvi = {0};
    lvi.mask = LVIF_TEXT;
    lvi.iItem = selected;
    lvi.iSubItem = 0;
    lvi.pszText = path;
    lvi.cchTextMax = maxLen;

    if (!ListView_GetItem(ctx->hFileList, &lvi)) {
        return -1;
    }

    return 0;
}

#endif
