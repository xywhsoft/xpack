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

static const int g_defaultColumnWidths[XPKGUI_ARCHIVE_COLUMN_COUNT] = { 340, 110, 110, 90, 120, 120, 170, 90, 110, 100 };
static const ULONGLONG XPKGUI_TEMP_ROOT_MAX_AGE_100NS = 7ull * 24ull * 60ull * 60ull * 10000000ull;

BOOL GuiIsSmokeMode(void)
{
	WCHAR value[16];
	DWORD cch;

	cch = GetEnvironmentVariableW(L"XPKGUI_SMOKE", value, _countof(value));
	return cch > 0 && cch < _countof(value) && value[0] != L'\0' && wcscmp(value, L"0") != 0;
}

static void GuiApplyDefaultColumnWidths(GuiApp* app)
{
	int i;

	if ( app == NULL ) {
		return;
	}
	for ( i = 0; i < XPKGUI_ARCHIVE_COLUMN_COUNT; ++i ) {
		if ( app->columnWidths[i] <= 0 ) {
			app->columnWidths[i] = g_defaultColumnWidths[i];
		}
	}
}

static void GuiSetDefaultColumnWidths(GuiApp* app)
{
	int i;

	if ( app == NULL ) {
		return;
	}
	for ( i = 0; i < XPKGUI_ARCHIVE_COLUMN_COUNT; ++i ) {
		app->columnWidths[i] = g_defaultColumnWidths[i];
	}
}

void GuiSetDefaultColumnVisibility(GuiApp* app)
{
	int i;

	if ( app == NULL ) {
		return;
	}
	for ( i = 0; i < XPKGUI_ARCHIVE_COLUMN_COUNT; ++i ) {
		app->visibleColumns[i] = TRUE;
	}
}

BOOL GuiIsArchiveColumnVisible(const GuiApp* app, int logicalColumn)
{
	if ( app == NULL || logicalColumn < 0 || logicalColumn >= XPKGUI_ARCHIVE_COLUMN_COUNT ) {
		return FALSE;
	}
	if ( logicalColumn == 0 ) {
		return TRUE;
	}
	return app->visibleColumns[logicalColumn] != FALSE;
}

int GuiLogicalColumnToVisible(const GuiApp* app, int logicalColumn)
{
	int i;
	int visible;

	if ( !GuiIsArchiveColumnVisible(app, logicalColumn) ) {
		return -1;
	}
	visible = 0;
	for ( i = 0; i < logicalColumn; ++i ) {
		if ( GuiIsArchiveColumnVisible(app, i) ) {
			visible++;
		}
	}
	return visible;
}

int GuiVisibleColumnToLogical(const GuiApp* app, int visibleColumn)
{
	int i;
	int visible;

	if ( app == NULL || visibleColumn < 0 ) {
		return -1;
	}
	visible = 0;
	for ( i = 0; i < XPKGUI_ARCHIVE_COLUMN_COUNT; ++i ) {
		if ( !GuiIsArchiveColumnVisible(app, i) ) {
			continue;
		}
		if ( visible == visibleColumn ) {
			return i;
		}
		visible++;
	}
	return -1;
}

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

static BOOL GuiValidateWindowRect(const RECT* rc)
{
	RECT workArea;
	HMONITOR monitor;
	MONITORINFO mi;

	if ( rc == NULL || rc->right <= rc->left || rc->bottom <= rc->top ) {
		return FALSE;
	}
	if ( (rc->right - rc->left) < 640 || (rc->bottom - rc->top) < 400 ) {
		return FALSE;
	}

	monitor = MonitorFromRect(rc, MONITOR_DEFAULTTONULL);
	if ( monitor == NULL ) {
		return FALSE;
	}
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	if ( !GetMonitorInfoW(monitor, &mi) ) {
		return FALSE;
	}
	if ( !IntersectRect(&workArea, rc, &mi.rcWork) ) {
		return FALSE;
	}
	return TRUE;
}

BOOL GuiLoadWindowPlacement(RECT* rectOut, BOOL* maximizedOut)
{
	WCHAR iniPath[MAX_PATH];
	RECT rc;
	int maximized;

	if ( rectOut == NULL || maximizedOut == NULL ) {
		return FALSE;
	}
	ZeroMemory(rectOut, sizeof(*rectOut));
	*maximizedOut = FALSE;
	if ( !GuiBuildSettingsIniPath(iniPath, _countof(iniPath)) ) {
		return FALSE;
	}

	rc.left = GetPrivateProfileIntW(L"Window", L"Left", CW_USEDEFAULT, iniPath);
	rc.top = GetPrivateProfileIntW(L"Window", L"Top", CW_USEDEFAULT, iniPath);
	rc.right = GetPrivateProfileIntW(L"Window", L"Right", CW_USEDEFAULT, iniPath);
	rc.bottom = GetPrivateProfileIntW(L"Window", L"Bottom", CW_USEDEFAULT, iniPath);
	maximized = GetPrivateProfileIntW(L"Window", L"Maximized", 0, iniPath);
	if ( !GuiValidateWindowRect(&rc) ) {
		return FALSE;
	}

	*rectOut = rc;
	*maximizedOut = (maximized != 0);
	return TRUE;
}

