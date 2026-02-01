#ifndef XPKGUI_HISTORY_H
#define XPKGUI_HISTORY_H

#include <xpkgui/define.h>

void History_Load(AppContext* ctx)
{
    wchar_t path[MAX_PATH_W];
    SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, path);
    wcscat_s(path, MAX_PATH_W, L"\\xPack\\history.ini");

    FILE* f = _wfopen(path, L"r");
    if (f) {
        wchar_t line[MAX_PATH_W];
        while (fgetws(line, sizeof(line) / sizeof(wchar_t), f)) {
            if (wcsstr(line, L"Path") == line) {
                int index;
                swscanf(line, L"Path%d=%s", &index, ctx->history[index].path);
            } else if (wcsstr(line, L"Count=")) {
                swscanf(line, L"Count=%d", &ctx->historyCount);
            }
        }
        fclose(f);
    }
}

void History_Add(AppContext* ctx, const wchar_t* path)
{
    if (!path || wcslen(path) == 0) {
        return;
    }

    for (int i = 0; i < ctx->historyCount; i++) {
        if (_wcsicmp(ctx->history[i].path, path) == 0) {
            memmove(&ctx->history[i], &ctx->history[i + 1],
                (ctx->historyCount - i - 1) * sizeof(HistoryItem));
            ctx->historyCount--;
            break;
        }
    }

    if (ctx->historyCount >= MAX_RECENT_FILES) {
        memmove(&ctx->history[0], &ctx->history[1],
            (ctx->historyCount - 1) * sizeof(HistoryItem));
        ctx->historyCount--;
    }

    wcscpy_s(ctx->history[ctx->historyCount].path, MAX_PATH_W, path);
    ctx->history[ctx->historyCount].timestamp = time(NULL);
    ctx->historyCount++;

    wchar_t settingsPath[MAX_PATH_W];
    SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, settingsPath);
    wcscat_s(settingsPath, MAX_PATH_W, L"\\xPack");

    CreateDirectoryW(settingsPath, NULL);
    wcscat_s(settingsPath, MAX_PATH_W, L"\\history.ini");

    FILE* f = _wfopen(settingsPath, L"w");
    if (f) {
        fwprintf(f, L"[History]\n");
        fwprintf(f, L"Count=%d\n", ctx->historyCount);
        for (int i = 0; i < ctx->historyCount; i++) {
            fwprintf(f, L"Path%d=%s\n", i, ctx->history[i].path);
        }
        fclose(f);
    }
}

#endif
