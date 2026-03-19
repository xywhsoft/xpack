#ifndef XPKGUI_PACKAGE_OPS_H
#define XPKGUI_PACKAGE_OPS_H

#include <xpkgui/define.h>

int PackageOps_Open(AppContext* ctx, const wchar_t* path)
{
    if (ctx->xpk != NULL) {
        xpkClose(ctx->xpk);
        ctx->xpk = NULL;
    }

    char* utf8Path = String_WcharToUtf8(path);
    if (!utf8Path) {
        ErrorHandler_ShowDetailed(ctx->hMainWnd, L"无法转换路径编码");
        return -1;
    }

    ctx->xpk = xpkOpen(utf8Path, 0, 0);
    free(utf8Path);

    if (ctx->xpk == NULL) {
        ErrorHandler_ShowDetailed(ctx->hMainWnd, L"无法打开压缩包文件");
        return -1;
    }

    wcscpy_s(ctx->xpkPath, MAX_PATH_W, path);
    History_Add(ctx, path);
    FileList_Refresh(ctx);
    Window_UpdateTitle(ctx);
    return 0;
}

int PackageOps_Close(AppContext* ctx)
{
    if (ctx->xpk != NULL) {
        xpkClose(ctx->xpk);
        ctx->xpk = NULL;
        ctx->xpkPath[0] = L'\0';
        ListView_DeleteAllItems(ctx->hFileList);
        Window_UpdateStatusBar(ctx);
        Window_UpdateTitle(ctx);
        return 0;
    }
    return -1;
}

int PackageOps_Save(AppContext* ctx)
{
    if (ctx->xpk != NULL) {
        if (xpkSave(ctx->xpk) == 0) {
            ErrorHandler_Info(ctx->hMainWnd, L"保存成功");
        } else {
            ErrorHandler_ShowDetailed(ctx->hMainWnd, L"保存失败");
        }
    } else {
        ErrorHandler_Error(ctx->hMainWnd, L"没有打开的压缩包");
    }
    return 0;
}

int PackageOps_CreateNew(AppContext* ctx)
{
    wchar_t path[MAX_PATH_W] = {0};
    int solidMode = ctx->settings.solidMode;
    int pkgType = ctx->settings.defaultPkgType;

    if (Dialog_NewPackage(ctx->hMainWnd, path, &solidMode, &pkgType) != IDOK) {
        return -1;
    }

    char* utf8Path = String_WcharToUtf8(path);
    if (!utf8Path) {
        ErrorHandler_ShowDetailed(ctx->hMainWnd, L"无法转换路径编码");
        return -1;
    }

    ctx->xpk = xpkOpen(utf8Path, 0, 0);
    free(utf8Path);

    if (ctx->xpk == NULL) {
        ErrorHandler_ShowDetailed(ctx->hMainWnd, L"无法创建压缩包");
        return -1;
    }

    if (xpkTypeSet(ctx->xpk, pkgType) != 0) {
        ErrorHandler_ShowDetailed(ctx->hMainWnd, L"无法设置包类型");
        xpkClose(ctx->xpk);
        ctx->xpk = NULL;
        return -1;
    }

    if (xpkSolidModeSet(ctx->xpk, solidMode) != 0) {
        ErrorHandler_ShowDetailed(ctx->hMainWnd, L"无法设置固实模式");
        xpkClose(ctx->xpk);
        ctx->xpk = NULL;
        return -1;
    }

    wcscpy_s(ctx->xpkPath, MAX_PATH_W, path);
    ctx->settings.solidMode = solidMode;
    ctx->settings.defaultPkgType = pkgType;
    FileList_Refresh(ctx);
    Window_UpdateTitle(ctx);
    return 0;
}

int PackageOps_Rebuild(AppContext* ctx)
{
    if (ctx->xpk == NULL) {
        ErrorHandler_Error(ctx->hMainWnd, L"没有打开的压缩包");
        return -1;
    }

    if (xpkRebuild(ctx->xpk) == 0) {
        ErrorHandler_Info(ctx->hMainWnd, L"重建成功");
        FileList_Refresh(ctx);
    } else {
        ErrorHandler_ShowDetailed(ctx->hMainWnd, L"重建失败");
    }

    return 0;
}

int PackageOps_GetProperties(AppContext* ctx)
{
    if (ctx->xpk == NULL) {
        ErrorHandler_Error(ctx->hMainWnd, L"没有打开的压缩包");
        return -1;
    }

    int type = xpkType(ctx->xpk);
    uint32_t count = xpkCount(ctx->xpk);
    uint64_t totalSize = 0;
    uint64_t totalPacked = 0;

    for (uint32_t i = 0; i < count; i++) {
        totalSize += xpkInfoSize(ctx->xpk, i);
        totalPacked += xpkInfoPacked(ctx->xpk, i);
    }

    wchar_t msg[512];
    swprintf_s(msg, sizeof(msg) / sizeof(wchar_t), 
        L"压缩包类型: %s\n文件数: %u\n原始大小: %llu\n压缩后大小: %llu\n压缩率: %.1f%%",
        type == XPK_TYPE_WIN32 ? L"Windows" : (type == XPK_TYPE_LINUX ? L"Linux" : L"未知"),
        count, totalSize, totalPacked, totalSize > 0 ? (double)totalPacked / totalSize * 100 : 0);

    MessageBoxW(ctx->hMainWnd, msg, L"压缩包属性", MB_OK | MB_ICONINFORMATION);
    return 0;
}

