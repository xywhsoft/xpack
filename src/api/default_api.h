#ifndef XPK_API_DEFAULT_API_H
#define XPK_API_DEFAULT_API_H

static inline int procXpkRemoveUnsavedEntry(xpkObject objXpk, uint32_t iPos)
{
	xpkEntry* pEntry;
	xpkEntry objEntry;
	uint32_t iInsertPos;
	int iRet;

	pEntry = procXpkGetEntryByPos(objXpk, iPos);
	if ( pEntry == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorNotFound);
	}

	objEntry = *pEntry;
	if ( !xrtArrayRemove(&objXpk->arrEntry, iPos, 1) ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}
	objXpk->iEntryCount = objXpk->arrEntry.Count;

	iRet = procXpkRebuildLookup(objXpk);
	if ( iRet != XPK_OK ) {
		iInsertPos = xrtArrayInsert(&objXpk->arrEntry, iPos - 1, 1);
		if ( iInsertPos != iPos ) {
			procXpkFreeEntryOwned(&objEntry);
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}

		pEntry = (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, iPos);
		if ( pEntry == NULL ) {
			procXpkFreeEntryOwned(&objEntry);
			return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		}

		memset(pEntry, 0, sizeof(*pEntry));
		*pEntry = objEntry;
		objEntry.pInfoExt = NULL;
		objEntry.sPath = NULL;
		objXpk->iEntryCount = objXpk->arrEntry.Count;
		procXpkRebuildLookup(objXpk);
		return iRet;
	}

	procXpkFreeEntryOwned(&objEntry);
	procXpkDropQueuedWrite(objXpk, iPos);
	procXpkShiftQueuedWritePos(objXpk, iPos);
	return XPK_OK;
}

