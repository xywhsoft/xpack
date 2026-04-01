#ifndef XPKGUI_APP_H
#define XPKGUI_APP_H

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <commctrl.h>
#include <stdint.h>

#include "../../../xpack.h"

#ifndef _countof
#define _countof(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#define XPKGUI_APP_TITLE L"xpkgui"
#define XPKGUI_MAIN_CLASS L"xPack.Gui.MainWindow"
#define XPKGUI_MAX_INPUTS 64
#define XPKGUI_ITEM_TEXT 512
#define XPKGUI_MAX_RECENT_ARCHIVES 10
#define XPKGUI_MAX_NAV_HISTORY 64
#define XPKGUI_MAX_TEMP_PATH 1024

typedef enum GuiLaunchCommand {
	GUI_LAUNCH_NONE = 0,
	GUI_LAUNCH_OPEN,
	GUI_LAUNCH_SHELL_ADD,
	GUI_LAUNCH_SHELL_ADD_AUTO,
	GUI_LAUNCH_SHELL_EXTRACT,
	GUI_LAUNCH_SHELL_EXTRACT_AUTO,
	GUI_LAUNCH_SHELL_EXTRACT_HERE,
	GUI_LAUNCH_SHELL_VERIFY,
	GUI_LAUNCH_SHELL_PROPERTIES
} GuiLaunchCommand;

typedef struct GuiLaunchState {
	GuiLaunchCommand command;
	int inputCount;
	WCHAR** inputs;
	WCHAR archivePath[MAX_PATH];
	WCHAR destPath[MAX_PATH];
} GuiLaunchState;

typedef struct GuiArchiveItem {
	uint32_t pos;
	int64_t fileIndex;
	uint64_t fileSize;
	uint64_t packedSize;
	uint32_t flag;
	uint32_t fileHash;
	uint32_t attr;
	xtime createTime;
	xtime modifyTime;
	xtime accessTime;
	char packagePath[XPK_PATH_BYTES];
	WCHAR name[XPKGUI_ITEM_TEXT];
	WCHAR sizeText[64];
	WCHAR packedText[64];
	WCHAR methodText[48];
	WCHAR modifiedText[64];
	WCHAR idText[64];
	WCHAR hashText[32];
	WCHAR attrText[32];
} GuiArchiveItem;

typedef enum GuiViewItemKind {
	GUI_VIEW_ITEM_FILE = 1,
	GUI_VIEW_ITEM_DIR,
	GUI_VIEW_ITEM_PARENT
} GuiViewItemKind;

typedef struct GuiViewItem {
	GuiViewItemKind kind;
	size_t sourceIndex;
	uint32_t pos;
	int64_t fileIndex;
	uint64_t fileSize;
	uint64_t packedSize;
	uint32_t fileHash;
	uint32_t attr;
	xtime modifyTime;
	WCHAR name[XPKGUI_ITEM_TEXT];
	WCHAR fullPath[XPKGUI_ITEM_TEXT];
	WCHAR sizeText[64];
	WCHAR packedText[64];
	WCHAR methodText[48];
	WCHAR modifiedText[64];
	WCHAR idText[64];
	WCHAR hashText[32];
	WCHAR attrText[32];
} GuiViewItem;

typedef struct GuiArchiveOptions {
	WCHAR archivePath[MAX_PATH];
	xpkPackType packType;
	uint8_t defaultComp;
	uint8_t metaComp;
	uint8_t infoComp;
	uint32_t infoExtSize;
	uint32_t volumeSize;
	uint8_t writePolicy;
	BOOL solidMode;
} GuiArchiveOptions;

typedef struct GuiInputDialogState {
	const WCHAR* title;
	const WCHAR* prompt;
	WCHAR value[260];
} GuiInputDialogState;

typedef struct GuiArchiveConfigDialogState {
	BOOL createMode;
	BOOL allowPackTypeEdit;
	GuiArchiveOptions options;
} GuiArchiveConfigDialogState;

typedef struct GuiTextEditorDialogState {
	const WCHAR* title;
	const WCHAR* prompt;
	WCHAR* text;
	size_t cchText;
	BOOL readOnly;
} GuiTextEditorDialogState;

