#ifndef XPK_SERVICE_VERIFY_H
#define XPK_SERVICE_VERIFY_H

static inline int procXpkVerifyEntry(xpkObject objXpk, xpkEntry* pEntry)
{
	void* pData;
	uint64_t iSize;
	uint32_t iHash;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( procXpkEntryDeleted(pEntry) ) {
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorDeleted);
	}

	pData = procXpkReadEntryData(objXpk, pEntry, &iSize);
	if ( pData == NULL ) {
		return xpkLastError(objXpk);
	}

	iHash = xpkHash32Internal(pData, iSize);
	xpkFree(pData);
	if ( iHash != pEntry->iFileHash ) {
		return procXpkSetError(objXpk, XPK_ERR_HASH, sXpkErrorHashMismatch);
	}

	procXpkClearError(objXpk);
	return XPK_OK;
}

static inline int procXpkVerifyAllSolidEntries(xpkObject objXpk)
{
	uint32_t iPos;
	uint32_t iLiveCount;
	xpkEntry* pEntry;
	void* pSolidRaw;
	uint64_t iSolidSize;
	uint32_t iHash;
	int iRet;

	iRet = procXpkVisibleEntryCountStrict(objXpk, &iLiveCount);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( iLiveCount == 0 ) {
		procXpkClearError(objXpk);
		return XPK_OK;
	}

	pSolidRaw = NULL;
	iSolidSize = 0;
	iRet = procXpkReadSolidStream(objXpk, &pSolidRaw, &iSolidSize);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	for ( iPos = 1; iPos <= objXpk->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, iPos);
		if ( pEntry == NULL ) {
			xpkFreeInternal(pSolidRaw);
			return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}
		iRet = procXpkValidateLiveEntryLookup(objXpk, pEntry);
		if ( iRet != XPK_OK ) {
			xpkFreeInternal(pSolidRaw);
			return iRet;
		}
		if ( (pEntry->iDataOffset + pEntry->iFileSize) > iSolidSize ) {
			xpkFreeInternal(pSolidRaw);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}

		iHash = xpkHash32Internal((const uint8_t*)pSolidRaw + pEntry->iDataOffset, pEntry->iFileSize);
		if ( iHash != pEntry->iFileHash ) {
			xpkFreeInternal(pSolidRaw);
			return procXpkSetError(objXpk, XPK_ERR_HASH, sXpkErrorHashMismatch);
		}
	}

	xpkFreeInternal(pSolidRaw);
	procXpkClearError(objXpk);
	return XPK_OK;
}

static inline int procXpkVerifyAllEntries(xpkObject objXpk)
{
	uint32_t iPos;
	xpkEntry* pEntry;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( procXpkValidateEntryCountState(objXpk) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( objXpk->bSolidApplied ) {
		return procXpkVerifyAllSolidEntries(objXpk);
	}

	for ( iPos = 1; iPos <= objXpk->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, iPos);
		if ( pEntry == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}
		iRet = procXpkValidateLiveEntryLookup(objXpk, pEntry);
		if ( iRet != XPK_OK ) {
			return iRet;
		}

		iRet = procXpkVerifyEntry(objXpk, pEntry);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
	}

	procXpkClearError(objXpk);
	return XPK_OK;
}

static inline int procXpkCalcCurrentMetaBytes(xpkObject objXpk, uint64_t* pSizeRet)
{
	void* pMetaComp;
	uint32_t iMetaCompSize;
	uint8_t iMetaLevel;
	int iRet;

	if ( pSizeRet == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	*pSizeRet = 0;
	pMetaComp = NULL;
	iMetaCompSize = 0;
	iMetaLevel = (uint8_t)objXpk->objHead.metaComp;
	iRet = procXpkCodecEncode(objXpk, (uint8_t)objXpk->objHead.metaComp, objXpk->pPackageMeta, objXpk->iPackageMetaSize, &pMetaComp, &iMetaCompSize, &iMetaLevel);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	if ( pMetaComp != NULL ) {
		xpkFreeInternal(pMetaComp);
	}
	*pSizeRet = iMetaCompSize;
	return XPK_OK;
}

static inline int procXpkCalcCurrentEntryBytes(xpkObject objXpk, uint64_t* pSizeRet)
{
	void* pEntryRaw;
	void* pEntryComp;
	uint32_t iEntryRawSize;
	uint32_t iEntryCompSize;
	uint8_t iInfoLevel;
	int iRet;

	if ( pSizeRet == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	*pSizeRet = 0;
	pEntryRaw = NULL;
	pEntryComp = NULL;
	iEntryRawSize = 0;
	iEntryCompSize = 0;
	iInfoLevel = (uint8_t)objXpk->objHead.infoComp;

	iRet = procXpkEncodeEntryTable(objXpk, &pEntryRaw, &iEntryRawSize);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	iRet = procXpkCodecEncode(objXpk, (uint8_t)objXpk->objHead.infoComp, pEntryRaw, iEntryRawSize, &pEntryComp, &iEntryCompSize, &iInfoLevel);
	if ( pEntryRaw != NULL ) {
		xpkFreeInternal(pEntryRaw);
	}
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	if ( pEntryComp != NULL ) {
		xpkFreeInternal(pEntryComp);
	}
	*pSizeRet = iEntryCompSize;
	return XPK_OK;
}

static inline int procXpkStatCurrent(xpkObject objXpk, xpkStat* pStatRet)
{
	uint32_t iPos;
	xpkEntry* pEntry;
	uint64_t iEndPos;
	uint64_t iDataBytes;
	uint64_t iDataRawMax;
	uint64_t iMetaBytes;
	uint64_t iEntryBytes;
	uint64_t iDataArea;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pStatRet == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( procXpkValidateEntryCountState(objXpk) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( procXpkWriteQueueCount(objXpk) > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBufferedPending);
	}

	memset(pStatRet, 0, sizeof(*pStatRet));
	iDataBytes = 0;
	iDataRawMax = 0;
	for ( iPos = 1; iPos <= objXpk->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, iPos);
		if ( pEntry == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		}

		if ( objXpk->bSolidApplied ) {
			iEndPos = pEntry->iDataOffset + pEntry->iFileSize;
			if ( iEndPos > iDataRawMax ) {
				iDataRawMax = iEndPos;
			}
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}
		iRet = procXpkValidateLiveEntryLookup(objXpk, pEntry);
		if ( iRet != XPK_OK ) {
			return iRet;
		}

		pStatRet->fileCount++;
		if ( objXpk->bSolidApplied ) {
			iDataBytes += pEntry->iFileSize;
		} else {
			iDataBytes += pEntry->iDataSize;
		}
	}

	iRet = procXpkCalcCurrentMetaBytes(objXpk, &iMetaBytes);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	iRet = procXpkCalcCurrentEntryBytes(objXpk, &iEntryBytes);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	if ( objXpk->bSolidApplied ) {
		pStatRet->liveDataBytes = iDataBytes;
		pStatRet->holeBytes = (iDataRawMax > iDataBytes) ? (iDataRawMax - iDataBytes) : 0;
	} else {
		iDataArea = 0;
		if ( objXpk->iAppendPos >= XPK_HEAD_SIZE ) {
			iDataArea = objXpk->iAppendPos - XPK_HEAD_SIZE;
		}

		pStatRet->liveDataBytes = iDataBytes;
		pStatRet->holeBytes = (iDataArea > iDataBytes) ? (iDataArea - iDataBytes) : 0;
	}
	pStatRet->metaBytes = iMetaBytes;
	pStatRet->entryTableBytes = iEntryBytes;
	procXpkClearError(objXpk);
	return XPK_OK;
}

#endif
