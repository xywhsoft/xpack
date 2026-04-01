#include "app.h"
#include "resources/resource.h"

#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Shlwapi.lib")

#define XRT_TIME_FORMAT_DATETIME 0

extern char* xrtTimeToStr(xtime iTime, int iFormat);
extern void xrtFree(void* pMem);

static BOOL GuiBuildSettingsIniPath(WCHAR* pathBuf, size_t cchPathBuf)
{
	WCHAR appData[MAX_PATH];

	if ( pathBuf == NULL || cchPathBuf == 0 ) {
		return FALSE;
	}

	if ( SHGetFolderPathW(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, appData) != S_OK ) {
		pathBuf[0] = L'\0';
		return FALSE;
	}

	_snwprintf_s(pathBuf, cchPathBuf, _TRUNCATE, L"%s\\xPack", appData);
	if ( !GuiEnsureDirectoryRecursive(pathBuf) ) {
		pathBuf[0] = L'\0';
		return FALSE;
	}
	wcscat_s(pathBuf, cchPathBuf, L"\\xpkgui.ini");
	return TRUE;
}

static void GuiSaveRecentArchives(const GuiApp* app)
{
	WCHAR iniPath[MAX_PATH];
	static const WCHAR emptySection[] = { L'\0', L'\0' };
	UINT i;

	if ( app == NULL || !GuiBuildSettingsIniPath(iniPath, _countof(iniPath)) ) {
		return;
	}

	WritePrivateProfileSectionW(L"RecentArchives", emptySection, iniPath);
	for ( i = 0; i < app->recentArchiveCount && i < XPKGUI_MAX_RECENT_ARCHIVES; ++i ) {
		WCHAR key[32];

		_snwprintf_s(key, _countof(key), _TRUNCATE, L"Item%u", i);
		WritePrivateProfileStringW(L"RecentArchives", key, app->recentArchives[i], iniPath);
	}
}

BOOL GuiLoadRecentArchives(GuiApp* app)
{
	WCHAR iniPath[MAX_PATH];
	UINT i;

	if ( app == NULL ) {
		return FALSE;
	}

	app->recentArchiveCount = 0;
	ZeroMemory(app->recentArchives, sizeof(app->recentArchives));
	if ( !GuiBuildSettingsIniPath(iniPath, _countof(iniPath)) ) {
		return FALSE;
	}

	for ( i = 0; i < XPKGUI_MAX_RECENT_ARCHIVES; ++i ) {
		WCHAR key[32];
		WCHAR value[MAX_PATH];

		_snwprintf_s(key, _countof(key), _TRUNCATE, L"Item%u", i);
		value[0] = L'\0';
		GetPrivateProfileStringW(L"RecentArchives", key, L"", value, (DWORD)_countof(value), iniPath);
		if ( value[0] == L'\0' ) {
			continue;
		}
		wcsncpy_s(app->recentArchives[app->recentArchiveCount], _countof(app->recentArchives[0]), value, _TRUNCATE);
		app->recentArchiveCount++;
	}
	return TRUE;
}

BOOL GuiRememberRecentArchive(GuiApp* app, const WCHAR* archivePath)
{
	WCHAR normalized[MAX_PATH];
	UINT existing;
	UINT i;
	DWORD len;

	if ( app == NULL || archivePath == NULL || archivePath[0] == L'\0' ) {
		return FALSE;
	}

	len = GetFullPathNameW(archivePath, (DWORD)_countof(normalized), normalized, NULL);
	if ( len == 0 || len >= _countof(normalized) ) {
		wcsncpy_s(normalized, _countof(normalized), archivePath, _TRUNCATE);
	}

	existing = app->recentArchiveCount;
	for ( i = 0; i < app->recentArchiveCount; ++i ) {
		if ( _wcsicmp(app->recentArchives[i], normalized) == 0 ) {
			existing = i;
			break;
		}
	}

	if ( existing < app->recentArchiveCount ) {
		for ( i = existing; i > 0; --i ) {
			wcsncpy_s(app->recentArchives[i], _countof(app->recentArchives[0]), app->recentArchives[i - 1], _TRUNCATE);
		}
	} else {
		if ( app->recentArchiveCount < XPKGUI_MAX_RECENT_ARCHIVES ) {
			app->recentArchiveCount++;
		}
		for ( i = app->recentArchiveCount - 1; i > 0; --i ) {
			wcsncpy_s(app->recentArchives[i], _countof(app->recentArchives[0]), app->recentArchives[i - 1], _TRUNCATE);
		}
	}

	wcsncpy_s(app->recentArchives[0], _countof(app->recentArchives[0]), normalized, _TRUNCATE);
	GuiSaveRecentArchives(app);
	return TRUE;
}