void GuiSaveWindowPlacement(HWND hwnd)
{
	WCHAR iniPath[MAX_PATH];
	WINDOWPLACEMENT placement;
	WCHAR value[64];

	if ( hwnd == NULL || !IsWindow(hwnd) || !GuiBuildSettingsIniPath(iniPath, _countof(iniPath)) ) {
		return;
	}

	ZeroMemory(&placement, sizeof(placement));
	placement.length = sizeof(placement);
	if ( !GetWindowPlacement(hwnd, &placement) || !GuiValidateWindowRect(&placement.rcNormalPosition) ) {
		return;
	}

	_snwprintf_s(value, _countof(value), _TRUNCATE, L"%ld", placement.rcNormalPosition.left);
	WritePrivateProfileStringW(L"Window", L"Left", value, iniPath);
	_snwprintf_s(value, _countof(value), _TRUNCATE, L"%ld", placement.rcNormalPosition.top);
	WritePrivateProfileStringW(L"Window", L"Top", value, iniPath);
	_snwprintf_s(value, _countof(value), _TRUNCATE, L"%ld", placement.rcNormalPosition.right);
	WritePrivateProfileStringW(L"Window", L"Right", value, iniPath);
	_snwprintf_s(value, _countof(value), _TRUNCATE, L"%ld", placement.rcNormalPosition.bottom);
	WritePrivateProfileStringW(L"Window", L"Bottom", value, iniPath);
	WritePrivateProfileStringW(L"Window", L"Maximized", placement.showCmd == SW_SHOWMAXIMIZED ? L"1" : L"0", iniPath);
}

BOOL GuiLoadColumnWidths(GuiApp* app)
{
	WCHAR iniPath[MAX_PATH];
	int columnCount;
	int i;

	if ( app == NULL ) {
		return FALSE;
	}
	GuiApplyDefaultColumnWidths(app);
	if ( !GuiBuildSettingsIniPath(iniPath, _countof(iniPath)) ) {
		return FALSE;
	}

	columnCount = GetPrivateProfileIntW(L"Columns", L"ColumnCount", 0, iniPath);
	if ( columnCount != XPKGUI_ARCHIVE_COLUMN_COUNT ) {
		return TRUE;
	}

	for ( i = 0; i < XPKGUI_ARCHIVE_COLUMN_COUNT; ++i ) {
		WCHAR key[32];
		int value;

		_snwprintf_s(key, _countof(key), _TRUNCATE, L"Column%d", i);
		value = GetPrivateProfileIntW(L"Columns", key, app->columnWidths[i], iniPath);
		if ( value >= 40 && value <= 2000 ) {
			app->columnWidths[i] = value;
		}
	}
	return TRUE;
}

BOOL GuiLoadColumnVisibility(GuiApp* app)
{
	WCHAR iniPath[MAX_PATH];
	int columnCount;
	int i;

	if ( app == NULL ) {
		return FALSE;
	}
	if ( !GuiBuildSettingsIniPath(iniPath, _countof(iniPath)) ) {
		return FALSE;
	}

	columnCount = GetPrivateProfileIntW(L"Columns", L"ColumnCount", 0, iniPath);
	if ( columnCount != XPKGUI_ARCHIVE_COLUMN_COUNT ) {
		return TRUE;
	}

	app->visibleColumns[0] = TRUE;
	for ( i = 1; i < XPKGUI_ARCHIVE_COLUMN_COUNT; ++i ) {
		WCHAR key[32];

		_snwprintf_s(key, _countof(key), _TRUNCATE, L"Visible%d", i);
		app->visibleColumns[i] = GetPrivateProfileIntW(L"Columns", key, app->visibleColumns[i] ? 1 : 0, iniPath) != 0;
	}
	return TRUE;
}

void GuiCaptureColumnWidths(GuiApp* app)
{
	int i;

	if ( app == NULL || app->list == NULL || !IsWindow(app->list) ) {
		return;
	}
	for ( i = 0; i < XPKGUI_ARCHIVE_COLUMN_COUNT; ++i ) {
		int visibleIndex;
		int width;

		visibleIndex = GuiLogicalColumnToVisible(app, i);
		if ( visibleIndex < 0 ) {
			continue;
		}
		width = ListView_GetColumnWidth(app->list, visibleIndex);
		if ( width >= 40 && width <= 2000 ) {
			app->columnWidths[i] = width;
		}
	}
}