int PackageOps_ToggleSolidMode(AppContext* ctx)
{
    if (ctx->xpk == NULL) {
        ErrorHandler_Error(ctx->hMainWnd, L"没有打开的压缩包");
        return -1;
    }

    int solidMode = xpkSolidMode(ctx->xpk);
    if (xpkSolidModeSet(ctx->xpk, !solidMode) != 0) {
        ErrorHandler_ShowDetailed(ctx->hMainWnd, L"设置固实模式失败");
        return -1;
    }

    ctx->settings.solidMode = !solidMode;
    FileList_Refresh(ctx);
    Window_UpdateStatusBar(ctx);
    return 0;
}

int PackageOps_ToggleVolumeMode(AppContext* ctx)
{
    if (ctx->xpk == NULL) {
        ErrorHandler_Error(ctx->hMainWnd, L"没有打开的压缩包");
        return -1;
    }

    int volumeMode = xpkVolumeMode(ctx->xpk);
    if (xpkVolumeModeSet(ctx->xpk, !volumeMode) != 0) {
        ErrorHandler_ShowDetailed(ctx->hMainWnd, L"设置分卷模式失败");
        return -1;
    }

    ctx->settings.volumeMode = !volumeMode;
    FileList_Refresh(ctx);
    Window_UpdateStatusBar(ctx);
    return 0;
}

int PackageOps_SetVolumeSize(AppContext* ctx)
{
    if (ctx->xpk == NULL) {
        ErrorHandler_Error(ctx->hMainWnd, L"没有打开的压缩包");
        return -1;
    }

    int level = 0;
    if (Dialog_CompressLevel(ctx->hMainWnd, &level) != IDOK) {
        return -1;
    }

    if (xpkVolumeSizeSet(ctx->xpk, (uint32_t)(1024 * 1024 * level)) != 0) {
        ErrorHandler_ShowDetailed(ctx->hMainWnd, L"设置分卷大小失败");
        return -1;
    }

    ctx->settings.volumeSize = (uint32_t)(1024 * 1024 * level);
    FileList_Refresh(ctx);
    Window_UpdateStatusBar(ctx);
    return 0;
}

int PackageOps_SetDiscCode(AppContext* ctx)
{
    if (ctx->xpk == NULL) {
        ErrorHandler_Error(ctx->hMainWnd, L"没有打开的压缩包");
        return -1;
    }

    wchar_t code[64] = {0};
    if (Dialog_InputBox(ctx->hMainWnd, L"输入识别代码", L"请输入识别代码:", code, 64) != IDOK) {
        return -1;
    }

    if (wcslen(code) == 0) {
        return -1;
    }

    uint32_t discCode = (uint32_t)wcstoul(code, NULL, 10);
    if (xpkDiscCodeSet(ctx->xpk, discCode) != 0) {
        ErrorHandler_ShowDetailed(ctx->hMainWnd, L"设置识别代码失败");
        return -1;
    }

    FileList_Refresh(ctx);
    Window_UpdateStatusBar(ctx);
    return 0;
}

int PackageOps_SelectByPattern(AppContext* ctx)
{
    if (ctx->xpk == NULL) {
        ErrorHandler_Error(ctx->hMainWnd, L"没有打开的压缩包");
        return -1;
    }

    wchar_t pattern[MAX_PATH_W] = L"*";
    int operation = 0;

    if (Dialog_PatternSelect(ctx->hMainWnd, pattern, &operation) != IDOK) {
        return -1;
    }

    if (wcslen(pattern) == 0) {
        return -1;
    }

    char* utf8Pattern = String_WcharToUtf8(pattern);
    if (!utf8Pattern) {
        ErrorHandler_ShowDetailed(ctx->hMainWnd, L"无法转换模式");
        return -1;
    }

    uint32_t count = xpkEachMatch(ctx->xpk, utf8Pattern, NULL, NULL);
    free(utf8Pattern);

    if (count == 0) {
        ErrorHandler_ShowDetailed(ctx->hMainWnd, L"模式匹配失败");
        return -1;
    }

    if (operation == 0) {
        wchar_t extractPath[MAX_PATH_W] = {0};
        if (Dialog_BrowseForDirectory(ctx->hMainWnd, extractPath, L"选择解压目录") == 1) {
            char* utf8Path = String_WcharToUtf8(extractPath);
            if (utf8Path) {
                xpkExtractAll(ctx->xpk, utf8Path);
                free(utf8Path);
                ErrorHandler_Info(ctx->hMainWnd, L"解压成功");
                FileList_Refresh(ctx);
                Window_UpdateStatusBar(ctx);
                return 0;
            }
        }
    } else if (operation == 1) {
        ErrorHandler_ShowDetailed(ctx->hMainWnd, L"删除功能暂未实现");
        FileList_Refresh(ctx);
        Window_UpdateStatusBar(ctx);
        return 0;
    }

    return -1;
}

#endif
