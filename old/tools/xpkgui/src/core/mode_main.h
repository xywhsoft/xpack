#ifndef XPKGUI_MODE_MAIN_H
#define XPKGUI_MODE_MAIN_H

#include <xpkgui/define.h>

int ExtractModeMain(AppContext* ctx, const wchar_t* path)
{
    wchar_t savePath[MAX_PATH_W];
    wcscpy_s(savePath, MAX_PATH_W, ctx->currentDir);
    wcscat_s(savePath, MAX_PATH_W, L"\\extracted");

    if (!CreateDirectoryW(savePath, NULL)) {
        ErrorHandler_Error(ctx->hMainWnd, L"无法创建解压目录");
        return -1;
    }

    char* utf8Path = String_WcharToUtf8(path);
    char* utf8SavePath = String_WcharToUtf8(savePath);
    if (utf8Path && utf8SavePath) {
        if (xpkExtractAll(ctx->xpk, utf8SavePath) == 0) {
            ErrorHandler_Info(ctx->hMainWnd, L"解压成功");
        } else {
            ErrorHandler_ShowDetailed(ctx->hMainWnd, L"解压失败");
        }
    }
    if (utf8Path) free(utf8Path);
    if (utf8SavePath) free(utf8SavePath);

    return 0;
}

int ExtractHereModeMain(AppContext* ctx, const wchar_t* path)
{
    wchar_t savePath[MAX_PATH_W];
    wcscpy_s(savePath, MAX_PATH_W, ctx->currentDir);

    char* utf8Path = String_WcharToUtf8(path);
    char* utf8SavePath = String_WcharToUtf8(savePath);
    if (utf8Path && utf8SavePath) {
        if (xpkExtractAll(ctx->xpk, utf8SavePath) == 0) {
            ErrorHandler_Info(ctx->hMainWnd, L"解压成功");
        } else {
            ErrorHandler_ShowDetailed(ctx->hMainWnd, L"解压失败");
        }
    }
    if (utf8Path) free(utf8Path);
    if (utf8SavePath) free(utf8SavePath);

    return 0;
}

int VerifyModeMain(AppContext* ctx, const wchar_t* path)
{
    if (VerifyOps_VerifyAll(ctx) == 0) {
        ErrorHandler_Info(ctx->hMainWnd, L"验证成功");
    } else {
        ErrorHandler_ShowDetailed(ctx->hMainWnd, L"验证失败");
    }

    return 0;
}

int PropertiesModeMain(AppContext* ctx, const wchar_t* path)
{
    return PackageOps_GetProperties(ctx);
}

#endif
