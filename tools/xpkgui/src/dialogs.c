#include "app.h"
#include "resources/resource.h"

#include <stdio.h>
#include <stdlib.h>

static const WCHAR* arrGuiWritePolicyText[] = {
	L"Buffered",
	L"Immediate"
};

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

static BOOL GuiDialogPackTypeSupportsCustomInfoExt(xpkPackType packType)
{
	return packType == XPK_PACK_CORE;
}

static void GuiArchiveConfigDialogSyncPackType(HWND hDlg)
{
	LRESULT selection;
	BOOL enableInfoExt;

	selection = SendDlgItemMessageW(hDlg, IDC_CFG_PACK_TYPE, CB_GETCURSEL, 0, 0);
	enableInfoExt = GuiDialogPackTypeSupportsCustomInfoExt((xpkPackType)selection);
	EnableWindow(GetDlgItem(hDlg, IDC_CFG_INFOEXT_SIZE), enableInfoExt);
	if ( !enableInfoExt ) {
		SetDlgItemInt(hDlg, IDC_CFG_INFOEXT_SIZE, 0, FALSE);
	}
}

static void GuiFillLevelCombo(HWND hDlg, int ctrlId, uint8_t current)
{
	HWND hCtrl;
	WCHAR text[64];
	int i;

	hCtrl = GetDlgItem(hDlg, ctrlId);
	for ( i = 0; i < 16; ++i ) {
		_snwprintf_s(text, _countof(text), _TRUNCATE, L"%d - %s", i, arrGuiMethodNames[i]);
		SendMessageW(hCtrl, CB_ADDSTRING, 0, (LPARAM)text);
	}
	SendMessageW(hCtrl, CB_SETCURSEL, current & 0x0F, 0);
}

static INT_PTR CALLBACK GuiInputDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	GuiInputDialogState* state;

	state = (GuiInputDialogState*)GetWindowLongPtrW(hDlg, GWLP_USERDATA);
	switch ( msg ) {
		case WM_INITDIALOG:
			state = (GuiInputDialogState*)lParam;
			SetWindowLongPtrW(hDlg, GWLP_USERDATA, (LONG_PTR)state);
			SetWindowTextW(hDlg, state->title);
			SetDlgItemTextW(hDlg, IDC_INPUT_PROMPT, state->prompt);
			SetDlgItemTextW(hDlg, IDC_INPUT_VALUE, state->value);
			return TRUE;
		case WM_COMMAND:
			switch ( LOWORD(wParam) ) {
				case IDOK:
					if ( state != NULL ) {
						GetDlgItemTextW(hDlg, IDC_INPUT_VALUE, state->value, (int)_countof(state->value));
					}
					EndDialog(hDlg, IDOK);
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlg, IDCANCEL);
					return TRUE;
			}
			break;
	}
	return FALSE;
}