static inline int procXpkAddDataEntry(xpkObject objXpk, xpkEntry* pEntrySeed, const void* pData, uint64_t iSize, const xpkWriteOptions* pOpt, uint32_t* pPosRet)
{
	xpkEntry objEntry;
	xpkEntry* pEntry;
	int iRet;

	if ( pPosRet != NULL ) {
		*pPosRet = 0;
	}
	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( procXpkValidateEntryCountState(objXpk) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( pData == NULL && iSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	memset(&objEntry, 0, sizeof(objEntry));
	if ( pEntrySeed != NULL ) {
		objEntry = *pEntrySeed;
		if ( pEntrySeed->pInfoExt != NULL ) {
			objEntry.pInfoExt = procXpkDupInfoExt(objXpk, pEntrySeed->pInfoExt);
			if ( objEntry.pInfoExt == NULL ) {
				return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
			}
		} else if ( procXpkCoreInfoExtEnabled(objXpk) ) {
			objEntry.pInfoExt = procXpkAllocInfoExt(objXpk);
			if ( objEntry.pInfoExt == NULL ) {
				return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
			}
		}
		if ( pEntrySeed->sPath != NULL ) {
			objEntry.sPath = procXpkDupText(pEntrySeed->sPath);
			if ( objEntry.sPath == NULL ) {
				if ( objEntry.pInfoExt != NULL ) {
					xpkFreeInternal(objEntry.pInfoExt);
				}
				return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
			}
		}
	} else if ( procXpkCoreInfoExtEnabled(objXpk) ) {
		objEntry.pInfoExt = procXpkAllocInfoExt(objXpk);
		if ( objEntry.pInfoExt == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
	}

	if ( pOpt != NULL ) {
		objEntry.iFlag = (objEntry.iFlag & ~XPK_FLAG_TYPE_MASK) | (((uint32_t)pOpt->fileType & 0x0Fu) << 4);
	}

	iRet = procXpkAppendEntryOwned(objXpk, &objEntry);
	if ( iRet != XPK_OK ) {
		procXpkFreeEntryOwned(&objEntry);
		return iRet;
	}

	pEntry = procXpkGetEntryByPos(objXpk, objXpk->iEntryCount);
	if ( pEntry == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}

	iRet = procXpkStoreEntryData(objXpk, pEntry, pData, iSize, pOpt);
	if ( iRet != XPK_OK ) {
		procXpkRemoveUnsavedEntry(objXpk, pEntry->iPos);
		return iRet;
	}

	iRet = procXpkRebuildLookup(objXpk);
	if ( iRet != XPK_OK ) {
		procXpkRemoveUnsavedEntry(objXpk, pEntry->iPos);
		return iRet;
	}
	if ( pPosRet != NULL ) {
		*pPosRet = pEntry->iPos;
	}
	procXpkClearError(objXpk);
	return XPK_OK;
}

static inline int procXpkStoreEntryFile(xpkObject objXpk, xpkEntry* pEntry, const char* sSrcPath, const xpkWriteOptions* pOpt)
{
	xfile hFile;
	xpkMappedFile objMap;
	uint64_t iSize;
	uint8_t iLevel;
	uint8_t iWritePolicy;
	int iRet;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( sSrcPath == NULL || sSrcPath[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	iRet = procXpkResolveCompLevel(objXpk, pOpt, &iLevel);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	iWritePolicy = procXpkResolveWritePolicy(objXpk, pOpt);
	if ( iLevel == 0 && iWritePolicy == XPK_WRITE_IMMEDIATE ) {
		iRet = procXpkWriteImmediateStoreFile(objXpk, pEntry, sSrcPath);
		if ( iRet != XPK_OK ) {
			return iRet;
		}

		objXpk->bDirtyData = TRUE;
		procXpkMarkDirtyEntryTable(objXpk);
		objXpk->objHead.changeTime = xpkNowInternal();
		procXpkClearError(objXpk);
		return XPK_OK;
	}
	if ( (procXpkCompLevelToAlg(iLevel) == XPK_ALG_LZ4 || procXpkCompLevelToAlg(iLevel) == XPK_ALG_LZ4HC) && iWritePolicy == XPK_WRITE_IMMEDIATE ) {
		iRet = procXpkWriteImmediateLz4File(objXpk, pEntry, sSrcPath, iLevel);
		if ( iRet != XPK_OK ) {
			return iRet;
		}

		objXpk->bDirtyData = TRUE;
		procXpkMarkDirtyEntryTable(objXpk);
		objXpk->objHead.changeTime = xpkNowInternal();
		procXpkClearError(objXpk);
		return XPK_OK;
	}
	if ( (procXpkCompLevelToAlg(iLevel) == XPK_ALG_LZ4 || procXpkCompLevelToAlg(iLevel) == XPK_ALG_LZ4HC) && iWritePolicy == XPK_WRITE_BUFFERED ) {
		iRet = procXpkWriteBufferedLz4File(objXpk, pEntry, sSrcPath, iLevel);
		if ( iRet != XPK_OK ) {
			return iRet;
		}

		objXpk->bDirtyData = TRUE;
		objXpk->bDirtyEntryTable = TRUE;
		objXpk->bDirtyHead = TRUE;
		objXpk->objHead.changeTime = xpkNowInternal();
		procXpkClearError(objXpk);
		return XPK_OK;
	}
	if ( procXpkCompLevelToAlg(iLevel) == XPK_ALG_ZSTD && iWritePolicy == XPK_WRITE_IMMEDIATE ) {
		iRet = procXpkWriteImmediateZstdFile(objXpk, pEntry, sSrcPath, iLevel);
		if ( iRet != XPK_OK ) {
			return iRet;
		}

		objXpk->bDirtyData = TRUE;
		procXpkMarkDirtyEntryTable(objXpk);
		objXpk->objHead.changeTime = xpkNowInternal();
		procXpkClearError(objXpk);
		return XPK_OK;
	}
	if ( procXpkCompLevelToAlg(iLevel) == XPK_ALG_ZSTD && iWritePolicy == XPK_WRITE_BUFFERED ) {
		iRet = procXpkWriteBufferedZstdFile(objXpk, pEntry, sSrcPath, iLevel);
		if ( iRet != XPK_OK ) {
			return iRet;
		}

		objXpk->bDirtyData = TRUE;
		objXpk->bDirtyEntryTable = TRUE;
		objXpk->bDirtyHead = TRUE;
		objXpk->objHead.changeTime = xpkNowInternal();
		procXpkClearError(objXpk);
		return XPK_OK;
	}
	if ( procXpkCompLevelToAlg(iLevel) == XPK_ALG_LZMA2 && iWritePolicy == XPK_WRITE_IMMEDIATE ) {
		iRet = procXpkWriteImmediateLzma2File(objXpk, pEntry, sSrcPath, iLevel);
		if ( iRet != XPK_OK ) {
			return iRet;
		}

		objXpk->bDirtyData = TRUE;
		procXpkMarkDirtyEntryTable(objXpk);
		objXpk->objHead.changeTime = xpkNowInternal();
		procXpkClearError(objXpk);
		return XPK_OK;
	}
	if ( procXpkCompLevelToAlg(iLevel) == XPK_ALG_LZMA2 && iWritePolicy == XPK_WRITE_BUFFERED ) {
		iRet = procXpkWriteBufferedLzma2File(objXpk, pEntry, sSrcPath, iLevel);
		if ( iRet != XPK_OK ) {
			return iRet;
		}

		objXpk->bDirtyData = TRUE;
		objXpk->bDirtyEntryTable = TRUE;
		objXpk->bDirtyHead = TRUE;
		objXpk->objHead.changeTime = xpkNowInternal();
		procXpkClearError(objXpk);
		return XPK_OK;
	}

	hFile = NULL;
	memset(&objMap, 0, sizeof(objMap));
	iSize = 0;
	iRet = procXpkOpenMappedSourceFile(objXpk, sSrcPath, &hFile, &iSize, &objMap);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	iRet = procXpkStoreEntryData(objXpk, pEntry, objMap.pView, iSize, pOpt);
	procXpkUnmapFile(&objMap);
	xrtClose(hFile);
	return iRet;
}

static inline int procXpkAddFileEntry(xpkObject objXpk, xpkEntry* pEntrySeed, const char* sSrcPath, const xpkWriteOptions* pOpt, uint32_t* pPosRet)
{
	xpkEntry objEntry;
	xpkEntry* pEntry;
	int iRet;

	if ( pPosRet != NULL ) {
		*pPosRet = 0;
	}
	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( sSrcPath == NULL || sSrcPath[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( procXpkValidateEntryCountState(objXpk) != XPK_OK ) {
		return xpkLastError(objXpk);
	}

	memset(&objEntry, 0, sizeof(objEntry));
	if ( pEntrySeed != NULL ) {
		objEntry = *pEntrySeed;
		if ( pEntrySeed->pInfoExt != NULL ) {
			objEntry.pInfoExt = procXpkDupInfoExt(objXpk, pEntrySeed->pInfoExt);
			if ( objEntry.pInfoExt == NULL ) {
				return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
			}
		} else if ( procXpkCoreInfoExtEnabled(objXpk) ) {
			objEntry.pInfoExt = procXpkAllocInfoExt(objXpk);
			if ( objEntry.pInfoExt == NULL ) {
				return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
			}
		}
		if ( pEntrySeed->sPath != NULL ) {
			objEntry.sPath = procXpkDupText(pEntrySeed->sPath);
			if ( objEntry.sPath == NULL ) {
				if ( objEntry.pInfoExt != NULL ) {
					xpkFreeInternal(objEntry.pInfoExt);
				}
				return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
			}
		}
	} else if ( procXpkCoreInfoExtEnabled(objXpk) ) {
		objEntry.pInfoExt = procXpkAllocInfoExt(objXpk);
		if ( objEntry.pInfoExt == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
	}

	if ( pOpt != NULL ) {
		objEntry.iFlag = (objEntry.iFlag & ~XPK_FLAG_TYPE_MASK) | (((uint32_t)pOpt->fileType & 0x0Fu) << 4);
	}

	iRet = procXpkAppendEntryOwned(objXpk, &objEntry);
	if ( iRet != XPK_OK ) {
		procXpkFreeEntryOwned(&objEntry);
		return iRet;
	}

	pEntry = procXpkGetEntryByPos(objXpk, objXpk->iEntryCount);
	if ( pEntry == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}

	iRet = procXpkStoreEntryFile(objXpk, pEntry, sSrcPath, pOpt);
	if ( iRet != XPK_OK ) {
		procXpkRemoveUnsavedEntry(objXpk, pEntry->iPos);
		return iRet;
	}

	iRet = procXpkRebuildLookup(objXpk);
	if ( iRet != XPK_OK ) {
		procXpkRemoveUnsavedEntry(objXpk, pEntry->iPos);
		return iRet;
	}
	if ( pPosRet != NULL ) {
		*pPosRet = pEntry->iPos;
	}
	procXpkClearError(objXpk);
	return XPK_OK;
}

static inline int procXpkUpdateEntryData(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint64_t iSize, const xpkWriteOptions* pOpt)
{
	uint32_t iFlagOld;
	int iRet;

	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( pEntry == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorNotFound);
	}
	if ( procXpkEntryDeleted(pEntry) ) {
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorDeleted);
	}

	iFlagOld = pEntry->iFlag;
	if ( pOpt != NULL ) {
		pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_TYPE_MASK) | (((uint32_t)pOpt->fileType & 0x0Fu) << 4);
	}
	iRet = procXpkStoreEntryData(objXpk, pEntry, pData, iSize, pOpt);
	if ( iRet != XPK_OK ) {
		pEntry->iFlag = iFlagOld;
		return iRet;
	}
	return XPK_OK;
}

static inline int procXpkUpdateEntryFile(xpkObject objXpk, xpkEntry* pEntry, const char* sSrcPath, const xpkWriteOptions* pOpt)
{
	uint32_t iFlagOld;
	int iRet;

	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( pEntry == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorNotFound);
	}
	if ( procXpkEntryDeleted(pEntry) ) {
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorDeleted);
	}

	iFlagOld = pEntry->iFlag;
	if ( pOpt != NULL ) {
		pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_TYPE_MASK) | (((uint32_t)pOpt->fileType & 0x0Fu) << 4);
	}
	iRet = procXpkStoreEntryFile(objXpk, pEntry, sSrcPath, pOpt);
	if ( iRet != XPK_OK ) {
		pEntry->iFlag = iFlagOld;
		return iRet;
	}
	return XPK_OK;
}

static inline int procXpkValidatePublicPosAccess(xpkObject objXpk, xpkEntry* pEntry)
{
	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( procXpkEntryDeleted(pEntry) ) {
		return XPK_OK;
	}
	return procXpkValidateLiveEntryLookup(objXpk, pEntry);
}

XPKAPI uint32_t xpkCount(xpkObject objXpk)
{
	uint32_t iCount;

	if ( objXpk == NULL ) {
		procXpkSetError(NULL, XPK_ERR_PARAM, sXpkErrorInvalidParam);
		return 0;
	}
	if ( procXpkVisibleEntryCountStrict(objXpk, &iCount) != XPK_OK ) {
		return 0;
	}
	return iCount;
}

XPKAPI int xpkGetInfo(xpkObject objXpk, uint32_t iPos, xpkFileInfo* pInfoRet)
{
	xpkEntry* pEntry;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pInfoRet == NULL || iPos == 0 ) {
		return procXpkReturnParamError(objXpk);
	}

	if ( procXpkGetPublicEntryByPos(objXpk, iPos, &pEntry) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( procXpkEntryDeleted(pEntry) ) {
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorDeleted);
	}
	if ( procXpkValidatePublicPosAccess(objXpk, pEntry) != XPK_OK ) {
		return xpkLastError(objXpk);
	}

	memset(pInfoRet, 0, sizeof(*pInfoRet));
	pInfoRet->flag = pEntry->iFlag;
	pInfoRet->fileHash = pEntry->iFileHash;
	pInfoRet->dataOffset = pEntry->iDataOffset;
	pInfoRet->dataSize = pEntry->iDataSize;
	pInfoRet->fileSize = pEntry->iFileSize;
	procXpkClearError(objXpk);
	return XPK_OK;
}

XPKAPI int xpkGetInfoExt(xpkObject objXpk, uint32_t iPos, void* pDataRet, uint32_t iSize)
{
	xpkEntry* pEntry;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( !procXpkCoreInfoExtEnabled(objXpk) ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorCoreExtUnsupported);
	}
	if ( iPos == 0 ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( iSize > 0 && pDataRet == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( iSize != objXpk->objHead.infoExtSize ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInfoExtSizeMismatch);
	}

	if ( procXpkGetPublicEntryByPos(objXpk, iPos, &pEntry) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( procXpkEntryDeleted(pEntry) ) {
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorDeleted);
	}

	if ( pEntry->pInfoExt == NULL ) {
		memset(pDataRet, 0, iSize);
	} else if ( iSize > 0 ) {
		memcpy(pDataRet, pEntry->pInfoExt, iSize);
	}
	procXpkClearError(objXpk);
	return XPK_OK;
}

XPKAPI int xpkSetInfoExt(xpkObject objXpk, uint32_t iPos, const void* pData, uint32_t iSize)
{
	xpkEntry* pEntry;
	void* pInfoExt;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( !procXpkCoreInfoExtEnabled(objXpk) ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorCoreExtUnsupported);
	}
	if ( iPos == 0 ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( iSize > 0 && pData == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( iSize != objXpk->objHead.infoExtSize ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInfoExtSizeMismatch);
	}
	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	if ( procXpkGetPublicEntryByPos(objXpk, iPos, &pEntry) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( procXpkEntryDeleted(pEntry) ) {
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorDeleted);
	}

	if ( pEntry->pInfoExt == NULL ) {
		pInfoExt = procXpkAllocInfoExt(objXpk);
		if ( pInfoExt == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
		pEntry->pInfoExt = pInfoExt;
	}

	if ( iSize > 0 ) {
		memcpy(pEntry->pInfoExt, pData, iSize);
	}
	objXpk->bDirtyEntryTable = TRUE;
	objXpk->bDirtyHead = TRUE;
	procXpkClearError(objXpk);
	return XPK_OK;
}

XPKAPI int xpkAddFile(xpkObject objXpk, const char* sSrcPath, const xpkWriteOptions* pOpt, uint32_t* pPosRet)
{
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( sSrcPath == NULL || sSrcPath[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( objXpk->objHead.packType != XPK_PACK_CORE ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
	}
	if ( procXpkValidateEntryCountState(objXpk) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	return procXpkAddFileEntry(objXpk, NULL, sSrcPath, pOpt, pPosRet);
}

XPKAPI int xpkAddData(xpkObject objXpk, const void* pData, uint64_t iSize, const xpkWriteOptions* pOpt, uint32_t* pPosRet)
{
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pData == NULL && iSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( objXpk->objHead.packType != XPK_PACK_CORE ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
	}
	return procXpkAddDataEntry(objXpk, NULL, pData, iSize, pOpt, pPosRet);
}

XPKAPI int xpkReadToFile(xpkObject objXpk, uint32_t iPos, const char* sDstPath)
{
	xpkEntry* pEntry;
	xpkWriteNode* pNode;
	void* pData;
	uint64_t iSize;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( sDstPath == NULL || sDstPath[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	if ( procXpkGetPublicEntryByPos(objXpk, iPos, &pEntry) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( procXpkValidatePublicPosAccess(objXpk, pEntry) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( objXpk->bSolidApplied ) {
		return procXpkCopySolidEntryToFile(objXpk, pEntry, sDstPath);
	}
	if ( !objXpk->bSolidApplied ) {
		pNode = procXpkFindWriteNode(objXpk, pEntry->iPos, NULL);
		if ( (pNode != NULL) && ((procXpkCompLevelToAlg(pNode->iLevel) == XPK_ALG_LZ4) || (procXpkCompLevelToAlg(pNode->iLevel) == XPK_ALG_LZ4HC)) ) {
			return procXpkCopyQueuedLz4EntryToFile(objXpk, pEntry, sDstPath);
		}
		if ( (pNode != NULL) && (procXpkCompLevelToAlg(pNode->iLevel) == XPK_ALG_ZSTD) ) {
			return procXpkCopyQueuedZstdEntryToFile(objXpk, pEntry, sDstPath);
		}
		if ( (pNode != NULL) && (procXpkCompLevelToAlg(pNode->iLevel) == XPK_ALG_LZMA2) ) {
			return procXpkCopyQueuedLzma2EntryToFile(objXpk, pEntry, sDstPath);
		}
		if ( (pNode != NULL) && (pNode->iLevel == 0) ) {
			return procXpkCopyQueuedStoredEntryToFile(objXpk, pEntry, sDstPath);
		}
		if ( (pNode == NULL) && ((pEntry->iFlag & XPK_FLAG_COMP_MASK) == 0) ) {
			return procXpkCopyStoredEntryToFile(objXpk, pEntry, sDstPath);
		}
		if ( (pNode == NULL) && ((procXpkCompLevelToAlg((uint8_t)(pEntry->iFlag & XPK_FLAG_COMP_MASK)) == XPK_ALG_LZ4) || (procXpkCompLevelToAlg((uint8_t)(pEntry->iFlag & XPK_FLAG_COMP_MASK)) == XPK_ALG_LZ4HC)) ) {
			return procXpkCopyStoredLz4EntryToFile(objXpk, pEntry, sDstPath);
		}
		if ( (pNode == NULL) && (procXpkCompLevelToAlg((uint8_t)(pEntry->iFlag & XPK_FLAG_COMP_MASK)) == XPK_ALG_ZSTD) ) {
			return procXpkCopyStoredZstdEntryToFile(objXpk, pEntry, sDstPath);
		}
		if ( (pNode == NULL) && (procXpkCompLevelToAlg((uint8_t)(pEntry->iFlag & XPK_FLAG_COMP_MASK)) == XPK_ALG_LZMA2) ) {
			return procXpkCopyStoredLzma2EntryToFile(objXpk, pEntry, sDstPath);
		}
	}

	pData = procXpkReadEntryData(objXpk, pEntry, &iSize);
	if ( pData == NULL ) {
		return xpkLastError(objXpk);
	}

	iRet = procXpkWriteFileData(objXpk, sDstPath, pData, iSize);
	xpkFree(pData);
	return iRet;
}

XPKAPI void* xpkReadToMemory(xpkObject objXpk, uint32_t iPos, uint64_t* pSizeRet)
{
	xpkEntry* pEntry;

	if ( pSizeRet != NULL ) {
		*pSizeRet = 0;
	}
	if ( objXpk == NULL ) {
		procXpkSetError(NULL, XPK_ERR_PARAM, sXpkErrorInvalidParam);
		return NULL;
	}
	if ( iPos == 0 ) {
		procXpkSetParamErrorIfObject(objXpk);
		return NULL;
	}

	if ( procXpkGetPublicEntryByPos(objXpk, iPos, &pEntry) != XPK_OK ) {
		return NULL;
	}
	if ( procXpkValidatePublicPosAccess(objXpk, pEntry) != XPK_OK ) {
		return NULL;
	}
	return procXpkReadEntryData(objXpk, pEntry, pSizeRet);
}

XPKAPI int xpkUpdateFile(xpkObject objXpk, uint32_t iPos, const char* sSrcPath, const xpkWriteOptions* pOpt)
{
	xpkEntry* pEntry;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( iPos == 0 ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( sSrcPath == NULL || sSrcPath[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( procXpkGetPublicEntryByPos(objXpk, iPos, &pEntry) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( procXpkValidatePublicPosAccess(objXpk, pEntry) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( procXpkEntryDeleted(pEntry) ) {
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorDeleted);
	}
	return procXpkUpdateEntryFile(objXpk, pEntry, sSrcPath, pOpt);
}

XPKAPI int xpkUpdateData(xpkObject objXpk, uint32_t iPos, const void* pData, uint64_t iSize, const xpkWriteOptions* pOpt)
{
	xpkEntry* pEntry;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( iPos == 0 ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pData == NULL && iSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	if ( procXpkGetPublicEntryByPos(objXpk, iPos, &pEntry) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( procXpkValidatePublicPosAccess(objXpk, pEntry) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	return procXpkUpdateEntryData(objXpk, pEntry, pData, iSize, pOpt);
}

XPKAPI int xpkRemove(xpkObject objXpk, uint32_t iPos)
{
	xpkEntry* pEntry;
	uint32_t iFlagOld;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( iPos == 0 ) {
		return procXpkReturnParamError(objXpk);
	}
	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	if ( procXpkGetPublicEntryByPos(objXpk, iPos, &pEntry) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( procXpkEntryDeleted(pEntry) ) {
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorDeleted);
	}
	if ( procXpkValidatePublicPosAccess(objXpk, pEntry) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( !pEntry->bStored ) {
		iRet = procXpkRemoveUnsavedEntry(objXpk, iPos);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		objXpk->bDirtyEntryTable = TRUE;
		objXpk->bDirtyHead = TRUE;
		procXpkClearError(objXpk);
		return XPK_OK;
	}

	iFlagOld = pEntry->iFlag;
	pEntry->iFlag |= XPK_FLAG_DELETED_MASK;
	iRet = procXpkRebuildLookup(objXpk);
	if ( iRet != XPK_OK ) {
		pEntry->iFlag = iFlagOld;
		procXpkRebuildLookup(objXpk);
		return iRet;
	}
	procXpkDropQueuedWrite(objXpk, iPos);
	objXpk->bDirtyEntryTable = TRUE;
	objXpk->bDirtyHead = TRUE;
	procXpkClearError(objXpk);
	return XPK_OK;
}

XPKAPI int xpkSetFlag(xpkObject objXpk, uint32_t iPos, uint32_t iMask, uint32_t iValue)
{
	xpkEntry* pEntry;
	uint32_t iFlagOld;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( iPos == 0 ) {
		return procXpkReturnParamError(objXpk);
	}
	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	if ( procXpkGetPublicEntryByPos(objXpk, iPos, &pEntry) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( procXpkValidatePublicPosAccess(objXpk, pEntry) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( (iMask & XPK_FLAG_COMP_MASK) != 0 ) {
		if ( !procXpkEntryDeleted(pEntry) ) {
			if ( ((pEntry->iFlag ^ iValue) & XPK_FLAG_COMP_MASK) != 0 ) {
				return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorCompManaged);
			}
		}
	}

	iFlagOld = pEntry->iFlag;
	pEntry->iFlag = (pEntry->iFlag & ~iMask) | (iValue & iMask);
	if ( (iMask & XPK_FLAG_DELETED_MASK) != 0 ) {
		iRet = procXpkRebuildLookup(objXpk);
		if ( iRet != XPK_OK ) {
			pEntry->iFlag = iFlagOld;
			procXpkRebuildLookup(objXpk);
			return iRet;
		}
	}
	objXpk->bDirtyEntryTable = TRUE;
	objXpk->bDirtyHead = TRUE;
	procXpkClearError(objXpk);
	return XPK_OK;
}

#endif
