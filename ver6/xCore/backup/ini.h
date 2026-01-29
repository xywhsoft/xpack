


// 读文本
XXAPI astr Ini_ReadA(const astr sFile, const astr sSec, const astr sKey)
{
	astr sBuf = malloc(8192);
	uint32 iSize = GetPrivateProfileStringA(sSec, sKey, "", sBuf, 8192, sFile);
	sBuf[iSize] = 0;
	return sBuf;
}

XXAPI wstr Ini_ReadW(const wstr sFile, const wstr sSec, const wstr sKey)
{
	wstr sBuf = malloc(8192);
	uint32 iSize = GetPrivateProfileStringW(sSec, sKey, L"", sBuf, 8192, sFile);
	sBuf[iSize] = 0;
	return sBuf;
}

XXAPI ustr Ini_ReadU(const ustr sFile, const ustr sSec, const ustr sKey)
{
	wstr sFileW = xCore_U2W(sFile, 0);
	wstr sSecW = xCore_U2W(sSec, 0);
	wstr sKeyW = xCore_U2W(sKey, 0);
	wstr sBuf = malloc(8192);
	uint32 iSize = GetPrivateProfileStringW(sSecW, sKeyW, L"", sBuf, 8192, sFileW);
	sBuf[iSize] = 0;
	ustr sRet = xCore_W2U(sBuf, 0);
	free(sFileW);
	free(sSecW);
	free(sKeyW);
	free(sBuf);
	return sRet;
}

// 读数字
XXAPI uint32 Ini_ReadIntA(const astr sFile, const astr sSec, const astr sKey)
{
	return GetPrivateProfileIntA(sSec, sKey, 0, sFile);
}

XXAPI uint32 Ini_ReadIntW(const wstr sFile, const wstr sSec, const wstr sKey)
{
	return GetPrivateProfileIntW(sSec, sKey, 0, sFile);
}

XXAPI uint32 Ini_ReadIntU(const ustr sFile, const ustr sSec, const ustr sKey)
{
	wstr sFileW = xCore_U2W(sFile, 0);
	wstr sSecW = xCore_U2W(sSec, 0);
	wstr sKeyW = xCore_U2W(sKey, 0);
	uint32 iRet = GetPrivateProfileIntW(sSecW, sKeyW, 0, sFileW);
	free(sFileW);
	free(sSecW);
	free(sKeyW);
	return iRet;
}

// 写文本
XXAPI int Ini_WriteA(const astr sFile, const astr sSec, const astr sKey, const astr sVal)
{
	return WritePrivateProfileStringA(sSec, sKey, sVal, sFile);
}

XXAPI int Ini_WriteW(const wstr sFile, const wstr sSec, const wstr sKey, const wstr sVal)
{
	return WritePrivateProfileStringW(sSec, sKey, sVal, sFile);
}

XXAPI int Ini_WriteU(const ustr sFile, const ustr sSec, const ustr sKey, const ustr sVal)
{
	wstr sFileW = xCore_U2W(sFile, 0);
	wstr sSecW = xCore_U2W(sSec, 0);
	wstr sKeyW = xCore_U2W(sKey, 0);
	wstr sValW = xCore_U2W(sVal, 0);
	int iRet = WritePrivateProfileStringW(sSecW, sKeyW, sValW, sFileW);
	free(sFileW);
	free(sSecW);
	free(sKeyW);
	free(sValW);
	return iRet;
}


