#define COBJMACROS

#include <windows.h>
#include <limits.h>
#include <objbase.h>
#include <ole2.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <strsafe.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#define XPKGUI_MAX_INPUTS 64
#define XPKGUI_SHELL_NAME L"xPack Shell Extension"
#define XPKGUI_SHELL_KEY L"xPackShell"
#define XPKGUI_SHELL_MAX_CMDLINE 32768
#define XPKGUI_SHELL_ARCHIVE_ITEMS 6
#define XPKGUI_SHELL_CREATE_ITEMS 2

static const CLSID CLSID_XpkguiShellExt =
	{0xa1b2c3d4, 0xe5f6, 0x7890, {0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x90}};

typedef enum XpkShellMenuMode {
	XPK_SHELL_MENU_NONE = 0,
	XPK_SHELL_MENU_ARCHIVE,
	XPK_SHELL_MENU_CREATE
} XpkShellMenuMode;

enum {
	XPK_CMD_OPEN = 1,
	XPK_CMD_EXTRACT,
	XPK_CMD_EXTRACT_AUTO,
	XPK_CMD_EXTRACT_HERE,
	XPK_CMD_VERIFY,
	XPK_CMD_PROPERTIES,
	XPK_CMD_ADD,
	XPK_CMD_ADD_AUTO
};

typedef struct XpkShellExt {
	IContextMenu contextMenu;
	IShellExtInit shellExtInit;
	LONG refCount;
	XpkShellMenuMode menuMode;
	UINT selectedCount;
	WCHAR selectedPaths[XPKGUI_MAX_INPUTS][MAX_PATH];
} XpkShellExt;

typedef struct XpkShellClassFactory {
	IClassFactory classFactory;
} XpkShellClassFactory;

static HMODULE g_module = NULL;
static LONG g_objectCount = 0;
static LONG g_lockCount = 0;

