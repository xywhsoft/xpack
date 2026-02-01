#ifndef XPKGUI_APP_H
#define XPKGUI_APP_H

#include <xpkgui/define.h>

static int RegisterWindowClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wc;
    
    memset(&wc, 0, sizeof(WNDCLASSEXW));
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = Window_Proc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = XPKGUI_CLASS_NAME;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    
    BOOL result = RegisterClassExW(&wc);
    return result != 0;
}

int App_Initialize(AppContext* ctx)
{
    INITCOMMONCONTROLSEX icc;
    
    ctx->initialized = 0;
    
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES;
    if (!InitCommonControlsEx(&icc)) {
        return -1;
    }
    
    if (!RegisterWindowClass(ctx->hInstance)) {
        return -1;
    }
    
    Settings_Load(&ctx->settings);
    History_Load(ctx);
    
    ctx->hMainWnd = Window_Create(ctx);
    if (!ctx->hMainWnd) {
        return -1;
    }
    
    GetCurrentDirectoryW(MAX_PATH_W, ctx->currentDir);
    DragAcceptFiles(ctx->hMainWnd, TRUE);
    
    ctx->initialized = 1;
    return 0;
}

void App_Run(AppContext* ctx)
{
    MSG msg;
    
    if (!ctx->initialized) {
        return;
    }
    
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    Settings_Save(&ctx->settings);
}

void App_Cleanup(AppContext* ctx)
{
    if (ctx->xpk != NULL) {
        xpkClose(ctx->xpk);
        ctx->xpk = NULL;
    }
    
    if (ctx->hShell32) {
        FreeLibrary(ctx->hShell32);
        ctx->hShell32 = NULL;
    }
}

int App_ProcessCommandLine(AppContext* ctx, LPWSTR lpCmdLine)
{
    int argc;
    LPWSTR* argv = CommandLineToArgvW(lpCmdLine, &argc);
    
    if (argc < 2) {
        LocalFree(argv);
        return -1;
    }
    
    wchar_t cmd[64];
    wchar_t path[MAX_PATH_W] = {0};
    
    wcscpy_s(cmd, 64, argv[1]);
    if (argc >= 3) {
        wcscpy_s(path, MAX_PATH_W, argv[2]);
    }
    
    if (wcscmp(cmd, L"-extract") == 0) {
        LocalFree(argv);
        return ExtractModeMain(ctx, path);
    } else if (wcscmp(cmd, L"-extract_here") == 0) {
        LocalFree(argv);
        return ExtractHereModeMain(ctx, path);
    } else if (wcscmp(cmd, L"-add") == 0) {
        LocalFree(argv);
        ctx->commandMode = CMD_ADD;
        wcscpy_s(ctx->commandPath, MAX_PATH_W, path);
        return -1;
    } else if (wcscmp(cmd, L"-add_auto") == 0) {
        LocalFree(argv);
        ctx->commandMode = CMD_ADD_AUTO;
        wcscpy_s(ctx->commandPath, MAX_PATH_W, path);
        return -1;
    } else if (wcscmp(cmd, L"-verify") == 0) {
        LocalFree(argv);
        return VerifyModeMain(ctx, path);
    } else if (wcscmp(cmd, L"-properties") == 0) {
        LocalFree(argv);
        return PropertiesModeMain(ctx, path);
    } else {
        LocalFree(argv);
        wcscpy_s(ctx->commandPath, MAX_PATH_W, cmd);
        return -1;
    }
    
    LocalFree(argv);
    return -1;
}

#endif
