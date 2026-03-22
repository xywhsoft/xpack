#ifndef XPK_SERVICE_OPEN_H
#define XPK_SERVICE_OPEN_H

static inline int procXpkLoadPackage(xpkObject objXpk, const xpkOpenOptions* pOpt)
{
	xfile hFile;
	uint8_t* pHeadBuf;
	xpkMappedFile objMap;
	xpkHead objHead;
	void* pMetaComp;
	void* pEntryComp;
	void* pMetaRaw;
	void* pEntryRaw;
	uint64_t iEntryRawSize;
	uint64_t iMapSize;
	uint64_t iTailPos;
	uint32_t iLooseVolumeCount;
	int iRet;

	if ( xrtDirExists((str)objXpk->sPathPackage) ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}
	if ( !xrtFileExists(objXpk->sPathPackage) ) {
		if ( (pOpt != NULL) && pOpt->createIfMissing ) {
			iRet = procXpkEnsureVolumePathUnusedText(objXpk, objXpk->sPathPackage, sXpkErrorPackagePathExists);
			if ( iRet != XPK_OK ) {
				return iRet;
			}
			if ( objXpk->bReadonly ) {
				return procXpkSetError(objXpk, XPK_ERR_READONLY, sXpkErrorReadonly);
			}
			objXpk->bDirtyHead = TRUE;
			procXpkClearError(objXpk);
			return XPK_OK;
		}
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorNotFound);
	}

	hFile = xrtOpen(objXpk->sPathPackage, TRUE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	objXpk->iFileSize = xrtGetEOF(hFile);
	if ( objXpk->iFileSize == 0 ) {
		iLooseVolumeCount = 0;
		iRet = procXpkScanLooseVolumeFilesFromText(objXpk, objXpk->sPathPackage, 1, FALSE, &iLooseVolumeCount);
		if ( iRet != XPK_OK ) {
			xrtClose(hFile);
			return iRet;
		}
		if ( iLooseVolumeCount > 0 ) {
			xrtClose(hFile);
			return procXpkSetError(objXpk, XPK_ERR_EXISTS, sXpkErrorPackagePathExists);
		}
		xrtClose(hFile);
		objXpk->bDirtyHead = TRUE;
		procXpkClearError(objXpk);
		return XPK_OK;
	}
	if ( objXpk->iFileSize < XPK_HEAD_SIZE ) {
		xrtClose(hFile);
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadHead);
	}

	iRet = procXpkReadAtAlloc(objXpk, hFile, 0, XPK_HEAD_SIZE, (void**)&pHeadBuf);
	if ( iRet != XPK_OK ) {
		xrtClose(hFile);
		return iRet;
	}

	procXpkDecodeHead(pHeadBuf, &objHead);
	xrtFree(pHeadBuf);

	iRet = procXpkValidateHead(objXpk, &objHead);
	if ( iRet != XPK_OK ) {
		xrtClose(hFile);
		return iRet;
	}

	objXpk->objHead = objHead;
	if ( objHead.volumeMode ) {
		iRet = procXpkCalcLogicalFileSize(objXpk, &objHead, &objXpk->iFileSize);
		if ( iRet != XPK_OK ) {
			xrtClose(hFile);
			return iRet;
		}
	}

	iEntryRawSize = procXpkEntryTableRawSize(&objHead);
	iTailPos = objHead.dataOffset + objHead.metaCompSize + objHead.infoCompSize;
	if ( iTailPos > objXpk->iFileSize ) {
		xrtClose(hFile);
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( ((objHead.metaCompSize == 0) && (objHead.metaRawSize != 0)) ||
		((objHead.metaCompSize != 0) && (objHead.metaRawSize == 0)) ) {
		xrtClose(hFile);
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( (objHead.infoCompSize == 0) && (iEntryRawSize != 0) ) {
		xrtClose(hFile);
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( (objHead.infoCompSize != 0) && (iEntryRawSize == 0) ) {
		xrtClose(hFile);
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	objXpk->iAppendPos = objHead.dataOffset;
	objXpk->iFileSize = iTailPos;
	procXpkMarkAppliedLayout(objXpk);
	procXpkMarkClean(objXpk);

	memset(&objMap, 0, sizeof(objMap));
	iMapSize = 0;
	if ( !objHead.volumeMode && ((objHead.metaCompSize > 0) || (objHead.infoCompSize > 0)) ) {
		iMapSize = xrtGetEOF(hFile);
		iRet = procXpkMapFileReadOnly(objXpk, hFile, iMapSize, &objMap);
		if ( iRet != XPK_OK ) {
			xrtClose(hFile);
			return iRet;
		}
	}

	if ( objHead.metaCompSize > 0 ) {
		pMetaComp = NULL;
		pMetaRaw = NULL;
		if ( objMap.pView != NULL ) {
			if ( objHead.dataOffset > iMapSize || objHead.metaCompSize > (iMapSize - objHead.dataOffset) ) {
				procXpkUnmapFile(&objMap);
				xrtClose(hFile);
				return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
			}
			iRet = procXpkCodecDecode(objXpk, (uint8_t)objHead.metaComp, (const uint8_t*)objMap.pView + objHead.dataOffset, objHead.metaCompSize, objHead.metaRawSize, &pMetaRaw);
		} else {
			iRet = procXpkReadAtAlloc(objXpk, hFile, objHead.dataOffset, objHead.metaCompSize, &pMetaComp);
			if ( iRet != XPK_OK ) {
				procXpkUnmapFile(&objMap);
				xrtClose(hFile);
				return iRet;
			}
			iRet = procXpkCodecDecode(objXpk, (uint8_t)objHead.metaComp, pMetaComp, objHead.metaCompSize, objHead.metaRawSize, &pMetaRaw);
			xrtFree(pMetaComp);
		}
		if ( iRet != XPK_OK ) {
			procXpkUnmapFile(&objMap);
			xrtClose(hFile);
			return iRet;
		}
		if ( xpkHash32Internal(pMetaRaw, objHead.metaRawSize) != objHead.metaHash ) {
			xpkFreeInternal(pMetaRaw);
			procXpkUnmapFile(&objMap);
			xrtClose(hFile);
			return procXpkSetError(objXpk, XPK_ERR_HASH, sXpkErrorHashMismatch);
		}

		objXpk->pPackageMeta = pMetaRaw;
		objXpk->iPackageMetaSize = objHead.metaRawSize;
	}

	if ( objHead.infoCompSize > 0 ) {
		pEntryComp = NULL;
		pEntryRaw = NULL;
		if ( objMap.pView != NULL ) {
			iTailPos = objHead.dataOffset + objHead.metaCompSize;
			if ( iTailPos > iMapSize || objHead.infoCompSize > (iMapSize - iTailPos) ) {
				procXpkUnmapFile(&objMap);
				xrtClose(hFile);
				return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
			}
			iRet = procXpkCodecDecode(objXpk, (uint8_t)objHead.infoComp, (const uint8_t*)objMap.pView + iTailPos, objHead.infoCompSize, (uint32_t)iEntryRawSize, &pEntryRaw);
		} else {
			iRet = procXpkReadAtAlloc(objXpk, hFile, objHead.dataOffset + objHead.metaCompSize, objHead.infoCompSize, &pEntryComp);
			if ( iRet != XPK_OK ) {
				procXpkUnmapFile(&objMap);
				xrtClose(hFile);
				return iRet;
			}

			iRet = procXpkCodecDecode(objXpk, (uint8_t)objHead.infoComp, pEntryComp, objHead.infoCompSize, (uint32_t)iEntryRawSize, &pEntryRaw);
			xrtFree(pEntryComp);
		}
		if ( iRet != XPK_OK ) {
			procXpkUnmapFile(&objMap);
			xrtClose(hFile);
			return iRet;
		}
		if ( xpkHash32Internal(pEntryRaw, iEntryRawSize) != objHead.infoHash ) {
			xpkFreeInternal(pEntryRaw);
			procXpkUnmapFile(&objMap);
			xrtClose(hFile);
			return procXpkSetError(objXpk, XPK_ERR_HASH, sXpkErrorHashMismatch);
		}

		iRet = procXpkDecodeEntryTable(objXpk, pEntryRaw, (uint32_t)iEntryRawSize);
		xpkFreeInternal(pEntryRaw);
		if ( iRet != XPK_OK ) {
			procXpkUnmapFile(&objMap);
			xrtClose(hFile);
			return iRet;
		}
	}

	procXpkUnmapFile(&objMap);
	xrtClose(hFile);
	procXpkClearError(objXpk);
	return XPK_OK;
}

#endif
