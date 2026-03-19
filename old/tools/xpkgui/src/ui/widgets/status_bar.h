#ifndef XPKGUI_STATUS_BAR_H
#define XPKGUI_STATUS_BAR_H

#include <xpkgui/define.h>

HWND StatusBar_Create(AppContext* ctx)
{
    RECT rc;
    GetClientRect(ctx->hMainWnd, &rc);

    HWND hStatus = CreateWindowExW(
        0,
        STATUSCLASSNAMEW,
        NULL,
        WS_CHILD | WS_VISIBLE,
        0, rc.bottom - 24,
        rc.right, 24,
        ctx->hMainWnd,
        (HMENU)IDC_STATUSBAR,
        ctx->hInstance,
        NULL
    );

    if (!ctx->settings.showStatusBar) {
        ShowWindow(hStatus, SW_HIDE);
    }

    return hStatus;
}

void StatusBar_Update(AppContext* ctx)
{
    wchar_t buf[512] = {0};

    if (ctx->xpk != NULL) {
        xpkStat stat;
        if (xpkStatGet(ctx->xpk, &stat) == 0) {
            wchar_t sizeBuf[64];
            FormatSize(stat.totalSize, sizeBuf, sizeof(sizeBuf) / sizeof(wchar_t));
            int solidMode = xpkSolidMode(ctx->xpk);
            int volumeMode = xpkVolumeMode(ctx->xpk);
            uint32_t volumeSize = xpkVolumeSize(ctx->xpk);
            wchar_t volumeInfo[128] = {0};

            if (volumeMode) {
                wchar_t vsizeBuf[64];
                FormatSize(volumeSize, vsizeBuf, sizeof(vsizeBuf) / sizeof(wchar_t));
                swprintf_s(volumeInfo, sizeof(volumeInfo) / sizeof(wchar_t), L" | 分卷: %s", vsizeBuf);
            }

            swprintf_s(buf, sizeof(buf) / sizeof(wchar_t), L"文件: %u | 总大小: %s | 压缩率: %.1f%% | %s%s",
                stat.fileCount, sizeBuf, stat.ratio * 100, solidMode ? L"固实" : L"独立", volumeInfo);
        }
    } else {
        wcscpy_s(buf, sizeof(buf) / sizeof(wchar_t), L"未打开压缩包");
    }

    SendMessageW(ctx->hStatusBar, WM_SETTEXT, 0, (LPARAM)buf);
}

#endif
