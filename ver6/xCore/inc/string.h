


// 字符串格式化（需使用 xCore.free 释放）
XXAPI astr xCore_FormatA(astr sFormat, ...)
{
	if ( sFormat == NULL ) { return (astr)xCore.nullstring; }
	va_list ip;
	va_start(ip, sFormat);
	int iSize = vsnprintf(NULL, 0, sFormat, ip);
	va_end(ip);
	if ( iSize >= 0 ) {
		astr sRet = malloc(iSize + 1);
		va_start(ip, sFormat);
		iSize = vsnprintf(sRet, iSize + 1, sFormat, ip);
		va_end(ip);
		sRet[iSize] = 0;
		xCore.iRetSize = iSize;
		return sRet;
	} else {
		return (astr)xCore.nullstring;
	}
}
XXAPI wstr xCore_FormatW(wstr sFormat, ...)
{
	if ( sFormat == NULL ) { return (wstr)xCore.nullstring; }
	va_list ip;
	va_start(ip, sFormat);
	int iSize = vsnwprintf(NULL, 0, sFormat, ip);
	va_end(ip);
	if ( iSize >= 0 ) {
		wstr sRet = malloc( (iSize + 1) * 2 );
		va_start(ip, sFormat);
		iSize = vsnwprintf(sRet, iSize + 1, sFormat, ip);
		va_end(ip);
		sRet[iSize] = 0;
		xCore.iRetSize = iSize * 2;
		return sRet;
	} else {
		return (wstr)xCore.nullstring;
	}
}



// 字符串分割（需使用 xCore.free 释放）
XXAPI astr* xCore_SplitCharA(astr sText, char iChar, int bSrcRevise)
{
	if ( sText == NULL ) { sText = (astr)xCore.nullstring; }
	int iPos = 0;
	int iCount = 0;
	// 统计分隔符出现的次数
	while (TRUE) {
		if ( sText[iPos] == 0 ) {
			if (iChar == 0) {
				if ( sText[iPos+1] == 0 ) {
					break;
				}
			} else {
				break;
			}
		}
		if ( sText[iPos] == iChar ) {
			iCount++;
		}
		iPos++;
	}
	// 准备返回的数据 [分割指针 + NULL + 字符串表 + \0]
	astr* sRet;
	astr pData;
	if ( bSrcRevise ) {
		sRet = malloc( (iCount + 2) * sizeof(void*) );
		pData = sText;
	} else {
		sRet = malloc( ((iCount + 2) * sizeof(void*)) + iPos + 1);
		pData = (astr)&sRet[iCount + 2];
		memcpy(pData, sText, iPos);
		pData[iPos] = 0;
	}
	sRet[iCount + 1] = NULL;
	astr pAddr = pData;
	// 开始分割数据
	iCount = 0;
	for (int i = 0; i < iPos; i++) {
		if ( pData[i] == iChar ) {
			pData[i] = 0;
			sRet[iCount] = pAddr;
			pAddr = pData + i + 1;
			iCount++;
		}
	}
	sRet[iCount] = pAddr;
	xCore.iRetSize = iCount + 1;
	return sRet;
}

XXAPI astr* xCore_SplitTextA(astr sText, astr sSep, int bSrcRevise)
{
	if ( sText == NULL ) { sText = (astr)xCore.nullstring; }
	if ( sSep == NULL ) { sSep = ","; }
	size_t iSepSize = strlen(sSep);
	int iPos = 0;
	int iCount = 0;
	// 统计分隔符出现的次数
	while (TRUE) {
		if ( sText[iPos] == 0 ) {
			break;
		}
		if ( strncmp(&sText[iPos], sSep, iSepSize) == 0 ) {
			iCount++;
		}
		iPos++;
	}
	// 准备返回的数据 [分割指针 + NULL + 字符串表 + \0]
	astr* sRet;
	astr pData;
	if ( bSrcRevise ) {
		sRet = malloc( (iCount + 2) * sizeof(void*) );
		pData = sText;
	} else {
		sRet = malloc( ((iCount + 2) * sizeof(void*)) + iPos + 1);
		pData = (astr)&sRet[iCount + 2];
		memcpy(pData, sText, iPos);
		pData[iPos] = 0;
	}
	sRet[iCount + 1] = NULL;
	astr pAddr = pData;
	// 开始分割数据
	iCount = 0;
	for (int i = 0; i < iPos; i++) {
		if ( strncmp(&sText[i], sSep, iSepSize) == 0 ) {
			pData[i] = 0;
			sRet[iCount] = pAddr;
			pAddr = pData + i + iSepSize;
			i += iSepSize - 1;
			iCount++;
		}
	}
	sRet[iCount] = pAddr;
	xCore.iRetSize = iCount + 1;
	return sRet;
}