BOOL GuiClearRecentArchives(GuiApp* app)
{
	if ( app == NULL ) {
		return FALSE;
	}

	app->recentArchiveCount = 0;
	ZeroMemory(app->recentArchives, sizeof(app->recentArchives));
	GuiSaveRecentArchives(app);
	return TRUE;
}

const WCHAR* GuiGetRecentArchivePath(const GuiApp* app, UINT commandId)
{
	UINT index;

	if ( app == NULL || commandId < ID_FILE_RECENT_FIRST || commandId > ID_FILE_RECENT_LAST ) {
		return NULL;
	}

	index = commandId - ID_FILE_RECENT_FIRST;
	if ( index >= app->recentArchiveCount ) {
		return NULL;
	}
	return app->recentArchives[index];
}

static void GuiFreeLaunchInputs(GuiApp* app)
{
	int i;

	if ( app->launch.inputs != NULL ) {
		for ( i = 0; i < app->launch.inputCount; ++i ) {
			LocalFree(app->launch.inputs[i]);
		}
		LocalFree(app->launch.inputs);
	}
	ZeroMemory(&app->launch, sizeof(app->launch));
}

void GuiAppInitDefaults(GuiApp* app)
{
	app->packType = XPK_PACK_WIN32;
	app->defaultComp = 7;
	app->metaComp = 7;
	app->infoComp = 7;
	app->infoExtSize = 0;
	app->volumeSize = 0;
	app->writePolicy = XPK_WRITE_BUFFERED;
	app->solidMode = FALSE;
	app->currentFolder[0] = L'\0';
	app->flatView = FALSE;
	app->sortColumn = 0;
	app->sortAscending = TRUE;
	app->navHistoryCount = 0;
	app->navHistoryIndex = 0;
	app->navHistoryLocked = FALSE;
	app->launchErrorShown = FALSE;
	app->archiveChangePromptActive = FALSE;
	ZeroMemory(&app->archiveWriteTime, sizeof(app->archiveWriteTime));
	app->archiveFileSize = 0;
}

void GuiAppCloseArchive(GuiApp* app)
{
	if ( app->archive != NULL ) {
		xpkClose(app->archive);
		app->archive = NULL;
	}
	free(app->items);
	app->items = NULL;
	app->itemCount = 0;
	app->itemCapacity = 0;
	free(app->viewItems);
	app->viewItems = NULL;
	app->viewCount = 0;
	app->viewCapacity = 0;
	app->archivePath[0] = L'\0';
	GuiAppInitDefaults(app);
}

void GuiAppCleanup(GuiApp* app)
{
	GuiAppCloseArchive(app);
	GuiFreeLaunchInputs(app);
}

BOOL GuiUtf8FromWide(const WCHAR* src, char* dst, size_t dstCount)
{
	int iRet;

	if ( src == NULL || dst == NULL || dstCount == 0 ) {
		return FALSE;
	}

	iRet = WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, (int)dstCount, NULL, NULL);
	if ( iRet <= 0 ) {
		dst[0] = '\0';
		return FALSE;
	}
	return TRUE;
}

BOOL GuiWideFromUtf8(const char* src, WCHAR* dst, size_t dstCount)
{
	int iRet;

	if ( src == NULL || dst == NULL || dstCount == 0 ) {
		return FALSE;
	}

	iRet = MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, (int)dstCount);
	if ( iRet <= 0 ) {
		dst[0] = L'\0';
		return FALSE;
	}
	return TRUE;
}

void GuiFormatUInt64(uint64_t value, WCHAR* buf, size_t cchBuf)
{
	_snwprintf_s(buf, cchBuf, _TRUNCATE, L"%llu", (unsigned long long)value);
}

void GuiFormatTime(xtime value, WCHAR* buf, size_t cchBuf)
{
	char* sText;

	if ( value == 0 ) {
		buf[0] = L'\0';
		return;
	}

	sText = xrtTimeToStr(value, XRT_TIME_FORMAT_DATETIME);
	if ( sText == NULL ) {
		_snwprintf_s(buf, cchBuf, _TRUNCATE, L"%lld", (long long)value);
		return;
	}

	if ( !GuiWideFromUtf8(sText, buf, cchBuf) ) {
		_snwprintf_s(buf, cchBuf, _TRUNCATE, L"%lld", (long long)value);
	}
	xrtFree(sText);
}