static HRESULT STDMETHODCALLTYPE XpkContextMenu_QueryInterface(IContextMenu* This, REFIID riid, void** ppvObject);
static ULONG STDMETHODCALLTYPE XpkContextMenu_AddRef(IContextMenu* This);
static ULONG STDMETHODCALLTYPE XpkContextMenu_Release(IContextMenu* This);
static HRESULT STDMETHODCALLTYPE XpkContextMenu_QueryContextMenu(IContextMenu* This, HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
static HRESULT STDMETHODCALLTYPE XpkContextMenu_InvokeCommand(IContextMenu* This, LPCMINVOKECOMMANDINFO pici);
static HRESULT STDMETHODCALLTYPE XpkContextMenu_GetCommandString(IContextMenu* This, UINT_PTR idCmd, UINT uType, UINT* pwReserved, LPSTR pszName, UINT cchMax);

static HRESULT STDMETHODCALLTYPE XpkShellExtInit_QueryInterface(IShellExtInit* This, REFIID riid, void** ppvObject);
static ULONG STDMETHODCALLTYPE XpkShellExtInit_AddRef(IShellExtInit* This);
static ULONG STDMETHODCALLTYPE XpkShellExtInit_Release(IShellExtInit* This);
static HRESULT STDMETHODCALLTYPE XpkShellExtInit_Initialize(IShellExtInit* This, LPCITEMIDLIST pidlFolder, IDataObject* pDataObj, HKEY hProgID);

static HRESULT STDMETHODCALLTYPE XpkClassFactory_QueryInterface(IClassFactory* This, REFIID riid, void** ppvObject);
static ULONG STDMETHODCALLTYPE XpkClassFactory_AddRef(IClassFactory* This);
static ULONG STDMETHODCALLTYPE XpkClassFactory_Release(IClassFactory* This);
static HRESULT STDMETHODCALLTYPE XpkClassFactory_CreateInstance(IClassFactory* This, IUnknown* pUnkOuter, REFIID riid, void** ppvObject);
static HRESULT STDMETHODCALLTYPE XpkClassFactory_LockServer(IClassFactory* This, BOOL fLock);

static IContextMenuVtbl g_contextMenuVtbl = {
	XpkContextMenu_QueryInterface,
	XpkContextMenu_AddRef,
	XpkContextMenu_Release,
	XpkContextMenu_QueryContextMenu,
	XpkContextMenu_InvokeCommand,
	XpkContextMenu_GetCommandString
};

static IShellExtInitVtbl g_shellExtInitVtbl = {
	XpkShellExtInit_QueryInterface,
	XpkShellExtInit_AddRef,
	XpkShellExtInit_Release,
	XpkShellExtInit_Initialize
};

static IClassFactoryVtbl g_classFactoryVtbl = {
	XpkClassFactory_QueryInterface,
	XpkClassFactory_AddRef,
	XpkClassFactory_Release,
	XpkClassFactory_CreateInstance,
	XpkClassFactory_LockServer
};

static XpkShellClassFactory g_classFactory = {
	{ &g_classFactoryVtbl }
};

static const struct {
	UINT id;
	const WCHAR* helpText;
	const WCHAR* verb;
} g_commandInfo[] = {
	{ XPK_CMD_OPEN, L"使用 xpkgui 打开 xPack 压缩包", L"open" },
	{ XPK_CMD_EXTRACT, L"解压到指定目录", L"extract" },
	{ XPK_CMD_EXTRACT_AUTO, L"解压到同名目录", L"extract_auto" },
	{ XPK_CMD_EXTRACT_HERE, L"解压到当前目录", L"extract_here" },
	{ XPK_CMD_VERIFY, L"校验压缩包完整性", L"verify" },
	{ XPK_CMD_PROPERTIES, L"查看 xPack 属性", L"properties" },
	{ XPK_CMD_ADD, L"打开创建对话框并添加所选项", L"add" },
	{ XPK_CMD_ADD_AUTO, L"直接按建议名称创建 .xpk", L"add_auto" }
};

static XpkShellExt* XpkShellExtFromContextMenu(IContextMenu* iface)
{
	return CONTAINING_RECORD(iface, XpkShellExt, contextMenu);
}

static XpkShellExt* XpkShellExtFromInit(IShellExtInit* iface)
{
	return CONTAINING_RECORD(iface, XpkShellExt, shellExtInit);
}

static HRESULT XpkShellExtQueryInterfaceInternal(XpkShellExt* shellExt, REFIID riid, void** ppvObject)
{
	if ( ppvObject == NULL ) {
		return E_POINTER;
	}

	*ppvObject = NULL;
	if ( IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IContextMenu) ) {
		*ppvObject = &shellExt->contextMenu;
	} else if ( IsEqualIID(riid, &IID_IShellExtInit) ) {
		*ppvObject = &shellExt->shellExtInit;
	} else {
		return E_NOINTERFACE;
	}

	InterlockedIncrement(&shellExt->refCount);
	return S_OK;
}

static ULONG XpkShellExtReleaseInternal(XpkShellExt* shellExt)
{
	LONG refCount;

	refCount = InterlockedDecrement(&shellExt->refCount);
	if ( refCount == 0 ) {
		LocalFree(shellExt);
		InterlockedDecrement(&g_objectCount);
	}
	return (ULONG)refCount;
}

static BOOL XpkShellPathIsArchive(const WCHAR* path)
{
	const WCHAR* ext;

	ext = PathFindExtensionW(path);
	return (ext != NULL && _wcsicmp(ext, L".xpk") == 0);
}

static void XpkShellExtResetSelection(XpkShellExt* shellExt)
{
	UINT i;

	shellExt->menuMode = XPK_SHELL_MENU_NONE;
	shellExt->selectedCount = 0;
	for ( i = 0; i < ARRAYSIZE(shellExt->selectedPaths); ++i ) {
		shellExt->selectedPaths[i][0] = L'\0';
	}
}