XXAPI wstr* xCore_SplitCharW(wstr sText, wchar_t iChar, int bSrcRevise)
{
	if ( sText == NULL ) { sText = (wstr)xCore.nullstring; }
	int iPos = 0;
	int iCount = 0;
	// 统计分隔符出现的次数
	while (TRUE) {
		if ( sText[iPos] == 0 ) {
			if (iChar == 0) {
				if ( sText[iPos+1] == 0 ) {
					break;
				}
			} else {
				break;
			}
		}
		if ( sText[iPos] == iChar ) {
			iCount++;
		}
		iPos++;
	}
	// 准备返回的数据 [分割指针 + NULL + 字符串表 + \0]
	wstr* sRet;
	wstr pData;
	if ( bSrcRevise ) {
		sRet = malloc( (iCount + 2) * sizeof(void*) );
		pData = sText;
	} else {
		sRet = malloc( ((iCount + 2) * sizeof(void*)) + ((iPos + 1) * sizeof(wchar_t)));
		pData = (wstr)&sRet[iCount + 2];
		memcpy(pData, sText, iPos * sizeof(wchar_t));
		pData[iPos] = 0;
	}
	sRet[iCount + 1] = NULL;
	wstr pAddr = pData;
	// 开始分割数据
	iCount = 0;
	for (int i = 0; i < iPos; i++) {
		if ( pData[i] == iChar ) {
			pData[i] = 0;
			sRet[iCount] = pAddr;
			pAddr = pData + i + 1;
			iCount++;
		}
	}
	sRet[iCount] = pAddr;
	xCore.iRetSize = iCount + 1;
	return sRet;
}

XXAPI wstr* xCore_SplitTextW(wstr sText, wstr sSep, int bSrcRevise)
{
	if ( sText == NULL ) { sText = (wstr)xCore.nullstring; }
	if ( sSep == NULL ) { sSep = L","; }
	size_t iSepSize = wcslen(sSep);
	int iPos = 0;
	int iCount = 0;
	// 统计分隔符出现的次数
	while (TRUE) {
		if ( sText[iPos] == 0 ) {
			break;
		}
		if ( wcsncmp(&sText[iPos], sSep, iSepSize) == 0 ) {
			iCount++;
		}
		iPos++;
	}
	// 准备返回的数据 [分割指针 + NULL + 字符串表 + \0]
	wstr* sRet;
	wstr pData;
	if ( bSrcRevise ) {
		sRet = malloc( (iCount + 2) * sizeof(void*) );
		pData = sText;
	} else {
		sRet = malloc( ((iCount + 2) * sizeof(void*)) + ((iPos + 1) * sizeof(wchar_t)));
		pData = (wstr)&sRet[iCount + 2];
		memcpy(pData, sText, iPos * sizeof(wchar_t));
		pData[iPos] = 0;
	}
	sRet[iCount + 1] = NULL;
	wstr pAddr = pData;
	// 开始分割数据
	iCount = 0;
	for (int i = 0; i < iPos; i++) {
		if ( wcsncmp(&sText[i], sSep, iSepSize) == 0 ) {
			pData[i] = 0;
			sRet[iCount] = pAddr;
			pAddr = pData + i + iSepSize;
			i += iSepSize - 1;
			iCount++;
		}
	}
	sRet[iCount] = pAddr;
	xCore.iRetSize = iCount + 1;
	return sRet;
}

