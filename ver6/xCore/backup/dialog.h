


// 单个文件路径最大长度
#define DLG_BUFFSIZE_ONE 32768

// 多选文件路径最大长度
#define DLG_BUFFSIZE_MUL 1048576



// 内部函数 : InputBox窗口
int bInputBox_RetVal;
ptr sInputBox_RetVal = NULL;
HWND hInputBox_Edit = NULL;
int InputBox_Proc(HWND hWin, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if ( uMsg == WM_COMMAND ) {
		int id = LOWORD(wParam);
		int eve = HIWORD(wParam);
		if ( eve == BN_CLICKED ) {
			if ( id == 10 ) {
				// 确定按钮
				bInputBox_RetVal = -1;
				int iSize = SendMessage(hInputBox_Edit, WM_GETTEXTLENGTH, 0, 0);
				sInputBox_RetVal = malloc(iSize + 1);
				SendMessage(hInputBox_Edit, WM_GETTEXT, iSize + 1, (LONG_PTR)sInputBox_RetVal);
				DestroyWindow(hWin);
				xuiStop();
			} else if ( id = 11 ) {
				// 取消按钮
				DestroyWindow(hWin);
				xuiStop();
			}
		}
	} else if ( uMsg == WM_CLOSE ) {
		DestroyWindow(hWin);
		xuiStop();
	} else {
		return FALSE;
	}
	return TRUE;
}



// 内部函数 : 处理过滤规则（需使用 xCore.free 释放）
wstr priConvFilterW(wstr sFilter)
{
	wstr sRet;
	if ( sFilter == NULL ) {
		sRet = xCore_CopyStringW(L"All Files(*)\0*\0\0", 0);
	} else {
		int iSize = wcslen(sFilter);
		sRet = malloc((iSize + 2) * sizeof(wchar_t));
		memcpy(sRet, sFilter, iSize * sizeof(wchar_t));
		sRet[iSize] = 0;
		sRet[iSize + 1] = 0;
		for ( int i = 0; i < iSize; i++ ) {
			if ( sRet[i] == '|' ) { sRet[i] = 0; }
		}
	}
	return sRet;
}
astr priConvFilterA(astr sFilter)
{
	astr sRet;
	if ( sFilter == NULL ) {
		sRet = xCore_CopyStringA("All Files(*)\0*\0\0", 0);
	} else {
		int iSize = strlen(sFilter);
		sRet = malloc(iSize + 2);
		memcpy(sRet, sFilter, iSize);
		sRet[iSize] = 0;
		sRet[iSize + 1] = 0;
		for ( int i = 0; i < iSize; i++ ) {
			if ( sRet[i] == '|' ) { sRet[i] = 0; }
		}
	}
	return sRet;
}