void GuiUpdateTitle(GuiApp* app)
{
	WCHAR title[512];
	BOOL pathPack;

	if ( app->window == NULL ) {
		return;
	}

	pathPack = (app->packType == XPK_PACK_LINUX || app->packType == XPK_PACK_WIN32);
	if ( app->archive != NULL && app->archivePath[0] != L'\0' ) {
		if ( pathPack ) {
			if ( app->flatView ) {
				_snwprintf_s(title, _countof(title), _TRUNCATE, L"%s - %s [flat]", XPKGUI_APP_TITLE, app->archivePath);
			} else {
				_snwprintf_s(title, _countof(title), _TRUNCATE, L"%s - %s [%s%s]", XPKGUI_APP_TITLE, app->archivePath, L"/", app->currentFolder);
			}
		} else {
			_snwprintf_s(title, _countof(title), _TRUNCATE, L"%s - %s", XPKGUI_APP_TITLE, app->archivePath);
		}
	} else {
		wcsncpy_s(title, _countof(title), XPKGUI_APP_TITLE, _TRUNCATE);
	}
	SetWindowTextW(app->window, title);
}

BOOL GuiSetClipboardText(HWND owner, const WCHAR* text)
{
	HGLOBAL hMem;
	WCHAR* dst;
	size_t cbText;

	if ( text == NULL ) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	cbText = (wcslen(text) + 1) * sizeof(WCHAR);
	hMem = GlobalAlloc(GMEM_MOVEABLE, cbText);
	if ( hMem == NULL ) {
		SetLastError(ERROR_OUTOFMEMORY);
		return FALSE;
	}

	dst = (WCHAR*)GlobalLock(hMem);
	if ( dst == NULL ) {
		GlobalFree(hMem);
		SetLastError(ERROR_OUTOFMEMORY);
		return FALSE;
	}
	memcpy(dst, text, cbText);
	GlobalUnlock(hMem);

	if ( !OpenClipboard(owner) ) {
		GlobalFree(hMem);
		return FALSE;
	}
	if ( !EmptyClipboard() ) {
		DWORD err;

		err = GetLastError();
		CloseClipboard();
		GlobalFree(hMem);
		SetLastError(err);
		return FALSE;
	}
	if ( SetClipboardData(CF_UNICODETEXT, hMem) == NULL ) {
		DWORD err;

		err = GetLastError();
		CloseClipboard();
		GlobalFree(hMem);
		SetLastError(err);
		return FALSE;
	}
	CloseClipboard();
	return TRUE;
}

void GuiUpdateStatus(GuiApp* app)
{
	WCHAR status[768];
	WCHAR viewText[128];
	WCHAR selSizeText[64];
	WCHAR selPackedText[64];
	xpkStat statInfo;
	BOOL pathPack;
	uint32_t selectedRows;
	uint32_t selectedFiles;
	uint64_t selectedSize;
	uint64_t selectedPacked;

	if ( app->status == NULL ) {
		return;
	}

	if ( app->archive == NULL ) {
		SendMessageW(app->status, SB_SETTEXTW, 0, (LPARAM)L"Ready");
		return;
	}

	pathPack = (app->packType == XPK_PACK_LINUX || app->packType == XPK_PACK_WIN32);
	if ( pathPack ) {
		if ( app->flatView ) {
			_snwprintf_s(viewText, _countof(viewText), _TRUNCATE, L" | view=flat");
		} else {
			_snwprintf_s(viewText, _countof(viewText), _TRUNCATE, L" | view=/%s", app->currentFolder);
		}
	} else {
		viewText[0] = L'\0';
	}

	selectedRows = GuiArchiveSelectedCount(app);
	selectedFiles = 0;
	selectedSize = 0;
	selectedPacked = 0;
	GuiArchiveGetSelectionSummary(app, &selectedFiles, &selectedSize, &selectedPacked);
	GuiFormatUInt64(selectedSize, selSizeText, _countof(selSizeText));
	GuiFormatUInt64(selectedPacked, selPackedText, _countof(selPackedText));

	ZeroMemory(&statInfo, sizeof(statInfo));
	if ( xpkStatGet(app->archive, &statInfo) == XPK_OK ) {
		_snwprintf_s(
			status,
			_countof(status),
			_TRUNCATE,
			L"type=%d | entries=%u | shown=%u | sel=%u rows/%u files | selSize=%s | selPacked=%s | live=%llu | holes=%llu | meta=%llu | table=%llu%s",
			(int)app->packType,
			statInfo.fileCount,
			(unsigned)app->viewCount,
			(unsigned)selectedRows,
			(unsigned)selectedFiles,
			selSizeText,
			selPackedText,
			(unsigned long long)statInfo.liveDataBytes,
			(unsigned long long)statInfo.holeBytes,
			(unsigned long long)statInfo.metaBytes,
			(unsigned long long)statInfo.entryTableBytes,
			viewText);
	} else {
		_snwprintf_s(status, _countof(status), _TRUNCATE, L"%S", xpkLastErrorMessage(app->archive));
	}
	SendMessageW(app->status, SB_SETTEXTW, 0, (LPARAM)status);
}