XXAPI astr* xCore_SplitA(astr sText, astr sSep, int bSrcRevise)
{
	if ( sSep == NULL ) {
		return xCore_SplitCharA(sText, 0, bSrcRevise);
	} else {
		int iSize = strlen(sSep);
		if ( iSize == 1 ) {
			return xCore_SplitCharA(sText, sSep[0], bSrcRevise);
		} else {
			return xCore_SplitTextA(sText, sSep, bSrcRevise);
		}
	}
}
XXAPI wstr* xCore_SplitW(wstr sText, wstr sSep, int bSrcRevise)
{
	if ( sSep == NULL ) {
		return xCore_SplitCharW(sText, 0, bSrcRevise);
	} else {
		int iSize = wcslen(sSep);
		if ( iSize == 1 ) {
			return xCore_SplitCharW(sText, sSep[0], bSrcRevise);
		} else {
			return xCore_SplitTextW(sText, sSep, bSrcRevise);
		}
	}
}



// 字符串替换（需使用 xCore.free 释放）
XXAPI astr xCore_ReplaceA(astr original, astr pattern, astr replacement)
{
	if ( original == NULL ) { xCore.iRetSize = 0; return (astr)xCore.nullstring; }
	if ( pattern == NULL ) { xCore.iRetSize = strlen(original); return xCore_CopyStringA(original, xCore.iRetSize); }
	if ( replacement == NULL ) { replacement = (astr)xCore.nullstring; }
	
	size_t replen = strlen(replacement);
	size_t patlen = strlen(pattern);
	size_t orilen = strlen(original);
	
	size_t patcnt = 0;
	astr oriptr;
	astr patloc;
	
	// 计算 pattern 在原始字符串中出现的次数
	for ( oriptr = original; (patloc = strstr(oriptr, pattern)); oriptr = patloc + patlen ) {
		patcnt++;
	}
	
	// 为新字符串分配内存
	xCore.iRetSize = orilen + patcnt * (replen - patlen);
	astr returned = (astr)malloc( xCore.iRetSize + 1 );
	
	if ( returned != NULL ) {
		// 复制原始字符串, 替换需要改变的部分
		astr retptr = returned;
		for (oriptr = original; (patloc = strstr(oriptr, pattern)); oriptr = patloc + patlen)
		{
			size_t const skplen = patloc - oriptr;
			// 复制前面的部分，直到出现要查找的字符串
			strncpy(retptr, oriptr, skplen);
			retptr += skplen;
			// 复制要替换的字符串
			strncpy(retptr, replacement, replen);
			retptr += replen;
		}
		// 复制最后一段剩下的字符串
		strcpy(retptr, oriptr);
		return returned;
	} else {
		xCore.iRetSize = 0;
		return xCore.nullstring;
	}
}
XXAPI wstr xCore_ReplaceW(wstr original, wstr pattern, wstr replacement)
{
	if ( original == NULL ) { xCore.iRetSize = 0; return (wstr)xCore.nullstring; }
	if ( pattern == NULL ) { xCore.iRetSize = wcslen(original); return xCore_CopyStringW(original, xCore.iRetSize); }
	if ( replacement == NULL ) { replacement = (wstr)xCore.nullstring; }

	size_t replen = wcslen(replacement);
	size_t patlen = wcslen(pattern);
	size_t orilen = wcslen(original);
	
	size_t patcnt = 0;
	wstr oriptr;
	wstr patloc;
	
	// 计算 pattern 在原始字符串中出现的次数
	for ( oriptr = original; (patloc = wcsstr(oriptr, pattern)); oriptr = patloc + patlen ) {
		patcnt++;
	}
	
	// 为新字符串分配内存
	xCore.iRetSize = orilen + patcnt * (replen - patlen);
	wstr returned = (wstr)malloc( (xCore.iRetSize + 1) * sizeof(wchar_t) );
	
	if ( returned != NULL ) {
		// 复制原始字符串, 替换需要改变的部分
		wstr retptr = returned;
		for (oriptr = original; (patloc = wcsstr(oriptr, pattern)); oriptr = patloc + patlen)
		{
			size_t const skplen = patloc - oriptr;
			// 复制前面的部分，直到出现要查找的字符串
			wcsncpy(retptr, oriptr, skplen);
			retptr += skplen;
			// 复制要替换的字符串
			wcsncpy(retptr, replacement, replen);
			retptr += replen;
		}
		// 复制最后一段剩下的字符串
		wcscpy(retptr, oriptr);
		return returned;
	} else {
		xCore.iRetSize = 0;
		return (wstr)xCore.nullstring;
	}
}