// 内部函数 : 选择文件夹对话框 HOOK
WNDPROC priOldWndProc = NULL;
#define  ID_COMBO_ADDR 0x47C
#define  ID_LEFT_TOOBAR 0x4A0
LRESULT static __stdcall priSelectFolderProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam  )
{
    if ( uMsg == WM_COMMAND ) {
		if( wParam == IDOK ) {
			wchar_t wcDirPath[MAX_PATH] = {0};
			HWND hComboAddr = GetDlgItem(hWnd, ID_COMBO_ADDR);
			if ( hComboAddr != NULL ) {
				GetWindowTextW(hComboAddr, wcDirPath, MAX_PATH);
			}
			if ( wcslen(wcDirPath) ) {
				DWORD dwAttr = GetFileAttributesW(wcDirPath);
				if( dwAttr != -1 && (FILE_ATTRIBUTE_DIRECTORY & dwAttr) ) {
					LPOPENFILENAMEW oFn = (LPOPENFILENAMEW)GetPropW(hWnd, L"OPENFILENAME");
					if ( oFn ) {
						int size = oFn->nMaxFile > MAX_PATH?MAX_PATH: oFn->nMaxFile;
						memcpy(oFn->lpstrFile, wcDirPath, size * sizeof(wchar_t));
						RemovePropW(hWnd, L"OPENFILENAME");
						EndDialog(hWnd, 1);
					} else {
						EndDialog(hWnd, 0);
					}
				}
			}
		} else {
			//如果是左边toolbar发出的WM_COMMOND消息（即点击左边的toolbar）, 则清空OK按钮旁的组合框。
			HWND hCtrl = (HWND)lParam;
			if ( hCtrl != NULL ) {
				int ctrlId = GetDlgCtrlID(hCtrl);
				if ( ctrlId == ID_LEFT_TOOBAR ) {
					HWND hComboAddr = GetDlgItem(hWnd, ID_COMBO_ADDR);
					if ( hComboAddr != NULL ) {
						SetWindowTextW(hComboAddr, L"");
					}
				}
			}
		}
    }
    return CallWindowProc(priOldWndProc, hWnd, uMsg, wParam, lParam);
}
static __stdcall UINT_PTR priSelectFolderHookProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // hWnd 是一个隐藏的对话框，其父窗口为打开文件对话框，按钮等控件的消息在父窗口处理。
    if ( uMsg == WM_NOTIFY ) {
        LPOFNOTIFY lpOfNotify = (LPOFNOTIFY)lParam;
        if ( lpOfNotify->hdr.code == CDN_INITDONE ) {
            SetPropW(GetParent(hWnd), L"OPENFILENAME", (HANDLE)(lpOfNotify->lpOFN));
            priOldWndProc = (WNDPROC)SetWindowLongPtrW(GetParent(hWnd), GWLP_WNDPROC, (LONG_PTR)priSelectFolderProc);
        }
        if ( lpOfNotify->hdr.code == CDN_SELCHANGE ) {
            wchar_t wcDirPath[MAX_PATH] = {0};
            CommDlg_OpenSave_GetFilePathW(GetParent(hWnd), wcDirPath, sizeof(wcDirPath));
            HWND hComboAddr = GetDlgItem(GetParent(hWnd), ID_COMBO_ADDR);
            if ( hComboAddr != NULL ) {
                if ( wcslen(wcDirPath) ) {
                    //去掉文件夹快捷方式的后缀名。
                    int pathSize = wcslen(wcDirPath);
                    if ( pathSize >= 4 ) {
                        wchar_t* wcExtension = Path_FileExtW(wcDirPath);
                        if ( wcslen(wcExtension) ) {
                            wcExtension = CharLowerW(wcExtension);
                            if( !wcscmp(wcExtension, L"lnk") ) {
                                wcDirPath[pathSize - 4] = L'\0';
                            }
                        }
                    }
                    SetWindowTextW(hComboAddr, wcDirPath);
                } else {
                    SetWindowTextW(hComboAddr, L"");
                }
            }
        }
    }
    return 1;
}







// 输入对话框（需使用 xCore.free 释放）
XXAPI wstr xInputBoxW(HWND hParent, wstr sLabel, wstr sTitle, wstr sText, int x, int y, int iExitStyle)
{
	bInputBox_RetVal = 0;
	if ( x < 0 ) { x = (GetSystemMetrics(SM_CXSCREEN) - 440) / 2; }
	if ( y < 0 ) { y = (GetSystemMetrics(SM_CYSCREEN) - 130) / 2; }
	HWND hWin = xuiCreatWindowW(hParent, x, y, 440, 130, sTitle, 0x90C80800, 0x0, InputBox_Proc);
	xuiCreatLabelW(hWin, 10, 10, 330, 60, sLabel, 0);
	xuiCreatButtonA(hWin, 355, 10, 70, 22, "确 定(&K)", 10);
	xuiCreatButtonA(hWin, 355, 42, 70, 22, "取 消(&C)", 11);
	hInputBox_Edit = xuiCreatTextBoxW(hWin, 10, 75, 415, 16, sText, 0);
	if ( iExitStyle ) { SetWindowLong(hInputBox_Edit, GWL_STYLE, iExitStyle); }
	xuiStart();
	xCore.iRet = bInputBox_RetVal;
	if ( bInputBox_RetVal ) {
		return sInputBox_RetVal;
	} else {
		return (wstr)xCore.nullstring;
	}
}
XXAPI astr xInputBoxA(HWND hParent, astr sLabel, astr sTitle, astr sText, int x, int y, int iExitStyle)
{
	bInputBox_RetVal = 0;
	if ( x < 0 ) { x = (GetSystemMetrics(SM_CXSCREEN) - 440) / 2; }
	if ( y < 0 ) { y = (GetSystemMetrics(SM_CYSCREEN) - 130) / 2; }
	HWND hWin = xuiCreatWindowA(hParent, x, y, 440, 130, sTitle, 0x90C80800, 0x0, InputBox_Proc);
	xuiCreatLabelA(hWin, 10, 10, 330, 60, sLabel, 0);
	xuiCreatButtonA(hWin, 355, 10, 70, 22, "确 定(&K)", 10);
	xuiCreatButtonA(hWin, 355, 42, 70, 22, "取 消(&C)", 11);
	hInputBox_Edit = xuiCreatTextBoxA(hWin, 10, 75, 415, 16, sText, 0);
	if ( iExitStyle ) { SetWindowLong(hInputBox_Edit, GWL_STYLE, iExitStyle); }
	xuiStart();
	xCore.iRet = bInputBox_RetVal;
	if ( bInputBox_RetVal ) {
		return sInputBox_RetVal;
	} else {
		return (astr)xCore.nullstring;
	}
}