static HRESULT XpkShellExtLoadSelection(XpkShellExt* shellExt, IDataObject* dataObject)
{
	FORMATETC formatEtc;
	STGMEDIUM medium;
	HDROP dropHandle;
	UINT fileCount;
	UINT i;

	ZeroMemory(&formatEtc, sizeof(formatEtc));
	formatEtc.cfFormat = CF_HDROP;
	formatEtc.dwAspect = DVASPECT_CONTENT;
	formatEtc.lindex = -1;
	formatEtc.tymed = TYMED_HGLOBAL;

	if ( dataObject == NULL ) {
		return E_INVALIDARG;
	}
	if ( IDataObject_GetData(dataObject, &formatEtc, &medium) != S_OK ) {
		return E_INVALIDARG;
	}

	XpkShellExtResetSelection(shellExt);
	dropHandle = (HDROP)GlobalLock(medium.hGlobal);
	if ( dropHandle == NULL ) {
		ReleaseStgMedium(&medium);
		return E_INVALIDARG;
	}

	fileCount = DragQueryFileW(dropHandle, 0xFFFFFFFF, NULL, 0);
	if ( fileCount == 0 ) {
		GlobalUnlock(medium.hGlobal);
		ReleaseStgMedium(&medium);
		return E_INVALIDARG;
	}
	if ( fileCount > XPKGUI_MAX_INPUTS ) {
		fileCount = XPKGUI_MAX_INPUTS;
	}

	for ( i = 0; i < fileCount; ++i ) {
		DragQueryFileW(dropHandle, i, shellExt->selectedPaths[i], ARRAYSIZE(shellExt->selectedPaths[i]));
	}
	shellExt->selectedCount = fileCount;

	if ( fileCount == 1 && XpkShellPathIsArchive(shellExt->selectedPaths[0]) ) {
		shellExt->menuMode = XPK_SHELL_MENU_ARCHIVE;
	} else {
		shellExt->menuMode = XPK_SHELL_MENU_CREATE;
	}

	GlobalUnlock(medium.hGlobal);
	ReleaseStgMedium(&medium);
	return S_OK;
}

static HRESULT XpkShellAppendToken(WCHAR* buffer, size_t cchBuffer, const WCHAR* token)
{
	HRESULT hr;

	hr = StringCchCatW(buffer, cchBuffer, L" ");
	if ( FAILED(hr) ) {
		return hr;
	}
	hr = StringCchCatW(buffer, cchBuffer, token);
	if ( FAILED(hr) ) {
		return hr;
	}
	return S_OK;
}

static HRESULT XpkShellAppendQuotedArg(WCHAR* buffer, size_t cchBuffer, const WCHAR* arg)
{
	HRESULT hr;

	hr = StringCchCatW(buffer, cchBuffer, L" \"");
	if ( FAILED(hr) ) {
		return hr;
	}
	hr = StringCchCatW(buffer, cchBuffer, arg);
	if ( FAILED(hr) ) {
		return hr;
	}
	hr = StringCchCatW(buffer, cchBuffer, L"\"");
	if ( FAILED(hr) ) {
		return hr;
	}
	return S_OK;
}

static void XpkShellBuildArchiveBaseName(const WCHAR* archivePath, WCHAR* nameBuf, size_t cchNameBuf)
{
	wcsncpy_s(nameBuf, cchNameBuf, PathFindFileNameW(archivePath), _TRUNCATE);
	PathRemoveExtensionW(nameBuf);
	if ( nameBuf[0] == L'\0' ) {
		wcsncpy_s(nameBuf, cchNameBuf, L"xpack_extract", _TRUNCATE);
	}
}

static void XpkShellBuildAutoArchiveName(const XpkShellExt* shellExt, WCHAR* nameBuf, size_t cchNameBuf)
{
	DWORD attr;

	if ( shellExt->selectedCount > 1 ) {
		wcsncpy_s(nameBuf, cchNameBuf, L"xpack_archive.xpk", _TRUNCATE);
		return;
	}

	wcsncpy_s(nameBuf, cchNameBuf, PathFindFileNameW(shellExt->selectedPaths[0]), _TRUNCATE);
	attr = GetFileAttributesW(shellExt->selectedPaths[0]);
	if ( attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY) == 0 ) {
		PathRemoveExtensionW(nameBuf);
	}
	if ( nameBuf[0] == L'\0' ) {
		wcsncpy_s(nameBuf, cchNameBuf, L"xpack_archive", _TRUNCATE);
	}
	StringCchCatW(nameBuf, cchNameBuf, L".xpk");
}