static BOOL GuiIsArchiveFormatErrorText(const char* sError)
{
	if ( sError == NULL ) {
		return FALSE;
	}
	return (strcmp(sError, "invalid xpk file header") == 0) ||
		(strcmp(sError, "invalid xpk package format") == 0);
}

static void GuiShowArchiveErrorInternal(GuiApp* app, const WCHAR* actionText, const WCHAR* archivePath)
{
	WCHAR text[1024];
	WCHAR wideError[768];
	const char* sError;

	if ( app != NULL ) {
		app->launchErrorShown = TRUE;
	}

	sError = (app->archive != NULL) ? xpkLastErrorMessage(app->archive) : xpkLastErrorMessage(NULL);
	if ( sError == NULL || sError[0] == '\0' ) {
		sError = "unknown error";
	}
	GuiWideFromUtf8(sError, wideError, _countof(wideError));
	if ( archivePath != NULL && archivePath[0] != L'\0' ) {
		_snwprintf_s(text, _countof(text), _TRUNCATE, L"%s失败。\n\n文件：%s\n错误：%s", actionText, archivePath, wideError);
		if ( GuiIsArchiveFormatErrorText(sError) ) {
			wcsncat_s(
				text,
				_countof(text),
				L"\n\n提示：当前版本不兼容旧版/遗留 xpk 包。"
				L"如果这个文件来自旧工具或历史测试产物，请用当前 xPack 重新创建后再打开。",
				_TRUNCATE);
		}
	} else {
		_snwprintf_s(text, _countof(text), _TRUNCATE, L"%s失败。\n\n%s", actionText, wideError);
	}
	MessageBoxW(app->window, text, XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
}

void GuiShowArchiveError(GuiApp* app, const WCHAR* actionText)
{
	GuiShowArchiveErrorInternal(app, actionText, NULL);
}

void GuiShowArchiveErrorPath(GuiApp* app, const WCHAR* actionText, const WCHAR* archivePath)
{
	GuiShowArchiveErrorInternal(app, actionText, archivePath);
}

void GuiShowSystemError(HWND hwnd, const WCHAR* title, DWORD err)
{
	WCHAR text[512];
	DWORD len;

	len = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, 0, text, (DWORD)_countof(text), NULL);
	if ( len == 0 ) {
		_snwprintf_s(text, _countof(text), _TRUNCATE, L"系统错误 %lu", err);
	}
	MessageBoxW(hwnd, text, title, MB_OK | MB_ICONERROR);
}

BOOL GuiOpenArchiveDialog(HWND hwnd, WCHAR* pathBuf, DWORD cchBuf)
{
	OPENFILENAMEW ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	pathBuf[0] = L'\0';
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = L"xPack Archive (*.xpk)\0*.xpk\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = pathBuf;
	ofn.nMaxFile = cchBuf;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	return GetOpenFileNameW(&ofn);
}

BOOL GuiSaveArchiveDialog(HWND hwnd, WCHAR* pathBuf, DWORD cchBuf)
{
	OPENFILENAMEW ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = L"xPack Archive (*.xpk)\0*.xpk\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = pathBuf;
	ofn.nMaxFile = cchBuf;
	ofn.lpstrDefExt = L"xpk";
	ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
	return GetSaveFileNameW(&ofn);
}