// 选择文件（需使用 xCore.free 释放）
XXAPI wstr xOpenFileDialogW(HWND hParent, wstr sDefPath, wstr sFilter, wstr sTitle, int iFlag)
{
	// 安全检查
	sFilter = priConvFilterW(sFilter);
	if ( sTitle == NULL ) { sTitle = L"Open file :"; }
	int iBufSize = (iFlag & OFN_ALLOWMULTISELECT) ? DLG_BUFFSIZE_MUL : DLG_BUFFSIZE_ONE;
	wstr sOutPath = malloc(iBufSize * sizeof(wchar_t));
	if ( sDefPath == NULL ) { sOutPath[0] = 0; } else { wcscpy(sOutPath, sDefPath); }
	// 弹出对话框
	OPENFILENAMEW ofn;
	memset(&ofn, 0, sizeof(OPENFILENAMEW));
	ofn.lStructSize = sizeof(OPENFILENAMEW);
	ofn.hwndOwner = hParent;
	ofn.hInstance = GetModuleHandle(NULL);
	ofn.lpstrFilter = sFilter;
	ofn.lpstrFile = sOutPath;
	ofn.lpstrInitialDir = sOutPath;
	ofn.nMaxFile = iBufSize;
	ofn.lpstrTitle = sTitle;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER | iFlag;
	xCore.iRet = GetOpenFileNameW(&ofn);
	// 释放内存，返回结果
	xCore.free(sFilter);
	wstr sRet;
	if ( xCore.iRet ) {
		if ( iFlag & OFN_ALLOWMULTISELECT ) {
			sRet = (wstr)xCore_SplitCharW(sOutPath, 0, FALSE);
		} else {
			sRet = xCore_CopyStringW(sOutPath, 0);
		}
	} else {
		sRet = (wstr)xCore.nullstring;
	}
	free(sOutPath);
	return sRet;
}
XXAPI astr xOpenFileDialogA(HWND hParent, astr sDefPath, astr sFilter, astr sTitle, int iFlag)
{
	// 安全检查
	sFilter = priConvFilterA(sFilter);
	if ( sTitle == NULL ) { sTitle = "Open file :"; }
	int iBufSize = (iFlag & OFN_ALLOWMULTISELECT) ? DLG_BUFFSIZE_MUL : DLG_BUFFSIZE_ONE;
	astr sOutPath = malloc(iBufSize);
	if ( sDefPath == NULL ) { sOutPath[0] = 0; } else { strcpy(sOutPath, sDefPath); }
	// 弹出对话框
	OPENFILENAMEA ofn;
	memset(&ofn, 0, sizeof(OPENFILENAMEA));
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = hParent;
	ofn.hInstance = GetModuleHandle(NULL);
	ofn.lpstrFilter = sFilter;
	ofn.lpstrFile = sOutPath;
	ofn.lpstrInitialDir = sOutPath;
	ofn.nMaxFile = iBufSize;
	ofn.lpstrTitle = sTitle;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER | iFlag;
	xCore.iRet = GetOpenFileNameA(&ofn);
	// 释放内存，返回结果
	xCore.free(sFilter);
	astr sRet;
	if ( xCore.iRet ) {
		if ( iFlag & OFN_ALLOWMULTISELECT ) {
			sRet = (astr)xCore_SplitCharA(sOutPath, 0, FALSE);
		} else {
			sRet = xCore_CopyStringA(sOutPath, 0);
		}
	} else {
		sRet = (astr)xCore.nullstring;
	}
	free(sOutPath);
	return sRet;
}