static UINT XpkShellCommandFromOffset(const XpkShellExt* shellExt, UINT offset)
{
	if ( shellExt->menuMode == XPK_SHELL_MENU_ARCHIVE ) {
		switch ( offset ) {
			case 0: return XPK_CMD_OPEN;
			case 1: return XPK_CMD_EXTRACT;
			case 2: return XPK_CMD_EXTRACT_AUTO;
			case 3: return XPK_CMD_EXTRACT_HERE;
			case 4: return XPK_CMD_VERIFY;
			case 5: return XPK_CMD_PROPERTIES;
			default: return 0;
		}
	}

	if ( shellExt->menuMode == XPK_SHELL_MENU_CREATE ) {
		switch ( offset ) {
			case 0: return XPK_CMD_ADD;
			case 1: return XPK_CMD_ADD_AUTO;
			default: return 0;
		}
	}

	return 0;
}

static HRESULT XpkShellBuildCommandLine(XpkShellExt* shellExt, UINT commandId, WCHAR* cmdLine, size_t cchCmdLine)
{
	WCHAR exePath[MAX_PATH];
	UINT i;
	HRESULT hr;

	if ( cmdLine == NULL || cchCmdLine == 0 ) {
		return E_POINTER;
	}
	cmdLine[0] = L'\0';

	if ( GetModuleFileNameW(g_module, exePath, ARRAYSIZE(exePath)) == 0 ) {
		return HRESULT_FROM_WIN32(GetLastError());
	}
	if ( !PathRemoveFileSpecW(exePath) ) {
		return E_FAIL;
	}
	if ( !PathAppendW(exePath, L"xpkgui.exe") ) {
		return E_FAIL;
	}

	hr = StringCchPrintfW(cmdLine, cchCmdLine, L"\"%s\"", exePath);
	if ( FAILED(hr) ) {
		return hr;
	}

	switch ( commandId ) {
		case XPK_CMD_OPEN:
			break;
		case XPK_CMD_EXTRACT:
			hr = XpkShellAppendToken(cmdLine, cchCmdLine, L"/extract");
			break;
		case XPK_CMD_EXTRACT_AUTO:
			hr = XpkShellAppendToken(cmdLine, cchCmdLine, L"/extract-auto");
			break;
		case XPK_CMD_EXTRACT_HERE:
			hr = XpkShellAppendToken(cmdLine, cchCmdLine, L"/extract-here");
			break;
		case XPK_CMD_VERIFY:
			hr = XpkShellAppendToken(cmdLine, cchCmdLine, L"/verify");
			break;
		case XPK_CMD_PROPERTIES:
			hr = XpkShellAppendToken(cmdLine, cchCmdLine, L"/properties");
			break;
		case XPK_CMD_ADD:
			hr = XpkShellAppendToken(cmdLine, cchCmdLine, L"/shell-add");
			break;
		case XPK_CMD_ADD_AUTO:
			hr = XpkShellAppendToken(cmdLine, cchCmdLine, L"/shell-add-auto");
			break;
		default:
			return E_INVALIDARG;
	}
	if ( FAILED(hr) ) {
		return hr;
	}

	for ( i = 0; i < shellExt->selectedCount; ++i ) {
		hr = XpkShellAppendQuotedArg(cmdLine, cchCmdLine, shellExt->selectedPaths[i]);
		if ( FAILED(hr) ) {
			return hr;
		}
	}

	return S_OK;
}

static UINT XpkShellResolveStringVerb(LPCMINVOKECOMMANDINFO pici)
{
	LPCWSTR verbW;
	const char* verbA;
	size_t i;

	verbA = (const char*)pici->lpVerb;
	for ( i = 0; i < ARRAYSIZE(g_commandInfo); ++i ) {
		char verbBuf[32];
		WideCharToMultiByte(CP_ACP, 0, g_commandInfo[i].verb, -1, verbBuf, (int)ARRAYSIZE(verbBuf), NULL, NULL);
		if ( lstrcmpiA(verbA, verbBuf) == 0 ) {
			return g_commandInfo[i].id;
		}
	}

	if ( (pici->fMask & CMIC_MASK_UNICODE) != 0 ) {
		const CMINVOKECOMMANDINFOEX* picix;
		picix = (const CMINVOKECOMMANDINFOEX*)pici;
		verbW = picix->lpVerbW;
		for ( i = 0; i < ARRAYSIZE(g_commandInfo); ++i ) {
			if ( verbW != NULL && lstrcmpiW(verbW, g_commandInfo[i].verb) == 0 ) {
				return g_commandInfo[i].id;
			}
		}
	}

	return UINT_MAX;
}

