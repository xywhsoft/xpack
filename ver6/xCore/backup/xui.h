


// 公共字体
HFONT xuiBaseFont = NULL;



// 补充定义
#define XUI_DWL_DLGPROC		0
#define XUI_DWL_USERDATA	8



// 公共回调函数
int xuiWindowClassProc(HWND hWin, uint uMsg, WPARAM wParam, LPARAM lParam)
{
	if ( uMsg == WM_CREATE ) {
		HDC hDC = GetDC(hWin);
		SelectObject(hDC, xuiBaseFont);
		ReleaseDC(hWin, hDC);
	}
	ptr pDlgProc = (ptr)GetWindowLongPtr(hWin, XUI_DWL_DLGPROC);
	CallWindowProc(pDlgProc, hWin, uMsg, wParam, lParam);
	return DefWindowProc(hWin, uMsg, wParam, lParam);
}



// 创建窗口
XXAPI HWND xuiCreatWindowW(HWND hParent, int x, int y, int w, int h, wstr sTitle, int iStyle, int iExStyle, ptr pProc)
{
	HWND hWin = CreateWindowExW(iExStyle, L"xuiWindowClassW", sTitle, iStyle, x, y, w, h, hParent, NULL, GetModuleHandle(NULL), NULL);
	if ( hWin ) {
		SetWindowLongPtr(hWin, XUI_DWL_DLGPROC, (LONG_PTR)pProc);
		SendMessage(hWin, WM_INITDIALOG, 0, 0);
	}
	return hWin;
}
XXAPI HWND xuiCreatWindowA(HWND hParent, int x, int y, int w, int h, astr sTitle, int iStyle, int iExStyle, ptr pProc)
{
	HWND hWin = CreateWindowExA(iExStyle, "xuiWindowClassA", sTitle, iStyle, x, y, w, h, hParent, NULL, GetModuleHandle(NULL), NULL);
	if ( hWin ) {
		SetWindowLongPtr(hWin, XUI_DWL_DLGPROC, (LONG_PTR)pProc);
		SendMessage(hWin, WM_INITDIALOG, 0, 0);
	}
	return hWin;
}



// 创建控件
XXAPI HWND xuiCraetControlW(HWND hParent, int x, int y, int w, int h, wstr sClassName, wstr sCaption, int id, int iStyle, int iExStyle)
{
	HWND hCon = CreateWindowExW(iExStyle, sClassName, sCaption, iStyle, x, y, w, h, hParent, NULL, GetModuleHandle(NULL), NULL);
	SendMessage(hCon, WM_SETFONT, (LONG_PTR)xuiBaseFont, FALSE);
	if ( id ) {
		SetWindowLong(hCon, GWL_ID, id);
	}
	return hCon;
}
XXAPI HWND xuiCraetControlA(HWND hParent, int x, int y, int w, int h, astr sClassName, astr sCaption, int id, int iStyle, int iExStyle)
{
	HWND hCon = CreateWindowExA(iExStyle, sClassName, sCaption, iStyle, x, y, w, h, hParent, NULL, GetModuleHandle(NULL), NULL);
	SendMessage(hCon, WM_SETFONT, (LONG_PTR)xuiBaseFont, FALSE);
	if ( id ) {
		SetWindowLong(hCon, GWL_ID, id);
	}
	return hCon;
}



// 创建标签
XXAPI HWND xuiCreatLabelW(HWND hParent, int x, int y, int w, int h, wstr sCaption, int id)
{
	return xuiCraetControlW(hParent, x, y, w, h, L"Static", sCaption, id, 0x50000000, 0);
}
XXAPI HWND xuiCreatLabelA(HWND hParent, int x, int y, int w, int h, astr sCaption, int id)
{
	return xuiCraetControlA(hParent, x, y, w, h, "Static", sCaption, id, 0x50000000, 0);
}



// 创建按钮
XXAPI HWND xuiCreatButtonW(HWND hParent, int x, int y, int w, int h, wstr sCaption, int id)
{
	return xuiCraetControlW(hParent, x, y, w, h, L"Button", sCaption, id, 0x50018000, 0);
}
XXAPI HWND xuiCreatButtonA(HWND hParent, int x, int y, int w, int h, astr sCaption, int id)
{
	return xuiCraetControlA(hParent, x, y, w, h, "Button", sCaption, id, 0x50018000, 0);
}



// 创建输入框
XXAPI HWND xuiCreatTextBoxW(HWND hParent, int x, int y, int w, int h, wstr sText, int id)
{
	return xuiCraetControlW(hParent, x, y, w, h, L"Edit", sText, id, 0x50810000, 0);
}
XXAPI HWND xuiCreatTextBoxA(HWND hParent, int x, int y, int w, int h, astr sText, int id)
{
	return xuiCraetControlA(hParent, x, y, w, h, "Edit", sText, id, 0x50810000, 0);
}



// 创建框架
XXAPI HWND xuiCreatFrameW(HWND hParent, int x, int y, int w, int h, wstr sCaption, int id)
{
	return xuiCraetControlW(hParent, x, y, w, h, L"Static", sCaption, id, 0x50008007, 0);
}
XXAPI HWND xuiCreatFrameA(HWND hParent, int x, int y, int w, int h, astr sCaption, int id)
{
	return xuiCraetControlA(hParent, x, y, w, h, "Static", sCaption, id, 0x50008007, 0);
}



