#ifndef XPKGUI_FORMAT_UTILS_H
#define XPKGUI_FORMAT_UTILS_H

#include <xpkgui/define.h>

void FormatSize(uint64_t size, wchar_t* buf, int bufSize)
{
    if (size < 1024) {
        swprintf_s(buf, bufSize, L"%llu B", size);
    } else if (size < 1024 * 1024) {
        swprintf_s(buf, bufSize, L"%.2f KB", size / 1024.0);
    } else if (size < 1024 * 1024 * 1024) {
        swprintf_s(buf, bufSize, L"%.2f MB", size / (1024.0 * 1024.0));
    } else {
        swprintf_s(buf, bufSize, L"%.2f GB", size / (1024.0 * 1024.0 * 1024.0));
    }
}

void FormatTime(time_t t, wchar_t* buf, int bufSize)
{
    if (t == 0) {
        wcscpy_s(buf, bufSize, L"N/A");
        return;
    }
    struct tm* tm = localtime(&t);
    wchar_t timeStr[64];
    wcsftime(timeStr, bufSize, L"%Y-%m-%d %H:%M:%S", tm);
    wcscpy_s(buf, bufSize, timeStr);
}

const wchar_t* FormatUtils_GetFileTypeString(int type)
{
    static const wchar_t* typeStrings[] = {
        L"未知",
        L"二进制",
        L"文本",
        L"图像",
        L"音频",
        L"视频",
        L"归档",
        L"未知",
        L"未知",
        L"未知",
        L"未知",
        L"未知",
        L"未知",
        L"未知",
        L"未知",
        L"未知",
        L"目录"
    };
    if (type >= 0 && type < 16) {
        return typeStrings[type];
    }
    return L"未知";
}

#endif