void GuiSaveColumnVisibility(const GuiApp* app)
{
	WCHAR iniPath[MAX_PATH];
	WCHAR countValue[32];
	int i;

	if ( app == NULL || !GuiBuildSettingsIniPath(iniPath, _countof(iniPath)) ) {
		return;
	}

	_snwprintf_s(countValue, _countof(countValue), _TRUNCATE, L"%d", XPKGUI_ARCHIVE_COLUMN_COUNT);
	WritePrivateProfileStringW(L"Columns", L"ColumnCount", countValue, iniPath);
	WritePrivateProfileStringW(L"Columns", L"Visible0", L"1", iniPath);

	for ( i = 1; i < XPKGUI_ARCHIVE_COLUMN_COUNT; ++i ) {
		WCHAR key[32];

		_snwprintf_s(key, _countof(key), _TRUNCATE, L"Visible%d", i);
		WritePrivateProfileStringW(L"Columns", key, app->visibleColumns[i] ? L"1" : L"0", iniPath);
	}
}

void GuiSaveColumnWidths(const GuiApp* app)
{
	WCHAR iniPath[MAX_PATH];
	WCHAR countValue[32];
	int i;

	if ( app == NULL || !GuiBuildSettingsIniPath(iniPath, _countof(iniPath)) ) {
		return;
	}

	_snwprintf_s(countValue, _countof(countValue), _TRUNCATE, L"%d", XPKGUI_ARCHIVE_COLUMN_COUNT);
	WritePrivateProfileStringW(L"Columns", L"ColumnCount", countValue, iniPath);

	for ( i = 0; i < XPKGUI_ARCHIVE_COLUMN_COUNT; ++i ) {
		WCHAR key[32];
		WCHAR value[32];

		if ( app->columnWidths[i] < 40 || app->columnWidths[i] > 2000 ) {
			continue;
		}
		_snwprintf_s(key, _countof(key), _TRUNCATE, L"Column%d", i);
		_snwprintf_s(value, _countof(value), _TRUNCATE, L"%d", app->columnWidths[i]);
		WritePrivateProfileStringW(L"Columns", key, value, iniPath);
	}
}

void GuiResetColumnWidths(GuiApp* app)
{
	int i;

	if ( app == NULL ) {
		return;
	}
	GuiSetDefaultColumnWidths(app);
	if ( app->list != NULL && IsWindow(app->list) ) {
		for ( i = 0; i < XPKGUI_ARCHIVE_COLUMN_COUNT; ++i ) {
			int visibleIndex;

			visibleIndex = GuiLogicalColumnToVisible(app, i);
			if ( visibleIndex >= 0 ) {
				ListView_SetColumnWidth(app->list, visibleIndex, app->columnWidths[i]);
			}
		}
	}
	GuiSaveColumnWidths(app);
	GuiSaveColumnVisibility(app);
}

void GuiAutoSizeColumnWidths(GuiApp* app)
{
	int i;

	if ( app == NULL || app->list == NULL || !IsWindow(app->list) ) {
		return;
	}
	for ( i = 0; i < XPKGUI_ARCHIVE_COLUMN_COUNT; ++i ) {
		if ( GuiIsArchiveColumnVisible(app, i) ) {
			ListView_SetColumnWidth(app->list, GuiLogicalColumnToVisible(app, i), LVSCW_AUTOSIZE_USEHEADER);
		}
	}
	GuiCaptureColumnWidths(app);
	GuiSaveColumnWidths(app);
}

void GuiApplyListViewStyle(GuiApp* app)
{
	DWORD style;

	if ( app == NULL || app->list == NULL || !IsWindow(app->list) ) {
		return;
	}

	style = LVS_EX_DOUBLEBUFFER;
	if ( app->fullRowSelect ) {
		style |= LVS_EX_FULLROWSELECT;
	}
	if ( app->showGridLines ) {
		style |= LVS_EX_GRIDLINES;
	}
	ListView_SetExtendedListViewStyle(app->list, style);
}

BOOL GuiLoadListViewSettings(GuiApp* app)
{
	WCHAR iniPath[MAX_PATH];

	if ( app == NULL ) {
		return FALSE;
	}
	if ( !GuiBuildSettingsIniPath(iniPath, _countof(iniPath)) ) {
		return FALSE;
	}

	app->showGridLines = GetPrivateProfileIntW(L"ListView", L"ShowGridLines", app->showGridLines ? 1 : 0, iniPath) != 0;
	app->fullRowSelect = GetPrivateProfileIntW(L"ListView", L"FullRowSelect", app->fullRowSelect ? 1 : 0, iniPath) != 0;
	app->listViewSettingsInitialized = TRUE;
	GuiApplyListViewStyle(app);
	return TRUE;
}

void GuiSaveListViewSettings(const GuiApp* app)
{
	WCHAR iniPath[MAX_PATH];

	if ( app == NULL || !GuiBuildSettingsIniPath(iniPath, _countof(iniPath)) ) {
		return;
	}

	WritePrivateProfileStringW(L"ListView", L"ShowGridLines", app->showGridLines ? L"1" : L"0", iniPath);
	WritePrivateProfileStringW(L"ListView", L"FullRowSelect", app->fullRowSelect ? L"1" : L"0", iniPath);
}

