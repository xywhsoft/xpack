#include "app.h"

#include <shellapi.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
	GuiApp app;
	WCHAR** argv;
	int argc;
	int iRet;

	(void)hPrevInstance;
	(void)lpCmdLine;

	ZeroMemory(&app, sizeof(app));
	app.instance = hInstance;
	GuiAppInitDefaults(&app);
	GuiLoadRecentArchives(&app);
	GuiLoadColumnWidths(&app);
	GuiLoadSortSettings(&app);

	argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if ( argv == NULL ) {
		MessageBoxW(NULL, L"无法解析命令行。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
		return 1;
	}

	iRet = GuiParseCommandLineArgs(&app, argc, argv);
	LocalFree(argv);
	if ( iRet != 0 ) {
		GuiAppCleanup(&app);
		return iRet;
	}

	iRet = GuiAppRun(&app, nCmdShow);
	GuiAppCleanup(&app);
	return iRet;
}