typedef struct GuiBrowseHistoryEntry {
	WCHAR folder[XPKGUI_ITEM_TEXT];
	BOOL flatView;
} GuiBrowseHistoryEntry;

typedef struct GuiApp {
	HINSTANCE instance;
	HWND window;
	HWND actNew;
	HWND actOpen;
	HWND actAdd;
	HWND actExtract;
	HWND actVerify;
	HWND actInfo;
	HWND navBack;
	HWND navForward;
	HWND navRoot;
	HWND navUp;
	HWND navPath;
	HWND navGo;
	HWND filterEdit;
	HWND filterClear;
	HWND list;
	HWND status;

	xpkObject archive;
	xpkPackType packType;
	uint8_t defaultComp;
	uint8_t metaComp;
	uint8_t infoComp;
	uint32_t infoExtSize;
	uint32_t volumeSize;
	uint8_t writePolicy;
	BOOL solidMode;

	WCHAR archivePath[MAX_PATH];
	GuiArchiveItem* items;
	size_t itemCount;
	size_t itemCapacity;
	GuiViewItem* viewItems;
	size_t viewCount;
	size_t viewCapacity;
	WCHAR currentFolder[XPKGUI_ITEM_TEXT];
	WCHAR filterText[XPKGUI_ITEM_TEXT];
	BOOL flatView;
	int sortColumn;
	BOOL sortAscending;
	GuiBrowseHistoryEntry navHistory[XPKGUI_MAX_NAV_HISTORY];
	UINT navHistoryCount;
	UINT navHistoryIndex;
	BOOL navHistoryLocked;
	WCHAR recentArchives[XPKGUI_MAX_RECENT_ARCHIVES][MAX_PATH];
	UINT recentArchiveCount;
	BOOL launchErrorShown;
	BOOL archiveChangePromptActive;
	FILETIME archiveWriteTime;
	uint64_t archiveFileSize;

	GuiLaunchState launch;
} GuiApp;

void GuiAppInitDefaults(GuiApp* app);
void GuiAppCleanup(GuiApp* app);
void GuiAppCloseArchive(GuiApp* app);

int GuiParseCommandLineArgs(GuiApp* app, int argc, WCHAR** argv);
int GuiAppRun(GuiApp* app, int nCmdShow);

BOOL GuiUtf8FromWide(const WCHAR* src, char* dst, size_t dstCount);
BOOL GuiWideFromUtf8(const char* src, WCHAR* dst, size_t dstCount);
void GuiFormatUInt64(uint64_t value, WCHAR* buf, size_t cchBuf);
void GuiFormatTime(xtime value, WCHAR* buf, size_t cchBuf);
void GuiUpdateTitle(GuiApp* app);
void GuiUpdateStatus(GuiApp* app);
BOOL GuiSetClipboardText(HWND owner, const WCHAR* text);
void GuiShowArchiveError(GuiApp* app, const WCHAR* actionText);
void GuiShowArchiveErrorPath(GuiApp* app, const WCHAR* actionText, const WCHAR* archivePath);
void GuiShowSystemError(HWND hwnd, const WCHAR* title, DWORD err);
BOOL GuiLoadRecentArchives(GuiApp* app);
BOOL GuiRememberRecentArchive(GuiApp* app, const WCHAR* archivePath);
BOOL GuiClearRecentArchives(GuiApp* app);
const WCHAR* GuiGetRecentArchivePath(const GuiApp* app, UINT commandId);

BOOL GuiOpenArchiveDialog(HWND hwnd, WCHAR* pathBuf, DWORD cchBuf);
BOOL GuiSaveArchiveDialog(HWND hwnd, WCHAR* pathBuf, DWORD cchBuf);
BOOL GuiPickFilesDialog(HWND hwnd, WCHAR* buffer, DWORD cchBuffer);
BOOL GuiPickSingleFileDialog(HWND hwnd, WCHAR* pathBuf, DWORD cchBuf, const WCHAR* title);
BOOL GuiPickFolderDialog(HWND hwnd, WCHAR* pathBuf, DWORD cchBuf, const WCHAR* title);
BOOL GuiReadWholeFile(const WCHAR* path, void** dataOut, DWORD* sizeOut);
BOOL GuiWriteWholeFile(const WCHAR* path, const void* data, DWORD size);
BOOL GuiEnsureDirectoryRecursive(const WCHAR* dirPath);
BOOL GuiEnsureParentDirectory(const WCHAR* filePath);
BOOL GuiSuggestArchivePath(int count, WCHAR** items, WCHAR* pathBuf, DWORD cchBuf, BOOL makeUnique);
BOOL GuiSuggestExtractFolderPath(const WCHAR* archivePath, WCHAR* pathBuf, DWORD cchBuf, BOOL makeUnique);