BOOL GuiLoadSortSettings(GuiApp* app)
{
	WCHAR iniPath[MAX_PATH];
	int columnCount;
	int column;
	int ascending;

	if ( app == NULL ) {
		return FALSE;
	}
	if ( !GuiBuildSettingsIniPath(iniPath, _countof(iniPath)) ) {
		return FALSE;
	}

	columnCount = GetPrivateProfileIntW(L"ListView", L"ColumnCount", 0, iniPath);
	if ( columnCount != XPKGUI_ARCHIVE_COLUMN_COUNT ) {
		app->sortColumn = 0;
		app->sortAscending = TRUE;
		app->sortInitialized = TRUE;
		return TRUE;
	}

	column = GetPrivateProfileIntW(L"ListView", L"SortColumn", app->sortColumn, iniPath);
	ascending = GetPrivateProfileIntW(L"ListView", L"SortAscending", app->sortAscending ? 1 : 0, iniPath);
	if ( column >= 0 && column < XPKGUI_ARCHIVE_COLUMN_COUNT ) {
		app->sortColumn = column;
	}
	app->sortAscending = (ascending != 0);
	app->sortInitialized = TRUE;
	return TRUE;
}

void GuiSaveSortSettings(const GuiApp* app)
{
	WCHAR iniPath[MAX_PATH];
	WCHAR value[32];

	if ( app == NULL || !GuiBuildSettingsIniPath(iniPath, _countof(iniPath)) ) {
		return;
	}

	_snwprintf_s(value, _countof(value), _TRUNCATE, L"%d", XPKGUI_ARCHIVE_COLUMN_COUNT);
	WritePrivateProfileStringW(L"ListView", L"ColumnCount", value, iniPath);
	if ( app->sortColumn >= 0 && app->sortColumn < XPKGUI_ARCHIVE_COLUMN_COUNT ) {
		_snwprintf_s(value, _countof(value), _TRUNCATE, L"%d", app->sortColumn);
		WritePrivateProfileStringW(L"ListView", L"SortColumn", value, iniPath);
	}
	WritePrivateProfileStringW(L"ListView", L"SortAscending", app->sortAscending ? L"1" : L"0", iniPath);
}

static int GuiClampInt(int value, int minValue, int maxValue)
{
	if ( value < minValue ) {
		return minValue;
	}
	if ( value > maxValue ) {
		return maxValue;
	}
	return value;
}

BOOL GuiLoadArchiveDefaults(GuiApp* app)
{
	WCHAR iniPath[MAX_PATH];
	int packType;

	if ( app == NULL ) {
		return FALSE;
	}
	if ( !GuiBuildSettingsIniPath(iniPath, _countof(iniPath)) ) {
		return FALSE;
	}

	packType = GetPrivateProfileIntW(L"ArchiveDefaults", L"PackType", app->packType, iniPath);
	if ( packType >= XPK_PACK_CORE && packType <= XPK_PACK_WIN32 ) {
		app->packType = (xpkPackType)packType;
	}
	app->defaultComp = (uint8_t)GuiClampInt(GetPrivateProfileIntW(L"ArchiveDefaults", L"DefaultComp", app->defaultComp, iniPath), 0, 15);
	app->metaComp = (uint8_t)GuiClampInt(GetPrivateProfileIntW(L"ArchiveDefaults", L"MetaComp", app->metaComp, iniPath), 0, 15);
	app->infoComp = (uint8_t)GuiClampInt(GetPrivateProfileIntW(L"ArchiveDefaults", L"InfoComp", app->infoComp, iniPath), 0, 15);
	app->infoExtSize = (uint32_t)GuiClampInt(GetPrivateProfileIntW(L"ArchiveDefaults", L"InfoExtSize", app->infoExtSize, iniPath), 0, 1024 * 1024);
	app->volumeSize = (uint32_t)GetPrivateProfileIntW(L"ArchiveDefaults", L"VolumeSize", app->volumeSize, iniPath);
	app->writePolicy = GetPrivateProfileIntW(L"ArchiveDefaults", L"WritePolicy", app->writePolicy, iniPath) == XPK_WRITE_IMMEDIATE ? XPK_WRITE_IMMEDIATE : XPK_WRITE_BUFFERED;
	app->solidMode = GetPrivateProfileIntW(L"ArchiveDefaults", L"SolidMode", app->solidMode ? 1 : 0, iniPath) != 0;
	return TRUE;
}

