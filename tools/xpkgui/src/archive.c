#include "app.h"
#include "resources/resource.h"

#include <commdlg.h>
#include <errno.h>
#include <process.h>
#include <shellapi.h>
#include <shlobj_core.h>
#include <shlwapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "Shlwapi.lib")

static const WCHAR* arrGuiMethodNames[16] = {
	L"STORE-0",
	L"LZ4-1",
	L"LZ4-4",
	L"LZ4HC-8",
	L"LZ4HC-12",
	L"ZSTD-fast",
	L"ZSTD-dfast",
	L"ZSTD-greedy",
	L"ZSTD-lazy",
	L"ZSTD-lazy2",
	L"ZSTD-btlazy2",
	L"ZSTD-btopt",
	L"ZSTD-btultra",
	L"ZSTD-btultra2",
	L"LZMA2-6",
	L"LZMA2-9"
};

typedef struct GuiFileTypeName {
	uint8_t value;
	const WCHAR* label;
	const WCHAR* token;
} GuiFileTypeName;

static const GuiFileTypeName arrGuiFileTypeNames[] = {
	{ 0, L"Unknown", L"unknown" },
	{ 1, L"Binary", L"binary" },
	{ 2, L"Text", L"text" },
	{ 3, L"Image", L"image" },
	{ 4, L"Audio", L"audio" },
	{ 5, L"Video", L"video" },
	{ 6, L"Archive", L"archive" },
	{ 15, L"Folder", L"folder" }
};

static LONG g_openTempSequence = 0;

#define GUI_ARCHIVE_COLUMN_COUNT XPKGUI_ARCHIVE_COLUMN_COUNT
#define XPKGUI_INLINE_VIEW_LIMIT (8ull * 1024ull * 1024ull)

static BOOL GuiPathMatchFolderPrefix(const WCHAR* fullPath, const WCHAR* folder, const WCHAR** remainderOut);
static GuiViewItem* GuiArchiveGetViewItemByListIndex(GuiApp* app, int index);
static BOOL GuiArchiveFolderExists(GuiApp* app, const WCHAR* folder);
static BOOL GuiCollectSelectedSourceIndexes(GuiApp* app, size_t** indexesOut, size_t* countOut);
static BOOL GuiQueryArchiveDiskStamp(const WCHAR* archivePath, FILETIME* writeTimeOut, uint64_t* fileSizeOut);
static BOOL GuiUpdateArchiveDiskStamp(GuiApp* app);
static WCHAR* GuiAllocEmptyWideText(void);
static BOOL GuiEncodeMetaUtf8Text(const WCHAR* text, void** outData, uint32_t* outSize);
static uint8_t GuiEntryFileType(uint32_t flag);
static void GuiFormatFileType(uint8_t fileType, WCHAR* buf, size_t cchBuf);
static void GuiBuildDuplicateLeafName(const WCHAR* sourceName, UINT copyIndex, WCHAR* outName, size_t cchOutName);
static BOOL GuiBuildRenameTargetPath(GuiApp* app, const WCHAR* inputValue, WCHAR* targetPath, size_t cchTargetPath);
static BOOL GuiSelectViewItemByFullPath(GuiApp* app, GuiViewItemKind kind, const WCHAR* fullPath);

static const WCHAR* arrGuiArchiveColumnNames[GUI_ARCHIVE_COLUMN_COUNT] = {
	L"Name / Path",
	L"Size",
	L"Packed",
	L"Ratio",
	L"Method",
	L"File Type",
	L"Modified",
	L"ID",
	L"Hash",
	L"Attr"
};

static const WCHAR* GuiArchiveGetViewItemColumnText(const GuiViewItem* item, int logicalColumn)
{
	if ( item == NULL ) {
		return L"";
	}
	switch ( logicalColumn ) {
		case 0: return item->name;
		case 1: return item->sizeText;
		case 2: return item->packedText;
		case 3: return item->ratioText;
		case 4: return item->methodText;
		case 5: return item->fileTypeText;
		case 6: return item->modifiedText;
		case 7: return item->idText;
		case 8: return item->hashText;
		case 9: return item->attrText;
		default: return L"";
	}
}

static void GuiBuildExtractOutputPath(xpkPackType packType, const WCHAR* destPath, const GuiArchiveItem* item, WCHAR* outPath, size_t cchOutPath)
{
	size_t i;

	if ( packType == XPK_PACK_CORE ) {
		_snwprintf_s(outPath, cchOutPath, _TRUNCATE, L"%s\\entry_%04u.bin", destPath, item->pos);
	} else if ( packType == XPK_PACK_INDEX ) {
		_snwprintf_s(outPath, cchOutPath, _TRUNCATE, L"%s\\%lld.bin", destPath, (long long)item->fileIndex);
	} else {
		_snwprintf_s(outPath, cchOutPath, _TRUNCATE, L"%s\\%s", destPath, item->name);
	}
	for ( i = 0; outPath[i] != L'\0'; ++i ) {
		if ( outPath[i] == L'/' ) {
			outPath[i] = L'\\';
		}
	}
}

static uint32_t GuiHashWideText(const WCHAR* text)
{
	uint32_t hash;

	hash = 2166136261u;
	if ( text == NULL ) {
		return hash;
	}
	while ( *text != L'\0' ) {
		hash ^= (uint32_t)(*text++);
		hash *= 16777619u;
	}
	return hash;
}

static void GuiSanitizeFileComponent(const WCHAR* src, WCHAR* dst, size_t cchDst)
{
	size_t i;
	size_t out;

	if ( dst == NULL || cchDst == 0 ) {
		return;
	}

	out = 0;
	if ( src != NULL ) {
		for ( i = 0; src[i] != L'\0' && out + 1 < cchDst; ++i ) {
			WCHAR ch;

			ch = src[i];
			if ( ch < 32 || ch == L'<' || ch == L'>' || ch == L':' || ch == L'"' || ch == L'/' || ch == L'\\' || ch == L'|' || ch == L'?' || ch == L'*' ) {
				ch = L'_';
			}
			dst[out++] = ch;
		}
	}
	dst[out] = L'\0';
	if ( dst[0] == L'\0' ) {
		wcsncpy_s(dst, cchDst, L"archive", _TRUNCATE);
	}
}

static BOOL GuiBuildOpenTempRootPath(const GuiApp* app, WCHAR* pathBuf, size_t cchPathBuf)
{
	WCHAR tempPath[MAX_PATH];
	WCHAR archiveName[MAX_PATH];
	WCHAR safeName[MAX_PATH];
	DWORD cchTemp;
	const WCHAR* fileName;
	LONG sequence;

	if ( app == NULL || pathBuf == NULL || cchPathBuf == 0 ) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	cchTemp = GetTempPathW(_countof(tempPath), tempPath);
	if ( cchTemp == 0 || cchTemp >= _countof(tempPath) ) {
		if ( GetLastError() == ERROR_SUCCESS ) {
			SetLastError(ERROR_INSUFFICIENT_BUFFER);
		}
		return FALSE;
	}

	fileName = PathFindFileNameW(app->archivePath);
	if ( fileName == NULL || fileName[0] == L'\0' ) {
		fileName = L"archive";
	}
	wcsncpy_s(archiveName, _countof(archiveName), fileName, _TRUNCATE);
	PathRemoveExtensionW(archiveName);
	GuiSanitizeFileComponent(archiveName, safeName, _countof(safeName));
	sequence = InterlockedIncrement(&g_openTempSequence);

	_snwprintf_s(
		pathBuf,
		cchPathBuf,
		_TRUNCATE,
		L"%sxpkgui-open\\%s_%08X_%08lX_%08lX",
		tempPath,
		safeName,
		(unsigned)GuiHashWideText(app->archivePath),
		(unsigned long)GetCurrentProcessId(),
		(unsigned long)sequence);
	return pathBuf[0] != L'\0';
}

static BOOL GuiPathIsXpkArchive(const WCHAR* path)
{
	const WCHAR* ext;

	if ( path == NULL || path[0] == L'\0' ) {
		return FALSE;
	}
	ext = PathFindExtensionW(path);
	return ext != NULL && _wcsicmp(ext, L".xpk") == 0;
}

static BOOL GuiLaunchArchiveInNewGui(HWND owner, const WCHAR* archivePath)
{
	WCHAR exePath[MAX_PATH];
	WCHAR parameters[MAX_PATH * 2];
	INT_PTR shellResult;

	if ( archivePath == NULL || archivePath[0] == L'\0' ) {
		return FALSE;
	}
	if ( GetModuleFileNameW(NULL, exePath, _countof(exePath)) == 0 ) {
		return FALSE;
	}

	_snwprintf_s(parameters, _countof(parameters), _TRUNCATE, L"\"%s\"", archivePath);
	shellResult = (INT_PTR)ShellExecuteW(owner, L"open", exePath, parameters, NULL, SW_SHOWNORMAL);
	if ( shellResult <= 32 ) {
		SetLastError((DWORD)shellResult);
	}
	return shellResult > 32;
}

static BOOL GuiShowOpenWithDialogForPath(HWND owner, const WCHAR* path)
{
	OPENASINFO openAsInfo;
	HRESULT hr;

	if ( path == NULL || path[0] == L'\0' ) {
		return FALSE;
	}

	ZeroMemory(&openAsInfo, sizeof(openAsInfo));
	openAsInfo.pcszFile = path;
	openAsInfo.pcszClass = NULL;
	openAsInfo.oaifInFlags = OAIF_EXEC;

	hr = SHOpenWithDialog(owner, &openAsInfo);
	if ( hr == HRESULT_FROM_WIN32(ERROR_CANCELLED) ) {
		SetLastError(ERROR_CANCELLED);
		return FALSE;
	}
	if ( FAILED(hr) ) {
		WCHAR text[1024];

		_snwprintf_s(
			text,
			_countof(text),
			_TRUNCATE,
			L"文件已提取到临时目录，但无法打开“打开方式”对话框。\n\n%s\n\nHRESULT: 0x%08lX",
			path,
			(unsigned long)hr);
		MessageBoxW(owner, text, XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		SetLastError(HRESULT_CODE(hr));
		return FALSE;
	}
	return TRUE;
}

static BOOL GuiLaunchPathWithNotepad(HWND owner, const WCHAR* path)
{
	WCHAR commandLine[(MAX_PATH * 2) + 32];
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	DWORD waitRet;

	(void)owner;

	if ( path == NULL || path[0] == L'\0' ) {
		return FALSE;
	}

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWNORMAL;
	_snwprintf_s(commandLine, _countof(commandLine), _TRUNCATE, L"notepad.exe \"%s\"", path);
	if ( !CreateProcessW(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi) ) {
		return FALSE;
	}

	waitRet = WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return waitRet == WAIT_OBJECT_0;
}

static BOOL GuiOpenExtractedPath(HWND owner, const WCHAR* path)
{
	INT_PTR shellResult;

	if ( path == NULL || path[0] == L'\0' ) {
		return FALSE;
	}

	shellResult = (INT_PTR)ShellExecuteW(owner, L"open", path, NULL, NULL, SW_SHOWNORMAL);
	if ( shellResult > 32 ) {
		return TRUE;
	}
	SetLastError((DWORD)shellResult);
	if ( GuiShowOpenWithDialogForPath(owner, path) ) {
		return TRUE;
	}
	if ( GetLastError() != ERROR_CANCELLED ) {
		GuiShowSystemErrorDetail(owner, L"启动外部程序失败", L"已提取临时文件，但系统无法用默认程序打开，也无法完成“打开方式”回退。", path, GetLastError());
	}
	return FALSE;
}

static BOOL GuiGetFileStamp(const WCHAR* path, WIN32_FILE_ATTRIBUTE_DATA* stampOut)
{
	if ( path == NULL || path[0] == L'\0' || stampOut == NULL ) {
		return FALSE;
	}
	return GetFileAttributesExW(path, GetFileExInfoStandard, stampOut);
}

static BOOL GuiFileStampEquals(const WIN32_FILE_ATTRIBUTE_DATA* left, const WIN32_FILE_ATTRIBUTE_DATA* right)
{
	if ( left == NULL || right == NULL ) {
		return FALSE;
	}
	return left->dwFileAttributes == right->dwFileAttributes
		&& left->nFileSizeHigh == right->nFileSizeHigh
		&& left->nFileSizeLow == right->nFileSizeLow
		&& left->ftLastWriteTime.dwLowDateTime == right->ftLastWriteTime.dwLowDateTime
		&& left->ftLastWriteTime.dwHighDateTime == right->ftLastWriteTime.dwHighDateTime;
}

static const WCHAR* GuiPackTypeLabel(xpkPackType type)
{
	switch ( type ) {
		case XPK_PACK_CORE: return L"Core";
		case XPK_PACK_INDEX: return L"Index";
		case XPK_PACK_LINUX: return L"Linux";
		case XPK_PACK_WIN32: return L"Win32";
		default: return L"Unknown";
	}
}

static BOOL GuiIsPathPackType(xpkPackType type)
{
	return type == XPK_PACK_LINUX || type == XPK_PACK_WIN32;
}

static BOOL GuiEnsureItemCapacity(GuiApp* app, size_t needCount)
{
	size_t newCap;
	GuiArchiveItem* pNew;

	if ( needCount <= app->itemCapacity ) {
		return TRUE;
	}

	newCap = (app->itemCapacity == 0) ? 64 : app->itemCapacity;
	while ( newCap < needCount ) {
		newCap *= 2;
	}

	pNew = (GuiArchiveItem*)realloc(app->items, newCap * sizeof(GuiArchiveItem));
	if ( pNew == NULL ) {
		MessageBoxW(app->window, L"内存不足。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	app->items = pNew;
	app->itemCapacity = newCap;
	return TRUE;
}

static BOOL GuiEnsureViewCapacity(GuiApp* app, size_t needCount)
{
	size_t newCap;
	GuiViewItem* pNew;

	if ( needCount <= app->viewCapacity ) {
		return TRUE;
	}

	newCap = (app->viewCapacity == 0) ? 64 : app->viewCapacity;
	while ( newCap < needCount ) {
		newCap *= 2;
	}

	pNew = (GuiViewItem*)realloc(app->viewItems, newCap * sizeof(GuiViewItem));
	if ( pNew == NULL ) {
		MessageBoxW(app->window, L"内存不足。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	app->viewItems = pNew;
	app->viewCapacity = newCap;
	return TRUE;
}

static void GuiClearItems(GuiApp* app)
{
	free(app->items);
	app->items = NULL;
	app->itemCount = 0;
	app->itemCapacity = 0;
}

static void GuiClearViewItems(GuiApp* app)
{
	free(app->viewItems);
	app->viewItems = NULL;
	app->viewCount = 0;
	app->viewCapacity = 0;
}

static const WCHAR* GuiFindPathSeparator(const WCHAR* path)
{
	for ( ; *path != L'\0'; ++path ) {
		if ( *path == L'/' || *path == L'\\' ) {
			return path;
		}
	}
	return NULL;
}

static void GuiNormalizeViewPath(const WCHAR* src, WCHAR* dst, size_t cchDst)
{
	size_t i;
	size_t out;

	if ( dst == NULL || cchDst == 0 ) {
		return;
	}

	if ( src == NULL ) {
		dst[0] = L'\0';
		return;
	}

	while ( *src == L'/' || *src == L'\\' ) {
		src++;
	}

	out = 0;
	for ( i = 0; src[i] != L'\0' && out + 1 < cchDst; ++i ) {
		WCHAR ch;

		ch = src[i];
		if ( ch == L'\\' ) {
			ch = L'/';
		}
		dst[out++] = ch;
	}
	dst[out] = L'\0';

	while ( out > 0 && dst[out - 1] == L'/' ) {
		dst[--out] = L'\0';
	}
}

static void GuiBuildChildViewPath(const WCHAR* folder, const WCHAR* child, WCHAR* dst, size_t cchDst)
{
	if ( folder == NULL || folder[0] == L'\0' ) {
		wcsncpy_s(dst, cchDst, child, _TRUNCATE);
	} else {
		_snwprintf_s(dst, cchDst, _TRUNCATE, L"%s/%s", folder, child);
	}
}

static void GuiGetParentViewPath(const WCHAR* folder, WCHAR* dst, size_t cchDst)
{
	WCHAR temp[XPKGUI_ITEM_TEXT];
	WCHAR* slash;

	GuiNormalizeViewPath(folder, temp, _countof(temp));
	slash = wcsrchr(temp, L'/');
	if ( slash != NULL ) {
		*slash = L'\0';
	} else {
		temp[0] = L'\0';
	}
	wcsncpy_s(dst, cchDst, temp, _TRUNCATE);
}

static BOOL GuiPathMatchFolderPrefix(const WCHAR* fullPath, const WCHAR* folder, const WCHAR** remainderOut)
{
	size_t folderLen;

	if ( folder == NULL || folder[0] == L'\0' ) {
		if ( remainderOut != NULL ) {
			*remainderOut = fullPath;
		}
		return fullPath != NULL && fullPath[0] != L'\0';
	}

	folderLen = wcslen(folder);
	if ( _wcsnicmp(fullPath, folder, folderLen) != 0 ) {
		return FALSE;
	}
	if ( fullPath[folderLen] != L'/' && fullPath[folderLen] != L'\\' ) {
		return FALSE;
	}
	if ( fullPath[folderLen + 1] == L'\0' ) {
		return FALSE;
	}
	if ( remainderOut != NULL ) {
		*remainderOut = fullPath + folderLen + 1;
	}
	return TRUE;
}

static BOOL GuiPathGetCurrentChild(const WCHAR* fullPath, const WCHAR* folder, WCHAR* childName, size_t cchChildName, BOOL* isDirOut)
{
	const WCHAR* remainder;
	const WCHAR* slash;
	size_t cchSegment;

	if ( fullPath == NULL || fullPath[0] == L'\0' ) {
		return FALSE;
	}

	if ( folder == NULL || folder[0] == L'\0' ) {
		remainder = fullPath;
	} else if ( !GuiPathMatchFolderPrefix(fullPath, folder, &remainder) ) {
		return FALSE;
	}

	slash = GuiFindPathSeparator(remainder);
	if ( slash == NULL ) {
		wcsncpy_s(childName, cchChildName, remainder, _TRUNCATE);
		if ( isDirOut != NULL ) {
			*isDirOut = FALSE;
		}
		return remainder[0] != L'\0';
	}

	cchSegment = (size_t)(slash - remainder);
	if ( cchSegment == 0 ) {
		return FALSE;
	}
	if ( cchSegment >= cchChildName ) {
		cchSegment = cchChildName - 1;
	}
	wcsncpy_s(childName, cchChildName, remainder, cchSegment);
	childName[cchSegment] = L'\0';
	if ( isDirOut != NULL ) {
		*isDirOut = TRUE;
	}
	return TRUE;
}

static BOOL GuiAppendViewItem(GuiApp* app, const GuiViewItem* item)
{
	if ( !GuiEnsureViewCapacity(app, app->viewCount + 1) ) {
		return FALSE;
	}
	app->viewItems[app->viewCount++] = *item;
	return TRUE;
}

static BOOL GuiViewHasDirectory(const GuiApp* app, const WCHAR* fullPath)
{
	size_t i;

	for ( i = 0; i < app->viewCount; ++i ) {
		if ( app->viewItems[i].kind == GUI_VIEW_ITEM_DIR && _wcsicmp(app->viewItems[i].fullPath, fullPath) == 0 ) {
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL GuiAddViewParentItem(GuiApp* app)
{
	GuiViewItem item;

	ZeroMemory(&item, sizeof(item));
	item.kind = GUI_VIEW_ITEM_PARENT;
	item.sourceIndex = (size_t)-1;
	wcsncpy_s(item.name, _countof(item.name), L"..", _TRUNCATE);
	wcsncpy_s(item.methodText, _countof(item.methodText), L"Up", _TRUNCATE);
	wcsncpy_s(item.fileTypeText, _countof(item.fileTypeText), L"-", _TRUNCATE);
	GuiGetParentViewPath(app->currentFolder, item.fullPath, _countof(item.fullPath));
	return GuiAppendViewItem(app, &item);
}

static BOOL GuiAddViewDirectoryItem(GuiApp* app, const WCHAR* childName, const WCHAR* fullPath)
{
	GuiViewItem item;

	if ( GuiViewHasDirectory(app, fullPath) ) {
		return TRUE;
	}

	ZeroMemory(&item, sizeof(item));
	item.kind = GUI_VIEW_ITEM_DIR;
	item.sourceIndex = (size_t)-1;
	_snwprintf_s(item.name, _countof(item.name), _TRUNCATE, L"%s/", childName);
	wcsncpy_s(item.fullPath, _countof(item.fullPath), fullPath, _TRUNCATE);
	wcsncpy_s(item.methodText, _countof(item.methodText), L"Dir", _TRUNCATE);
	wcsncpy_s(item.fileTypeText, _countof(item.fileTypeText), L"Folder", _TRUNCATE);
	return GuiAppendViewItem(app, &item);
}

static BOOL GuiAddViewFileItem(GuiApp* app, size_t sourceIndex, const WCHAR* displayName)
{
	GuiViewItem item;
	const GuiArchiveItem* src;

	src = &app->items[sourceIndex];
	ZeroMemory(&item, sizeof(item));
	item.kind = GUI_VIEW_ITEM_FILE;
	item.sourceIndex = sourceIndex;
	item.pos = src->pos;
	item.fileIndex = src->fileIndex;
	item.fileSize = src->fileSize;
	item.packedSize = src->packedSize;
	item.fileHash = src->fileHash;
	item.attr = src->attr;
	item.modifyTime = src->modifyTime;
	wcsncpy_s(item.name, _countof(item.name), displayName, _TRUNCATE);
	wcsncpy_s(item.fullPath, _countof(item.fullPath), src->name, _TRUNCATE);
	wcsncpy_s(item.sizeText, _countof(item.sizeText), src->sizeText, _TRUNCATE);
	wcsncpy_s(item.packedText, _countof(item.packedText), src->packedText, _TRUNCATE);
	wcsncpy_s(item.ratioText, _countof(item.ratioText), src->ratioText, _TRUNCATE);
	wcsncpy_s(item.methodText, _countof(item.methodText), src->methodText, _TRUNCATE);
	wcsncpy_s(item.fileTypeText, _countof(item.fileTypeText), src->fileTypeText, _TRUNCATE);
	wcsncpy_s(item.modifiedText, _countof(item.modifiedText), src->modifiedText, _TRUNCATE);
	wcsncpy_s(item.idText, _countof(item.idText), src->idText, _TRUNCATE);
	wcsncpy_s(item.hashText, _countof(item.hashText), src->hashText, _TRUNCATE);
	wcsncpy_s(item.attrText, _countof(item.attrText), src->attrText, _TRUNCATE);
	return GuiAppendViewItem(app, &item);
}

static int GuiEnumArchiveCallback(xpkObject xpk, uint32_t pos, const void* info, void* userData)
{
	GuiApp* app;
	GuiArchiveItem* item;

	(void)xpk;

	app = (GuiApp*)userData;
	if ( !GuiEnsureItemCapacity(app, app->itemCount + 1) ) {
		return XPK_ERR_MEMORY;
	}

	item = &app->items[app->itemCount++];
	ZeroMemory(item, sizeof(*item));
	item->pos = pos;

	if ( app->packType == XPK_PACK_INDEX ) {
		const xpkFileInfoIndex* pInfo;
		pInfo = (const xpkFileInfoIndex*)info;
		item->fileIndex = pInfo->fileIndex;
		item->fileSize = pInfo->fileSize;
		item->packedSize = pInfo->dataSize;
		item->flag = pInfo->flag;
		item->fileHash = pInfo->fileHash;
		_snwprintf_s(item->name, _countof(item->name), _TRUNCATE, L"%lld", (long long)pInfo->fileIndex);
		_snwprintf_s(item->idText, _countof(item->idText), _TRUNCATE, L"%lld", (long long)pInfo->fileIndex);
	} else if ( app->packType == XPK_PACK_LINUX || app->packType == XPK_PACK_WIN32 ) {
		const xpkFileInfoPath* pInfo;
		pInfo = (const xpkFileInfoPath*)info;
		item->fileSize = pInfo->fileSize;
		item->packedSize = pInfo->dataSize;
		item->flag = pInfo->flag;
		item->fileHash = pInfo->fileHash;
		item->attr = pInfo->platformAttr;
		item->createTime = (xtime)pInfo->createTime;
		item->modifyTime = (xtime)pInfo->modifyTime;
		item->accessTime = (xtime)pInfo->accessTime;
		strncpy_s(item->packagePath, sizeof(item->packagePath), pInfo->pathBytes, _TRUNCATE);
		GuiWideFromUtf8(pInfo->pathBytes, item->name, _countof(item->name));
		_snwprintf_s(item->idText, _countof(item->idText), _TRUNCATE, L"%u", pos);
		_snwprintf_s(item->attrText, _countof(item->attrText), _TRUNCATE, L"0x%08X", pInfo->platformAttr);
	} else {
		const xpkFileInfo* pInfo;
		pInfo = (const xpkFileInfo*)info;
		item->fileSize = pInfo->fileSize;
		item->packedSize = pInfo->dataSize;
		item->flag = pInfo->flag;
		item->fileHash = pInfo->fileHash;
		_snwprintf_s(item->name, _countof(item->name), _TRUNCATE, L"Entry %u", pos);
		_snwprintf_s(item->idText, _countof(item->idText), _TRUNCATE, L"%u", pos);
	}

	GuiFormatUInt64(item->fileSize, item->sizeText, _countof(item->sizeText));
	GuiFormatUInt64(item->packedSize, item->packedText, _countof(item->packedText));
	GuiFormatRatio(item->packedSize, item->fileSize, item->ratioText, _countof(item->ratioText));
	wcsncpy_s(item->methodText, _countof(item->methodText), arrGuiMethodNames[item->flag & XPK_FLAG_COMP_MASK], _TRUNCATE);
	GuiFormatFileType(GuiEntryFileType(item->flag), item->fileTypeText, _countof(item->fileTypeText));
	GuiFormatTime(item->modifyTime, item->modifiedText, _countof(item->modifiedText));
	_snwprintf_s(item->hashText, _countof(item->hashText), _TRUNCATE, L"0x%08X", item->fileHash);
	return XPK_OK;
}

static BOOL GuiLoadArchiveItems(GuiApp* app)
{
	int iRet;

	GuiClearItems(app);
	if ( app->archive == NULL ) {
		return TRUE;
	}

	iRet = xpkEach(app->archive, GuiEnumArchiveCallback, app);
	if ( iRet != XPK_OK ) {
		GuiShowArchiveError(app, L"读取归档条目");
		return FALSE;
	}
	return TRUE;
}

static void GuiRefreshColumns(GuiApp* app)
{
	LVCOLUMNW col;
	int i;
	int visibleIndex;

	if ( app->list == NULL ) {
		return;
	}

	GuiCaptureColumnWidths(app);
	while ( ListView_DeleteColumn(app->list, 0) ) {
	}

	ZeroMemory(&col, sizeof(col));
	col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	visibleIndex = 0;
	for ( i = 0; i < GUI_ARCHIVE_COLUMN_COUNT; ++i ) {
		if ( !GuiIsArchiveColumnVisible(app, i) ) {
			continue;
		}
		col.pszText = (LPWSTR)arrGuiArchiveColumnNames[i];
		col.cx = app->columnWidths[i] > 0 ? app->columnWidths[i] : 80;
		col.iSubItem = visibleIndex;
		ListView_InsertColumn(app->list, visibleIndex, &col);
		visibleIndex++;
	}
}

typedef struct GuiSortContext {
	int column;
	BOOL ascending;
} GuiSortContext;

typedef struct GuiSelectionSnapshotItem {
	GuiViewItemKind kind;
	WCHAR fullPath[XPKGUI_ITEM_TEXT];
} GuiSelectionSnapshotItem;

static BOOL GuiSnapshotMatchesViewItem(const GuiSelectionSnapshotItem* snapshot, const GuiViewItem* item)
{
	if ( snapshot == NULL || item == NULL ) {
		return FALSE;
	}
	return snapshot->kind == item->kind && _wcsicmp(snapshot->fullPath, item->fullPath) == 0;
}

static void GuiCaptureViewSelection(
	GuiApp* app,
	GuiSelectionSnapshotItem** selectedOut,
	size_t* selectedCountOut,
	GuiSelectionSnapshotItem* focusedOut,
	BOOL* hasFocusedOut)
{
	int index;
	int focusIndex;
	GuiSelectionSnapshotItem* selected;
	size_t selectedCount;

	*selectedOut = NULL;
	*selectedCountOut = 0;
	if ( hasFocusedOut != NULL ) {
		*hasFocusedOut = FALSE;
	}
	if ( app == NULL || app->list == NULL || app->viewItems == NULL ) {
		return;
	}

	selected = NULL;
	selectedCount = 0;
	index = -1;
	while ( TRUE ) {
		GuiViewItem* viewItem;
		GuiSelectionSnapshotItem* newSelected;

		index = ListView_GetNextItem(app->list, index, LVNI_SELECTED);
		if ( index < 0 ) {
			break;
		}
		viewItem = GuiArchiveGetViewItemByListIndex(app, index);
		if ( viewItem == NULL ) {
			continue;
		}
		newSelected = (GuiSelectionSnapshotItem*)realloc(selected, (selectedCount + 1) * sizeof(GuiSelectionSnapshotItem));
		if ( newSelected == NULL ) {
			free(selected);
			selected = NULL;
			selectedCount = 0;
			break;
		}
		selected = newSelected;
		selected[selectedCount].kind = viewItem->kind;
		wcsncpy_s(selected[selectedCount].fullPath, _countof(selected[selectedCount].fullPath), viewItem->fullPath, _TRUNCATE);
		selectedCount++;
	}

	if ( focusedOut != NULL && hasFocusedOut != NULL ) {
		focusIndex = ListView_GetNextItem(app->list, -1, LVNI_FOCUSED);
		if ( focusIndex >= 0 ) {
			GuiViewItem* viewItem;

			viewItem = GuiArchiveGetViewItemByListIndex(app, focusIndex);
			if ( viewItem != NULL ) {
				focusedOut->kind = viewItem->kind;
				wcsncpy_s(focusedOut->fullPath, _countof(focusedOut->fullPath), viewItem->fullPath, _TRUNCATE);
				*hasFocusedOut = TRUE;
			}
		}
	}

	*selectedOut = selected;
	*selectedCountOut = selectedCount;
}

static void GuiRestoreViewSelection(
	GuiApp* app,
	const GuiSelectionSnapshotItem* selected,
	size_t selectedCount,
	const GuiSelectionSnapshotItem* focused,
	BOOL hasFocused)
{
	size_t i;
	int focusIndex;
	int firstSelectedIndex;

	if ( app == NULL || app->list == NULL ) {
		return;
	}

	focusIndex = -1;
	firstSelectedIndex = -1;
	for ( i = 0; i < app->viewCount; ++i ) {
		size_t j;
		BOOL isSelected;

		isSelected = FALSE;
		for ( j = 0; j < selectedCount; ++j ) {
			if ( GuiSnapshotMatchesViewItem(&selected[j], &app->viewItems[i]) ) {
				isSelected = TRUE;
				break;
			}
		}
		if ( isSelected ) {
			ListView_SetItemState(app->list, (int)i, LVIS_SELECTED, LVIS_SELECTED);
			if ( firstSelectedIndex < 0 ) {
				firstSelectedIndex = (int)i;
			}
		}
		if ( hasFocused && GuiSnapshotMatchesViewItem(focused, &app->viewItems[i]) ) {
			focusIndex = (int)i;
		}
	}

	if ( focusIndex < 0 ) {
		focusIndex = firstSelectedIndex;
	}
	if ( focusIndex >= 0 ) {
		ListView_SetItemState(app->list, focusIndex, LVIS_FOCUSED, LVIS_FOCUSED);
		ListView_SetSelectionMark(app->list, focusIndex);
		ListView_EnsureVisible(app->list, focusIndex, FALSE);
	}
}

static int GuiCompareSigned64(int64_t left, int64_t right)
{
	if ( left < right ) return -1;
	if ( left > right ) return 1;
	return 0;
}

static int GuiCompareUnsigned64(uint64_t left, uint64_t right)
{
	if ( left < right ) return -1;
	if ( left > right ) return 1;
	return 0;
}

static int GuiCompareTextInsensitive(const WCHAR* left, const WCHAR* right)
{
	int cmp;

	cmp = _wcsicmp(left, right);
	if ( cmp < 0 ) return -1;
	if ( cmp > 0 ) return 1;
	return 0;
}

static int GuiCompareRatio(uint64_t leftPacked, uint64_t leftSize, uint64_t rightPacked, uint64_t rightSize)
{
	double leftRatio;
	double rightRatio;

	if ( leftSize == 0 && rightSize == 0 ) {
		return GuiCompareUnsigned64(leftPacked, rightPacked);
	}
	if ( leftSize == 0 ) {
		return -1;
	}
	if ( rightSize == 0 ) {
		return 1;
	}
	leftRatio = (double)leftPacked / (double)leftSize;
	rightRatio = (double)rightPacked / (double)rightSize;
	if ( leftRatio < rightRatio ) return -1;
	if ( leftRatio > rightRatio ) return 1;
	return 0;
}

static int __cdecl GuiArchiveItemCompare(void* context, const void* leftPtr, const void* rightPtr)
{
	const GuiSortContext* sort;
	const GuiArchiveItem* left;
	const GuiArchiveItem* right;
	int cmp;

	sort = (const GuiSortContext*)context;
	left = (const GuiArchiveItem*)leftPtr;
	right = (const GuiArchiveItem*)rightPtr;
	cmp = 0;

	switch ( sort->column ) {
		case 0:
			cmp = GuiCompareTextInsensitive(left->name, right->name);
			break;
		case 1:
			cmp = GuiCompareUnsigned64(left->fileSize, right->fileSize);
			break;
		case 2:
			cmp = GuiCompareUnsigned64(left->packedSize, right->packedSize);
			break;
		case 3:
			cmp = GuiCompareRatio(left->packedSize, left->fileSize, right->packedSize, right->fileSize);
			break;
		case 4:
			cmp = GuiCompareTextInsensitive(left->methodText, right->methodText);
			break;
		case 5:
			cmp = GuiCompareTextInsensitive(left->fileTypeText, right->fileTypeText);
			break;
		case 6:
			cmp = GuiCompareSigned64((int64_t)left->modifyTime, (int64_t)right->modifyTime);
			break;
		case 7:
			if ( left->fileIndex != 0 || right->fileIndex != 0 ) {
				cmp = GuiCompareSigned64(left->fileIndex, right->fileIndex);
			} else {
				cmp = GuiCompareUnsigned64(left->pos, right->pos);
			}
			break;
		case 8:
			cmp = GuiCompareUnsigned64(left->fileHash, right->fileHash);
			break;
		case 9:
			cmp = GuiCompareUnsigned64(left->attr, right->attr);
			break;
		default:
			cmp = GuiCompareTextInsensitive(left->name, right->name);
			break;
	}

	if ( cmp == 0 ) {
		cmp = GuiCompareTextInsensitive(left->name, right->name);
	}
	if ( cmp == 0 ) {
		cmp = GuiCompareUnsigned64(left->pos, right->pos);
	}
	return sort->ascending ? cmp : -cmp;
}

static void GuiSortArchiveItems(GuiApp* app)
{
	GuiSortContext sort;

	if ( app->items == NULL || app->itemCount <= 1 ) {
		return;
	}

	sort.column = app->sortColumn;
	sort.ascending = app->sortAscending;
	qsort_s(app->items, app->itemCount, sizeof(GuiArchiveItem), GuiArchiveItemCompare, &sort);
}

static void GuiUpdateSortHeader(GuiApp* app)
{
	HWND header;
	int i;

	if ( app->list == NULL ) {
		return;
	}

	header = ListView_GetHeader(app->list);
	if ( header == NULL ) {
		return;
	}

	for ( i = 0; i < GUI_ARCHIVE_COLUMN_COUNT; ++i ) {
		int visibleIndex;
		HDITEMW item;

		visibleIndex = GuiLogicalColumnToVisible(app, i);
		if ( visibleIndex < 0 ) {
			continue;
		}
		ZeroMemory(&item, sizeof(item));
		item.mask = HDI_FORMAT;
		if ( !SendMessageW(header, HDM_GETITEMW, (WPARAM)visibleIndex, (LPARAM)&item) ) {
			continue;
		}
		item.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
		if ( i == app->sortColumn ) {
			item.fmt |= app->sortAscending ? HDF_SORTUP : HDF_SORTDOWN;
		}
		SendMessageW(header, HDM_SETITEMW, (WPARAM)visibleIndex, (LPARAM)&item);
	}
}

static int GuiViewItemKindRank(GuiViewItemKind kind)
{
	switch ( kind ) {
		case GUI_VIEW_ITEM_PARENT: return 0;
		case GUI_VIEW_ITEM_DIR: return 1;
		case GUI_VIEW_ITEM_FILE: return 2;
		default: return 3;
	}
}

static int __cdecl GuiViewItemCompare(void* context, const void* leftPtr, const void* rightPtr)
{
	const GuiSortContext* sort;
	const GuiViewItem* left;
	const GuiViewItem* right;
	int cmp;
	int leftRank;
	int rightRank;

	sort = (const GuiSortContext*)context;
	left = (const GuiViewItem*)leftPtr;
	right = (const GuiViewItem*)rightPtr;
	leftRank = GuiViewItemKindRank(left->kind);
	rightRank = GuiViewItemKindRank(right->kind);
	if ( leftRank != rightRank ) {
		return leftRank - rightRank;
	}

	if ( left->kind != GUI_VIEW_ITEM_FILE || right->kind != GUI_VIEW_ITEM_FILE ) {
		cmp = GuiCompareTextInsensitive(left->name, right->name);
		if ( cmp == 0 ) {
			cmp = GuiCompareTextInsensitive(left->fullPath, right->fullPath);
		}
		return sort->ascending ? cmp : -cmp;
	}

	switch ( sort->column ) {
		case 0:
			cmp = GuiCompareTextInsensitive(left->name, right->name);
			break;
		case 1:
			cmp = GuiCompareUnsigned64(left->fileSize, right->fileSize);
			break;
		case 2:
			cmp = GuiCompareUnsigned64(left->packedSize, right->packedSize);
			break;
		case 3:
			cmp = GuiCompareRatio(left->packedSize, left->fileSize, right->packedSize, right->fileSize);
			break;
		case 4:
			cmp = GuiCompareTextInsensitive(left->methodText, right->methodText);
			break;
		case 5:
			cmp = GuiCompareTextInsensitive(left->fileTypeText, right->fileTypeText);
			break;
		case 6:
			cmp = GuiCompareSigned64((int64_t)left->modifyTime, (int64_t)right->modifyTime);
			break;
		case 7:
			if ( left->fileIndex != 0 || right->fileIndex != 0 ) {
				cmp = GuiCompareSigned64(left->fileIndex, right->fileIndex);
			} else {
				cmp = GuiCompareUnsigned64(left->pos, right->pos);
			}
			break;
		case 8:
			cmp = GuiCompareUnsigned64(left->fileHash, right->fileHash);
			break;
		case 9:
			cmp = GuiCompareUnsigned64(left->attr, right->attr);
			break;
		default:
			cmp = GuiCompareTextInsensitive(left->name, right->name);
			break;
	}

	if ( cmp == 0 ) {
		cmp = GuiCompareTextInsensitive(left->name, right->name);
	}
	if ( cmp == 0 ) {
		cmp = GuiCompareTextInsensitive(left->fullPath, right->fullPath);
	}
	if ( cmp == 0 ) {
		cmp = GuiCompareUnsigned64(left->pos, right->pos);
	}
	return sort->ascending ? cmp : -cmp;
}

static void GuiSortViewItems(GuiApp* app)
{
	GuiSortContext sort;

	if ( app->viewItems == NULL || app->viewCount <= 1 ) {
		return;
	}

	sort.column = app->sortColumn;
	sort.ascending = app->sortAscending;
	qsort_s(app->viewItems, app->viewCount, sizeof(GuiViewItem), GuiViewItemCompare, &sort);
}

static BOOL GuiFilterIsEmpty(const GuiApp* app)
{
	return app == NULL || app->filterText[0] == L'\0';
}

static BOOL GuiFilterNextToken(const WCHAR** cursor, WCHAR* token, size_t cchToken)
{
	const WCHAR* p;
	size_t len;

	if ( cursor == NULL || *cursor == NULL || token == NULL || cchToken == 0 ) {
		return FALSE;
	}

	p = *cursor;
	while ( *p == L' ' || *p == L'\t' ) {
		p++;
	}
	if ( *p == L'\0' ) {
		*cursor = p;
		token[0] = L'\0';
		return FALSE;
	}

	len = 0;
	if ( *p == L'"' ) {
		p++;
		while ( *p != L'\0' && *p != L'"' ) {
			if ( len + 1 < cchToken ) {
				token[len++] = *p;
			}
			p++;
		}
		if ( *p == L'"' ) {
			p++;
		}
	} else {
		while ( *p != L'\0' && *p != L' ' && *p != L'\t' ) {
			if ( len + 1 < cchToken ) {
				token[len++] = *p;
			}
			p++;
		}
	}

	token[len] = L'\0';
	*cursor = p;
	return len > 0;
}

static BOOL GuiFilterTokenHasWildcard(const WCHAR* token)
{
	for ( ; token != NULL && *token != L'\0'; ++token ) {
		if ( *token == L'*' || *token == L'?' ) {
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL GuiTextMatchesFilterToken(const WCHAR* text, const WCHAR* token)
{
	if ( token == NULL || token[0] == L'\0' ) {
		return TRUE;
	}
	if ( text == NULL || text[0] == L'\0' ) {
		return FALSE;
	}
	if ( GuiFilterTokenHasWildcard(token) ) {
		return PathMatchSpecW(text, token);
	}
	return StrStrIW(text, token) != NULL;
}

static BOOL GuiAnyTextMatchesFilterToken(const WCHAR* token, const WCHAR** fields, size_t fieldCount)
{
	size_t i;

	for ( i = 0; i < fieldCount; ++i ) {
		if ( GuiTextMatchesFilterToken(fields[i], token) ) {
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL GuiTextListMatchesFilter(const WCHAR* filterText, const WCHAR** fields, size_t fieldCount)
{
	const WCHAR* cursor;
	WCHAR token[128];

	if ( filterText == NULL || filterText[0] == L'\0' ) {
		return TRUE;
	}

	cursor = filterText;
	while ( GuiFilterNextToken(&cursor, token, _countof(token)) ) {
		if ( !GuiAnyTextMatchesFilterToken(token, fields, fieldCount) ) {
			return FALSE;
		}
	}
	return TRUE;
}

static BOOL GuiArchiveItemMatchesFilterText(const GuiArchiveItem* item, const WCHAR* displayName, const WCHAR* filterText)
{
	const WCHAR* fields[7];

	fields[0] = displayName;
	fields[1] = item->name;
	fields[2] = item->ratioText;
	fields[3] = item->methodText;
	fields[4] = item->idText;
	fields[5] = item->hashText;
	fields[6] = item->attrText;
	return GuiTextListMatchesFilter(filterText, fields, _countof(fields));
}

static BOOL GuiDirectoryMatchesFilterText(GuiApp* app, const WCHAR* childName, const WCHAR* dirPath)
{
	size_t i;
	const WCHAR* fields[2];

	if ( GuiFilterIsEmpty(app) ) {
		return TRUE;
	}
	fields[0] = childName;
	fields[1] = dirPath;
	if ( GuiTextListMatchesFilter(app->filterText, fields, _countof(fields)) ) {
		return TRUE;
	}

	for ( i = 0; i < app->itemCount; ++i ) {
		if ( _wcsicmp(app->items[i].name, dirPath) == 0 || GuiPathMatchFolderPrefix(app->items[i].name, dirPath, NULL) ) {
			if ( GuiArchiveItemMatchesFilterText(&app->items[i], app->items[i].name, app->filterText) ) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

static BOOL GuiBuildFlatViewItems(GuiApp* app)
{
	size_t i;

	for ( i = 0; i < app->itemCount; ++i ) {
		if ( !GuiArchiveItemMatchesFilterText(&app->items[i], app->items[i].name, app->filterText) ) {
			continue;
		}
		if ( !GuiAddViewFileItem(app, i, app->items[i].name) ) {
			return FALSE;
		}
	}
	return TRUE;
}

static BOOL GuiBuildPathViewItems(GuiApp* app)
{
	size_t i;

	if ( app->currentFolder[0] != L'\0' && !GuiAddViewParentItem(app) ) {
		return FALSE;
	}

	for ( i = 0; i < app->itemCount; ++i ) {
		WCHAR childName[XPKGUI_ITEM_TEXT];
		WCHAR childPath[XPKGUI_ITEM_TEXT];
		BOOL isDir;

		if ( !GuiPathGetCurrentChild(app->items[i].name, app->currentFolder, childName, _countof(childName), &isDir) ) {
			continue;
		}
		if ( isDir ) {
			GuiBuildChildViewPath(app->currentFolder, childName, childPath, _countof(childPath));
			if ( !GuiDirectoryMatchesFilterText(app, childName, childPath) ) {
				continue;
			}
			if ( !GuiAddViewDirectoryItem(app, childName, childPath) ) {
				return FALSE;
			}
		} else {
			if ( !GuiArchiveItemMatchesFilterText(&app->items[i], childName, app->filterText) ) {
				continue;
			}
			if ( !GuiAddViewFileItem(app, i, childName) ) {
				return FALSE;
			}
		}
	}
	return TRUE;
}

static BOOL GuiBuildVisibleItems(GuiApp* app)
{
	GuiClearViewItems(app);
	if ( app->archive == NULL ) {
		return TRUE;
	}

	if ( GuiIsPathPackType(app->packType) ) {
		if ( app->flatView ) {
			return GuiBuildFlatViewItems(app);
		}
		GuiNormalizeViewPath(app->currentFolder, app->currentFolder, _countof(app->currentFolder));
		return GuiBuildPathViewItems(app);
	}

	app->currentFolder[0] = L'\0';
	return GuiBuildFlatViewItems(app);
}

static BOOL GuiRenderArchiveView(GuiApp* app)
{
	LVITEMW item;
	size_t i;
	GuiSelectionSnapshotItem* selectedSnapshot;
	size_t selectedCount;
	GuiSelectionSnapshotItem focusedSnapshot;
	BOOL hasFocused;

	selectedSnapshot = NULL;
	selectedCount = 0;
	hasFocused = FALSE;

	if ( app->list != NULL ) {
		GuiCaptureViewSelection(app, &selectedSnapshot, &selectedCount, &focusedSnapshot, &hasFocused);
		ListView_DeleteAllItems(app->list);
		GuiRefreshColumns(app);
	}

	if ( !GuiBuildVisibleItems(app) ) {
		free(selectedSnapshot);
		return FALSE;
	}
	GuiSortViewItems(app);

	if ( app->list != NULL ) {
		ZeroMemory(&item, sizeof(item));
		item.mask = LVIF_TEXT | LVIF_PARAM;
		for ( i = 0; i < app->viewCount; ++i ) {
			item.iItem = (int)i;
			item.iSubItem = 0;
			item.pszText = app->viewItems[i].name;
			item.lParam = (LPARAM)i;
			ListView_InsertItem(app->list, &item);
			{
				int logicalColumn;

				for ( logicalColumn = 1; logicalColumn < GUI_ARCHIVE_COLUMN_COUNT; ++logicalColumn ) {
					int visibleColumn;

					visibleColumn = GuiLogicalColumnToVisible(app, logicalColumn);
					if ( visibleColumn >= 0 ) {
						ListView_SetItemText(app->list, (int)i, visibleColumn, (LPWSTR)GuiArchiveGetViewItemColumnText(&app->viewItems[i], logicalColumn));
					}
				}
			}
		}
		GuiRestoreViewSelection(app, selectedSnapshot, selectedCount, &focusedSnapshot, hasFocused);
		GuiUpdateSortHeader(app);
	}
	free(selectedSnapshot);

	GuiSetMenuState(app);
	GuiUpdateNavigationBar(app);
	GuiUpdateStatus(app);
	GuiUpdateTitle(app);
	return TRUE;
}

BOOL GuiArchiveRefreshView(GuiApp* app)
{
	if ( !GuiLoadArchiveItems(app) ) {
		return FALSE;
	}
	GuiSortArchiveItems(app);
	if ( !GuiRenderArchiveView(app) ) {
		return FALSE;
	}
	if ( app != NULL && app->archive != NULL && GuiIsPathPackType(app->packType) && app->navHistoryCount == 0 ) {
		GuiNormalizeViewPath(app->currentFolder, app->navHistory[0].folder, _countof(app->navHistory[0].folder));
		app->navHistory[0].flatView = app->flatView;
		app->navHistoryCount = 1;
		app->navHistoryIndex = 0;
		GuiSetMenuState(app);
		GuiUpdateNavigationBar(app);
	}
	return TRUE;
}

static BOOL GuiQueryArchiveDiskStamp(const WCHAR* archivePath, FILETIME* writeTimeOut, uint64_t* fileSizeOut)
{
	WIN32_FILE_ATTRIBUTE_DATA attrData;

	if ( writeTimeOut != NULL ) {
		ZeroMemory(writeTimeOut, sizeof(*writeTimeOut));
	}
	if ( fileSizeOut != NULL ) {
		*fileSizeOut = 0;
	}
	if ( archivePath == NULL || archivePath[0] == L'\0' ) {
		return FALSE;
	}
	if ( !GetFileAttributesExW(archivePath, GetFileExInfoStandard, &attrData) ) {
		return FALSE;
	}
	if ( writeTimeOut != NULL ) {
		*writeTimeOut = attrData.ftLastWriteTime;
	}
	if ( fileSizeOut != NULL ) {
		*fileSizeOut = ((uint64_t)attrData.nFileSizeHigh << 32) | (uint64_t)attrData.nFileSizeLow;
	}
	return TRUE;
}

static BOOL GuiUpdateArchiveDiskStamp(GuiApp* app)
{
	FILETIME writeTime;
	uint64_t fileSize;

	if ( app == NULL ) {
		return FALSE;
	}
	if ( !GuiQueryArchiveDiskStamp(app->archivePath, &writeTime, &fileSize) ) {
		ZeroMemory(&app->archiveWriteTime, sizeof(app->archiveWriteTime));
		app->archiveFileSize = 0;
		return FALSE;
	}
	app->archiveWriteTime = writeTime;
	app->archiveFileSize = fileSize;
	return TRUE;
}

static void GuiArchiveRestoreBrowseHistory(
	GuiApp* app,
	const GuiBrowseHistoryEntry* savedHistory,
	UINT savedCount,
	UINT savedIndex)
{
	UINT i;
	UINT restoredCount;
	UINT restoredIndex;
	BOOL hasRestoredIndex;

	if ( app == NULL || !GuiIsPathPackType(app->packType) ) {
		return;
	}

	restoredCount = 0;
	restoredIndex = 0;
	hasRestoredIndex = FALSE;
	for ( i = 0; i < savedCount && restoredCount < XPKGUI_MAX_NAV_HISTORY; ++i ) {
		if ( !savedHistory[i].flatView && savedHistory[i].folder[0] != L'\0' && !GuiArchiveFolderExists(app, savedHistory[i].folder) ) {
			continue;
		}
		app->navHistory[restoredCount] = savedHistory[i];
		if ( i == savedIndex ) {
			restoredIndex = restoredCount;
			hasRestoredIndex = TRUE;
		}
		restoredCount++;
	}

	if ( restoredCount == 0 ) {
		GuiNormalizeViewPath(app->currentFolder, app->navHistory[0].folder, _countof(app->navHistory[0].folder));
		app->navHistory[0].flatView = app->flatView;
		app->navHistoryCount = 1;
		app->navHistoryIndex = 0;
		return;
	}

	app->navHistoryCount = restoredCount;
	app->navHistoryIndex = hasRestoredIndex ? restoredIndex : (restoredCount - 1);
}

BOOL GuiArchiveReloadFromDisk(GuiApp* app)
{
	WCHAR archivePath[MAX_PATH];
	WCHAR savedFolder[XPKGUI_ITEM_TEXT];
	WCHAR savedFilter[XPKGUI_ITEM_TEXT];
	GuiBrowseHistoryEntry savedHistory[XPKGUI_MAX_NAV_HISTORY];
	GuiSelectionSnapshotItem* selectedSnapshot;
	size_t selectedCount;
	GuiSelectionSnapshotItem focusedSnapshot;
	BOOL hasFocused;
	BOOL savedFlatView;
	int savedSortColumn;
	BOOL savedSortAscending;
	UINT savedNavCount;
	UINT savedNavIndex;
	BOOL savedPathPack;
	GuiApp tempApp;

	if ( app == NULL || app->archive == NULL || app->archivePath[0] == L'\0' ) {
		return FALSE;
	}

	wcsncpy_s(archivePath, _countof(archivePath), app->archivePath, _TRUNCATE);
	wcsncpy_s(savedFolder, _countof(savedFolder), app->currentFolder, _TRUNCATE);
	wcsncpy_s(savedFilter, _countof(savedFilter), app->filterText, _TRUNCATE);
	savedFlatView = app->flatView;
	savedSortColumn = app->sortColumn;
	savedSortAscending = app->sortAscending;
	savedNavCount = app->navHistoryCount;
	savedNavIndex = app->navHistoryIndex;
	savedPathPack = GuiIsPathPackType(app->packType);
	if ( savedNavCount > XPKGUI_MAX_NAV_HISTORY ) {
		savedNavCount = XPKGUI_MAX_NAV_HISTORY;
	}
	CopyMemory(savedHistory, app->navHistory, sizeof(savedHistory));

	selectedSnapshot = NULL;
	selectedCount = 0;
	hasFocused = FALSE;
	GuiCaptureViewSelection(app, &selectedSnapshot, &selectedCount, &focusedSnapshot, &hasFocused);

	ZeroMemory(&tempApp, sizeof(tempApp));
	tempApp.instance = app->instance;
	tempApp.window = app->window;
	GuiAppInitDefaults(&tempApp);
	tempApp.writePolicy = app->writePolicy;
	tempApp.recentArchiveCount = app->recentArchiveCount;
	CopyMemory(tempApp.recentArchives, app->recentArchives, sizeof(tempApp.recentArchives));
	if ( !GuiArchiveOpenPath(&tempApp, archivePath, FALSE) ) {
		free(selectedSnapshot);
		GuiAppCleanup(&tempApp);
		return FALSE;
	}

	GuiAppCloseArchive(app);
	app->archive = tempApp.archive;
	tempApp.archive = NULL;
	app->packType = tempApp.packType;
	app->defaultComp = tempApp.defaultComp;
	app->metaComp = tempApp.metaComp;
	app->infoComp = tempApp.infoComp;
	app->infoExtSize = tempApp.infoExtSize;
	app->volumeSize = tempApp.volumeSize;
	app->writePolicy = tempApp.writePolicy;
	app->solidMode = tempApp.solidMode;
	wcsncpy_s(app->archivePath, _countof(app->archivePath), tempApp.archivePath, _TRUNCATE);
	app->archiveWriteTime = tempApp.archiveWriteTime;
	app->archiveFileSize = tempApp.archiveFileSize;
	app->items = tempApp.items;
	app->itemCount = tempApp.itemCount;
	app->itemCapacity = tempApp.itemCapacity;
	app->viewItems = tempApp.viewItems;
	app->viewCount = tempApp.viewCount;
	app->viewCapacity = tempApp.viewCapacity;
	tempApp.items = NULL;
	tempApp.itemCount = 0;
	tempApp.itemCapacity = 0;
	tempApp.viewItems = NULL;
	tempApp.viewCount = 0;
	tempApp.viewCapacity = 0;
	GuiAppCleanup(&tempApp);

	wcsncpy_s(app->filterText, _countof(app->filterText), savedFilter, _TRUNCATE);
	app->sortColumn = savedSortColumn;
	app->sortAscending = savedSortAscending;
	if ( savedPathPack && GuiIsPathPackType(app->packType) ) {
		wcsncpy_s(app->currentFolder, _countof(app->currentFolder), savedFolder, _TRUNCATE);
		app->flatView = savedFlatView;
		if ( !app->flatView && app->currentFolder[0] != L'\0' && !GuiArchiveFolderExists(app, app->currentFolder) ) {
			app->currentFolder[0] = L'\0';
		}
	} else {
		app->currentFolder[0] = L'\0';
		app->flatView = FALSE;
	}

	GuiSortArchiveItems(app);
	if ( !GuiRenderArchiveView(app) ) {
		free(selectedSnapshot);
		return FALSE;
	}

	ListView_SetItemState(app->list, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
	GuiRestoreViewSelection(app, selectedSnapshot, selectedCount, &focusedSnapshot, hasFocused);
	free(selectedSnapshot);

	if ( savedPathPack && GuiIsPathPackType(app->packType) ) {
		GuiArchiveRestoreBrowseHistory(app, savedHistory, savedNavCount, savedNavIndex);
		GuiSetMenuState(app);
		GuiUpdateNavigationBar(app);
		GuiUpdateStatus(app);
		GuiUpdateTitle(app);
	}
	return TRUE;
}

BOOL GuiArchiveCheckExternalChanges(GuiApp* app)
{
	FILETIME currentWriteTime;
	uint64_t currentFileSize;
	int cmp;
	int answer;

	if ( app == NULL || app->archive == NULL || app->archivePath[0] == L'\0' || app->archiveChangePromptActive ) {
		return FALSE;
	}
	if ( !GuiQueryArchiveDiskStamp(app->archivePath, &currentWriteTime, &currentFileSize) ) {
		return FALSE;
	}

	cmp = CompareFileTime(&currentWriteTime, &app->archiveWriteTime);
	if ( cmp == 0 && currentFileSize == app->archiveFileSize ) {
		return FALSE;
	}

	app->archiveChangePromptActive = TRUE;
	answer = MessageBoxW(
		app->window,
		L"检测到当前归档已被外部修改。\r\n\r\n是否从磁盘重新载入？",
		XPKGUI_APP_TITLE,
		MB_YESNO | MB_ICONQUESTION);
	app->archiveChangePromptActive = FALSE;

	if ( answer == IDYES ) {
		if ( GuiArchiveReloadFromDisk(app) ) {
			return TRUE;
		}
	}

	app->archiveWriteTime = currentWriteTime;
	app->archiveFileSize = currentFileSize;
	return FALSE;
}

void GuiArchiveToggleSort(GuiApp* app, int column)
{
	if ( app == NULL || column < 0 || column >= GUI_ARCHIVE_COLUMN_COUNT ) {
		return;
	}

	if ( app->sortColumn == column ) {
		app->sortAscending = !app->sortAscending;
	} else {
		app->sortColumn = column;
		app->sortAscending = TRUE;
	}
	GuiSaveSortSettings(app);
	GuiRenderArchiveView(app);
}

void GuiArchiveResetSort(GuiApp* app)
{
	if ( app == NULL ) {
		return;
	}
	app->sortColumn = 0;
	app->sortAscending = TRUE;
	app->sortInitialized = TRUE;
	GuiSaveSortSettings(app);
	GuiRenderArchiveView(app);
}

BOOL GuiArchiveReadOptions(GuiApp* app, GuiArchiveOptions* options)
{
	if ( app->archive == NULL || options == NULL ) {
		return FALSE;
	}

	ZeroMemory(options, sizeof(*options));
	wcsncpy_s(options->archivePath, _countof(options->archivePath), app->archivePath, _TRUNCATE);
	if ( xpkGetPackType(app->archive, &options->packType) != XPK_OK ) return FALSE;
	if ( xpkGetDefaultComp(app->archive, &options->defaultComp) != XPK_OK ) return FALSE;
	if ( xpkGetMetaComp(app->archive, &options->metaComp) != XPK_OK ) return FALSE;
	if ( xpkGetInfoComp(app->archive, &options->infoComp) != XPK_OK ) return FALSE;
	if ( xpkGetInfoExtSize(app->archive, &options->infoExtSize) != XPK_OK ) return FALSE;
	if ( xpkGetVolumeSize(app->archive, &options->volumeSize) != XPK_OK ) return FALSE;
	if ( xpkGetSolidMode(app->archive, (int*)&options->solidMode) != XPK_OK ) return FALSE;
	options->writePolicy = app->writePolicy;
	return TRUE;
}

BOOL GuiArchiveApplyOptions(GuiApp* app, const GuiArchiveOptions* options, BOOL saveNow)
{
	BOOL packTypeChanged;
	BOOL solidChanged;
	BOOL volumeChanged;

	if ( app->archive == NULL || options == NULL ) {
		return FALSE;
	}

	packTypeChanged = (options->packType != app->packType);
	solidChanged = (options->solidMode != app->solidMode);
	volumeChanged = (options->volumeSize != app->volumeSize);
	if ( saveNow && app->window != NULL && app->itemCount > 0 && (packTypeChanged || solidChanged || volumeChanged) ) {
		WCHAR text[1024];

		_snwprintf_s(
			text,
			_countof(text),
			_TRUNCATE,
			L"将保存归档布局相关变更：\r\n\r\n"
			L"%s%s%s"
			L"\r\nPack Type 会改变条目语义；Solid / Volume 布局变更通常还需要执行 Rebuild 才会反映到物理文件。\r\n\r\n是否继续保存？",
			packTypeChanged ? L"- Pack Type\r\n" : L"",
			solidChanged ? L"- Solid Layout\r\n" : L"",
			volumeChanged ? L"- Volume Size\r\n" : L"");
		if ( MessageBoxW(app->window, text, XPKGUI_APP_TITLE, MB_YESNO | MB_ICONWARNING) != IDYES ) {
			return FALSE;
		}
	}

	if ( xpkSetPackType(app->archive, options->packType) != XPK_OK ) return FALSE;
	if ( xpkSetDefaultComp(app->archive, options->defaultComp) != XPK_OK ) return FALSE;
	if ( xpkSetMetaComp(app->archive, options->metaComp) != XPK_OK ) return FALSE;
	if ( xpkSetInfoComp(app->archive, options->infoComp) != XPK_OK ) return FALSE;
	if ( options->packType == XPK_PACK_CORE ) {
		if ( xpkSetInfoExtSize(app->archive, options->infoExtSize) != XPK_OK ) return FALSE;
	}
	if ( xpkSetVolumeSize(app->archive, options->volumeSize) != XPK_OK ) return FALSE;
	if ( xpkSetSolidMode(app->archive, options->solidMode ? 1 : 0) != XPK_OK ) return FALSE;

	app->packType = options->packType;
	app->defaultComp = options->defaultComp;
	app->metaComp = options->metaComp;
	app->infoComp = options->infoComp;
	app->infoExtSize = (options->packType == XPK_PACK_CORE) ? options->infoExtSize : 0;
	app->volumeSize = options->volumeSize;
	app->writePolicy = options->writePolicy;
	app->solidMode = options->solidMode;

	if ( saveNow ) {
		return GuiArchiveSave(app);
	}
	return TRUE;
}

BOOL GuiArchiveOpenPath(GuiApp* app, const WCHAR* archivePath, BOOL readonly)
{
	xpkOpenOptions opt;
	char utf8Path[MAX_PATH * 4];
	WCHAR fullPath[MAX_PATH];
	const WCHAR* openPath;
	DWORD fullPathLen;
	xpkPackType packType;
	uint8_t previousWritePolicy;

	if ( archivePath == NULL || archivePath[0] == L'\0' ) {
		return FALSE;
	}

	openPath = archivePath;
	fullPathLen = GetFullPathNameW(archivePath, (DWORD)_countof(fullPath), fullPath, NULL);
	if ( fullPathLen > 0 && fullPathLen < _countof(fullPath) ) {
		openPath = fullPath;
	}

	previousWritePolicy = app->writePolicy;
	GuiAppCloseArchive(app);
	GuiAppInitDefaults(app);
	app->writePolicy = previousWritePolicy;

	ZeroMemory(&opt, sizeof(opt));
	opt.readonly = readonly ? 1 : 0;
	opt.bufferedDefault = (app->writePolicy == XPK_WRITE_BUFFERED) ? 1 : 0;

	if ( !GuiUtf8FromWide(openPath, utf8Path, sizeof(utf8Path)) ) {
		MessageBoxW(app->window, L"路径转换失败。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	app->archive = xpkOpen(utf8Path, &opt);
	if ( app->archive == NULL ) {
		GuiShowArchiveErrorPath(app, L"打开归档", openPath);
		return FALSE;
	}

	if ( xpkGetPackType(app->archive, &packType) == XPK_OK ) {
		app->packType = packType;
	}
	(void)xpkGetDefaultComp(app->archive, &app->defaultComp);
	(void)xpkGetMetaComp(app->archive, &app->metaComp);
	(void)xpkGetInfoComp(app->archive, &app->infoComp);
	(void)xpkGetInfoExtSize(app->archive, &app->infoExtSize);
	(void)xpkGetVolumeSize(app->archive, &app->volumeSize);
	(void)xpkGetSolidMode(app->archive, (int*)&app->solidMode);
	wcsncpy_s(app->archivePath, _countof(app->archivePath), openPath, _TRUNCATE);
	GuiRememberRecentArchive(app, openPath);
	GuiUpdateArchiveDiskStamp(app);

	return GuiArchiveRefreshView(app);
}

BOOL GuiArchiveCreateWithOptions(GuiApp* app, const GuiArchiveOptions* options)
{
	xpkOpenOptions openOpt;
	char utf8Path[MAX_PATH * 4];

	if ( options == NULL || options->archivePath[0] == L'\0' ) {
		return FALSE;
	}

	GuiAppCloseArchive(app);
	GuiAppInitDefaults(app);

	ZeroMemory(&openOpt, sizeof(openOpt));
	openOpt.createIfMissing = 1;
	openOpt.bufferedDefault = (options->writePolicy == XPK_WRITE_BUFFERED) ? 1 : 0;

	if ( !GuiUtf8FromWide(options->archivePath, utf8Path, sizeof(utf8Path)) ) {
		MessageBoxW(app->window, L"路径转换失败。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	app->archive = xpkOpen(utf8Path, &openOpt);
	if ( app->archive == NULL ) {
		GuiShowArchiveError(app, L"创建归档");
		return FALSE;
	}

	wcsncpy_s(app->archivePath, _countof(app->archivePath), options->archivePath, _TRUNCATE);
	if ( !GuiArchiveApplyOptions(app, options, TRUE) ) {
		GuiShowArchiveError(app, L"初始化归档配置");
		return FALSE;
	}
	GuiRememberRecentArchive(app, options->archivePath);

	return GuiArchiveRefreshView(app);
}

BOOL GuiArchiveSave(GuiApp* app)
{
	DWORD attr;

	if ( app->archive == NULL ) {
		return FALSE;
	}
	attr = (app->archivePath[0] != L'\0') ? GetFileAttributesW(app->archivePath) : INVALID_FILE_ATTRIBUTES;
	if ( attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_READONLY) ) {
		GuiShowSystemErrorDetail(app->window, L"保存归档失败", L"归档文件是只读文件，xpkgui 不会尝试覆盖它。", app->archivePath, ERROR_ACCESS_DENIED);
		return FALSE;
	}
	if ( xpkSave(app->archive) != XPK_OK ) {
		GuiShowArchiveError(app, L"保存归档");
		return FALSE;
	}
	GuiUpdateArchiveDiskStamp(app);
	return GuiArchiveRefreshView(app);
}

static void GuiArchiveNormalizeFilePath(const WCHAR* path, WCHAR* outPath, size_t cchOutPath)
{
	WCHAR fullPath[MAX_PATH];
	DWORD cchFullPath;

	if ( outPath == NULL || cchOutPath == 0 ) {
		return;
	}
	outPath[0] = L'\0';
	if ( path == NULL || path[0] == L'\0' ) {
		return;
	}

	cchFullPath = GetFullPathNameW(path, (DWORD)_countof(fullPath), fullPath, NULL);
	if ( cchFullPath > 0 && cchFullPath < _countof(fullPath) ) {
		wcsncpy_s(outPath, cchOutPath, fullPath, _TRUNCATE);
		return;
	}
	wcsncpy_s(outPath, cchOutPath, path, _TRUNCATE);
}

BOOL GuiArchiveSaveAs(GuiApp* app)
{
	WCHAR currentPath[MAX_PATH];
	WCHAR targetPath[MAX_PATH];
	WCHAR normalizedCurrent[MAX_PATH];
	WCHAR normalizedTarget[MAX_PATH];
	BOOL copied;
	DWORD copyError;

	if ( app == NULL || app->archive == NULL || app->archivePath[0] == L'\0' ) {
		return FALSE;
	}

	wcsncpy_s(currentPath, _countof(currentPath), app->archivePath, _TRUNCATE);
	wcsncpy_s(targetPath, _countof(targetPath), app->archivePath, _TRUNCATE);
	if ( !GuiSaveArchiveDialog(app->window, targetPath, _countof(targetPath)) ) {
		return FALSE;
	}

	GuiArchiveNormalizeFilePath(currentPath, normalizedCurrent, _countof(normalizedCurrent));
	GuiArchiveNormalizeFilePath(targetPath, normalizedTarget, _countof(normalizedTarget));
	if ( normalizedTarget[0] == L'\0' ) {
		return FALSE;
	}
	if ( _wcsicmp(normalizedCurrent, normalizedTarget) == 0 ) {
		return GuiArchiveSave(app);
	}

	if ( !GuiArchiveSave(app) ) {
		return FALSE;
	}

	GuiAppCloseArchive(app);
	copied = CopyFileW(normalizedCurrent, normalizedTarget, FALSE);
	copyError = copied ? ERROR_SUCCESS : GetLastError();
	if ( !copied ) {
		GuiShowSystemError(app->window, L"另存归档失败", copyError);
		GuiArchiveOpenPath(app, normalizedCurrent, FALSE);
		return FALSE;
	}

	if ( !GuiArchiveOpenPath(app, normalizedTarget, FALSE) ) {
		GuiArchiveOpenPath(app, normalizedCurrent, FALSE);
		return FALSE;
	}
	return TRUE;
}

BOOL GuiArchiveShowArchiveInExplorer(GuiApp* app)
{
	WCHAR normalizedPath[MAX_PATH];
	WCHAR parameters[(MAX_PATH * 2) + 32];
	INT_PTR shellResult;

	if ( app == NULL || app->archivePath[0] == L'\0' ) {
		return FALSE;
	}

	GuiArchiveNormalizeFilePath(app->archivePath, normalizedPath, _countof(normalizedPath));
	if ( normalizedPath[0] == L'\0' ) {
		return FALSE;
	}

	_snwprintf_s(parameters, _countof(parameters), _TRUNCATE, L"/select,\"%s\"", normalizedPath);
	shellResult = (INT_PTR)ShellExecuteW(app->window, L"open", L"explorer.exe", parameters, NULL, SW_SHOWNORMAL);
	if ( shellResult <= 32 ) {
		GuiShowSystemError(app->window, L"定位归档失败", (DWORD)shellResult);
		return FALSE;
	}
	return TRUE;
}

BOOL GuiArchiveCopyArchivePath(GuiApp* app)
{
	WCHAR normalizedPath[MAX_PATH];

	if ( app == NULL || app->archivePath[0] == L'\0' ) {
		return FALSE;
	}

	GuiArchiveNormalizeFilePath(app->archivePath, normalizedPath, _countof(normalizedPath));
	if ( normalizedPath[0] == L'\0' ) {
		return FALSE;
	}
	return GuiSetClipboardText(app->window, normalizedPath);
}

BOOL GuiArchivePromptNew(GuiApp* app, const WCHAR* suggestedPath, GuiArchiveOptions* optionsOut)
{
	GuiArchiveConfigDialogState state;

	ZeroMemory(&state, sizeof(state));
	state.createMode = TRUE;
	state.allowPackTypeEdit = TRUE;
	state.options.packType = app->packType;
	state.options.defaultComp = app->defaultComp;
	state.options.metaComp = app->metaComp;
	state.options.infoComp = app->infoComp;
	state.options.infoExtSize = (state.options.packType == XPK_PACK_CORE) ? app->infoExtSize : 0;
	state.options.volumeSize = app->volumeSize;
	state.options.writePolicy = app->writePolicy;
	state.options.solidMode = app->solidMode;
	if ( suggestedPath != NULL ) {
		wcsncpy_s(state.options.archivePath, _countof(state.options.archivePath), suggestedPath, _TRUNCATE);
	}

	if ( !GuiRunArchiveConfigDialog(app->window, &state) ) {
		return FALSE;
	}

	*optionsOut = state.options;
	app->packType = state.options.packType;
	app->defaultComp = state.options.defaultComp;
	app->metaComp = state.options.metaComp;
	app->infoComp = state.options.infoComp;
	app->infoExtSize = state.options.infoExtSize;
	app->volumeSize = state.options.volumeSize;
	app->writePolicy = state.options.writePolicy;
	app->solidMode = state.options.solidMode;
	GuiSaveArchiveDefaults(&state.options);
	return TRUE;
}

BOOL GuiArchivePromptSettings(GuiApp* app, GuiArchiveOptions* optionsOut)
{
	GuiArchiveConfigDialogState state;

	if ( !GuiArchiveReadOptions(app, &state.options) ) {
		GuiShowArchiveError(app, L"读取归档配置");
		return FALSE;
	}

	state.createMode = FALSE;
	state.allowPackTypeEdit = (app->itemCount == 0);
	if ( !GuiRunArchiveConfigDialog(app->window, &state) ) {
		return FALSE;
	}

	*optionsOut = state.options;
	return TRUE;
}

uint32_t GuiArchiveSelectedCount(GuiApp* app)
{
	int index;
	uint32_t count;

	if ( app->list == NULL ) {
		return 0;
	}

	count = 0;
	index = -1;
	while ( TRUE ) {
		index = ListView_GetNextItem(app->list, index, LVNI_SELECTED);
		if ( index < 0 ) {
			break;
		}
		count++;
	}
	return count;
}

static GuiViewItem* GuiArchiveGetViewItemByListIndex(GuiApp* app, int index)
{
	if ( app == NULL || index < 0 || index >= (int)app->viewCount ) {
		return NULL;
	}
	return &app->viewItems[index];
}

static GuiViewItem* GuiArchiveGetSingleSelectedViewItem(GuiApp* app)
{
	int index;

	if ( app->list == NULL ) {
		return NULL;
	}

	index = ListView_GetNextItem(app->list, -1, LVNI_SELECTED);
	if ( index < 0 || index >= (int)app->viewCount ) {
		return NULL;
	}
	if ( ListView_GetNextItem(app->list, index, LVNI_SELECTED) >= 0 ) {
		return NULL;
	}
	return &app->viewItems[index];
}

static BOOL GuiArchiveCanCopyViewItem(const GuiViewItem* viewItem)
{
	return viewItem != NULL && (viewItem->kind == GUI_VIEW_ITEM_FILE || viewItem->kind == GUI_VIEW_ITEM_DIR);
}

static const WCHAR* GuiViewItemFileExtension(const GuiViewItem* viewItem)
{
	const WCHAR* name;

	if ( viewItem == NULL || viewItem->kind != GUI_VIEW_ITEM_FILE ) {
		return NULL;
	}

	name = PathFindFileNameW(viewItem->fullPath[0] != L'\0' ? viewItem->fullPath : viewItem->name);
	return PathFindExtensionW(name);
}

static GuiViewItem* GuiArchiveGetReferenceFileViewItem(GuiApp* app, int* indexOut)
{
	int index;

	if ( indexOut != NULL ) {
		*indexOut = -1;
	}
	if ( app == NULL || app->list == NULL ) {
		return NULL;
	}

	index = ListView_GetNextItem(app->list, -1, LVNI_FOCUSED);
	if ( index >= 0 ) {
		GuiViewItem* viewItem;

		viewItem = GuiArchiveGetViewItemByListIndex(app, index);
		if ( viewItem != NULL && viewItem->kind == GUI_VIEW_ITEM_FILE ) {
			if ( indexOut != NULL ) {
				*indexOut = index;
			}
			return viewItem;
		}
	}

	index = -1;
	while ( TRUE ) {
		GuiViewItem* viewItem;

		index = ListView_GetNextItem(app->list, index, LVNI_SELECTED);
		if ( index < 0 ) {
			break;
		}
		viewItem = GuiArchiveGetViewItemByListIndex(app, index);
		if ( viewItem != NULL && viewItem->kind == GUI_VIEW_ITEM_FILE ) {
			if ( indexOut != NULL ) {
				*indexOut = index;
			}
			return viewItem;
		}
	}
	return NULL;
}

BOOL GuiArchiveCanOpenSelection(GuiApp* app)
{
	return app != NULL && app->archive != NULL && GuiArchiveGetSingleSelectedItem(app) != NULL;
}

BOOL GuiArchiveCanViewSelection(GuiApp* app)
{
	return GuiArchiveCanOpenSelection(app);
}

BOOL GuiArchiveCanEditTextSelection(GuiApp* app)
{
	GuiArchiveItem* item;

	item = GuiArchiveGetSingleSelectedItem(app);
	return item != NULL && item->fileSize <= XPKGUI_INLINE_VIEW_LIMIT;
}

BOOL GuiArchiveCanCopySelectionHash(GuiApp* app)
{
	return GuiArchiveCanOpenSelection(app);
}

BOOL GuiArchiveCanCopySelectionHashes(GuiApp* app)
{
	int index;

	if ( app == NULL || app->archive == NULL || app->list == NULL ) {
		return FALSE;
	}

	index = -1;
	while ( TRUE ) {
		GuiViewItem* viewItem;

		index = ListView_GetNextItem(app->list, index, LVNI_SELECTED);
		if ( index < 0 ) {
			break;
		}
		viewItem = GuiArchiveGetViewItemByListIndex(app, index);
		if ( viewItem != NULL && viewItem->kind == GUI_VIEW_ITEM_FILE && viewItem->sourceIndex < app->itemCount ) {
			return TRUE;
		}
	}
	return FALSE;
}

BOOL GuiArchiveCanCopyVisibleHashes(GuiApp* app)
{
	size_t i;

	if ( app == NULL || app->archive == NULL || app->list == NULL ) {
		return FALSE;
	}

	for ( i = 0; i < app->viewCount; ++i ) {
		if ( app->viewItems[i].kind == GUI_VIEW_ITEM_FILE && app->viewItems[i].sourceIndex < app->itemCount ) {
			return TRUE;
		}
	}
	return FALSE;
}

BOOL GuiArchiveCanShowSelectionInExplorer(GuiApp* app)
{
	return GuiArchiveCanOpenSelection(app);
}

BOOL GuiArchiveCanEditSelection(GuiApp* app)
{
	return GuiArchiveCanOpenSelection(app);
}

BOOL GuiArchiveCanReplaceSelection(GuiApp* app)
{
	return GuiArchiveCanOpenSelection(app);
}

BOOL GuiArchiveCanDuplicateSelection(GuiApp* app)
{
	return GuiArchiveCanOpenSelection(app);
}

BOOL GuiArchiveCanEditSelectionInfoExt(GuiApp* app)
{
	return app != NULL
		&& app->archive != NULL
		&& app->packType == XPK_PACK_CORE
		&& app->infoExtSize > 0
		&& GuiArchiveGetSingleSelectedItem(app) != NULL;
}

GuiArchiveItem* GuiArchiveGetSingleSelectedItem(GuiApp* app)
{
	GuiViewItem* viewItem;

	viewItem = GuiArchiveGetSingleSelectedViewItem(app);
	if ( viewItem == NULL || viewItem->kind != GUI_VIEW_ITEM_FILE || viewItem->sourceIndex >= app->itemCount ) {
		return NULL;
	}
	return &app->items[viewItem->sourceIndex];
}

BOOL GuiArchiveCanCopySelection(GuiApp* app)
{
	int index;

	if ( app == NULL || app->archive == NULL || app->list == NULL ) {
		return FALSE;
	}

	index = -1;
	while ( TRUE ) {
		GuiViewItem* viewItem;

		index = ListView_GetNextItem(app->list, index, LVNI_SELECTED);
		if ( index < 0 ) {
			break;
		}
		viewItem = GuiArchiveGetViewItemByListIndex(app, index);
		if ( GuiArchiveCanCopyViewItem(viewItem) ) {
			return TRUE;
		}
	}
	return FALSE;
}

BOOL GuiArchiveCanCopyVisibleDetails(GuiApp* app)
{
	size_t i;

	if ( app == NULL || app->archive == NULL || app->list == NULL || app->viewCount == 0 ) {
		return FALSE;
	}

	for ( i = 0; i < app->viewCount; ++i ) {
		if ( GuiArchiveCanCopyViewItem(&app->viewItems[i]) ) {
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL GuiViewItemMatchesSelectionPattern(const GuiViewItem* item, const WCHAR* patternText)
{
	const WCHAR* fields[9];

	if ( item == NULL || item->kind == GUI_VIEW_ITEM_PARENT ) {
		return FALSE;
	}

	fields[0] = item->name;
	fields[1] = item->fullPath;
	fields[2] = item->ratioText;
	fields[3] = item->methodText;
	fields[4] = item->fileTypeText;
	fields[5] = item->idText;
	fields[6] = item->hashText;
	fields[7] = item->attrText;
	fields[8] = item->modifiedText;
	return GuiTextListMatchesFilter(patternText, fields, _countof(fields));
}

BOOL GuiArchiveCanSelectByPattern(GuiApp* app)
{
	return app != NULL && app->archive != NULL && app->list != NULL && app->viewCount > 0;
}

BOOL GuiArchiveCanSelectSameExtension(GuiApp* app)
{
	return app != NULL && app->archive != NULL && GuiArchiveGetReferenceFileViewItem(app, NULL) != NULL;
}

BOOL GuiArchiveCanSelectSameHash(GuiApp* app)
{
	return app != NULL && app->archive != NULL && GuiArchiveGetReferenceFileViewItem(app, NULL) != NULL;
}

BOOL GuiArchiveCanSelectSameMethod(GuiApp* app)
{
	return app != NULL && app->archive != NULL && GuiArchiveGetReferenceFileViewItem(app, NULL) != NULL;
}

BOOL GuiArchiveCanSelectSameFileType(GuiApp* app)
{
	return app != NULL && app->archive != NULL && GuiArchiveGetReferenceFileViewItem(app, NULL) != NULL;
}

BOOL GuiArchiveCanSelectSamePathAttr(GuiApp* app)
{
	return app != NULL
		&& app->archive != NULL
		&& GuiIsPathPackType(app->packType)
		&& GuiArchiveGetReferenceFileViewItem(app, NULL) != NULL;
}

BOOL GuiArchiveCanSelectDuplicateFiles(GuiApp* app)
{
	size_t i;
	uint32_t fileCount;

	if ( app == NULL || app->archive == NULL || app->list == NULL || app->viewCount == 0 ) {
		return FALSE;
	}

	fileCount = 0;
	for ( i = 0; i < app->viewCount; ++i ) {
		if ( app->viewItems[i].kind == GUI_VIEW_ITEM_FILE && app->viewItems[i].sourceIndex < app->itemCount ) {
			fileCount++;
			if ( fileCount >= 2 ) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL GuiArchiveCanSelectDuplicateCopies(GuiApp* app)
{
	return GuiArchiveCanSelectDuplicateFiles(app);
}

BOOL GuiArchiveCanDeleteDuplicateCopies(GuiApp* app)
{
	return GuiArchiveCanSelectDuplicateCopies(app);
}

BOOL GuiArchiveCanInvertSelection(GuiApp* app)
{
	return app != NULL && app->archive != NULL && app->list != NULL && app->viewCount > 0;
}

BOOL GuiArchiveCanLocateInTree(GuiApp* app)
{
	return app != NULL
		&& app->archive != NULL
		&& GuiIsPathPackType(app->packType)
		&& app->flatView
		&& GuiArchiveGetReferenceFileViewItem(app, NULL) != NULL;
}

BOOL GuiArchiveSelectByPattern(GuiApp* app)
{
	GuiInputDialogState dialogState;
	size_t i;
	int firstSelected;

	if ( !GuiArchiveCanSelectByPattern(app) ) {
		return FALSE;
	}

	ZeroMemory(&dialogState, sizeof(dialogState));
	dialogState.title = L"按模式选择";
	dialogState.prompt = L"模式:";
	if ( !GuiRunInputDialog(app->window, &dialogState) ) {
		return FALSE;
	}
	if ( dialogState.value[0] == L'\0' ) {
		MessageBoxW(app->window, L"请输入选择模式。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	firstSelected = -1;
	ListView_SetItemState(app->list, -1, 0, LVIS_SELECTED);
	for ( i = 0; i < app->viewCount; ++i ) {
		if ( !GuiViewItemMatchesSelectionPattern(&app->viewItems[i], dialogState.value) ) {
			continue;
		}
		ListView_SetItemState(app->list, (int)i, LVIS_SELECTED, LVIS_SELECTED);
		if ( firstSelected < 0 ) {
			firstSelected = (int)i;
		}
	}

	if ( firstSelected < 0 ) {
		MessageBoxW(app->window, L"当前视图中没有匹配项。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		GuiSetMenuState(app);
		GuiUpdateStatus(app);
		return FALSE;
	}

	ListView_SetItemState(app->list, firstSelected, LVIS_FOCUSED, LVIS_FOCUSED);
	ListView_SetSelectionMark(app->list, firstSelected);
	ListView_EnsureVisible(app->list, firstSelected, FALSE);
	GuiSetMenuState(app);
	GuiUpdateStatus(app);
	return TRUE;
}

BOOL GuiArchiveDeselectByPattern(GuiApp* app)
{
	GuiInputDialogState dialogState;
	size_t i;
	int matchCount;
	int firstSelected;

	if ( !GuiArchiveCanSelectByPattern(app) ) {
		return FALSE;
	}

	ZeroMemory(&dialogState, sizeof(dialogState));
	dialogState.title = L"按模式取消选择";
	dialogState.prompt = L"模式:";
	if ( !GuiRunInputDialog(app->window, &dialogState) ) {
		return FALSE;
	}
	if ( dialogState.value[0] == L'\0' ) {
		MessageBoxW(app->window, L"请输入选择模式。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	matchCount = 0;
	for ( i = 0; i < app->viewCount; ++i ) {
		if ( !GuiViewItemMatchesSelectionPattern(&app->viewItems[i], dialogState.value) ) {
			continue;
		}
		ListView_SetItemState(app->list, (int)i, 0, LVIS_SELECTED | LVIS_FOCUSED);
		matchCount++;
	}

	if ( matchCount <= 0 ) {
		MessageBoxW(app->window, L"当前视图中没有匹配项。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		GuiSetMenuState(app);
		GuiUpdateStatus(app);
		return FALSE;
	}

	firstSelected = -1;
	for ( i = 0; i < app->viewCount; ++i ) {
		if ( app->viewItems[i].kind == GUI_VIEW_ITEM_PARENT ) {
			continue;
		}
		if ( (ListView_GetItemState(app->list, (int)i, LVIS_SELECTED) & LVIS_SELECTED) != 0 ) {
			firstSelected = (int)i;
			break;
		}
	}

	if ( firstSelected >= 0 ) {
		ListView_SetItemState(app->list, firstSelected, LVIS_FOCUSED, LVIS_FOCUSED);
		ListView_SetSelectionMark(app->list, firstSelected);
		ListView_EnsureVisible(app->list, firstSelected, FALSE);
	} else {
		ListView_SetSelectionMark(app->list, -1);
	}
	GuiSetMenuState(app);
	GuiUpdateStatus(app);
	return TRUE;
}

BOOL GuiArchiveSelectSameExtension(GuiApp* app)
{
	GuiViewItem* refItem;
	const WCHAR* refExt;
	int refIndex;
	size_t i;
	int firstSelected;

	if ( !GuiArchiveCanSelectSameExtension(app) ) {
		MessageBoxW(app != NULL ? app->window : NULL, L"请选择一个文件作为扩展名参考。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	refItem = GuiArchiveGetReferenceFileViewItem(app, &refIndex);
	if ( refItem == NULL ) {
		return FALSE;
	}

	refExt = GuiViewItemFileExtension(refItem);
	firstSelected = -1;
	ListView_SetItemState(app->list, -1, 0, LVIS_SELECTED);
	for ( i = 0; i < app->viewCount; ++i ) {
		const GuiViewItem* item;
		const WCHAR* ext;
		BOOL match;

		item = &app->viewItems[i];
		if ( item->kind != GUI_VIEW_ITEM_FILE ) {
			continue;
		}
		ext = GuiViewItemFileExtension(item);
		match = _wcsicmp(refExt != NULL ? refExt : L"", ext != NULL ? ext : L"") == 0;
		if ( match ) {
			ListView_SetItemState(app->list, (int)i, LVIS_SELECTED, LVIS_SELECTED);
			if ( firstSelected < 0 ) {
				firstSelected = (int)i;
			}
		}
	}

	if ( refIndex >= 0 ) {
		ListView_SetItemState(app->list, refIndex, LVIS_FOCUSED, LVIS_FOCUSED);
		ListView_SetSelectionMark(app->list, refIndex);
		ListView_EnsureVisible(app->list, refIndex, FALSE);
	} else if ( firstSelected >= 0 ) {
		ListView_SetItemState(app->list, firstSelected, LVIS_FOCUSED, LVIS_FOCUSED);
		ListView_SetSelectionMark(app->list, firstSelected);
		ListView_EnsureVisible(app->list, firstSelected, FALSE);
	}

	GuiSetMenuState(app);
	GuiUpdateStatus(app);
	return TRUE;
}

BOOL GuiArchiveSelectSameHash(GuiApp* app)
{
	GuiViewItem* refViewItem;
	const GuiArchiveItem* refItem;
	int refIndex;
	size_t i;
	int firstSelected;

	if ( !GuiArchiveCanSelectSameHash(app) ) {
		MessageBoxW(app != NULL ? app->window : NULL, L"请选择一个文件作为 Hash 参考。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	refViewItem = GuiArchiveGetReferenceFileViewItem(app, &refIndex);
	if ( refViewItem == NULL || refViewItem->sourceIndex >= app->itemCount ) {
		return FALSE;
	}
	refItem = &app->items[refViewItem->sourceIndex];

	firstSelected = -1;
	ListView_SetItemState(app->list, -1, 0, LVIS_SELECTED);
	for ( i = 0; i < app->viewCount; ++i ) {
		const GuiViewItem* viewItem;
		const GuiArchiveItem* item;
		BOOL match;

		viewItem = &app->viewItems[i];
		if ( viewItem->kind != GUI_VIEW_ITEM_FILE || viewItem->sourceIndex >= app->itemCount ) {
			continue;
		}
		item = &app->items[viewItem->sourceIndex];
		match = (item->fileHash == refItem->fileHash) && (item->fileSize == refItem->fileSize);
		if ( match ) {
			ListView_SetItemState(app->list, (int)i, LVIS_SELECTED, LVIS_SELECTED);
			if ( firstSelected < 0 ) {
				firstSelected = (int)i;
			}
		}
	}

	if ( refIndex >= 0 ) {
		ListView_SetItemState(app->list, refIndex, LVIS_FOCUSED, LVIS_FOCUSED);
		ListView_SetSelectionMark(app->list, refIndex);
		ListView_EnsureVisible(app->list, refIndex, FALSE);
	} else if ( firstSelected >= 0 ) {
		ListView_SetItemState(app->list, firstSelected, LVIS_FOCUSED, LVIS_FOCUSED);
		ListView_SetSelectionMark(app->list, firstSelected);
		ListView_EnsureVisible(app->list, firstSelected, FALSE);
	}

	GuiSetMenuState(app);
	GuiUpdateStatus(app);
	return TRUE;
}

BOOL GuiArchiveSelectSameMethod(GuiApp* app)
{
	GuiViewItem* refViewItem;
	const GuiArchiveItem* refItem;
	int refIndex;
	size_t i;
	int firstSelected;

	if ( !GuiArchiveCanSelectSameMethod(app) ) {
		MessageBoxW(app != NULL ? app->window : NULL, L"请选择一个文件作为压缩方法参考。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	refViewItem = GuiArchiveGetReferenceFileViewItem(app, &refIndex);
	if ( refViewItem == NULL || refViewItem->sourceIndex >= app->itemCount ) {
		return FALSE;
	}
	refItem = &app->items[refViewItem->sourceIndex];

	firstSelected = -1;
	ListView_SetItemState(app->list, -1, 0, LVIS_SELECTED);
	for ( i = 0; i < app->viewCount; ++i ) {
		const GuiViewItem* viewItem;
		const GuiArchiveItem* item;

		viewItem = &app->viewItems[i];
		if ( viewItem->kind != GUI_VIEW_ITEM_FILE || viewItem->sourceIndex >= app->itemCount ) {
			continue;
		}
		item = &app->items[viewItem->sourceIndex];
		if ( _wcsicmp(item->methodText, refItem->methodText) != 0 ) {
			continue;
		}
		ListView_SetItemState(app->list, (int)i, LVIS_SELECTED, LVIS_SELECTED);
		if ( firstSelected < 0 ) {
			firstSelected = (int)i;
		}
	}

	if ( refIndex >= 0 ) {
		ListView_SetItemState(app->list, refIndex, LVIS_FOCUSED, LVIS_FOCUSED);
		ListView_SetSelectionMark(app->list, refIndex);
		ListView_EnsureVisible(app->list, refIndex, FALSE);
	} else if ( firstSelected >= 0 ) {
		ListView_SetItemState(app->list, firstSelected, LVIS_FOCUSED, LVIS_FOCUSED);
		ListView_SetSelectionMark(app->list, firstSelected);
		ListView_EnsureVisible(app->list, firstSelected, FALSE);
	}

	GuiSetMenuState(app);
	GuiUpdateStatus(app);
	return TRUE;
}

BOOL GuiArchiveSelectSameFileType(GuiApp* app)
{
	GuiViewItem* refViewItem;
	const GuiArchiveItem* refItem;
	uint8_t refFileType;
	int refIndex;
	size_t i;
	int firstSelected;

	if ( !GuiArchiveCanSelectSameFileType(app) ) {
		MessageBoxW(app != NULL ? app->window : NULL, L"请选择一个文件作为 File Type 参考。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	refViewItem = GuiArchiveGetReferenceFileViewItem(app, &refIndex);
	if ( refViewItem == NULL || refViewItem->sourceIndex >= app->itemCount ) {
		return FALSE;
	}
	refItem = &app->items[refViewItem->sourceIndex];
	refFileType = GuiEntryFileType(refItem->flag);

	firstSelected = -1;
	ListView_SetItemState(app->list, -1, 0, LVIS_SELECTED);
	for ( i = 0; i < app->viewCount; ++i ) {
		const GuiViewItem* viewItem;
		const GuiArchiveItem* item;

		viewItem = &app->viewItems[i];
		if ( viewItem->kind != GUI_VIEW_ITEM_FILE || viewItem->sourceIndex >= app->itemCount ) {
			continue;
		}
		item = &app->items[viewItem->sourceIndex];
		if ( GuiEntryFileType(item->flag) != refFileType ) {
			continue;
		}
		ListView_SetItemState(app->list, (int)i, LVIS_SELECTED, LVIS_SELECTED);
		if ( firstSelected < 0 ) {
			firstSelected = (int)i;
		}
	}

	if ( refIndex >= 0 ) {
		ListView_SetItemState(app->list, refIndex, LVIS_FOCUSED, LVIS_FOCUSED);
		ListView_SetSelectionMark(app->list, refIndex);
		ListView_EnsureVisible(app->list, refIndex, FALSE);
	} else if ( firstSelected >= 0 ) {
		ListView_SetItemState(app->list, firstSelected, LVIS_FOCUSED, LVIS_FOCUSED);
		ListView_SetSelectionMark(app->list, firstSelected);
		ListView_EnsureVisible(app->list, firstSelected, FALSE);
	}

	GuiSetMenuState(app);
	GuiUpdateStatus(app);
	return TRUE;
}

BOOL GuiArchiveSelectSamePathAttr(GuiApp* app)
{
	GuiViewItem* refViewItem;
	const GuiArchiveItem* refItem;
	int refIndex;
	size_t i;
	int firstSelected;

	if ( !GuiArchiveCanSelectSamePathAttr(app) ) {
		MessageBoxW(app != NULL ? app->window : NULL, L"请选择 path 包中的一个文件作为 Platform Attr 参考。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	refViewItem = GuiArchiveGetReferenceFileViewItem(app, &refIndex);
	if ( refViewItem == NULL || refViewItem->sourceIndex >= app->itemCount ) {
		return FALSE;
	}
	refItem = &app->items[refViewItem->sourceIndex];

	firstSelected = -1;
	ListView_SetItemState(app->list, -1, 0, LVIS_SELECTED);
	for ( i = 0; i < app->viewCount; ++i ) {
		const GuiViewItem* viewItem;
		const GuiArchiveItem* item;

		viewItem = &app->viewItems[i];
		if ( viewItem->kind != GUI_VIEW_ITEM_FILE || viewItem->sourceIndex >= app->itemCount ) {
			continue;
		}
		item = &app->items[viewItem->sourceIndex];
		if ( item->attr != refItem->attr ) {
			continue;
		}
		ListView_SetItemState(app->list, (int)i, LVIS_SELECTED, LVIS_SELECTED);
		if ( firstSelected < 0 ) {
			firstSelected = (int)i;
		}
	}

	if ( refIndex >= 0 ) {
		ListView_SetItemState(app->list, refIndex, LVIS_FOCUSED, LVIS_FOCUSED);
		ListView_SetSelectionMark(app->list, refIndex);
		ListView_EnsureVisible(app->list, refIndex, FALSE);
	} else if ( firstSelected >= 0 ) {
		ListView_SetItemState(app->list, firstSelected, LVIS_FOCUSED, LVIS_FOCUSED);
		ListView_SetSelectionMark(app->list, firstSelected);
		ListView_EnsureVisible(app->list, firstSelected, FALSE);
	}

	GuiSetMenuState(app);
	GuiUpdateStatus(app);
	return TRUE;
}

static BOOL GuiBuildDuplicateMatchFlags(
	GuiApp* app,
	BOOL copiesOnly,
	BOOL** flagsOut,
	uint32_t* groupCountOut,
	uint32_t* fileCountOut,
	uint64_t* totalSizeOut,
	uint64_t* totalPackedOut)
{
	size_t i;
	BOOL* dupFlags;
	uint32_t groupCount;
	uint32_t fileCount;
	uint64_t totalSize;
	uint64_t totalPacked;

	if ( flagsOut != NULL ) {
		*flagsOut = NULL;
	}
	if ( groupCountOut != NULL ) {
		*groupCountOut = 0;
	}
	if ( fileCountOut != NULL ) {
		*fileCountOut = 0;
	}
	if ( totalSizeOut != NULL ) {
		*totalSizeOut = 0;
	}
	if ( totalPackedOut != NULL ) {
		*totalPackedOut = 0;
	}
	if ( app == NULL || app->archive == NULL || app->viewCount == 0 || flagsOut == NULL ) {
		return FALSE;
	}

	dupFlags = (BOOL*)calloc(app->viewCount, sizeof(*dupFlags));
	if ( dupFlags == NULL ) {
		MessageBoxW(app->window, L"内存不足。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	groupCount = 0;
	fileCount = 0;
	totalSize = 0;
	totalPacked = 0;
	for ( i = 0; i < app->viewCount; ++i ) {
		const GuiViewItem* leftView;
		const GuiArchiveItem* leftItem;
		BOOL hasPrevious;
		BOOL hasLater;
		size_t j;

		leftView = &app->viewItems[i];
		if ( leftView->kind != GUI_VIEW_ITEM_FILE || leftView->sourceIndex >= app->itemCount ) {
			continue;
		}
		leftItem = &app->items[leftView->sourceIndex];

		hasPrevious = FALSE;
		for ( j = 0; j < i; ++j ) {
			const GuiViewItem* prevView;
			const GuiArchiveItem* prevItem;

			prevView = &app->viewItems[j];
			if ( prevView->kind != GUI_VIEW_ITEM_FILE || prevView->sourceIndex >= app->itemCount ) {
				continue;
			}
			prevItem = &app->items[prevView->sourceIndex];
			if ( leftItem->fileHash == prevItem->fileHash && leftItem->fileSize == prevItem->fileSize ) {
				hasPrevious = TRUE;
				break;
			}
		}
		if ( hasPrevious ) {
			continue;
		}

		hasLater = FALSE;
		for ( j = i + 1; j < app->viewCount; ++j ) {
			const GuiViewItem* rightView;
			const GuiArchiveItem* rightItem;

			rightView = &app->viewItems[j];
			if ( rightView->kind != GUI_VIEW_ITEM_FILE || rightView->sourceIndex >= app->itemCount ) {
				continue;
			}
			rightItem = &app->items[rightView->sourceIndex];
			if ( leftItem->fileHash == rightItem->fileHash && leftItem->fileSize == rightItem->fileSize ) {
				hasLater = TRUE;
				dupFlags[j] = TRUE;
				fileCount++;
				totalSize += rightItem->fileSize;
				totalPacked += rightItem->packedSize;
			}
		}

		if ( hasLater ) {
			groupCount++;
			if ( !copiesOnly ) {
				dupFlags[i] = TRUE;
				fileCount++;
				totalSize += leftItem->fileSize;
				totalPacked += leftItem->packedSize;
			}
		}
	}

	*flagsOut = dupFlags;
	if ( groupCountOut != NULL ) {
		*groupCountOut = groupCount;
	}
	if ( fileCountOut != NULL ) {
		*fileCountOut = fileCount;
	}
	if ( totalSizeOut != NULL ) {
		*totalSizeOut = totalSize;
	}
	if ( totalPackedOut != NULL ) {
		*totalPackedOut = totalPacked;
	}
	return TRUE;
}

static BOOL GuiArchiveSelectDuplicateMatches(GuiApp* app, BOOL copiesOnly)
{
	size_t i;
	BOOL* dupFlags;
	int firstSelected;
	int focusIndex;

	if ( !GuiArchiveCanSelectDuplicateFiles(app) ) {
		return FALSE;
	}

	dupFlags = NULL;
	if ( !GuiBuildDuplicateMatchFlags(app, copiesOnly, &dupFlags, NULL, NULL, NULL, NULL) ) {
		return FALSE;
	}

	firstSelected = -1;
	ListView_SetItemState(app->list, -1, 0, LVIS_SELECTED);
	for ( i = 0; i < app->viewCount; ++i ) {
		if ( !dupFlags[i] ) {
			continue;
		}
		ListView_SetItemState(app->list, (int)i, LVIS_SELECTED, LVIS_SELECTED);
		if ( firstSelected < 0 ) {
			firstSelected = (int)i;
		}
	}
	free(dupFlags);

	if ( firstSelected < 0 ) {
		MessageBoxW(app->window, copiesOnly ? L"当前视图中没有检测到可单独清理的重复副本。" : L"当前视图中没有检测到重复文件。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		GuiSetMenuState(app);
		GuiUpdateStatus(app);
		return FALSE;
	}

	focusIndex = ListView_GetNextItem(app->list, -1, LVNI_FOCUSED);
	if ( focusIndex < 0 || !ListView_GetItemState(app->list, focusIndex, LVIS_SELECTED) ) {
		focusIndex = firstSelected;
	}
	ListView_SetItemState(app->list, focusIndex, LVIS_FOCUSED, LVIS_FOCUSED);
	ListView_SetSelectionMark(app->list, focusIndex);
	ListView_EnsureVisible(app->list, focusIndex, FALSE);
	GuiSetMenuState(app);
	GuiUpdateStatus(app);
	return TRUE;
}

BOOL GuiArchiveSelectDuplicateFiles(GuiApp* app)
{
	return GuiArchiveSelectDuplicateMatches(app, FALSE);
}

BOOL GuiArchiveSelectDuplicateCopies(GuiApp* app)
{
	if ( !GuiArchiveCanSelectDuplicateCopies(app) ) {
		return FALSE;
	}
	return GuiArchiveSelectDuplicateMatches(app, TRUE);
}

static int __cdecl GuiArchiveItemPosDescCompare(const void* leftPtr, const void* rightPtr);

BOOL GuiArchiveDeleteDuplicateCopies(GuiApp* app)
{
	BOOL* dupFlags;
	uint32_t groupCount;
	uint32_t fileCount;
	uint64_t totalSize;
	uint64_t totalPacked;
	GuiArchiveItem* items;
	WCHAR sizeText[64];
	WCHAR packedText[64];
	WCHAR prompt[512];
	size_t i;
	int itemIndex;

	if ( app == NULL || app->archive == NULL || !GuiArchiveCanDeleteDuplicateCopies(app) ) {
		return FALSE;
	}

	dupFlags = NULL;
	groupCount = 0;
	fileCount = 0;
	totalSize = 0;
	totalPacked = 0;
	if ( !GuiBuildDuplicateMatchFlags(app, TRUE, &dupFlags, &groupCount, &fileCount, &totalSize, &totalPacked) ) {
		return FALSE;
	}

	if ( fileCount == 0 ) {
		free(dupFlags);
		MessageBoxW(app->window, L"当前视图中没有检测到可单独清理的重复副本。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	GuiFormatUInt64(totalSize, sizeText, _countof(sizeText));
	GuiFormatUInt64(totalPacked, packedText, _countof(packedText));
	_snwprintf_s(
		prompt,
		_countof(prompt),
		_TRUNCATE,
		L"将删除当前视图中的重复副本。\r\n\r\n重复组数: %u\r\n待删文件: %u\r\n原始大小: %s\r\n压缩后大小: %s\r\n\r\n每组默认保留当前视图中排在最前的一个文件，是否继续？",
		groupCount,
		fileCount,
		sizeText,
		packedText);
	if ( MessageBoxW(app->window, prompt, XPKGUI_APP_TITLE, MB_YESNO | MB_ICONQUESTION) != IDYES ) {
		free(dupFlags);
		return FALSE;
	}

	items = (GuiArchiveItem*)malloc((size_t)fileCount * sizeof(*items));
	if ( items == NULL ) {
		free(dupFlags);
		MessageBoxW(app->window, L"内存不足。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	itemIndex = 0;
	for ( i = 0; i < app->viewCount; ++i ) {
		const GuiViewItem* viewItem;

		if ( !dupFlags[i] ) {
			continue;
		}
		viewItem = &app->viewItems[i];
		if ( viewItem->kind != GUI_VIEW_ITEM_FILE || viewItem->sourceIndex >= app->itemCount ) {
			continue;
		}
		items[itemIndex++] = app->items[viewItem->sourceIndex];
	}
	free(dupFlags);

	if ( itemIndex <= 0 ) {
		free(items);
		MessageBoxW(app->window, L"当前视图中没有检测到可单独清理的重复副本。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	qsort(items, (size_t)itemIndex, sizeof(*items), GuiArchiveItemPosDescCompare);
	for ( i = 0; i < (size_t)itemIndex; ++i ) {
		if ( xpkRemove(app->archive, items[i].pos) != XPK_OK ) {
			free(items);
			GuiShowArchiveError(app, L"删除重复副本");
			return FALSE;
		}
	}
	free(items);

	return GuiArchiveSave(app);
}

BOOL GuiArchiveInvertSelection(GuiApp* app)
{
	int focusIndex;
	size_t i;
	int firstSelected;

	if ( !GuiArchiveCanInvertSelection(app) ) {
		return FALSE;
	}

	focusIndex = ListView_GetNextItem(app->list, -1, LVNI_FOCUSED);
	firstSelected = -1;
	for ( i = 0; i < app->viewCount; ++i ) {
		UINT state;

		if ( app->viewItems[i].kind == GUI_VIEW_ITEM_PARENT ) {
			ListView_SetItemState(app->list, (int)i, 0, LVIS_SELECTED);
			continue;
		}
		state = ListView_GetItemState(app->list, (int)i, LVIS_SELECTED);
		if ( (state & LVIS_SELECTED) != 0 ) {
			ListView_SetItemState(app->list, (int)i, 0, LVIS_SELECTED);
		} else {
			ListView_SetItemState(app->list, (int)i, LVIS_SELECTED, LVIS_SELECTED);
			if ( firstSelected < 0 ) {
				firstSelected = (int)i;
			}
		}
	}

	if ( focusIndex < 0 ) {
		focusIndex = firstSelected;
	}
	if ( focusIndex >= 0 ) {
		ListView_SetItemState(app->list, focusIndex, LVIS_FOCUSED, LVIS_FOCUSED);
		ListView_SetSelectionMark(app->list, focusIndex);
		ListView_EnsureVisible(app->list, focusIndex, FALSE);
	}

	GuiSetMenuState(app);
	GuiUpdateStatus(app);
	return TRUE;
}

static BOOL GuiArchiveGetDirectorySummary(GuiApp* app, const WCHAR* dirPath, uint32_t* outFileCount, uint64_t* outSize, uint64_t* outPackedSize)
{
	size_t i;
	uint32_t fileCount;
	uint64_t totalSize;
	uint64_t totalPacked;

	if ( outFileCount != NULL ) {
		*outFileCount = 0;
	}
	if ( outSize != NULL ) {
		*outSize = 0;
	}
	if ( outPackedSize != NULL ) {
		*outPackedSize = 0;
	}
	if ( app == NULL || dirPath == NULL || dirPath[0] == L'\0' ) {
		return FALSE;
	}

	fileCount = 0;
	totalSize = 0;
	totalPacked = 0;
	for ( i = 0; i < app->itemCount; ++i ) {
		if ( _wcsicmp(app->items[i].name, dirPath) == 0 || GuiPathMatchFolderPrefix(app->items[i].name, dirPath, NULL) ) {
			fileCount++;
			totalSize += app->items[i].fileSize;
			totalPacked += app->items[i].packedSize;
		}
	}

	if ( outFileCount != NULL ) {
		*outFileCount = fileCount;
	}
	if ( outSize != NULL ) {
		*outSize = totalSize;
	}
	if ( outPackedSize != NULL ) {
		*outPackedSize = totalPacked;
	}
	return TRUE;
}

BOOL GuiArchiveShowSelectionProperties(GuiApp* app)
{
	GuiViewItem* viewItem;
	GuiArchiveItem* item;
	WCHAR text[2048];
	WCHAR sizeText[64];
	WCHAR packedText[64];
	WCHAR ratioText[32];
	WCHAR fileTypeText[64];
	WCHAR hashText[32];
	WCHAR createText[64];
	WCHAR accessText[64];
	uint32_t selectedRows;
	uint32_t fileCount;
	uint64_t totalSize;
	uint64_t totalPacked;

	if ( app == NULL || app->archive == NULL ) {
		return FALSE;
	}

	selectedRows = GuiArchiveSelectedCount(app);
	if ( selectedRows == 0 || !GuiArchiveCanCopySelection(app) ) {
		MessageBoxW(app->window, L"请选择文件或目录查看属性。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	if ( selectedRows > 1 ) {
		fileCount = 0;
		totalSize = 0;
		totalPacked = 0;
		if ( !GuiArchiveGetSelectionSummary(app, &fileCount, &totalSize, &totalPacked) ) {
			return FALSE;
		}
		GuiFormatUInt64(totalSize, sizeText, _countof(sizeText));
		GuiFormatUInt64(totalPacked, packedText, _countof(packedText));
		GuiFormatRatio(totalPacked, totalSize, ratioText, _countof(ratioText));
		_snwprintf_s(
			text,
			_countof(text),
			_TRUNCATE,
			L"Pack Type: %s\r\nSelected Rows: %u\r\nFiles: %u\r\nTotal Size: %s\r\nTotal Packed: %s\r\nRatio: %s",
			GuiPackTypeLabel(app->packType),
			(unsigned)selectedRows,
			(unsigned)fileCount,
			sizeText,
			packedText,
			ratioText);
		MessageBoxW(app->window, text, L"Selection Properties", MB_OK | MB_ICONINFORMATION);
		return TRUE;
	}

	viewItem = GuiArchiveGetSingleSelectedViewItem(app);
	if ( viewItem == NULL || viewItem->kind == GUI_VIEW_ITEM_PARENT ) {
		MessageBoxW(app->window, L"请选择文件或目录查看属性。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	if ( viewItem->kind == GUI_VIEW_ITEM_DIR ) {
		fileCount = 0;
		totalSize = 0;
		totalPacked = 0;
		if ( !GuiArchiveGetDirectorySummary(app, viewItem->fullPath, &fileCount, &totalSize, &totalPacked) ) {
			return FALSE;
		}
		GuiFormatUInt64(totalSize, sizeText, _countof(sizeText));
		GuiFormatUInt64(totalPacked, packedText, _countof(packedText));
		GuiFormatRatio(totalPacked, totalSize, ratioText, _countof(ratioText));
		_snwprintf_s(
			text,
			_countof(text),
			_TRUNCATE,
			L"Type: Directory\r\nPack Type: %s\r\nName: %s\r\nPath: %s\r\nFiles: %u\r\nTotal Size: %s\r\nTotal Packed: %s\r\nRatio: %s",
			GuiPackTypeLabel(app->packType),
			viewItem->name,
			viewItem->fullPath,
			(unsigned)fileCount,
			sizeText,
			packedText,
			ratioText);
		MessageBoxW(app->window, text, L"Selection Properties", MB_OK | MB_ICONINFORMATION);
		return TRUE;
	}

	item = GuiArchiveGetSingleSelectedItem(app);
	if ( item == NULL ) {
		return FALSE;
	}
	GuiFormatUInt64(item->fileSize, sizeText, _countof(sizeText));
	GuiFormatUInt64(item->packedSize, packedText, _countof(packedText));
	GuiFormatRatio(item->packedSize, item->fileSize, ratioText, _countof(ratioText));
	GuiFormatFileType(GuiEntryFileType(item->flag), fileTypeText, _countof(fileTypeText));
	_snwprintf_s(hashText, _countof(hashText), _TRUNCATE, L"0x%08X", item->fileHash);
	GuiFormatTime(item->createTime, createText, _countof(createText));
	GuiFormatTime(item->accessTime, accessText, _countof(accessText));
	if ( GuiIsPathPackType(app->packType) ) {
		_snwprintf_s(
			text,
			_countof(text),
			_TRUNCATE,
			L"Type: File\r\nPack Type: %s\r\nName: %s\r\nPath: %s\r\nSize: %s\r\nPacked: %s\r\nRatio: %s\r\nMethod: %s\r\nFile Type: %s\r\nHash: %s\r\nCreated: %s\r\nModified: %s\r\nAccessed: %s\r\nAttr: %s\r\nPos: %u",
			GuiPackTypeLabel(app->packType),
			viewItem->name,
			item->name,
			sizeText,
			packedText,
			ratioText,
			item->methodText,
			fileTypeText,
			hashText,
			createText[0] != L'\0' ? createText : L"(none)",
			item->modifiedText[0] != L'\0' ? item->modifiedText : L"(none)",
			accessText[0] != L'\0' ? accessText : L"(none)",
			item->attrText[0] != L'\0' ? item->attrText : L"(none)",
			(unsigned)item->pos);
	} else if ( app->packType == XPK_PACK_INDEX ) {
		_snwprintf_s(
			text,
			_countof(text),
			_TRUNCATE,
			L"Type: File\r\nPack Type: %s\r\nName: %s\r\nIndex: %s\r\nSize: %s\r\nPacked: %s\r\nRatio: %s\r\nMethod: %s\r\nFile Type: %s\r\nHash: %s\r\nPos: %u",
			GuiPackTypeLabel(app->packType),
			viewItem->name,
			item->idText,
			sizeText,
			packedText,
			ratioText,
			item->methodText,
			fileTypeText,
			hashText,
			(unsigned)item->pos);
	} else {
		_snwprintf_s(
			text,
			_countof(text),
			_TRUNCATE,
			L"Type: File\r\nPack Type: %s\r\nName: %s\r\nID: %s\r\nSize: %s\r\nPacked: %s\r\nRatio: %s\r\nMethod: %s\r\nFile Type: %s\r\nHash: %s\r\nPos: %u\r\nInfoExt Size: %u",
			GuiPackTypeLabel(app->packType),
			viewItem->name,
			item->idText,
			sizeText,
			packedText,
			ratioText,
			item->methodText,
			fileTypeText,
			hashText,
			(unsigned)item->pos,
			(unsigned)app->infoExtSize);
	}
	MessageBoxW(app->window, text, L"Selection Properties", MB_OK | MB_ICONINFORMATION);
	return TRUE;
}

static BOOL GuiPathExists(const WCHAR* path)
{
	return path != NULL && path[0] != L'\0' && GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES;
}

static uint32_t GuiCountExistingExtractTargets(const GuiArchiveItem* items, int count, xpkPackType packType, const WCHAR* destPath, WCHAR* samplePath, size_t cchSamplePath)
{
	uint32_t existingCount;
	int i;

	if ( samplePath != NULL && cchSamplePath > 0 ) {
		samplePath[0] = L'\0';
	}
	if ( items == NULL || count <= 0 || destPath == NULL || destPath[0] == L'\0' ) {
		return 0;
	}

	existingCount = 0;
	for ( i = 0; i < count; ++i ) {
		WCHAR outPath[XPKGUI_MAX_TEMP_PATH];

		GuiBuildExtractOutputPath(packType, destPath, &items[i], outPath, _countof(outPath));
		if ( GuiPathExists(outPath) ) {
			if ( existingCount == 0 && samplePath != NULL && cchSamplePath > 0 ) {
				wcsncpy_s(samplePath, cchSamplePath, outPath, _TRUNCATE);
			}
			existingCount++;
		}
	}
	return existingCount;
}

static int GuiPromptExtractOverwrite(HWND owner, uint32_t existingCount, const WCHAR* samplePath)
{
	WCHAR text[1024];

	if ( existingCount <= 1 ) {
		_snwprintf_s(
			text,
			_countof(text),
			_TRUNCATE,
			L"目标文件已存在：\r\n\r\n%s\r\n\r\n是：全部覆盖\r\n否：全部跳过\r\n取消：终止本次解压",
			(samplePath != NULL && samplePath[0] != L'\0') ? samplePath : L"(unknown)");
	} else {
		_snwprintf_s(
			text,
			_countof(text),
			_TRUNCATE,
			L"检测到 %u 个目标文件已存在。\r\n\r\n例如：%s\r\n\r\n是：全部覆盖\r\n否：全部跳过\r\n取消：终止本次解压",
			(unsigned)existingCount,
			(samplePath != NULL && samplePath[0] != L'\0') ? samplePath : L"(unknown)");
	}
	return MessageBoxW(owner, text, XPKGUI_APP_TITLE, MB_ICONQUESTION | MB_YESNOCANCEL);
}

BOOL GuiArchiveCanExtractSelection(GuiApp* app)
{
	int index;

	if ( app == NULL || app->list == NULL ) {
		return FALSE;
	}

	index = -1;
	while ( TRUE ) {
		GuiViewItem* viewItem;

		index = ListView_GetNextItem(app->list, index, LVNI_SELECTED);
		if ( index < 0 ) {
			break;
		}
		viewItem = GuiArchiveGetViewItemByListIndex(app, index);
		if ( viewItem != NULL && (viewItem->kind == GUI_VIEW_ITEM_FILE || viewItem->kind == GUI_VIEW_ITEM_DIR) ) {
			return TRUE;
		}
	}
	return FALSE;
}

BOOL GuiArchiveCanVerifySelection(GuiApp* app)
{
	return GuiArchiveCanExtractSelection(app);
}

BOOL GuiArchiveCanDeleteSelection(GuiApp* app)
{
	int index;

	if ( app == NULL || app->list == NULL ) {
		return FALSE;
	}

	index = -1;
	while ( TRUE ) {
		GuiViewItem* viewItem;

		index = ListView_GetNextItem(app->list, index, LVNI_SELECTED);
		if ( index < 0 ) {
			break;
		}
		viewItem = GuiArchiveGetViewItemByListIndex(app, index);
		if ( viewItem == NULL ) {
			continue;
		}
		if ( viewItem->kind == GUI_VIEW_ITEM_FILE ) {
			return TRUE;
		}
		if ( viewItem->kind == GUI_VIEW_ITEM_DIR && GuiIsPathPackType(app->packType) ) {
			return TRUE;
		}
	}
	return FALSE;
}

BOOL GuiArchiveCanRenameSelection(GuiApp* app)
{
	GuiViewItem* viewItem;

	if ( app == NULL || !GuiIsPathPackType(app->packType) ) {
		return FALSE;
	}

	viewItem = GuiArchiveGetSingleSelectedViewItem(app);
	return viewItem != NULL && (viewItem->kind == GUI_VIEW_ITEM_FILE || viewItem->kind == GUI_VIEW_ITEM_DIR);
}

BOOL GuiArchiveCanMoveSelection(GuiApp* app)
{
	int index;

	if ( app == NULL || app->archive == NULL || app->list == NULL || !GuiIsPathPackType(app->packType) ) {
		return FALSE;
	}

	index = -1;
	while ( TRUE ) {
		GuiViewItem* viewItem;

		index = ListView_GetNextItem(app->list, index, LVNI_SELECTED);
		if ( index < 0 ) {
			break;
		}
		viewItem = GuiArchiveGetViewItemByListIndex(app, index);
		if ( viewItem != NULL && (viewItem->kind == GUI_VIEW_ITEM_FILE || viewItem->kind == GUI_VIEW_ITEM_DIR) ) {
			return TRUE;
		}
	}
	return FALSE;
}

BOOL GuiArchiveCanCopyToSelection(GuiApp* app)
{
	return GuiArchiveCanMoveSelection(app);
}

BOOL GuiArchiveCanSetSelectionFileIndex(GuiApp* app)
{
	return app != NULL
		&& app->archive != NULL
		&& app->packType == XPK_PACK_INDEX
		&& GuiArchiveGetSingleSelectedItem(app) != NULL;
}

BOOL GuiArchiveCanSetSelectionFileType(GuiApp* app)
{
	size_t* sourceIndexes;
	size_t count;
	BOOL ok;

	if ( app == NULL || app->archive == NULL ) {
		return FALSE;
	}
	ok = GuiCollectSelectedSourceIndexes(app, &sourceIndexes, &count);
	free(sourceIndexes);
	return ok && count > 0;
}

BOOL GuiArchiveCanSetSelectionPathAttr(GuiApp* app)
{
	return app != NULL && GuiIsPathPackType(app->packType) && GuiArchiveCanDeleteSelection(app);
}

static BOOL GuiBrowseHistoryMatchesState(const GuiBrowseHistoryEntry* entry, const WCHAR* folder, BOOL flatView)
{
	if ( entry == NULL || folder == NULL ) {
		return FALSE;
	}
	return entry->flatView == flatView && _wcsicmp(entry->folder, folder) == 0;
}

static void GuiArchiveRememberBrowseState(GuiApp* app)
{
	WCHAR normalized[XPKGUI_ITEM_TEXT];
	UINT i;

	if ( app == NULL || !GuiIsPathPackType(app->packType) ) {
		return;
	}

	GuiNormalizeViewPath(app->currentFolder, normalized, _countof(normalized));
	if ( app->navHistoryCount > 0 && app->navHistoryIndex < app->navHistoryCount
		&& GuiBrowseHistoryMatchesState(&app->navHistory[app->navHistoryIndex], normalized, app->flatView) ) {
		return;
	}

	if ( app->navHistoryIndex + 1 < app->navHistoryCount ) {
		app->navHistoryCount = app->navHistoryIndex + 1;
	}
	if ( app->navHistoryCount >= XPKGUI_MAX_NAV_HISTORY ) {
		for ( i = 1; i < app->navHistoryCount; ++i ) {
			app->navHistory[i - 1] = app->navHistory[i];
		}
		app->navHistoryCount--;
		if ( app->navHistoryIndex > 0 ) {
			app->navHistoryIndex--;
		}
	}

	wcsncpy_s(app->navHistory[app->navHistoryCount].folder, _countof(app->navHistory[0].folder), normalized, _TRUNCATE);
	app->navHistory[app->navHistoryCount].flatView = app->flatView;
	app->navHistoryCount++;
	app->navHistoryIndex = app->navHistoryCount - 1;
}

static BOOL GuiArchiveApplyBrowseState(GuiApp* app, const WCHAR* folder, BOOL flatView, BOOL recordHistory)
{
	WCHAR normalized[XPKGUI_ITEM_TEXT];
	WCHAR oldFolder[XPKGUI_ITEM_TEXT];
	BOOL oldFlatView;

	if ( app == NULL || !GuiIsPathPackType(app->packType) ) {
		return FALSE;
	}

	GuiNormalizeViewPath(folder, normalized, _countof(normalized));
	if ( !flatView && normalized[0] != L'\0' && !GuiArchiveFolderExists(app, normalized) ) {
		return FALSE;
	}
	if ( _wcsicmp(app->currentFolder, normalized) == 0 && app->flatView == flatView ) {
		return TRUE;
	}

	wcsncpy_s(oldFolder, _countof(oldFolder), app->currentFolder, _TRUNCATE);
	oldFlatView = app->flatView;

	wcsncpy_s(app->currentFolder, _countof(app->currentFolder), normalized, _TRUNCATE);
	app->flatView = flatView;
	if ( !GuiRenderArchiveView(app) ) {
		wcsncpy_s(app->currentFolder, _countof(app->currentFolder), oldFolder, _TRUNCATE);
		app->flatView = oldFlatView;
		return FALSE;
	}

	if ( recordHistory && !app->navHistoryLocked ) {
		GuiArchiveRememberBrowseState(app);
		GuiSetMenuState(app);
		GuiUpdateNavigationBar(app);
	}
	return TRUE;
}

static BOOL GuiArchiveFolderExists(GuiApp* app, const WCHAR* folder)
{
	size_t i;
	WCHAR normalized[XPKGUI_ITEM_TEXT];

	if ( app == NULL || !GuiIsPathPackType(app->packType) ) {
		return FALSE;
	}

	GuiNormalizeViewPath(folder, normalized, _countof(normalized));
	if ( normalized[0] == L'\0' ) {
		return TRUE;
	}

	for ( i = 0; i < app->itemCount; ++i ) {
		if ( GuiPathMatchFolderPrefix(app->items[i].name, normalized, NULL) ) {
			return TRUE;
		}
	}
	return FALSE;
}

BOOL GuiArchiveCanBrowseBack(GuiApp* app)
{
	return app != NULL && GuiIsPathPackType(app->packType) && app->navHistoryCount > 0 && app->navHistoryIndex > 0;
}

BOOL GuiArchiveCanBrowseForward(GuiApp* app)
{
	return app != NULL && GuiIsPathPackType(app->packType) && app->navHistoryIndex + 1 < app->navHistoryCount;
}

BOOL GuiArchiveBrowseBack(GuiApp* app)
{
	GuiBrowseHistoryEntry entry;

	if ( !GuiArchiveCanBrowseBack(app) ) {
		return FALSE;
	}

	app->navHistoryIndex--;
	entry = app->navHistory[app->navHistoryIndex];
	app->navHistoryLocked = TRUE;
	if ( !GuiArchiveApplyBrowseState(app, entry.folder, entry.flatView, FALSE) ) {
		app->navHistoryLocked = FALSE;
		app->navHistoryIndex++;
		GuiSetMenuState(app);
		GuiUpdateNavigationBar(app);
		return FALSE;
	}
	app->navHistoryLocked = FALSE;
	return TRUE;
}

BOOL GuiArchiveBrowseForward(GuiApp* app)
{
	GuiBrowseHistoryEntry entry;

	if ( !GuiArchiveCanBrowseForward(app) ) {
		return FALSE;
	}

	app->navHistoryIndex++;
	entry = app->navHistory[app->navHistoryIndex];
	app->navHistoryLocked = TRUE;
	if ( !GuiArchiveApplyBrowseState(app, entry.folder, entry.flatView, FALSE) ) {
		app->navHistoryLocked = FALSE;
		app->navHistoryIndex--;
		GuiSetMenuState(app);
		GuiUpdateNavigationBar(app);
		return FALSE;
	}
	app->navHistoryLocked = FALSE;
	return TRUE;
}

BOOL GuiArchiveBrowseParent(GuiApp* app)
{
	WCHAR parentPath[XPKGUI_ITEM_TEXT];

	if ( app == NULL || !GuiIsPathPackType(app->packType) || app->flatView || app->currentFolder[0] == L'\0' ) {
		return FALSE;
	}

	GuiGetParentViewPath(app->currentFolder, parentPath, _countof(parentPath));
	return GuiArchiveApplyBrowseState(app, parentPath, FALSE, TRUE);
}

BOOL GuiArchiveBrowseRoot(GuiApp* app)
{
	if ( app == NULL || !GuiIsPathPackType(app->packType) || app->flatView ) {
		return FALSE;
	}
	if ( app->currentFolder[0] == L'\0' ) {
		return TRUE;
	}
	return GuiArchiveApplyBrowseState(app, L"", FALSE, TRUE);
}

BOOL GuiArchiveLocateInTree(GuiApp* app)
{
	GuiViewItem* viewItem;
	WCHAR folder[XPKGUI_ITEM_TEXT];
	BOOL oldFlatView;

	if ( app == NULL ) {
		return FALSE;
	}

	viewItem = GuiArchiveGetReferenceFileViewItem(app, NULL);
	if ( viewItem == NULL || !GuiIsPathPackType(app->packType) || !app->flatView ) {
		MessageBoxW(app->window, L"请先在 Flat View 中选中一个文件。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	GuiGetParentViewPath(viewItem->fullPath, folder, _countof(folder));
	oldFlatView = app->flatView;
	if ( !GuiArchiveApplyBrowseState(app, folder, FALSE, TRUE) ) {
		app->flatView = oldFlatView;
		return FALSE;
	}
	return TRUE;
}

BOOL GuiArchiveActivateSelection(GuiApp* app)
{
	GuiViewItem* viewItem;

	if ( app == NULL ) {
		return FALSE;
	}

	viewItem = GuiArchiveGetSingleSelectedViewItem(app);
	if ( viewItem == NULL ) {
		return FALSE;
	}

	if ( viewItem->kind == GUI_VIEW_ITEM_PARENT ) {
		return GuiArchiveBrowseParent(app);
	}
	if ( viewItem->kind == GUI_VIEW_ITEM_DIR ) {
		return GuiArchiveApplyBrowseState(app, viewItem->fullPath, FALSE, TRUE);
	}
	if ( viewItem->kind == GUI_VIEW_ITEM_FILE ) {
		return GuiArchiveOpenSelection(app);
	}
	return FALSE;
}

BOOL GuiArchiveToggleFlatView(GuiApp* app)
{
	if ( app == NULL || app->archive == NULL || !GuiIsPathPackType(app->packType) ) {
		return FALSE;
	}
	return GuiArchiveApplyBrowseState(app, app->currentFolder, app->flatView ? FALSE : TRUE, TRUE);
}

BOOL GuiArchiveNavigateToFolder(GuiApp* app, const WCHAR* folder)
{
	WCHAR normalized[XPKGUI_ITEM_TEXT];

	if ( app == NULL || !GuiIsPathPackType(app->packType) || app->flatView ) {
		return FALSE;
	}

	GuiNormalizeViewPath(folder, normalized, _countof(normalized));
	if ( !GuiArchiveFolderExists(app, normalized) ) {
		return FALSE;
	}
	return GuiArchiveApplyBrowseState(app, normalized, FALSE, TRUE);
}

BOOL GuiArchiveSetFilter(GuiApp* app, const WCHAR* filterText)
{
	WCHAR normalized[XPKGUI_ITEM_TEXT];

	if ( app == NULL ) {
		return FALSE;
	}

	if ( filterText == NULL ) {
		normalized[0] = L'\0';
	} else {
		wcsncpy_s(normalized, _countof(normalized), filterText, _TRUNCATE);
		StrTrimW(normalized, L" \t");
	}

	if ( _wcsicmp(app->filterText, normalized) == 0 ) {
		return TRUE;
	}

	wcsncpy_s(app->filterText, _countof(app->filterText), normalized, _TRUNCATE);
	return GuiRenderArchiveView(app);
}

static BOOL GuiPathRelativeToBase(const WCHAR* filePath, const WCHAR* baseDir, WCHAR* relBuf, size_t cchRelBuf)
{
	size_t baseLen;
	const WCHAR* rel;

	baseLen = wcslen(baseDir);
	if ( _wcsnicmp(filePath, baseDir, baseLen) == 0 ) {
		rel = filePath + baseLen;
		if ( rel[0] == L'\\' || rel[0] == L'/' ) {
			rel++;
		}
		if ( rel[0] != L'\0' ) {
			wcsncpy_s(relBuf, cchRelBuf, rel, _TRUNCATE);
			return TRUE;
		}
	}

	wcsncpy_s(relBuf, cchRelBuf, PathFindFileNameW(filePath), _TRUNCATE);
	return TRUE;
}

static uint8_t GuiEntryFileType(uint32_t flag)
{
	return (uint8_t)((flag & XPK_FLAG_TYPE_MASK) >> 4);
}

static const WCHAR* GuiFindFileTypeLabel(uint8_t fileType)
{
	size_t i;

	for ( i = 0; i < _countof(arrGuiFileTypeNames); ++i ) {
		if ( arrGuiFileTypeNames[i].value == fileType ) {
			return arrGuiFileTypeNames[i].label;
		}
	}
	return NULL;
}

static void GuiFormatFileType(uint8_t fileType, WCHAR* buf, size_t cchBuf)
{
	const WCHAR* label;

	label = GuiFindFileTypeLabel(fileType);
	if ( label != NULL ) {
		_snwprintf_s(buf, cchBuf, _TRUNCATE, L"%s (%u)", label, (unsigned)fileType);
	} else {
		_snwprintf_s(buf, cchBuf, _TRUNCATE, L"Custom (%u)", (unsigned)fileType);
	}
}

static void GuiPathToPackageUtf8(const WCHAR* relPath, char* utf8Buf, size_t cchUtf8Buf)
{
	WCHAR temp[MAX_PATH];
	size_t i;

	wcsncpy_s(temp, _countof(temp), relPath, _TRUNCATE);
	for ( i = 0; temp[i] != L'\0'; ++i ) {
		if ( temp[i] == L'\\' ) {
			temp[i] = L'/';
		}
	}
	(void)GuiUtf8FromWide(temp, utf8Buf, cchUtf8Buf);
}

static void GuiTrimWhitespaceCopy(const WCHAR* src, WCHAR* dst, size_t cchDst)
{
	const WCHAR* start;
	const WCHAR* end;
	size_t len;

	if ( dst == NULL || cchDst == 0 ) {
		return;
	}
	dst[0] = L'\0';
	if ( src == NULL ) {
		return;
	}

	start = src;
	while ( *start != L'\0' && iswspace((wint_t)*start) ) {
		start++;
	}

	end = start + wcslen(start);
	while ( end > start && iswspace((wint_t)end[-1]) ) {
		end--;
	}

	len = (size_t)(end - start);
	if ( len >= cchDst ) {
		len = cchDst - 1;
	}
	wmemcpy(dst, start, len);
	dst[len] = L'\0';
}

static BOOL GuiParseInt64Input(const WCHAR* text, int64_t* outValue)
{
	WCHAR trimmed[64];
	WCHAR* endPtr;
	long long value;

	if ( outValue == NULL ) {
		return FALSE;
	}

	GuiTrimWhitespaceCopy(text, trimmed, _countof(trimmed));
	if ( trimmed[0] == L'\0' ) {
		return FALSE;
	}

	errno = 0;
	value = _wcstoi64(trimmed, &endPtr, 10);
	if ( errno != 0 || endPtr == trimmed || *endPtr != L'\0' ) {
		return FALSE;
	}

	*outValue = (int64_t)value;
	return TRUE;
}

static BOOL GuiParseUInt32InputFlexible(const WCHAR* text, uint32_t* outValue)
{
	WCHAR trimmed[64];
	WCHAR* endPtr;
	unsigned long long value;

	if ( outValue == NULL ) {
		return FALSE;
	}

	GuiTrimWhitespaceCopy(text, trimmed, _countof(trimmed));
	if ( trimmed[0] == L'\0' ) {
		return FALSE;
	}

	errno = 0;
	value = wcstoull(trimmed, &endPtr, 0);
	if ( errno != 0 || endPtr == trimmed || *endPtr != L'\0' || value > 0xFFFFFFFFull ) {
		return FALSE;
	}

	*outValue = (uint32_t)value;
	return TRUE;
}

static BOOL GuiParseFileTypeInput(const WCHAR* text, uint8_t* outValue)
{
	WCHAR trimmed[64];
	uint32_t numericValue;
	size_t i;

	if ( outValue == NULL ) {
		return FALSE;
	}

	GuiTrimWhitespaceCopy(text, trimmed, _countof(trimmed));
	if ( trimmed[0] == L'\0' ) {
		return FALSE;
	}
	if ( GuiParseUInt32InputFlexible(trimmed, &numericValue) ) {
		if ( numericValue > 15u ) {
			return FALSE;
		}
		*outValue = (uint8_t)numericValue;
		return TRUE;
	}

	for ( i = 0; i < _countof(arrGuiFileTypeNames); ++i ) {
		if ( _wcsicmp(trimmed, arrGuiFileTypeNames[i].label) == 0 || _wcsicmp(trimmed, arrGuiFileTypeNames[i].token) == 0 ) {
			*outValue = arrGuiFileTypeNames[i].value;
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL GuiNormalizePackageRoot(const WCHAR* input, WCHAR* rootBuf, size_t cchRootBuf)
{
	WCHAR trimmed[XPKGUI_ITEM_TEXT];
	size_t src;
	size_t dst;
	BOOL lastSlash;

	if ( rootBuf == NULL || cchRootBuf == 0 ) {
		return FALSE;
	}

	GuiTrimWhitespaceCopy(input, trimmed, _countof(trimmed));
	dst = 0;
	lastSlash = FALSE;
	for ( src = 0; trimmed[src] != L'\0'; ++src ) {
		WCHAR ch;

		ch = trimmed[src];
		if ( ch == L'\\' ) {
			ch = L'/';
		}
		if ( ch == L'/' ) {
			if ( dst == 0 || lastSlash ) {
				lastSlash = TRUE;
				continue;
			}
			rootBuf[dst++] = L'/';
			lastSlash = TRUE;
			continue;
		}
		if ( ch == L':' ) {
			return FALSE;
		}
		if ( dst + 1 >= cchRootBuf ) {
			return FALSE;
		}
		rootBuf[dst++] = ch;
		lastSlash = FALSE;
	}
	while ( dst > 0 && rootBuf[dst - 1] == L'/' ) {
		dst--;
	}
	rootBuf[dst] = L'\0';

	if ( wcsstr(rootBuf, L"/./") != NULL || wcsstr(rootBuf, L"/../") != NULL ) {
		return FALSE;
	}
	if ( wcscmp(rootBuf, L".") == 0 || wcscmp(rootBuf, L"..") == 0 ) {
		return FALSE;
	}
	if ( wcsncmp(rootBuf, L"./", 2) == 0 || wcsncmp(rootBuf, L"../", 3) == 0 ) {
		return FALSE;
	}
	if ( dst >= 2 && wcscmp(rootBuf + dst - 2, L"/.") == 0 ) {
		return FALSE;
	}
	if ( dst >= 3 && wcscmp(rootBuf + dst - 3, L"/..") == 0 ) {
		return FALSE;
	}
	return TRUE;
}

static void GuiBuildPackagePathForAdd(const WCHAR* packageRoot, const WCHAR* relPath, WCHAR* packagePath, size_t cchPackagePath)
{
	if ( packagePath == NULL || cchPackagePath == 0 ) {
		return;
	}
	if ( packageRoot != NULL && packageRoot[0] != L'\0' ) {
		_snwprintf_s(packagePath, cchPackagePath, _TRUNCATE, L"%s\\%s", packageRoot, relPath != NULL ? relPath : L"");
	} else {
		wcsncpy_s(packagePath, cchPackagePath, relPath != NULL ? relPath : L"", _TRUNCATE);
	}
}

static int64_t GuiFindNextIndexSeed(GuiApp* app)
{
	int64_t maxIndex;
	size_t i;

	maxIndex = 0;
	for ( i = 0; i < app->itemCount; ++i ) {
		if ( app->items[i].fileIndex > maxIndex ) {
			maxIndex = app->items[i].fileIndex;
		}
	}
	return maxIndex + 1;
}

#define WM_XPKGUI_TASK_DONE (WM_APP + 200)
#define XPKGUI_TASK_TIMER_ID 1

typedef enum GuiTaskKind {
	GUI_TASK_ADD = 1,
	GUI_TASK_CREATE_ADD,
	GUI_TASK_REPLACE,
	GUI_TASK_EXTRACT,
	GUI_TASK_VERIFY,
	GUI_TASK_BUILD
} GuiTaskKind;

typedef enum GuiTaskResult {
	GUI_TASK_RESULT_FAILED = 0,
	GUI_TASK_RESULT_SUCCESS = 1,
	GUI_TASK_RESULT_CANCELLED = 2
} GuiTaskResult;

typedef struct GuiTaskState {
	GuiTaskKind kind;
	GuiApp* app;
	HWND dialog;
	HANDLE thread;
	CRITICAL_SECTION detailLock;
	LONG cancelRequested;
	LONG progressCurrent;
	LONG progressTotal;
	BOOL canCancel;
	BOOL marquee;
	BOOL skipExisting;
	GuiTaskResult dialogResult;
	xpkPackType packType;
	uint8_t writePolicy;
	WCHAR title[64];
	WCHAR archivePath[XPKGUI_MAX_TEMP_PATH];
	WCHAR destPath[XPKGUI_MAX_TEMP_PATH];
	WCHAR successText[128];
	WCHAR errorText[1024];
	WCHAR detailText[1024];
	WCHAR addPackageRoot[XPKGUI_ITEM_TEXT];
	int64_t addStartIndex;
	GuiArchiveOptions createOptions;
	int inputCount;
	WCHAR** inputs;
	GuiArchiveItem* extractItems;
	int extractCount;
} GuiTaskState;

static GuiTaskResult GuiRunTaskDialog(GuiTaskState* task);

static void GuiTaskSetErrorText(GuiTaskState* task, const WCHAR* text)
{
	wcsncpy_s(task->errorText, _countof(task->errorText), text, _TRUNCATE);
}

static void GuiTaskSetDetailText(GuiTaskState* task, const WCHAR* text)
{
	EnterCriticalSection(&task->detailLock);
	if ( text == NULL ) {
		task->detailText[0] = L'\0';
	} else {
		wcsncpy_s(task->detailText, _countof(task->detailText), text, _TRUNCATE);
	}
	LeaveCriticalSection(&task->detailLock);
}

static void GuiTaskSetArchiveError(GuiTaskState* task, xpkObject archive, const WCHAR* actionText)
{
	WCHAR wideError[768];
	WCHAR fullText[1024];
	const char* sError;

	sError = xpkLastErrorMessage(archive);
	if ( sError == NULL || sError[0] == '\0' ) {
		sError = "unknown error";
	}
	if ( !GuiWideFromUtf8(sError, wideError, _countof(wideError)) ) {
		wcsncpy_s(wideError, _countof(wideError), L"unknown error", _TRUNCATE);
	}
	_snwprintf_s(fullText, _countof(fullText), _TRUNCATE, L"%s失败。\n\n%s", actionText, wideError);
	GuiTaskSetErrorText(task, fullText);
}

static void GuiTaskSetSystemError(GuiTaskState* task, const WCHAR* actionText, DWORD err)
{
	WCHAR systemText[768];
	WCHAR fullText[1024];
	DWORD len;

	len = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, 0, systemText, (DWORD)_countof(systemText), NULL);
	if ( len == 0 ) {
		_snwprintf_s(systemText, _countof(systemText), _TRUNCATE, L"系统错误 %lu", err);
	}
	_snwprintf_s(fullText, _countof(fullText), _TRUNCATE, L"%s失败。\n\n%s", actionText, systemText);
	GuiTaskSetErrorText(task, fullText);
}

static const WCHAR* GuiTaskGetArchivePath(const GuiTaskState* task)
{
	if ( task == NULL ) {
		return NULL;
	}
	if ( task->archivePath[0] != L'\0' ) {
		return task->archivePath;
	}
	if ( task->app != NULL && task->app->archivePath[0] != L'\0' ) {
		return task->app->archivePath;
	}
	return NULL;
}

static BOOL GuiTaskOpenArchiveWorker(GuiTaskState* task, BOOL readonly, BOOL createIfMissing, const WCHAR* actionText, xpkObject* archiveOut)
{
	const WCHAR* archivePath;
	char utf8Path[XPKGUI_MAX_TEMP_PATH * 4];
	xpkOpenOptions openOpt;
	xpkObject archive;

	if ( archiveOut == NULL ) {
		GuiTaskSetErrorText(task, L"任务参数无效。");
		return FALSE;
	}

	archivePath = GuiTaskGetArchivePath(task);
	if ( archivePath == NULL || archivePath[0] == L'\0' ) {
		GuiTaskSetErrorText(task, L"归档路径为空。");
		return FALSE;
	}

	GuiTaskSetDetailText(task, archivePath);
	if ( !GuiUtf8FromWide(archivePath, utf8Path, sizeof(utf8Path)) ) {
		GuiTaskSetErrorText(task, L"路径转换失败。");
		return FALSE;
	}

	ZeroMemory(&openOpt, sizeof(openOpt));
	openOpt.readonly = readonly ? 1 : 0;
	openOpt.createIfMissing = createIfMissing ? 1 : 0;
	openOpt.bufferedDefault = (task->writePolicy == XPK_WRITE_BUFFERED) ? 1 : 0;

	archive = xpkOpen(utf8Path, &openOpt);
	if ( archive == NULL ) {
		GuiTaskSetArchiveError(task, NULL, actionText);
		return FALSE;
	}

	*archiveOut = archive;
	return TRUE;
}

static DWORD GuiCountFolderFiles(const WCHAR* folderPath)
{
	WCHAR searchPath[MAX_PATH];
	WIN32_FIND_DATAW ffd;
	HANDLE hFind;
	DWORD count;

	_snwprintf_s(searchPath, _countof(searchPath), _TRUNCATE, L"%s\\*", folderPath);
	hFind = FindFirstFileW(searchPath, &ffd);
	if ( hFind == INVALID_HANDLE_VALUE ) {
		DWORD err = GetLastError();
		return (err == ERROR_FILE_NOT_FOUND) ? 0 : 0;
	}

	count = 0;
	do {
		WCHAR fullPath[MAX_PATH];

		if ( wcscmp(ffd.cFileName, L".") == 0 || wcscmp(ffd.cFileName, L"..") == 0 ) {
			continue;
		}

		_snwprintf_s(fullPath, _countof(fullPath), _TRUNCATE, L"%s\\%s", folderPath, ffd.cFileName);
		if ( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
			count += GuiCountFolderFiles(fullPath);
		} else {
			count++;
		}
	} while ( FindNextFileW(hFind, &ffd) );

	FindClose(hFind);
	return count;
}

static DWORD GuiCountInputFiles(int count, WCHAR** items)
{
	DWORD total;
	int i;

	total = 0;
	for ( i = 0; i < count; ++i ) {
		DWORD attr;

		attr = GetFileAttributesW(items[i]);
		if ( attr == INVALID_FILE_ATTRIBUTES ) {
			continue;
		}
		if ( attr & FILE_ATTRIBUTE_DIRECTORY ) {
			total += GuiCountFolderFiles(items[i]);
		} else {
			total++;
		}
	}
	return total;
}

static WCHAR** GuiDuplicateInputList(int count, WCHAR** items)
{
	WCHAR** copy;
	int i;

	copy = (WCHAR**)calloc((size_t)count, sizeof(WCHAR*));
	if ( copy == NULL ) {
		return NULL;
	}

	for ( i = 0; i < count; ++i ) {
		copy[i] = _wcsdup(items[i]);
		if ( copy[i] == NULL ) {
			while ( i-- > 0 ) {
				free(copy[i]);
			}
			free(copy);
			return NULL;
		}
	}
	return copy;
}

static void GuiFreeInputList(int count, WCHAR** items)
{
	int i;

	if ( items == NULL ) {
		return;
	}
	for ( i = 0; i < count; ++i ) {
		free(items[i]);
	}
	free(items);
}

static BOOL GuiAppendSourceIndexUnique(size_t** indexes, size_t* count, size_t* capacity, size_t sourceIndex)
{
	size_t i;
	size_t newCapacity;
	size_t* newIndexes;

	for ( i = 0; i < *count; ++i ) {
		if ( (*indexes)[i] == sourceIndex ) {
			return TRUE;
		}
	}

	if ( *count >= *capacity ) {
		newCapacity = (*capacity == 0) ? 32 : (*capacity * 2);
		newIndexes = (size_t*)realloc(*indexes, newCapacity * sizeof(size_t));
		if ( newIndexes == NULL ) {
			return FALSE;
		}
		*indexes = newIndexes;
		*capacity = newCapacity;
	}

	(*indexes)[(*count)++] = sourceIndex;
	return TRUE;
}

static BOOL GuiCollectSelectedSourceIndexes(GuiApp* app, size_t** indexesOut, size_t* countOut)
{
	int index;
	size_t* indexes;
	size_t count;
	size_t capacity;

	*indexesOut = NULL;
	*countOut = 0;
	if ( app == NULL || app->list == NULL ) {
		return TRUE;
	}

	indexes = NULL;
	count = 0;
	capacity = 0;
	index = -1;
	while ( TRUE ) {
		GuiViewItem* viewItem;

		index = ListView_GetNextItem(app->list, index, LVNI_SELECTED);
		if ( index < 0 ) {
			break;
		}
		viewItem = GuiArchiveGetViewItemByListIndex(app, index);
		if ( viewItem == NULL ) {
			continue;
		}
		if ( viewItem->kind == GUI_VIEW_ITEM_FILE ) {
			if ( !GuiAppendSourceIndexUnique(&indexes, &count, &capacity, viewItem->sourceIndex) ) {
				free(indexes);
				return FALSE;
			}
		} else if ( viewItem->kind == GUI_VIEW_ITEM_DIR && GuiIsPathPackType(app->packType) ) {
			size_t i;

			for ( i = 0; i < app->itemCount; ++i ) {
				if ( _wcsicmp(app->items[i].name, viewItem->fullPath) == 0 || GuiPathMatchFolderPrefix(app->items[i].name, viewItem->fullPath, NULL) ) {
					if ( !GuiAppendSourceIndexUnique(&indexes, &count, &capacity, i) ) {
						free(indexes);
						return FALSE;
					}
				}
			}
		}
	}

	*indexesOut = indexes;
	*countOut = count;
	return TRUE;
}

BOOL GuiArchiveGetSelectionSummary(GuiApp* app, uint32_t* outFileCount, uint64_t* outSize, uint64_t* outPackedSize)
{
	size_t* sourceIndexes;
	size_t count;
	size_t i;
	uint64_t totalSize;
	uint64_t totalPacked;

	if ( outFileCount != NULL ) {
		*outFileCount = 0;
	}
	if ( outSize != NULL ) {
		*outSize = 0;
	}
	if ( outPackedSize != NULL ) {
		*outPackedSize = 0;
	}
	if ( app == NULL ) {
		return FALSE;
	}
	if ( !GuiCollectSelectedSourceIndexes(app, &sourceIndexes, &count) ) {
		return FALSE;
	}

	totalSize = 0;
	totalPacked = 0;
	for ( i = 0; i < count; ++i ) {
		totalSize += app->items[sourceIndexes[i]].fileSize;
		totalPacked += app->items[sourceIndexes[i]].packedSize;
	}
	free(sourceIndexes);

	if ( outFileCount != NULL ) {
		*outFileCount = (uint32_t)count;
	}
	if ( outSize != NULL ) {
		*outSize = totalSize;
	}
	if ( outPackedSize != NULL ) {
		*outPackedSize = totalPacked;
	}
	return TRUE;
}

static BOOL GuiAppendWideText(WCHAR** textBuf, size_t* len, size_t* cap, const WCHAR* text)
{
	size_t addLen;
	size_t needCap;
	size_t newCap;
	WCHAR* newBuf;

	if ( text == NULL || textBuf == NULL || len == NULL || cap == NULL ) {
		return FALSE;
	}

	addLen = wcslen(text);
	needCap = *len + addLen + 1;
	if ( needCap > *cap ) {
		newCap = (*cap == 0) ? 256 : *cap;
		while ( newCap < needCap ) {
			newCap *= 2;
		}
		newBuf = (WCHAR*)realloc(*textBuf, newCap * sizeof(WCHAR));
		if ( newBuf == NULL ) {
			return FALSE;
		}
		*textBuf = newBuf;
		*cap = newCap;
	}

	memcpy(*textBuf + *len, text, addLen * sizeof(WCHAR));
	*len += addLen;
	(*textBuf)[*len] = L'\0';
	return TRUE;
}

static BOOL GuiAppendTsvCell(WCHAR** textBuf, size_t* len, size_t* cap, const WCHAR* text)
{
	const WCHAR* p;
	WCHAR ch[2];

	if ( text == NULL ) {
		return TRUE;
	}

	ch[1] = L'\0';
	for ( p = text; *p != L'\0'; ++p ) {
		ch[0] = (*p == L'\t' || *p == L'\r' || *p == L'\n') ? L' ' : *p;
		if ( !GuiAppendWideText(textBuf, len, cap, ch) ) {
			return FALSE;
		}
	}
	return TRUE;
}

static BOOL GuiAppendTsvSeparator(WCHAR** textBuf, size_t* len, size_t* cap)
{
	return GuiAppendWideText(textBuf, len, cap, L"\t");
}

static BOOL GuiAppendViewItemDetailsTsvRow(GuiApp* app, WCHAR** textBuf, size_t* len, size_t* cap, const GuiViewItem* item)
{
	int logicalColumn;
	BOOL appended;

	if ( item == NULL || !GuiArchiveCanCopyViewItem(item) ) {
		return TRUE;
	}

	appended = FALSE;
	for ( logicalColumn = 0; logicalColumn < GUI_ARCHIVE_COLUMN_COUNT; ++logicalColumn ) {
		if ( !GuiIsArchiveColumnVisible(app, logicalColumn) ) {
			continue;
		}
		if ( appended && !GuiAppendTsvSeparator(textBuf, len, cap) ) {
			return FALSE;
		}
		if ( !GuiAppendTsvCell(textBuf, len, cap, GuiArchiveGetViewItemColumnText(item, logicalColumn)) ) {
			return FALSE;
		}
		appended = TRUE;
	}
	return TRUE;
}

static BOOL GuiArchiveBuildDetailsTsv(GuiApp* app, BOOL selectedOnly, WCHAR** outText)
{
	WCHAR* textBuf;
	size_t textLen;
	size_t textCap;
	BOOL copied;
	BOOL ok;
	int index;
	size_t i;

	if ( outText == NULL ) {
		return FALSE;
	}
	*outText = NULL;
	if ( app == NULL || app->archive == NULL || app->list == NULL ) {
		return FALSE;
	}

	textBuf = NULL;
	textLen = 0;
	textCap = 0;
	copied = FALSE;
	ok = TRUE;
	for ( i = 0; i < GUI_ARCHIVE_COLUMN_COUNT; ++i ) {
		if ( !GuiIsArchiveColumnVisible(app, (int)i) ) {
			continue;
		}
		if ( textLen > 0 && !GuiAppendTsvSeparator(&textBuf, &textLen, &textCap) ) {
			ok = FALSE;
			break;
		}
		if ( !GuiAppendTsvCell(&textBuf, &textLen, &textCap, arrGuiArchiveColumnNames[i]) ) {
			ok = FALSE;
			break;
		}
	}
	if ( ok ) {
		if ( selectedOnly ) {
			index = -1;
			while ( TRUE ) {
				GuiViewItem* viewItem;

				index = ListView_GetNextItem(app->list, index, LVNI_SELECTED);
				if ( index < 0 ) {
					break;
				}
				viewItem = GuiArchiveGetViewItemByListIndex(app, index);
				if ( !GuiArchiveCanCopyViewItem(viewItem) ) {
					continue;
				}
				if ( !GuiAppendWideText(&textBuf, &textLen, &textCap, L"\r\n") || !GuiAppendViewItemDetailsTsvRow(app, &textBuf, &textLen, &textCap, viewItem) ) {
					ok = FALSE;
					break;
				}
				copied = TRUE;
			}
		} else {
			for ( i = 0; i < app->viewCount; ++i ) {
				if ( !GuiArchiveCanCopyViewItem(&app->viewItems[i]) ) {
					continue;
				}
				if ( !GuiAppendWideText(&textBuf, &textLen, &textCap, L"\r\n") || !GuiAppendViewItemDetailsTsvRow(app, &textBuf, &textLen, &textCap, &app->viewItems[i]) ) {
					ok = FALSE;
					break;
				}
				copied = TRUE;
			}
		}
	}

	if ( !ok ) {
		free(textBuf);
		MessageBoxW(app->window, L"内存不足。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}
	if ( !copied ) {
		free(textBuf);
		MessageBoxW(app->window, selectedOnly ? L"请选择文件或目录复制详细信息。" : L"当前视图没有可复制的文件或目录。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}
	*outText = textBuf;
	return TRUE;
}

static BOOL GuiArchiveCopyDetailsCore(GuiApp* app, BOOL selectedOnly)
{
	WCHAR* textBuf;

	textBuf = NULL;
	if ( !GuiArchiveBuildDetailsTsv(app, selectedOnly, &textBuf) ) {
		return FALSE;
	}
	if ( !GuiSetClipboardText(app->window, textBuf) ) {
		DWORD err;

		err = GetLastError();
		free(textBuf);
		if ( err == 0 ) {
			err = ERROR_GEN_FAILURE;
		}
		GuiShowSystemError(app->window, L"复制详细信息到剪贴板失败", err);
		return FALSE;
	}

	free(textBuf);
	return TRUE;
}

static BOOL GuiSaveTsvFileDialog(HWND hwnd, WCHAR* pathBuf, DWORD cchBuf)
{
	OPENFILENAMEW ofn;

	if ( pathBuf == NULL || cchBuf == 0 ) {
		return FALSE;
	}

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = L"Tab-separated Values (*.tsv)\0*.tsv\0Text File (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = pathBuf;
	ofn.nMaxFile = cchBuf;
	ofn.lpstrDefExt = L"tsv";
	ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
	return GetSaveFileNameW(&ofn);
}

static BOOL GuiSaveTextFileDialog(HWND hwnd, WCHAR* pathBuf, DWORD cchBuf)
{
	OPENFILENAMEW ofn;

	if ( pathBuf == NULL || cchBuf == 0 ) {
		return FALSE;
	}

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = L"Text File (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = pathBuf;
	ofn.nMaxFile = cchBuf;
	ofn.lpstrDefExt = L"txt";
	ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
	return GetSaveFileNameW(&ofn);
}

static BOOL GuiWriteUtf8BomTextFile(HWND owner, const WCHAR* path, const WCHAR* text)
{
	size_t cchText;
	int byteCount;
	DWORD totalBytes;
	BYTE* bytes;
	BOOL ok;

	if ( path == NULL || path[0] == L'\0' || text == NULL ) {
		return FALSE;
	}

	cchText = wcslen(text);
	if ( cchText > 0x7FFFFFFFu ) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		GuiShowSystemError(owner, L"写出 TSV 失败", GetLastError());
		return FALSE;
	}

	byteCount = WideCharToMultiByte(CP_UTF8, 0, text, (int)cchText, NULL, 0, NULL, NULL);
	if ( byteCount < 0 || byteCount > 0x7FFFFFFC ) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		GuiShowSystemError(owner, L"写出 TSV 失败", GetLastError());
		return FALSE;
	}

	totalBytes = (DWORD)byteCount + 3;
	bytes = (BYTE*)malloc(totalBytes > 0 ? totalBytes : 3);
	if ( bytes == NULL ) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		GuiShowSystemError(owner, L"写出 TSV 失败", GetLastError());
		return FALSE;
	}

	bytes[0] = 0xEFu;
	bytes[1] = 0xBBu;
	bytes[2] = 0xBFu;
	if ( byteCount > 0 && WideCharToMultiByte(CP_UTF8, 0, text, (int)cchText, (char*)bytes + 3, byteCount, NULL, NULL) != byteCount ) {
		DWORD err;

		err = GetLastError();
		free(bytes);
		GuiShowSystemError(owner, L"写出 TSV 失败", err != 0 ? err : ERROR_GEN_FAILURE);
		return FALSE;
	}

	ok = GuiWriteWholeFile(path, bytes, totalBytes);
	if ( !ok ) {
		DWORD err;

		err = GetLastError();
		free(bytes);
		GuiShowSystemError(owner, L"写出 TSV 失败", err != 0 ? err : ERROR_GEN_FAILURE);
		return FALSE;
	}

	free(bytes);
	return TRUE;
}

static BOOL GuiArchiveExportDetailsCore(GuiApp* app, BOOL selectedOnly)
{
	WCHAR* textBuf;
	WCHAR pathBuf[MAX_PATH];
	BOOL ok;

	if ( app == NULL || app->archive == NULL ) {
		return FALSE;
	}

	textBuf = NULL;
	if ( !GuiArchiveBuildDetailsTsv(app, selectedOnly, &textBuf) ) {
		return FALSE;
	}

	wcsncpy_s(pathBuf, _countof(pathBuf), app->archivePath, _TRUNCATE);
	if ( pathBuf[0] == L'\0' ) {
		wcsncpy_s(pathBuf, _countof(pathBuf), L"xpack_list", _TRUNCATE);
	}
	PathRemoveExtensionW(pathBuf);
	wcscat_s(pathBuf, _countof(pathBuf), selectedOnly ? L"_selected.tsv" : L"_visible.tsv");

	if ( !GuiSaveTsvFileDialog(app->window, pathBuf, _countof(pathBuf)) ) {
		free(textBuf);
		return FALSE;
	}

	ok = GuiWriteUtf8BomTextFile(app->window, pathBuf, textBuf);
	free(textBuf);
	if ( ok ) {
		MessageBoxW(app->window, L"导出完成。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
	}
	return ok;
}

static BOOL GuiArchiveBuildPathsText(GuiApp* app, BOOL selectedOnly, WCHAR** outText)
{
	WCHAR* textBuf;
	size_t textLen;
	size_t textCap;
	int index;
	size_t i;
	BOOL pathPack;
	BOOL copied;
	BOOL ok;

	if ( outText == NULL ) {
		return FALSE;
	}
	*outText = NULL;
	if ( app == NULL || app->archive == NULL || app->list == NULL ) {
		return FALSE;
	}

	textBuf = NULL;
	textLen = 0;
	textCap = 0;
	index = -1;
	pathPack = GuiIsPathPackType(app->packType);
	copied = FALSE;
	ok = TRUE;

	while ( ok && selectedOnly ) {
		GuiViewItem* viewItem;
		const WCHAR* lineText;

		index = ListView_GetNextItem(app->list, index, LVNI_SELECTED);
		if ( index < 0 ) {
			break;
		}
		viewItem = GuiArchiveGetViewItemByListIndex(app, index);
		if ( !GuiArchiveCanCopyViewItem(viewItem) ) {
			continue;
		}

		lineText = pathPack ? viewItem->fullPath : viewItem->name;
		if ( lineText == NULL || lineText[0] == L'\0' ) {
			continue;
		}
		if ( copied && !GuiAppendWideText(&textBuf, &textLen, &textCap, L"\r\n") ) {
			ok = FALSE;
			break;
		}
		if ( !GuiAppendWideText(&textBuf, &textLen, &textCap, lineText) ) {
			ok = FALSE;
			break;
		}
		copied = TRUE;
	}
	for ( i = 0; ok && !selectedOnly && i < app->viewCount; ++i ) {
		GuiViewItem* viewItem;
		const WCHAR* lineText;

		viewItem = &app->viewItems[i];
		if ( !GuiArchiveCanCopyViewItem(viewItem) ) {
			continue;
		}

		lineText = pathPack ? viewItem->fullPath : viewItem->name;
		if ( lineText == NULL || lineText[0] == L'\0' ) {
			continue;
		}
		if ( copied && !GuiAppendWideText(&textBuf, &textLen, &textCap, L"\r\n") ) {
			ok = FALSE;
			break;
		}
		if ( !GuiAppendWideText(&textBuf, &textLen, &textCap, lineText) ) {
			ok = FALSE;
			break;
		}
		copied = TRUE;
	}

	if ( !ok ) {
		free(textBuf);
		MessageBoxW(app->window, L"内存不足。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}
	if ( !copied ) {
		free(textBuf);
		MessageBoxW(app->window, selectedOnly ? L"请选择文件或目录进行复制。" : L"当前视图没有可复制的文件或目录。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}
	if ( textBuf == NULL ) {
		MessageBoxW(app->window, L"内存不足。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}
	*outText = textBuf;
	return TRUE;
}

static BOOL GuiArchiveCopyPathsCore(GuiApp* app, BOOL selectedOnly)
{
	WCHAR* textBuf;

	textBuf = NULL;
	if ( !GuiArchiveBuildPathsText(app, selectedOnly, &textBuf) ) {
		return FALSE;
	}
	if ( !GuiSetClipboardText(app->window, textBuf) ) {
		DWORD err;

		err = GetLastError();
		free(textBuf);
		if ( err == 0 ) {
			err = ERROR_GEN_FAILURE;
		}
		GuiShowSystemError(app->window, L"复制到剪贴板失败", err);
		return FALSE;
	}

	free(textBuf);
	return TRUE;
}

static BOOL GuiArchiveExportPathsCore(GuiApp* app, BOOL selectedOnly)
{
	WCHAR* textBuf;
	WCHAR pathBuf[MAX_PATH];
	BOOL ok;

	if ( app == NULL || app->archive == NULL ) {
		return FALSE;
	}

	textBuf = NULL;
	if ( !GuiArchiveBuildPathsText(app, selectedOnly, &textBuf) ) {
		return FALSE;
	}

	wcsncpy_s(pathBuf, _countof(pathBuf), app->archivePath, _TRUNCATE);
	if ( pathBuf[0] == L'\0' ) {
		wcsncpy_s(pathBuf, _countof(pathBuf), L"xpack_paths", _TRUNCATE);
	}
	PathRemoveExtensionW(pathBuf);
	wcscat_s(pathBuf, _countof(pathBuf), selectedOnly ? L"_selected_paths.txt" : L"_visible_paths.txt");

	if ( !GuiSaveTextFileDialog(app->window, pathBuf, _countof(pathBuf)) ) {
		free(textBuf);
		return FALSE;
	}

	ok = GuiWriteUtf8BomTextFile(app->window, pathBuf, textBuf);
	free(textBuf);
	if ( ok ) {
		MessageBoxW(app->window, L"导出完成。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
	}
	return ok;
}

BOOL GuiArchiveCopySelectionPaths(GuiApp* app)
{
	return GuiArchiveCopyPathsCore(app, TRUE);
}

BOOL GuiArchiveCopyVisiblePaths(GuiApp* app)
{
	return GuiArchiveCopyPathsCore(app, FALSE);
}

BOOL GuiArchiveExportSelectionPaths(GuiApp* app)
{
	return GuiArchiveExportPathsCore(app, TRUE);
}

BOOL GuiArchiveExportVisiblePaths(GuiApp* app)
{
	return GuiArchiveExportPathsCore(app, FALSE);
}

BOOL GuiArchiveCopySelectionDetails(GuiApp* app)
{
	return GuiArchiveCopyDetailsCore(app, TRUE);
}

BOOL GuiArchiveCopyVisibleDetails(GuiApp* app)
{
	return GuiArchiveCopyDetailsCore(app, FALSE);
}

BOOL GuiArchiveExportSelectionDetails(GuiApp* app)
{
	return GuiArchiveExportDetailsCore(app, TRUE);
}

BOOL GuiArchiveExportVisibleDetails(GuiApp* app)
{
	return GuiArchiveExportDetailsCore(app, FALSE);
}

BOOL GuiArchiveCopySelectionHash(GuiApp* app)
{
	GuiArchiveItem* item;
	WCHAR text[32];

	if ( !GuiArchiveCanCopySelectionHash(app) ) {
		MessageBoxW(app != NULL ? app->window : NULL, L"请选择单个文件复制哈希。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	item = GuiArchiveGetSingleSelectedItem(app);
	if ( item == NULL ) {
		return FALSE;
	}

	_snwprintf_s(text, _countof(text), _TRUNCATE, L"0x%08X", item->fileHash);
	if ( !GuiSetClipboardText(app->window, text) ) {
		DWORD err;

		err = GetLastError();
		if ( err == 0 ) {
			err = ERROR_GEN_FAILURE;
		}
		GuiShowSystemError(app->window, L"复制哈希到剪贴板失败", err);
		return FALSE;
	}
	return TRUE;
}

static BOOL GuiAppendViewItemHashTsvRow(GuiApp* app, WCHAR** textBuf, size_t* textLen, size_t* textCap, const GuiViewItem* viewItem)
{
	const GuiArchiveItem* item;
	const WCHAR* pathText;
	WCHAR hashText[32];

	if ( app == NULL || viewItem == NULL || viewItem->kind != GUI_VIEW_ITEM_FILE || viewItem->sourceIndex >= app->itemCount ) {
		return TRUE;
	}

	item = &app->items[viewItem->sourceIndex];
	pathText = GuiIsPathPackType(app->packType) ? viewItem->fullPath : viewItem->name;
	_snwprintf_s(hashText, _countof(hashText), _TRUNCATE, L"0x%08X", item->fileHash);
	if ( !GuiAppendTsvCell(textBuf, textLen, textCap, hashText) ) {
		return FALSE;
	}
	if ( !GuiAppendTsvSeparator(textBuf, textLen, textCap) ) {
		return FALSE;
	}
	return GuiAppendTsvCell(textBuf, textLen, textCap, pathText);
}

static BOOL GuiArchiveBuildHashesTsv(GuiApp* app, BOOL selectedOnly, WCHAR** outText)
{
	WCHAR* textBuf;
	size_t textLen;
	size_t textCap;
	int index;
	size_t i;
	BOOL copied;
	BOOL ok;

	if ( outText == NULL ) {
		return FALSE;
	}
	*outText = NULL;
	if ( selectedOnly ? !GuiArchiveCanCopySelectionHashes(app) : !GuiArchiveCanCopyVisibleHashes(app) ) {
		MessageBoxW(app != NULL ? app->window : NULL, selectedOnly ? L"请选择文件复制哈希。" : L"当前视图没有可复制哈希的文件。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	textBuf = NULL;
	textLen = 0;
	textCap = 0;
	index = -1;
	copied = FALSE;
	ok = GuiAppendWideText(&textBuf, &textLen, &textCap, L"Hash\tPath");

	while ( ok && selectedOnly ) {
		GuiViewItem* viewItem;

		index = ListView_GetNextItem(app->list, index, LVNI_SELECTED);
		if ( index < 0 ) {
			break;
		}
		viewItem = GuiArchiveGetViewItemByListIndex(app, index);
		if ( viewItem == NULL || viewItem->kind != GUI_VIEW_ITEM_FILE || viewItem->sourceIndex >= app->itemCount ) {
			continue;
		}

		if ( !GuiAppendWideText(&textBuf, &textLen, &textCap, L"\r\n") || !GuiAppendViewItemHashTsvRow(app, &textBuf, &textLen, &textCap, viewItem) ) {
			ok = FALSE;
			break;
		}
		copied = TRUE;
	}
	for ( i = 0; ok && !selectedOnly && i < app->viewCount; ++i ) {
		GuiViewItem* viewItem;

		viewItem = &app->viewItems[i];
		if ( viewItem->kind != GUI_VIEW_ITEM_FILE || viewItem->sourceIndex >= app->itemCount ) {
			continue;
		}

		if ( !GuiAppendWideText(&textBuf, &textLen, &textCap, L"\r\n") || !GuiAppendViewItemHashTsvRow(app, &textBuf, &textLen, &textCap, viewItem) ) {
			ok = FALSE;
			break;
		}
		copied = TRUE;
	}

	if ( !ok ) {
		free(textBuf);
		MessageBoxW(app->window, L"内存不足。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}
	if ( !copied ) {
		free(textBuf);
		MessageBoxW(app->window, selectedOnly ? L"请选择文件复制哈希。" : L"当前视图没有可复制哈希的文件。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}
	*outText = textBuf;
	return TRUE;
}

static BOOL GuiArchiveCopyHashesCore(GuiApp* app, BOOL selectedOnly)
{
	WCHAR* textBuf;

	textBuf = NULL;
	if ( !GuiArchiveBuildHashesTsv(app, selectedOnly, &textBuf) ) {
		return FALSE;
	}
	if ( !GuiSetClipboardText(app->window, textBuf) ) {
		DWORD err;

		err = GetLastError();
		free(textBuf);
		if ( err == 0 ) {
			err = ERROR_GEN_FAILURE;
		}
		GuiShowSystemError(app->window, L"复制哈希到剪贴板失败", err);
		return FALSE;
	}

	free(textBuf);
	return TRUE;
}

static BOOL GuiArchiveExportHashesCore(GuiApp* app, BOOL selectedOnly)
{
	WCHAR* textBuf;
	WCHAR pathBuf[MAX_PATH];
	BOOL ok;

	if ( app == NULL || app->archive == NULL ) {
		return FALSE;
	}

	textBuf = NULL;
	if ( !GuiArchiveBuildHashesTsv(app, selectedOnly, &textBuf) ) {
		return FALSE;
	}

	wcsncpy_s(pathBuf, _countof(pathBuf), app->archivePath, _TRUNCATE);
	if ( pathBuf[0] == L'\0' ) {
		wcsncpy_s(pathBuf, _countof(pathBuf), L"xpack_hashes", _TRUNCATE);
	}
	PathRemoveExtensionW(pathBuf);
	wcscat_s(pathBuf, _countof(pathBuf), selectedOnly ? L"_selected_hashes.tsv" : L"_visible_hashes.tsv");

	if ( !GuiSaveTsvFileDialog(app->window, pathBuf, _countof(pathBuf)) ) {
		free(textBuf);
		return FALSE;
	}

	ok = GuiWriteUtf8BomTextFile(app->window, pathBuf, textBuf);
	free(textBuf);
	if ( ok ) {
		MessageBoxW(app->window, L"导出完成。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
	}
	return ok;
}

BOOL GuiArchiveCopySelectionHashes(GuiApp* app)
{
	return GuiArchiveCopyHashesCore(app, TRUE);
}

BOOL GuiArchiveCopyVisibleHashes(GuiApp* app)
{
	return GuiArchiveCopyHashesCore(app, FALSE);
}

BOOL GuiArchiveExportSelectionHashes(GuiApp* app)
{
	return GuiArchiveExportHashesCore(app, TRUE);
}

BOOL GuiArchiveExportVisibleHashes(GuiApp* app)
{
	return GuiArchiveExportHashesCore(app, FALSE);
}

BOOL GuiArchiveSetSelectionFileType(GuiApp* app)
{
	size_t* sourceIndexes;
	size_t count;
	size_t i;
	BOOL mixedType;
	uint8_t currentType;
	uint8_t newType;
	GuiInputDialogState dialogState;
	WCHAR prompt[512];
	WCHAR currentText[64];

	if ( !GuiArchiveCanSetSelectionFileType(app) ) {
		MessageBoxW(app != NULL ? app->window : NULL, L"请选择文件或目录设置 File Type。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}
	if ( !GuiCollectSelectedSourceIndexes(app, &sourceIndexes, &count) ) {
		return FALSE;
	}
	if ( sourceIndexes == NULL || count == 0 ) {
		free(sourceIndexes);
		MessageBoxW(app->window, L"请选择文件或目录设置 File Type。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	mixedType = FALSE;
	currentType = GuiEntryFileType(app->items[sourceIndexes[0]].flag);
	for ( i = 1; i < count; ++i ) {
		if ( GuiEntryFileType(app->items[sourceIndexes[i]].flag) != currentType ) {
			mixedType = TRUE;
			break;
		}
	}

	ZeroMemory(&dialogState, sizeof(dialogState));
	dialogState.title = L"设置 File Type";
	_snwprintf_s(
		prompt,
		_countof(prompt),
		_TRUNCATE,
		L"将更新 %u 个底层文件的 File Type。\r\n可输入名称或 0-15 数字：Unknown(0), Binary(1), Text(2), Image(3), Audio(4), Video(5), Archive(6), Folder(15)。",
		(unsigned)count);
	dialogState.prompt = prompt;
	if ( !mixedType ) {
		GuiFormatFileType(currentType, currentText, _countof(currentText));
		wcsncpy_s(dialogState.value, _countof(dialogState.value), currentText, _TRUNCATE);
	}

	for ( ;; ) {
		uint32_t flagValue;

		if ( !GuiRunInputDialog(app->window, &dialogState) ) {
			free(sourceIndexes);
			return FALSE;
		}
		if ( !GuiParseFileTypeInput(dialogState.value, &newType) ) {
			MessageBoxW(app->window, L"请输入有效的 File Type。可用名称：Unknown/Binary/Text/Image/Audio/Video/Archive/Folder，或输入 0-15。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
			continue;
		}

		flagValue = ((uint32_t)newType & 0x0Fu) << 4;
		for ( i = 0; i < count; ++i ) {
			if ( xpkSetFlag(app->archive, app->items[sourceIndexes[i]].pos, XPK_FLAG_TYPE_MASK, flagValue) != XPK_OK ) {
				free(sourceIndexes);
				GuiShowArchiveError(app, L"设置 File Type");
				return FALSE;
			}
		}
		free(sourceIndexes);
		return GuiArchiveSave(app);
	}
}

BOOL GuiArchiveSetSelectionPathAttr(GuiApp* app)
{
	size_t* sourceIndexes;
	size_t count;
	size_t i;
	BOOL mixedAttr;
	uint32_t currentAttr;
	uint32_t newAttr;
	GuiInputDialogState dialogState;
	WCHAR prompt[256];

	if ( !GuiArchiveCanSetSelectionPathAttr(app) ) {
		MessageBoxW(app != NULL ? app->window : NULL, L"请选择 path 包中的文件或目录设置 platformAttr。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}
	if ( !GuiCollectSelectedSourceIndexes(app, &sourceIndexes, &count) ) {
		return FALSE;
	}
	if ( sourceIndexes == NULL || count == 0 ) {
		free(sourceIndexes);
		MessageBoxW(app->window, L"请选择 path 包中的文件或目录设置 platformAttr。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	mixedAttr = FALSE;
	currentAttr = app->items[sourceIndexes[0]].attr;
	for ( i = 1; i < count; ++i ) {
		if ( app->items[sourceIndexes[i]].attr != currentAttr ) {
			mixedAttr = TRUE;
			break;
		}
	}

	ZeroMemory(&dialogState, sizeof(dialogState));
	dialogState.title = L"设置 Platform Attr";
	_snwprintf_s(
		prompt,
		_countof(prompt),
		_TRUNCATE,
		L"将更新 %u 个底层文件的 platformAttr。支持十进制或 0x 前缀十六进制：",
		(unsigned)count);
	dialogState.prompt = prompt;
	if ( !mixedAttr ) {
		_snwprintf_s(dialogState.value, _countof(dialogState.value), _TRUNCATE, L"0x%08X", currentAttr);
	}

	for ( ;; ) {
		if ( !GuiRunInputDialog(app->window, &dialogState) ) {
			free(sourceIndexes);
			return FALSE;
		}
		if ( !GuiParseUInt32InputFlexible(dialogState.value, &newAttr) ) {
			MessageBoxW(app->window, L"请输入有效的 platformAttr。可以使用十进制，或使用 0x 前缀输入十六进制。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
			continue;
		}

		for ( i = 0; i < count; ++i ) {
			char utf8Path[XPK_PATH_BYTES];

			GuiPathToPackageUtf8(app->items[sourceIndexes[i]].name, utf8Path, sizeof(utf8Path));
			if ( xpkPathSetAttr(app->archive, utf8Path, newAttr) != XPK_OK ) {
				free(sourceIndexes);
				GuiShowArchiveError(app, L"设置 platformAttr");
				return FALSE;
			}
		}
		free(sourceIndexes);
		return GuiArchiveSave(app);
	}
}

static GuiArchiveItem* GuiDuplicateExtractItems(GuiApp* app, BOOL allItems, int* outCount)
{
	GuiArchiveItem* items;
	size_t* sourceIndexes;
	size_t count;
	size_t i;

	*outCount = 0;
	if ( allItems ) {
		if ( app->itemCount == 0 ) {
			return NULL;
		}
		items = (GuiArchiveItem*)malloc(app->itemCount * sizeof(GuiArchiveItem));
		if ( items == NULL ) {
			return NULL;
		}
		memcpy(items, app->items, app->itemCount * sizeof(GuiArchiveItem));
		*outCount = (int)app->itemCount;
		return items;
	}

	if ( !GuiCollectSelectedSourceIndexes(app, &sourceIndexes, &count) ) {
		return NULL;
	}
	if ( count == 0 ) {
		free(sourceIndexes);
		return NULL;
	}

	items = (GuiArchiveItem*)malloc(count * sizeof(GuiArchiveItem));
	if ( items == NULL ) {
		free(sourceIndexes);
		return NULL;
	}

	for ( i = 0; i < count; ++i ) {
		memcpy(&items[i], &app->items[sourceIndexes[i]], sizeof(GuiArchiveItem));
	}
	free(sourceIndexes);

	*outCount = (int)i;
	return items;
}

static BOOL GuiApplyArchiveOptionsRaw(xpkObject archive, const GuiArchiveOptions* options, GuiTaskState* task)
{
	if ( xpkSetPackType(archive, options->packType) != XPK_OK ) {
		GuiTaskSetArchiveError(task, archive, L"设置 Pack Type");
		return FALSE;
	}
	if ( xpkSetDefaultComp(archive, options->defaultComp) != XPK_OK ) {
		GuiTaskSetArchiveError(task, archive, L"设置默认压缩级别");
		return FALSE;
	}
	if ( xpkSetMetaComp(archive, options->metaComp) != XPK_OK ) {
		GuiTaskSetArchiveError(task, archive, L"设置元数据压缩级别");
		return FALSE;
	}
	if ( xpkSetInfoComp(archive, options->infoComp) != XPK_OK ) {
		GuiTaskSetArchiveError(task, archive, L"设置信息区压缩级别");
		return FALSE;
	}
	if ( options->packType == XPK_PACK_CORE ) {
		if ( xpkSetInfoExtSize(archive, options->infoExtSize) != XPK_OK ) {
			GuiTaskSetArchiveError(task, archive, L"设置 InfoExt Size");
			return FALSE;
		}
	}
	if ( xpkSetVolumeSize(archive, options->volumeSize) != XPK_OK ) {
		GuiTaskSetArchiveError(task, archive, L"设置分卷大小");
		return FALSE;
	}
	if ( xpkSetSolidMode(archive, options->solidMode ? 1 : 0) != XPK_OK ) {
		GuiTaskSetArchiveError(task, archive, L"设置 Solid Mode");
		return FALSE;
	}
	return TRUE;
}

typedef struct GuiIndexSeedState {
	int64_t maxIndex;
} GuiIndexSeedState;

static int GuiFindNextIndexSeedCallback(xpkObject xpk, uint32_t pos, const void* info, void* userData)
{
	const xpkFileInfoIndex* pInfo;
	GuiIndexSeedState* state;

	(void)xpk;
	(void)pos;

	pInfo = (const xpkFileInfoIndex*)info;
	state = (GuiIndexSeedState*)userData;
	if ( pInfo->fileIndex > state->maxIndex ) {
		state->maxIndex = pInfo->fileIndex;
	}
	return XPK_OK;
}

static int64_t GuiFindNextIndexSeedObject(xpkObject archive, xpkPackType packType, GuiTaskState* task)
{
	GuiIndexSeedState state;

	if ( packType != XPK_PACK_INDEX ) {
		return 1;
	}

	state.maxIndex = 0;
	if ( xpkEach(archive, GuiFindNextIndexSeedCallback, &state) != XPK_OK ) {
		if ( task != NULL ) {
			GuiTaskSetArchiveError(task, archive, L"扫描 index 条目");
		}
		return -1;
	}
	return state.maxIndex + 1;
}

static BOOL GuiTrySuggestIndexSeedFromArchivePath(const WCHAR* archivePath, int64_t* outSeed)
{
	xpkOpenOptions openOpt;
	xpkObject archive;
	char utf8Path[MAX_PATH * 4];
	int64_t seed;

	if ( outSeed == NULL || archivePath == NULL || archivePath[0] == L'\0' ) {
		return FALSE;
	}
	if ( GetFileAttributesW(archivePath) == INVALID_FILE_ATTRIBUTES ) {
		return FALSE;
	}
	if ( !GuiUtf8FromWide(archivePath, utf8Path, sizeof(utf8Path)) ) {
		return FALSE;
	}

	ZeroMemory(&openOpt, sizeof(openOpt));
	openOpt.readonly = 1;
	archive = xpkOpen(utf8Path, &openOpt);
	if ( archive == NULL ) {
		return FALSE;
	}

	seed = GuiFindNextIndexSeedObject(archive, XPK_PACK_INDEX, NULL);
	xpkClose(archive);
	if ( seed < 0 ) {
		return FALSE;
	}

	*outSeed = seed;
	return TRUE;
}

static BOOL GuiPromptIndexAddOptions(HWND owner, const WCHAR* title, int64_t defaultIndex, int64_t* outStartIndex)
{
	GuiInputDialogState dialogState;
	int64_t startIndex;

	if ( outStartIndex == NULL ) {
		return FALSE;
	}

	ZeroMemory(&dialogState, sizeof(dialogState));
	dialogState.title = title;
	dialogState.prompt = L"起始 fileIndex:";
	_snwprintf_s(dialogState.value, _countof(dialogState.value), _TRUNCATE, L"%lld", (long long)defaultIndex);
	for ( ;; ) {
		if ( !GuiRunInputDialog(owner, &dialogState) ) {
			return FALSE;
		}
		if ( GuiParseInt64Input(dialogState.value, &startIndex) ) {
			break;
		}
		MessageBoxW(owner, L"请输入有效的 64 位整数 fileIndex。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
	}

	*outStartIndex = startIndex;
	return TRUE;
}

static BOOL GuiPromptPathAddOptions(HWND owner, const WCHAR* title, const WCHAR* defaultRoot, WCHAR* outPackageRoot, size_t cchPackageRoot)
{
	GuiInputDialogState dialogState;

	if ( outPackageRoot == NULL || cchPackageRoot == 0 ) {
		return FALSE;
	}

	ZeroMemory(&dialogState, sizeof(dialogState));
	dialogState.title = title;
	dialogState.prompt = L"包内根路径(可留空):";
	wcsncpy_s(dialogState.value, _countof(dialogState.value), defaultRoot != NULL ? defaultRoot : L"", _TRUNCATE);
	for ( ;; ) {
		if ( !GuiRunInputDialog(owner, &dialogState) ) {
			return FALSE;
		}
		if ( GuiNormalizePackageRoot(dialogState.value, outPackageRoot, cchPackageRoot) ) {
			break;
		}
		MessageBoxW(owner, L"请输入有效的包内根路径；不能是绝对路径，也不能包含 . 或 .. 段。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
	}
	return TRUE;
}

static BOOL GuiPrepareAddTaskOptions(GuiApp* app, xpkPackType packType, const WCHAR* archivePath, GuiTaskState* task)
{
	int64_t defaultIndex;
	WCHAR defaultRoot[XPKGUI_ITEM_TEXT];

	if ( task == NULL ) {
		return FALSE;
	}

	task->addStartIndex = 0;
	task->addPackageRoot[0] = L'\0';
	if ( packType == XPK_PACK_INDEX ) {
		defaultIndex = 1;
		if ( app != NULL && app->archive != NULL ) {
			defaultIndex = GuiFindNextIndexSeed(app);
		} else if ( !GuiTrySuggestIndexSeedFromArchivePath(archivePath, &defaultIndex) ) {
			defaultIndex = 1;
		}
		return GuiPromptIndexAddOptions(app != NULL ? app->window : NULL, L"添加到 Index 包", defaultIndex, &task->addStartIndex);
	}
	if ( GuiIsPathPackType(packType) ) {
		defaultRoot[0] = L'\0';
		if ( app != NULL && app->archive != NULL && !app->flatView ) {
			wcsncpy_s(defaultRoot, _countof(defaultRoot), app->currentFolder, _TRUNCATE);
		}
		return GuiPromptPathAddOptions(app != NULL ? app->window : NULL, L"添加到 Path 包", defaultRoot, task->addPackageRoot, _countof(task->addPackageRoot));
	}
	return TRUE;
}

static BOOL GuiTaskShouldCancel(GuiTaskState* task)
{
	return InterlockedCompareExchange(&task->cancelRequested, 0, 0) != 0;
}

static void GuiTaskAdvanceProgress(GuiTaskState* task)
{
	InterlockedIncrement(&task->progressCurrent);
}

static void GuiTaskBuildStatusText(const GuiTaskState* task, WCHAR* buf, size_t cchBuf)
{
	LONG current;
	LONG total;

	if ( GuiTaskShouldCancel((GuiTaskState*)task) ) {
		wcsncpy_s(buf, cchBuf, L"正在取消...", _TRUNCATE);
		return;
	}

	current = InterlockedCompareExchange((LONG*)&task->progressCurrent, 0, 0);
	total = InterlockedCompareExchange((LONG*)&task->progressTotal, 0, 0);
	if ( total <= 0 ) {
		total = 1;
	}

	switch ( task->kind ) {
		case GUI_TASK_ADD:
			_snwprintf_s(buf, cchBuf, _TRUNCATE, L"正在添加文件... %ld / %ld", current, total);
			break;
		case GUI_TASK_CREATE_ADD:
			_snwprintf_s(buf, cchBuf, _TRUNCATE, L"正在创建并写入归档... %ld / %ld", current, total);
			break;
		case GUI_TASK_REPLACE:
			wcsncpy_s(buf, cchBuf, L"正在替换条目...", _TRUNCATE);
			break;
		case GUI_TASK_EXTRACT:
			_snwprintf_s(buf, cchBuf, _TRUNCATE, L"正在解压文件... %ld / %ld", current, total);
			break;
		case GUI_TASK_VERIFY:
			if ( task->extractItems != NULL && task->extractCount > 0 && !task->marquee ) {
				_snwprintf_s(buf, cchBuf, _TRUNCATE, L"正在校验条目... %ld / %ld", current, total);
			} else {
				wcsncpy_s(buf, cchBuf, L"正在校验归档完整性...", _TRUNCATE);
			}
			break;
		case GUI_TASK_BUILD:
			wcsncpy_s(buf, cchBuf, L"正在重构归档...", _TRUNCATE);
			break;
		default:
			wcsncpy_s(buf, cchBuf, L"正在处理...", _TRUNCATE);
			break;
	}
}

static void GuiTaskBuildDetailText(const GuiTaskState* task, WCHAR* buf, size_t cchBuf)
{
	WCHAR detail[1024];

	EnterCriticalSection((CRITICAL_SECTION*)&task->detailLock);
	wcsncpy_s(detail, _countof(detail), task->detailText, _TRUNCATE);
	LeaveCriticalSection((CRITICAL_SECTION*)&task->detailLock);

	if ( detail[0] == L'\0' ) {
		buf[0] = L'\0';
		return;
	}

	if ( !PathCompactPathExW(buf, detail, (UINT)cchBuf, 0) ) {
		wcsncpy_s(buf, cchBuf, detail, _TRUNCATE);
	}
}

static BOOL GuiAddFilesystemEntryRaw(xpkObject archive, xpkPackType packType, uint8_t writePolicy, const WCHAR* filePath, const WCHAR* baseDir, int64_t* nextIndex, GuiTaskState* task)
{
	char utf8File[MAX_PATH * 4];
	char utf8Pkg[XPK_PATH_BYTES];
	WCHAR relPath[MAX_PATH];
	WCHAR packagePath[XPKGUI_ITEM_TEXT];
	xpkWriteOptions writeOpt;
	int iRet;

	ZeroMemory(&writeOpt, sizeof(writeOpt));
	writeOpt.compLevel = 0xFFu;
	writeOpt.writePolicy = writePolicy;
	writeOpt.fileType = 0;
	GuiTaskSetDetailText(task, filePath);

	if ( !GuiUtf8FromWide(filePath, utf8File, sizeof(utf8File)) ) {
		GuiTaskSetErrorText(task, L"路径转换失败。");
		return FALSE;
	}

	if ( packType == XPK_PACK_CORE ) {
		iRet = xpkAddFile(archive, utf8File, &writeOpt, NULL);
	} else if ( packType == XPK_PACK_INDEX ) {
		iRet = xpkIndexAddFile(archive, *nextIndex, utf8File, &writeOpt);
		(*nextIndex)++;
	} else {
		GuiPathRelativeToBase(filePath, baseDir, relPath, _countof(relPath));
		GuiBuildPackagePathForAdd(task != NULL ? task->addPackageRoot : NULL, relPath, packagePath, _countof(packagePath));
		GuiPathToPackageUtf8(packagePath, utf8Pkg, sizeof(utf8Pkg));
		iRet = xpkPathAddFile(archive, utf8Pkg, utf8File, &writeOpt);
	}

	if ( iRet != XPK_OK ) {
		GuiTaskSetArchiveError(task, archive, L"添加文件");
		return FALSE;
	}
	return TRUE;
}

static BOOL GuiReplaceArchiveEntryRaw(xpkObject archive, xpkPackType packType, const GuiArchiveItem* item, uint8_t writePolicy, const WCHAR* sourcePath, GuiTaskState* task)
{
	char utf8File[XPKGUI_MAX_TEMP_PATH * 4];
	char utf8Pkg[XPK_PATH_BYTES];
	WCHAR detail[XPKGUI_MAX_TEMP_PATH];
	xpkWriteOptions writeOpt;
	int iRet;

	if ( archive == NULL || item == NULL || sourcePath == NULL || sourcePath[0] == L'\0' ) {
		GuiTaskSetErrorText(task, L"替换任务参数无效。");
		return FALSE;
	}

	ZeroMemory(&writeOpt, sizeof(writeOpt));
	writeOpt.compLevel = 0xFFu;
	writeOpt.writePolicy = writePolicy;
	writeOpt.fileType = GuiEntryFileType(item->flag);

	_snwprintf_s(detail, _countof(detail), _TRUNCATE, L"%s <= %s", item->name, sourcePath);
	GuiTaskSetDetailText(task, detail);

	if ( !GuiUtf8FromWide(sourcePath, utf8File, sizeof(utf8File)) ) {
		GuiTaskSetErrorText(task, L"路径转换失败。");
		return FALSE;
	}

	if ( packType == XPK_PACK_CORE ) {
		iRet = xpkUpdateFile(archive, item->pos, utf8File, &writeOpt);
	} else if ( packType == XPK_PACK_INDEX ) {
		iRet = xpkIndexUpdateFile(archive, item->fileIndex, utf8File, &writeOpt);
	} else {
		if ( item->packagePath[0] != '\0' ) {
			strncpy_s(utf8Pkg, sizeof(utf8Pkg), item->packagePath, _TRUNCATE);
		} else {
			GuiPathToPackageUtf8(item->name, utf8Pkg, sizeof(utf8Pkg));
		}
		iRet = xpkPathUpdateFile(archive, utf8Pkg, utf8File, &writeOpt);
	}

	if ( iRet != XPK_OK ) {
		GuiTaskSetArchiveError(task, archive, L"替换条目");
		return FALSE;
	}
	return TRUE;
}

static BOOL GuiAddFolderRecursiveRaw(xpkObject archive, xpkPackType packType, uint8_t writePolicy, const WCHAR* folderPath, const WCHAR* baseDir, int64_t* nextIndex, GuiTaskState* task)
{
	WCHAR searchPath[MAX_PATH];
	WIN32_FIND_DATAW ffd;
	HANDLE hFind;

	_snwprintf_s(searchPath, _countof(searchPath), _TRUNCATE, L"%s\\*", folderPath);
	hFind = FindFirstFileW(searchPath, &ffd);
	if ( hFind == INVALID_HANDLE_VALUE ) {
		DWORD err = GetLastError();
		if ( err == ERROR_FILE_NOT_FOUND ) {
			return TRUE;
		}
		GuiTaskSetSystemError(task, L"扫描目录", err);
		return FALSE;
	}

	do {
		WCHAR fullPath[MAX_PATH];

		if ( GuiTaskShouldCancel(task) ) {
			FindClose(hFind);
			task->dialogResult = GUI_TASK_RESULT_CANCELLED;
			return FALSE;
		}

		if ( wcscmp(ffd.cFileName, L".") == 0 || wcscmp(ffd.cFileName, L"..") == 0 ) {
			continue;
		}

		_snwprintf_s(fullPath, _countof(fullPath), _TRUNCATE, L"%s\\%s", folderPath, ffd.cFileName);
		if ( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
			if ( !GuiAddFolderRecursiveRaw(archive, packType, writePolicy, fullPath, baseDir, nextIndex, task) ) {
				FindClose(hFind);
				return FALSE;
			}
		} else {
			if ( !GuiAddFilesystemEntryRaw(archive, packType, writePolicy, fullPath, baseDir, nextIndex, task) ) {
				FindClose(hFind);
				return FALSE;
			}
			GuiTaskAdvanceProgress(task);
		}
	} while ( FindNextFileW(hFind, &ffd) );

	FindClose(hFind);
	return TRUE;
}

static BOOL GuiTaskRunAddSelection(GuiTaskState* task)
{
	xpkObject archive;
	int i;
	int64_t nextIndex;

	archive = NULL;
	nextIndex = (task->packType == XPK_PACK_INDEX) ? task->addStartIndex : 0;
	if ( !GuiTaskOpenArchiveWorker(task, FALSE, FALSE, L"打开归档", &archive) ) {
		return FALSE;
	}
	for ( i = 0; i < task->inputCount; ++i ) {
		DWORD attr;
		WCHAR baseDir[MAX_PATH];
		WCHAR* slash;

		if ( GuiTaskShouldCancel(task) ) {
			task->dialogResult = GUI_TASK_RESULT_CANCELLED;
			return FALSE;
		}

		attr = GetFileAttributesW(task->inputs[i]);
		if ( attr == INVALID_FILE_ATTRIBUTES ) {
			GuiTaskSetSystemError(task, L"读取文件属性", GetLastError());
			return FALSE;
		}

		wcsncpy_s(baseDir, _countof(baseDir), task->inputs[i], _TRUNCATE);
		if ( attr & FILE_ATTRIBUTE_DIRECTORY ) {
			slash = wcsrchr(baseDir, L'\\');
			if ( slash == NULL ) {
				slash = wcsrchr(baseDir, L'/');
			}
			if ( slash != NULL ) {
				*slash = L'\0';
			}
			if ( !GuiAddFolderRecursiveRaw(archive, task->packType, task->writePolicy, task->inputs[i], baseDir, &nextIndex, task) ) {
				xpkClose(archive);
				return FALSE;
			}
		} else {
			PathRemoveFileSpecW(baseDir);
			if ( !GuiAddFilesystemEntryRaw(archive, task->packType, task->writePolicy, task->inputs[i], baseDir, &nextIndex, task) ) {
				xpkClose(archive);
				return FALSE;
			}
			GuiTaskAdvanceProgress(task);
		}
	}

	if ( xpkSave(archive) != XPK_OK ) {
		GuiTaskSetArchiveError(task, archive, L"保存归档");
		xpkClose(archive);
		return FALSE;
	}
	xpkClose(archive);
	return TRUE;
}

static BOOL GuiTaskRunReplace(GuiTaskState* task)
{
	xpkObject archive;

	archive = NULL;
	if ( task == NULL || task->inputs == NULL || task->inputCount <= 0 || task->extractItems == NULL || task->extractCount <= 0 ) {
		GuiTaskSetErrorText(task, L"替换任务参数无效。");
		return FALSE;
	}
	if ( !GuiTaskOpenArchiveWorker(task, FALSE, FALSE, L"打开归档", &archive) ) {
		return FALSE;
	}
	if ( !GuiReplaceArchiveEntryRaw(archive, task->packType, &task->extractItems[0], task->writePolicy, task->inputs[0], task) ) {
		xpkClose(archive);
		return FALSE;
	}
	GuiTaskAdvanceProgress(task);
	if ( xpkSave(archive) != XPK_OK ) {
		GuiTaskSetArchiveError(task, archive, L"保存归档");
		xpkClose(archive);
		return FALSE;
	}
	xpkClose(archive);
	return TRUE;
}

static BOOL GuiTaskRunCreateAdd(GuiTaskState* task)
{
	xpkOpenOptions openOpt;
	xpkObject archive;
	xpkPackType packType;
	char utf8Path[MAX_PATH * 4];
	int64_t nextIndex;
	int i;

	GuiTaskSetDetailText(task, task->createOptions.archivePath);
	if ( !GuiUtf8FromWide(task->createOptions.archivePath, utf8Path, sizeof(utf8Path)) ) {
		GuiTaskSetErrorText(task, L"路径转换失败。");
		return FALSE;
	}

	ZeroMemory(&openOpt, sizeof(openOpt));
	openOpt.createIfMissing = 1;
	openOpt.bufferedDefault = (task->createOptions.writePolicy == XPK_WRITE_BUFFERED) ? 1 : 0;
	archive = xpkOpen(utf8Path, &openOpt);
	if ( archive == NULL ) {
		GuiTaskSetArchiveError(task, NULL, L"创建归档");
		return FALSE;
	}

	if ( !GuiApplyArchiveOptionsRaw(archive, &task->createOptions, task) ) {
		xpkClose(archive);
		return FALSE;
	}

	packType = task->createOptions.packType;
	nextIndex = (packType == XPK_PACK_INDEX) ? task->addStartIndex : 0;

	for ( i = 0; i < task->inputCount; ++i ) {
		DWORD attr;
		WCHAR baseDir[MAX_PATH];
		WCHAR* slash;

		if ( GuiTaskShouldCancel(task) ) {
			task->dialogResult = GUI_TASK_RESULT_CANCELLED;
			xpkClose(archive);
			return FALSE;
		}

		attr = GetFileAttributesW(task->inputs[i]);
		if ( attr == INVALID_FILE_ATTRIBUTES ) {
			GuiTaskSetSystemError(task, L"读取文件属性", GetLastError());
			xpkClose(archive);
			return FALSE;
		}

		wcsncpy_s(baseDir, _countof(baseDir), task->inputs[i], _TRUNCATE);
		if ( attr & FILE_ATTRIBUTE_DIRECTORY ) {
			slash = wcsrchr(baseDir, L'\\');
			if ( slash == NULL ) {
				slash = wcsrchr(baseDir, L'/');
			}
			if ( slash != NULL ) {
				*slash = L'\0';
			}
			if ( !GuiAddFolderRecursiveRaw(archive, packType, task->createOptions.writePolicy, task->inputs[i], baseDir, &nextIndex, task) ) {
				xpkClose(archive);
				return FALSE;
			}
		} else {
			PathRemoveFileSpecW(baseDir);
			if ( !GuiAddFilesystemEntryRaw(archive, packType, task->createOptions.writePolicy, task->inputs[i], baseDir, &nextIndex, task) ) {
				xpkClose(archive);
				return FALSE;
			}
			GuiTaskAdvanceProgress(task);
		}
	}

	if ( xpkSave(archive) != XPK_OK ) {
		GuiTaskSetArchiveError(task, archive, L"保存归档");
		xpkClose(archive);
		return FALSE;
	}

	xpkClose(archive);
	return TRUE;
}

static BOOL GuiTaskRunExtract(GuiTaskState* task)
{
	xpkObject archive;
	int i;

	archive = NULL;
	if ( !GuiTaskOpenArchiveWorker(task, TRUE, FALSE, L"打开归档", &archive) ) {
		return FALSE;
	}

	for ( i = 0; i < task->extractCount; ++i ) {
		WCHAR outPath[XPKGUI_MAX_TEMP_PATH];
		char utf8Out[XPKGUI_MAX_TEMP_PATH * 4];
		const GuiArchiveItem* item;

		if ( GuiTaskShouldCancel(task) ) {
			task->dialogResult = GUI_TASK_RESULT_CANCELLED;
			return FALSE;
		}

		item = &task->extractItems[i];
		GuiBuildExtractOutputPath(task->packType, task->destPath, item, outPath, _countof(outPath));
		GuiTaskSetDetailText(task, outPath);
		if ( task->skipExisting && GuiPathExists(outPath) ) {
			GuiTaskAdvanceProgress(task);
			continue;
		}

		if ( !GuiEnsureParentDirectory(outPath) ) {
			GuiTaskSetSystemError(task, L"创建目录", GetLastError());
			return FALSE;
		}
		if ( !GuiUtf8FromWide(outPath, utf8Out, sizeof(utf8Out)) ) {
			GuiTaskSetErrorText(task, L"路径转换失败。");
			xpkClose(archive);
			return FALSE;
		}
		if ( xpkReadToFile(archive, item->pos, utf8Out) != XPK_OK ) {
			GuiTaskSetArchiveError(task, archive, L"解压条目");
			xpkClose(archive);
			return FALSE;
		}
		GuiTaskAdvanceProgress(task);
	}

	xpkClose(archive);
	return TRUE;
}

static BOOL GuiArchiveExtractSingleSelectionToTemp(
	GuiApp* app,
	const WCHAR* taskTitle,
	WCHAR* tempRoot,
	size_t cchTempRoot,
	WCHAR* outPath,
	size_t cchOutPath)
{
	GuiArchiveItem* item;
	GuiArchiveItem itemCopy;
	GuiTaskState task;
	GuiTaskResult result;

	if ( app == NULL || app->archive == NULL || tempRoot == NULL || cchTempRoot == 0 || outPath == NULL || cchOutPath == 0 ) {
		return FALSE;
	}

	item = GuiArchiveGetSingleSelectedItem(app);
	if ( item == NULL ) {
		MessageBoxW(app->window, L"请选择单个文件进行操作。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	if ( !GuiBuildOpenTempRootPath(app, tempRoot, cchTempRoot) ) {
		GuiShowSystemErrorDetail(app->window, L"创建临时目录失败", L"无法为包内文件创建临时解压根目录。", NULL, GetLastError());
		return FALSE;
	}
	GuiBuildExtractOutputPath(app->packType, tempRoot, item, outPath, cchOutPath);

	itemCopy = *item;
	ZeroMemory(&task, sizeof(task));
	task.kind = GUI_TASK_EXTRACT;
	task.app = app;
	task.canCancel = TRUE;
	task.marquee = FALSE;
	task.packType = app->packType;
	task.writePolicy = XPK_WRITE_BUFFERED;
	task.progressTotal = 1;
	wcsncpy_s(task.title, _countof(task.title), taskTitle, _TRUNCATE);
	wcsncpy_s(task.archivePath, _countof(task.archivePath), app->archivePath, _TRUNCATE);
	wcsncpy_s(task.destPath, _countof(task.destPath), tempRoot, _TRUNCATE);
	task.extractItems = &itemCopy;
	task.extractCount = 1;

	result = GuiRunTaskDialog(&task);
	return result == GUI_TASK_RESULT_SUCCESS;
}

static BOOL GuiArchiveReplaceItemFromPath(GuiApp* app, const GuiArchiveItem* sourceItem, const WCHAR* sourcePath, const WCHAR* taskTitle)
{
	GuiTaskState task;
	GuiTaskResult result;
	GuiArchiveItem* itemCopy;
	WCHAR* inputs[1];
	WCHAR reopenPath[MAX_PATH];

	if ( app == NULL || app->archive == NULL || sourceItem == NULL || sourcePath == NULL || sourcePath[0] == L'\0' ) {
		return FALSE;
	}

	itemCopy = (GuiArchiveItem*)malloc(sizeof(*itemCopy));
	if ( itemCopy == NULL ) {
		MessageBoxW(app->window, L"内存不足。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}
	*itemCopy = *sourceItem;

	ZeroMemory(&task, sizeof(task));
	task.kind = GUI_TASK_REPLACE;
	task.app = app;
	task.packType = app->packType;
	task.writePolicy = app->writePolicy;
	task.canCancel = FALSE;
	task.marquee = TRUE;
	task.progressTotal = 1;
	wcsncpy_s(task.title, _countof(task.title), (taskTitle != NULL && taskTitle[0] != L'\0') ? taskTitle : L"替换条目", _TRUNCATE);
	wcsncpy_s(task.archivePath, _countof(task.archivePath), app->archivePath, _TRUNCATE);
	task.extractItems = itemCopy;
	task.extractCount = 1;
	inputs[0] = (WCHAR*)sourcePath;
	task.inputs = GuiDuplicateInputList(1, inputs);
	task.inputCount = 1;
	if ( task.inputs == NULL ) {
		free(itemCopy);
		MessageBoxW(app->window, L"内存不足。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	wcsncpy_s(reopenPath, _countof(reopenPath), app->archivePath, _TRUNCATE);
	result = GuiRunTaskDialog(&task);
	GuiFreeInputList(task.inputCount, task.inputs);
	free(task.extractItems);
	if ( reopenPath[0] != L'\0' ) {
		GuiArchiveOpenPath(app, reopenPath, FALSE);
	}
	return result == GUI_TASK_RESULT_SUCCESS;
}

BOOL GuiArchiveOpenSelection(GuiApp* app)
{
	WCHAR tempRoot[XPKGUI_MAX_TEMP_PATH];
	WCHAR outPath[XPKGUI_MAX_TEMP_PATH];

	if ( app == NULL || app->archive == NULL ) {
		return FALSE;
	}

	if ( !GuiArchiveExtractSingleSelectionToTemp(app, L"打开文件", tempRoot, _countof(tempRoot), outPath, _countof(outPath)) ) {
		return FALSE;
	}
	if ( GuiPathIsXpkArchive(outPath) ) {
		if ( !GuiLaunchArchiveInNewGui(app->window, outPath) ) {
			GuiShowSystemErrorDetail(app->window, L"启动 xpkgui 失败", L"已提取内层 .xpk 到临时目录，但无法启动新的 xpkgui 进程。", outPath, GetLastError());
			return FALSE;
		}
		return TRUE;
	}
	return GuiOpenExtractedPath(app->window, outPath);
}

BOOL GuiArchiveEditSelection(GuiApp* app)
{
	GuiArchiveItem* selectedItem;
	GuiArchiveItem itemCopy;
	WCHAR tempRoot[XPKGUI_MAX_TEMP_PATH];
	WCHAR outPath[XPKGUI_MAX_TEMP_PATH];
	WIN32_FILE_ATTRIBUTE_DATA beforeStamp;
	WIN32_FILE_ATTRIBUTE_DATA afterStamp;
	BOOL hadBeforeStamp;

	if ( app == NULL || app->archive == NULL ) {
		return FALSE;
	}
	selectedItem = GuiArchiveGetSingleSelectedItem(app);
	if ( selectedItem == NULL ) {
		MessageBoxW(app->window, L"请选择单个文件进行编辑。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}
	itemCopy = *selectedItem;

	if ( !GuiArchiveExtractSingleSelectionToTemp(app, L"编辑文件", tempRoot, _countof(tempRoot), outPath, _countof(outPath)) ) {
		return FALSE;
	}
	hadBeforeStamp = GuiGetFileStamp(outPath, &beforeStamp);
	if ( !GuiLaunchPathWithNotepad(app->window, outPath) ) {
		GuiShowSystemErrorDetail(app->window, L"启动文本编辑器失败", L"文件已保留在临时目录，未写回归档。", outPath, GetLastError());
		return FALSE;
	}
	if ( !GuiGetFileStamp(outPath, &afterStamp) ) {
		GuiShowSystemErrorDetail(app->window, L"临时文件不存在", L"编辑后的临时文件已不存在，未写回归档。", outPath, GetLastError());
		return FALSE;
	}
	if ( hadBeforeStamp && GuiFileStampEquals(&beforeStamp, &afterStamp) ) {
		return TRUE;
	}
	if ( MessageBoxW(app->window, L"检测到文件已修改，是否写回归档？", XPKGUI_APP_TITLE, MB_YESNO | MB_ICONQUESTION) != IDYES ) {
		return TRUE;
	}
	if ( !GuiArchiveReplaceItemFromPath(app, &itemCopy, outPath, L"回写条目") ) {
		WCHAR text[1024];

		_snwprintf_s(
			text,
			_countof(text),
			_TRUNCATE,
			L"外部编辑后的文件没有成功写回归档。\n\n临时文件已保留，可手工复制或稍后用 Replace 重试：\n%s",
			outPath);
		MessageBoxW(app->window, text, XPKGUI_APP_TITLE, MB_OK | MB_ICONWARNING);
		return FALSE;
	}
	return TRUE;
}

BOOL GuiArchiveReplaceSelection(GuiApp* app)
{
	GuiArchiveItem* selectedItem;
	WCHAR sourcePath[MAX_PATH];

	if ( app == NULL || app->archive == NULL ) {
		return FALSE;
	}
	if ( !GuiArchiveCanReplaceSelection(app) ) {
		MessageBoxW(app->window, L"请选择单个文件进行替换。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}
	if ( !GuiPickSingleFileDialog(app->window, sourcePath, _countof(sourcePath), L"选择替换源文件") ) {
		return FALSE;
	}
	selectedItem = GuiArchiveGetSingleSelectedItem(app);
	if ( selectedItem == NULL ) {
		return FALSE;
	}
	return GuiArchiveReplaceItemFromPath(app, selectedItem, sourcePath, L"替换条目");
}

BOOL GuiArchiveDuplicateSelection(GuiApp* app)
{
	GuiArchiveItem* item;
	xpkWriteOptions writeOpt;
	WCHAR tempRoot[XPKGUI_MAX_TEMP_PATH];
	WCHAR sourcePath[XPKGUI_MAX_TEMP_PATH];
	WCHAR targetPath[XPKGUI_ITEM_TEXT];
	char utf8Source[XPK_PATH_BYTES];
	char utf8Target[XPK_PATH_BYTES];
	int64_t fileIndex;
	uint32_t newPos;
	void* infoBuf;

	if ( app == NULL || app->archive == NULL ) {
		return FALSE;
	}
	if ( !GuiArchiveCanDuplicateSelection(app) ) {
		MessageBoxW(app->window, L"请选择单个文件进行复制。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	item = GuiArchiveGetSingleSelectedItem(app);
	if ( item == NULL ) {
		return FALSE;
	}
	if ( !GuiArchiveExtractSingleSelectionToTemp(app, L"复制条目", tempRoot, _countof(tempRoot), sourcePath, _countof(sourcePath)) ) {
		return FALSE;
	}
	if ( !GuiUtf8FromWide(sourcePath, utf8Source, sizeof(utf8Source)) ) {
		MessageBoxW(app->window, L"临时文件路径转换失败。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	ZeroMemory(&writeOpt, sizeof(writeOpt));
	writeOpt.compLevel = 0xFFu;
	writeOpt.writePolicy = app->writePolicy;
	writeOpt.fileType = GuiEntryFileType(item->flag);

	infoBuf = NULL;
	newPos = 0;
	fileIndex = 0;
	targetPath[0] = L'\0';

	if ( app->packType == XPK_PACK_CORE && app->infoExtSize > 0 ) {
		infoBuf = malloc(app->infoExtSize);
		if ( infoBuf == NULL ) {
			MessageBoxW(app->window, L"内存不足，无法复制条目 InfoExt。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
			return FALSE;
		}
		if ( xpkGetInfoExt(app->archive, item->pos, infoBuf, app->infoExtSize) != XPK_OK ) {
			free(infoBuf);
			GuiShowArchiveError(app, L"读取条目 InfoExt");
			return FALSE;
		}
	}

	if ( app->packType == XPK_PACK_INDEX ) {
		int64_t defaultIndex;

		defaultIndex = item->fileIndex + 1;
		if ( defaultIndex <= item->fileIndex || xpkIndexFind(app->archive, defaultIndex, NULL) == XPK_OK ) {
			defaultIndex = GuiFindNextIndexSeed(app);
		}
		if ( !GuiPromptIndexAddOptions(app->window, L"复制条目", defaultIndex, &fileIndex) ) {
			free(infoBuf);
			return FALSE;
		}
		if ( xpkIndexFind(app->archive, fileIndex, NULL) == XPK_OK ) {
			free(infoBuf);
			MessageBoxW(app->window, L"该 fileIndex 已存在。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
			return FALSE;
		}
		if ( xpkIndexAddFile(app->archive, fileIndex, utf8Source, &writeOpt) != XPK_OK ) {
			free(infoBuf);
			GuiShowArchiveError(app, L"复制条目");
			return FALSE;
		}
	} else if ( GuiIsPathPackType(app->packType) ) {
		GuiInputDialogState dialogState;
		WCHAR defaultValue[XPKGUI_ITEM_TEXT];
		WCHAR candidateLeaf[XPKGUI_ITEM_TEXT];
		WCHAR candidateFull[XPKGUI_ITEM_TEXT];
		WCHAR sourceDir[XPKGUI_ITEM_TEXT];
		const WCHAR* remainder;
		const WCHAR* slash;
		size_t dirLen;
		UINT copyIndex;
		BOOL foundCandidate;

		sourceDir[0] = L'\0';
		slash = wcsrchr(item->name, L'/');
		if ( slash == NULL ) {
			slash = wcsrchr(item->name, L'\\');
		}
		if ( slash != NULL ) {
			dirLen = (size_t)(slash - item->name);
			if ( dirLen >= _countof(sourceDir) ) {
				dirLen = _countof(sourceDir) - 1;
			}
			wcsncpy_s(sourceDir, _countof(sourceDir), item->name, dirLen);
			sourceDir[dirLen] = L'\0';
		}

		candidateFull[0] = L'\0';
		foundCandidate = FALSE;
		for ( copyIndex = 0; copyIndex < 1024u; ++copyIndex ) {
			GuiBuildDuplicateLeafName(item->name, copyIndex, candidateLeaf, _countof(candidateLeaf));
			if ( sourceDir[0] != L'\0' ) {
				GuiBuildChildViewPath(sourceDir, candidateLeaf, candidateFull, _countof(candidateFull));
			} else {
				wcsncpy_s(candidateFull, _countof(candidateFull), candidateLeaf, _TRUNCATE);
			}
			GuiPathToPackageUtf8(candidateFull, utf8Target, sizeof(utf8Target));
			if ( !xpkPathExists(app->archive, utf8Target) ) {
				foundCandidate = TRUE;
				break;
			}
		}
		if ( !foundCandidate ) {
			free(infoBuf);
			MessageBoxW(app->window, L"无法为复制条目生成目标路径。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
			return FALSE;
		}

		ZeroMemory(&dialogState, sizeof(dialogState));
		dialogState.title = L"复制条目";
		if ( app->currentFolder[0] != L'\0' && !app->flatView ) {
			dialogState.prompt = L"新包内路径(相对当前目录):";
		} else {
			dialogState.prompt = L"新包内路径:";
		}
		if ( app->currentFolder[0] != L'\0' && !app->flatView && GuiPathMatchFolderPrefix(candidateFull, app->currentFolder, &remainder) ) {
			wcsncpy_s(defaultValue, _countof(defaultValue), remainder, _TRUNCATE);
		} else {
			wcsncpy_s(defaultValue, _countof(defaultValue), candidateFull, _TRUNCATE);
		}
		wcsncpy_s(dialogState.value, _countof(dialogState.value), defaultValue, _TRUNCATE);
		if ( !GuiRunInputDialog(app->window, &dialogState) ) {
			free(infoBuf);
			return FALSE;
		}
		if ( !GuiBuildRenameTargetPath(app, dialogState.value, targetPath, _countof(targetPath)) ) {
			free(infoBuf);
			MessageBoxW(app->window, L"请输入有效的包内路径。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
			return FALSE;
		}
		if ( _wcsicmp(targetPath, item->name) == 0 ) {
			free(infoBuf);
			MessageBoxW(app->window, L"新包内路径不能与原条目相同。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
			return FALSE;
		}
		GuiPathToPackageUtf8(targetPath, utf8Target, sizeof(utf8Target));
		if ( xpkPathExists(app->archive, utf8Target) ) {
			free(infoBuf);
			MessageBoxW(app->window, L"该包内路径已存在。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
			return FALSE;
		}
		if ( xpkPathAddFile(app->archive, utf8Target, utf8Source, &writeOpt) != XPK_OK ) {
			free(infoBuf);
			GuiShowArchiveError(app, L"复制条目");
			return FALSE;
		}
		if ( xpkPathSetAttr(app->archive, utf8Target, item->attr) != XPK_OK ) {
			free(infoBuf);
			GuiShowArchiveError(app, L"复制条目");
			GuiArchiveOpenPath(app, app->archivePath, FALSE);
			return FALSE;
		}
	} else {
		if ( xpkAddFile(app->archive, utf8Source, &writeOpt, &newPos) != XPK_OK ) {
			free(infoBuf);
			GuiShowArchiveError(app, L"复制条目");
			return FALSE;
		}
	}

	if ( infoBuf != NULL ) {
		if ( xpkSetInfoExt(app->archive, newPos, infoBuf, app->infoExtSize) != XPK_OK ) {
			free(infoBuf);
			GuiShowArchiveError(app, L"复制条目");
			GuiArchiveOpenPath(app, app->archivePath, FALSE);
			return FALSE;
		}
		free(infoBuf);
	}

	if ( !GuiArchiveSave(app) ) {
		return FALSE;
	}

	if ( app->packType == XPK_PACK_INDEX ) {
		_snwprintf_s(targetPath, _countof(targetPath), _TRUNCATE, L"%lld", (long long)fileIndex);
		GuiSelectViewItemByFullPath(app, GUI_VIEW_ITEM_FILE, targetPath);
	} else if ( GuiIsPathPackType(app->packType) ) {
		GuiSelectViewItemByFullPath(app, GUI_VIEW_ITEM_FILE, targetPath);
	} else {
		_snwprintf_s(targetPath, _countof(targetPath), _TRUNCATE, L"Entry %u", newPos);
		GuiSelectViewItemByFullPath(app, GUI_VIEW_ITEM_FILE, targetPath);
	}
	return TRUE;
}

BOOL GuiArchiveOpenSelectionWith(GuiApp* app)
{
	WCHAR tempRoot[XPKGUI_MAX_TEMP_PATH];
	WCHAR outPath[XPKGUI_MAX_TEMP_PATH];

	if ( app == NULL || app->archive == NULL ) {
		return FALSE;
	}

	if ( !GuiArchiveExtractSingleSelectionToTemp(app, L"选择打开方式", tempRoot, _countof(tempRoot), outPath, _countof(outPath)) ) {
		return FALSE;
	}
	return GuiShowOpenWithDialogForPath(app->window, outPath);
}

BOOL GuiArchiveShowSelectionInExplorer(GuiApp* app)
{
	WCHAR tempRoot[XPKGUI_MAX_TEMP_PATH];
	WCHAR outPath[XPKGUI_MAX_TEMP_PATH];
	WCHAR parameters[(XPKGUI_MAX_TEMP_PATH * 2) + 32];
	INT_PTR shellResult;

	if ( !GuiArchiveExtractSingleSelectionToTemp(app, L"定位文件", tempRoot, _countof(tempRoot), outPath, _countof(outPath)) ) {
		return FALSE;
	}

	_snwprintf_s(parameters, _countof(parameters), _TRUNCATE, L"/select,\"%s\"", outPath);
	shellResult = (INT_PTR)ShellExecuteW(app->window, L"open", L"explorer.exe", parameters, NULL, SW_SHOWNORMAL);
	if ( shellResult <= 32 ) {
		WCHAR text[1024];

		_snwprintf_s(
			text,
			_countof(text),
			_TRUNCATE,
			L"文件已提取到临时目录，但无法在 Explorer 中定位。\n\n%s",
			outPath);
		MessageBoxW(app->window, text, XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}
	return TRUE;
}

static BOOL GuiTaskRunVerify(GuiTaskState* task)
{
	xpkObject archive;
	int i;

	archive = NULL;
	if ( !GuiTaskOpenArchiveWorker(task, TRUE, FALSE, L"打开归档", &archive) ) {
		return FALSE;
	}

	if ( task->extractItems != NULL && task->extractCount > 0 ) {
		for ( i = 0; i < task->extractCount; ++i ) {
			if ( GuiTaskShouldCancel(task) ) {
				task->dialogResult = GUI_TASK_RESULT_CANCELLED;
				xpkClose(archive);
				return FALSE;
			}
			GuiTaskSetDetailText(task, task->extractItems[i].name);
			if ( xpkVerify(archive, task->extractItems[i].pos) != XPK_OK ) {
				GuiTaskSetArchiveError(task, archive, L"校验条目");
				xpkClose(archive);
				return FALSE;
			}
			GuiTaskAdvanceProgress(task);
		}
	} else {
		if ( xpkVerifyAll(archive) != XPK_OK ) {
			GuiTaskSetArchiveError(task, archive, L"校验归档");
			xpkClose(archive);
			return FALSE;
		}
	}
	xpkClose(archive);
	return TRUE;
}

static BOOL GuiTaskRunBuild(GuiTaskState* task)
{
	xpkObject archive;

	archive = NULL;
	if ( !GuiTaskOpenArchiveWorker(task, FALSE, FALSE, L"打开归档", &archive) ) {
		return FALSE;
	}
	if ( xpkBuild(archive, NULL) != XPK_OK ) {
		GuiTaskSetArchiveError(task, archive, L"重构归档");
		xpkClose(archive);
		return FALSE;
	}
	xpkClose(archive);
	return TRUE;
}

static unsigned __stdcall GuiTaskThreadProc(void* userData)
{
	GuiTaskState* task;
	BOOL ok;

	task = (GuiTaskState*)userData;
	ok = FALSE;

	switch ( task->kind ) {
		case GUI_TASK_ADD:
			ok = GuiTaskRunAddSelection(task);
			break;
		case GUI_TASK_CREATE_ADD:
			ok = GuiTaskRunCreateAdd(task);
			break;
		case GUI_TASK_REPLACE:
			ok = GuiTaskRunReplace(task);
			break;
		case GUI_TASK_EXTRACT:
			ok = GuiTaskRunExtract(task);
			break;
		case GUI_TASK_VERIFY:
			ok = GuiTaskRunVerify(task);
			break;
		case GUI_TASK_BUILD:
			ok = GuiTaskRunBuild(task);
			break;
	}

	if ( ok ) {
		task->dialogResult = GUI_TASK_RESULT_SUCCESS;
	} else if ( task->dialogResult != GUI_TASK_RESULT_CANCELLED ) {
		task->dialogResult = GUI_TASK_RESULT_FAILED;
	}

	PostMessageW(task->dialog, WM_XPKGUI_TASK_DONE, 0, 0);
	return 0;
}

static INT_PTR CALLBACK GuiTaskDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	GuiTaskState* task;

	task = (GuiTaskState*)GetWindowLongPtrW(hDlg, GWLP_USERDATA);
	switch ( msg ) {
		case WM_INITDIALOG:
		{
			unsigned threadId;
			WCHAR status[256];
			WCHAR detail[256];
			LONG total;

			task = (GuiTaskState*)lParam;
			SetWindowLongPtrW(hDlg, GWLP_USERDATA, (LONG_PTR)task);
			task->dialog = hDlg;
			SetWindowTextW(hDlg, task->title);
			GuiTaskBuildStatusText(task, status, _countof(status));
			GuiTaskBuildDetailText(task, detail, _countof(detail));
			SetDlgItemTextW(hDlg, IDC_TASK_STATUS, status);
			SetDlgItemTextW(hDlg, IDC_TASK_DETAIL, detail);

			total = task->progressTotal;
			if ( total <= 0 ) {
				total = 1;
			}
			SendDlgItemMessageW(hDlg, IDC_TASK_PROGRESS_BAR, PBM_SETRANGE32, 0, total);
			SendDlgItemMessageW(hDlg, IDC_TASK_PROGRESS_BAR, PBM_SETPOS, 0, 0);
			if ( task->marquee ) {
				SendDlgItemMessageW(hDlg, IDC_TASK_PROGRESS_BAR, PBM_SETMARQUEE, TRUE, 30);
			}
			if ( !task->canCancel ) {
				EnableWindow(GetDlgItem(hDlg, IDCANCEL), FALSE);
			}

			task->thread = (HANDLE)_beginthreadex(NULL, 0, GuiTaskThreadProc, task, 0, &threadId);
			if ( task->thread == NULL ) {
				task->dialogResult = GUI_TASK_RESULT_FAILED;
				GuiTaskSetErrorText(task, L"无法创建后台线程。");
				EndDialog(hDlg, task->dialogResult);
				return TRUE;
			}

			SetTimer(hDlg, XPKGUI_TASK_TIMER_ID, 120, NULL);
			return TRUE;
		}
		case WM_TIMER:
			if ( task != NULL && wParam == XPKGUI_TASK_TIMER_ID ) {
				WCHAR status[256];
				WCHAR detail[256];
				GuiTaskBuildStatusText(task, status, _countof(status));
				GuiTaskBuildDetailText(task, detail, _countof(detail));
				SetDlgItemTextW(hDlg, IDC_TASK_STATUS, status);
				SetDlgItemTextW(hDlg, IDC_TASK_DETAIL, detail);
				if ( !task->marquee ) {
					LONG current;
					current = InterlockedCompareExchange(&task->progressCurrent, 0, 0);
					SendDlgItemMessageW(hDlg, IDC_TASK_PROGRESS_BAR, PBM_SETPOS, current, 0);
				}
			}
			return TRUE;
		case WM_CLOSE:
			if ( task != NULL && task->canCancel && task->thread != NULL ) {
				InterlockedExchange(&task->cancelRequested, 1);
				EnableWindow(GetDlgItem(hDlg, IDCANCEL), FALSE);
				SetDlgItemTextW(hDlg, IDC_TASK_STATUS, L"正在取消...");
				return TRUE;
			}
			return TRUE;
		case WM_COMMAND:
			if ( LOWORD(wParam) == IDCANCEL ) {
				SendMessageW(hDlg, WM_CLOSE, 0, 0);
				return TRUE;
			}
			break;
		case WM_XPKGUI_TASK_DONE:
			if ( task != NULL ) {
				KillTimer(hDlg, XPKGUI_TASK_TIMER_ID);
				if ( task->thread != NULL ) {
					WaitForSingleObject(task->thread, INFINITE);
					CloseHandle(task->thread);
					task->thread = NULL;
				}
				EndDialog(hDlg, task->dialogResult);
				return TRUE;
			}
			break;
	}
	return FALSE;
}

static GuiTaskResult GuiRunTaskDialog(GuiTaskState* task)
{
	INT_PTR ret;

	InitializeCriticalSection(&task->detailLock);
	ret = DialogBoxParamW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDD_TASK_PROGRESS), task->app != NULL ? task->app->window : NULL, GuiTaskDialogProc, (LPARAM)task);
	DeleteCriticalSection(&task->detailLock);
	if ( ret == GUI_TASK_RESULT_FAILED && task->errorText[0] != L'\0' ) {
		MessageBoxW(task->app != NULL ? task->app->window : NULL, task->errorText, XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
	} else if ( ret == GUI_TASK_RESULT_CANCELLED && !GuiIsSmokeMode() ) {
		WCHAR message[512];

		_snwprintf_s(
			message,
			_countof(message),
			_TRUNCATE,
			L"%s 已取消。\r\n\r\n已完成的文件操作不会自动回滚；未处理的条目已停止继续执行。",
			task->title[0] != L'\0' ? task->title : L"任务");
		MessageBoxW(task->app != NULL ? task->app->window : NULL, message, XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
	}
	return (GuiTaskResult)ret;
}

BOOL GuiArchiveAddSelection(GuiApp* app, int count, WCHAR** items)
{
	GuiTaskState task;
	WCHAR reopenPath[MAX_PATH];
	GuiTaskResult result;

	if ( app->archive == NULL || count <= 0 || items == NULL ) {
		return FALSE;
	}

	ZeroMemory(&task, sizeof(task));
	task.kind = GUI_TASK_ADD;
	task.app = app;
	task.packType = app->packType;
	task.writePolicy = app->writePolicy;
	wcsncpy_s(task.archivePath, _countof(task.archivePath), app->archivePath, _TRUNCATE);
	if ( !GuiPrepareAddTaskOptions(app, app->packType, app->archivePath, &task) ) {
		return FALSE;
	}
	task.canCancel = TRUE;
	task.marquee = FALSE;
	task.progressTotal = (LONG)GuiCountInputFiles(count, items);
	if ( task.progressTotal <= 0 ) {
		task.progressTotal = 1;
	}
	wcsncpy_s(task.title, _countof(task.title), L"添加文件", _TRUNCATE);
	task.inputs = GuiDuplicateInputList(count, items);
	task.inputCount = count;
	if ( task.inputs == NULL ) {
		MessageBoxW(app->window, L"内存不足。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	wcsncpy_s(reopenPath, _countof(reopenPath), app->archivePath, _TRUNCATE);
	result = GuiRunTaskDialog(&task);
	GuiFreeInputList(task.inputCount, task.inputs);
	if ( result == GUI_TASK_RESULT_SUCCESS ) {
		if ( reopenPath[0] != L'\0' ) {
			return GuiArchiveOpenPath(app, reopenPath, FALSE);
		}
		return GuiArchiveRefreshView(app);
	}

	if ( reopenPath[0] != L'\0' ) {
		GuiArchiveOpenPath(app, reopenPath, FALSE);
	}
	return FALSE;
}

BOOL GuiArchiveCreateFromSelection(GuiApp* app, int count, WCHAR** items, BOOL autoPath, BOOL exitAfterCreate)
{
	GuiArchiveOptions options;
	WCHAR suggestedPath[MAX_PATH];
	GuiTaskState task;
	GuiTaskResult result;
	DWORD existingAttr;

	if ( count <= 0 || items == NULL ) {
		return FALSE;
	}

	ZeroMemory(&options, sizeof(options));
	GuiSuggestArchivePath(count, items, suggestedPath, _countof(suggestedPath), autoPath);

	if ( autoPath ) {
		options.packType = XPK_PACK_WIN32;
		options.defaultComp = app->defaultComp;
		options.metaComp = app->metaComp;
		options.infoComp = app->infoComp;
		options.infoExtSize = 0;
		options.volumeSize = 0;
		options.writePolicy = app->writePolicy;
		options.solidMode = FALSE;
		wcsncpy_s(options.archivePath, _countof(options.archivePath), suggestedPath, _TRUNCATE);
	} else {
		if ( !GuiArchivePromptNew(app, suggestedPath, &options) ) {
			return FALSE;
		}
	}

	ZeroMemory(&task, sizeof(task));
	task.kind = GUI_TASK_CREATE_ADD;
	task.app = app;
	task.packType = options.packType;
	task.writePolicy = options.writePolicy;
	wcsncpy_s(task.archivePath, _countof(task.archivePath), options.archivePath, _TRUNCATE);
	if ( !autoPath && !GuiPrepareAddTaskOptions(app, options.packType, options.archivePath, &task) ) {
		return FALSE;
	}
	task.canCancel = TRUE;
	task.marquee = FALSE;
	task.progressTotal = (LONG)GuiCountInputFiles(count, items);
	if ( task.progressTotal <= 0 ) {
		task.progressTotal = 1;
	}
	wcsncpy_s(task.title, _countof(task.title), L"创建归档", _TRUNCATE);
	task.createOptions = options;
	task.inputs = GuiDuplicateInputList(count, items);
	task.inputCount = count;
	if ( task.inputs == NULL ) {
		MessageBoxW(app->window, L"内存不足。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	existingAttr = GetFileAttributesW(options.archivePath);
	result = GuiRunTaskDialog(&task);
	GuiFreeInputList(task.inputCount, task.inputs);
	if ( result == GUI_TASK_RESULT_SUCCESS ) {
		if ( !exitAfterCreate && !GuiArchiveOpenPath(app, options.archivePath, FALSE) ) {
			return FALSE;
		}
		if ( exitAfterCreate && !GuiIsSmokeMode() ) {
			MessageBoxW(app->window, L"压缩包创建完成。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		}
		return TRUE;
	}

	if ( result != GUI_TASK_RESULT_SUCCESS && existingAttr == INVALID_FILE_ATTRIBUTES ) {
		DeleteFileW(options.archivePath);
	}
	if ( exitAfterCreate ) {
		return FALSE;
	}
	if ( app->archive != NULL ) {
		GuiArchiveRefreshView(app);
	} else {
		GuiUpdateStatus(app);
		GuiUpdateTitle(app);
	}
	return FALSE;
}

BOOL GuiArchiveExtractPath(const WCHAR* archivePath, const WCHAR* destPath)
{
	BOOL cancelled;

	cancelled = FALSE;
	return GuiArchiveExtractPathTask(archivePath, destPath, &cancelled);
}

BOOL GuiArchiveExtractPathTask(const WCHAR* archivePath, const WCHAR* destPath, BOOL* cancelledOut)
{
	GuiApp tempApp;
	GuiTaskState task;
	GuiTaskResult result;
	WCHAR samplePath[MAX_PATH];
	uint32_t existingCount;
	int promptResult;

	ZeroMemory(&tempApp, sizeof(tempApp));
	GuiAppInitDefaults(&tempApp);
	if ( cancelledOut != NULL ) {
		*cancelledOut = FALSE;
	}
	if ( !GuiArchiveOpenPath(&tempApp, archivePath, TRUE) ) {
		return FALSE;
	}
	if ( tempApp.itemCount == 0 ) {
		GuiAppCloseArchive(&tempApp);
		return TRUE;
	}

	ZeroMemory(&task, sizeof(task));
	task.kind = GUI_TASK_EXTRACT;
	task.app = &tempApp;
	task.canCancel = TRUE;
	task.marquee = FALSE;
	task.packType = tempApp.packType;
	task.writePolicy = XPK_WRITE_BUFFERED;
	task.progressTotal = (LONG)tempApp.itemCount;
	wcsncpy_s(task.title, _countof(task.title), L"解压文件", _TRUNCATE);
	wcsncpy_s(task.archivePath, _countof(task.archivePath), archivePath, _TRUNCATE);
	wcsncpy_s(task.destPath, _countof(task.destPath), destPath, _TRUNCATE);
	task.extractItems = tempApp.items;
	task.extractCount = (int)tempApp.itemCount;
	task.skipExisting = FALSE;

	existingCount = GuiCountExistingExtractTargets(task.extractItems, task.extractCount, task.packType, task.destPath, samplePath, _countof(samplePath));
	if ( existingCount > 0 ) {
		promptResult = GuiPromptExtractOverwrite(tempApp.window, existingCount, samplePath);
		if ( promptResult == IDCANCEL ) {
			GuiAppCloseArchive(&tempApp);
			if ( cancelledOut != NULL ) {
				*cancelledOut = TRUE;
			}
			return FALSE;
		}
		task.skipExisting = (promptResult == IDNO);
	}

	result = GuiRunTaskDialog(&task);
	GuiAppCloseArchive(&tempApp);
	if ( result == GUI_TASK_RESULT_CANCELLED ) {
		if ( cancelledOut != NULL ) {
			*cancelledOut = TRUE;
		}
		return FALSE;
	}
	return result == GUI_TASK_RESULT_SUCCESS;
}

BOOL GuiArchiveExtractSelection(GuiApp* app, BOOL allItems, const WCHAR* fixedDest)
{
	WCHAR destPath[MAX_PATH];
	GuiTaskState task;
	GuiTaskResult result;
	WCHAR samplePath[MAX_PATH];
	uint32_t existingCount;
	int promptResult;

	if ( app->archive == NULL ) {
		return FALSE;
	}
	if ( !allItems && !GuiArchiveCanExtractSelection(app) ) {
		MessageBoxW(app->window, L"请选择文件或目录进行解压。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	if ( fixedDest != NULL ) {
		wcsncpy_s(destPath, _countof(destPath), fixedDest, _TRUNCATE);
	} else if ( !GuiPickFolderDialog(app->window, destPath, _countof(destPath), L"选择解压目录") ) {
		return FALSE;
	}

	ZeroMemory(&task, sizeof(task));
	task.kind = GUI_TASK_EXTRACT;
	task.app = app;
	task.canCancel = TRUE;
	task.marquee = FALSE;
	task.packType = app->packType;
	task.writePolicy = XPK_WRITE_BUFFERED;
	wcsncpy_s(task.title, _countof(task.title), L"解压文件", _TRUNCATE);
	wcsncpy_s(task.archivePath, _countof(task.archivePath), app->archivePath, _TRUNCATE);
	wcsncpy_s(task.destPath, _countof(task.destPath), destPath, _TRUNCATE);
	task.extractItems = GuiDuplicateExtractItems(app, allItems, &task.extractCount);
	if ( task.extractItems == NULL || task.extractCount <= 0 ) {
		free(task.extractItems);
		if ( !allItems ) {
			MessageBoxW(app->window, L"请选择文件或目录进行解压。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		}
		return FALSE;
	}
	task.progressTotal = task.extractCount;
	task.skipExisting = FALSE;
	existingCount = GuiCountExistingExtractTargets(task.extractItems, task.extractCount, task.packType, task.destPath, samplePath, _countof(samplePath));
	if ( existingCount > 0 ) {
		promptResult = GuiPromptExtractOverwrite(app->window, existingCount, samplePath);
		if ( promptResult == IDCANCEL ) {
			free(task.extractItems);
			return FALSE;
		}
		task.skipExisting = (promptResult == IDNO);
	}
	result = GuiRunTaskDialog(&task);
	free(task.extractItems);

	if ( result == GUI_TASK_RESULT_SUCCESS ) {
		MessageBoxW(app->window, L"解压完成。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return TRUE;
	}
	return FALSE;
}

static int __cdecl GuiArchiveItemPosDescCompare(const void* leftPtr, const void* rightPtr)
{
	const GuiArchiveItem* left;
	const GuiArchiveItem* right;

	left = (const GuiArchiveItem*)leftPtr;
	right = (const GuiArchiveItem*)rightPtr;
	if ( left->pos < right->pos ) return 1;
	if ( left->pos > right->pos ) return -1;
	return 0;
}

typedef struct GuiRenamePathPair {
	WCHAR oldPath[XPKGUI_ITEM_TEXT];
	WCHAR newPath[XPKGUI_ITEM_TEXT];
	size_t oldLen;
} GuiRenamePathPair;

static void GuiTrimPathNameForEdit(const WCHAR* src, WCHAR* dst, size_t cchDst)
{
	size_t len;

	if ( src == NULL ) {
		dst[0] = L'\0';
		return;
	}

	wcsncpy_s(dst, cchDst, src, _TRUNCATE);
	len = wcslen(dst);
	while ( len > 0 && (dst[len - 1] == L'/' || dst[len - 1] == L'\\') ) {
		dst[--len] = L'\0';
	}
}

static void GuiBuildDuplicateLeafName(const WCHAR* sourceName, UINT copyIndex, WCHAR* outName, size_t cchOutName)
{
	const WCHAR* leaf;
	const WCHAR* slash1;
	const WCHAR* slash2;
	const WCHAR* ext;
	size_t leafLen;
	size_t baseLen;

	if ( outName == NULL || cchOutName == 0 ) {
		return;
	}

	leaf = sourceName;
	if ( leaf == NULL || leaf[0] == L'\0' ) {
		leaf = L"entry";
	} else {
		slash1 = wcsrchr(leaf, L'/');
		slash2 = wcsrchr(leaf, L'\\');
		if ( slash1 != NULL || slash2 != NULL ) {
			const WCHAR* lastSlash;

			lastSlash = slash1;
			if ( lastSlash == NULL || (slash2 != NULL && slash2 > lastSlash) ) {
				lastSlash = slash2;
			}
			if ( lastSlash != NULL && lastSlash[1] != L'\0' ) {
				leaf = lastSlash + 1;
			}
		}
	}

	leafLen = wcslen(leaf);
	ext = wcsrchr(leaf, L'.');
	if ( ext == NULL || ext == leaf ) {
		ext = leaf + leafLen;
	}
	baseLen = (size_t)(ext - leaf);

	if ( copyIndex == 0 ) {
		_snwprintf_s(outName, cchOutName, _TRUNCATE, L"%.*s - Copy%s", (int)baseLen, leaf, ext);
	} else {
		_snwprintf_s(outName, cchOutName, _TRUNCATE, L"%.*s - Copy %u%s", (int)baseLen, leaf, (unsigned)(copyIndex + 1), ext);
	}
}

static BOOL GuiBuildRenameTargetPath(GuiApp* app, const WCHAR* inputValue, WCHAR* targetPath, size_t cchTargetPath)
{
	WCHAR trimmed[XPKGUI_ITEM_TEXT];

	if ( inputValue == NULL ) {
		return FALSE;
	}

	wcsncpy_s(trimmed, _countof(trimmed), inputValue, _TRUNCATE);
	StrTrimW(trimmed, L" \t");
	if ( trimmed[0] == L'\0' ) {
		return FALSE;
	}

	if ( app->currentFolder[0] != L'\0' && wcschr(trimmed, L'/') == NULL && wcschr(trimmed, L'\\') == NULL ) {
		GuiBuildChildViewPath(app->currentFolder, trimmed, targetPath, cchTargetPath);
	} else {
		wcsncpy_s(targetPath, cchTargetPath, trimmed, _TRUNCATE);
	}
	GuiNormalizeViewPath(targetPath, targetPath, cchTargetPath);
	return targetPath[0] != L'\0';
}

static BOOL GuiBuildMoveTargetFolder(GuiApp* app, const WCHAR* inputValue, WCHAR* targetPath, size_t cchTargetPath)
{
	WCHAR trimmed[XPKGUI_ITEM_TEXT];

	if ( targetPath == NULL || cchTargetPath == 0 ) {
		return FALSE;
	}
	targetPath[0] = L'\0';
	if ( inputValue == NULL ) {
		return FALSE;
	}

	wcsncpy_s(trimmed, _countof(trimmed), inputValue, _TRUNCATE);
	StrTrimW(trimmed, L" \t");
	if ( trimmed[0] == L'\0' || wcscmp(trimmed, L".") == 0 || wcscmp(trimmed, L"/") == 0 || wcscmp(trimmed, L"\\") == 0 ) {
		return TRUE;
	}

	if ( app->currentFolder[0] != L'\0' && !app->flatView && wcschr(trimmed, L'/') == NULL && wcschr(trimmed, L'\\') == NULL ) {
		GuiBuildChildViewPath(app->currentFolder, trimmed, targetPath, cchTargetPath);
	} else {
		wcsncpy_s(targetPath, cchTargetPath, trimmed, _TRUNCATE);
	}
	GuiNormalizeViewPath(targetPath, targetPath, cchTargetPath);
	return TRUE;
}

static BOOL GuiSelectViewItemByFullPath(GuiApp* app, GuiViewItemKind kind, const WCHAR* fullPath)
{
	size_t i;

	if ( app == NULL || app->list == NULL || fullPath == NULL || fullPath[0] == L'\0' ) {
		return FALSE;
	}

	for ( i = 0; i < app->viewCount; ++i ) {
		if ( app->viewItems[i].kind != kind ) {
			continue;
		}
		if ( _wcsicmp(app->viewItems[i].fullPath, fullPath) != 0 ) {
			continue;
		}
		ListView_SetItemState(app->list, -1, 0, LVIS_SELECTED);
		ListView_SetItemState(app->list, (int)i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		ListView_SetSelectionMark(app->list, (int)i);
		ListView_EnsureVisible(app->list, (int)i, FALSE);
		GuiSetMenuState(app);
		GuiUpdateStatus(app);
		return TRUE;
	}
	return FALSE;
}

BOOL GuiArchiveAddEmptyEntry(GuiApp* app)
{
	xpkWriteOptions writeOpt;
	WCHAR targetPath[XPKGUI_ITEM_TEXT];
	WCHAR promptText[128];
	GuiInputDialogState dialogState;
	char utf8Path[XPK_PATH_BYTES];
	int64_t fileIndex;
	uint32_t pos;
	int iRet;

	if ( app == NULL || app->archive == NULL ) {
		return FALSE;
	}

	ZeroMemory(&writeOpt, sizeof(writeOpt));
	writeOpt.compLevel = 0xFFu;
	writeOpt.writePolicy = app->writePolicy;
	writeOpt.fileType = 0;

	if ( app->packType == XPK_PACK_INDEX ) {
		if ( !GuiPromptIndexAddOptions(app->window, L"添加空条目", GuiFindNextIndexSeed(app), &fileIndex) ) {
			return FALSE;
		}
		if ( xpkIndexFind(app->archive, fileIndex, NULL) == XPK_OK ) {
			MessageBoxW(app->window, L"该 fileIndex 已存在。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
			return FALSE;
		}
		if ( xpkIndexAddData(app->archive, fileIndex, NULL, 0, &writeOpt) != XPK_OK ) {
			GuiShowArchiveError(app, L"添加空条目");
			return FALSE;
		}
		if ( !GuiArchiveSave(app) ) {
			return FALSE;
		}
		_snwprintf_s(targetPath, _countof(targetPath), _TRUNCATE, L"%lld", (long long)fileIndex);
		GuiSelectViewItemByFullPath(app, GUI_VIEW_ITEM_FILE, targetPath);
		return TRUE;
	}

	if ( GuiIsPathPackType(app->packType) ) {
		ZeroMemory(&dialogState, sizeof(dialogState));
		dialogState.title = L"添加空条目";
		if ( app->currentFolder[0] != L'\0' && !app->flatView ) {
			dialogState.prompt = L"包内路径(相对当前目录):";
		} else {
			dialogState.prompt = L"包内路径:";
		}
		wcsncpy_s(dialogState.value, _countof(dialogState.value), L"new_file", _TRUNCATE);
		if ( !GuiRunInputDialog(app->window, &dialogState) ) {
			return FALSE;
		}
		if ( !GuiBuildRenameTargetPath(app, dialogState.value, targetPath, _countof(targetPath)) ) {
			MessageBoxW(app->window, L"请输入有效的包内路径。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
			return FALSE;
		}
		GuiPathToPackageUtf8(targetPath, utf8Path, sizeof(utf8Path));
		if ( xpkPathExists(app->archive, utf8Path) ) {
			MessageBoxW(app->window, L"该包内路径已存在。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
			return FALSE;
		}
		if ( xpkPathAddData(app->archive, utf8Path, NULL, 0, &writeOpt) != XPK_OK ) {
			GuiShowArchiveError(app, L"添加空条目");
			return FALSE;
		}
		if ( !GuiArchiveSave(app) ) {
			return FALSE;
		}
		GuiSelectViewItemByFullPath(app, GUI_VIEW_ITEM_FILE, targetPath);
		return TRUE;
	}

	pos = 0;
	iRet = xpkAddData(app->archive, NULL, 0, &writeOpt, &pos);
	if ( iRet != XPK_OK ) {
		GuiShowArchiveError(app, L"添加空条目");
		return FALSE;
	}
	if ( !GuiArchiveSave(app) ) {
		return FALSE;
	}
	_snwprintf_s(promptText, _countof(promptText), _TRUNCATE, L"Entry %u", pos);
	GuiSelectViewItemByFullPath(app, GUI_VIEW_ITEM_FILE, promptText);
	return TRUE;
}

BOOL GuiArchiveCreateTextEntry(GuiApp* app)
{
	xpkWriteOptions writeOpt;
	GuiTextEditorDialogState state;
	WCHAR targetPath[XPKGUI_ITEM_TEXT];
	WCHAR title[128];
	WCHAR prompt[512];
	WCHAR idText[64];
	WCHAR* textBuf;
	void* utf8Buf;
	uint32_t utf8Size;
	char utf8Path[XPK_PATH_BYTES];
	int64_t fileIndex;
	uint32_t pos;

	if ( app == NULL || app->archive == NULL ) {
		return FALSE;
	}

	targetPath[0] = L'\0';
	fileIndex = 0;
	pos = 0;
	if ( app->packType == XPK_PACK_INDEX ) {
		if ( !GuiPromptIndexAddOptions(app->window, L"创建文本条目", GuiFindNextIndexSeed(app), &fileIndex) ) {
			return FALSE;
		}
		if ( xpkIndexFind(app->archive, fileIndex, NULL) == XPK_OK ) {
			MessageBoxW(app->window, L"该 fileIndex 已存在。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
			return FALSE;
		}
		_snwprintf_s(idText, _countof(idText), _TRUNCATE, L"%lld", (long long)fileIndex);
		_snwprintf_s(title, _countof(title), _TRUNCATE, L"Create Text Entry - %s", idText);
		_snwprintf_s(prompt, _countof(prompt), _TRUNCATE, L"将创建 fileIndex=%s 的 UTF-8 文本条目。直接输入文本并确定即可写入归档。", idText);
	} else if ( GuiIsPathPackType(app->packType) ) {
		GuiInputDialogState pathDialog;

		ZeroMemory(&pathDialog, sizeof(pathDialog));
		pathDialog.title = L"创建文本条目";
		pathDialog.prompt = (app->currentFolder[0] != L'\0' && !app->flatView) ? L"包内路径(相对当前目录):" : L"包内路径:";
		wcsncpy_s(pathDialog.value, _countof(pathDialog.value), L"new_text.txt", _TRUNCATE);
		if ( !GuiRunInputDialog(app->window, &pathDialog) ) {
			return FALSE;
		}
		if ( !GuiBuildRenameTargetPath(app, pathDialog.value, targetPath, _countof(targetPath)) ) {
			MessageBoxW(app->window, L"请输入有效的包内路径。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
			return FALSE;
		}
		GuiPathToPackageUtf8(targetPath, utf8Path, sizeof(utf8Path));
		if ( xpkPathExists(app->archive, utf8Path) ) {
			MessageBoxW(app->window, L"该包内路径已存在。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
			return FALSE;
		}
		_snwprintf_s(title, _countof(title), _TRUNCATE, L"Create Text Entry - %s", targetPath);
		_snwprintf_s(prompt, _countof(prompt), _TRUNCATE, L"将创建包内路径 %s 的 UTF-8 文本条目。直接输入文本并确定即可写入归档。", targetPath);
	} else {
		wcsncpy_s(title, _countof(title), L"Create Text Entry", _TRUNCATE);
		wcsncpy_s(prompt, _countof(prompt), L"将创建新的 UTF-8 文本条目。直接输入文本并确定即可写入归档。", _TRUNCATE);
	}

	textBuf = GuiAllocEmptyWideText();
	if ( textBuf == NULL ) {
		MessageBoxW(app->window, L"内存不足，无法创建文本编辑器。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	ZeroMemory(&state, sizeof(state));
	state.title = title;
	state.prompt = prompt;
	state.text = textBuf;
	state.cchText = 1;
	state.readOnly = FALSE;
	if ( !GuiRunTextEditorDialog(app->window, &state) ) {
		free(state.text);
		return FALSE;
	}

	utf8Buf = NULL;
	utf8Size = 0;
	if ( !GuiEncodeMetaUtf8Text(state.text, &utf8Buf, &utf8Size) ) {
		free(state.text);
		MessageBoxW(app->window, L"无法把文本编码为 UTF-8。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}
	free(state.text);

	ZeroMemory(&writeOpt, sizeof(writeOpt));
	writeOpt.compLevel = 0xFFu;
	writeOpt.writePolicy = app->writePolicy;
	writeOpt.fileType = 2u;

	if ( app->packType == XPK_PACK_INDEX ) {
		if ( xpkIndexAddData(app->archive, fileIndex, utf8Buf, utf8Size, &writeOpt) != XPK_OK ) {
			free(utf8Buf);
			GuiShowArchiveError(app, L"创建文本条目");
			return FALSE;
		}
	} else if ( GuiIsPathPackType(app->packType) ) {
		if ( xpkPathAddData(app->archive, utf8Path, utf8Buf, utf8Size, &writeOpt) != XPK_OK ) {
			free(utf8Buf);
			GuiShowArchiveError(app, L"创建文本条目");
			return FALSE;
		}
	} else {
		if ( xpkAddData(app->archive, utf8Buf, utf8Size, &writeOpt, &pos) != XPK_OK ) {
			free(utf8Buf);
			GuiShowArchiveError(app, L"创建文本条目");
			return FALSE;
		}
	}
	free(utf8Buf);

	if ( !GuiArchiveSave(app) ) {
		return FALSE;
	}

	if ( app->packType == XPK_PACK_INDEX ) {
		_snwprintf_s(targetPath, _countof(targetPath), _TRUNCATE, L"%lld", (long long)fileIndex);
		GuiSelectViewItemByFullPath(app, GUI_VIEW_ITEM_FILE, targetPath);
	} else if ( GuiIsPathPackType(app->packType) ) {
		GuiSelectViewItemByFullPath(app, GUI_VIEW_ITEM_FILE, targetPath);
	} else {
		_snwprintf_s(targetPath, _countof(targetPath), _TRUNCATE, L"Entry %u", pos);
		GuiSelectViewItemByFullPath(app, GUI_VIEW_ITEM_FILE, targetPath);
	}
	return TRUE;
}

static BOOL GuiRenameOldSetContainsPath(const GuiRenamePathPair* pairs, int count, const WCHAR* path)
{
	int i;

	for ( i = 0; i < count; ++i ) {
		if ( _wcsicmp(pairs[i].oldPath, path) == 0 ) {
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL GuiValidateRenamePairs(GuiApp* app, const GuiRenamePathPair* pairs, int count)
{
	int i;
	int j;
	size_t k;

	for ( i = 0; i < count; ++i ) {
		if ( pairs[i].newPath[0] == L'\0' ) {
			MessageBoxW(app->window, L"目标路径不能为空。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
			return FALSE;
		}
		for ( j = i + 1; j < count; ++j ) {
			if ( _wcsicmp(pairs[i].newPath, pairs[j].newPath) == 0 ) {
				MessageBoxW(app->window, L"目标路径发生冲突。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
				return FALSE;
			}
		}
		for ( k = 0; k < app->itemCount; ++k ) {
			if ( _wcsicmp(app->items[k].name, pairs[i].newPath) == 0 && !GuiRenameOldSetContainsPath(pairs, count, app->items[k].name) ) {
				MessageBoxW(app->window, L"目标路径已存在。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
				return FALSE;
			}
		}
	}
	return TRUE;
}

static BOOL GuiValidateCopyPairs(GuiApp* app, const GuiRenamePathPair* pairs, int count)
{
	int i;
	int j;
	size_t k;

	for ( i = 0; i < count; ++i ) {
		if ( pairs[i].newPath[0] == L'\0' ) {
			MessageBoxW(app->window, L"目标路径不能为空。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
			return FALSE;
		}
		for ( j = i + 1; j < count; ++j ) {
			if ( _wcsicmp(pairs[i].newPath, pairs[j].newPath) == 0 ) {
				MessageBoxW(app->window, L"目标路径发生冲突。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
				return FALSE;
			}
		}
		for ( k = 0; k < app->itemCount; ++k ) {
			if ( _wcsicmp(app->items[k].name, pairs[i].newPath) == 0 ) {
				MessageBoxW(app->window, L"目标路径已存在。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
				return FALSE;
			}
		}
	}
	return TRUE;
}

static const WCHAR* GuiPathLeafName(const WCHAR* path)
{
	const WCHAR* leaf;
	const WCHAR* p;

	if ( path == NULL ) {
		return L"";
	}
	leaf = path;
	for ( p = path; *p != L'\0'; ++p ) {
		if ( *p == L'/' || *p == L'\\' ) {
			leaf = p + 1;
		}
	}
	return leaf;
}

static BOOL GuiAppendRenamePair(GuiApp* app, GuiRenamePathPair** pairs, int* count, int* capacity, const WCHAR* oldPath, const WCHAR* newPath)
{
	GuiRenamePathPair* newPairs;
	int newCapacity;

	if ( pairs == NULL || count == NULL || capacity == NULL || oldPath == NULL || newPath == NULL || oldPath[0] == L'\0' || newPath[0] == L'\0' ) {
		return FALSE;
	}
	if ( *count >= *capacity ) {
		newCapacity = (*capacity == 0) ? 32 : (*capacity * 2);
		newPairs = (GuiRenamePathPair*)realloc(*pairs, (size_t)newCapacity * sizeof(**pairs));
		if ( newPairs == NULL ) {
			MessageBoxW(app != NULL ? app->window : NULL, L"内存不足。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
			return FALSE;
		}
		*pairs = newPairs;
		*capacity = newCapacity;
	}
	wcsncpy_s((*pairs)[*count].oldPath, _countof((*pairs)[*count].oldPath), oldPath, _TRUNCATE);
	wcsncpy_s((*pairs)[*count].newPath, _countof((*pairs)[*count].newPath), newPath, _TRUNCATE);
	(*pairs)[*count].oldLen = wcslen((*pairs)[*count].oldPath);
	(*count)++;
	return TRUE;
}

static int __cdecl GuiRenamePairCompare(void* context, const void* leftPtr, const void* rightPtr)
{
	BOOL descending;
	const GuiRenamePathPair* left;
	const GuiRenamePathPair* right;

	descending = *(BOOL*)context;
	left = (const GuiRenamePathPair*)leftPtr;
	right = (const GuiRenamePathPair*)rightPtr;

	if ( left->oldLen < right->oldLen ) return descending ? 1 : -1;
	if ( left->oldLen > right->oldLen ) return descending ? -1 : 1;
	return _wcsicmp(left->oldPath, right->oldPath);
}

static BOOL GuiBuildDirectoryRenamePairs(GuiApp* app, const WCHAR* oldPrefix, const WCHAR* newPrefix, GuiRenamePathPair** pairsOut, int* countOut)
{
	GuiRenamePathPair* pairs;
	int count;
	size_t i;
	BOOL renameIntoDescendant;

	*pairsOut = NULL;
	*countOut = 0;
	count = 0;
	for ( i = 0; i < app->itemCount; ++i ) {
		if ( _wcsicmp(app->items[i].name, oldPrefix) == 0 || GuiPathMatchFolderPrefix(app->items[i].name, oldPrefix, NULL) ) {
			count++;
		}
	}
	if ( count <= 0 ) {
		return FALSE;
	}

	pairs = (GuiRenamePathPair*)calloc((size_t)count, sizeof(GuiRenamePathPair));
	if ( pairs == NULL ) {
		MessageBoxW(app->window, L"内存不足。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	count = 0;
	for ( i = 0; i < app->itemCount; ++i ) {
		const WCHAR* suffix;

		if ( _wcsicmp(app->items[i].name, oldPrefix) != 0 && !GuiPathMatchFolderPrefix(app->items[i].name, oldPrefix, NULL) ) {
			continue;
		}
		wcsncpy_s(pairs[count].oldPath, _countof(pairs[count].oldPath), app->items[i].name, _TRUNCATE);
		pairs[count].oldLen = wcslen(pairs[count].oldPath);
		suffix = app->items[i].name + wcslen(oldPrefix);
		if ( suffix[0] == L'/' || suffix[0] == L'\\' ) {
			_snwprintf_s(pairs[count].newPath, _countof(pairs[count].newPath), _TRUNCATE, L"%s%s", newPrefix, suffix);
		} else {
			wcsncpy_s(pairs[count].newPath, _countof(pairs[count].newPath), newPrefix, _TRUNCATE);
		}
		count++;
	}

	if ( !GuiValidateRenamePairs(app, pairs, count) ) {
		free(pairs);
		return FALSE;
	}

	renameIntoDescendant = GuiPathMatchFolderPrefix(newPrefix, oldPrefix, NULL);
	qsort_s(pairs, (size_t)count, sizeof(GuiRenamePathPair), GuiRenamePairCompare, &renameIntoDescendant);

	*pairsOut = pairs;
	*countOut = count;
	return TRUE;
}

static BOOL GuiApplyRenamePairs(GuiApp* app, const GuiRenamePathPair* pairs, int count, const WCHAR* actionText)
{
	int i;

	for ( i = 0; i < count; ++i ) {
		char oldUtf8[XPK_PATH_BYTES];
		char newUtf8[XPK_PATH_BYTES];

		if ( _wcsicmp(pairs[i].oldPath, pairs[i].newPath) == 0 ) {
			continue;
		}
		if ( !GuiUtf8FromWide(pairs[i].oldPath, oldUtf8, sizeof(oldUtf8)) || !GuiUtf8FromWide(pairs[i].newPath, newUtf8, sizeof(newUtf8)) ) {
			MessageBoxW(app->window, L"路径转换失败。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
			GuiArchiveOpenPath(app, app->archivePath, FALSE);
			return FALSE;
		}
		if ( xpkPathRename(app->archive, oldUtf8, newUtf8) != XPK_OK ) {
			GuiShowArchiveError(app, actionText);
			GuiArchiveOpenPath(app, app->archivePath, FALSE);
			return FALSE;
		}
	}
	return TRUE;
}

BOOL GuiArchiveRemoveSelection(GuiApp* app)
{
	GuiArchiveItem* items;
	int count;
	int i;

	if ( app->archive == NULL || !GuiArchiveCanDeleteSelection(app) ) {
		return FALSE;
	}

	if ( MessageBoxW(app->window, L"确认删除选中的条目？", XPKGUI_APP_TITLE, MB_YESNO | MB_ICONQUESTION) != IDYES ) {
		return FALSE;
	}

	items = GuiDuplicateExtractItems(app, FALSE, &count);
	if ( items == NULL || count <= 0 ) {
		free(items);
		MessageBoxW(app->window, L"请选择文件或目录进行删除。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	qsort(items, (size_t)count, sizeof(GuiArchiveItem), GuiArchiveItemPosDescCompare);
	for ( i = 0; i < count; ++i ) {
		if ( xpkRemove(app->archive, items[i].pos) != XPK_OK ) {
			free(items);
			GuiShowArchiveError(app, L"删除条目");
			return FALSE;
		}
	}
	free(items);

	return GuiArchiveSave(app);
}

BOOL GuiArchiveRenameSelection(GuiApp* app)
{
	GuiArchiveItem* item;
	GuiViewItem* viewItem;
	GuiInputDialogState dialogState;
	WCHAR targetPath[XPKGUI_ITEM_TEXT];
	WCHAR editName[XPKGUI_ITEM_TEXT];

	if ( app->archive == NULL || !GuiIsPathPackType(app->packType) ) {
		return FALSE;
	}

	viewItem = GuiArchiveGetSingleSelectedViewItem(app);
	item = GuiArchiveGetSingleSelectedItem(app);
	if ( viewItem == NULL ) {
		MessageBoxW(app->window, L"请选择单个条目进行重命名。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	ZeroMemory(&dialogState, sizeof(dialogState));
	dialogState.title = (viewItem->kind == GUI_VIEW_ITEM_DIR) ? L"重命名目录" : L"重命名包内路径";
	dialogState.prompt = L"新路径:";
	GuiTrimPathNameForEdit(viewItem->name, editName, _countof(editName));
	wcsncpy_s(dialogState.value, _countof(dialogState.value), editName, _TRUNCATE);
	if ( !GuiRunInputDialog(app->window, &dialogState) ) {
		return FALSE;
	}

	if ( !GuiBuildRenameTargetPath(app, dialogState.value, targetPath, _countof(targetPath)) ) {
		MessageBoxW(app->window, L"请输入有效的新路径。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	if ( viewItem->kind == GUI_VIEW_ITEM_DIR ) {
		GuiRenamePathPair* pairs;
		int count;

		if ( _wcsicmp(viewItem->fullPath, targetPath) == 0 ) {
			return TRUE;
		}
		if ( !GuiBuildDirectoryRenamePairs(app, viewItem->fullPath, targetPath, &pairs, &count) ) {
			return FALSE;
		}
		if ( !GuiApplyRenamePairs(app, pairs, count, L"重命名目录") ) {
			free(pairs);
			return FALSE;
		}
		free(pairs);
	} else {
		GuiRenamePathPair pair;

		if ( item == NULL ) {
			MessageBoxW(app->window, L"请选择单个文件进行重命名。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
			return FALSE;
		}
		if ( _wcsicmp(item->name, targetPath) == 0 ) {
			return TRUE;
		}
		ZeroMemory(&pair, sizeof(pair));
		wcsncpy_s(pair.oldPath, _countof(pair.oldPath), item->name, _TRUNCATE);
		wcsncpy_s(pair.newPath, _countof(pair.newPath), targetPath, _TRUNCATE);
		pair.oldLen = wcslen(pair.oldPath);
		if ( !GuiValidateRenamePairs(app, &pair, 1) ) {
			return FALSE;
		}
		if ( !GuiApplyRenamePairs(app, &pair, 1, L"重命名条目") ) {
			return FALSE;
		}
	}

	return GuiArchiveSave(app);
}

static BOOL GuiCollectSelectedMoveViewItems(GuiApp* app, GuiViewItem** itemsOut, int* countOut)
{
	GuiViewItem* items;
	int count;
	int capacity;
	int index;

	*itemsOut = NULL;
	*countOut = 0;
	if ( app == NULL || app->list == NULL ) {
		return TRUE;
	}

	items = NULL;
	count = 0;
	capacity = 0;
	index = -1;
	while ( TRUE ) {
		GuiViewItem* viewItem;

		index = ListView_GetNextItem(app->list, index, LVNI_SELECTED);
		if ( index < 0 ) {
			break;
		}
		viewItem = GuiArchiveGetViewItemByListIndex(app, index);
		if ( viewItem == NULL || (viewItem->kind != GUI_VIEW_ITEM_FILE && viewItem->kind != GUI_VIEW_ITEM_DIR) ) {
			continue;
		}
		if ( count >= capacity ) {
			GuiViewItem* newItems;
			int newCapacity;

			newCapacity = (capacity == 0) ? 16 : (capacity * 2);
			newItems = (GuiViewItem*)realloc(items, (size_t)newCapacity * sizeof(*items));
			if ( newItems == NULL ) {
				free(items);
				MessageBoxW(app->window, L"内存不足。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
				return FALSE;
			}
			items = newItems;
			capacity = newCapacity;
		}
		items[count++] = *viewItem;
	}

	*itemsOut = items;
	*countOut = count;
	return TRUE;
}

static BOOL GuiMoveSelectionItemCoveredByDirectory(const GuiViewItem* items, int count, int itemIndex)
{
	int i;

	if ( items == NULL || itemIndex < 0 || itemIndex >= count ) {
		return FALSE;
	}
	for ( i = 0; i < count; ++i ) {
		if ( i == itemIndex || items[i].kind != GUI_VIEW_ITEM_DIR ) {
			continue;
		}
		if ( _wcsicmp(items[itemIndex].fullPath, items[i].fullPath) == 0 || GuiPathMatchFolderPrefix(items[itemIndex].fullPath, items[i].fullPath, NULL) ) {
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL GuiAppendMovePairsForSelectionItem(GuiApp* app, const GuiViewItem* viewItem, const WCHAR* targetFolder, GuiRenamePathPair** pairs, int* count, int* capacity)
{
	WCHAR targetBase[XPKGUI_ITEM_TEXT];
	const WCHAR* leaf;
	size_t i;

	if ( app == NULL || viewItem == NULL || targetFolder == NULL ) {
		return FALSE;
	}
	leaf = GuiPathLeafName(viewItem->fullPath);
	if ( leaf[0] == L'\0' ) {
		return FALSE;
	}
	GuiBuildChildViewPath(targetFolder, leaf, targetBase, _countof(targetBase));
	GuiNormalizeViewPath(targetBase, targetBase, _countof(targetBase));
	if ( targetBase[0] == L'\0' ) {
		return FALSE;
	}

	if ( viewItem->kind == GUI_VIEW_ITEM_FILE ) {
		return GuiAppendRenamePair(app, pairs, count, capacity, viewItem->fullPath, targetBase);
	}
	if ( viewItem->kind != GUI_VIEW_ITEM_DIR ) {
		return TRUE;
	}

	for ( i = 0; i < app->itemCount; ++i ) {
		const WCHAR* suffix;
		WCHAR newPath[XPKGUI_ITEM_TEXT];

		if ( _wcsicmp(app->items[i].name, viewItem->fullPath) != 0 && !GuiPathMatchFolderPrefix(app->items[i].name, viewItem->fullPath, NULL) ) {
			continue;
		}
		suffix = app->items[i].name + wcslen(viewItem->fullPath);
		if ( suffix[0] == L'/' || suffix[0] == L'\\' ) {
			_snwprintf_s(newPath, _countof(newPath), _TRUNCATE, L"%s%s", targetBase, suffix);
		} else {
			wcsncpy_s(newPath, _countof(newPath), targetBase, _TRUNCATE);
		}
		if ( !GuiAppendRenamePair(app, pairs, count, capacity, app->items[i].name, newPath) ) {
			return FALSE;
		}
	}
	return TRUE;
}

BOOL GuiArchiveMoveSelection(GuiApp* app)
{
	GuiInputDialogState dialogState;
	GuiViewItem* selectedItems;
	GuiRenamePathPair* pairs;
	WCHAR targetFolder[XPKGUI_ITEM_TEXT];
	int selectedCount;
	int pairCount;
	int pairCapacity;
	int i;
	int changedCount;
	BOOL sortDescending;

	if ( app == NULL || app->archive == NULL || !GuiArchiveCanMoveSelection(app) ) {
		return FALSE;
	}

	ZeroMemory(&dialogState, sizeof(dialogState));
	dialogState.title = L"移动到目录";
	dialogState.prompt = (app->currentFolder[0] != L'\0' && !app->flatView) ? L"目标目录(相对当前目录，留空表示根目录):" : L"目标目录(留空表示根目录):";
	if ( app->currentFolder[0] != L'\0' && !app->flatView ) {
		wcsncpy_s(dialogState.value, _countof(dialogState.value), app->currentFolder, _TRUNCATE);
	}
	if ( !GuiRunInputDialog(app->window, &dialogState) ) {
		return FALSE;
	}
	if ( !GuiBuildMoveTargetFolder(app, dialogState.value, targetFolder, _countof(targetFolder)) ) {
		MessageBoxW(app->window, L"请输入有效的目标目录。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	selectedItems = NULL;
	selectedCount = 0;
	if ( !GuiCollectSelectedMoveViewItems(app, &selectedItems, &selectedCount) || selectedCount <= 0 ) {
		free(selectedItems);
		MessageBoxW(app->window, L"请选择文件或目录进行移动。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	pairs = NULL;
	pairCount = 0;
	pairCapacity = 0;
	for ( i = 0; i < selectedCount; ++i ) {
		if ( GuiMoveSelectionItemCoveredByDirectory(selectedItems, selectedCount, i) ) {
			continue;
		}
		if ( !GuiAppendMovePairsForSelectionItem(app, &selectedItems[i], targetFolder, &pairs, &pairCount, &pairCapacity) ) {
			free(selectedItems);
			free(pairs);
			return FALSE;
		}
	}
	free(selectedItems);

	if ( pairCount <= 0 ) {
		free(pairs);
		MessageBoxW(app->window, L"没有可移动的条目。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}
	if ( !GuiValidateRenamePairs(app, pairs, pairCount) ) {
		free(pairs);
		return FALSE;
	}

	changedCount = 0;
	for ( i = 0; i < pairCount; ++i ) {
		if ( _wcsicmp(pairs[i].oldPath, pairs[i].newPath) != 0 ) {
			changedCount++;
		}
	}
	if ( changedCount <= 0 ) {
		free(pairs);
		MessageBoxW(app->window, L"选中项已经位于目标目录。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	sortDescending = TRUE;
	qsort_s(pairs, (size_t)pairCount, sizeof(*pairs), GuiRenamePairCompare, &sortDescending);
	if ( !GuiApplyRenamePairs(app, pairs, pairCount, L"移动条目") ) {
		free(pairs);
		return FALSE;
	}
	free(pairs);

	if ( !GuiArchiveSave(app) ) {
		return FALSE;
	}
	if ( targetFolder[0] == L'\0' ) {
		GuiArchiveBrowseRoot(app);
	} else {
		GuiArchiveNavigateToFolder(app, targetFolder);
	}
	return TRUE;
}

static const GuiArchiveItem* GuiFindArchiveItemByPath(GuiApp* app, const WCHAR* path)
{
	size_t i;

	if ( app == NULL || path == NULL || path[0] == L'\0' ) {
		return NULL;
	}
	for ( i = 0; i < app->itemCount; ++i ) {
		if ( _wcsicmp(app->items[i].name, path) == 0 ) {
			return &app->items[i];
		}
	}
	return NULL;
}

static BOOL GuiApplyCopyPairs(GuiApp* app, const GuiRenamePathPair* pairs, int count)
{
	int i;

	for ( i = 0; i < count; ++i ) {
		const GuiArchiveItem* sourceItem;
		xpkWriteOptions writeOpt;
		void* data;
		uint64_t dataSize;
		char oldUtf8[XPK_PATH_BYTES];
		char newUtf8[XPK_PATH_BYTES];

		sourceItem = GuiFindArchiveItemByPath(app, pairs[i].oldPath);
		if ( sourceItem == NULL ) {
			MessageBoxW(app->window, L"找不到要复制的源条目。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
			return FALSE;
		}
		if ( !GuiUtf8FromWide(pairs[i].oldPath, oldUtf8, sizeof(oldUtf8)) || !GuiUtf8FromWide(pairs[i].newPath, newUtf8, sizeof(newUtf8)) ) {
			MessageBoxW(app->window, L"路径转换失败。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
			return FALSE;
		}

		dataSize = 0;
		data = xpkPathReadToMemory(app->archive, oldUtf8, &dataSize);
		if ( data == NULL && dataSize > 0 ) {
			GuiShowArchiveError(app, L"读取源条目");
			return FALSE;
		}

		ZeroMemory(&writeOpt, sizeof(writeOpt));
		writeOpt.compLevel = (uint8_t)(sourceItem->flag & XPK_FLAG_COMP_MASK);
		writeOpt.writePolicy = app->writePolicy;
		writeOpt.fileType = GuiEntryFileType(sourceItem->flag);
		if ( xpkPathAddData(app->archive, newUtf8, data, dataSize, &writeOpt) != XPK_OK ) {
			if ( data != NULL ) {
				xpkFree(data);
			}
			GuiShowArchiveError(app, L"复制条目");
			return FALSE;
		}
		if ( data != NULL ) {
			xpkFree(data);
		}
		if ( sourceItem->attr != 0 && xpkPathSetAttr(app->archive, newUtf8, sourceItem->attr) != XPK_OK ) {
			GuiShowArchiveError(app, L"复制 platformAttr");
			return FALSE;
		}
	}
	return TRUE;
}

BOOL GuiArchiveCopyToSelection(GuiApp* app)
{
	GuiInputDialogState dialogState;
	GuiViewItem* selectedItems;
	GuiRenamePathPair* pairs;
	WCHAR targetFolder[XPKGUI_ITEM_TEXT];
	int selectedCount;
	int pairCount;
	int pairCapacity;
	int i;

	if ( app == NULL || app->archive == NULL || !GuiArchiveCanCopyToSelection(app) ) {
		return FALSE;
	}

	ZeroMemory(&dialogState, sizeof(dialogState));
	dialogState.title = L"复制到目录";
	dialogState.prompt = (app->currentFolder[0] != L'\0' && !app->flatView) ? L"目标目录(相对当前目录，留空表示根目录):" : L"目标目录(留空表示根目录):";
	if ( app->currentFolder[0] != L'\0' && !app->flatView ) {
		wcsncpy_s(dialogState.value, _countof(dialogState.value), app->currentFolder, _TRUNCATE);
	}
	if ( !GuiRunInputDialog(app->window, &dialogState) ) {
		return FALSE;
	}
	if ( !GuiBuildMoveTargetFolder(app, dialogState.value, targetFolder, _countof(targetFolder)) ) {
		MessageBoxW(app->window, L"请输入有效的目标目录。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	selectedItems = NULL;
	selectedCount = 0;
	if ( !GuiCollectSelectedMoveViewItems(app, &selectedItems, &selectedCount) || selectedCount <= 0 ) {
		free(selectedItems);
		MessageBoxW(app->window, L"请选择文件或目录进行复制。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	pairs = NULL;
	pairCount = 0;
	pairCapacity = 0;
	for ( i = 0; i < selectedCount; ++i ) {
		if ( GuiMoveSelectionItemCoveredByDirectory(selectedItems, selectedCount, i) ) {
			continue;
		}
		if ( !GuiAppendMovePairsForSelectionItem(app, &selectedItems[i], targetFolder, &pairs, &pairCount, &pairCapacity) ) {
			free(selectedItems);
			free(pairs);
			return FALSE;
		}
	}
	free(selectedItems);

	if ( pairCount <= 0 ) {
		free(pairs);
		MessageBoxW(app->window, L"没有可复制的条目。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}
	if ( !GuiValidateCopyPairs(app, pairs, pairCount) ) {
		free(pairs);
		return FALSE;
	}
	if ( !GuiApplyCopyPairs(app, pairs, pairCount) ) {
		free(pairs);
		GuiArchiveOpenPath(app, app->archivePath, FALSE);
		return FALSE;
	}
	free(pairs);

	if ( !GuiArchiveSave(app) ) {
		return FALSE;
	}
	if ( targetFolder[0] == L'\0' ) {
		GuiArchiveBrowseRoot(app);
	} else {
		GuiArchiveNavigateToFolder(app, targetFolder);
	}
	return TRUE;
}

BOOL GuiArchiveSetSelectionFileIndex(GuiApp* app)
{
	GuiArchiveItem* item;
	xpkWriteOptions writeOpt;
	WCHAR tempRoot[XPKGUI_MAX_TEMP_PATH];
	WCHAR sourcePath[XPKGUI_MAX_TEMP_PATH];
	char utf8Source[XPK_PATH_BYTES];
	int64_t newFileIndex;
	WCHAR targetPath[XPKGUI_ITEM_TEXT];

	if ( app == NULL || app->archive == NULL ) {
		return FALSE;
	}
	if ( !GuiArchiveCanSetSelectionFileIndex(app) ) {
		MessageBoxW(app != NULL ? app->window : NULL, L"请选择 Index 包中的单个文件设置 fileIndex。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	item = GuiArchiveGetSingleSelectedItem(app);
	if ( item == NULL ) {
		return FALSE;
	}
	if ( !GuiPromptIndexAddOptions(app->window, L"设置 FileIndex", item->fileIndex, &newFileIndex) ) {
		return FALSE;
	}
	if ( newFileIndex == item->fileIndex ) {
		return TRUE;
	}
	if ( xpkIndexFind(app->archive, newFileIndex, NULL) == XPK_OK ) {
		MessageBoxW(app->window, L"该 fileIndex 已存在。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}
	if ( !GuiArchiveExtractSingleSelectionToTemp(app, L"调整 FileIndex", tempRoot, _countof(tempRoot), sourcePath, _countof(sourcePath)) ) {
		return FALSE;
	}
	if ( !GuiUtf8FromWide(sourcePath, utf8Source, sizeof(utf8Source)) ) {
		MessageBoxW(app->window, L"临时文件路径转换失败。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	ZeroMemory(&writeOpt, sizeof(writeOpt));
	writeOpt.compLevel = 0xFFu;
	writeOpt.writePolicy = app->writePolicy;
	writeOpt.fileType = GuiEntryFileType(item->flag);

	if ( xpkIndexAddFile(app->archive, newFileIndex, utf8Source, &writeOpt) != XPK_OK ) {
		GuiShowArchiveError(app, L"设置 FileIndex");
		return FALSE;
	}
	if ( xpkIndexRemove(app->archive, item->fileIndex) != XPK_OK ) {
		(void)xpkIndexRemove(app->archive, newFileIndex);
		GuiShowArchiveError(app, L"设置 FileIndex");
		GuiArchiveOpenPath(app, app->archivePath, FALSE);
		return FALSE;
	}
	if ( !GuiArchiveSave(app) ) {
		return FALSE;
	}

	_snwprintf_s(targetPath, _countof(targetPath), _TRUNCATE, L"%lld", (long long)newFileIndex);
	GuiSelectViewItemByFullPath(app, GUI_VIEW_ITEM_FILE, targetPath);
	return TRUE;
}

BOOL GuiArchiveVerify(GuiApp* app)
{
	GuiTaskState task;
	GuiTaskResult result;

	if ( app->archive == NULL ) {
		return FALSE;
	}

	ZeroMemory(&task, sizeof(task));
	task.kind = GUI_TASK_VERIFY;
	task.app = app;
	task.canCancel = FALSE;
	task.marquee = TRUE;
	task.writePolicy = XPK_WRITE_BUFFERED;
	wcsncpy_s(task.title, _countof(task.title), L"校验归档", _TRUNCATE);
	wcsncpy_s(task.archivePath, _countof(task.archivePath), app->archivePath, _TRUNCATE);
	result = GuiRunTaskDialog(&task);
	if ( result == GUI_TASK_RESULT_SUCCESS ) {
		MessageBoxW(app->window, L"校验通过。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return TRUE;
	}
	return FALSE;
}

BOOL GuiArchiveVerifySelection(GuiApp* app)
{
	GuiTaskState task;
	GuiTaskResult result;

	if ( app == NULL || app->archive == NULL ) {
		return FALSE;
	}
	if ( !GuiArchiveCanVerifySelection(app) ) {
		MessageBoxW(app->window, L"请选择文件或目录进行校验。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	ZeroMemory(&task, sizeof(task));
	task.kind = GUI_TASK_VERIFY;
	task.app = app;
	task.canCancel = TRUE;
	task.marquee = FALSE;
	task.writePolicy = XPK_WRITE_BUFFERED;
	wcsncpy_s(task.title, _countof(task.title), L"校验条目", _TRUNCATE);
	wcsncpy_s(task.archivePath, _countof(task.archivePath), app->archivePath, _TRUNCATE);
	task.extractItems = GuiDuplicateExtractItems(app, FALSE, &task.extractCount);
	if ( task.extractItems == NULL || task.extractCount <= 0 ) {
		free(task.extractItems);
		MessageBoxW(app->window, L"请选择文件或目录进行校验。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}
	task.progressTotal = task.extractCount;

	result = GuiRunTaskDialog(&task);
	free(task.extractItems);
	if ( result == GUI_TASK_RESULT_SUCCESS ) {
		MessageBoxW(app->window, L"选中条目校验通过。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return TRUE;
	}
	return FALSE;
}

BOOL GuiArchiveVerifyPath(const WCHAR* archivePath)
{
	BOOL cancelled;

	cancelled = FALSE;
	return GuiArchiveVerifyPathTask(archivePath, &cancelled);
}

BOOL GuiArchiveVerifyPathTask(const WCHAR* archivePath, BOOL* cancelledOut)
{
	GuiApp tempApp;
	GuiTaskState task;
	GuiTaskResult result;

	ZeroMemory(&tempApp, sizeof(tempApp));
	GuiAppInitDefaults(&tempApp);
	if ( cancelledOut != NULL ) {
		*cancelledOut = FALSE;
	}
	if ( !GuiArchiveOpenPath(&tempApp, archivePath, TRUE) ) {
		return FALSE;
	}

	ZeroMemory(&task, sizeof(task));
	task.kind = GUI_TASK_VERIFY;
	task.app = &tempApp;
	task.canCancel = FALSE;
	task.marquee = TRUE;
	task.writePolicy = XPK_WRITE_BUFFERED;
	wcsncpy_s(task.title, _countof(task.title), L"校验归档", _TRUNCATE);
	wcsncpy_s(task.archivePath, _countof(task.archivePath), archivePath, _TRUNCATE);
	result = GuiRunTaskDialog(&task);
	GuiAppCloseArchive(&tempApp);
	if ( result == GUI_TASK_RESULT_CANCELLED ) {
		if ( cancelledOut != NULL ) {
			*cancelledOut = TRUE;
		}
		return FALSE;
	}
	return result == GUI_TASK_RESULT_SUCCESS;
}

BOOL GuiArchiveBuild(GuiApp* app)
{
	GuiTaskState task;
	GuiTaskResult result;
	WCHAR reopenPath[MAX_PATH];

	if ( app->archive == NULL ) {
		return FALSE;
	}

	ZeroMemory(&task, sizeof(task));
	task.kind = GUI_TASK_BUILD;
	task.app = app;
	task.canCancel = FALSE;
	task.marquee = TRUE;
	task.writePolicy = app->writePolicy;
	wcsncpy_s(task.title, _countof(task.title), L"重构归档", _TRUNCATE);
	wcsncpy_s(task.archivePath, _countof(task.archivePath), app->archivePath, _TRUNCATE);
	wcsncpy_s(reopenPath, _countof(reopenPath), app->archivePath, _TRUNCATE);
	result = GuiRunTaskDialog(&task);
	if ( reopenPath[0] != L'\0' ) {
		GuiArchiveOpenPath(app, reopenPath, FALSE);
	}
	return result == GUI_TASK_RESULT_SUCCESS;
}

typedef struct GuiArchivePropertyTotals {
	xpkPackType packType;
	uint64_t totalSize;
	uint64_t totalPacked;
} GuiArchivePropertyTotals;

static int GuiArchivePropertyTotalsCallback(xpkObject xpk, uint32_t pos, const void* info, void* userData)
{
	GuiArchivePropertyTotals* totals;

	(void)xpk;
	(void)pos;

	totals = (GuiArchivePropertyTotals*)userData;
	if ( totals == NULL || info == NULL ) {
		return XPK_ERR_PARAM;
	}

	if ( totals->packType == XPK_PACK_INDEX ) {
		const xpkFileInfoIndex* pInfo;

		pInfo = (const xpkFileInfoIndex*)info;
		totals->totalSize += pInfo->fileSize;
		totals->totalPacked += pInfo->dataSize;
	} else if ( totals->packType == XPK_PACK_LINUX || totals->packType == XPK_PACK_WIN32 ) {
		const xpkFileInfoPath* pInfo;

		pInfo = (const xpkFileInfoPath*)info;
		totals->totalSize += pInfo->fileSize;
		totals->totalPacked += pInfo->dataSize;
	} else {
		const xpkFileInfo* pInfo;

		pInfo = (const xpkFileInfo*)info;
		totals->totalSize += pInfo->fileSize;
		totals->totalPacked += pInfo->dataSize;
	}
	return XPK_OK;
}

static uint64_t GuiQueryFileSizeOrZero(const WCHAR* path)
{
	WIN32_FILE_ATTRIBUTE_DATA attrData;

	if ( path == NULL || path[0] == L'\0' || !GetFileAttributesExW(path, GetFileExInfoStandard, &attrData) ) {
		return 0;
	}
	return (((uint64_t)attrData.nFileSizeHigh) << 32) | (uint64_t)attrData.nFileSizeLow;
}

static void GuiFormatPercent(uint64_t value, uint64_t total, WCHAR* buf, size_t cchBuf)
{
	double percent;

	if ( buf == NULL || cchBuf == 0 ) {
		return;
	}
	if ( total == 0 ) {
		wcsncpy_s(buf, cchBuf, L"-", _TRUNCATE);
		return;
	}
	percent = ((double)value * 100.0) / (double)total;
	_snwprintf_s(buf, cchBuf, _TRUNCATE, L"%.1f%%", percent);
}

static BOOL GuiShowPropertiesCore(const WCHAR* archivePath, xpkObject archive, xpkPackType packType, uint8_t defComp, uint8_t metaComp, uint8_t infoComp, uint32_t infoExtSize, uint32_t volumeSize, BOOL solidMode)
{
	xpkStat statInfo;
	GuiArchivePropertyTotals totals;
	uint64_t physicalSize;
	uint64_t headerTableBytes;
	WCHAR totalSizeText[64];
	WCHAR packedText[64];
	WCHAR physicalText[64];
	WCHAR liveText[64];
	WCHAR holesText[64];
	WCHAR metaText[64];
	WCHAR tableText[64];
	WCHAR headerTableText[64];
	WCHAR ratioText[32];
	WCHAR holePercentText[32];
	WCHAR message[3072];

	if ( xpkStatGet(archive, &statInfo) != XPK_OK ) {
		return FALSE;
	}
	ZeroMemory(&totals, sizeof(totals));
	totals.packType = packType;
	if ( xpkEach(archive, GuiArchivePropertyTotalsCallback, &totals) != XPK_OK ) {
		return FALSE;
	}

	physicalSize = GuiQueryFileSizeOrZero(archivePath);
	headerTableBytes = XPK_HEAD_SIZE + statInfo.metaBytes + statInfo.entryTableBytes;
	GuiFormatUInt64(totals.totalSize, totalSizeText, _countof(totalSizeText));
	GuiFormatUInt64(totals.totalPacked, packedText, _countof(packedText));
	GuiFormatUInt64(physicalSize, physicalText, _countof(physicalText));
	GuiFormatUInt64(statInfo.liveDataBytes, liveText, _countof(liveText));
	GuiFormatUInt64(statInfo.holeBytes, holesText, _countof(holesText));
	GuiFormatUInt64(statInfo.metaBytes, metaText, _countof(metaText));
	GuiFormatUInt64(statInfo.entryTableBytes, tableText, _countof(tableText));
	GuiFormatUInt64(headerTableBytes, headerTableText, _countof(headerTableText));
	GuiFormatRatio(totals.totalPacked, totals.totalSize, ratioText, _countof(ratioText));
	GuiFormatPercent(statInfo.holeBytes, statInfo.liveDataBytes + statInfo.holeBytes, holePercentText, _countof(holePercentText));

	_snwprintf_s(
		message,
		_countof(message),
		_TRUNCATE,
		L"Path: %s\n"
		L"Pack Type: %s\n"
		L"Default Comp: %u\n"
		L"Meta Comp: %u\n"
		L"Info Comp: %u\n"
		L"InfoExt Size: %u\n"
		L"Solid Mode: %s\n"
		L"Volume Size: %u\n"
		L"\n"
		L"Visible Entries: %u\n"
		L"Total Size: %s\n"
		L"Total Packed: %s\n"
		L"Compression Ratio: %s\n"
		L"Physical Size: %s\n"
		L"\n"
		L"Live Data: %s\n"
		L"Holes: %s (%s)\n"
		L"Metadata: %s\n"
		L"Entry Table: %s\n"
		L"Header + Meta + Table: %s",
		archivePath,
		GuiPackTypeLabel(packType),
		(unsigned)defComp,
		(unsigned)metaComp,
		(unsigned)infoComp,
		(unsigned)infoExtSize,
		solidMode ? L"On" : L"Off",
		(unsigned)volumeSize,
		(unsigned)statInfo.fileCount,
		totalSizeText,
		packedText,
		ratioText,
		physicalText,
		liveText,
		holesText,
		holePercentText,
		metaText,
		tableText,
		headerTableText);

	if ( !GuiIsSmokeMode() ) {
		MessageBoxW(NULL, message, XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
	}
	return TRUE;
}

BOOL GuiArchiveShowProperties(GuiApp* app)
{
	return GuiShowPropertiesCore(app->archivePath, app->archive, app->packType, app->defaultComp, app->metaComp, app->infoComp, app->infoExtSize, app->volumeSize, app->solidMode);
}

BOOL GuiArchiveShowPropertiesPath(const WCHAR* archivePath)
{
	GuiApp tempApp;
	GuiArchiveOptions options;

	ZeroMemory(&tempApp, sizeof(tempApp));
	GuiAppInitDefaults(&tempApp);
	if ( !GuiArchiveOpenPath(&tempApp, archivePath, TRUE) ) {
		return FALSE;
	}
	if ( !GuiArchiveReadOptions(&tempApp, &options) ) {
		GuiAppCloseArchive(&tempApp);
		return FALSE;
	}
	GuiShowPropertiesCore(archivePath, tempApp.archive, options.packType, options.defaultComp, options.metaComp, options.infoComp, options.infoExtSize, options.volumeSize, options.solidMode);
	GuiAppCloseArchive(&tempApp);
	return TRUE;
}

static WCHAR* GuiAllocEmptyWideText(void)
{
	WCHAR* text;

	text = (WCHAR*)malloc(sizeof(WCHAR));
	if ( text != NULL ) {
		text[0] = L'\0';
	}
	return text;
}

static BOOL GuiTryDecodeUtf8MetaText(const void* data, uint32_t size, WCHAR** outText)
{
	const char* bytes;
	WCHAR* text;
	int cchText;
	uint32_t i;

	if ( outText == NULL ) {
		return FALSE;
	}

	*outText = NULL;
	if ( data == NULL || size == 0 ) {
		*outText = GuiAllocEmptyWideText();
		return *outText != NULL;
	}
	if ( size > 0x7FFFFFFFU ) {
		return FALSE;
	}

	bytes = (const char*)data;
	for ( i = 0; i < size; ++i ) {
		if ( bytes[i] == '\0' ) {
			return FALSE;
		}
	}

	cchText = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, bytes, (int)size, NULL, 0);
	if ( cchText <= 0 ) {
		return FALSE;
	}

	text = (WCHAR*)malloc(((size_t)cchText + 1) * sizeof(WCHAR));
	if ( text == NULL ) {
		return FALSE;
	}
	if ( MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, bytes, (int)size, text, cchText) != cchText ) {
		free(text);
		return FALSE;
	}
	text[cchText] = L'\0';
	*outText = text;
	return TRUE;
}

static BOOL GuiEncodeMetaUtf8Text(const WCHAR* text, void** outData, uint32_t* outSize)
{
	int textLen;
	int byteCount;
	char* bytes;

	if ( outData == NULL || outSize == NULL ) {
		return FALSE;
	}

	*outData = NULL;
	*outSize = 0;
	if ( text == NULL || text[0] == L'\0' ) {
		return TRUE;
	}

	textLen = (int)wcslen(text);
	byteCount = WideCharToMultiByte(CP_UTF8, 0, text, textLen, NULL, 0, NULL, NULL);
	if ( byteCount <= 0 ) {
		return FALSE;
	}

	bytes = (char*)malloc((size_t)byteCount);
	if ( bytes == NULL ) {
		return FALSE;
	}
	if ( WideCharToMultiByte(CP_UTF8, 0, text, textLen, bytes, byteCount, NULL, NULL) != byteCount ) {
		free(bytes);
		return FALSE;
	}

	*outData = bytes;
	*outSize = (uint32_t)byteCount;
	return TRUE;
}

static BOOL GuiTryDecodeUtf8PaddedText(const void* data, uint32_t size, WCHAR** outText)
{
	const char* bytes;
	uint32_t end;
	uint32_t i;
	int cchText;
	WCHAR* text;

	if ( outText == NULL ) {
		return FALSE;
	}

	*outText = NULL;
	if ( data == NULL || size == 0 ) {
		*outText = GuiAllocEmptyWideText();
		return *outText != NULL;
	}
	if ( size > 0x7FFFFFFFU ) {
		return FALSE;
	}

	bytes = (const char*)data;
	end = size;
	while ( end > 0 && bytes[end - 1] == '\0' ) {
		end--;
	}
	for ( i = 0; i < end; ++i ) {
		if ( bytes[i] == '\0' ) {
			return FALSE;
		}
	}
	if ( end == 0 ) {
		*outText = GuiAllocEmptyWideText();
		return *outText != NULL;
	}

	cchText = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, bytes, (int)end, NULL, 0);
	if ( cchText <= 0 ) {
		return FALSE;
	}

	text = (WCHAR*)malloc(((size_t)cchText + 1) * sizeof(WCHAR));
	if ( text == NULL ) {
		return FALSE;
	}
	if ( MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, bytes, (int)end, text, cchText) != cchText ) {
		free(text);
		return FALSE;
	}
	text[cchText] = L'\0';
	*outText = text;
	return TRUE;
}

static WCHAR* GuiBuildMetaHexPreview(const uint8_t* data, uint32_t size, BOOL* truncatedOut)
{
	static const uint32_t previewLimit = 4096;
	uint32_t previewSize;
	size_t lineCount;
	size_t cchText;
	WCHAR* text;
	uint32_t offset;

	if ( truncatedOut != NULL ) {
		*truncatedOut = FALSE;
	}

	if ( data == NULL || size == 0 ) {
		return GuiAllocEmptyWideText();
	}

	previewSize = (size > previewLimit) ? previewLimit : size;
	lineCount = (previewSize + 15u) / 16u;
	cchText = (lineCount * 80u) + 64u;
	text = (WCHAR*)malloc(cchText * sizeof(WCHAR));
	if ( text == NULL ) {
		return NULL;
	}
	text[0] = L'\0';

	for ( offset = 0; offset < previewSize; offset += 16u ) {
		WCHAR line[128];
		WCHAR cell[8];
		uint32_t i;

		_snwprintf_s(line, _countof(line), _TRUNCATE, L"%08X  ", (unsigned)offset);
		for ( i = 0; i < 16u; ++i ) {
			if ( offset + i < previewSize ) {
				_snwprintf_s(cell, _countof(cell), _TRUNCATE, L"%02X ", data[offset + i]);
			} else {
				wcsncpy_s(cell, _countof(cell), L"   ", _TRUNCATE);
			}
			wcscat_s(line, _countof(line), cell);
		}
		wcscat_s(line, _countof(line), L"\r\n");
		wcscat_s(text, cchText, line);
	}

	if ( previewSize < size ) {
		if ( truncatedOut != NULL ) {
			*truncatedOut = TRUE;
		}
		wcscat_s(text, cchText, L"...\r\n");
	}

	return text;
}

static void* GuiArchiveReadItemDataToMemory(GuiApp* app, const GuiArchiveItem* item, uint64_t* outSize)
{
	char utf8Pkg[XPK_PATH_BYTES];

	if ( outSize != NULL ) {
		*outSize = 0;
	}
	if ( app == NULL || app->archive == NULL || item == NULL ) {
		return NULL;
	}

	switch ( app->packType ) {
		case XPK_PACK_CORE:
			return xpkReadToMemory(app->archive, item->pos, outSize);
		case XPK_PACK_INDEX:
			return xpkIndexReadToMemory(app->archive, item->fileIndex, outSize);
		case XPK_PACK_LINUX:
		case XPK_PACK_WIN32:
			if ( item->packagePath[0] != '\0' ) {
				strncpy_s(utf8Pkg, sizeof(utf8Pkg), item->packagePath, _TRUNCATE);
			} else {
				GuiPathToPackageUtf8(item->name, utf8Pkg, sizeof(utf8Pkg));
			}
			return xpkPathReadToMemory(app->archive, utf8Pkg, outSize);
		default:
			return NULL;
	}
}

static BOOL GuiArchiveUpdateItemDataRaw(GuiApp* app, const GuiArchiveItem* item, const void* data, uint64_t size, const WCHAR* actionText)
{
	xpkWriteOptions writeOpt;
	int iRet;
	char utf8Pkg[XPK_PATH_BYTES];

	if ( app == NULL || app->archive == NULL || item == NULL ) {
		return FALSE;
	}

	ZeroMemory(&writeOpt, sizeof(writeOpt));
	writeOpt.compLevel = 0xFFu;
	writeOpt.writePolicy = app->writePolicy;
	writeOpt.fileType = GuiEntryFileType(item->flag);

	if ( app->packType == XPK_PACK_CORE ) {
		iRet = xpkUpdateData(app->archive, item->pos, data, size, &writeOpt);
	} else if ( app->packType == XPK_PACK_INDEX ) {
		iRet = xpkIndexUpdateData(app->archive, item->fileIndex, data, size, &writeOpt);
	} else {
		if ( item->packagePath[0] != '\0' ) {
			strncpy_s(utf8Pkg, sizeof(utf8Pkg), item->packagePath, _TRUNCATE);
		} else {
			GuiPathToPackageUtf8(item->name, utf8Pkg, sizeof(utf8Pkg));
		}
		iRet = xpkPathUpdateData(app->archive, utf8Pkg, data, size, &writeOpt);
	}

	if ( iRet != XPK_OK ) {
		GuiShowArchiveError(app, actionText);
		return FALSE;
	}
	return GuiArchiveSave(app);
}

BOOL GuiArchiveViewSelection(GuiApp* app)
{
	GuiArchiveItem* item;
	void* data;
	uint64_t size64;
	uint32_t size32;
	WCHAR* textBuf;
	WCHAR prompt[512];
	WCHAR title[128];
	WCHAR sizeText[64];
	GuiTextEditorDialogState state;
	BOOL truncated;

	if ( !GuiArchiveCanViewSelection(app) ) {
		MessageBoxW(app != NULL ? app->window : NULL, L"请选择单个文件进行查看。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	item = GuiArchiveGetSingleSelectedItem(app);
	if ( item == NULL ) {
		return FALSE;
	}
	if ( item->fileSize > XPKGUI_INLINE_VIEW_LIMIT ) {
		GuiFormatUInt64(item->fileSize, sizeText, _countof(sizeText));
		_snwprintf_s(
			prompt,
			_countof(prompt),
			_TRUNCATE,
			L"当前文件较大（%s），不直接在 GUI 内预览。请改用 Open / Edit / Extract。",
			sizeText);
		MessageBoxW(app->window, prompt, XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	size64 = 0;
	data = GuiArchiveReadItemDataToMemory(app, item, &size64);
	if ( data == NULL && size64 != 0 ) {
		GuiShowArchiveError(app, L"读取条目内容");
		return FALSE;
	}
	if ( size64 > 0xFFFFFFFFull ) {
		if ( data != NULL ) {
			xpkFree(data);
		}
		MessageBoxW(app->window, L"当前文件过大，无法在文本查看器中显示。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	size32 = (uint32_t)size64;
	textBuf = NULL;
	truncated = FALSE;
	GuiFormatUInt64(size64, sizeText, _countof(sizeText));
	if ( GuiTryDecodeUtf8MetaText(data, size32, &textBuf) ) {
		_snwprintf_s(
			prompt,
			_countof(prompt),
			_TRUNCATE,
			L"条目大小 %s 字节，已按 UTF-8 文本只读显示。若要修改，请使用 Edit 或 Replace。",
			sizeText);
	} else {
		textBuf = GuiBuildMetaHexPreview((const uint8_t*)data, size32, &truncated);
		_snwprintf_s(
			prompt,
			_countof(prompt),
			_TRUNCATE,
			L"条目大小 %s 字节，不是 UTF-8 文本。以下显示%s十六进制只读预览；若要处理原始内容，请使用 Open / Edit / Replace。",
			sizeText,
			truncated ? L"前 4096 字节的" : L"完整");
	}
	if ( data != NULL ) {
		xpkFree(data);
	}
	if ( textBuf == NULL ) {
		MessageBoxW(app->window, L"内存不足，无法显示条目内容。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	_snwprintf_s(title, _countof(title), _TRUNCATE, L"View - %s", item->name[0] != L'\0' ? item->name : L"entry");
	ZeroMemory(&state, sizeof(state));
	state.title = title;
	state.prompt = prompt;
	state.text = textBuf;
	state.cchText = wcslen(textBuf) + 1;
	state.readOnly = TRUE;
	(void)GuiRunTextEditorDialog(app->window, &state);
	free(state.text);
	return TRUE;
}

BOOL GuiArchiveEditSelectionText(GuiApp* app)
{
	GuiArchiveItem* item;
	void* data;
	uint64_t size64;
	uint32_t size32;
	WCHAR* textBuf;
	WCHAR title[128];
	WCHAR prompt[512];
	WCHAR sizeText[64];
	GuiTextEditorDialogState state;
	BOOL truncated;
	void* utf8Buf;
	uint32_t utf8Size;

	if ( app == NULL || app->archive == NULL ) {
		return FALSE;
	}
	item = GuiArchiveGetSingleSelectedItem(app);
	if ( item == NULL ) {
		MessageBoxW(app->window, L"请选择单个文件进行文本编辑。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}
	if ( item->fileSize > XPKGUI_INLINE_VIEW_LIMIT ) {
		GuiFormatUInt64(item->fileSize, sizeText, _countof(sizeText));
		_snwprintf_s(
			prompt,
			_countof(prompt),
			_TRUNCATE,
			L"当前文件较大（%s），不直接在 GUI 内编辑。请改用 Edit Externally / Open / Extract。",
			sizeText);
		MessageBoxW(app->window, prompt, XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	size64 = 0;
	data = GuiArchiveReadItemDataToMemory(app, item, &size64);
	if ( data == NULL && size64 != 0 ) {
		GuiShowArchiveError(app, L"读取条目内容");
		return FALSE;
	}
	if ( size64 > 0xFFFFFFFFull ) {
		if ( data != NULL ) {
			xpkFree(data);
		}
		MessageBoxW(app->window, L"当前文件过大，无法在文本编辑器中打开。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	size32 = (uint32_t)size64;
	textBuf = NULL;
	truncated = FALSE;
	GuiFormatUInt64(size64, sizeText, _countof(sizeText));
	if ( GuiTryDecodeUtf8MetaText(data, size32, &textBuf) ) {
		_snwprintf_s(
			prompt,
			_countof(prompt),
			_TRUNCATE,
			L"当前条目大小 %s 字节，已按 UTF-8 文本加载。可直接编辑并保存写回归档。",
			sizeText);
		_snwprintf_s(title, _countof(title), _TRUNCATE, L"Edit Text - %s", item->name[0] != L'\0' ? item->name : L"entry");
		ZeroMemory(&state, sizeof(state));
		state.title = title;
		state.prompt = prompt;
		state.text = textBuf;
		state.cchText = wcslen(textBuf) + 1;
		state.readOnly = FALSE;
		if ( data != NULL ) {
			xpkFree(data);
		}
		if ( !GuiRunTextEditorDialog(app->window, &state) ) {
			free(state.text);
			return FALSE;
		}

		utf8Buf = NULL;
		utf8Size = 0;
		if ( !GuiEncodeMetaUtf8Text(state.text, &utf8Buf, &utf8Size) ) {
			free(state.text);
			MessageBoxW(app->window, L"无法把文本编码为 UTF-8。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
			return FALSE;
		}
		free(state.text);
		if ( !GuiArchiveUpdateItemDataRaw(app, item, utf8Buf, utf8Size, L"写回条目文本") ) {
			free(utf8Buf);
			return FALSE;
		}
		free(utf8Buf);
		return TRUE;
	}

	textBuf = GuiBuildMetaHexPreview((const uint8_t*)data, size32, &truncated);
	if ( data != NULL ) {
		xpkFree(data);
	}
	if ( textBuf == NULL ) {
		MessageBoxW(app->window, L"内存不足，无法显示条目内容。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	_snwprintf_s(
		prompt,
		_countof(prompt),
		_TRUNCATE,
		L"当前条目大小 %s 字节，不是 UTF-8 文本。以下显示%s十六进制只读预览；如需修改，请使用 Edit / Replace。",
		sizeText,
		truncated ? L"前 4096 字节的" : L"完整");
	_snwprintf_s(title, _countof(title), _TRUNCATE, L"View Binary - %s", item->name[0] != L'\0' ? item->name : L"entry");
	ZeroMemory(&state, sizeof(state));
	state.title = title;
	state.prompt = prompt;
	state.text = textBuf;
	state.cchText = wcslen(textBuf) + 1;
	state.readOnly = TRUE;
	(void)GuiRunTextEditorDialog(app->window, &state);
	free(state.text);
	return TRUE;
}

BOOL GuiArchiveMetaEdit(GuiApp* app)
{
	void* pMeta;
	uint32_t metaSize;
	GuiTextEditorDialogState state;
	WCHAR prompt[256];
	WCHAR* textBuf;
	BOOL readOnly;
	BOOL truncated;
	void* utf8Buf;
	uint32_t utf8Size;

	if ( app == NULL || app->archive == NULL ) {
		return FALSE;
	}

	pMeta = xpkMetaGet(app->archive, &metaSize);
	textBuf = NULL;
	readOnly = FALSE;
	truncated = FALSE;
	if ( pMeta == NULL || metaSize == 0 ) {
		textBuf = GuiAllocEmptyWideText();
		_snwprintf_s(
			prompt,
			_countof(prompt),
			_TRUNCATE,
			L"当前归档没有包元数据。可直接输入 UTF-8 文本并保存；留空后确定将清空元数据。");
	} else if ( GuiTryDecodeUtf8MetaText(pMeta, metaSize, &textBuf) ) {
		_snwprintf_s(
			prompt,
			_countof(prompt),
			_TRUNCATE,
			L"当前包元数据大小 %u 字节，已按 UTF-8 文本加载。直接编辑并保存即可写回归档；留空后确定将清空元数据。",
			(unsigned)metaSize);
	} else {
		textBuf = GuiBuildMetaHexPreview((const uint8_t*)pMeta, metaSize, &truncated);
		readOnly = TRUE;
		_snwprintf_s(
			prompt,
			_countof(prompt),
			_TRUNCATE,
			L"当前包元数据大小 %u 字节，不是 UTF-8 文本。以下显示%s十六进制只读预览；若要替换，请使用 Import Meta... 或 Clear Meta。",
			(unsigned)metaSize,
			truncated ? L"前 4096 字节的" : L"完整");
	}
	if ( pMeta != NULL ) {
		xpkFree(pMeta);
	}
	if ( textBuf == NULL ) {
		MessageBoxW(app->window, L"内存不足，无法加载包元数据。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	ZeroMemory(&state, sizeof(state));
	state.title = readOnly ? L"查看包元数据" : L"编辑包元数据";
	state.prompt = prompt;
	state.text = textBuf;
	state.cchText = wcslen(textBuf) + 1;
	state.readOnly = readOnly;
	if ( !GuiRunTextEditorDialog(app->window, &state) ) {
		free(state.text);
		return FALSE;
	}
	if ( readOnly ) {
		free(state.text);
		return TRUE;
	}

	utf8Buf = NULL;
	utf8Size = 0;
	if ( !GuiEncodeMetaUtf8Text(state.text, &utf8Buf, &utf8Size) ) {
		free(state.text);
		MessageBoxW(app->window, L"无法把文本编码为 UTF-8。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	if ( utf8Size == 0 ) {
		if ( xpkMetaClear(app->archive) != XPK_OK ) {
			free(state.text);
			GuiShowArchiveError(app, L"清空包元数据");
			return FALSE;
		}
	} else {
		if ( xpkMetaSet(app->archive, utf8Buf, utf8Size, app->metaComp) != XPK_OK ) {
			free(utf8Buf);
			free(state.text);
			GuiShowArchiveError(app, L"写入包元数据");
			return FALSE;
		}
		free(utf8Buf);
	}

	free(state.text);
	return GuiArchiveSave(app);
}

BOOL GuiArchiveEditSelectionInfoExt(GuiApp* app)
{
	GuiArchiveItem* item;
	void* infoBuf;
	GuiTextEditorDialogState state;
	WCHAR prompt[256];
	WCHAR* textBuf;
	BOOL readOnly;
	BOOL truncated;
	void* utf8Buf;
	uint32_t utf8Size;

	if ( !GuiArchiveCanEditSelectionInfoExt(app) ) {
		MessageBoxW(app != NULL ? app->window : NULL, L"当前选中条目不支持编辑 InfoExt。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	item = GuiArchiveGetSingleSelectedItem(app);
	if ( item == NULL ) {
		return FALSE;
	}

	infoBuf = malloc(app->infoExtSize);
	if ( infoBuf == NULL ) {
		MessageBoxW(app->window, L"内存不足，无法加载条目 InfoExt。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}
	if ( xpkGetInfoExt(app->archive, item->pos, infoBuf, app->infoExtSize) != XPK_OK ) {
		free(infoBuf);
		GuiShowArchiveError(app, L"读取条目 InfoExt");
		return FALSE;
	}

	textBuf = NULL;
	readOnly = FALSE;
	truncated = FALSE;
	if ( GuiTryDecodeUtf8PaddedText(infoBuf, app->infoExtSize, &textBuf) ) {
		_snwprintf_s(
			prompt,
			_countof(prompt),
			_TRUNCATE,
			L"当前条目的 InfoExt 固定为 %u 字节。可按 UTF-8 文本编辑；保存时会自动以 0 填充剩余字节。",
			(unsigned)app->infoExtSize);
	} else {
		textBuf = GuiBuildMetaHexPreview((const uint8_t*)infoBuf, app->infoExtSize, &truncated);
		readOnly = TRUE;
		_snwprintf_s(
			prompt,
			_countof(prompt),
			_TRUNCATE,
			L"当前条目的 InfoExt 固定为 %u 字节，但不是可直接编辑的 UTF-8 文本。以下显示%s十六进制只读预览；如需处理原始二进制，请使用 Import / Export / Clear Entry InfoExt。",
			(unsigned)app->infoExtSize,
			truncated ? L"前 4096 字节的" : L"完整");
	}
	free(infoBuf);
	if ( textBuf == NULL ) {
		MessageBoxW(app->window, L"内存不足，无法显示条目 InfoExt。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	ZeroMemory(&state, sizeof(state));
	state.title = readOnly ? L"查看 Entry InfoExt" : L"编辑 Entry InfoExt";
	state.prompt = prompt;
	state.text = textBuf;
	state.cchText = wcslen(textBuf) + 1;
	state.readOnly = readOnly;
	if ( !GuiRunTextEditorDialog(app->window, &state) ) {
		free(state.text);
		return FALSE;
	}
	if ( readOnly ) {
		free(state.text);
		return TRUE;
	}

	utf8Buf = NULL;
	utf8Size = 0;
	if ( !GuiEncodeMetaUtf8Text(state.text, &utf8Buf, &utf8Size) ) {
		free(state.text);
		MessageBoxW(app->window, L"无法把 InfoExt 文本编码为 UTF-8。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}
	if ( utf8Size > app->infoExtSize ) {
		free(utf8Buf);
		free(state.text);
		MessageBoxW(app->window, L"文本编码后的长度超过当前 InfoExt 固定大小。请缩短内容，或先调整归档的 InfoExt Size。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	infoBuf = calloc(1, app->infoExtSize);
	if ( infoBuf == NULL ) {
		free(utf8Buf);
		free(state.text);
		MessageBoxW(app->window, L"内存不足，无法写回条目 InfoExt。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}
	if ( utf8Buf != NULL && utf8Size > 0 ) {
		memcpy(infoBuf, utf8Buf, utf8Size);
	}
	free(utf8Buf);

	if ( xpkSetInfoExt(app->archive, item->pos, infoBuf, app->infoExtSize) != XPK_OK ) {
		free(infoBuf);
		free(state.text);
		GuiShowArchiveError(app, L"写入条目 InfoExt");
		return FALSE;
	}
	free(infoBuf);
	free(state.text);
	return GuiArchiveSave(app);
}

static BOOL GuiArchiveReadSelectionInfoExt(GuiApp* app, GuiArchiveItem** itemOut, void** infoBufOut)
{
	GuiArchiveItem* item;
	void* infoBuf;

	if ( itemOut != NULL ) {
		*itemOut = NULL;
	}
	if ( infoBufOut != NULL ) {
		*infoBufOut = NULL;
	}
	if ( !GuiArchiveCanEditSelectionInfoExt(app) ) {
		MessageBoxW(app != NULL ? app->window : NULL, L"当前选中条目不支持操作 InfoExt。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	item = GuiArchiveGetSingleSelectedItem(app);
	if ( item == NULL ) {
		return FALSE;
	}

	infoBuf = malloc(app->infoExtSize);
	if ( infoBuf == NULL ) {
		MessageBoxW(app->window, L"内存不足，无法加载条目 InfoExt。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}
	if ( xpkGetInfoExt(app->archive, item->pos, infoBuf, app->infoExtSize) != XPK_OK ) {
		free(infoBuf);
		GuiShowArchiveError(app, L"读取条目 InfoExt");
		return FALSE;
	}

	if ( itemOut != NULL ) {
		*itemOut = item;
	}
	if ( infoBufOut != NULL ) {
		*infoBufOut = infoBuf;
	} else {
		free(infoBuf);
	}
	return TRUE;
}

static BOOL GuiArchiveWriteSelectionInfoExtRaw(GuiApp* app, GuiArchiveItem* item, const void* data, uint32_t size, const WCHAR* actionText)
{
	void* infoBuf;

	if ( app == NULL || item == NULL ) {
		return FALSE;
	}
	if ( size > app->infoExtSize ) {
		MessageBoxW(app->window, L"导入数据长度超过当前 InfoExt 固定大小。请改用更大的 InfoExt Size，或缩小输入数据。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	infoBuf = calloc(1, app->infoExtSize);
	if ( infoBuf == NULL ) {
		MessageBoxW(app->window, L"内存不足，无法写回条目 InfoExt。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return FALSE;
	}
	if ( data != NULL && size > 0 ) {
		memcpy(infoBuf, data, size);
	}

	if ( xpkSetInfoExt(app->archive, item->pos, infoBuf, app->infoExtSize) != XPK_OK ) {
		free(infoBuf);
		GuiShowArchiveError(app, actionText);
		return FALSE;
	}
	free(infoBuf);
	return GuiArchiveSave(app);
}

static void GuiBuildInfoExtDataPath(const GuiApp* app, const GuiArchiveItem* item, WCHAR* pathBuf, size_t cchPathBuf)
{
	WCHAR entryName[XPKGUI_ITEM_TEXT];
	WCHAR safeName[XPKGUI_ITEM_TEXT];
	const WCHAR* fileName;

	if ( pathBuf == NULL || cchPathBuf == 0 ) {
		return;
	}

	pathBuf[0] = L'\0';
	if ( app != NULL && app->archivePath[0] != L'\0' ) {
		wcsncpy_s(pathBuf, cchPathBuf, app->archivePath, _TRUNCATE);
		PathRemoveExtensionW(pathBuf);
	}

	fileName = L"entry";
	if ( item != NULL ) {
		fileName = PathFindFileNameW(item->name[0] != L'\0' ? item->name : L"entry");
	}
	wcsncpy_s(entryName, _countof(entryName), fileName, _TRUNCATE);
	PathRemoveExtensionW(entryName);
	GuiSanitizeFileComponent(entryName, safeName, _countof(safeName));
	if ( safeName[0] == L'\0' ) {
		if ( item != NULL ) {
			_snwprintf_s(safeName, _countof(safeName), _TRUNCATE, L"entry_%04u", item->pos);
		} else {
			wcsncpy_s(safeName, _countof(safeName), L"entry", _TRUNCATE);
		}
	}

	if ( pathBuf[0] != L'\0' ) {
		wcscat_s(pathBuf, cchPathBuf, L".");
	}
	wcscat_s(pathBuf, cchPathBuf, safeName);
	wcscat_s(pathBuf, cchPathBuf, L".infoext.bin");
}

static BOOL GuiOpenDataFileDialog(HWND hwnd, WCHAR* pathBuf, DWORD cchBuf)
{
	OPENFILENAMEW ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	pathBuf[0] = L'\0';
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
	ofn.lpstrFile = pathBuf;
	ofn.nMaxFile = cchBuf;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	return GetOpenFileNameW(&ofn);
}

static BOOL GuiSaveDataFileDialog(HWND hwnd, WCHAR* pathBuf, DWORD cchBuf)
{
	OPENFILENAMEW ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = L"Binary File (*.bin)\0*.bin\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = pathBuf;
	ofn.nMaxFile = cchBuf;
	ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
	return GetSaveFileNameW(&ofn);
}

BOOL GuiArchiveMetaImport(GuiApp* app)
{
	WCHAR pathBuf[MAX_PATH];
	void* pData;
	DWORD size;

	if ( !GuiOpenDataFileDialog(app->window, pathBuf, _countof(pathBuf)) ) {
		return FALSE;
	}
	if ( !GuiReadWholeFile(pathBuf, &pData, &size) ) {
		GuiShowSystemError(app->window, L"读取元数据文件失败", GetLastError());
		return FALSE;
	}
	if ( xpkMetaSet(app->archive, pData, size, app->metaComp) != XPK_OK ) {
		free(pData);
		GuiShowArchiveError(app, L"写入包元数据");
		return FALSE;
	}
	free(pData);
	return GuiArchiveSave(app);
}

BOOL GuiArchiveMetaExport(GuiApp* app)
{
	WCHAR pathBuf[MAX_PATH];
	void* pMeta;
	uint32_t metaSize;

	metaSize = 0;
	pMeta = xpkMetaGet(app->archive, &metaSize);
	if ( pMeta == NULL || metaSize == 0 ) {
		if ( pMeta != NULL ) {
			xpkFree(pMeta);
		}
		MessageBoxW(app->window, L"当前归档没有包元数据。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	wcsncpy_s(pathBuf, _countof(pathBuf), app->archivePath, _TRUNCATE);
	PathRemoveExtensionW(pathBuf);
	wcscat_s(pathBuf, _countof(pathBuf), L".meta.bin");
	if ( !GuiSaveDataFileDialog(app->window, pathBuf, _countof(pathBuf)) ) {
		xpkFree(pMeta);
		return FALSE;
	}
	if ( !GuiWriteWholeFile(pathBuf, pMeta, metaSize) ) {
		xpkFree(pMeta);
		GuiShowSystemError(app->window, L"写出元数据文件失败", GetLastError());
		return FALSE;
	}
	xpkFree(pMeta);
	return TRUE;
}

BOOL GuiArchiveMetaClear(GuiApp* app)
{
	if ( MessageBoxW(app->window, L"确认清空包元数据？", XPKGUI_APP_TITLE, MB_YESNO | MB_ICONQUESTION) != IDYES ) {
		return FALSE;
	}
	if ( xpkMetaClear(app->archive) != XPK_OK ) {
		GuiShowArchiveError(app, L"清空包元数据");
		return FALSE;
	}
	return GuiArchiveSave(app);
}

BOOL GuiArchiveImportSelectionInfoExt(GuiApp* app)
{
	GuiArchiveItem* item;
	WCHAR pathBuf[MAX_PATH];
	void* pData;
	DWORD size;

	if ( !GuiArchiveCanEditSelectionInfoExt(app) ) {
		MessageBoxW(app != NULL ? app->window : NULL, L"当前选中条目不支持导入 InfoExt。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}
	item = GuiArchiveGetSingleSelectedItem(app);
	if ( item == NULL ) {
		return FALSE;
	}
	if ( !GuiOpenDataFileDialog(app->window, pathBuf, _countof(pathBuf)) ) {
		return FALSE;
	}
	if ( !GuiReadWholeFile(pathBuf, &pData, &size) ) {
		GuiShowSystemError(app->window, L"读取 InfoExt 文件失败", GetLastError());
		return FALSE;
	}
	if ( size > app->infoExtSize ) {
		free(pData);
		MessageBoxW(app->window, L"导入文件大于当前 InfoExt 固定大小。请先调整归档的 InfoExt Size，或换用更小的文件。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}
	if ( !GuiArchiveWriteSelectionInfoExtRaw(app, item, pData, (uint32_t)size, L"写入条目 InfoExt") ) {
		free(pData);
		return FALSE;
	}
	free(pData);
	return TRUE;
}

BOOL GuiArchiveExportSelectionInfoExt(GuiApp* app)
{
	GuiArchiveItem* item;
	void* infoBuf;
	WCHAR pathBuf[MAX_PATH];

	item = NULL;
	infoBuf = NULL;
	if ( !GuiArchiveReadSelectionInfoExt(app, &item, &infoBuf) ) {
		return FALSE;
	}
	GuiBuildInfoExtDataPath(app, item, pathBuf, _countof(pathBuf));
	if ( !GuiSaveDataFileDialog(app->window, pathBuf, _countof(pathBuf)) ) {
		free(infoBuf);
		return FALSE;
	}
	if ( !GuiWriteWholeFile(pathBuf, infoBuf, app->infoExtSize) ) {
		free(infoBuf);
		GuiShowSystemError(app->window, L"写出 InfoExt 文件失败", GetLastError());
		return FALSE;
	}
	free(infoBuf);
	return TRUE;
}

BOOL GuiArchiveClearSelectionInfoExt(GuiApp* app)
{
	GuiArchiveItem* item;

	if ( !GuiArchiveCanEditSelectionInfoExt(app) ) {
		MessageBoxW(app != NULL ? app->window : NULL, L"当前选中条目不支持清空 InfoExt。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}
	if ( MessageBoxW(app->window, L"确认把当前选中条目的 InfoExt 清零？", XPKGUI_APP_TITLE, MB_YESNO | MB_ICONQUESTION) != IDYES ) {
		return FALSE;
	}
	item = GuiArchiveGetSingleSelectedItem(app);
	if ( item == NULL ) {
		return FALSE;
	}
	return GuiArchiveWriteSelectionInfoExtRaw(app, item, NULL, 0, L"清空条目 InfoExt");
}