static HRESULT XpkShellLaunchCommand(XpkShellExt* shellExt, UINT commandId)
{
	WCHAR cmdLine[XPKGUI_SHELL_MAX_CMDLINE];
	WCHAR workDir[MAX_PATH];
	STARTUPINFOW startupInfo;
	PROCESS_INFORMATION processInfo;
	DWORD attr;
	HRESULT hr;

	hr = XpkShellBuildCommandLine(shellExt, commandId, cmdLine, ARRAYSIZE(cmdLine));
	if ( FAILED(hr) ) {
		return hr;
	}

	workDir[0] = L'\0';
	wcsncpy_s(workDir, ARRAYSIZE(workDir), shellExt->selectedPaths[0], _TRUNCATE);
	attr = GetFileAttributesW(workDir);
	if ( attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY) == 0 ) {
		PathRemoveFileSpecW(workDir);
	}

	ZeroMemory(&startupInfo, sizeof(startupInfo));
	ZeroMemory(&processInfo, sizeof(processInfo));
	startupInfo.cb = sizeof(startupInfo);
	if ( !CreateProcessW(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, workDir[0] != L'\0' ? workDir : NULL, &startupInfo, &processInfo) ) {
		return HRESULT_FROM_WIN32(GetLastError());
	}

	WaitForInputIdle(processInfo.hProcess, 5000);
	CloseHandle(processInfo.hThread);
	CloseHandle(processInfo.hProcess);
	return S_OK;
}

static HRESULT XpkShellWriteString(HKEY root, const WCHAR* subKey, const WCHAR* valueName, const WCHAR* valueData)
{
	HKEY key;
	LSTATUS status;
	DWORD sizeInBytes;

	status = RegCreateKeyExW(root, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &key, NULL);
	if ( status != ERROR_SUCCESS ) {
		return HRESULT_FROM_WIN32(status);
	}

	sizeInBytes = ((DWORD)wcslen(valueData) + 1U) * sizeof(WCHAR);
	status = RegSetValueExW(key, valueName, 0, REG_SZ, (const BYTE*)valueData, sizeInBytes);
	RegCloseKey(key);
	if ( status != ERROR_SUCCESS ) {
		return HRESULT_FROM_WIN32(status);
	}
	return S_OK;
}

static HRESULT XpkShellRegisterContextHandler(const WCHAR* classesSubKey, const WCHAR* clsidText)
{
	WCHAR fullKey[256];

	if ( FAILED(StringCchPrintfW(fullKey, ARRAYSIZE(fullKey), L"Software\\Classes\\%s\\shellex\\ContextMenuHandlers\\%s", classesSubKey, XPKGUI_SHELL_KEY)) ) {
		return E_FAIL;
	}
	return XpkShellWriteString(HKEY_CURRENT_USER, fullKey, NULL, clsidText);
}

static void XpkShellDeleteKeyTree(HKEY root, const WCHAR* subKey)
{
	RegDeleteTreeW(root, subKey);
}

static const WCHAR* XpkShellHelpTextById(UINT id)
{
	size_t i;

	for ( i = 0; i < ARRAYSIZE(g_commandInfo); ++i ) {
		if ( g_commandInfo[i].id == id ) {
			return g_commandInfo[i].helpText;
		}
	}
	return NULL;
}

static const WCHAR* XpkShellVerbById(UINT id)
{
	size_t i;

	for ( i = 0; i < ARRAYSIZE(g_commandInfo); ++i ) {
		if ( g_commandInfo[i].id == id ) {
			return g_commandInfo[i].verb;
		}
	}
	return NULL;
}

static HRESULT STDMETHODCALLTYPE XpkContextMenu_QueryInterface(IContextMenu* This, REFIID riid, void** ppvObject)
{
	return XpkShellExtQueryInterfaceInternal(XpkShellExtFromContextMenu(This), riid, ppvObject);
}

static ULONG STDMETHODCALLTYPE XpkContextMenu_AddRef(IContextMenu* This)
{
	return (ULONG)InterlockedIncrement(&XpkShellExtFromContextMenu(This)->refCount);
}

static ULONG STDMETHODCALLTYPE XpkContextMenu_Release(IContextMenu* This)
{
	return XpkShellExtReleaseInternal(XpkShellExtFromContextMenu(This));
}