BOOL GuiPickFilesDialog(HWND hwnd, WCHAR* buffer, DWORD cchBuffer)
{
	OPENFILENAMEW ofn;

	ZeroMemory(buffer, sizeof(WCHAR) * cchBuffer);
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
	ofn.lpstrFile = buffer;
	ofn.nMaxFile = cchBuffer;
	ofn.Flags = OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	return GetOpenFileNameW(&ofn);
}

BOOL GuiPickSingleFileDialog(HWND hwnd, WCHAR* pathBuf, DWORD cchBuf, const WCHAR* title)
{
	OPENFILENAMEW ofn;

	if ( pathBuf == NULL || cchBuf == 0 ) {
		return FALSE;
	}

	ZeroMemory(pathBuf, sizeof(WCHAR) * cchBuf);
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
	ofn.lpstrFile = pathBuf;
	ofn.nMaxFile = cchBuf;
	ofn.lpstrTitle = title;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	return GetOpenFileNameW(&ofn);
}

BOOL GuiPickFolderDialog(HWND hwnd, WCHAR* pathBuf, DWORD cchBuf, const WCHAR* title)
{
	BROWSEINFOW bi;
	PIDLIST_ABSOLUTE pidl;
	BOOL ok;

	(void)cchBuf;

	ZeroMemory(&bi, sizeof(bi));
	bi.hwndOwner = hwnd;
	bi.lpszTitle = title;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI | BIF_NONEWFOLDERBUTTON;

	pidl = SHBrowseForFolderW(&bi);
	if ( pidl == NULL ) {
		return FALSE;
	}

	ok = SHGetPathFromIDListW(pidl, pathBuf);
	CoTaskMemFree((LPVOID)pidl);
	if ( !ok ) {
		pathBuf[0] = L'\0';
	}
	return ok;
}

BOOL GuiReadWholeFile(const WCHAR* path, void** dataOut, DWORD* sizeOut)
{
	HANDLE hFile;
	LARGE_INTEGER fileSize;
	void* pData;
	DWORD bytesRead;

	*dataOut = NULL;
	*sizeOut = 0;

	hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hFile == INVALID_HANDLE_VALUE ) {
		return FALSE;
	}
	if ( !GetFileSizeEx(hFile, &fileSize) || fileSize.QuadPart > MAXDWORD ) {
		CloseHandle(hFile);
		return FALSE;
	}

	pData = NULL;
	if ( fileSize.QuadPart > 0 ) {
		pData = malloc((size_t)fileSize.QuadPart);
		if ( pData == NULL ) {
			CloseHandle(hFile);
			return FALSE;
		}
		if ( !ReadFile(hFile, pData, (DWORD)fileSize.QuadPart, &bytesRead, NULL) || bytesRead != (DWORD)fileSize.QuadPart ) {
			free(pData);
			CloseHandle(hFile);
			return FALSE;
		}
	}

	CloseHandle(hFile);
	*dataOut = pData;
	*sizeOut = (DWORD)fileSize.QuadPart;
	return TRUE;
}

BOOL GuiEnsureDirectoryRecursive(const WCHAR* dirPath)
{
	WCHAR temp[MAX_PATH];
	size_t i;

	if ( dirPath == NULL || dirPath[0] == L'\0' ) {
		return TRUE;
	}

	wcsncpy_s(temp, _countof(temp), dirPath, _TRUNCATE);
	for ( i = 3; temp[i] != L'\0'; ++i ) {
		if ( temp[i] == L'/' ) {
			temp[i] = L'\\';
		}
		if ( temp[i] == L'\\' ) {
			temp[i] = L'\0';
			CreateDirectoryW(temp, NULL);
			temp[i] = L'\\';
		}
	}
	if ( !CreateDirectoryW(temp, NULL) ) {
		DWORD err = GetLastError();
		if ( err != ERROR_ALREADY_EXISTS ) {
			return FALSE;
		}
	}
	return TRUE;
}

BOOL GuiEnsureParentDirectory(const WCHAR* filePath)
{
	WCHAR temp[MAX_PATH];
	WCHAR* slash;

	wcsncpy_s(temp, _countof(temp), filePath, _TRUNCATE);
	slash = wcsrchr(temp, L'\\');
	if ( slash == NULL ) {
		slash = wcsrchr(temp, L'/');
	}
	if ( slash == NULL ) {
		return TRUE;
	}

	*slash = L'\0';
	return GuiEnsureDirectoryRecursive(temp);
}

