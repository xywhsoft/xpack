#include "app.h"
#include "resources/resource.h"

#include <objbase.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>

static LRESULT CALLBACK GuiMainWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK GuiNavPathEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR idSubclass, DWORD_PTR refData);
static LRESULT CALLBACK GuiFilterEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR idSubclass, DWORD_PTR refData);

static void GuiFocusEditControl(HWND edit)
{
	if ( edit != NULL ) {
		SetFocus(edit);
		SendMessageW(edit, EM_SETSEL, 0, -1);
	}
}

static void GuiSelectAllVisibleItems(GuiApp* app)
{
	int itemCount;

	if ( app == NULL || app->archive == NULL || app->list == NULL ) {
		return;
	}

	itemCount = ListView_GetItemCount(app->list);
	if ( itemCount <= 0 ) {
		return;
	}

	ListView_SetItemState(app->list, -1, LVIS_SELECTED, LVIS_SELECTED);
	if ( ListView_GetNextItem(app->list, -1, LVNI_FOCUSED) < 0 ) {
		ListView_SetItemState(app->list, 0, LVIS_FOCUSED, LVIS_FOCUSED);
		ListView_SetSelectionMark(app->list, 0);
	}
	SetFocus(app->list);
	GuiSetMenuState(app);
	GuiUpdateStatus(app);
}

static void GuiBuildRecentMenuLabel(UINT index, const WCHAR* path, WCHAR* label, size_t cchLabel)
{
	WCHAR escapedPath[2 * MAX_PATH];
	size_t out;

	out = 0;
	while ( path != NULL && *path != L'\0' && out + 1 < _countof(escapedPath) ) {
		if ( *path == L'&' && out + 2 < _countof(escapedPath) ) {
			escapedPath[out++] = L'&';
		}
		escapedPath[out++] = *path++;
	}
	escapedPath[out] = L'\0';

	if ( index < 9 ) {
		_snwprintf_s(label, cchLabel, _TRUNCATE, L"&%u %s", index + 1, escapedPath);
	} else {
		_snwprintf_s(label, cchLabel, _TRUNCATE, L"1&0 %s", escapedPath);
	}
}

static void GuiUpdateRecentMenu(GuiApp* app, HMENU mainMenu)
{
	HMENU fileMenu;
	HMENU recentMenu;
	UINT i;

	if ( app == NULL || mainMenu == NULL ) {
		return;
	}

	fileMenu = GetSubMenu(mainMenu, 0);
	if ( fileMenu == NULL ) {
		return;
	}
	recentMenu = GetSubMenu(fileMenu, 2);
	if ( recentMenu == NULL ) {
		return;
	}

	while ( GetMenuItemCount(recentMenu) > 0 ) {
		DeleteMenu(recentMenu, 0, MF_BYPOSITION);
	}

	if ( app->recentArchiveCount == 0 ) {
		AppendMenuW(recentMenu, MF_STRING | MF_GRAYED, 0, L"(Empty)");
		return;
	}

	for ( i = 0; i < app->recentArchiveCount && i < XPKGUI_MAX_RECENT_ARCHIVES; ++i ) {
		WCHAR label[2 * MAX_PATH];

		GuiBuildRecentMenuLabel(i, app->recentArchives[i], label, _countof(label));
		AppendMenuW(recentMenu, MF_STRING, ID_FILE_RECENT_FIRST + i, label);
	}
	AppendMenuW(recentMenu, MF_SEPARATOR, 0, NULL);
	AppendMenuW(recentMenu, MF_STRING, ID_FILE_RECENT_CLEAR, L"Clear Recent");
}

static BOOL GuiShouldTranslateAccelerator(const GuiApp* app, const MSG* msg)
{
	if ( app == NULL || msg == NULL ) {
		return TRUE;
	}
	if ( msg->message != WM_KEYDOWN && msg->message != WM_SYSKEYDOWN ) {
		return TRUE;
	}
	if ( (msg->wParam == 'A' || msg->wParam == 'C') && (GetKeyState(VK_CONTROL) & 0x8000) != 0 ) {
		if ( msg->hwnd == app->filterEdit || msg->hwnd == app->navPath ) {
			return FALSE;
		}
	}
	if ( msg->wParam == VK_RETURN && (GetKeyState(VK_MENU) & 0x8000) != 0 ) {
		if ( msg->hwnd == app->filterEdit || msg->hwnd == app->navPath ) {
			return FALSE;
		}
	}
	return TRUE;
}

