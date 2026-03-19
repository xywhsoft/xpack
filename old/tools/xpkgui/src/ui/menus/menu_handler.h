#ifndef XPKGUI_MENU_HANDLER_H
#define XPKGUI_MENU_HANDLER_H

#include <xpkgui/define.h>

void MenuHandler_HandleCommand(AppContext* ctx, WORD commandId)
{
    switch (commandId) {
        case ID_FILE_NEW:
            PackageOps_CreateNew(ctx);
            break;

        case ID_FILE_OPEN:
            {
                wchar_t path[MAX_PATH_W] = {0};
                OPENFILENAMEW ofn = {0};
                ofn.lStructSize = sizeof(OPENFILENAMEW);
                ofn.hwndOwner = ctx->hMainWnd;
                ofn.lpstrFilter = L"xPack 文件 (*.xpk)\0*.xpk\0所有文件 (*.*)\0*.*\0";
                ofn.lpstrFile = path;
                ofn.nMaxFile = MAX_PATH_W;
                ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
                ofn.lpstrDefExt = L"xpk";

                if (GetOpenFileNameW(&ofn)) {
                    PackageOps_Open(ctx, path);
                }
            }
            break;

        case ID_FILE_CLOSE:
            PackageOps_Close(ctx);
            break;

        case ID_FILE_SAVE:
            PackageOps_Save(ctx);
            break;

        case ID_FILE_EXIT:
            PostMessage(ctx->hMainWnd, WM_CLOSE, 0, 0);
            break;

        case ID_FILE_ADD:
            MenuHandler_HandleAddFiles(ctx);
            break;

        case ID_FILE_ADDDIR:
            DirOps_AddDirectory(ctx);
            break;

        case ID_FILE_EXTRACT:
            ExtractOps_ExtractSelected(ctx);
            break;

        case ID_FILE_DELETE:
            FileOps_Delete(ctx);
            break;

        case ID_FILE_RENAME:
            FileOps_Rename(ctx);
            break;

        case ID_FILE_REBUILD:
            PackageOps_Rebuild(ctx);
            break;

        case ID_FILE_PROPERTIES:
            PackageOps_GetProperties(ctx);
            break;

        case ID_VIEW_REFRESH:
            FileList_Refresh(ctx);
            break;

        case ID_TOOLS_VERIFY:
            VerifyOps_VerifyAll(ctx);
            break;

        case ID_TOOLS_TEST:
            VerifyOps_TestPackage(ctx);
            break;

        case ID_TOOLS_EXTRACTALL:
            ExtractOps_ExtractAll(ctx);
            break;

        case ID_TOOLS_SOLIDMODE:
            PackageOps_ToggleSolidMode(ctx);
            break;

        case ID_TOOLS_VOLUME_MODE:
            PackageOps_ToggleVolumeMode(ctx);
            break;

        case ID_TOOLS_VOLUME_SIZE:
            PackageOps_SetVolumeSize(ctx);
            break;

        case ID_TOOLS_DISCCODE:
            PackageOps_SetDiscCode(ctx);
            break;

        case ID_TOOLS_PATTERN:
            PackageOps_SelectByPattern(ctx);
            break;

        case ID_HELP_ABOUT:
            MessageBoxW(ctx->hMainWnd, L"XPK GUI v1.0\nxPack 压缩包管理工具", L"关于", MB_OK | MB_ICONINFORMATION);
            break;
    }
}

void MenuHandler_HandleNotify(AppContext* ctx, LPNMHDR pnmh)
{
    if (pnmh->code == NM_DBLCLK && pnmh->idFrom == IDC_FILELIST) {
        ExtractOps_ExtractSelected(ctx);
    }
}