// 创建复选框
XXAPI HWND xuiCreatCheckBoxW(HWND hParent, int x, int y, int w, int h, wstr sCaption, int id)
{
	return xuiCraetControlW(hParent, x, y, w, h, L"Button", sCaption, id, 0x50018003, 0);
}
XXAPI HWND xuiCreatCheckBoxA(HWND hParent, int x, int y, int w, int h, astr sCaption, int id)
{
	return xuiCraetControlA(hParent, x, y, w, h, "Button", sCaption, id, 0x50018003, 0);
}



// 创建单选框
XXAPI HWND xuiCreatRadioBoxW(HWND hParent, int x, int y, int w, int h, wstr sCaption, int id)
{
	return xuiCraetControlW(hParent, x, y, w, h, L"Button", sCaption, id, 0x50018009, 0);
}
XXAPI HWND xuiCreatRadioBoxA(HWND hParent, int x, int y, int w, int h, astr sCaption, int id)
{
	return xuiCraetControlA(hParent, x, y, w, h, "Button", sCaption, id, 0x50018009, 0);
}



// 创建列表框
XXAPI HWND xuiCreatListBoxW(HWND hParent, int x, int y, int w, int h, int id)
{
	return xuiCraetControlW(hParent, x, y, w, h, L"ListBox", NULL, id, 0x50B10141, 0);
}
XXAPI HWND xuiCreatListBoxA(HWND hParent, int x, int y, int w, int h, int id)
{
	return xuiCraetControlA(hParent, x, y, w, h, "ListBox", NULL, id, 0x50B10141, 0);
}



// 创建组合框
XXAPI HWND xuiCreatComboBoxW(HWND hParent, int x, int y, int w, int h, int id)
{
	return xuiCraetControlW(hParent, x, y, w, h, L"ComboBox", NULL, id, 0x50310043, 0);
}
XXAPI HWND xuiCreatComboBoxA(HWND hParent, int x, int y, int w, int h, int id)
{
	return xuiCraetControlA(hParent, x, y, w, h, "ComboBox", NULL, id, 0x50310043, 0);
}



// 创建滚动条
XXAPI HWND xuiCreatScrollBarW(HWND hParent, int x, int y, int w, int h, int bHori, int id)
{
	return xuiCraetControlW(hParent, x, y, w, h, L"ScrollBar", NULL, id, bHori ? 0x50000000 : 0x50000001, 0);
}
XXAPI HWND xuiCreatScrollBarA(HWND hParent, int x, int y, int w, int h, int bHori, int id)
{
	return xuiCraetControlA(hParent, x, y, w, h, "ScrollBar", NULL, id, bHori ? 0x50000000 : 0x50000001, 0);
}



// 创建图片框
XXAPI HWND xuiCreatImageBoxW(HWND hParent, int x, int y, int w, int h, int id)
{
	return xuiCraetControlW(hParent, x, y, w, h, L"Static", NULL, id, 0x5000020E, 0);
}
XXAPI HWND xuiCreatImageBoxA(HWND hParent, int x, int y, int w, int h, int id)
{
	return xuiCraetControlA(hParent, x, y, w, h, "Static", NULL, id, 0x5000020E, 0);
}



// 修改默认字体
XXAPI void xuiSetFont(HFONT hFont)
{
	if ( xuiBaseFont ) {
		DeleteObject(xuiBaseFont);
	}
	xuiBaseFont = hFont;
}



// 初始化 xui 库（iWindowExtData 从 16 往后排，前16位xui占用了）
XXAPI int xuiInit(int iWindowExtData, int iClassExtData)
{
	// 创建默认字体
	xuiSetFont(CreateFontW(12, 6, 0, 0, 12, 0, 0, 0, GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"宋体"));
	// 注册窗口类（UNICODE）
	WNDCLASSEXW wc;
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.cbWndExtra = 16 + iWindowExtData;
	wc.cbClsExtra = iClassExtData;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (ptr)xuiWindowClassProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_BTNSHADOW;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"xuiWindowClassW";
	wc.hIconSm = wc.hIcon;
	int bRet = RegisterClassExW(&wc);
	// 注册窗口类（ANSI）
	WNDCLASSEXA wca;
	wca.cbSize = sizeof(WNDCLASSEXA);
	wca.cbWndExtra = 16 + iWindowExtData;
	wca.cbClsExtra = iClassExtData;
	wca.style = CS_HREDRAW | CS_VREDRAW;
	wca.lpfnWndProc = (ptr)xuiWindowClassProc;
	wca.hInstance = GetModuleHandle(NULL);
	wca.hIcon = LoadIcon(0, IDI_APPLICATION);
	wca.hCursor = LoadCursor(0, IDC_ARROW);
	wca.hbrBackground = (HBRUSH)COLOR_BTNSHADOW;
	wca.lpszMenuName = NULL;
	wca.lpszClassName = "xuiWindowClassA";
	wca.hIconSm = wca.hIcon;
	return ( bRet && RegisterClassExA(&wca) );
}



// 卸载 xui 库
XXAPI void xuiUnit()
{
	xuiSetFont(NULL);
	UnregisterClassW(L"xuiWindowClass", GetModuleHandle(NULL));
}



// 开始消息循环
XXAPI void xuiStart()
{
	MSG uMsg;
	while ( GetMessage(&uMsg, 0, 0, 0) ) {
		TranslateMessage (&uMsg);
		DispatchMessage (&uMsg);
	}
}



// 停止消息循环
XXAPI void xuiStop()
{
	PostQuitMessage(0);
}