// 创建字符串副本（需使用 xCore.free 释放）
XXAPI wstr xCore_CopyStringW(wstr sText, int iSize)
{
	if ( sText == NULL ) { return (wstr)xCore.nullstring; }
	if ( iSize == 0 ) { iSize = wcslen(sText); }
	if ( iSize == 0 ) { return (wstr)xCore.nullstring; }
	wstr sRet = malloc((iSize + 1) * sizeof(wchar_t));
	if ( sRet == 0 ) { return (wstr)xCore.nullstring; }
	memcpy(sRet, sText, iSize * sizeof(wchar_t));
	sRet[iSize] = 0;
	return sRet;
}
XXAPI astr xCore_CopyStringA(astr sText, int iSize)
{
	if ( sText == NULL ) { return (astr)xCore.nullstring; }
	if ( iSize == 0 ) { iSize = strlen(sText); }
	if ( iSize == 0 ) { return (astr)xCore.nullstring; }
	astr sRet = malloc(iSize + 1);
	if ( sRet == NULL ) { return xCore.nullstring; }
	memcpy(sRet, sText, iSize);
	sRet[iSize] = 0;
	return sRet;
}



// 搜索字符串（没找到字符串的情况下会返回 NULL）
XXAPI wstr xCore_FindStrW(wstr sText, wstr sSubText, int bCase)
{
	if ( sText == NULL ) { return (wstr)xCore.nullstring; }
	if ( sSubText == NULL ) { xCore.iRetSize = 0; return sText; }
	wstr sSub;
	if ( bCase ) {
		wstr sSrc = sText;
		sText = xCore_LCaseW(sText, 0, false);
		sSubText = xCore_LCaseW(sSubText, 0, false);
		sSub = wcsstr(sText, sSubText);
		free(sText);
		free(sSubText);
		if ( sSub ) { sSub = &sSrc[sSub - sText]; sText = sSrc; }
	} else {
		sSub = wcsstr(sText, sSubText);
	}
	if ( sSub ) {
		xCore.iRetSize = (sSub - sText) + 1;
		return sSub;
	} else {
		xCore.iRetSize = 0;
		return NULL;
	}
}
XXAPI uint xCore_InStrW(wstr sText, wstr sSubText, int bCase)
{
	xCore_FindStrW(sText, sSubText, bCase);
	return xCore.iRetSize;
}
XXAPI astr xCore_FindStrA(astr sText, astr sSubText, int bCase)
{
	if ( sText == NULL ) { return (astr)xCore.nullstring; }
	if ( sSubText == NULL ) { xCore.iRetSize = 0; return sText; }
	astr sSub;
	if ( bCase ) {
		astr sSrc = sText;
		sText = xCore_LCaseA(sText, 0, false);
		sSubText = xCore_LCaseA(sSubText, 0, false);
		sSub = strstr(sText, sSubText);
		free(sText);
		free(sSubText);
		if ( sSub ) { sSub = &sSrc[sSub - sText]; sText = sSrc; }
	} else {
		sSub = strstr(sText, sSubText);
	}
	if ( sSub ) {
		xCore.iRetSize = (sSub - sText) + 1;
		return sSub;
	} else {
		xCore.iRetSize = 0;
		return NULL;
	}
}
XXAPI uint xCore_InStrA(astr sText, astr sSubText, int bCase)
{
	xCore_FindStrA(sText, sSubText, bCase);
	return xCore.iRetSize;
}