void MenuHandler_HandleDropFiles(AppContext* ctx, HDROP hDrop)
{
    UINT fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);

    if (fileCount > 0) {
        if (ctx->xpk == NULL) {
            PackageOps_CreateNew(ctx);
        }

        int packType = xpkType(ctx->xpk);
        int solidMode = xpkSolidMode(ctx->xpk);

        if (packType == XPK_TYPE_WIN32 || packType == XPK_TYPE_LINUX) {
            if (solidMode) {
                ErrorHandler_Error(ctx->hMainWnd, L"固实压缩包不支持追加文件");
                DragFinish(hDrop);
                return;
            }
        }

        for (UINT i = 0; i < fileCount; i++) {
            wchar_t path[MAX_PATH_W] = {0};
            DragQueryFileW(hDrop, i, path, MAX_PATH_W);
            char* utf8Path = String_WcharToUtf8(path);
            if (utf8Path) {
                if (packType == XPK_TYPE_WIN32 || packType == XPK_TYPE_LINUX) {
                    wchar_t* fileName = wcsrchr(path, L'\\');
                    if (fileName) {
                        fileName++;
                    } else {
                        fileName = path;
                    }
                    char* utf8FileName = String_WcharToUtf8(fileName);
                    if (utf8FileName) {
                        xpkPathAppendFile(ctx->xpk, utf8FileName, utf8Path, ctx->settings.defaultCompLevel);
                        free(utf8FileName);
                    }
                } else {
                    xpkAppendFile(ctx->xpk, utf8Path, ctx->settings.defaultCompLevel);
                }
                free(utf8Path);
            }
        }

        FileList_Refresh(ctx);
    }

    DragFinish(hDrop);
}

void MenuHandler_HandleAddFiles(AppContext* ctx)
{
    wchar_t files[4096] = {0};
    int fileCount = 0;

    if (Dialog_BrowseForFiles(ctx->hMainWnd, files, &fileCount, L"所有文件 (*.*)\0*.*\0") == 1) {
        if (ctx->xpk == NULL) {
            PackageOps_CreateNew(ctx);
        }

        int packType = xpkType(ctx->xpk);
        int solidMode = xpkSolidMode(ctx->xpk);

        if (packType == XPK_TYPE_WIN32 || packType == XPK_TYPE_LINUX) {
            if (solidMode) {
                ErrorHandler_Error(ctx->hMainWnd, L"固实压缩包不支持追加文件");
                return;
            }
        }

        wchar_t* p = files;

        if (fileCount == 1) {
            char* utf8Path = String_WcharToUtf8(p);
            if (utf8Path) {
                if (packType == XPK_TYPE_WIN32 || packType == XPK_TYPE_LINUX) {
                    wchar_t* fileName = wcsrchr(p, L'\\');
                    if (fileName) {
                        fileName++;
                    } else {
                        fileName = p;
                    }
                    char* utf8FileName = String_WcharToUtf8(fileName);
                    if (utf8FileName) {
                        xpkPathAppendFile(ctx->xpk, utf8FileName, utf8Path, ctx->settings.defaultCompLevel);
                        free(utf8FileName);
                    }
                } else {
                    xpkAppendFile(ctx->xpk, utf8Path, ctx->settings.defaultCompLevel);
                }
                free(utf8Path);
            }
        } else {
            wchar_t dirPath[MAX_PATH_W] = {0};
            wcscpy_s(dirPath, MAX_PATH_W, p);
            p += wcslen(p) + 1;

            for (int i = 0; i < fileCount; i++) {
                wchar_t fullPath[MAX_PATH_W] = {0};
                wcscpy_s(fullPath, MAX_PATH_W, dirPath);
                wcscat_s(fullPath, MAX_PATH_W, L"\\");
                wcscat_s(fullPath, MAX_PATH_W, p);

                char* utf8Path = String_WcharToUtf8(fullPath);
                if (utf8Path) {
                    if (packType == XPK_TYPE_WIN32 || packType == XPK_TYPE_LINUX) {
                        wchar_t* fileName = wcsrchr(p, L'\\');
                        if (fileName) {
                            fileName++;
                        } else {
                            fileName = p;
                        }
                        char* utf8FileName = String_WcharToUtf8(fileName);
                        if (utf8FileName) {
                            xpkPathAppendFile(ctx->xpk, utf8FileName, utf8Path, ctx->settings.defaultCompLevel);
                            free(utf8FileName);
                        }
                    } else {
                        xpkAppendFile(ctx->xpk, utf8Path, ctx->settings.defaultCompLevel);
                    }
                    free(utf8Path);
                }

                p += wcslen(p) + 1;
            }
        }

        FileList_Refresh(ctx);
    }
}

#endif
