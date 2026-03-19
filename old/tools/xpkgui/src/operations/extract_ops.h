#ifndef XPKGUI_EXTRACT_OPS_H
#define XPKGUI_EXTRACT_OPS_H

#include <xpkgui/define.h>

int ExtractOps_ExtractSelected(AppContext* ctx)
{
    if (ctx->xpk == NULL) {
        ErrorHandler_Error(ctx->hMainWnd, L"没有打开的压缩包");
        return -1;
    }

    int selected = FileList_GetSelectedIndex(ctx);
    if (selected < 0) {
        return -1;
    }

    wchar_t path[MAX_PATH_W];
    if (FileList_GetSelectedPath(ctx, path, MAX_PATH_W) == 0) {
        return -1;
    }

    wchar_t savePath[MAX_PATH_W];
    wcscpy_s(savePath, MAX_PATH_W, ctx->currentDir);

    char* utf8Path = String_WcharToUtf8(path);
    char* utf8SavePath = String_WcharToUtf8(savePath);
    if (utf8Path && utf8SavePath) {
        if (xpkPathExtractFile(ctx->xpk, utf8Path, utf8SavePath) == 0) {
            ErrorHandler_Info(ctx->hMainWnd, L"解压成功");
            FileList_Refresh(ctx);
        } else {
            ErrorHandler_ShowDetailed(ctx->hMainWnd, L"解压失败");
        }
    }
    if (utf8Path) free(utf8Path);
    if (utf8SavePath) free(utf8SavePath);

    return 0;
}

int ExtractOps_ExtractAll(AppContext* ctx)
{
    if (ctx->xpk == NULL) {
        ErrorHandler_Error(ctx->hMainWnd, L"没有打开的压缩包");
        return -1;
    }

    wchar_t savePath[MAX_PATH_W];
    wcscpy_s(savePath, MAX_PATH_W, ctx->currentDir);

    char* utf8SavePath = String_WcharToUtf8(savePath);
    if (utf8SavePath) {
        if (xpkExtractAll(ctx->xpk, utf8SavePath) == 0) {
            ErrorHandler_Info(ctx->hMainWnd, L"解压成功");
            FileList_Refresh(ctx);
        } else {
            ErrorHandler_ShowDetailed(ctx->hMainWnd, L"解压失败");
        }
        free(utf8SavePath);
    }

    return 0;
}

#endif