// 多选文件（需使用 xCore.free 释放）
XXAPI wstr* xOpenFilesDialogW(HWND hParent, wstr sDefPath, wstr sFilter, wstr sTitle, int iFlag)
{
	return (wstr*)xOpenFileDialogW(hParent, sDefPath, sFilter, sTitle, iFlag | OFN_ALLOWMULTISELECT);
}
XXAPI astr* xOpenFilesDialogA(HWND hParent, astr sDefPath, astr sFilter, astr sTitle, int iFlag)
{
	return (astr*)xOpenFileDialogA(hParent, sDefPath, sFilter, sTitle, iFlag | OFN_ALLOWMULTISELECT);
}



// 选择文件夹（需使用 xCore.free 释放）
XXAPI wstr xSelectFolderDialogW(HWND hParent, wstr sDefPath, wstr sTitle)
{
	// 安全检查
	if ( sTitle == NULL ) { sTitle = L"Select folder :"; }
	wstr sOutPath = malloc(DLG_BUFFSIZE_ONE * sizeof(wchar_t));
	if ( sDefPath == NULL ) { sOutPath[0] = 0; } else { wcscpy(sOutPath, sDefPath); }
	// 弹出对话框
	OPENFILENAMEW ofn;
	memset(&ofn, 0, sizeof(OPENFILENAMEW));
	ofn.lStructSize = sizeof(OPENFILENAMEW);
	ofn.hwndOwner = hParent;
	ofn.hInstance = GetModuleHandle(NULL);
	ofn.lpstrFilter = L"folder\0..\0\0";
	ofn.lpstrFile = sOutPath;
	ofn.lpstrInitialDir = sOutPath;
	ofn.nMaxFile = DLG_BUFFSIZE_ONE;
	ofn.lpstrTitle = sTitle;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST |OFN_ENABLEHOOK;
	ofn.lpfnHook = priSelectFolderHookProc;
	xCore.iRet = GetOpenFileNameW(&ofn);
	// 释放内存，返回结果
	wstr sRet;
	if ( xCore.iRet ) { sRet = xCore_CopyStringW(sOutPath, 0); } else { sRet = (wstr)xCore.nullstring; }
	free(sOutPath);
	return sRet;
}
XXAPI astr xSelectFolderDialogA(HWND hParent, astr sDefPath, astr sTitle)
{
	wstr sDefPathW = NULL;
	wstr sTitleW = NULL;
	if ( sDefPath ) { sDefPathW = xCore_A2W(sDefPath, 0); }
	if ( sTitle ) { sTitleW = xCore_A2W(sTitle, 0); }
	wstr sRetW = xSelectFolderDialogW(hParent, sDefPathW, sTitleW);
	if ( sDefPathW ) { xCore_free(sDefPathW); }
	if ( sTitleW ) { xCore_free(sTitleW); }
	if ( xCore.iRet ) {
		astr sRet = xCore_W2A(sRetW, 0);
		xCore_free(sRetW);
		return sRet;
	} else {
		return (astr)xCore.nullstring;
	}
}



