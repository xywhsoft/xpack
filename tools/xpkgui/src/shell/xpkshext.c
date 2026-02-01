#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "shell32")
#pragma comment(lib, "shlwapi")
#pragma comment(lib, "ole32")

#define CLSID_XPKSHELLEXT {0xA1B2C3D4, 0xE5F6, 0x7890, {0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x90}}

static const GUID CLSID_XPKShellExt = CLSID_XPKSHELLEXT;
static HMODULE g_hModule = NULL;
static LONG g_cRef = 0;

typedef struct _XPKShellExt {
    IContextMenuVtbl* lpVtbl;
    IShellExtInitVtbl* lpVtblInit;
    LONG refCount;
    HWND hwnd;
    char selectedFiles[MAX_PATH * 10];
    int fileCount;
    int isXpkFile;
} XPKShellExt;

static IContextMenuVtbl g_ContextMenuVtbl;
static IShellExtInitVtbl g_ShellExtInitVtbl;

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
STDAPI DllCanUnloadNow(void);
HRESULT STDMETHODCALLTYPE XPKShellExt_QueryInterface(IContextMenu* This, REFIID riid, void** ppv);
ULONG STDMETHODCALLTYPE XPKShellExt_AddRef(IContextMenu* This);
ULONG STDMETHODCALLTYPE XPKShellExt_Release(IContextMenu* This);
HRESULT STDMETHODCALLTYPE XPKShellExt_QueryContextMenu(IContextMenu* This, HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
HRESULT STDMETHODCALLTYPE XPKShellExt_InvokeCommand(IContextMenu* This, LPCMINVOKECOMMANDINFO pici);
HRESULT STDMETHODCALLTYPE XPKShellExt_GetCommandString(IContextMenu* This, UINT_PTR idCmd, UINT uFlags, UINT* pwReserved, LPSTR pszName, UINT cchMax);
HRESULT STDMETHODCALLTYPE XPKShellExt_Init(IShellExtInit* This, LPCITEMIDLIST pidlFolder, IDataObject* pDataObj, HKEY hkeyProgID);
void XPKShellExt_Constructor(XPKShellExt* pShellExt);

HRESULT STDMETHODCALLTYPE XPKShellExt_QueryInterface(IContextMenu* This, REFIID riid, void** ppv)
{
    XPKShellExt* pShellExt = (XPKShellExt*)This;

    if (IsEqualIID(riid, &IID_IUnknown) ||
        IsEqualIID(riid, &IID_IContextMenu)) {
        *ppv = (IContextMenu*)pShellExt;
        InterlockedIncrement(&pShellExt->refCount);
        return S_OK;
    } else if (IsEqualIID(riid, &IID_IShellExtInit)) {
        *ppv = (IShellExtInit*)pShellExt;
        InterlockedIncrement(&pShellExt->refCount);
        return S_OK;
    }

    *ppv = NULL;
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE XPKShellExt_AddRef(IContextMenu* This)
{
    XPKShellExt* pShellExt = (XPKShellExt*)This;
    return InterlockedIncrement(&pShellExt->refCount);
}

ULONG STDMETHODCALLTYPE XPKShellExt_Release(IContextMenu* This)
{
    XPKShellExt* pShellExt = (XPKShellExt*)This;
    LONG cRef = InterlockedDecrement(&pShellExt->refCount);
    if (cRef == 0) {
        LocalFree(pShellExt);
        InterlockedDecrement(&g_cRef);
    }
    return cRef;
}

HRESULT STDMETHODCALLTYPE XPKShellExt_Init(IShellExtInit* This, LPCITEMIDLIST pidlFolder, IDataObject* pDataObj, HKEY hkeyProgID)
{
    XPKShellExt* pShellExt = (XPKShellExt*)This;
    FORMATETC fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM stm;
    HDROP hDrop;
    UINT fileCount;
    UINT i;
    char filePath[MAX_PATH];
    const char* ext;

    if (!pDataObj) {
        return E_INVALIDARG;
    }

    if (pDataObj->lpVtbl->GetData(pDataObj, &fe, &stm) != S_OK) {
        return E_INVALIDARG;
    }

    hDrop = (HDROP)GlobalLock(stm.hGlobal);
    if (!hDrop) {
        GlobalUnlock(stm.hGlobal);
        ReleaseStgMedium(&stm);
        return E_INVALIDARG;
    }

    fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
    pShellExt->fileCount = 0;
    pShellExt->isXpkFile = 0;

    for (i = 0; i < fileCount && i < 10; i++) {
        DragQueryFile(hDrop, i, filePath, MAX_PATH);
        ext = PathFindExtension(filePath);

        if (ext && _stricmp(ext, ".xpk") == 0) {
            pShellExt->isXpkFile = 1;
        }

        if (i == 0) {
            strcpy(pShellExt->selectedFiles, filePath);
        } else {
            strcat(pShellExt->selectedFiles, "\" \"");
            strcat(pShellExt->selectedFiles, filePath);
        }
        pShellExt->fileCount++;
    }

    GlobalUnlock(stm.hGlobal);
    ReleaseStgMedium(&stm);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE XPKShellExt_QueryContextMenu(IContextMenu* This, HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    XPKShellExt* pShellExt = (XPKShellExt*)This;
    HMENU hSubMenu;
    MENUITEMINFO mii;

    if (pShellExt->isXpkFile) {
        InsertMenu(hmenu, indexMenu++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);

        InsertMenu(hmenu, indexMenu, MF_STRING | MF_BYPOSITION,
            idCmdFirst + 0, "打开 xpkgui");

        InsertMenu(hmenu, ++indexMenu, MF_STRING | MF_BYPOSITION,
            idCmdFirst + 1, "解压到...");

        InsertMenu(hmenu, ++indexMenu, MF_STRING | MF_BYPOSITION,
            idCmdFirst + 2, "解压到当前文件夹");

        InsertMenu(hmenu, ++indexMenu, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);

        InsertMenu(hmenu, ++indexMenu, MF_STRING | MF_BYPOSITION,
            idCmdFirst + 3, "验证压缩包");

        InsertMenu(hmenu, ++indexMenu, MF_STRING | MF_BYPOSITION,
            idCmdFirst + 4, "属性");

        return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 5);
    } else {
        InsertMenu(hmenu, indexMenu++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);

        hSubMenu = CreatePopupMenu();
        if (hSubMenu) {
            AppendMenu(hSubMenu, MF_STRING, idCmdFirst + 10, "添加到 xPack...");
            AppendMenu(hSubMenu, MF_STRING, idCmdFirst + 11, "添加到 xPack (自命名)");

            memset(&mii, 0, sizeof(MENUITEMINFO));
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_SUBMENU | MIIM_STRING;
            mii.hSubMenu = hSubMenu;
            mii.dwTypeData = "xPack";

            InsertMenuItem(hmenu, indexMenu++, TRUE, &mii);
        }

        return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 2);
    }
}

HRESULT STDMETHODCALLTYPE XPKShellExt_InvokeCommand(IContextMenu* This, LPCMINVOKECOMMANDINFO pici)
{
    XPKShellExt* pShellExt = (XPKShellExt*)This;
    char cmdLine[MAX_PATH * 12];
    char exePath[MAX_PATH];
    STARTUPINFO si = {sizeof(si)};
    PROCESS_INFORMATION pi;

    if (HIWORD(pici->lpVerb)) {
        return E_INVALIDARG;
    }

    GetModuleFileName(g_hModule, exePath, MAX_PATH);
    PathRemoveFileSpec(exePath);
    PathAppend(exePath, "xpkgui.exe");

    if (pShellExt->isXpkFile) {
        switch (LOWORD(pici->lpVerb)) {
            case 0:
                sprintf_s(cmdLine, sizeof(cmdLine), "\"%s\" \"%s\"", exePath, pShellExt->selectedFiles);
                break;
            case 1:
                sprintf_s(cmdLine, sizeof(cmdLine), "\"%s\" -extract \"%s\"", exePath, pShellExt->selectedFiles);
                break;
            case 2:
                sprintf_s(cmdLine, sizeof(cmdLine), "\"%s\" -extract_here \"%s\"", exePath, pShellExt->selectedFiles);
                break;
            case 3:
                sprintf_s(cmdLine, sizeof(cmdLine), "\"%s\" -verify \"%s\"", exePath, pShellExt->selectedFiles);
                break;
            case 4:
                sprintf_s(cmdLine, sizeof(cmdLine), "\"%s\" -properties \"%s\"", exePath, pShellExt->selectedFiles);
                break;
            default:
                return E_INVALIDARG;
        }
    } else {
        switch (LOWORD(pici->lpVerb)) {
            case 10:
                sprintf_s(cmdLine, sizeof(cmdLine), "\"%s\" -add \"%s\"", exePath, pShellExt->selectedFiles);
                break;
            case 11:
                sprintf_s(cmdLine, sizeof(cmdLine), "\"%s\" -add_auto \"%s\"", exePath, pShellExt->selectedFiles);
                break;
            default:
                return E_INVALIDARG;
        }
    }

    if (CreateProcess(NULL, cmdLine, NULL, NULL, FALSE,
        NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
        WaitForInputIdle(pi.hProcess, 5000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return S_OK;
    }

    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE XPKShellExt_GetCommandString(IContextMenu* This, UINT_PTR idCmd, UINT uFlags, UINT* pwReserved, LPSTR pszName, UINT cchMax)
{
    const char* helpText[] = {
        "使用 xpkgui 打开压缩包",
        "提取文件到指定目录",
        "提取文件到当前文件夹",
        "验证压缩包完整性",
        "查看压缩包属性",
        "添加文件到压缩包",
        "自动命名并添加到压缩包"
    };

    if (uFlags & GCS_HELPTEXTA) {
        if (idCmd < 7 && helpText[idCmd]) {
            lstrcpynA(pszName, helpText[idCmd], cchMax);
            return S_OK;
        }
    } else if (uFlags & GCS_VERBA) {
        switch (idCmd) {
            case 0: lstrcpynA(pszName, "open", cchMax); return S_OK;
            case 1: lstrcpynA(pszName, "extract", cchMax); return S_OK;
            case 2: lstrcpynA(pszName, "extract_here", cchMax); return S_OK;
            case 3: lstrcpynA(pszName, "verify", cchMax); return S_OK;
            case 4: lstrcpynA(pszName, "properties", cchMax); return S_OK;
            case 10: lstrcpynA(pszName, "add", cchMax); return S_OK;
            case 11: lstrcpynA(pszName, "add_auto", cchMax); return S_OK;
        }
    }

    return E_INVALIDARG;
}

static IContextMenuVtbl g_ContextMenuVtbl = {
    XPKShellExt_QueryInterface,
    XPKShellExt_AddRef,
    XPKShellExt_Release,
    XPKShellExt_QueryContextMenu,
    XPKShellExt_InvokeCommand,
    XPKShellExt_GetCommandString
};

static IShellExtInitVtbl g_ShellExtInitVtbl = {
    (HRESULT STDMETHODCALLTYPE (*)(IShellExtInit*, REFIID, void**))XPKShellExt_QueryInterface,
    (ULONG STDMETHODCALLTYPE (*)(IShellExtInit*))XPKShellExt_AddRef,
    (ULONG STDMETHODCALLTYPE (*)(IShellExtInit*))XPKShellExt_Release,
    XPKShellExt_Init
};

void XPKShellExt_Constructor(XPKShellExt* pShellExt)
{
    pShellExt->lpVtbl = &g_ContextMenuVtbl;
    pShellExt->lpVtblInit = &g_ShellExtInitVtbl;
    pShellExt->refCount = 1;
    pShellExt->hwnd = NULL;
    pShellExt->fileCount = 0;
    pShellExt->isXpkFile = 0;
    pShellExt->selectedFiles[0] = '\0';
}

typedef struct _XPKClassFactory {
    IClassFactoryVtbl* lpVtbl;
} XPKClassFactory;

static HRESULT STDMETHODCALLTYPE XPKClassFactory_QueryInterface(IClassFactory* This, REFIID riid, void** ppv);
static ULONG STDMETHODCALLTYPE XPKClassFactory_AddRef(IClassFactory* This);
static ULONG STDMETHODCALLTYPE XPKClassFactory_Release(IClassFactory* This);
static HRESULT STDMETHODCALLTYPE XPKClassFactory_CreateInstance(IClassFactory* This, LPUNKNOWN pUnkOuter, REFIID riid, void** ppv);
static HRESULT STDMETHODCALLTYPE XPKClassFactory_LockServer(IClassFactory* This, BOOL fLock);

static IClassFactoryVtbl g_ClassFactoryVtbl = {
    XPKClassFactory_QueryInterface,
    XPKClassFactory_AddRef,
    XPKClassFactory_Release,
    XPKClassFactory_CreateInstance,
    XPKClassFactory_LockServer
};

HRESULT STDMETHODCALLTYPE XPKClassFactory_QueryInterface(IClassFactory* This, REFIID riid, void** ppv)
{
    if (IsEqualIID(riid, &IID_IUnknown) ||
        IsEqualIID(riid, &IID_IClassFactory)) {
        *ppv = This;
        return S_OK;
    }
    *ppv = NULL;
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE XPKClassFactory_AddRef(IClassFactory* This)
{
    return 1;
}

ULONG STDMETHODCALLTYPE XPKClassFactory_Release(IClassFactory* This)
{
    return 1;
}

HRESULT STDMETHODCALLTYPE XPKClassFactory_CreateInstance(IClassFactory* This, LPUNKNOWN pUnkOuter, REFIID riid, void** ppv)
{
    XPKShellExt* pShellExt;

    if (pUnkOuter) {
        return CLASS_E_NOAGGREGATION;
    }

    pShellExt = (XPKShellExt*)LocalAlloc(LPTR, sizeof(XPKShellExt));
    if (!pShellExt) {
        return E_OUTOFMEMORY;
    }

    XPKShellExt_Constructor(pShellExt);
    InterlockedIncrement(&g_cRef);

    return XPKShellExt_QueryInterface((IContextMenu*)pShellExt, riid, ppv);
}

HRESULT STDMETHODCALLTYPE XPKClassFactory_LockServer(IClassFactory* This, BOOL fLock)
{
    return S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    static XPKClassFactory g_ClassFactory = {&g_ClassFactoryVtbl};

    if (!IsEqualCLSID(rclsid, &CLSID_XPKShellExt)) {
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    return XPKClassFactory_QueryInterface((IClassFactory*)&g_ClassFactory, riid, ppv);
}

STDAPI DllCanUnloadNow(void)
{
    return g_cRef == 0 ? S_OK : S_FALSE;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            g_hModule = hinstDLL;
            DisableThreadLibraryCalls(hinstDLL);
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
