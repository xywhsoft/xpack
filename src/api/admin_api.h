XPKAPI int xpkEach(xpkObject objXpk, xpkEachProc procEach, void* pArg)
{
	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	return procXpkEachWalk(objXpk, NULL, procEach, pArg);
}

XPKAPI int xpkEachMatch(xpkObject objXpk, const char* sPattern, xpkEachProc procEach, void* pArg)
{
	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	return procXpkEachWalk(objXpk, sPattern, procEach, pArg);
}

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

XPKAPI int xpkVerifyAll(xpkObject objXpk)
{
	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	return procXpkVerifyAllEntries(objXpk);
}

XPKAPI int xpkStatGet(xpkObject objXpk, xpkStat* pStatRet)
{
	return procXpkStatCurrent(objXpk, pStatRet);
}

XPKAPI void xpkFree(void* pMem)
{
	xpkFreeInternal(pMem);
}

XPKAPI uint32_t xpkHash32(const void* pData, uint64_t iSize)
{
	return xpkHash32Internal(pData, iSize);
}

XPKAPI xpkErrorCode xpkLastError(xpkObject objXpk)
{
	if ( objXpk == NULL ) {
		return (xpkErrorCode)g_objXpkErrorTls.iCode;
	}
	return (xpkErrorCode)objXpk->err.iCode;
}

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
