#ifndef XPKGUI_SETTINGS_H
#define XPKGUI_SETTINGS_H

#include <xpkgui/define.h>

void Settings_Load(Settings* settings)
{
    wchar_t path[MAX_PATH_W];
    SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, path);
    wcscat_s(path, MAX_PATH_W, L"\\xPack\\settings.ini");

    FILE* f = _wfopen(path, L"r");
    if (f) {
        wchar_t line[256];
        while (fgetws(line, sizeof(line) / sizeof(wchar_t), f)) {
            if (wcsstr(line, L"DefaultCompLevel=")) {
                swscanf(line, L"DefaultCompLevel=%d", &settings->defaultCompLevel);
            } else if (wcsstr(line, L"DefaultPkgType=")) {
                swscanf(line, L"DefaultPkgType=%d", &settings->defaultPkgType);
            } else if (wcsstr(line, L"SolidMode=")) {
                swscanf(line, L"SolidMode=%d", &settings->solidMode);
            } else if (wcsstr(line, L"VolumeMode=")) {
                swscanf(line, L"VolumeMode=%d", &settings->volumeMode);
            } else if (wcsstr(line, L"VolumeSize=")) {
                swscanf(line, L"VolumeSize=%u", &settings->volumeSize);
            } else if (wcsstr(line, L"ConfirmDelete=")) {
                swscanf(line, L"ConfirmDelete=%d", &settings->confirmDelete);
            } else if (wcsstr(line, L"OverwriteFiles=")) {
                swscanf(line, L"OverwriteFiles=%d", &settings->overwriteFiles);
            } else if (wcsstr(line, L"ShowStatusBar=")) {
                swscanf(line, L"ShowStatusBar=%d", &settings->showStatusBar);
            } else if (wcsstr(line, L"ShowGridLines=")) {
                swscanf(line, L"ShowGridLines=%d", &settings->showGridLines);
            } else if (wcsstr(line, L"WindowWidth=")) {
                swscanf(line, L"WindowWidth=%d", &settings->windowWidth);
            } else if (wcsstr(line, L"WindowHeight=")) {
                swscanf(line, L"WindowHeight=%d", &settings->windowHeight);
            } else if (wcsstr(line, L"WindowMaximized=")) {
                swscanf(line, L"WindowMaximized=%d", &settings->windowMaximized);
            }
        }
        fclose(f);
    }
}

void Settings_Save(Settings* settings)
{
    wchar_t path[MAX_PATH_W];
    SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, path);
    wcscat_s(path, MAX_PATH_W, L"\\xPack");

    CreateDirectoryW(path, NULL);
    wcscat_s(path, MAX_PATH_W, L"\\settings.ini");

    FILE* f = _wfopen(path, L"w");
    if (f) {
        fwprintf(f, L"[Settings]\n");
        fwprintf(f, L"DefaultCompLevel=%d\n", settings->defaultCompLevel);
        fwprintf(f, L"DefaultPkgType=%d\n", settings->defaultPkgType);
        fwprintf(f, L"SolidMode=%d\n", settings->solidMode);
        fwprintf(f, L"VolumeMode=%d\n", settings->volumeMode);
        fwprintf(f, L"VolumeSize=%u\n", settings->volumeSize);
        fwprintf(f, L"ConfirmDelete=%d\n", settings->confirmDelete);
        fwprintf(f, L"OverwriteFiles=%d\n", settings->overwriteFiles);
        fwprintf(f, L"ShowStatusBar=%d\n", settings->showStatusBar);
        fwprintf(f, L"ShowGridLines=%d\n", settings->showGridLines);
        fwprintf(f, L"[Window]\n");
        fwprintf(f, L"WindowWidth=%d\n", settings->windowWidth);
        fwprintf(f, L"WindowHeight=%d\n", settings->windowHeight);
        fwprintf(f, L"WindowMaximized=%d\n", settings->windowMaximized);
        fclose(f);
    }
}

#endif