static LRESULT CALLBACK GuiNavPathEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR idSubclass, DWORD_PTR refData)
{
	(void)idSubclass;
	(void)refData;

	if ( msg == WM_KEYDOWN && wParam == VK_RETURN ) {
		HWND parent;

		parent = GetParent(hwnd);
		if ( parent != NULL ) {
			PostMessageW(parent, WM_COMMAND, MAKEWPARAM(IDC_NAV_GO, BN_CLICKED), (LPARAM)GetDlgItem(parent, IDC_NAV_GO));
		}
		return 0;
	}
	return DefSubclassProc(hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK GuiFilterEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR idSubclass, DWORD_PTR refData)
{
	(void)idSubclass;
	(void)refData;

	if ( msg == WM_KEYDOWN && wParam == VK_ESCAPE ) {
		HWND parent;

		parent = GetParent(hwnd);
		if ( parent != NULL ) {
			SetWindowTextW(hwnd, L"");
			PostMessageW(parent, WM_COMMAND, MAKEWPARAM(IDC_FILTER_CLEAR, BN_CLICKED), (LPARAM)GetDlgItem(parent, IDC_FILTER_CLEAR));
		}
		return 0;
	}
	return DefSubclassProc(hwnd, msg, wParam, lParam);
}

static BOOL GuiShouldShowNavigation(const GuiApp* app)
{
	return app != NULL && app->archive != NULL && (app->packType == XPK_PACK_LINUX || app->packType == XPK_PACK_WIN32);
}

static BOOL GuiShouldShowFilter(const GuiApp* app)
{
	return app != NULL && app->archive != NULL;
}

static void GuiLayoutMainWindow(GuiApp* app)
{
	RECT rcClient;
	RECT rcStatus;
	BOOL showNav;
	BOOL showFilter;
	int contentTop;
	int listTop;
	int actionRowTop;
	int actionButtonWidth;
	int actionButtonHeight;
	int navRowTop;
	int filterRowTop;
	int navControlHeight;
	int navButtonWidth;
	int navGoWidth;
	int filterClearWidth;
	int margin;
	int pathLeft;
	int pathWidth;
	int filterLeft;
	int filterWidth;
	int rowGap;

	if ( app == NULL || app->window == NULL || app->list == NULL || app->status == NULL || app->filterEdit == NULL || app->filterClear == NULL ||
		app->navBack == NULL || app->navForward == NULL || app->navRoot == NULL || app->navUp == NULL || app->navPath == NULL || app->navGo == NULL ||
		app->actNew == NULL || app->actOpen == NULL || app->actAdd == NULL || app->actExtract == NULL || app->actVerify == NULL || app->actInfo == NULL ) {
		return;
	}

	GetClientRect(app->window, &rcClient);
	SendMessageW(app->status, WM_SIZE, 0, 0);
	GetWindowRect(app->status, &rcStatus);

	showNav = GuiShouldShowNavigation(app);
	showFilter = GuiShouldShowFilter(app);
	margin = 6;
	rowGap = 4;
	actionRowTop = 6;
	actionButtonWidth = 78;
	actionButtonHeight = 26;
	navRowTop = actionRowTop + actionButtonHeight + rowGap;
	navControlHeight = 24;
	navButtonWidth = 64;
	navGoWidth = 52;
	filterClearWidth = 56;
	pathLeft = margin + navButtonWidth + margin + navButtonWidth + margin + navButtonWidth + margin + navButtonWidth + margin;
	pathWidth = rcClient.right - pathLeft - navGoWidth - (margin * 2);
	if ( pathWidth < 120 ) {
		pathWidth = 120;
	}
	filterRowTop = showNav ? (navRowTop + navControlHeight + rowGap) : navRowTop;
	filterLeft = margin;
	filterWidth = rcClient.right - filterLeft - filterClearWidth - (margin * 2);
	if ( filterWidth < 120 ) {
		filterWidth = 120;
	}
	contentTop = actionRowTop + actionButtonHeight + margin;
	if ( showNav ) {
		contentTop = navRowTop + navControlHeight + rowGap;
	}
	if ( showFilter ) {
		contentTop = filterRowTop + navControlHeight + margin;
	}
	listTop = contentTop;

	MoveWindow(app->actNew, margin, actionRowTop, actionButtonWidth, actionButtonHeight, TRUE);
	MoveWindow(app->actOpen, margin + (actionButtonWidth + margin) * 1, actionRowTop, actionButtonWidth, actionButtonHeight, TRUE);
	MoveWindow(app->actAdd, margin + (actionButtonWidth + margin) * 2, actionRowTop, actionButtonWidth, actionButtonHeight, TRUE);
	MoveWindow(app->actExtract, margin + (actionButtonWidth + margin) * 3, actionRowTop, actionButtonWidth, actionButtonHeight, TRUE);
	MoveWindow(app->actVerify, margin + (actionButtonWidth + margin) * 4, actionRowTop, actionButtonWidth, actionButtonHeight, TRUE);
	MoveWindow(app->actInfo, margin + (actionButtonWidth + margin) * 5, actionRowTop, actionButtonWidth, actionButtonHeight, TRUE);

	ShowWindow(app->navBack, showNav ? SW_SHOW : SW_HIDE);
	ShowWindow(app->navForward, showNav ? SW_SHOW : SW_HIDE);
	ShowWindow(app->navRoot, showNav ? SW_SHOW : SW_HIDE);
	ShowWindow(app->navUp, showNav ? SW_SHOW : SW_HIDE);
	ShowWindow(app->navPath, showNav ? SW_SHOW : SW_HIDE);
	ShowWindow(app->navGo, showNav ? SW_SHOW : SW_HIDE);
	ShowWindow(app->filterEdit, showFilter ? SW_SHOW : SW_HIDE);
	ShowWindow(app->filterClear, showFilter ? SW_SHOW : SW_HIDE);

	if ( showNav ) {
		MoveWindow(app->navBack, margin, navRowTop, navButtonWidth, navControlHeight, TRUE);
		MoveWindow(app->navForward, margin + navButtonWidth + margin, navRowTop, navButtonWidth, navControlHeight, TRUE);
		MoveWindow(app->navRoot, margin + (navButtonWidth + margin) * 2, navRowTop, navButtonWidth, navControlHeight, TRUE);
		MoveWindow(app->navUp, margin + (navButtonWidth + margin) * 3, navRowTop, navButtonWidth, navControlHeight, TRUE);
		MoveWindow(app->navPath, pathLeft, navRowTop, pathWidth, navControlHeight, TRUE);
		MoveWindow(app->navGo, pathLeft + pathWidth + margin, navRowTop, navGoWidth, navControlHeight, TRUE);
	}
	if ( showFilter ) {
		MoveWindow(app->filterEdit, filterLeft, filterRowTop, filterWidth, navControlHeight, TRUE);
		MoveWindow(app->filterClear, filterLeft + filterWidth + margin, filterRowTop, filterClearWidth, navControlHeight, TRUE);
	}

	MoveWindow(app->list, 0, listTop, rcClient.right, rcClient.bottom - listTop - (rcStatus.bottom - rcStatus.top), TRUE);
}

void GuiUpdateNavigationBar(GuiApp* app)
{
	WCHAR displayPath[XPKGUI_ITEM_TEXT + 4];
	WCHAR filterBuf[XPKGUI_ITEM_TEXT];
	BOOL showNav;
	BOOL showFilter;

	if ( app == NULL || app->navBack == NULL || app->navForward == NULL || app->navRoot == NULL || app->navUp == NULL || app->navPath == NULL || app->navGo == NULL || app->filterEdit == NULL || app->filterClear == NULL ) {
		return;
	}

	showNav = GuiShouldShowNavigation(app);
	showFilter = GuiShouldShowFilter(app);
	if ( showNav ) {
		EnableWindow(app->navBack, GuiArchiveCanBrowseBack(app));
		EnableWindow(app->navForward, GuiArchiveCanBrowseForward(app));
		if ( app->flatView ) {
			wcsncpy_s(displayPath, _countof(displayPath), L"[Flat View]", _TRUNCATE);
			SetWindowTextW(app->navPath, displayPath);
			EnableWindow(app->navPath, FALSE);
			EnableWindow(app->navGo, FALSE);
			EnableWindow(app->navRoot, FALSE);
			EnableWindow(app->navUp, FALSE);
		} else {
			if ( app->currentFolder[0] != L'\0' ) {
				_snwprintf_s(displayPath, _countof(displayPath), _TRUNCATE, L"/%s", app->currentFolder);
			} else {
				wcsncpy_s(displayPath, _countof(displayPath), L"/", _TRUNCATE);
			}
			SetWindowTextW(app->navPath, displayPath);
			EnableWindow(app->navPath, TRUE);
			EnableWindow(app->navGo, TRUE);
			EnableWindow(app->navRoot, app->currentFolder[0] != L'\0');
			EnableWindow(app->navUp, app->currentFolder[0] != L'\0');
		}
	} else {
		SetWindowTextW(app->navPath, L"");
		EnableWindow(app->navBack, FALSE);
		EnableWindow(app->navForward, FALSE);
		EnableWindow(app->navPath, FALSE);
		EnableWindow(app->navGo, FALSE);
		EnableWindow(app->navRoot, FALSE);
		EnableWindow(app->navUp, FALSE);
	}

	if ( showFilter ) {
		GetWindowTextW(app->filterEdit, filterBuf, _countof(filterBuf));
		if ( wcscmp(filterBuf, app->filterText) != 0 ) {
			SetWindowTextW(app->filterEdit, app->filterText);
		}
		EnableWindow(app->filterEdit, TRUE);
		EnableWindow(app->filterClear, app->filterText[0] != L'\0');
	} else {
		SetWindowTextW(app->filterEdit, L"");
		EnableWindow(app->filterEdit, FALSE);
		EnableWindow(app->filterClear, FALSE);
	}

	GuiLayoutMainWindow(app);
}

static void GuiShowListContextMenu(GuiApp* app, int itemIndex, POINT ptScreen)
{
	HMENU menu;
	BOOL hasArchive;
	BOOL canOpen;
	BOOL canView;
	BOOL canEditText;
	BOOL canExtract;
	BOOL canDelete;
	BOOL allowRename;
	BOOL canSetFileType;
	BOOL canSetPathAttr;
	BOOL canVerifySelection;
	BOOL canCopy;
	BOOL canCopyHash;
	BOOL canShowProperties;
	BOOL canShowInExplorer;
	BOOL canEdit;
	BOOL canReplace;
	BOOL canEditInfoExt;
	BOOL canSelectByPattern;
	BOOL canSelectSameExt;
	BOOL canSelectSameHash;
	BOOL canSelectDuplicates;
	BOOL canSelectDuplicateCopies;
	BOOL canDeleteDuplicateCopies;
	BOOL canInvertSelection;
	BOOL canLocateInTree;

	if ( app == NULL ) {
		return;
	}

	hasArchive = (app->archive != NULL);
	if ( !hasArchive ) {
		return;
	}

	if ( itemIndex >= 0 && !ListView_GetItemState(app->list, itemIndex, LVIS_SELECTED) ) {
		ListView_SetItemState(app->list, -1, 0, LVIS_SELECTED);
		ListView_SetItemState(app->list, itemIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		GuiSetMenuState(app);
		GuiUpdateStatus(app);
	}

	canOpen = GuiArchiveCanOpenSelection(app);
	canView = GuiArchiveCanViewSelection(app);
	canEditText = GuiArchiveCanEditTextSelection(app);
	canShowInExplorer = GuiArchiveCanShowSelectionInExplorer(app);
	canEdit = GuiArchiveCanEditSelection(app);
	canReplace = GuiArchiveCanReplaceSelection(app);
	canEditInfoExt = GuiArchiveCanEditSelectionInfoExt(app);
	canExtract = GuiArchiveCanExtractSelection(app);
	canDelete = GuiArchiveCanDeleteSelection(app);
	allowRename = GuiArchiveCanRenameSelection(app);
	canSetFileType = GuiArchiveCanSetSelectionFileType(app);
	canSetPathAttr = GuiArchiveCanSetSelectionPathAttr(app);
	canVerifySelection = GuiArchiveCanVerifySelection(app);
	canCopy = GuiArchiveCanCopySelection(app);
	canCopyHash = GuiArchiveCanCopySelectionHash(app);
	canShowProperties = canCopy;
	canSelectByPattern = GuiArchiveCanSelectByPattern(app);
	canSelectSameExt = GuiArchiveCanSelectSameExtension(app);
	canSelectSameHash = GuiArchiveCanSelectSameHash(app);
	canSelectDuplicates = GuiArchiveCanSelectDuplicateFiles(app);
	canSelectDuplicateCopies = GuiArchiveCanSelectDuplicateCopies(app);
	canDeleteDuplicateCopies = GuiArchiveCanDeleteDuplicateCopies(app);
	canInvertSelection = GuiArchiveCanInvertSelection(app);
	canLocateInTree = GuiArchiveCanLocateInTree(app);

	menu = CreatePopupMenu();
	if ( menu == NULL ) {
		return;
	}

	if ( canOpen || canView || canEditText || canShowInExplorer || canEdit || canReplace || canEditInfoExt || canExtract || canDelete || allowRename || canSetFileType || canSetPathAttr || canVerifySelection || canCopy || canCopyHash || canShowProperties || canSelectByPattern || canSelectSameExt || canSelectSameHash || canSelectDuplicates || canSelectDuplicateCopies || canDeleteDuplicateCopies || canInvertSelection || canLocateInTree ) {
		if ( canOpen ) {
			AppendMenuW(menu, MF_STRING, ID_ACTION_OPEN, L"Open");
			AppendMenuW(menu, MF_STRING, ID_ACTION_OPEN_WITH, L"Open With...");
		}
		if ( canView ) {
			AppendMenuW(menu, MF_STRING, ID_ACTION_VIEW, L"View");
		}
		if ( canEditText ) {
			AppendMenuW(menu, MF_STRING, ID_ACTION_EDIT_TEXT, L"Edit Text...");
		}
		if ( canEdit ) {
			AppendMenuW(menu, MF_STRING, ID_ACTION_EDIT, L"Edit");
		}
		if ( canReplace ) {
			AppendMenuW(menu, MF_STRING, ID_ACTION_REPLACE, L"Replace...");
		}
		if ( canShowInExplorer ) {
			AppendMenuW(menu, MF_STRING, ID_ACTION_SHOW_IN_EXPLORER, L"Show In Explorer");
		}
		if ( canEditInfoExt ) {
			AppendMenuW(menu, MF_STRING, ID_ACTION_INFO_EXT, L"View/Edit Entry InfoExt...");
			AppendMenuW(menu, MF_STRING, ID_ACTION_INFO_EXT_IMPORT, L"Import Entry InfoExt...");
			AppendMenuW(menu, MF_STRING, ID_ACTION_INFO_EXT_EXPORT, L"Export Entry InfoExt...");
			AppendMenuW(menu, MF_STRING, ID_ACTION_INFO_EXT_CLEAR, L"Clear Entry InfoExt");
		}
		if ( canLocateInTree ) {
			AppendMenuW(menu, MF_STRING, ID_VIEW_LOCATE_IN_TREE, L"Locate In Tree");
		}
		if ( canExtract ) {
			AppendMenuW(menu, MF_STRING, ID_ACTION_EXTRACT, L"Extract Selected...");
		}
		if ( canVerifySelection ) {
			AppendMenuW(menu, MF_STRING, ID_ACTION_VERIFY, L"Verify Selected");
		}
		if ( canDelete ) {
			AppendMenuW(menu, MF_STRING, ID_ACTION_DELETE, L"Delete");
		}
		if ( allowRename ) {
			AppendMenuW(menu, MF_STRING, ID_ACTION_RENAME, L"Rename...");
		}
		if ( canSetFileType ) {
			AppendMenuW(menu, MF_STRING, ID_ACTION_SET_FILE_TYPE, L"Set File Type...");
		}
		if ( canSetPathAttr ) {
			AppendMenuW(menu, MF_STRING, ID_ACTION_SET_PATH_ATTR, L"Set Platform Attr...");
		}
		if ( canCopy ) {
			AppendMenuW(menu, MF_STRING, ID_EDIT_COPY_PATHS, L"Copy Selected Names/Paths");
		}
		if ( canCopyHash ) {
			AppendMenuW(menu, MF_STRING, ID_EDIT_COPY_HASH, L"Copy Entry Hash");
		}
		if ( canShowProperties ) {
			AppendMenuW(menu, MF_STRING, ID_ACTION_PROPERTIES, L"Properties");
		}
		if ( canSelectByPattern ) {
			AppendMenuW(menu, MF_STRING, ID_EDIT_SELECT_BY_PATTERN, L"Select By Pattern...");
		}
		if ( canSelectSameExt ) {
			AppendMenuW(menu, MF_STRING, ID_EDIT_SELECT_SAME_EXT, L"Select Same Extension");
		}
		if ( canSelectSameHash ) {
			AppendMenuW(menu, MF_STRING, ID_EDIT_SELECT_SAME_HASH, L"Select Same Hash");
		}
		if ( canSelectDuplicates ) {
			AppendMenuW(menu, MF_STRING, ID_EDIT_SELECT_DUPLICATES, L"Select Duplicate Files");
		}
		if ( canSelectDuplicateCopies ) {
			AppendMenuW(menu, MF_STRING, ID_EDIT_SELECT_DUP_COPIES, L"Select Duplicate Copies");
		}
		if ( canDeleteDuplicateCopies ) {
			AppendMenuW(menu, MF_STRING, ID_EDIT_DELETE_DUP_COPIES, L"Delete Duplicate Copies...");
		}
		if ( canInvertSelection ) {
			AppendMenuW(menu, MF_STRING, ID_EDIT_INVERT_SELECTION, L"Invert Selection");
		}
		AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
	}
	AppendMenuW(menu, MF_STRING, ID_ACTION_EXTRACT_ALL, L"Extract All...");
	AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
	AppendMenuW(menu, MF_STRING, ID_ACTION_ADD_FILES, L"Add Files...");
	AppendMenuW(menu, MF_STRING, ID_ACTION_ADD_FOLDER, L"Add Folder...");
	AppendMenuW(menu, MF_STRING, ID_ACTION_ADD_EMPTY, L"Add Empty Entry...");
	AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
	AppendMenuW(menu, MF_STRING, ID_ACTION_VERIFY_ALL, L"Verify All");
	AppendMenuW(menu, MF_STRING, ID_FILE_PROPERTIES, L"Archive Properties");

	SetForegroundWindow(app->window);
	TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptScreen.x, ptScreen.y, 0, app->window, NULL);
	DestroyMenu(menu);
}

void GuiSetMenuState(GuiApp* app)
{
	HMENU hMenu;
	BOOL hasArchive;
	BOOL canOpen;
	BOOL canView;
	BOOL canEditText;
	BOOL canExtract;
	BOOL canDelete;
	BOOL canRename;
	BOOL canSetFileType;
	BOOL canSetPathAttr;
	BOOL canVerifySelection;
	BOOL canCopy;
	BOOL canCopyHash;
	BOOL canShowProperties;
	BOOL canShowInExplorer;
	BOOL canEdit;
	BOOL canReplace;
	BOOL canEditInfoExt;
	BOOL canSelectByPattern;
	BOOL canSelectSameExt;
	BOOL canSelectSameHash;
	BOOL canSelectDuplicates;
	BOOL canSelectDuplicateCopies;
	BOOL canDeleteDuplicateCopies;
	BOOL canInvertSelection;
	BOOL canLocateInTree;
	BOOL canBrowseBack;
	BOOL canBrowseForward;

	if ( app->window == NULL ) {
		return;
	}

	hMenu = GetMenu(app->window);
	if ( hMenu == NULL ) {
		return;
	}

	GuiUpdateRecentMenu(app, hMenu);

	hasArchive = (app->archive != NULL);
	canOpen = hasArchive && GuiArchiveCanOpenSelection(app);
	canView = hasArchive && GuiArchiveCanViewSelection(app);
	canEditText = hasArchive && GuiArchiveCanEditTextSelection(app);
	canShowInExplorer = hasArchive && GuiArchiveCanShowSelectionInExplorer(app);
	canEdit = hasArchive && GuiArchiveCanEditSelection(app);
	canReplace = hasArchive && GuiArchiveCanReplaceSelection(app);
	canEditInfoExt = hasArchive && GuiArchiveCanEditSelectionInfoExt(app);
	canExtract = hasArchive && GuiArchiveCanExtractSelection(app);
	canDelete = hasArchive && GuiArchiveCanDeleteSelection(app);
	canRename = hasArchive && GuiArchiveCanRenameSelection(app);
	canSetFileType = hasArchive && GuiArchiveCanSetSelectionFileType(app);
	canSetPathAttr = hasArchive && GuiArchiveCanSetSelectionPathAttr(app);
	canVerifySelection = hasArchive && GuiArchiveCanVerifySelection(app);
	canCopy = hasArchive && GuiArchiveCanCopySelection(app);
	canCopyHash = hasArchive && GuiArchiveCanCopySelectionHash(app);
	canShowProperties = hasArchive && GuiArchiveCanCopySelection(app);
	canSelectByPattern = hasArchive && GuiArchiveCanSelectByPattern(app);
	canSelectSameExt = hasArchive && GuiArchiveCanSelectSameExtension(app);
	canSelectSameHash = hasArchive && GuiArchiveCanSelectSameHash(app);
	canSelectDuplicates = hasArchive && GuiArchiveCanSelectDuplicateFiles(app);
	canSelectDuplicateCopies = hasArchive && GuiArchiveCanSelectDuplicateCopies(app);
	canDeleteDuplicateCopies = hasArchive && GuiArchiveCanDeleteDuplicateCopies(app);
	canInvertSelection = hasArchive && GuiArchiveCanInvertSelection(app);
	canLocateInTree = hasArchive && GuiArchiveCanLocateInTree(app);
	canBrowseBack = hasArchive && GuiArchiveCanBrowseBack(app);
	canBrowseForward = hasArchive && GuiArchiveCanBrowseForward(app);

	EnableMenuItem(hMenu, ID_FILE_CLOSE, MF_BYCOMMAND | (hasArchive ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_FILE_SAVE, MF_BYCOMMAND | (hasArchive ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_FILE_REBUILD, MF_BYCOMMAND | (hasArchive ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_FILE_PROPERTIES, MF_BYCOMMAND | (hasArchive ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_EDIT_COPY_PATHS, MF_BYCOMMAND | (canCopy ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_EDIT_COPY_HASH, MF_BYCOMMAND | (canCopyHash ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_EDIT_SELECT_BY_PATTERN, MF_BYCOMMAND | (canSelectByPattern ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_EDIT_SELECT_SAME_EXT, MF_BYCOMMAND | (canSelectSameExt ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_EDIT_SELECT_SAME_HASH, MF_BYCOMMAND | (canSelectSameHash ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_EDIT_SELECT_DUPLICATES, MF_BYCOMMAND | (canSelectDuplicates ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_EDIT_SELECT_DUP_COPIES, MF_BYCOMMAND | (canSelectDuplicateCopies ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_EDIT_DELETE_DUP_COPIES, MF_BYCOMMAND | (canDeleteDuplicateCopies ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_EDIT_INVERT_SELECTION, MF_BYCOMMAND | (canInvertSelection ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_OPEN, MF_BYCOMMAND | (canOpen ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_OPEN_WITH, MF_BYCOMMAND | (canOpen ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_VIEW, MF_BYCOMMAND | (canView ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_EDIT_TEXT, MF_BYCOMMAND | (canEditText ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_EDIT, MF_BYCOMMAND | (canEdit ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_REPLACE, MF_BYCOMMAND | (canReplace ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_SHOW_IN_EXPLORER, MF_BYCOMMAND | (canShowInExplorer ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_INFO_EXT, MF_BYCOMMAND | (canEditInfoExt ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_INFO_EXT_IMPORT, MF_BYCOMMAND | (canEditInfoExt ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_INFO_EXT_EXPORT, MF_BYCOMMAND | (canEditInfoExt ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_INFO_EXT_CLEAR, MF_BYCOMMAND | (canEditInfoExt ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_PROPERTIES, MF_BYCOMMAND | (canShowProperties ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_ADD_FILES, MF_BYCOMMAND | (hasArchive ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_ADD_FOLDER, MF_BYCOMMAND | (hasArchive ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_ADD_EMPTY, MF_BYCOMMAND | (hasArchive ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_EXTRACT, MF_BYCOMMAND | (canExtract ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_EXTRACT_ALL, MF_BYCOMMAND | (hasArchive ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_DELETE, MF_BYCOMMAND | (canDelete ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_RENAME, MF_BYCOMMAND | (canRename ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_SET_FILE_TYPE, MF_BYCOMMAND | (canSetFileType ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_SET_PATH_ATTR, MF_BYCOMMAND | (canSetPathAttr ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_VERIFY, MF_BYCOMMAND | (canVerifySelection ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_ACTION_VERIFY_ALL, MF_BYCOMMAND | (hasArchive ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_TOOLS_ARCHIVE_SETTINGS, MF_BYCOMMAND | (hasArchive ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_TOOLS_META_EDIT, MF_BYCOMMAND | (hasArchive ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_TOOLS_META_IMPORT, MF_BYCOMMAND | (hasArchive ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_TOOLS_META_EXPORT, MF_BYCOMMAND | (hasArchive ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_TOOLS_META_CLEAR, MF_BYCOMMAND | (hasArchive ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_VIEW_BACK, MF_BYCOMMAND | (canBrowseBack ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_VIEW_FORWARD, MF_BYCOMMAND | (canBrowseForward ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_VIEW_FLAT, MF_BYCOMMAND | ((hasArchive && (app->packType == XPK_PACK_LINUX || app->packType == XPK_PACK_WIN32)) ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, ID_VIEW_LOCATE_IN_TREE, MF_BYCOMMAND | (canLocateInTree ? MF_ENABLED : MF_GRAYED));
	CheckMenuItem(hMenu, ID_VIEW_FLAT, MF_BYCOMMAND | (app->flatView ? MF_CHECKED : MF_UNCHECKED));

	if ( app->actNew != NULL ) EnableWindow(app->actNew, TRUE);
	if ( app->actOpen != NULL ) EnableWindow(app->actOpen, TRUE);
	if ( app->actAdd != NULL ) EnableWindow(app->actAdd, TRUE);
	if ( app->actExtract != NULL ) EnableWindow(app->actExtract, hasArchive);
	if ( app->actVerify != NULL ) EnableWindow(app->actVerify, hasArchive);
	if ( app->actInfo != NULL ) EnableWindow(app->actInfo, hasArchive);
	DrawMenuBar(app->window);
}

BOOL GuiRunImmediateLaunch(GuiApp* app)
{
	WCHAR destPath[MAX_PATH];
	BOOL ok;
	BOOL cancelled;

	switch ( app->launch.command ) {
		case GUI_LAUNCH_SHELL_EXTRACT:
			if ( app->launch.destPath[0] == L'\0' ) {
				if ( !GuiPickFolderDialog(NULL, destPath, _countof(destPath), L"选择解压目录") ) {
					return TRUE;
				}
			} else {
				wcsncpy_s(destPath, _countof(destPath), app->launch.destPath, _TRUNCATE);
			}
			cancelled = FALSE;
			ok = GuiArchiveExtractPathTask(app->launch.archivePath, destPath, &cancelled);
			if ( cancelled ) {
				return TRUE;
			}
			MessageBoxW(NULL, ok ? L"解压完成。" : L"解压失败。", XPKGUI_APP_TITLE, MB_OK | (ok ? MB_ICONINFORMATION : MB_ICONERROR));
			return TRUE;
		case GUI_LAUNCH_SHELL_EXTRACT_AUTO:
			if ( !GuiSuggestExtractFolderPath(app->launch.archivePath, destPath, _countof(destPath), TRUE) ) {
				MessageBoxW(NULL, L"无法生成目标目录。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
				return TRUE;
			}
			cancelled = FALSE;
			ok = GuiArchiveExtractPathTask(app->launch.archivePath, destPath, &cancelled);
			if ( cancelled ) {
				return TRUE;
			}
			MessageBoxW(NULL, ok ? L"解压完成。" : L"解压失败。", XPKGUI_APP_TITLE, MB_OK | (ok ? MB_ICONINFORMATION : MB_ICONERROR));
			return TRUE;
		case GUI_LAUNCH_SHELL_EXTRACT_HERE:
			wcsncpy_s(destPath, _countof(destPath), app->launch.archivePath, _TRUNCATE);
			PathRemoveFileSpecW(destPath);
			cancelled = FALSE;
			ok = GuiArchiveExtractPathTask(app->launch.archivePath, destPath, &cancelled);
			if ( cancelled ) {
				return TRUE;
			}
			MessageBoxW(NULL, ok ? L"解压完成。" : L"解压失败。", XPKGUI_APP_TITLE, MB_OK | (ok ? MB_ICONINFORMATION : MB_ICONERROR));
			return TRUE;
		case GUI_LAUNCH_SHELL_VERIFY:
			cancelled = FALSE;
			ok = GuiArchiveVerifyPathTask(app->launch.archivePath, &cancelled);
			if ( cancelled ) {
				return TRUE;
			}
			MessageBoxW(NULL, ok ? L"校验通过。" : L"校验失败。", XPKGUI_APP_TITLE, MB_OK | (ok ? MB_ICONINFORMATION : MB_ICONERROR));
			return TRUE;
		case GUI_LAUNCH_SHELL_PROPERTIES:
			if ( !GuiArchiveShowPropertiesPath(app->launch.archivePath) ) {
				MessageBoxW(NULL, L"读取属性失败。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
			}
			return TRUE;
		default:
			return FALSE;
	}
}

BOOL GuiHandleGuiLaunch(GuiApp* app)
{
	switch ( app->launch.command ) {
		case GUI_LAUNCH_OPEN:
			return GuiArchiveOpenPath(app, app->launch.archivePath, FALSE);
		case GUI_LAUNCH_SHELL_ADD:
			return GuiArchiveCreateFromSelection(app, app->launch.inputCount, app->launch.inputs, FALSE, FALSE);
		case GUI_LAUNCH_SHELL_ADD_AUTO:
			if ( !GuiArchiveCreateFromSelection(app, app->launch.inputCount, app->launch.inputs, TRUE, TRUE) ) {
				return FALSE;
			}
			PostMessageW(app->window, WM_CLOSE, 0, 0);
			return TRUE;
		default:
			return TRUE;
	}
}

static void GuiHandleFileAddDialog(GuiApp* app, BOOL createIfNoArchive)
{
	WCHAR multiBuf[32768];
	WCHAR* inputs[XPKGUI_MAX_INPUTS];
	WCHAR folder[MAX_PATH];
	WCHAR* p;
	int count;

	if ( !GuiPickFilesDialog(app->window, multiBuf, _countof(multiBuf)) ) {
		return;
	}

	count = 0;
	p = multiBuf;
	if ( p[wcslen(p) + 1] == L'\0' ) {
		inputs[count++] = _wcsdup(p);
	} else {
		wcsncpy_s(folder, _countof(folder), p, _TRUNCATE);
		p += wcslen(p) + 1;
		while ( *p != L'\0' && count < XPKGUI_MAX_INPUTS ) {
			WCHAR fullPath[MAX_PATH];
			_snwprintf_s(fullPath, _countof(fullPath), _TRUNCATE, L"%s\\%s", folder, p);
			inputs[count++] = _wcsdup(fullPath);
			p += wcslen(p) + 1;
		}
	}

	if ( app->archive != NULL ) {
		GuiArchiveAddSelection(app, count, inputs);
	} else if ( createIfNoArchive ) {
		GuiArchiveCreateFromSelection(app, count, inputs, FALSE, FALSE);
	}
	while ( count-- > 0 ) {
		free(inputs[count]);
	}
}

static void GuiHandleDropFiles(GuiApp* app, HDROP hDrop)
{
	UINT fileCount;
	UINT i;
	WCHAR* dropped[XPKGUI_MAX_INPUTS];
	WCHAR buf[MAX_PATH];

	fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
	if ( fileCount > XPKGUI_MAX_INPUTS ) {
		fileCount = XPKGUI_MAX_INPUTS;
	}
	for ( i = 0; i < fileCount; ++i ) {
		DragQueryFileW(hDrop, i, buf, _countof(buf));
		dropped[i] = _wcsdup(buf);
	}
	DragFinish(hDrop);

	if ( app->archive == NULL && fileCount == 1 && PathMatchSpecW(dropped[0], L"*.xpk") ) {
		GuiArchiveOpenPath(app, dropped[0], FALSE);
	} else if ( app->archive != NULL ) {
		GuiArchiveAddSelection(app, (int)fileCount, dropped);
	} else {
		GuiArchiveCreateFromSelection(app, (int)fileCount, dropped, FALSE, FALSE);
	}

	for ( i = 0; i < fileCount; ++i ) {
		free(dropped[i]);
	}
}

static LRESULT CALLBACK GuiMainWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	GuiApp* app;

	app = (GuiApp*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	switch ( msg ) {
		case WM_CREATE:
		{
			CREATESTRUCTW* cs;
			cs = (CREATESTRUCTW*)lParam;
			app = (GuiApp*)cs->lpCreateParams;
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)app);
			app->window = hwnd;

			app->actNew = CreateWindowExW(
				0,
				L"Button",
				L"New",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
				0, 0, 78, 26,
				hwnd,
				(HMENU)ID_FILE_NEW,
				app->instance,
				NULL);

			app->actOpen = CreateWindowExW(
				0,
				L"Button",
				L"Open",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
				0, 0, 78, 26,
				hwnd,
				(HMENU)ID_FILE_OPEN,
				app->instance,
				NULL);

			app->actAdd = CreateWindowExW(
				0,
				L"Button",
				L"Add",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
				0, 0, 78, 26,
				hwnd,
				(HMENU)ID_ACTION_ADD_SMART,
				app->instance,
				NULL);

			app->actExtract = CreateWindowExW(
				0,
				L"Button",
				L"Extract",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
				0, 0, 78, 26,
				hwnd,
				(HMENU)ID_ACTION_EXTRACT_SMART,
				app->instance,
				NULL);

			app->actVerify = CreateWindowExW(
				0,
				L"Button",
				L"Verify",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
				0, 0, 78, 26,
				hwnd,
				(HMENU)ID_ACTION_VERIFY_SMART,
				app->instance,
				NULL);

			app->actInfo = CreateWindowExW(
				0,
				L"Button",
				L"Info",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
				0, 0, 78, 26,
				hwnd,
				(HMENU)ID_ACTION_INFO_SMART,
				app->instance,
				NULL);

			app->navBack = CreateWindowExW(
				0,
				L"Button",
				L"Back",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
				0, 0, 64, 24,
				hwnd,
				(HMENU)IDC_NAV_BACK,
				app->instance,
				NULL);

			app->navForward = CreateWindowExW(
				0,
				L"Button",
				L"Forward",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
				0, 0, 64, 24,
				hwnd,
				(HMENU)IDC_NAV_FORWARD,
				app->instance,
				NULL);

			app->navRoot = CreateWindowExW(
				0,
				L"Button",
				L"Root",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
				0, 0, 64, 24,
				hwnd,
				(HMENU)IDC_NAV_ROOT,
				app->instance,
				NULL);

			app->navUp = CreateWindowExW(
				0,
				L"Button",
				L"Up",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
				0, 0, 64, 24,
				hwnd,
				(HMENU)IDC_NAV_UP,
				app->instance,
				NULL);

			app->navPath = CreateWindowExW(
				WS_EX_CLIENTEDGE,
				L"Edit",
				L"",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
				0, 0, 100, 24,
				hwnd,
				(HMENU)IDC_NAV_PATH,
				app->instance,
				NULL);
			SetWindowSubclass(app->navPath, GuiNavPathEditProc, 1, 0);

			app->navGo = CreateWindowExW(
				0,
				L"Button",
				L"Go",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
				0, 0, 52, 24,
				hwnd,
				(HMENU)IDC_NAV_GO,
				app->instance,
				NULL);

			app->filterEdit = CreateWindowExW(
				WS_EX_CLIENTEDGE,
				L"Edit",
				L"",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
				0, 0, 100, 24,
				hwnd,
				(HMENU)IDC_FILTER_EDIT,
				app->instance,
				NULL);
			SetWindowSubclass(app->filterEdit, GuiFilterEditProc, 2, 0);

			app->filterClear = CreateWindowExW(
				0,
				L"Button",
				L"Clear",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
				0, 0, 56, 24,
				hwnd,
				(HMENU)IDC_FILTER_CLEAR,
				app->instance,
				NULL);

			app->list = CreateWindowExW(
				WS_EX_CLIENTEDGE,
				WC_LISTVIEWW,
				L"",
				WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS,
				0, 0, 100, 100,
				hwnd,
				(HMENU)IDC_MAIN_LIST,
				app->instance,
				NULL);
			ListView_SetExtendedListViewStyle(app->list, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

			app->status = CreateWindowExW(
				0,
				STATUSCLASSNAMEW,
				L"",
				WS_CHILD | WS_VISIBLE,
				0, 0, 0, 0,
				hwnd,
				(HMENU)IDC_MAIN_STATUS,
				app->instance,
				NULL);

			DragAcceptFiles(hwnd, TRUE);
			GuiArchiveRefreshView(app);
			return 0;
		}
		case WM_SIZE:
			if ( app != NULL ) {
				GuiLayoutMainWindow(app);
			}
			return 0;
		case WM_ACTIVATEAPP:
			if ( app != NULL && wParam ) {
				GuiArchiveCheckExternalChanges(app);
			}
			break;
		case WM_DROPFILES:
			if ( app != NULL ) {
				GuiHandleDropFiles(app, (HDROP)wParam);
			}
			return 0;
		case WM_COMMAND:
			if ( app == NULL ) {
				break;
			}
			if ( LOWORD(wParam) >= ID_FILE_RECENT_FIRST && LOWORD(wParam) <= ID_FILE_RECENT_LAST ) {
				const WCHAR* recentPath;

				recentPath = GuiGetRecentArchivePath(app, LOWORD(wParam));
				if ( recentPath != NULL ) {
					GuiArchiveOpenPath(app, recentPath, FALSE);
				}
				return 0;
			}
			switch ( LOWORD(wParam) ) {
				case ID_FILE_NEW:
				{
					GuiArchiveOptions options;
					WCHAR pathBuf[MAX_PATH];
					pathBuf[0] = L'\0';
					if ( GuiSaveArchiveDialog(hwnd, pathBuf, _countof(pathBuf)) && GuiArchivePromptNew(app, pathBuf, &options) ) {
						GuiArchiveCreateWithOptions(app, &options);
					}
					return 0;
				}
				case ID_FILE_OPEN:
				{
					WCHAR pathBuf[MAX_PATH];
					if ( GuiOpenArchiveDialog(hwnd, pathBuf, _countof(pathBuf)) ) {
						GuiArchiveOpenPath(app, pathBuf, FALSE);
					}
					return 0;
				}
				case ID_FILE_CLOSE:
					GuiAppCloseArchive(app);
					ListView_DeleteAllItems(app->list);
					GuiSetMenuState(app);
					GuiUpdateNavigationBar(app);
					GuiUpdateStatus(app);
					GuiUpdateTitle(app);
					return 0;
				case ID_FILE_SAVE:
					GuiArchiveSave(app);
					return 0;
				case ID_FILE_REBUILD:
					GuiArchiveBuild(app);
					return 0;
				case ID_FILE_PROPERTIES:
					GuiArchiveShowProperties(app);
					return 0;
				case ID_FILE_EXIT:
					DestroyWindow(hwnd);
					return 0;
				case ID_FILE_RECENT_CLEAR:
					GuiClearRecentArchives(app);
					GuiSetMenuState(app);
					return 0;
				case ID_EDIT_COPY_PATHS:
					GuiArchiveCopySelectionPaths(app);
					return 0;
				case ID_EDIT_COPY_HASH:
					GuiArchiveCopySelectionHash(app);
					return 0;
				case ID_EDIT_SELECT_BY_PATTERN:
					GuiArchiveSelectByPattern(app);
					return 0;
				case ID_EDIT_SELECT_SAME_EXT:
					GuiArchiveSelectSameExtension(app);
					return 0;
				case ID_EDIT_SELECT_SAME_HASH:
					GuiArchiveSelectSameHash(app);
					return 0;
				case ID_EDIT_SELECT_DUPLICATES:
					GuiArchiveSelectDuplicateFiles(app);
					return 0;
				case ID_EDIT_SELECT_DUP_COPIES:
					GuiArchiveSelectDuplicateCopies(app);
					return 0;
				case ID_EDIT_DELETE_DUP_COPIES:
					GuiArchiveDeleteDuplicateCopies(app);
					return 0;
				case ID_EDIT_INVERT_SELECTION:
					GuiArchiveInvertSelection(app);
					return 0;
				case ID_ACTION_OPEN:
					GuiArchiveOpenSelection(app);
					return 0;
				case ID_ACTION_VIEW:
					GuiArchiveViewSelection(app);
					return 0;
				case ID_ACTION_EDIT_TEXT:
					GuiArchiveEditSelectionText(app);
					return 0;
				case ID_ACTION_EDIT:
					GuiArchiveEditSelection(app);
					return 0;
				case ID_ACTION_REPLACE:
					GuiArchiveReplaceSelection(app);
					return 0;
				case ID_ACTION_ADD_EMPTY:
					GuiArchiveAddEmptyEntry(app);
					return 0;
				case ID_ACTION_OPEN_WITH:
					GuiArchiveOpenSelectionWith(app);
					return 0;
				case ID_ACTION_SHOW_IN_EXPLORER:
					GuiArchiveShowSelectionInExplorer(app);
					return 0;
				case ID_ACTION_INFO_EXT:
					GuiArchiveEditSelectionInfoExt(app);
					return 0;
				case ID_ACTION_INFO_EXT_IMPORT:
					GuiArchiveImportSelectionInfoExt(app);
					return 0;
				case ID_ACTION_INFO_EXT_EXPORT:
					GuiArchiveExportSelectionInfoExt(app);
					return 0;
				case ID_ACTION_INFO_EXT_CLEAR:
					GuiArchiveClearSelectionInfoExt(app);
					return 0;
				case ID_ACTION_PROPERTIES:
					GuiArchiveShowSelectionProperties(app);
					return 0;
				case ID_ACTION_ADD_FILES:
					GuiHandleFileAddDialog(app, FALSE);
					return 0;
				case ID_ACTION_ADD_SMART:
					GuiHandleFileAddDialog(app, TRUE);
					return 0;
				case ID_ACTION_ADD_FOLDER:
				{
					WCHAR folderPath[MAX_PATH];
					WCHAR* one[1];
					if ( GuiPickFolderDialog(hwnd, folderPath, _countof(folderPath), L"选择要添加的目录") ) {
						one[0] = folderPath;
						GuiArchiveAddSelection(app, 1, one);
					}
					return 0;
				}
				case ID_ACTION_EXTRACT:
					GuiArchiveExtractSelection(app, FALSE, NULL);
					return 0;
				case ID_ACTION_EXTRACT_ALL:
					GuiArchiveExtractSelection(app, TRUE, NULL);
					return 0;
				case ID_ACTION_EXTRACT_SMART:
					if ( GuiArchiveCanExtractSelection(app) ) {
						GuiArchiveExtractSelection(app, FALSE, NULL);
					} else {
						GuiArchiveExtractSelection(app, TRUE, NULL);
					}
					return 0;
				case ID_ACTION_DELETE:
					GuiArchiveRemoveSelection(app);
					return 0;
				case ID_ACTION_RENAME:
					GuiArchiveRenameSelection(app);
					return 0;
				case ID_ACTION_SET_FILE_TYPE:
					GuiArchiveSetSelectionFileType(app);
					return 0;
				case ID_ACTION_SET_PATH_ATTR:
					GuiArchiveSetSelectionPathAttr(app);
					return 0;
				case ID_ACTION_VERIFY:
					GuiArchiveVerifySelection(app);
					return 0;
				case ID_ACTION_VERIFY_ALL:
					GuiArchiveVerify(app);
					return 0;
				case ID_ACTION_VERIFY_SMART:
					if ( GuiArchiveCanVerifySelection(app) ) {
						GuiArchiveVerifySelection(app);
					} else {
						GuiArchiveVerify(app);
					}
					return 0;
				case ID_ACTION_INFO_SMART:
					if ( GuiArchiveCanCopySelection(app) ) {
						GuiArchiveShowSelectionProperties(app);
					} else {
						GuiArchiveShowProperties(app);
					}
					return 0;
				case ID_TOOLS_ARCHIVE_SETTINGS:
				{
					GuiArchiveOptions options;
					if ( GuiArchivePromptSettings(app, &options) ) {
						if ( !GuiArchiveApplyOptions(app, &options, TRUE) ) {
							GuiShowArchiveError(app, L"更新归档设置");
						} else {
							MessageBoxW(hwnd, L"`solid` / `volume` 等布局设置会在下一次“重构归档”后正式生效。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
						}
					}
					return 0;
				}
				case ID_TOOLS_META_EDIT:
					GuiArchiveMetaEdit(app);
					return 0;
				case ID_TOOLS_META_IMPORT:
					GuiArchiveMetaImport(app);
					return 0;
				case ID_TOOLS_META_EXPORT:
					GuiArchiveMetaExport(app);
					return 0;
				case ID_TOOLS_META_CLEAR:
					GuiArchiveMetaClear(app);
					return 0;
				case ID_VIEW_REFRESH:
					GuiArchiveReloadFromDisk(app);
					return 0;
				case ID_VIEW_BACK:
					GuiArchiveBrowseBack(app);
					return 0;
				case ID_VIEW_FORWARD:
					GuiArchiveBrowseForward(app);
					return 0;
				case ID_VIEW_FLAT:
					if ( app->archive != NULL && (app->packType == XPK_PACK_LINUX || app->packType == XPK_PACK_WIN32) ) {
						GuiArchiveToggleFlatView(app);
					}
					return 0;
				case ID_VIEW_LOCATE_IN_TREE:
					GuiArchiveLocateInTree(app);
					return 0;
				case ID_VIEW_FOCUS_FILTER:
					GuiFocusEditControl(app->filterEdit);
					return 0;
				case ID_VIEW_FOCUS_PATH:
					if ( GuiShouldShowNavigation(app) && !app->flatView && IsWindowEnabled(app->navPath) ) {
						GuiFocusEditControl(app->navPath);
					} else {
						GuiFocusEditControl(app->filterEdit);
					}
					return 0;
				case ID_VIEW_PARENT:
					GuiArchiveBrowseParent(app);
					return 0;
				case ID_VIEW_SELECT_ALL:
					GuiSelectAllVisibleItems(app);
					return 0;
				case IDC_FILTER_EDIT:
					if ( HIWORD(wParam) == EN_CHANGE ) {
						WCHAR filterBuf[XPKGUI_ITEM_TEXT];

						GetWindowTextW(app->filterEdit, filterBuf, _countof(filterBuf));
						GuiArchiveSetFilter(app, filterBuf);
					}
					return 0;
				case IDC_FILTER_CLEAR:
					if ( HIWORD(wParam) == BN_CLICKED ) {
						SetWindowTextW(app->filterEdit, L"");
						SetFocus(app->filterEdit);
					}
					return 0;
				case IDC_NAV_ROOT:
					if ( HIWORD(wParam) == BN_CLICKED ) {
						GuiArchiveBrowseRoot(app);
					}
					return 0;
				case IDC_NAV_BACK:
					if ( HIWORD(wParam) == BN_CLICKED ) {
						GuiArchiveBrowseBack(app);
					}
					return 0;
				case IDC_NAV_FORWARD:
					if ( HIWORD(wParam) == BN_CLICKED ) {
						GuiArchiveBrowseForward(app);
					}
					return 0;
				case IDC_NAV_UP:
					if ( HIWORD(wParam) == BN_CLICKED ) {
						GuiArchiveBrowseParent(app);
					}
					return 0;
				case IDC_NAV_GO:
					if ( HIWORD(wParam) == BN_CLICKED ) {
						WCHAR pathBuf[XPKGUI_ITEM_TEXT];

						GetWindowTextW(app->navPath, pathBuf, _countof(pathBuf));
						if ( !GuiArchiveNavigateToFolder(app, pathBuf) ) {
							MessageBoxW(hwnd, L"包内路径不存在。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
							GuiUpdateNavigationBar(app);
						}
					}
					return 0;
				case ID_HELP_ABOUT:
					MessageBoxW(hwnd, L"xpkgui\n\nWindows-only xPack archive manager with Explorer shell integration.", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
					return 0;
			}
			break;
		case WM_NOTIFY:
			if ( app != NULL ) {
				LPNMHDR hdr;

				hdr = (LPNMHDR)lParam;
				if ( hdr != NULL && hdr->idFrom == IDC_MAIN_LIST ) {
					switch ( hdr->code ) {
						case LVN_ITEMCHANGED:
						{
							const NMLISTVIEW* view;

							view = (const NMLISTVIEW*)lParam;
							if ( (view->uChanged & LVIF_STATE) != 0 && ((view->uOldState ^ view->uNewState) & LVIS_SELECTED) != 0 ) {
								GuiSetMenuState(app);
								GuiUpdateStatus(app);
							}
							return 0;
						}
						case LVN_COLUMNCLICK:
							GuiArchiveToggleSort(app, ((const NMLISTVIEW*)lParam)->iSubItem);
							return 0;
						case LVN_KEYDOWN:
						{
							const NMLVKEYDOWN* key;

							key = (const NMLVKEYDOWN*)lParam;
							if ( key->wVKey == VK_RETURN ) {
								GuiArchiveActivateSelection(app);
								return 0;
							}
							if ( key->wVKey == VK_BACK ) {
								GuiArchiveBrowseParent(app);
								return 0;
							}
							if ( key->wVKey == VK_F2 ) {
								GuiArchiveRenameSelection(app);
								return 0;
							}
							if ( key->wVKey == VK_DELETE ) {
								GuiArchiveRemoveSelection(app);
								return 0;
							}
							break;
						}
						case NM_DBLCLK:
							GuiArchiveActivateSelection(app);
							return 0;
						case NM_RCLICK:
						{
							const NMITEMACTIVATE* activate;
							POINT ptScreen;

							activate = (const NMITEMACTIVATE*)lParam;
							GetCursorPos(&ptScreen);
							GuiShowListContextMenu(app, activate->iItem, ptScreen);
							return 0;
						}
					}
				}
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}
	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int GuiAppRun(GuiApp* app, int nCmdShow)
{
	WNDCLASSEXW wc;
	INITCOMMONCONTROLSEX icc;
	HWND hwnd;
	HACCEL hAccel;
	ACCEL accels[11];
	MSG msg;

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES | ICC_PROGRESS_CLASS;
	InitCommonControlsEx(&icc);

	if ( GuiRunImmediateLaunch(app) ) {
		CoUninitialize();
		return 0;
	}

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.hInstance = app->instance;
	wc.lpfnWndProc = GuiMainWindowProc;
	wc.lpszClassName = XPKGUI_MAIN_CLASS;
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	RegisterClassExW(&wc);

	hwnd = CreateWindowExW(
		0,
		XPKGUI_MAIN_CLASS,
		XPKGUI_APP_TITLE,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		1200,
		760,
		NULL,
		LoadMenuW(app->instance, MAKEINTRESOURCEW(IDR_MAIN_MENU)),
		app->instance,
		app);
	if ( hwnd == NULL ) {
		CoUninitialize();
		return 1;
	}

	accels[0].fVirt = FCONTROL | FVIRTKEY;
	accels[0].key = 'F';
	accels[0].cmd = ID_VIEW_FOCUS_FILTER;
	accels[1].fVirt = FCONTROL | FVIRTKEY;
	accels[1].key = 'L';
	accels[1].cmd = ID_VIEW_FOCUS_PATH;
	accels[2].fVirt = FALT | FVIRTKEY;
	accels[2].key = VK_UP;
	accels[2].cmd = ID_VIEW_PARENT;
	accels[3].fVirt = FCONTROL | FVIRTKEY;
	accels[3].key = 'A';
	accels[3].cmd = ID_VIEW_SELECT_ALL;
	accels[4].fVirt = FCONTROL | FVIRTKEY;
	accels[4].key = 'C';
	accels[4].cmd = ID_EDIT_COPY_PATHS;
	accels[5].fVirt = FALT | FVIRTKEY;
	accels[5].key = VK_RETURN;
	accels[5].cmd = ID_ACTION_PROPERTIES;
	accels[6].fVirt = FALT | FVIRTKEY;
	accels[6].key = VK_LEFT;
	accels[6].cmd = ID_VIEW_BACK;
	accels[7].fVirt = FALT | FVIRTKEY;
	accels[7].key = VK_RIGHT;
	accels[7].cmd = ID_VIEW_FORWARD;
	accels[8].fVirt = FVIRTKEY;
	accels[8].key = VK_F3;
	accels[8].cmd = ID_ACTION_VIEW;
	accels[9].fVirt = FVIRTKEY;
	accels[9].key = VK_F4;
	accels[9].cmd = ID_ACTION_EDIT;
	accels[10].fVirt = FVIRTKEY;
	accels[10].key = VK_F5;
	accels[10].cmd = ID_VIEW_REFRESH;
	hAccel = CreateAcceleratorTableW(accels, (int)_countof(accels));

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	app->launchErrorShown = FALSE;
	if ( !GuiHandleGuiLaunch(app) ) {
		if ( !app->launchErrorShown ) {
			GuiShowArchiveError(app, L"启动任务");
		}
	}

	while ( GetMessageW(&msg, NULL, 0, 0) > 0 ) {
		BOOL handled;

		handled = FALSE;
		if ( hAccel != NULL && GuiShouldTranslateAccelerator(app, &msg) ) {
			handled = TranslateAcceleratorW(hwnd, hAccel, &msg);
		}
		if ( !handled ) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	if ( hAccel != NULL ) {
		DestroyAcceleratorTable(hAccel);
	}
	CoUninitialize();
	return (int)msg.wParam;
}
