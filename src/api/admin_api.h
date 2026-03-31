/*
	xPack 管理 API 实现

	负责遍历、校验、统计、内存释放与错误查询接口。
*/

// 遍历全部可见条目
XPKAPI int xpkEach(xpkObject objXpk, xpkEachProc procEach, void* pArg)
{
	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	return procXpkEachWalk(objXpk, NULL, procEach, pArg);
}

// 按模式遍历匹配条目
XPKAPI int xpkEachMatch(xpkObject objXpk, const char* sPattern, xpkEachProc procEach, void* pArg)
{
	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	return procXpkEachWalk(objXpk, sPattern, procEach, pArg);
}

// 校验单个条目
XPKAPI int xpkVerify(xpkObject objXpk, uint32_t iPos)
{
	xpkEntry* pEntry;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( iPos == 0 ) {
		return procXpkReturnParamError(objXpk);
	}

	if ( procXpkGetPublicEntryByPos(objXpk, iPos, &pEntry) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	iRet = procXpkValidateLiveEntryLookup(objXpk, pEntry);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	return procXpkVerifyEntry(objXpk, pEntry);
}

// 校验全部条目
XPKAPI int xpkVerifyAll(xpkObject objXpk)
{
	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	return procXpkVerifyAllEntries(objXpk);
}

// 获取包统计信息
XPKAPI int xpkStatGet(xpkObject objXpk, xpkStat* pStatRet)
{
	return procXpkStatCurrent(objXpk, pStatRet);
}

// 释放 xPack 返回的内存
XPKAPI void xpkFree(void* pMem)
{
	xpkFreeInternal(pMem);
}

// 计算 32 位哈希值
XPKAPI uint32_t xpkHash32(const void* pData, uint64_t iSize)
{
	return xpkHash32Internal(pData, iSize);
}

// 获取最后错误码
XPKAPI xpkErrorCode xpkLastError(xpkObject objXpk)
{
	if ( objXpk == NULL ) {
		return (xpkErrorCode)g_objXpkErrorTls.iCode;
	}
	return (xpkErrorCode)objXpk->err.iCode;
}

// 获取最后错误文本
XPKAPI const char* xpkLastErrorMessage(xpkObject objXpk)
{
	if ( objXpk == NULL ) {
		if ( g_objXpkErrorTls.sText[0] == '\0' ) {
			return "";
		}
		return g_objXpkErrorTls.sText;
	}
	if ( objXpk->err.sText[0] == '\0' ) {
		return "";
	}
	return objXpk->err.sText;
}