BOOL GuiRunInputDialog(HWND hwnd, GuiInputDialogState* state);
BOOL GuiRunArchiveConfigDialog(HWND hwnd, GuiArchiveConfigDialogState* state);
BOOL GuiRunTextEditorDialog(HWND hwnd, GuiTextEditorDialogState* state);

BOOL GuiArchiveOpenPath(GuiApp* app, const WCHAR* archivePath, BOOL readonly);
BOOL GuiArchiveCreateWithOptions(GuiApp* app, const GuiArchiveOptions* options);
BOOL GuiArchiveRefreshView(GuiApp* app);
BOOL GuiArchiveReloadFromDisk(GuiApp* app);
BOOL GuiArchiveCheckExternalChanges(GuiApp* app);
void GuiArchiveToggleSort(GuiApp* app, int column);
BOOL GuiArchiveCanExtractSelection(GuiApp* app);
BOOL GuiArchiveCanDeleteSelection(GuiApp* app);
BOOL GuiArchiveCanRenameSelection(GuiApp* app);
BOOL GuiArchiveCanSetSelectionFileType(GuiApp* app);
BOOL GuiArchiveCanSetSelectionPathAttr(GuiApp* app);
BOOL GuiArchiveCanVerifySelection(GuiApp* app);
BOOL GuiArchiveCanOpenSelection(GuiApp* app);
BOOL GuiArchiveCanViewSelection(GuiApp* app);
BOOL GuiArchiveCanEditTextSelection(GuiApp* app);
BOOL GuiArchiveCanCopySelectionHash(GuiApp* app);
BOOL GuiArchiveCanShowSelectionInExplorer(GuiApp* app);
BOOL GuiArchiveCanEditSelection(GuiApp* app);
BOOL GuiArchiveCanReplaceSelection(GuiApp* app);
BOOL GuiArchiveCanEditSelectionInfoExt(GuiApp* app);
BOOL GuiArchiveCanCopySelection(GuiApp* app);
BOOL GuiArchiveCanSelectByPattern(GuiApp* app);
BOOL GuiArchiveCanSelectSameExtension(GuiApp* app);
BOOL GuiArchiveCanSelectSameHash(GuiApp* app);
BOOL GuiArchiveCanSelectDuplicateFiles(GuiApp* app);
BOOL GuiArchiveCanSelectDuplicateCopies(GuiApp* app);
BOOL GuiArchiveCanDeleteDuplicateCopies(GuiApp* app);
BOOL GuiArchiveCanInvertSelection(GuiApp* app);
BOOL GuiArchiveCanLocateInTree(GuiApp* app);
BOOL GuiArchiveCanBrowseBack(GuiApp* app);
BOOL GuiArchiveCanBrowseForward(GuiApp* app);
BOOL GuiArchiveBrowseBack(GuiApp* app);
BOOL GuiArchiveBrowseForward(GuiApp* app);
BOOL GuiArchiveBrowseParent(GuiApp* app);
BOOL GuiArchiveBrowseRoot(GuiApp* app);
BOOL GuiArchiveActivateSelection(GuiApp* app);
BOOL GuiArchiveLocateInTree(GuiApp* app);
BOOL GuiArchiveToggleFlatView(GuiApp* app);
BOOL GuiArchiveNavigateToFolder(GuiApp* app, const WCHAR* folder);
BOOL GuiArchiveSetFilter(GuiApp* app, const WCHAR* filterText);
BOOL GuiArchiveGetSelectionSummary(GuiApp* app, uint32_t* outFileCount, uint64_t* outSize, uint64_t* outPackedSize);
BOOL GuiArchiveOpenSelection(GuiApp* app);
BOOL GuiArchiveViewSelection(GuiApp* app);
BOOL GuiArchiveEditSelectionText(GuiApp* app);
BOOL GuiArchiveEditSelection(GuiApp* app);
BOOL GuiArchiveReplaceSelection(GuiApp* app);
BOOL GuiArchiveOpenSelectionWith(GuiApp* app);
BOOL GuiArchiveShowSelectionInExplorer(GuiApp* app);
BOOL GuiArchiveEditSelectionInfoExt(GuiApp* app);
BOOL GuiArchiveImportSelectionInfoExt(GuiApp* app);
BOOL GuiArchiveExportSelectionInfoExt(GuiApp* app);
BOOL GuiArchiveClearSelectionInfoExt(GuiApp* app);
BOOL GuiArchiveCopySelectionPaths(GuiApp* app);
BOOL GuiArchiveCopySelectionHash(GuiApp* app);
BOOL GuiArchiveSelectByPattern(GuiApp* app);
BOOL GuiArchiveSelectSameExtension(GuiApp* app);
BOOL GuiArchiveSelectSameHash(GuiApp* app);
BOOL GuiArchiveSelectDuplicateFiles(GuiApp* app);
BOOL GuiArchiveSelectDuplicateCopies(GuiApp* app);
BOOL GuiArchiveDeleteDuplicateCopies(GuiApp* app);
BOOL GuiArchiveInvertSelection(GuiApp* app);
BOOL GuiArchiveShowSelectionProperties(GuiApp* app);
BOOL GuiArchiveReadOptions(GuiApp* app, GuiArchiveOptions* options);
BOOL GuiArchiveApplyOptions(GuiApp* app, const GuiArchiveOptions* options, BOOL saveNow);
BOOL GuiArchiveSave(GuiApp* app);
BOOL GuiArchiveBuild(GuiApp* app);
BOOL GuiArchivePromptNew(GuiApp* app, const WCHAR* suggestedPath, GuiArchiveOptions* optionsOut);
BOOL GuiArchivePromptSettings(GuiApp* app, GuiArchiveOptions* optionsOut);
BOOL GuiArchiveCreateFromSelection(GuiApp* app, int count, WCHAR** items, BOOL autoPath, BOOL exitAfterCreate);
BOOL GuiArchiveAddSelection(GuiApp* app, int count, WCHAR** items);
BOOL GuiArchiveAddEmptyEntry(GuiApp* app);
BOOL GuiArchiveExtractSelection(GuiApp* app, BOOL allItems, const WCHAR* fixedDest);
BOOL GuiArchiveExtractPath(const WCHAR* archivePath, const WCHAR* destPath);
BOOL GuiArchiveExtractPathTask(const WCHAR* archivePath, const WCHAR* destPath, BOOL* cancelledOut);
BOOL GuiArchiveRemoveSelection(GuiApp* app);
BOOL GuiArchiveRenameSelection(GuiApp* app);
BOOL GuiArchiveSetSelectionFileType(GuiApp* app);
BOOL GuiArchiveSetSelectionPathAttr(GuiApp* app);
BOOL GuiArchiveVerifySelection(GuiApp* app);
BOOL GuiArchiveVerify(GuiApp* app);
BOOL GuiArchiveVerifyPath(const WCHAR* archivePath);
BOOL GuiArchiveVerifyPathTask(const WCHAR* archivePath, BOOL* cancelledOut);
BOOL GuiArchiveShowProperties(GuiApp* app);
BOOL GuiArchiveShowPropertiesPath(const WCHAR* archivePath);
BOOL GuiArchiveMetaEdit(GuiApp* app);
BOOL GuiArchiveMetaImport(GuiApp* app);
BOOL GuiArchiveMetaExport(GuiApp* app);
BOOL GuiArchiveMetaClear(GuiApp* app);
uint32_t GuiArchiveSelectedCount(GuiApp* app);
GuiArchiveItem* GuiArchiveGetSingleSelectedItem(GuiApp* app);

BOOL GuiRunImmediateLaunch(GuiApp* app);
BOOL GuiHandleGuiLaunch(GuiApp* app);
void GuiSetMenuState(GuiApp* app);
void GuiUpdateNavigationBar(GuiApp* app);

#endif