static INT_PTR CALLBACK GuiArchiveConfigDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	GuiArchiveConfigDialogState* state;
	HWND hCtrl;

	state = (GuiArchiveConfigDialogState*)GetWindowLongPtrW(hDlg, GWLP_USERDATA);
	switch ( msg ) {
		case WM_INITDIALOG:
			state = (GuiArchiveConfigDialogState*)lParam;
			SetWindowLongPtrW(hDlg, GWLP_USERDATA, (LONG_PTR)state);
			SetWindowTextW(hDlg, state->createMode ? L"新建 xPack 归档" : L"归档设置");
			SetDlgItemTextW(hDlg, IDC_CFG_ARCHIVE_PATH, state->options.archivePath);

			hCtrl = GetDlgItem(hDlg, IDC_CFG_PACK_TYPE);
			SendMessageW(hCtrl, CB_ADDSTRING, 0, (LPARAM)L"Core");
			SendMessageW(hCtrl, CB_ADDSTRING, 0, (LPARAM)L"Index");
			SendMessageW(hCtrl, CB_ADDSTRING, 0, (LPARAM)L"Linux");
			SendMessageW(hCtrl, CB_ADDSTRING, 0, (LPARAM)L"Win32");
			SendMessageW(hCtrl, CB_SETCURSEL, state->options.packType, 0);

			hCtrl = GetDlgItem(hDlg, IDC_CFG_WRITE_POLICY);
			SendMessageW(hCtrl, CB_ADDSTRING, 0, (LPARAM)arrGuiWritePolicyText[0]);
			SendMessageW(hCtrl, CB_ADDSTRING, 0, (LPARAM)arrGuiWritePolicyText[1]);
			SendMessageW(hCtrl, CB_SETCURSEL, state->options.writePolicy == XPK_WRITE_IMMEDIATE ? 1 : 0, 0);

			GuiFillLevelCombo(hDlg, IDC_CFG_DEFAULT_COMP, state->options.defaultComp);
			GuiFillLevelCombo(hDlg, IDC_CFG_META_COMP, state->options.metaComp);
			GuiFillLevelCombo(hDlg, IDC_CFG_INFO_COMP, state->options.infoComp);

			SetDlgItemInt(hDlg, IDC_CFG_INFOEXT_SIZE, state->options.infoExtSize, FALSE);
			SetDlgItemInt(hDlg, IDC_CFG_VOLUME_SIZE, state->options.volumeSize, FALSE);
			CheckDlgButton(hDlg, IDC_CFG_SOLID_MODE, state->options.solidMode ? BST_CHECKED : BST_UNCHECKED);

			EnableWindow(GetDlgItem(hDlg, IDC_CFG_ARCHIVE_PATH), state->createMode);
			EnableWindow(GetDlgItem(hDlg, IDC_CFG_BROWSE), state->createMode);
			EnableWindow(GetDlgItem(hDlg, IDC_CFG_PACK_TYPE), state->allowPackTypeEdit);
			GuiArchiveConfigDialogSyncPackType(hDlg);
			return TRUE;
		case WM_COMMAND:
			switch ( LOWORD(wParam) ) {
				case IDC_CFG_PACK_TYPE:
					if ( HIWORD(wParam) == CBN_SELCHANGE ) {
						GuiArchiveConfigDialogSyncPackType(hDlg);
					}
					return TRUE;
				case IDC_CFG_BROWSE:
					if ( state != NULL && state->createMode ) {
						WCHAR pathBuf[MAX_PATH];
						GetDlgItemTextW(hDlg, IDC_CFG_ARCHIVE_PATH, pathBuf, _countof(pathBuf));
						if ( GuiSaveArchiveDialog(hDlg, pathBuf, _countof(pathBuf)) ) {
							SetDlgItemTextW(hDlg, IDC_CFG_ARCHIVE_PATH, pathBuf);
						}
					}
					return TRUE;
				case IDOK:
					if ( state != NULL ) {
						BOOL ok;
						state->options.packType = (xpkPackType)SendDlgItemMessageW(hDlg, IDC_CFG_PACK_TYPE, CB_GETCURSEL, 0, 0);
						state->options.defaultComp = (uint8_t)SendDlgItemMessageW(hDlg, IDC_CFG_DEFAULT_COMP, CB_GETCURSEL, 0, 0);
						state->options.metaComp = (uint8_t)SendDlgItemMessageW(hDlg, IDC_CFG_META_COMP, CB_GETCURSEL, 0, 0);
						state->options.infoComp = (uint8_t)SendDlgItemMessageW(hDlg, IDC_CFG_INFO_COMP, CB_GETCURSEL, 0, 0);
						state->options.writePolicy = (SendDlgItemMessageW(hDlg, IDC_CFG_WRITE_POLICY, CB_GETCURSEL, 0, 0) == 1) ? XPK_WRITE_IMMEDIATE : XPK_WRITE_BUFFERED;
						state->options.solidMode = (IsDlgButtonChecked(hDlg, IDC_CFG_SOLID_MODE) == BST_CHECKED);
						GetDlgItemTextW(hDlg, IDC_CFG_ARCHIVE_PATH, state->options.archivePath, _countof(state->options.archivePath));
						if ( GuiDialogPackTypeSupportsCustomInfoExt(state->options.packType) ) {
							state->options.infoExtSize = GetDlgItemInt(hDlg, IDC_CFG_INFOEXT_SIZE, &ok, FALSE);
							if ( !ok ) state->options.infoExtSize = 0;
						} else {
							state->options.infoExtSize = 0;
						}
						state->options.volumeSize = GetDlgItemInt(hDlg, IDC_CFG_VOLUME_SIZE, &ok, FALSE);
						if ( !ok ) state->options.volumeSize = 0;

						if ( state->createMode && state->options.archivePath[0] == L'\0' ) {
							MessageBoxW(hDlg, L"请指定归档路径。", XPKGUI_APP_TITLE, MB_OK | MB_ICONINFORMATION);
							return TRUE;
						}
					}
					EndDialog(hDlg, IDOK);
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlg, IDCANCEL);
					return TRUE;
			}
			break;
	}

	return FALSE;
}

