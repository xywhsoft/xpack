#ifndef XPKGUI_HEADERS_H
#define XPKGUI_HEADERS_H

#include <xpkgui/common.h>

void Settings_Load(Settings* settings);
void Settings_Save(Settings* settings);
void History_Load(AppContext* ctx);
void History_Add(AppContext* ctx, const wchar_t* path);
void ErrorHandler_Handle(int code, const char* message);
void ErrorHandler_Error(HWND hwnd, const wchar_t* msg);
void ErrorHandler_Info(HWND hwnd, const wchar_t* msg);
void ErrorHandler_ShowDetailed(HWND hwnd, const wchar_t* context);
wchar_t* String_Utf8ToWchar(const char* utf8Str);
char* String_WcharToUtf8(const wchar_t* wstr);
void FormatSize(uint64_t size, wchar_t* buf, int bufSize);
void FormatTime(time_t t, wchar_t* buf, int bufSize);
const wchar_t* FormatUtils_GetFileTypeString(int type);
int Dialog_BrowseForFolder(HWND hwnd, wchar_t* path, const wchar_t* title);
int Dialog_BrowseForFiles(HWND hwnd, wchar_t* files, int* fileCount, const wchar_t* filter);
int Dialog_CompressLevel(HWND hwnd, int* level);
int Dialog_InputBox(HWND hwnd, const wchar_t* title, const wchar_t* prompt, wchar_t* buffer, int bufferSize);
HWND FileList_Create(AppContext* ctx);
void FileList_Refresh(AppContext* ctx);
int FileList_GetSelectedIndex(AppContext* ctx);
int FileList_GetSelectedPath(AppContext* ctx, wchar_t* path, int maxLen);
HWND StatusBar_Create(AppContext* ctx);
void StatusBar_Update(AppContext* ctx);
void Window_UpdateStatusBar(AppContext* ctx);
void Window_UpdateTitle(AppContext* ctx);
HWND Window_Create(AppContext* ctx);
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
int FileOps_Delete(AppContext* ctx);
int FileOps_Rename(AppContext* ctx);
int ExtractOps_ExtractSelected(AppContext* ctx);
int ExtractOps_ExtractAll(AppContext* ctx);
int VerifyOps_VerifyAll(AppContext* ctx);
int VerifyOps_TestPackage(AppContext* ctx);
int ExtractModeMain(AppContext* ctx, const wchar_t* path);
int ExtractHereModeMain(AppContext* ctx, const wchar_t* path);
int VerifyModeMain(AppContext* ctx, const wchar_t* path);
int PropertiesModeMain(AppContext* ctx, const wchar_t* path);

#endif
