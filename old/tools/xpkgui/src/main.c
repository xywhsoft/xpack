#include <windows.h>
#include <stdio.h>

#include "xpkgui/define.h"

#include "core/app.h"
#include "core/window.h"
#include "core/mode_main.h"
#include "ui/dialogs/dialog_base.h"
#include "ui/widgets/file_list.h"
#include "ui/widgets/status_bar.h"
#include "ui/menus/menu_handler.h"
#include "operations/package_ops.h"
#include "operations/file_ops.h"
#include "operations/extract_ops.h"
#include "operations/verify_ops.h"
#include "operations/dir_ops.h"
#include "config/settings.h"
#include "config/history.h"
#include "utils/string_utils.h"
#include "utils/format_utils.h"
#include "utils/error_handler.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    AppContext ctx = {0};

    ctx.hInstance = hInstance;
    ctx.settings.defaultCompLevel = 7;
    ctx.settings.defaultPkgType = XPK_TYPE_WIN32;
    ctx.settings.solidMode = 0;
    ctx.settings.volumeMode = 0;
    ctx.settings.volumeSize = 0;
    ctx.settings.confirmDelete = 1;
    ctx.settings.overwriteFiles = 0;
    ctx.settings.showStatusBar = 1;
    ctx.settings.showGridLines = 1;
    ctx.settings.windowWidth = 900;
    ctx.settings.windowHeight = 600;
    ctx.settings.windowMaximized = 0;

    int cmdResult = App_ProcessCommandLine(&ctx, lpCmdLine);
    if (cmdResult >= 0) {
        return cmdResult;
    }

    if (App_Initialize(&ctx) != 0) {
        MessageBoxW(NULL, L"初始化失败", L"错误", MB_OK | MB_ICONERROR);
        return 1;
    }

    App_Run(&ctx);
    App_Cleanup(&ctx);

    return 0;
}
