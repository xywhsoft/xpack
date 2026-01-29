


/* ------------------------------------ C 函数库 ------------------------------------ */

// 多字节 转 Unicode
XXAPI wstr xCore_M2W(const str pStr, uint32 iCodePage, ulong iSize)
{
	if ( pStr ) {
		if (iSize == 0) { iSize = strlen(pStr); }
		if (iSize == 0) { xCore.iRetSize = 0; return (wstr)xCore.nullstring; }
		xCore.iRetSize = MultiByteToWideChar(iCodePage, 0, pStr, iSize, NULL, 0);
		ulong iMemSize = (xCore.iRetSize + 1) * sizeof(wchar_t);
		wstr pWStr = malloc(iMemSize);
		MultiByteToWideChar(iCodePage, 0, pStr, iSize, pWStr, xCore.iRetSize);
		pWStr[xCore.iRetSize] = 0;
		xCore.iRetSize = xCore.iRetSize * sizeof(wchar_t);
		return pWStr;
	}
	xCore.iRetSize = 0;
	return (wstr)xCore.nullstring;
}

// Unicode 转 多字节
XXAPI str xCore_W2M(const wstr pStr, uint32 iCodePage, ulong iSize)
{
    if ( pStr ) {
        if (iSize == 0) { iSize = wcslen(pStr); } else { iSize = iSize / sizeof(wchar_t); }
		if (iSize == 0) { xCore.iRetSize = 0; return xCore.nullstring; }
		xCore.iRetSize = WideCharToMultiByte(iCodePage, 0, pStr, iSize, NULL, 0, NULL, NULL);
		str pZStr = malloc(xCore.iRetSize + 1);
		WideCharToMultiByte(iCodePage, 0, pStr, iSize, pZStr, xCore.iRetSize, NULL, NULL);
		pZStr[xCore.iRetSize] = 0;
		return pZStr;
    }
	xCore.iRetSize = 0;
	return xCore.nullstring;
}

// ANSI 转 Unicode
XXAPI wstr xCore_A2W(const astr pZStr, ulong iSize)
{
	return xCore_M2W(pZStr, CP_ACP, iSize);
}

// utf-8 转 Unicode
XXAPI wstr xCore_U2W(const ustr pUStr, ulong iSize)
{
    return xCore_M2W(pUStr, CP_UTF8, iSize);
}

// Unicode 转 ANSI
XXAPI astr xCore_W2A(const wstr pWStr, ulong iSize)
{
    return xCore_W2M(pWStr, CP_ACP, iSize);
}

// Unicode 转 utf-8
XXAPI ustr xCore_W2U(const wstr pWStr, ulong iSize)
{
    return xCore_W2M(pWStr, CP_UTF8, iSize);
}

// ANSI 转 utf-8
XXAPI ustr xCore_A2U(const astr pZStr, ulong iSize)
{
    if ( pZStr ) {
        wstr sTmp = xCore_M2W(pZStr, CP_ACP , iSize * 2);
        ustr sRet = xCore_W2M(sTmp , CP_UTF8, iSize);
        free(sTmp);
        return sRet;
    }
	xCore.iRetSize = 0;
	return xCore.nullstring;
}

// utf-8 转 ANSI
XXAPI astr xCore_U2A(const ustr pUStr, ulong iSize)
{
    if ( pUStr ) {
        wstr sTmp = xCore_M2W(pUStr, CP_UTF8, iSize);
        astr sRet = xCore_W2M(sTmp , CP_ACP , iSize * 2);
        free(sTmp);
        return sRet;
    }
	xCore.iRetSize = 0;
	return xCore.nullstring;
}


