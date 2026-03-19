#ifndef XPKGUI_STRING_UTILS_H
#define XPKGUI_STRING_UTILS_H

#include <xpkgui/define.h>

wchar_t* String_Utf8ToWchar(const char* utf8Str)
{
    if (!utf8Str) return NULL;
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, NULL, 0);
    if (len == 0) return NULL;
    wchar_t* wstr = (wchar_t*)malloc(len * sizeof(wchar_t));
    if (!wstr) return NULL;
    MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, wstr, len);
    return wstr;
}

char* String_WcharToUtf8(const wchar_t* wstr)
{
    if (!wstr) return NULL;
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    if (len == 0) return NULL;
    char* utf8Str = (char*)malloc(len);
    if (!utf8Str) return NULL;
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, utf8Str, len, NULL, NULL);
    return utf8Str;
}

#endif
