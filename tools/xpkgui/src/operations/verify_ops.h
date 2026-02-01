#ifndef XPKGUI_VERIFY_OPS_H
#define XPKGUI_VERIFY_OPS_H

#include <xpkgui/define.h>

int VerifyOps_VerifyAll(AppContext* ctx)
{
    if (ctx->xpk == NULL) {
        ErrorHandler_Error(ctx->hMainWnd, L"没有打开的压缩包");
        return -1;
    }

    if (xpkVerifyAll(ctx->xpk) == 0) {
        ErrorHandler_Info(ctx->hMainWnd, L"验证成功");
    } else {
        ErrorHandler_ShowDetailed(ctx->hMainWnd, L"验证失败");
    }

    return 0;
}

int VerifyOps_TestPackage(AppContext* ctx)
{
    if (ctx->xpk == NULL) {
        ErrorHandler_Error(ctx->hMainWnd, L"没有打开的压缩包");
        return -1;
    }

    int type = xpkType(ctx->xpk);
    uint32_t count = xpkCount(ctx->xpk);
    int verified = 0;
    int failed = 0;

    for (uint32_t i = 0; i < count; i++) {
        if (xpkVerify(ctx->xpk, i)) {
            verified++;
        } else {
            failed++;
        }
    }

    wchar_t msg[256];
    swprintf_s(msg, sizeof(msg) / sizeof(wchar_t), L"测试完成: 成功 %u, 失败 %u", verified, failed);
    MessageBoxW(ctx->hMainWnd, msg, L"测试结果", MB_OK | MB_ICONINFORMATION);

    return 0;
}

#endif
