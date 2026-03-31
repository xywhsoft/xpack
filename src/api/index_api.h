static inline xpkEntry* procXpkLookupIndexEntry(xpkObject objXpk, int64_t iFileIndex)
{
	uint32_t iPos;
	xpkEntry* pMap;
	xpkEntry* pEntry;

	if ( objXpk == NULL ) {
		return NULL;
	}

	procXpkClearError(objXpk);
	if ( procXpkValidateEntryCountState(objXpk) != XPK_OK ) {
		return NULL;
	}
	pMap = (xpkEntry*)xrtListGet(&objXpk->lstEntry, iFileIndex);
	if ( pMap == NULL ) {
		for ( iPos = 1; iPos <= objXpk->iEntryCount; iPos++ ) {
			pEntry = (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, iPos);
			if ( pEntry == NULL ) {
				procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
				return NULL;
			}
			if ( procXpkEntryDeleted(pEntry) ) {
				continue;
			}
			if ( pEntry->iFileIndex == iFileIndex ) {
				procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
				return NULL;
			}
		}
		return NULL;
	}

	pEntry = procXpkGetEntryByPos(objXpk, pMap->iPos);
	if ( pEntry == NULL || procXpkEntryDeleted(pEntry) || pEntry->iFileIndex != iFileIndex ) {
		procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		return NULL;
	}
	return pEntry;
}

XPKAPI int xpkIndexFind(xpkObject objXpk, int64_t iFileIndex, uint32_t* pPosRet)
{
	xpkEntry* pEntry;
	int iRet;

	if ( pPosRet != NULL ) {
		*pPosRet = 0;
	}
	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( objXpk->objHead.packType != XPK_PACK_INDEX ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
	}

	pEntry = procXpkLookupIndexEntry(objXpk, iFileIndex);
	if ( pEntry == NULL ) {
		iRet = xpkLastError(objXpk);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorNotFound);
	}

	if ( pPosRet != NULL ) {
		*pPosRet = pEntry->iPos;
	}
	procXpkClearError(objXpk);
	return XPK_OK;
}

XPKAPI int xpkIndexGetInfo(xpkObject objXpk, int64_t iFileIndex, xpkFileInfoIndex* pInfoRet)
{
	xpkEntry* pEntry;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( objXpk->objHead.packType != XPK_PACK_INDEX ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
	}
	if ( pInfoRet == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	pEntry = procXpkLookupIndexEntry(objXpk, iFileIndex);
	if ( pEntry == NULL ) {
		iRet = xpkLastError(objXpk);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorNotFound);
	}

	memset(pInfoRet, 0, sizeof(*pInfoRet));
	pInfoRet->flag = pEntry->iFlag;
	pInfoRet->fileHash = pEntry->iFileHash;
	pInfoRet->dataOffset = pEntry->iDataOffset;
	pInfoRet->dataSize = pEntry->iDataSize;
	pInfoRet->fileSize = pEntry->iFileSize;
	pInfoRet->fileIndex = pEntry->iFileIndex;
	procXpkClearError(objXpk);
	return XPK_OK;
}

XPKAPI int xpkIndexAddFile(xpkObject objXpk, int64_t iFileIndex, const char* sSrcPath, const xpkWriteOptions* pOpt)
{
	xpkEntry objEntry;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( objXpk->objHead.packType != XPK_PACK_INDEX ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
	}
	if ( sSrcPath == NULL || sSrcPath[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	iRet = xpkIndexFind(objXpk, iFileIndex, NULL);
	if ( iRet == XPK_OK ) {
		return procXpkSetError(objXpk, XPK_ERR_EXISTS, sXpkErrorExists);
	}
	if ( iRet != XPK_ERR_NOT_FOUND ) {
		return iRet;
	}
	procXpkClearError(objXpk);

	memset(&objEntry, 0, sizeof(objEntry));
	objEntry.iFileIndex = iFileIndex;
	return procXpkAddFileEntry(objXpk, &objEntry, sSrcPath, pOpt, NULL);
}

XPKAPI int xpkIndexAddData(xpkObject objXpk, int64_t iFileIndex, const void* pData, uint64_t iSize, const xpkWriteOptions* pOpt)
{
	xpkEntry objEntry;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( objXpk->objHead.packType != XPK_PACK_INDEX ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
	}
	if ( pData == NULL && iSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	iRet = xpkIndexFind(objXpk, iFileIndex, NULL);
	if ( iRet == XPK_OK ) {
		return procXpkSetError(objXpk, XPK_ERR_EXISTS, sXpkErrorExists);
	}
	if ( iRet != XPK_ERR_NOT_FOUND ) {
		return iRet;
	}
	procXpkClearError(objXpk);

	memset(&objEntry, 0, sizeof(objEntry));
	objEntry.iFileIndex = iFileIndex;
	return procXpkAddDataEntry(objXpk, &objEntry, pData, iSize, pOpt, NULL);
}

XPKAPI int xpkIndexReadToFile(xpkObject objXpk, int64_t iFileIndex, const char* sDstPath)
{
	uint32_t iPos;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( objXpk->objHead.packType != XPK_PACK_INDEX ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
	}
	if ( sDstPath == NULL || sDstPath[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	iRet = xpkIndexFind(objXpk, iFileIndex, &iPos);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	return xpkReadToFile(objXpk, iPos, sDstPath);
}

XPKAPI void* xpkIndexReadToMemory(xpkObject objXpk, int64_t iFileIndex, uint64_t* pSizeRet)
{
	uint32_t iPos;

	if ( xpkIndexFind(objXpk, iFileIndex, &iPos) != XPK_OK ) {
		if ( pSizeRet != NULL ) {
			*pSizeRet = 0;
		}
		return NULL;
	}
	return xpkReadToMemory(objXpk, iPos, pSizeRet);
}

XPKAPI int xpkIndexUpdateFile(xpkObject objXpk, int64_t iFileIndex, const char* sSrcPath, const xpkWriteOptions* pOpt)
{
	uint32_t iPos;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( objXpk->objHead.packType != XPK_PACK_INDEX ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
	}
	if ( sSrcPath == NULL || sSrcPath[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	iRet = xpkIndexFind(objXpk, iFileIndex, &iPos);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	return xpkUpdateFile(objXpk, iPos, sSrcPath, pOpt);
}

XPKAPI int xpkIndexUpdateData(xpkObject objXpk, int64_t iFileIndex, const void* pData, uint64_t iSize, const xpkWriteOptions* pOpt)
{
	uint32_t iPos;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( objXpk->objHead.packType != XPK_PACK_INDEX ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
	}
	if ( pData == NULL && iSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	iRet = xpkIndexFind(objXpk, iFileIndex, &iPos);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	return xpkUpdateData(objXpk, iPos, pData, iSize, pOpt);
}

XPKAPI int xpkIndexRemove(xpkObject objXpk, int64_t iFileIndex)
{
	uint32_t iPos;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( objXpk->objHead.packType != XPK_PACK_INDEX ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
	}
	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	iRet = xpkIndexFind(objXpk, iFileIndex, &iPos);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	return xpkRemove(objXpk, iPos);
}

XPKAPI int xpkIndexSetFlag(xpkObject objXpk, int64_t iFileIndex, uint32_t iMask, uint32_t iValue)
{
	uint32_t iPos;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( objXpk->objHead.packType != XPK_PACK_INDEX ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
	}
	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	iRet = xpkIndexFind(objXpk, iFileIndex, &iPos);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	return xpkSetFlag(objXpk, iPos, iMask, iValue);
}
