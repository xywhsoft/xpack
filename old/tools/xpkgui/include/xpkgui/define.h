#ifndef XPKGUI_DEFINE_H
#define XPKGUI_DEFINE_H

#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <xrt.h>
#include <xpack.h>

#define MAX_PATH_W 32767
#define MAX_RECENT_FILES 10
#define XPKGUI_CLASS_NAME L"XPKGUIWindow"
#define XPKGUI_WINDOW_TITLE L"XPK GUI"

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
    void* (*callback)(void* userData);
    void* userData;
    int completed;
    int result;
} AsyncOperation;

struct AppContext {
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
};

typedef struct AppContext AppContext;

typedef int (*DialogCallback)(AppContext* ctx, void* result);

LRESULT CALLBACK Window_Proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

int App_Initialize(AppContext* ctx);
void App_Run(AppContext* ctx);
void App_Cleanup(AppContext* ctx);
int App_ProcessCommandLine(AppContext* ctx, LPWSTR lpCmdLine);

HWND Window_Create(AppContext* ctx);
int Window_InitializeControls(AppContext* ctx);
void Window_UpdateStatusBar(AppContext* ctx);
void Window_UpdateTitle(AppContext* ctx);

int ExtractModeMain(AppContext* ctx, const wchar_t* path);
int ExtractHereModeMain(AppContext* ctx, const wchar_t* path);
int VerifyModeMain(AppContext* ctx, const wchar_t* path);
int PropertiesModeMain(AppContext* ctx, const wchar_t* path);

int Dialog_BrowseForFolder(HWND hwnd, wchar_t* path, const wchar_t* title);
int Dialog_BrowseForFiles(HWND hwnd, wchar_t* files, int* fileCount, const wchar_t* filter);
int Dialog_BrowseForDirectory(HWND hwnd, wchar_t* path, const wchar_t* title);
int Dialog_CompressLevel(HWND hwnd, int* level);
int Dialog_NewPackage(HWND hwnd, wchar_t* path, int* solidMode, int* pkgType);
int Dialog_InputBox(HWND hwnd, const wchar_t* title, const wchar_t* prompt, wchar_t* buffer, int bufferSize);
int Dialog_PatternSelect(HWND hwnd, wchar_t* pattern, int* operation);

HWND FileList_Create(AppContext* ctx);
void FileList_Refresh(AppContext* ctx);
int FileList_GetSelectedIndex(AppContext* ctx);
int FileList_GetSelectedPath(AppContext* ctx, wchar_t* path, int maxLen);

HWND StatusBar_Create(AppContext* ctx);
void StatusBar_Update(AppContext* ctx);

void MenuHandler_HandleCommand(AppContext* ctx, WORD commandId);
void MenuHandler_HandleNotify(AppContext* ctx, LPNMHDR pnmh);
void MenuHandler_HandleDropFiles(AppContext* ctx, HDROP hDrop);
void MenuHandler_HandleAddFiles(AppContext* ctx);

int PackageOps_Open(AppContext* ctx, const wchar_t* path);
int PackageOps_Close(AppContext* ctx);
int PackageOps_Save(AppContext* ctx);
int PackageOps_CreateNew(AppContext* ctx);
int PackageOps_Rebuild(AppContext* ctx);
int PackageOps_GetProperties(AppContext* ctx);
int PackageOps_ToggleSolidMode(AppContext* ctx);
int PackageOps_ToggleVolumeMode(AppContext* ctx);
int PackageOps_SetVolumeSize(AppContext* ctx);
int PackageOps_SetDiscCode(AppContext* ctx);
int PackageOps_SelectByPattern(AppContext* ctx);

int DirOps_AddDirectory(AppContext* ctx);

int FileOps_Delete(AppContext* ctx);
int FileOps_Rename(AppContext* ctx);

int ExtractOps_ExtractSelected(AppContext* ctx);
int ExtractOps_ExtractAll(AppContext* ctx);

int VerifyOps_VerifyAll(AppContext* ctx);
int VerifyOps_TestPackage(AppContext* ctx);

void Settings_Load(Settings* settings);
void Settings_Save(Settings* settings);

void History_Load(AppContext* ctx);
void History_Add(AppContext* ctx, const wchar_t* path);

wchar_t* String_Utf8ToWchar(const char* utf8Str);
char* String_WcharToUtf8(const wchar_t* wstr);

void FormatSize(uint64_t size, wchar_t* buf, int bufSize);
void FormatTime(time_t t, wchar_t* buf, int bufSize);
const wchar_t* FormatUtils_GetFileTypeString(int type);

void ErrorHandler_Handle(int code, const char* message);
void ErrorHandler_Error(HWND hwnd, const wchar_t* msg);
void ErrorHandler_Info(HWND hwnd, const wchar_t* msg);
void ErrorHandler_ShowDetailed(HWND hwnd, const wchar_t* context);

#endif