// 保存文件（需使用 xCore.free 释放）
XXAPI wstr xSaveFileDialogW(HWND hParent, wstr sDefPath, wstr sFilter, wstr sTitle, int iFlag)
{
	// 安全检查
	sFilter = priConvFilterW(sFilter);
	if ( sTitle == NULL ) { sTitle = L"Save file :"; }
	wstr sOutPath = malloc(DLG_BUFFSIZE_ONE * sizeof(wchar_t));
	if ( sDefPath == NULL ) { sOutPath[0] = 0; } else { wcscpy(sOutPath, sDefPath); }
	// 弹出对话框
	OPENFILENAMEW ofn;
	memset(&ofn, 0, sizeof(OPENFILENAMEW));
	ofn.lStructSize = sizeof(OPENFILENAMEW);
	ofn.hwndOwner = hParent;
	ofn.hInstance = GetModuleHandle(NULL);
	ofn.lpstrFilter = sFilter;
	ofn.lpstrFile = sOutPath;
	ofn.lpstrInitialDir = sOutPath;
	ofn.nMaxFile = DLG_BUFFSIZE_ONE;
	ofn.lpstrTitle = sTitle;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER | iFlag;
	xCore.iRet = GetSaveFileNameW(&ofn);
	// 释放内存，返回结果
	xCore.free(sFilter);
	wstr sRet;
	if ( xCore.iRet ) { sRet = xCore_CopyStringW(sOutPath, 0); } else { sRet = (wstr)xCore.nullstring; }
	free(sOutPath);
	return sRet;
}
XXAPI astr xSaveFileDialogA(HWND hParent, astr sDefPath, astr sFilter, astr sTitle, int iFlag)
{
	// 安全检查
	sFilter = priConvFilterA(sFilter);
	if ( sTitle == NULL ) { sTitle = "Save file :"; }
	astr sOutPath = malloc(DLG_BUFFSIZE_ONE);
	if ( sDefPath == NULL ) { sOutPath[0] = 0; } else { strcpy(sOutPath, sDefPath); }
	// 弹出对话框
	OPENFILENAMEA ofn;
	memset(&ofn, 0, sizeof(OPENFILENAMEA));
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = hParent;
	ofn.hInstance = GetModuleHandle(NULL);
	ofn.lpstrFilter = sFilter;
	ofn.lpstrFile = sOutPath;
	ofn.lpstrInitialDir = sOutPath;
	ofn.nMaxFile = DLG_BUFFSIZE_ONE;
	ofn.lpstrTitle = sTitle;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER | iFlag;
	xCore.iRet = GetSaveFileNameA(&ofn);
	// 释放内存，返回结果
	xCore.free(sFilter);
	astr sRet;
	if ( xCore.iRet ) { sRet = xCore_CopyStringA(sOutPath, 0); } else { sRet = (astr)xCore.nullstring; }
	free(sOutPath);
	return sRet;
}



// 选择字体
XXAPI LOGFONTW* xSelectFontDialogW(HWND hParent)
{
	CHOOSEFONTW cf;
	LOGFONTW lf;
	memset(&cf, 0, sizeof(CHOOSEFONTW));
	cf.lStructSize = sizeof(CHOOSEFONTW);
	cf.hwndOwner = hParent;
	cf.lpLogFont = &lf;
	cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_EFFECTS;
	xCore.iRet = ChooseFontW(&cf);
	if ( xCore.iRet ) {
		return &lf;
	} else {
		return NULL;
	}
}
XXAPI LOGFONTA* xSelectFontDialogA(HWND hParent)
{
	CHOOSEFONTA cf;
	LOGFONTA lf;
	memset(&cf, 0, sizeof(CHOOSEFONTA));
	cf.lStructSize = sizeof(CHOOSEFONTA);
	cf.hwndOwner = hParent;
	cf.lpLogFont = &lf;
	cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_EFFECTS;
	xCore.iRet = ChooseFontA(&cf);
	if ( xCore.iRet ) {
		return &lf;
	} else {
		return NULL;
	}
}



// 选择颜色
XXAPI int xSelectColorDialog(HWND hParent, int iColor)
{
	CHOOSECOLORW cc;
	COLORREF arrColor[16];
	memset(&cc, 0, sizeof(CHOOSECOLORW));
	cc.lStructSize = sizeof(CHOOSECOLORW);
	cc.hwndOwner = hParent;
	cc.rgbResult = iColor;
	cc.lpCustColors = arrColor;
	cc.Flags = CC_RGBINIT | CC_FULLOPEN;
	xCore.iRet = ChooseColorW(&cc);
	if ( xCore.iRet ) {
		return cc.rgbResult;
	} else {
		return 0;
	}
}


