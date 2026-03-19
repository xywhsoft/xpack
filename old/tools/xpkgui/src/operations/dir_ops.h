#ifndef XPKGUI_DIR_OPS_H
#define XPKGUI_DIR_OPS_H

#include <xpkgui/define.h>

int DirOps_AddDirectory(AppContext* ctx)
{
    if (ctx->xpk == NULL) {
        ErrorHandler_Error(ctx->hMainWnd, L"请先打开或创建一个压缩包");
        return -1;
    }

    int packType = xpkType(ctx->xpk);
    if (packType != XPK_TYPE_WIN32 && packType != XPK_TYPE_LINUX) {
        ErrorHandler_Error(ctx->hMainWnd, L"当前模式不支持文件路径操作，请使用 Win32 或 Linux 类型");
        return -1;
    }

    int solidMode = xpkSolidMode(ctx->xpk);
    if (solidMode) {
        ErrorHandler_Error(ctx->hMainWnd, L"固实压缩包不支持追加文件或目录");
        return -1;
    }

    wchar_t dirPath[MAX_PATH_W] = {0};
    int result = Dialog_BrowseForDirectory(ctx->hMainWnd, dirPath, L"选择要添加的目录");
    if (result == 0) {
        return -1;
    }
    if (result == -1) {
        ErrorHandler_Error(ctx->hMainWnd, L"无法打开目录选择对话框");
        return -1;
    }

    char* utf8Path = String_WcharToUtf8(dirPath);
    if (!utf8Path) {
        ErrorHandler_ShowDetailed(ctx->hMainWnd, L"无法转换路径编码");
        return -1;
    }

    int count = xpkAppendDir(ctx->xpk, utf8Path, "*", ctx->settings.defaultCompLevel, 1);
    free(utf8Path);

    if (count > 0) {
        FileList_Refresh(ctx);
        wchar_t msg[128];
        swprintf_s(msg, sizeof(msg) / sizeof(wchar_t), L"添加目录成功，共 %d 个文件", count);
        ErrorHandler_Info(ctx->hMainWnd, msg);
        return 0;
    }

    ErrorHandler_ShowDetailed(ctx->hMainWnd, L"添加目录失败");
    return -1;
}

#endif
