static inline void procXpkFillInfoCore(const xpkEntry* pEntry, xpkFileInfo* pInfoRet)
{
	memset(pInfoRet, 0, sizeof(*pInfoRet));
	pInfoRet->flag = pEntry->iFlag;
	pInfoRet->fileHash = pEntry->iFileHash;
	pInfoRet->dataOffset = pEntry->iDataOffset;
	pInfoRet->dataSize = pEntry->iDataSize;
	pInfoRet->fileSize = pEntry->iFileSize;
}

static inline void procXpkFillInfoIndex(const xpkEntry* pEntry, xpkFileInfoIndex* pInfoRet)
{
	memset(pInfoRet, 0, sizeof(*pInfoRet));
	pInfoRet->flag = pEntry->iFlag;
	pInfoRet->fileHash = pEntry->iFileHash;
	pInfoRet->dataOffset = pEntry->iDataOffset;
	pInfoRet->dataSize = pEntry->iDataSize;
	pInfoRet->fileSize = pEntry->iFileSize;
	pInfoRet->fileIndex = pEntry->iFileIndex;
}

static inline void procXpkFillInfoPath(const xpkEntry* pEntry, xpkFileInfoPath* pInfoRet)
{
	size_t iPathLen;

	memset(pInfoRet, 0, sizeof(*pInfoRet));
	pInfoRet->flag = pEntry->iFlag;
	pInfoRet->fileHash = pEntry->iFileHash;
	pInfoRet->dataOffset = pEntry->iDataOffset;
	pInfoRet->dataSize = pEntry->iDataSize;
	pInfoRet->fileSize = pEntry->iFileSize;
	if ( pEntry->sPath != NULL ) {
		iPathLen = strlen(pEntry->sPath);
		if ( iPathLen >= XPK_PATH_BYTES ) {
			iPathLen = XPK_PATH_BYTES - 1;
		}
		memcpy(pInfoRet->pathBytes, pEntry->sPath, iPathLen);
	}
	pInfoRet->platformAttr = pEntry->iPlatformAttr;
	pInfoRet->createTime = pEntry->tCreateTime;
	pInfoRet->modifyTime = pEntry->tModifyTime;
	pInfoRet->accessTime = pEntry->tAccessTime;
}

static inline const char* procXpkEntryMatchKey(xpkObject objXpk, const xpkEntry* pEntry, char sBuf[64], int* pCaseRet)
{
	if ( pCaseRet != NULL ) {
		*pCaseRet = FALSE;
	}

	if ( objXpk->objHead.packType == XPK_PACK_INDEX ) {
		snprintf(sBuf, 64, "%lld", (long long)pEntry->iFileIndex);
		return sBuf;
	}
	if ( (objXpk->objHead.packType == XPK_PACK_LINUX) || (objXpk->objHead.packType == XPK_PACK_WIN32) ) {
		if ( pCaseRet != NULL ) {
			*pCaseRet = (objXpk->objHead.packType == XPK_PACK_WIN32) ? TRUE : FALSE;
		}
		return pEntry->sPath;
	}

	snprintf(sBuf, 64, "%u", pEntry->iPos);
	return sBuf;
}

static inline int procXpkPatternMatchEntry(xpkObject objXpk, const xpkEntry* pEntry, const char* sPattern)
{
	char sBuf[64];
	const char* sKey;
	int bCase;

	if ( sPattern == NULL || sPattern[0] == '\0' ) {
		return TRUE;
	}

	sKey = procXpkEntryMatchKey(objXpk, pEntry, sBuf, &bCase);
	if ( sKey == NULL ) {
		return FALSE;
	}

	return xrtStrLike((str)sKey, 0, (str)sPattern, 0, bCase) ? TRUE : FALSE;
}

static inline int procXpkEachInvoke(xpkObject objXpk, const xpkEntry* pEntry, xpkEachProc procEach, void* pArg)
{
	xpkFileInfo objInfo;
	xpkFileInfoIndex objInfoIndex;
	xpkFileInfoPath objInfoPath;
	int* pCount;

	if ( procEach == NULL ) {
		pCount = (int*)pArg;
		if ( pCount != NULL ) {
			(*pCount)++;
		}
		return XPK_OK;
	}

	if ( objXpk->objHead.packType == XPK_PACK_INDEX ) {
		procXpkFillInfoIndex(pEntry, &objInfoIndex);
		return procEach(objXpk, pEntry->iPos, &objInfoIndex, pArg);
	}
	if ( (objXpk->objHead.packType == XPK_PACK_LINUX) || (objXpk->objHead.packType == XPK_PACK_WIN32) ) {
		procXpkFillInfoPath(pEntry, &objInfoPath);
		return procEach(objXpk, pEntry->iPos, &objInfoPath, pArg);
	}

	procXpkFillInfoCore(pEntry, &objInfo);
	return procEach(objXpk, pEntry->iPos, &objInfo, pArg);
}

static inline int procXpkEachWalk(xpkObject objXpk, const char* sPattern, xpkEachProc procEach, void* pArg)
{
	uint32_t iPos;
	xpkEntry* pEntry;
	int iRet;

	if ( objXpk == NULL ) {
		return XPK_ERR_PARAM;
	}
	if ( procXpkValidateEntryCountState(objXpk) != XPK_OK ) {
		return xpkLastError(objXpk);
	}

	for ( iPos = 1; iPos <= objXpk->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, iPos);
		if ( pEntry == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		}
		if ( ((objXpk->objHead.packType == XPK_PACK_LINUX) || (objXpk->objHead.packType == XPK_PACK_WIN32)) &&
			((pEntry->sPath == NULL) || (pEntry->sPath[0] == '\0')) ) {
			return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}
		iRet = procXpkValidateLiveEntryLookup(objXpk, pEntry);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		if ( !procXpkPatternMatchEntry(objXpk, pEntry, sPattern) ) {
			continue;
		}

		procXpkClearError(objXpk);
		iRet = procXpkEachInvoke(objXpk, pEntry, procEach, pArg);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
	}

	procXpkClearError(objXpk);
	return XPK_OK;
}
