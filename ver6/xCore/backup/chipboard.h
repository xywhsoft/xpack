// 设置剪贴板内容
int xCore_ClipSetTextA(char* sText, int iSize)
{
	if ( OpenClipboard(NULL) == FALSE ) {
		return FALSE;
	}
	EmptyClipboard();
	if ( iSize == 0 ) {
		iSize = strlen(sText);
	}
	HGLOBAL pMem = GlobalAlloc(GMEM_MOVEABLE, iSize + 1);
	if ( pMem == NULL ) {
		CloseClipboard();
		return FALSE;
	}
	char* pStr = GlobalLock(pMem);
	if ( pStr == NULL ) {
		CloseClipboard();
		GlobalFree(pMem);
		return FALSE;
	}
	memcpy(pStr, sText, iSize);
	pStr[iSize] = 0;
	GlobalUnlock(pMem);
	SetClipboardData(CF_TEXT, pMem);
	CloseClipboard();
	return TRUE;
}



// 获取剪贴板内容
char* xCore_ClipGetTextA()
{
	if ( OpenClipboard(NULL) == FALSE ) {
		return xCore.nullstring;
	}
	if ( IsClipboardFormatAvailable(CF_TEXT) == FALSE ) {
		return xCore.nullstring;
	}
	HGLOBAL hMem = GetClipboardData(CF_TEXT);
	if ( hMem == NULL ) {
		return xCore.nullstring;
	}
	char* sText = GlobalLock(hMem);
	if ( sText == NULL ) {
		return xCore.nullstring;
	}
	uint iSize = strlen(sText);
	char* sRet = xCore.malloc(iSize + 1);
	if ( sRet == NULL ) {
		GlobalUnlock(hMem);
		return xCore.nullstring;
	}
	memcpy(sRet, sText, iSize);
	sRet[iSize] = 0;
	GlobalUnlock(hMem);
	CloseClipboard();
	return sRet;
}