static INT_PTR CALLBACK GuiTextEditorDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	GuiTextEditorDialogState* state;

	state = (GuiTextEditorDialogState*)GetWindowLongPtrW(hDlg, GWLP_USERDATA);
	switch ( msg ) {
		case WM_INITDIALOG:
			state = (GuiTextEditorDialogState*)lParam;
			SetWindowLongPtrW(hDlg, GWLP_USERDATA, (LONG_PTR)state);
			SetWindowTextW(hDlg, (state != NULL && state->title != NULL) ? state->title : L"Text Editor");
			SetDlgItemTextW(hDlg, IDC_TEXT_PROMPT, (state != NULL && state->prompt != NULL) ? state->prompt : L"");
			SetDlgItemTextW(hDlg, IDC_TEXT_VALUE, (state != NULL && state->text != NULL) ? state->text : L"");
			SendDlgItemMessageW(hDlg, IDC_TEXT_VALUE, EM_LIMITTEXT, 0x7FFFFFFE, 0);
			if ( state != NULL && state->readOnly ) {
				SendDlgItemMessageW(hDlg, IDC_TEXT_VALUE, EM_SETREADONLY, TRUE, 0);
				SetWindowTextW(GetDlgItem(hDlg, IDOK), L"Close");
				ShowWindow(GetDlgItem(hDlg, IDCANCEL), SW_HIDE);
			}
			return TRUE;
		case WM_COMMAND:
			switch ( LOWORD(wParam) ) {
				case IDOK:
					if ( state != NULL && !state->readOnly ) {
						HWND hEdit;
						int length;
						size_t cchRequired;
						WCHAR* newText;

						hEdit = GetDlgItem(hDlg, IDC_TEXT_VALUE);
						length = GetWindowTextLengthW(hEdit);
						if ( length < 0 ) {
							length = 0;
						}
						cchRequired = (size_t)length + 1;
						if ( state->text == NULL || state->cchText < cchRequired ) {
							newText = (WCHAR*)realloc(state->text, cchRequired * sizeof(WCHAR));
							if ( newText == NULL ) {
								MessageBoxW(hDlg, L"内存不足，无法保存文本内容。", XPKGUI_APP_TITLE, MB_OK | MB_ICONERROR);
								return TRUE;
							}
							state->text = newText;
							state->cchText = cchRequired;
						}
						GetDlgItemTextW(hDlg, IDC_TEXT_VALUE, state->text, (int)state->cchText);
					}
					EndDialog(hDlg, IDOK);
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlg, IDCANCEL);
					return TRUE;
			}
			break;
	}

	return FALSE;
}

BOOL GuiRunInputDialog(HWND hwnd, GuiInputDialogState* state)
{
	return DialogBoxParamW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDD_INPUT_DIALOG), hwnd, GuiInputDialogProc, (LPARAM)state) == IDOK;
}

BOOL GuiRunArchiveConfigDialog(HWND hwnd, GuiArchiveConfigDialogState* state)
{
	return DialogBoxParamW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDD_ARCHIVE_CONFIG), hwnd, GuiArchiveConfigDialogProc, (LPARAM)state) == IDOK;
}

BOOL GuiRunTextEditorDialog(HWND hwnd, GuiTextEditorDialogState* state)
{
	return DialogBoxParamW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDD_TEXT_EDITOR), hwnd, GuiTextEditorDialogProc, (LPARAM)state) == IDOK;
}