void GuiSaveArchiveDefaults(const GuiArchiveOptions* options)
{
	WCHAR iniPath[MAX_PATH];
	WCHAR value[32];

	if ( options == NULL || !GuiBuildSettingsIniPath(iniPath, _countof(iniPath)) ) {
		return;
	}

	_snwprintf_s(value, _countof(value), _TRUNCATE, L"%d", (int)options->packType);
	WritePrivateProfileStringW(L"ArchiveDefaults", L"PackType", value, iniPath);
	_snwprintf_s(value, _countof(value), _TRUNCATE, L"%u", options->defaultComp);
	WritePrivateProfileStringW(L"ArchiveDefaults", L"DefaultComp", value, iniPath);
	_snwprintf_s(value, _countof(value), _TRUNCATE, L"%u", options->metaComp);
	WritePrivateProfileStringW(L"ArchiveDefaults", L"MetaComp", value, iniPath);
	_snwprintf_s(value, _countof(value), _TRUNCATE, L"%u", options->infoComp);
	WritePrivateProfileStringW(L"ArchiveDefaults", L"InfoComp", value, iniPath);
	_snwprintf_s(value, _countof(value), _TRUNCATE, L"%lu", (unsigned long)options->infoExtSize);
	WritePrivateProfileStringW(L"ArchiveDefaults", L"InfoExtSize", value, iniPath);
	_snwprintf_s(value, _countof(value), _TRUNCATE, L"%lu", (unsigned long)options->volumeSize);
	WritePrivateProfileStringW(L"ArchiveDefaults", L"VolumeSize", value, iniPath);
	_snwprintf_s(value, _countof(value), _TRUNCATE, L"%u", options->writePolicy);
	WritePrivateProfileStringW(L"ArchiveDefaults", L"WritePolicy", value, iniPath);
	WritePrivateProfileStringW(L"ArchiveDefaults", L"SolidMode", options->solidMode ? L"1" : L"0", iniPath);
}

BOOL GuiResetUiPreferences(GuiApp* app)
{
	WCHAR iniPath[MAX_PATH];
	static const WCHAR emptySection[] = { L'\0', L'\0' };
	int i;

	if ( app == NULL || !GuiBuildSettingsIniPath(iniPath, _countof(iniPath)) ) {
		return FALSE;
	}

	WritePrivateProfileSectionW(L"Window", emptySection, iniPath);
	WritePrivateProfileSectionW(L"Columns", emptySection, iniPath);
	WritePrivateProfileSectionW(L"ListView", emptySection, iniPath);
	WritePrivateProfileSectionW(L"ArchiveDefaults", emptySection, iniPath);

	GuiSetDefaultColumnWidths(app);
	GuiSetDefaultColumnVisibility(app);
	for ( i = 0; i < XPKGUI_ARCHIVE_COLUMN_COUNT; ++i ) {
		if ( app->list != NULL && IsWindow(app->list) ) {
			int visibleIndex;

			visibleIndex = GuiLogicalColumnToVisible(app, i);
			if ( visibleIndex >= 0 ) {
				ListView_SetColumnWidth(app->list, visibleIndex, app->columnWidths[i]);
			}
		}
	}
	app->sortColumn = 0;
	app->sortAscending = TRUE;
	app->sortInitialized = TRUE;
	app->showGridLines = TRUE;
	app->fullRowSelect = TRUE;
	app->listViewSettingsInitialized = TRUE;
	GuiApplyListViewStyle(app);
	app->skipWindowPlacementSave = TRUE;

	if ( app->archive == NULL ) {
		app->packType = XPK_PACK_WIN32;
		app->defaultComp = 7;
		app->metaComp = 7;
		app->infoComp = 7;
		app->infoExtSize = 0;
		app->volumeSize = 0;
		app->writePolicy = XPK_WRITE_BUFFERED;
		app->solidMode = FALSE;
		GuiArchiveRefreshView(app);
	} else {
		GuiArchiveRefreshView(app);
	}
	return TRUE;
}

BOOL GuiShowSettingsFile(HWND owner)
{
	WCHAR iniPath[MAX_PATH];
	WCHAR parameters[MAX_PATH + 32];
	INT_PTR shellResult;

	if ( !GuiBuildSettingsIniPath(iniPath, _countof(iniPath)) ) {
		GuiShowSystemError(owner, L"定位设置文件失败", GetLastError() != 0 ? GetLastError() : ERROR_PATH_NOT_FOUND);
		return FALSE;
	}

	if ( PathFileExistsW(iniPath) ) {
		_snwprintf_s(parameters, _countof(parameters), _TRUNCATE, L"/select,\"%s\"", iniPath);
		shellResult = (INT_PTR)ShellExecuteW(owner, L"open", L"explorer.exe", parameters, NULL, SW_SHOWNORMAL);
	} else {
		PathRemoveFileSpecW(iniPath);
		shellResult = (INT_PTR)ShellExecuteW(owner, L"open", iniPath, NULL, NULL, SW_SHOWNORMAL);
	}
	if ( shellResult <= 32 ) {
		GuiShowSystemError(owner, L"打开设置位置失败", (DWORD)shellResult);
		return FALSE;
	}
	return TRUE;
}

static BOOL GuiFileTimeOlderThan(const FILETIME* fileTime, const FILETIME* nowTime, ULONGLONG maxAge100ns)
{
	ULARGE_INTEGER t;
	ULARGE_INTEGER now;

	if ( fileTime == NULL || nowTime == NULL ) {
		return FALSE;
	}
	t.LowPart = fileTime->dwLowDateTime;
	t.HighPart = fileTime->dwHighDateTime;
	now.LowPart = nowTime->dwLowDateTime;
	now.HighPart = nowTime->dwHighDateTime;
	return now.QuadPart > t.QuadPart && (now.QuadPart - t.QuadPart) > maxAge100ns;
}