static HRESULT STDMETHODCALLTYPE XpkContextMenu_QueryContextMenu(IContextMenu* This, HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	XpkShellExt* shellExt;
	HMENU submenu;
	MENUITEMINFOW itemInfo;
	UINT itemCount;
	UNREFERENCED_PARAMETER(idCmdLast);

	shellExt = XpkShellExtFromContextMenu(This);
	if ( (uFlags & CMF_DEFAULTONLY) != 0 || shellExt->menuMode == XPK_SHELL_MENU_NONE || shellExt->selectedCount == 0 ) {
		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);
	}

	submenu = CreatePopupMenu();
	if ( submenu == NULL ) {
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if ( shellExt->menuMode == XPK_SHELL_MENU_ARCHIVE ) {
		WCHAR folderName[MAX_PATH];
		WCHAR labelBuf[MAX_PATH + 32];

		XpkShellBuildArchiveBaseName(shellExt->selectedPaths[0], folderName, ARRAYSIZE(folderName));
		InsertMenuW(submenu, (UINT)-1, MF_BYPOSITION | MF_STRING, idCmdFirst + 0, L"打开 xpkgui");
		InsertMenuW(submenu, (UINT)-1, MF_BYPOSITION | MF_STRING, idCmdFirst + 1, L"解压到...");
		StringCchPrintfW(labelBuf, ARRAYSIZE(labelBuf), L"解压到 \"%s\\\"", folderName);
		InsertMenuW(submenu, (UINT)-1, MF_BYPOSITION | MF_STRING, idCmdFirst + 2, labelBuf);
		InsertMenuW(submenu, (UINT)-1, MF_BYPOSITION | MF_STRING, idCmdFirst + 3, L"解压到当前目录");
		InsertMenuW(submenu, (UINT)-1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
		InsertMenuW(submenu, (UINT)-1, MF_BYPOSITION | MF_STRING, idCmdFirst + 4, L"校验压缩包");
		InsertMenuW(submenu, (UINT)-1, MF_BYPOSITION | MF_STRING, idCmdFirst + 5, L"属性");
		itemCount = XPKGUI_SHELL_ARCHIVE_ITEMS;
	} else {
		WCHAR archiveName[MAX_PATH];
		WCHAR labelBuf[MAX_PATH + 32];

		XpkShellBuildAutoArchiveName(shellExt, archiveName, ARRAYSIZE(archiveName));
		InsertMenuW(submenu, (UINT)-1, MF_BYPOSITION | MF_STRING, idCmdFirst + 0, L"添加到 xPack...");
		StringCchPrintfW(labelBuf, ARRAYSIZE(labelBuf), L"添加到 \"%s\"", archiveName);
		InsertMenuW(submenu, (UINT)-1, MF_BYPOSITION | MF_STRING, idCmdFirst + 1, labelBuf);
		itemCount = XPKGUI_SHELL_CREATE_ITEMS;
	}

	ZeroMemory(&itemInfo, sizeof(itemInfo));
	itemInfo.cbSize = sizeof(itemInfo);
	itemInfo.fMask = MIIM_SUBMENU | MIIM_STRING;
	itemInfo.hSubMenu = submenu;
	itemInfo.dwTypeData = L"xPack";
	if ( !InsertMenuItemW(hmenu, indexMenu, TRUE, &itemInfo) ) {
		DestroyMenu(submenu);
		return HRESULT_FROM_WIN32(GetLastError());
	}

	return MAKE_HRESULT(SEVERITY_SUCCESS, 0, itemCount);
}

static HRESULT STDMETHODCALLTYPE XpkContextMenu_InvokeCommand(IContextMenu* This, LPCMINVOKECOMMANDINFO pici)
{
	XpkShellExt* shellExt;
	UINT commandId;

	shellExt = XpkShellExtFromContextMenu(This);
	if ( HIWORD(pici->lpVerb) == 0 ) {
		commandId = XpkShellCommandFromOffset(shellExt, LOWORD(pici->lpVerb));
	} else {
		commandId = XpkShellResolveStringVerb(pici);
	}
	if ( commandId == 0 || commandId == UINT_MAX ) {
		return E_FAIL;
	}

	return XpkShellLaunchCommand(shellExt, commandId);
}

static HRESULT STDMETHODCALLTYPE XpkContextMenu_GetCommandString(IContextMenu* This, UINT_PTR idCmd, UINT uType, UINT* pwReserved, LPSTR pszName, UINT cchMax)
{
	UINT realId;
	const WCHAR* text;
	UNREFERENCED_PARAMETER(pwReserved);

	realId = XpkShellCommandFromOffset(XpkShellExtFromContextMenu(This), (UINT)idCmd);
	text = (uType == GCS_VERBW || uType == GCS_VERBA) ? XpkShellVerbById(realId) : XpkShellHelpTextById(realId);
	if ( text == NULL ) {
		return E_INVALIDARG;
	}

	if ( uType == GCS_HELPTEXTW || uType == GCS_VERBW ) {
		StringCchCopyW((WCHAR*)pszName, cchMax, text);
		return S_OK;
	}
	if ( uType == GCS_HELPTEXTA || uType == GCS_VERBA ) {
		WideCharToMultiByte(CP_ACP, 0, text, -1, pszName, (int)cchMax, NULL, NULL);
		return S_OK;
	}

	return E_INVALIDARG;
}

static HRESULT STDMETHODCALLTYPE XpkShellExtInit_QueryInterface(IShellExtInit* This, REFIID riid, void** ppvObject)
{
	return XpkShellExtQueryInterfaceInternal(XpkShellExtFromInit(This), riid, ppvObject);
}

static ULONG STDMETHODCALLTYPE XpkShellExtInit_AddRef(IShellExtInit* This)
{
	return (ULONG)InterlockedIncrement(&XpkShellExtFromInit(This)->refCount);
}

static ULONG STDMETHODCALLTYPE XpkShellExtInit_Release(IShellExtInit* This)
{
	return XpkShellExtReleaseInternal(XpkShellExtFromInit(This));
}

static HRESULT STDMETHODCALLTYPE XpkShellExtInit_Initialize(IShellExtInit* This, LPCITEMIDLIST pidlFolder, IDataObject* pDataObj, HKEY hProgID)
{
	UNREFERENCED_PARAMETER(pidlFolder);
	UNREFERENCED_PARAMETER(hProgID);
	return XpkShellExtLoadSelection(XpkShellExtFromInit(This), pDataObj);
}

static HRESULT STDMETHODCALLTYPE XpkClassFactory_QueryInterface(IClassFactory* This, REFIID riid, void** ppvObject)
{
	UNREFERENCED_PARAMETER(This);

	if ( ppvObject == NULL ) {
		return E_POINTER;
	}
	*ppvObject = NULL;

	if ( IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IClassFactory) ) {
		*ppvObject = &g_classFactory.classFactory;
		return S_OK;
	}
	return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE XpkClassFactory_AddRef(IClassFactory* This)
{
	UNREFERENCED_PARAMETER(This);
	return 2;
}

static ULONG STDMETHODCALLTYPE XpkClassFactory_Release(IClassFactory* This)
{
	UNREFERENCED_PARAMETER(This);
	return 1;
}

static HRESULT STDMETHODCALLTYPE XpkClassFactory_CreateInstance(IClassFactory* This, IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
{
	XpkShellExt* shellExt;
	HRESULT hr;
	UNREFERENCED_PARAMETER(This);

	if ( pUnkOuter != NULL ) {
		return CLASS_E_NOAGGREGATION;
	}
	if ( ppvObject == NULL ) {
		return E_POINTER;
	}

	shellExt = (XpkShellExt*)LocalAlloc(LPTR, sizeof(*shellExt));
	if ( shellExt == NULL ) {
		return E_OUTOFMEMORY;
	}

	shellExt->contextMenu.lpVtbl = &g_contextMenuVtbl;
	shellExt->shellExtInit.lpVtbl = &g_shellExtInitVtbl;
	shellExt->refCount = 1;
	XpkShellExtResetSelection(shellExt);
	InterlockedIncrement(&g_objectCount);

	hr = XpkShellExtQueryInterfaceInternal(shellExt, riid, ppvObject);
	XpkShellExtReleaseInternal(shellExt);
	return hr;
}

static HRESULT STDMETHODCALLTYPE XpkClassFactory_LockServer(IClassFactory* This, BOOL fLock)
{
	UNREFERENCED_PARAMETER(This);

	if ( fLock ) {
		InterlockedIncrement(&g_lockCount);
	} else {
		InterlockedDecrement(&g_lockCount);
	}
	return S_OK;
}

STDAPI DllCanUnloadNow(void)
{
	return (g_objectCount == 0 && g_lockCount == 0) ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppvObject)
{
	if ( !IsEqualCLSID(rclsid, &CLSID_XpkguiShellExt) ) {
		return CLASS_E_CLASSNOTAVAILABLE;
	}
	return XpkClassFactory_QueryInterface(&g_classFactory.classFactory, riid, ppvObject);
}

STDAPI DllRegisterServer(void)
{
	WCHAR modulePath[MAX_PATH];
	WCHAR clsidText[64];
	WCHAR clsidKey[256];
	HRESULT hr;

	if ( GetModuleFileNameW(g_module, modulePath, ARRAYSIZE(modulePath)) == 0 ) {
		return HRESULT_FROM_WIN32(GetLastError());
	}
	if ( StringFromGUID2(&CLSID_XpkguiShellExt, clsidText, ARRAYSIZE(clsidText)) == 0 ) {
		return E_FAIL;
	}
	if ( FAILED(StringCchPrintfW(clsidKey, ARRAYSIZE(clsidKey), L"Software\\Classes\\CLSID\\%s", clsidText)) ) {
		return E_FAIL;
	}

	hr = XpkShellWriteString(HKEY_CURRENT_USER, clsidKey, NULL, XPKGUI_SHELL_NAME);
	if ( FAILED(hr) ) {
		return hr;
	}
	if ( FAILED(StringCchCatW(clsidKey, ARRAYSIZE(clsidKey), L"\\InprocServer32")) ) {
		return E_FAIL;
	}
	hr = XpkShellWriteString(HKEY_CURRENT_USER, clsidKey, NULL, modulePath);
	if ( FAILED(hr) ) {
		return hr;
	}
	hr = XpkShellWriteString(HKEY_CURRENT_USER, clsidKey, L"ThreadingModel", L"Apartment");
	if ( FAILED(hr) ) {
		return hr;
	}

	hr = XpkShellRegisterContextHandler(L"*", clsidText);
	if ( FAILED(hr) ) {
		return hr;
	}
	hr = XpkShellRegisterContextHandler(L"Directory", clsidText);
	if ( FAILED(hr) ) {
		return hr;
	}
	hr = XpkShellRegisterContextHandler(L"Folder", clsidText);
	if ( FAILED(hr) ) {
		return hr;
	}
	hr = XpkShellRegisterContextHandler(L".xpk", clsidText);
	if ( FAILED(hr) ) {
		return hr;
	}

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	return S_OK;
}

STDAPI DllUnregisterServer(void)
{
	WCHAR clsidText[64];
	WCHAR clsidKey[256];

	if ( StringFromGUID2(&CLSID_XpkguiShellExt, clsidText, ARRAYSIZE(clsidText)) == 0 ) {
		return E_FAIL;
	}
	if ( FAILED(StringCchPrintfW(clsidKey, ARRAYSIZE(clsidKey), L"Software\\Classes\\CLSID\\%s", clsidText)) ) {
		return E_FAIL;
	}

	XpkShellDeleteKeyTree(HKEY_CURRENT_USER, L"Software\\Classes\\*\\shellex\\ContextMenuHandlers\\xPackShell");
	XpkShellDeleteKeyTree(HKEY_CURRENT_USER, L"Software\\Classes\\Directory\\shellex\\ContextMenuHandlers\\xPackShell");
	XpkShellDeleteKeyTree(HKEY_CURRENT_USER, L"Software\\Classes\\Folder\\shellex\\ContextMenuHandlers\\xPackShell");
	XpkShellDeleteKeyTree(HKEY_CURRENT_USER, L"Software\\Classes\\.xpk\\shellex\\ContextMenuHandlers\\xPackShell");
	XpkShellDeleteKeyTree(HKEY_CURRENT_USER, clsidKey);

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	return S_OK;
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	UNREFERENCED_PARAMETER(reserved);

	if ( reason == DLL_PROCESS_ATTACH ) {
		g_module = instance;
		DisableThreadLibraryCalls(instance);
	}
	return TRUE;
}
