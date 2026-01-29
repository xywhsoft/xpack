


// 释放资源 (RT_RCDATA)
/*
int Res_UnPackA(HINSTANCE hInst, int iResID, astr sFile, astr sTpe)
{
	if ( hInst == NULL ) { hInst = xCore.AppHandle; }
	HRSRC hRes = FindResource(hInst, MAKEINTRESOURCE(iResID), sTpe);
	if ( hRes ) {
		HGLOBAL ResData = LoadResource(hInst, hRes);
		if ( ResData ) {
			xCore.iRetSize = SizeofResource(hInst, hRes);
			ffrtl.FileWriteAllA(sFile, ResData, xCore.iRetSize, CHARSET_NULL);
			return TRUE;
		}
	}
	return FALSE;
}
*/


// 读取资源到内存
ptr Res_ReadA(HINSTANCE hInst, int iResID, astr sTpe)
{
	if ( hInst == NULL ) { hInst = xCore.AppHandle; }
	HRSRC hRes = FindResource(hInst, MAKEINTRESOURCE(iResID), sTpe);
	if ( hRes ) {
		HGLOBAL ResData = LoadResource(hInst, hRes);
		if ( ResData ) {
			xCore.iRetSize = SizeofResource(hInst, hRes);
			return ResData;
		}
	}
	return NULL;
}



// 读取资源文件中的字符串
char* Res_ReadTextA(HINSTANCE hInst, int iResID)
{
	char* sRet = NULL;
	if ( hInst == NULL ) { hInst = xCore.AppHandle; }
	xCore.iRetSize = LoadString(hInst, iResID, (ptr)&sRet, 0);
	if ( xCore.iRetSize == 0 ) {
		return xCore.nullstring;
	}
	return sRet;
}