static BOOL GuiDeleteDirectoryTreeBestEffort(const WCHAR* dirPath)
{
	WCHAR pattern[MAX_PATH];
	WCHAR childPath[MAX_PATH];
	WIN32_FIND_DATAW data;
	HANDLE find;

	if ( dirPath == NULL || dirPath[0] == L'\0' ) {
		return FALSE;
	}

	_snwprintf_s(pattern, _countof(pattern), _TRUNCATE, L"%s\\*", dirPath);
	find = FindFirstFileW(pattern, &data);
	if ( find != INVALID_HANDLE_VALUE ) {
		do {
			if ( wcscmp(data.cFileName, L".") == 0 || wcscmp(data.cFileName, L"..") == 0 ) {
				continue;
			}
			_snwprintf_s(childPath, _countof(childPath), _TRUNCATE, L"%s\\%s", dirPath, data.cFileName);
			if ( childPath[0] == L'\0' ) {
				continue;
			}
			if ( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
				(void)GuiDeleteDirectoryTreeBestEffort(childPath);
			} else {
				(void)SetFileAttributesW(childPath, FILE_ATTRIBUTE_NORMAL);
				(void)DeleteFileW(childPath);
			}
		} while ( FindNextFileW(find, &data) );
		FindClose(find);
	}

	(void)SetFileAttributesW(dirPath, FILE_ATTRIBUTE_NORMAL);
	return RemoveDirectoryW(dirPath);
}