// 字符串转为小写（bSrcRevise 为 false 时，需使用 xCore.free 释放内存）
XXAPI wstr xCore_LCaseW(wstr sText, int iSize, int bSrcRevise)
{
	if ( sText == NULL ) { return (wstr)xCore.nullstring; }
	if ( iSize == 0 ) { iSize = wcslen(sText); }
	if ( iSize == 0 ) { return (wstr)xCore.nullstring; }
	wstr sRet;
	if ( bSrcRevise ) { sRet = sText; } else { sRet = xCore_CopyStringW(sText, iSize); }
	for ( int i = 0; i < iSize; i++ ) {
		sRet[i] = tolower(sRet[i]);
	}
	return sRet;
}
XXAPI astr xCore_LCaseA(astr sText, int iSize, int bSrcRevise)
{
	if ( sText == NULL ) { return (astr)xCore.nullstring; }
	if ( iSize == 0 ) { iSize = strlen(sText); }
	if ( iSize == 0 ) { return (astr)xCore.nullstring; }
	astr sRet;
	if ( bSrcRevise ) { sRet = sText; } else { sRet = xCore_CopyStringA(sText, iSize); }
	for ( int i = 0; i < iSize; i++ ) {
		if ( sRet[i] & 0x80 ) {
			// 跳过 OEM 编码字符
			i++;
		} else {
			sRet[i] = tolower(sRet[i]);
		}
	}
	return sRet;
}



// 字符串转为大写（bSrcRevise 为 false 时，需使用 xCore.free 释放内存）
XXAPI wstr xCore_UCaseW(wstr sText, int iSize, int bSrcRevise)
{
	if ( sText == NULL ) { return (wstr)xCore.nullstring; }
	if ( iSize == 0 ) { iSize = wcslen(sText); }
	if ( iSize == 0 ) { return (wstr)xCore.nullstring; }
	wstr sRet;
	if ( bSrcRevise != false ) { sRet = sText; } else { sRet = xCore_CopyStringW(sText, iSize); }
	for ( int i = 0; i < iSize; i++ ) {
		sRet[i] = toupper(sRet[i]);
	}
	return sRet;
}
XXAPI astr xCore_UCaseA(astr sText, int iSize, int bSrcRevise)
{
	if ( sText == NULL ) { return (astr)xCore.nullstring; }
	if ( iSize == 0 ) { iSize = strlen(sText); }
	if ( iSize == 0 ) { return (astr)xCore.nullstring; }
	astr sRet;
	if ( bSrcRevise != false ) { sRet = sText; } else { sRet = xCore_CopyStringA(sText, iSize); }
	for ( int i = 0; i < iSize; i++ ) {
		if ( sRet[i] & 0x80 ) {
			// 跳过 OEM 编码字符
			i++;
		} else {
			sRet[i] = toupper(sRet[i]);
		}
	}
	return sRet;
}



