#ifndef XPKGUI_ERROR_HANDLER_H
#define XPKGUI_ERROR_HANDLER_H

#include <xpkgui/define.h>

void ErrorHandler_Handle(int code, const char* message)
{
}

void ErrorHandler_Error(HWND hwnd, const wchar_t* msg)
{
    MessageBoxW(hwnd, msg, L"错误", MB_OK | MB_ICONERROR);
}

void ErrorHandler_Info(HWND hwnd, const wchar_t* msg)
{
    MessageBoxW(hwnd, msg, L"信息", MB_OK | MB_ICONINFORMATION);
}

void ErrorHandler_ShowDetailed(HWND hwnd, const wchar_t* context)
{
    int lastError = xpkLastError();
    const char* lastErrorMsg = xpkLastErrorMsg();

    wchar_t msg[512];
    if (lastErrorMsg && strlen(lastErrorMsg) > 0) {
        wchar_t* wErrorMsg = String_Utf8ToWchar(lastErrorMsg);
        if (wErrorMsg) {
            swprintf_s(msg, sizeof(msg) / sizeof(wchar_t), L"%s\n\n错误代码: %d\n错误信息: %s",
                context, lastError, wErrorMsg);
            free(wErrorMsg);
        } else {
            swprintf_s(msg, sizeof(msg) / sizeof(wchar_t), L"%s\n\n错误代码: %d", context, lastError);
        }
    } else {
        swprintf_s(msg, sizeof(msg) / sizeof(wchar_t), L"%s\n\n错误代码: %d", context, lastError);
    }
    MessageBoxW(hwnd, msg, L"错误", MB_OK | MB_ICONERROR);
}

#endif
