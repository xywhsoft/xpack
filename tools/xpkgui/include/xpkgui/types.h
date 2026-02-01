#ifndef XPKGUI_TYPES_H
#define XPKGUI_TYPES_H

#include <windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#define MAX_PATH_W 32767
#define MAX_RECENT_FILES 10

typedef enum {
    CMD_NORMAL,
    CMD_EXTRACT,
    CMD_EXTRACT_HERE,
    CMD_ADD,
    CMD_ADD_AUTO,
    CMD_VERIFY,
    CMD_PROPERTIES
} CommandMode;

typedef struct {
    int defaultCompLevel;
    int defaultPkgType;
    int solidMode;
    int volumeMode;
    uint32_t volumeSize;
    int confirmDelete;
    int overwriteFiles;
    int showStatusBar;
    int showGridLines;
    int windowWidth;
    int windowHeight;
    int windowMaximized;
} Settings;

typedef struct {
    wchar_t path[MAX_PATH_W];
    time_t timestamp;
} HistoryItem;

typedef struct {
    HINSTANCE hInstance;
    HWND hMainWnd;
    HWND hFileList;
    HWND hStatusBar;
    
    void* xpk;
    wchar_t xpkPath[MAX_PATH_W];
    wchar_t currentDir[MAX_PATH_W];
    
    CommandMode commandMode;
    wchar_t commandPath[MAX_PATH_W];
    
    Settings settings;
    HistoryItem history[MAX_RECENT_FILES];
    int historyCount;
    
    HMODULE hShell32;
    int initialized;
} AppContext;

typedef struct {
    void* (*callback)(void* userData);
    void* userData;
    int completed;
    int result;
} AsyncOperation;

typedef int (*DialogCallback)(AppContext* ctx, void* result);

#endif