void GuiCleanupStaleTempRoots(void)
{
	WCHAR tempPath[MAX_PATH];
	WCHAR rootPath[MAX_PATH];
	WCHAR pattern[MAX_PATH];
	WCHAR childPath[MAX_PATH];
	DWORD cchTemp;
	WIN32_FIND_DATAW data;
	HANDLE find;
	FILETIME nowTime;

	cchTemp = GetTempPathW(_countof(tempPath), tempPath);
	if ( cchTemp == 0 || cchTemp >= _countof(tempPath) ) {
		return;
	}
	_snwprintf_s(rootPath, _countof(rootPath), _TRUNCATE, L"%sxpkgui-open", tempPath);
	if ( rootPath[0] == L'\0' ) {
		return;
	}
	_snwprintf_s(pattern, _countof(pattern), _TRUNCATE, L"%s\\*", rootPath);
	GetSystemTimeAsFileTime(&nowTime);

	find = FindFirstFileW(pattern, &data);
	if ( find == INVALID_HANDLE_VALUE ) {
		return;
	}
	do {
		const FILETIME* timeToCheck;

		if ( wcscmp(data.cFileName, L".") == 0 || wcscmp(data.cFileName, L"..") == 0 ) {
			continue;
		}
		if ( (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 ) {
			continue;
		}
		timeToCheck = (data.ftCreationTime.dwLowDateTime != 0 || data.ftCreationTime.dwHighDateTime != 0) ? &data.ftCreationTime : &data.ftLastWriteTime;
		if ( !GuiFileTimeOlderThan(timeToCheck, &nowTime, XPKGUI_TEMP_ROOT_MAX_AGE_100NS) ) {
			continue;
		}
		_snwprintf_s(childPath, _countof(childPath), _TRUNCATE, L"%s\\%s", rootPath, data.cFileName);
		(void)GuiDeleteDirectoryTreeBestEffort(childPath);
	} while ( FindNextFileW(find, &data) );
	FindClose(find);
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
	if ( !app->listViewSettingsInitialized ) {
		app->showGridLines = TRUE;
		app->fullRowSelect = TRUE;
		app->listViewSettingsInitialized = TRUE;
	}
	if ( !app->sortInitialized ) {
		app->sortColumn = 0;
		app->sortAscending = TRUE;
		app->sortInitialized = TRUE;
	} else if ( app->sortColumn < 0 || app->sortColumn >= XPKGUI_ARCHIVE_COLUMN_COUNT ) {
		app->sortColumn = 0;
	}
	GuiApplyDefaultColumnWidths(app);
	if ( !app->visibleColumns[0] ) {
		GuiSetDefaultColumnVisibility(app);
	}
	app->navHistoryCount = 0;
	app->navHistoryIndex = 0;
	app->navHistoryLocked = FALSE;
	app->launchErrorShown = FALSE;
	app->archiveChangePromptActive = FALSE;
	ZeroMemory(&app->archiveWriteTime, sizeof(app->archiveWriteTime));
	app->archiveFileSize = 0;
	GuiLoadArchiveDefaults(app);
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

void GuiFormatRatio(uint64_t packedSize, uint64_t fileSize, WCHAR* buf, size_t cchBuf)
{
	double ratio;

	if ( buf == NULL || cchBuf == 0 ) {
		return;
	}
	if ( fileSize == 0 ) {
		wcsncpy_s(buf, cchBuf, L"n/a", _TRUNCATE);
		return;
	}
	ratio = ((double)packedSize * 100.0) / (double)fileSize;
	_snwprintf_s(buf, cchBuf, _TRUNCATE, L"%.1f%%", ratio);
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
	WCHAR pane0[160];
	WCHAR pane1[160];
	WCHAR pane2[192];
	WCHAR pane3[192];
	WCHAR pane4[256];
	WCHAR selSizeText[64];
	WCHAR selPackedText[64];
	WCHAR selRatioText[32];
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
		SendMessageW(app->status, SB_SETTEXTW, 1, (LPARAM)L"");
		SendMessageW(app->status, SB_SETTEXTW, 2, (LPARAM)L"");
		SendMessageW(app->status, SB_SETTEXTW, 3, (LPARAM)L"");
		SendMessageW(app->status, SB_SETTEXTW, 4, (LPARAM)L"");
		return;
	}

	pathPack = (app->packType == XPK_PACK_LINUX || app->packType == XPK_PACK_WIN32);
	if ( pathPack ) {
		if ( app->flatView ) {
			wcsncpy_s(pane4, _countof(pane4), L"view=flat", _TRUNCATE);
		} else {
			_snwprintf_s(pane4, _countof(pane4), _TRUNCATE, L"view=/%s", app->currentFolder);
		}
	} else {
		pane4[0] = L'\0';
	}

	selectedRows = GuiArchiveSelectedCount(app);
	selectedFiles = 0;
	selectedSize = 0;
	selectedPacked = 0;
	GuiArchiveGetSelectionSummary(app, &selectedFiles, &selectedSize, &selectedPacked);
	GuiFormatUInt64(selectedSize, selSizeText, _countof(selSizeText));
	GuiFormatUInt64(selectedPacked, selPackedText, _countof(selPackedText));
	GuiFormatRatio(selectedPacked, selectedSize, selRatioText, _countof(selRatioText));

	ZeroMemory(&statInfo, sizeof(statInfo));
	if ( xpkStatGet(app->archive, &statInfo) == XPK_OK ) {
		_snwprintf_s(
			pane0,
			_countof(pane0),
			_TRUNCATE,
			L"type=%d entries=%u shown=%u",
			(int)app->packType,
			statInfo.fileCount,
			(unsigned)app->viewCount);
		_snwprintf_s(
			pane1,
			_countof(pane1),
			_TRUNCATE,
			L"sel=%u rows/%u files",
			(unsigned)selectedRows,
			(unsigned)selectedFiles);
		_snwprintf_s(
			pane2,
			_countof(pane2),
			_TRUNCATE,
			L"size=%s packed=%s ratio=%s",
			selSizeText,
			selPackedText,
			selRatioText);
		_snwprintf_s(
			pane3,
			_countof(pane3),
			_TRUNCATE,
			L"live=%llu holes=%llu meta=%llu table=%llu",
			(unsigned long long)statInfo.liveDataBytes,
			(unsigned long long)statInfo.holeBytes,
			(unsigned long long)statInfo.metaBytes,
			(unsigned long long)statInfo.entryTableBytes);
		SendMessageW(app->status, SB_SETTEXTW, 0, (LPARAM)pane0);
		SendMessageW(app->status, SB_SETTEXTW, 1, (LPARAM)pane1);
		SendMessageW(app->status, SB_SETTEXTW, 2, (LPARAM)pane2);
		SendMessageW(app->status, SB_SETTEXTW, 3, (LPARAM)pane3);
		SendMessageW(app->status, SB_SETTEXTW, 4, (LPARAM)pane4);
	} else {
		_snwprintf_s(pane0, _countof(pane0), _TRUNCATE, L"%S", xpkLastErrorMessage(app->archive));
		SendMessageW(app->status, SB_SETTEXTW, 0, (LPARAM)pane0);
		SendMessageW(app->status, SB_SETTEXTW, 1, (LPARAM)L"");
		SendMessageW(app->status, SB_SETTEXTW, 2, (LPARAM)L"");
		SendMessageW(app->status, SB_SETTEXTW, 3, (LPARAM)L"");
		SendMessageW(app->status, SB_SETTEXTW, 4, (LPARAM)L"");
	}
}

void GuiUpdateEmptyState(GuiApp* app)
{
	const WCHAR* text;

	if ( app == NULL || app->emptyState == NULL ) {
		return;
	}

	text = NULL;
	if ( app->archive == NULL ) {
		text = L"No archive open. Use New, Open, or drag files here.";
	} else if ( app->viewCount == 0 && app->filterText[0] != L'\0' ) {
		text = L"No items match the current filter.";
	} else if ( app->viewCount == 0 ) {
		text = L"This folder is empty.";
	}

	if ( text == NULL ) {
		ShowWindow(app->emptyState, SW_HIDE);
		return;
	}
	SetWindowTextW(app->emptyState, text);
	ShowWindow(app->emptyState, SW_SHOW);
}

static BOOL GuiIsArchiveFormatErrorText(const char* sError)
{
	if ( sError == NULL ) {
		return FALSE;
	}
	return (strcmp(sError, "invalid xpk file header") == 0) ||
		(strcmp(sError, "invalid xpk package format") == 0);
}

static const WCHAR* GuiArchiveErrorCategory(const char* sError)
{
	if ( GuiIsArchiveFormatErrorText(sError) ) {
		return L"格式错误";
	}
	if ( sError != NULL && (strstr(sError, "file") != NULL || strstr(sError, "path") != NULL || strstr(sError, "open") != NULL || strstr(sError, "write") != NULL || strstr(sError, "read") != NULL) ) {
		return L"IO 错误";
	}
	if ( sError != NULL && strstr(sError, "cancel") != NULL ) {
		return L"用户取消";
	}
	return L"xPack API 状态错误";
}

static void GuiShowArchiveErrorInternal(GuiApp* app, const WCHAR* actionText, const WCHAR* archivePath)
{
	WCHAR text[1024];
	WCHAR wideError[768];
	const char* sError;
	const WCHAR* category;

	if ( app != NULL ) {
		app->launchErrorShown = TRUE;
	}

	sError = (app->archive != NULL) ? xpkLastErrorMessage(app->archive) : xpkLastErrorMessage(NULL);
	if ( sError == NULL || sError[0] == '\0' ) {
		sError = "unknown error";
	}
	GuiWideFromUtf8(sError, wideError, _countof(wideError));
	category = GuiArchiveErrorCategory(sError);
	if ( archivePath != NULL && archivePath[0] != L'\0' ) {
		_snwprintf_s(
			text,
			_countof(text),
			_TRUNCATE,
			L"%s失败。\n\n上下文：正在处理 xPack 归档\n文件：%s\n错误类型：%s\nxPack 错误：%s",
			actionText,
			archivePath,
			category,
			wideError);
		if ( GuiIsArchiveFormatErrorText(sError) ) {
			wcsncat_s(
				text,
				_countof(text),
				L"\n\n提示：这通常表示当前打开的文件不是新版 xpk 归档，或临时解压出的内层 .xpk 不是有效归档。"
				L"当前版本不兼容旧版/遗留 xpk 包。"
				L"如果这个文件来自旧工具或历史测试产物，请用当前 xPack 重新创建后再打开。",
				_TRUNCATE);
		}
	} else {
		_snwprintf_s(text, _countof(text), _TRUNCATE, L"%s失败。\n\n错误类型：%s\nxPack 错误：%s", actionText, category, wideError);
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
	GuiShowSystemErrorDetail(hwnd, title, NULL, NULL, err);
}

void GuiShowSystemErrorDetail(HWND hwnd, const WCHAR* title, const WCHAR* context, const WCHAR* path, DWORD err)
{
	WCHAR text[512];
	WCHAR body[1024];
	DWORD len;

	len = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, 0, text, (DWORD)_countof(text), NULL);
	if ( len == 0 ) {
		_snwprintf_s(text, _countof(text), _TRUNCATE, L"系统错误 %lu", err);
	}
	_snwprintf_s(
		body,
		_countof(body),
		_TRUNCATE,
		L"%s%s%s%s%s%s错误码：%lu (0x%08lX)\n系统消息：%s",
		(context != NULL && context[0] != L'\0') ? L"上下文：" : L"",
		(context != NULL && context[0] != L'\0') ? context : L"",
		(context != NULL && context[0] != L'\0') ? L"\n" : L"",
		(path != NULL && path[0] != L'\0') ? L"路径：" : L"",
		(path != NULL && path[0] != L'\0') ? path : L"",
		(path != NULL && path[0] != L'\0') ? L"\n" : L"",
		(unsigned long)err,
		(unsigned long)err,
		text);
	if ( err == ERROR_ACCESS_DENIED || err == ERROR_SHARING_VIOLATION || err == ERROR_LOCK_VIOLATION || err == ERROR_WRITE_PROTECT ) {
		wcsncat_s(
			body,
			_countof(body),
			L"\n\n提示：请检查文件是否只读、是否被其他程序占用，或当前用户是否有写入权限。",
			_TRUNCATE);
	}
	MessageBoxW(hwnd, body, title, MB_OK | MB_ICONERROR);
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

	if ( _wcsicmp(argv[1], L"/?") == 0 ||
		_wcsicmp(argv[1], L"-?") == 0 ||
		_wcsicmp(argv[1], L"/help") == 0 ||
		_wcsicmp(argv[1], L"-help") == 0 ||
		_wcsicmp(argv[1], L"--help") == 0 ) {
		app->launch.command = GUI_LAUNCH_HELP;
		return 0;
	} else if ( _wcsicmp(argv[1], L"/shell-add") == 0 || _wcsicmp(argv[1], L"-shell-add") == 0 ) {
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
		if ( !GuiIsSmokeMode() ) {
			MessageBoxW(NULL, L"不支持的命令行参数。请使用 /? 查看帮助。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		}
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
			if ( !GuiIsSmokeMode() ) {
				MessageBoxW(NULL, L"归档路径缺失。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
			}
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
		if ( !GuiIsSmokeMode() ) {
			MessageBoxW(NULL, L"输入项数量无效。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		}
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
