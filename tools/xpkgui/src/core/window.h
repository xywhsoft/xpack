#ifndef XPKGUI_WINDOW_H
#define XPKGUI_WINDOW_H

#include <xpkgui/define.h>
#include "../resources/resource.h"

LRESULT CALLBACK Window_Proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    AppContext* ctx = (AppContext*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    switch (message) {
        case WM_CREATE:
            {
                CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
                ctx = (AppContext*)cs->lpCreateParams;
                SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)ctx);
                
                HWND tempWnd = ctx->hMainWnd;
                ctx->hMainWnd = hwnd;
                
                if (!Window_InitializeControls(ctx)) {
                    ctx->hMainWnd = tempWnd;
                    return -1;
                }
                
                if (wcslen(ctx->commandPath) > 0) {
                    PostMessage(hwnd, WM_USER + 100, ctx->commandMode, 0);
                }
            }
            break;

        case WM_SIZE:
            {
                RECT rcClient, rcStatus;
                
                GetClientRect(hwnd, &rcClient);
                
                if (ctx->hStatusBar) {
                    GetWindowRect(ctx->hStatusBar, &rcStatus);
                    int statusHeight = rcStatus.bottom - rcStatus.top;
                    
                    MoveWindow(ctx->hFileList, 0, 0, rcClient.right, rcClient.bottom - statusHeight, TRUE);
                    MoveWindow(ctx->hStatusBar, 0, rcClient.bottom - statusHeight, rcClient.right, statusHeight, TRUE);
                } else if (ctx->hFileList) {
                    MoveWindow(ctx->hFileList, 0, 0, rcClient.right, rcClient.bottom, TRUE);
                }
                
                if (wParam != SIZE_MINIMIZED) {
                    RECT rcWindow;
                    GetWindowRect(hwnd, &rcWindow);
                    ctx->settings.windowWidth = rcWindow.right - rcWindow.left;
                    ctx->settings.windowHeight = rcWindow.bottom - rcWindow.top;
                }
            }
            break;

        case WM_SYSCOMMAND:
            if (wParam == SC_MAXIMIZE) {
                ctx->settings.windowMaximized = 1;
            } else if (wParam == SC_RESTORE) {
                ctx->settings.windowMaximized = 0;
            }
            break;

        case WM_DROPFILES:
            MenuHandler_HandleDropFiles(ctx, (HDROP)wParam);
            break;

        case WM_COMMAND:
            MenuHandler_HandleCommand(ctx, LOWORD(wParam));
            break;

        case WM_NOTIFY:
            MenuHandler_HandleNotify(ctx, (LPNMHDR)lParam);
            break;

        case WM_USER + 100:
            if (wParam == CMD_ADD || wParam == CMD_ADD_AUTO) {
                MenuHandler_HandleAddFiles(ctx);
            } else {
                PackageOps_Open(ctx, ctx->commandPath);
            }
            break;

        case WM_CLOSE:
            if (ctx->xpk != NULL) {
                xpkClose(ctx->xpk);
                ctx->xpk = NULL;
            }
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

HWND Window_Create(AppContext* ctx)
{
    HMENU hMenu = LoadMenuW(ctx->hInstance, MAKEINTRESOURCEW(IDR_MAINMENU));
    if (!hMenu) {
        return NULL;
    }
    
    HWND hwnd = CreateWindowExW(
        0,
        XPKGUI_CLASS_NAME,
        XPKGUI_WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW | WS_SYSMENU,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        ctx->settings.windowWidth, ctx->settings.windowHeight,
        NULL,
        hMenu,
        ctx->hInstance,
        ctx
    );

    if (!hwnd) {
        return NULL;
    }
    
    ctx->hMainWnd = hwnd;
    
    if (ctx->settings.windowMaximized) {
        ShowWindow(hwnd, SW_MAXIMIZE);
    } else {
        ShowWindow(hwnd, SW_SHOW);
    }
    
    UpdateWindow(hwnd);
    
    return hwnd;
}

int Window_InitializeControls(AppContext* ctx)
{
    RECT rc;
    GetClientRect(ctx->hMainWnd, &rc);

    ctx->hFileList = FileList_Create(ctx);
    if (!ctx->hFileList) {
        return FALSE;
    }

    ctx->hStatusBar = StatusBar_Create(ctx);
    if (!ctx->hStatusBar) {
        return FALSE;
    }

    Window_UpdateStatusBar(ctx);
    
    return TRUE;
}

void Window_UpdateStatusBar(AppContext* ctx)
{
    StatusBar_Update(ctx);
}

void Window_UpdateTitle(AppContext* ctx)
{
    wchar_t title[MAX_PATH_W + 64];

    if (ctx->xpk != NULL) {
        swprintf_s(title, sizeof(title) / sizeof(wchar_t), L"xpkgui - %s", ctx->xpkPath);
    } else {
        wcscpy_s(title, sizeof(title) / sizeof(wchar_t), XPKGUI_WINDOW_TITLE);
    }

    SetWindowTextW(ctx->hMainWnd, title);
}

#endif