// 裁剪字符串（bSrcRevise 为 false 时，需使用 xCore.free 释放内存）
XXAPI wstr xxLTrimW(wstr sText, wstr sSub, int bSrcRevise)
{
	if ( sText == NULL ) { xCore.iRetSize = 0; return (wstr)xCore.nullstring; }
	if ( sSub == NULL ) { sSub = L" \t\r\n"; }
	int iSize = wcslen(sText);
	int iCount = 0;
	for ( int i = 0; i < iSize; i++ ) {
		if ( wcschr(sSub, sText[i]) != NULL ) {
			iCount++;
		} else {
			break;
		}
	}
	xCore.iRetSize = iCount;
	if ( bSrcRevise ) {
		if ( iCount > 0 ) {
			memmove(sText, &sText[iCount], (iSize - iCount) * sizeof(wchar_t));
			sText[iSize - iCount] = 0;
		}
		return sText;
	} else {
		return xCore_CopyStringW(&sText[iCount], iSize - iCount);
	}
}
XXAPI wstr xxRTrimW(wstr sText, wstr sSub, int bSrcRevise)
{
	if ( sText == NULL ) { xCore.iRetSize = 0; return (wstr)xCore.nullstring; }
	if ( sSub == NULL ) { sSub = L" \t\r\n"; }
	int iSize = wcslen(sText);
	int iCount = 0;
	for ( int i = iSize - 1; i >= 0; i-- ) {
		if ( wcschr(sSub, sText[i]) != NULL ) {
			iCount++;
		} else {
			break;
		}
	}
	xCore.iRetSize = iCount;
	if ( bSrcRevise ) {
		if ( iCount > 0 ) {
			sText[iSize - iCount] = 0;
		}
		return sText;
	} else {
		return xCore_CopyStringW(sText, iSize - iCount);
	}
}
XXAPI wstr xxTrimW(wstr sText, wstr sSub, int bSrcRevise)
{
	if ( sText == NULL ) { xCore.iRetSize = 0; return (wstr)xCore.nullstring; }
	if ( sSub == NULL ) { sSub = L" \t\r\n"; }
	int iSize = wcslen(sText);
	int iCountL = 0;
	int iCountR = 0;
	for ( int i = 0; i < iSize; i++ ) {
		if ( wcschr(sSub, sText[i]) != NULL ) {
			iCountL++;
		} else {
			break;
		}
	}
	for ( int i = iSize - 1; i >= 0; i-- ) {
		if ( wcschr(sSub, sText[i]) != NULL ) {
			iCountR++;
		} else {
			break;
		}
	}
	int iCount = iCountL + iCountR;
	xCore.iRetSize = iCount;
	if ( bSrcRevise ) {
		if ( iCount > 0 ) {
			memmove(sText, &sText[iCountL], (iSize - iCount) * sizeof(wchar_t));
			sText[iSize - iCount] = 0;
		}
		return sText;
	} else {
		return xCore_CopyStringW(&sText[iCountL], iSize - iCount);
	}
}

// ANSI 版本尤其 OEM 编码的缘故，按单字节处理是不安全的，因此转为 UNICODE 处理
// 因为编码转换的缘故，bSrcRevise 参数不起作用，仅为了保证与 UNICODE 函数的兼容
XXAPI astr xxLTrimA(astr sText, astr sSub, int bSrcRevise)
{
	wstr sTextW = xCore_M2W(sText, CP_ACP, 0);
	wstr sSubW = xCore_M2W(sSub, CP_ACP, 0);
	wstr sRetW = xxLTrimW(sTextW, sSubW, TRUE);
	astr sRet = xCore_W2M(sRetW, CP_ACP, 0);
	xCore_free(sTextW);
	xCore_free(sSubW);
	return sRet;
}
XXAPI astr xxRTrimA(astr sText, astr sSub, int bSrcRevise)
{
	wstr sTextW = xCore_M2W(sText, CP_ACP, 0);
	wstr sSubW = xCore_M2W(sSub, CP_ACP, 0);
	wstr sRetW = xxRTrimW(sTextW, sSubW, TRUE);
	astr sRet = xCore_W2M(sRetW, CP_ACP, 0);
	xCore_free(sTextW);
	xCore_free(sSubW);
	return sRet;
}
XXAPI astr xxTrimA(astr sText, astr sSub, int bSrcRevise)
{
	wstr sTextW = xCore_M2W(sText, CP_ACP, 0);
	wstr sSubW = xCore_M2W(sSub, CP_ACP, 0);
	wstr sRetW = xxTrimW(sTextW, sSubW, TRUE);
	astr sRet = xCore_W2M(sRetW, CP_ACP, 0);
	xCore_free(sTextW);
	xCore_free(sSubW);
	return sRet;
}
XXAPI ustr xxLTrimU(ustr sText, ustr sSub, int bSrcRevise)
{
	wstr sTextW = xCore_M2W(sText, CP_UTF8, 0);
	wstr sSubW = xCore_M2W(sSub, CP_UTF8, 0);
	wstr sRetW = xxLTrimW(sTextW, sSubW, TRUE);
	ustr sRet = xCore_W2M(sRetW, CP_UTF8, 0);
	xCore_free(sTextW);
	xCore_free(sSubW);
	return sRet;
}
XXAPI ustr xxRTrimU(ustr sText, ustr sSub, int bSrcRevise)
{
	wstr sTextW = xCore_M2W(sText, CP_UTF8, 0);
	wstr sSubW = xCore_M2W(sSub, CP_UTF8, 0);
	wstr sRetW = xxRTrimW(sTextW, sSubW, TRUE);
	ustr sRet = xCore_W2M(sRetW, CP_UTF8, 0);
	xCore_free(sTextW);
	xCore_free(sSubW);
	return sRet;
}
XXAPI ustr xxTrimU(ustr sText, ustr sSub, int bSrcRevise)
{
	wstr sTextW = xCore_M2W(sText, CP_UTF8, 0);
	wstr sSubW = xCore_M2W(sSub, CP_UTF8, 0);
	wstr sRetW = xxTrimW(sTextW, sSubW, TRUE);
	ustr sRet = xCore_W2M(sRetW, CP_UTF8, 0);
	xCore_free(sTextW);
	xCore_free(sSubW);
	return sRet;
}