BOOL GuiWriteWholeFile(const WCHAR* path, const void* data, DWORD size)
{
	HANDLE hFile;
	DWORD bytesWritten;

	if ( !GuiEnsureParentDirectory(path) ) {
		return FALSE;
	}

	hFile = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hFile == INVALID_HANDLE_VALUE ) {
		return FALSE;
	}

	if ( size > 0 ) {
		if ( !WriteFile(hFile, data, size, &bytesWritten, NULL) || bytesWritten != size ) {
			CloseHandle(hFile);
			return FALSE;
		}
	}

	CloseHandle(hFile);
	return TRUE;
}

BOOL GuiSuggestArchivePath(int count, WCHAR** items, WCHAR* pathBuf, DWORD cchBuf, BOOL makeUnique)
{
	WCHAR dirBuf[MAX_PATH];
	WCHAR nameBuf[MAX_PATH];
	WCHAR candidate[MAX_PATH];
	DWORD attr;
	int i;

	if ( count <= 0 || items == NULL || pathBuf == NULL || cchBuf == 0 ) {
		return FALSE;
	}

	wcsncpy_s(dirBuf, _countof(dirBuf), items[0], _TRUNCATE);
	attr = GetFileAttributesW(items[0]);
	if ( attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) ) {
		PathRemoveFileSpecW(dirBuf);
		wcsncpy_s(nameBuf, _countof(nameBuf), PathFindFileNameW(items[0]), _TRUNCATE);
	} else {
		PathRemoveFileSpecW(dirBuf);
		wcsncpy_s(nameBuf, _countof(nameBuf), PathFindFileNameW(items[0]), _TRUNCATE);
		PathRemoveExtensionW(nameBuf);
	}

	if ( count > 1 ) {
		wcsncpy_s(nameBuf, _countof(nameBuf), L"xpack_archive", _TRUNCATE);
	}

	_snwprintf_s(candidate, _countof(candidate), _TRUNCATE, L"%s\\%s.xpk", dirBuf, nameBuf);
	if ( !makeUnique || GetFileAttributesW(candidate) == INVALID_FILE_ATTRIBUTES ) {
		wcsncpy_s(pathBuf, cchBuf, candidate, _TRUNCATE);
		return TRUE;
	}

	for ( i = 2; i < 1000; ++i ) {
		_snwprintf_s(candidate, _countof(candidate), _TRUNCATE, L"%s\\%s (%d).xpk", dirBuf, nameBuf, i);
		if ( GetFileAttributesW(candidate) == INVALID_FILE_ATTRIBUTES ) {
			wcsncpy_s(pathBuf, cchBuf, candidate, _TRUNCATE);
			return TRUE;
		}
	}

	return FALSE;
}

BOOL GuiSuggestExtractFolderPath(const WCHAR* archivePath, WCHAR* pathBuf, DWORD cchBuf, BOOL makeUnique)
{
	WCHAR dirBuf[MAX_PATH];
	WCHAR nameBuf[MAX_PATH];
	WCHAR candidate[MAX_PATH];
	DWORD attr;
	int i;

	if ( archivePath == NULL || archivePath[0] == L'\0' || pathBuf == NULL || cchBuf == 0 ) {
		return FALSE;
	}

	wcsncpy_s(dirBuf, _countof(dirBuf), archivePath, _TRUNCATE);
	PathRemoveFileSpecW(dirBuf);
	wcsncpy_s(nameBuf, _countof(nameBuf), PathFindFileNameW(archivePath), _TRUNCATE);
	PathRemoveExtensionW(nameBuf);
	if ( nameBuf[0] == L'\0' ) {
		wcsncpy_s(nameBuf, _countof(nameBuf), L"xpack_extract", _TRUNCATE);
	}

	_snwprintf_s(candidate, _countof(candidate), _TRUNCATE, L"%s\\%s", dirBuf, nameBuf);
	attr = GetFileAttributesW(candidate);
	if ( !makeUnique || attr == INVALID_FILE_ATTRIBUTES ) {
		wcsncpy_s(pathBuf, cchBuf, candidate, _TRUNCATE);
		return TRUE;
	}

	for ( i = 2; i < 1000; ++i ) {
		_snwprintf_s(candidate, _countof(candidate), _TRUNCATE, L"%s\\%s (%d)", dirBuf, nameBuf, i);
		if ( GetFileAttributesW(candidate) == INVALID_FILE_ATTRIBUTES ) {
			wcsncpy_s(pathBuf, cchBuf, candidate, _TRUNCATE);
			return TRUE;
		}
	}

	return FALSE;
}

