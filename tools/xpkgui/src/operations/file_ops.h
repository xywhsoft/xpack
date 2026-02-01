#ifndef XPKGUI_FILE_OPS_H
#define XPKGUI_FILE_OPS_H

#include <xpkgui/define.h>

int FileOps_Delete(AppContext* ctx)
{
    if (ctx->xpk == NULL) {
        ErrorHandler_Error(ctx->hMainWnd, L"没有打开的压缩包");
        return -1;
    }

    int selected = FileList_GetSelectedIndex(ctx);
    if (selected < 0) {
        ErrorHandler_Error(ctx->hMainWnd, L"请先选择要删除的文件");
        return -1;
    }

    if (!ctx->settings.confirmDelete) {
        wchar_t path[MAX_PATH_W];
        if (FileList_GetSelectedPath(ctx, path, MAX_PATH_W) == 0) {
            return -1;
        }

        wchar_t msg[256];
        swprintf_s(msg, sizeof(msg) / sizeof(wchar_t), L"确定要删除文件: %s?", path);
        if (MessageBoxW(ctx->hMainWnd, msg, L"确认删除", MB_YESNO | MB_ICONQUESTION) == IDNO) {
            return -1;
        }
    }

    wchar_t path[MAX_PATH_W];
    if (FileList_GetSelectedPath(ctx, path, MAX_PATH_W) == 0) {
        return -1;
    }

    char* utf8Path = String_WcharToUtf8(path);
    if (utf8Path) {
        int packType = xpkType(ctx->xpk);
        if (packType == XPK_TYPE_WIN32 || packType == XPK_TYPE_LINUX) {
            if (xpkPathRemove(ctx->xpk, utf8Path) == 0) {
                ErrorHandler_Info(ctx->hMainWnd, L"删除成功");
                FileList_Refresh(ctx);
            } else {
                ErrorHandler_ShowDetailed(ctx->hMainWnd, L"删除失败");
            }
        } else {
            ErrorHandler_Error(ctx->hMainWnd, L"当前模式不支持删除操作");
        }
        free(utf8Path);
    }

    return 0;
}

int FileOps_Rename(AppContext* ctx)
{
    if (ctx->xpk == NULL) {
        ErrorHandler_Error(ctx->hMainWnd, L"没有打开的压缩包");
        return -1;
    }

    int selected = FileList_GetSelectedIndex(ctx);
    if (selected < 0) {
        ErrorHandler_Error(ctx->hMainWnd, L"请先选择要重命名的文件");
        return -1;
    }

    int packType = xpkType(ctx->xpk);
    if (packType != XPK_TYPE_WIN32 && packType != XPK_TYPE_LINUX) {
        ErrorHandler_Error(ctx->hMainWnd, L"当前模式不支持重命名操作");
        return -1;
    }

    wchar_t oldPath[MAX_PATH_W];
    if (FileList_GetSelectedPath(ctx, oldPath, MAX_PATH_W) == 0) {
        return -1;
    }

    wchar_t newPath[MAX_PATH_W] = {0};
    if (Dialog_InputBox(ctx->hMainWnd, L"重命名", L"请输入新文件名:", newPath, MAX_PATH_W) != 1) {
        return -1;
    }

    if (wcslen(newPath) == 0) {
        return -1;
    }

    char* utf8OldPath = String_WcharToUtf8(oldPath);
    char* utf8NewPath = String_WcharToUtf8(newPath);
    if (utf8OldPath && utf8NewPath) {
        ErrorHandler_ShowDetailed(ctx->hMainWnd, L"重命名功能暂未实现");
        free(utf8OldPath);
        free(utf8NewPath);
    }

    return 0;
}

#endif