// 过滤字符串（bSrcRevise 为 false 时，需使用 xCore.free 释放内存）
XXAPI wstr xxStringFilterW(wstr sText, wstr sFilter, int bSrcRevise)
{
	if ( sText == NULL ) { xCore.iRetSize = 0; return (wstr)xCore.nullstring; }
	if ( bSrcRevise == FALSE ) { sText = xCore_CopyStringW(sText, 0); }
	if ( sFilter == NULL ) { xCore.iRetSize = 0; return sText; }
	if ( wcslen(sFilter) == 0 ) { xCore.iRetSize = 0; return sText; }
	int iSize = wcslen(sText);
	int iCount = 0;
	for ( int i = 0; i < iSize; i++ ) {
		if ( wcschr(sFilter, sText[i]) != NULL ) {
			iCount++;
			continue;
		}
		sText[i - iCount] = sText[i];
	}
	sText[iSize - iCount] = 0;
	xCore.iRetSize = iCount;
	return sText;
}

// ANSI 版本尤其 OEM 编码的缘故，按单字节处理是不安全的，因此转为 UNICODE 处理
// 因为编码转换的缘故，bSrcRevise 参数不起作用，仅为了保证与 UNICODE 函数的兼容
XXAPI astr xxStringFilterA(astr sText, astr sFilter, int bSrcRevise)
{
	wstr sTextW = xCore_M2W(sText, CP_ACP, 0);
	wstr sFilterW = xCore_M2W(sFilter, CP_ACP, 0);
	wstr sRetW = xxStringFilterW(sTextW, sFilterW, TRUE);
	astr sRet = xCore_W2M(sRetW, CP_ACP, 0);
	xCore_free(sTextW);
	xCore_free(sFilterW);
	return sRet;
}
XXAPI ustr xxStringFilterU(ustr sText, ustr sFilter, int bSrcRevise)
{
	wstr sTextW = xCore_M2W(sText, CP_UTF8, 0);
	wstr sFilterW = xCore_M2W(sFilter, CP_UTF8, 0);
	wstr sRetW = xxStringFilterW(sTextW, sFilterW, TRUE);
	ustr sRet = xCore_W2M(sRetW, CP_UTF8, 0);
	xCore_free(sTextW);
	xCore_free(sFilterW);
	return sRet;
}