int GuiParseCommandLineArgs(GuiApp* app, int argc, WCHAR** argv)
{
	int i;

	if ( argc <= 1 ) {
		return 0;
	}

	if ( _wcsicmp(argv[1], L"/shell-add") == 0 || _wcsicmp(argv[1], L"-shell-add") == 0 ) {
		app->launch.command = GUI_LAUNCH_SHELL_ADD;
	} else if ( _wcsicmp(argv[1], L"/shell-add-auto") == 0 || _wcsicmp(argv[1], L"-shell-add-auto") == 0 ) {
		app->launch.command = GUI_LAUNCH_SHELL_ADD_AUTO;
	} else if ( _wcsicmp(argv[1], L"/extract") == 0 || _wcsicmp(argv[1], L"-extract") == 0 ) {
		app->launch.command = GUI_LAUNCH_SHELL_EXTRACT;
	} else if ( _wcsicmp(argv[1], L"/extract-auto") == 0 || _wcsicmp(argv[1], L"-extract-auto") == 0 ) {
		app->launch.command = GUI_LAUNCH_SHELL_EXTRACT_AUTO;
	} else if ( _wcsicmp(argv[1], L"/extract-here") == 0 || _wcsicmp(argv[1], L"-extract-here") == 0 ) {
		app->launch.command = GUI_LAUNCH_SHELL_EXTRACT_HERE;
	} else if ( _wcsicmp(argv[1], L"/verify") == 0 || _wcsicmp(argv[1], L"-verify") == 0 ) {
		app->launch.command = GUI_LAUNCH_SHELL_VERIFY;
	} else if ( _wcsicmp(argv[1], L"/properties") == 0 || _wcsicmp(argv[1], L"-properties") == 0 ) {
		app->launch.command = GUI_LAUNCH_SHELL_PROPERTIES;
	} else if ( argv[1][0] == L'/' || argv[1][0] == L'-' ) {
		MessageBoxW(NULL, L"不支持的命令行参数。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return 2;
	} else {
		app->launch.command = GUI_LAUNCH_OPEN;
		wcsncpy_s(app->launch.archivePath, _countof(app->launch.archivePath), argv[1], _TRUNCATE);
		return 0;
	}

	if ( app->launch.command == GUI_LAUNCH_SHELL_EXTRACT ||
		app->launch.command == GUI_LAUNCH_SHELL_EXTRACT_AUTO ||
		app->launch.command == GUI_LAUNCH_SHELL_EXTRACT_HERE ||
		app->launch.command == GUI_LAUNCH_SHELL_VERIFY ||
		app->launch.command == GUI_LAUNCH_SHELL_PROPERTIES ) {
		if ( argc < 3 ) {
			MessageBoxW(NULL, L"归档路径缺失。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
			return 2;
		}
		wcsncpy_s(app->launch.archivePath, _countof(app->launch.archivePath), argv[2], _TRUNCATE);
		if ( app->launch.command == GUI_LAUNCH_SHELL_EXTRACT && argc >= 4 ) {
			wcsncpy_s(app->launch.destPath, _countof(app->launch.destPath), argv[3], _TRUNCATE);
		}
		return 0;
	}

	app->launch.inputCount = argc - 2;
	if ( app->launch.inputCount <= 0 || app->launch.inputCount > XPKGUI_MAX_INPUTS ) {
		MessageBoxW(NULL, L"输入项数量无效。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return 2;
	}

	app->launch.inputs = (WCHAR**)LocalAlloc(LPTR, sizeof(WCHAR*) * app->launch.inputCount);
	if ( app->launch.inputs == NULL ) {
		return 2;
	}

	for ( i = 0; i < app->launch.inputCount; ++i ) {
		size_t cch = wcslen(argv[i + 2]) + 1;
		app->launch.inputs[i] = (WCHAR*)LocalAlloc(LPTR, sizeof(WCHAR) * cch);
		if ( app->launch.inputs[i] == NULL ) {
			return 2;
		}
		wcsncpy_s(app->launch.inputs[i], cch, argv[i + 2], _TRUNCATE);
	}

	return 0;
}