int hex2dec(char c)
{
    if ('0' <= c && c <= '9') {
        return c - '0';
    } else if ('a' <= c && c <= 'f') {
        return c - 'a' + 10;
    } else if ('A' <= c && c <= 'F') {
        return c - 'A' + 10;
    } else {
        return -1;
    }
}

char dec2hex(short int c)
{
    if (0 <= c && c <= 9) {
        return c + '0';
    } else if (10 <= c && c <= 15) {
        return c + 'A' - 10;
    } else {
        return -1;
    }
}



// HEX 编码
XXAPI char* HexEncode(char* sMem, uint32 iSize)
{
	if ( iSize == 0 ) {
		iSize = strlen(sMem);
	}
    char* sRet = malloc((iSize * 3) + 1);
    int iPos = 0;
    for ( int i = 0; i < iSize; ++i ) {
        char c = sMem[i];
		int j = (short int)c;
		if (j < 0) { j += 256; }
		int i1 = j / 16;
		int i0 = j - i1 * 16;
		sRet[iPos++] = dec2hex(i1);
		sRet[iPos++] = dec2hex(i0);
    }
    sRet[iPos] = 0;
    return sRet;
}

// HEX 解码
XXAPI char* HexDecode(char* sMem, uint32 iSize)
{
	if ( iSize == 0 ) {
		iSize = strlen(sMem);
	}
    char* sRet = malloc(iSize + 1);
    int iPos = 0;
    for ( int i = 0; i < iSize; ++i ) {
		char c1 = sMem[i++];
		char c0 = sMem[i];
		int iChar = hex2dec(c1) * 16 + hex2dec(c0);
		printf("%d\n", iChar);
		sRet[iPos++] = iChar;
    }
    sRet[iPos] = 0;
    return sRet;
}



// URI 编码
XXAPI char* UriEncode(char* sURL)
{
    uint32 iSize = strlen(sURL);
    char* sRet = malloc((iSize * 3) + 1);
    int iPos = 0;
    for ( int i = 0; i < iSize; ++i ) {
        char c = sURL[i];
        // 无需编码的字符
        if ( (('0' <= c) && (c <= '9')) || (('a' <= c) && (c <= 'z')) || (('A' <= c) && (c <= 'Z')) || (c == '/') || (c == '.') || (c == '-') || (c == '_') ) {
            sRet[iPos++] = c;
        } else {
			// 处理协议头，协议头部分不进行编码 [ *://*/ ]
			if ( (c == ':') && (sURL[i+1] == '/') && (sURL[i+2] == '/') ) {
				int iStrPos = 0;
				for ( int j = i + 3; j < iSize; ++j ) {
					if ( (sURL[j] == '/') || (sURL[j] == 0) ) {
						iStrPos = j + 1;
						strncpy(&sRet[iPos], &sURL[i], iStrPos - i);
						iPos += iStrPos - i;
						i = j;
						break;
					}
				}
				if ( iStrPos ) { continue; }
			}
			// 编码其他字符
            int j = (short int)c;
            if (j < 0) { j += 256; }
            int i1 = j / 16;
            int i0 = j - i1 * 16;
            sRet[iPos++] = '%';
            sRet[iPos++] = dec2hex(i1);
            sRet[iPos++] = dec2hex(i0);
        }
    }
    sRet[iPos] = 0;
    return sRet;
}

// URI 解码
XXAPI char* UriDecode(char* sURL)
{
    uint32 iSize = strlen(sURL);
    char* sRet = malloc(iSize + 1);
    int iPos = 0;
    for ( int i = 0; i < iSize; ++i ) {
        char c = sURL[i];
        if (c != '%') {
            sRet[iPos++] = c;
        } else {
            char c1 = sURL[++i];
            char c0 = sURL[++i];
            int iChar = hex2dec(c1) * 16 + hex2dec(c0);
            sRet[iPos++] = iChar;
        }
    }
    sRet[iPos] = 0;
    return sRet;
}


