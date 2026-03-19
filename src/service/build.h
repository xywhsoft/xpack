#ifndef XPK_SERVICE_BUILD_H
#define XPK_SERVICE_BUILD_H

static inline int procXpkPathTextEqualFs(const char* sPathA, const char* sPathB)
{
	if ( sPathA == NULL || sPathB == NULL ) {
		return FALSE;
	}

#ifdef _WIN32
	size_t iPosA;
	size_t iPosB;
	int bSegStartA;
	int bSegStartB;
	unsigned char iChA;
	unsigned char iChB;

	iPosA = 0;
	iPosB = 0;
	bSegStartA = TRUE;
	bSegStartB = TRUE;
	for ( ;; ) {
		for ( ;; ) {
			if ( bSegStartA && sPathA[iPosA] == '.' &&
				(sPathA[iPosA + 1] == '/' || sPathA[iPosA + 1] == '\\') ) {
				iPosA += 2;
				while ( sPathA[iPosA] == '/' || sPathA[iPosA] == '\\' ) {
					iPosA++;
				}
				bSegStartA = TRUE;
				continue;
			}
			if ( sPathA[iPosA] == '/' || sPathA[iPosA] == '\\' ) {
				while ( sPathA[iPosA] == '/' || sPathA[iPosA] == '\\' ) {
					iPosA++;
				}
				if ( sPathA[iPosA] == '\0' ) {
					iChA = '\0';
					break;
				}
				iChA = '/';
				bSegStartA = TRUE;
				break;
			}
			iChA = (unsigned char)tolower((unsigned char)sPathA[iPosA]);
			if ( iChA != '\0' ) {
				iPosA++;
			}
			bSegStartA = FALSE;
			break;
		}

		for ( ;; ) {
			if ( bSegStartB && sPathB[iPosB] == '.' &&
				(sPathB[iPosB + 1] == '/' || sPathB[iPosB + 1] == '\\') ) {
				iPosB += 2;
				while ( sPathB[iPosB] == '/' || sPathB[iPosB] == '\\' ) {
					iPosB++;
				}
				bSegStartB = TRUE;
				continue;
			}
			if ( sPathB[iPosB] == '/' || sPathB[iPosB] == '\\' ) {
				while ( sPathB[iPosB] == '/' || sPathB[iPosB] == '\\' ) {
					iPosB++;
				}
				if ( sPathB[iPosB] == '\0' ) {
					iChB = '\0';
					break;
				}
				iChB = '/';
				bSegStartB = TRUE;
				break;
			}
			iChB = (unsigned char)tolower((unsigned char)sPathB[iPosB]);
			if ( iChB != '\0' ) {
				iPosB++;
			}
			bSegStartB = FALSE;
			break;
		}

		if ( iChA != iChB ) {
			return FALSE;
		}
		if ( iChA == '\0' ) {
			return TRUE;
		}
	}
#else
	return (strcmp(sPathA, sPathB) == 0) ? TRUE : FALSE;
#endif
}

static inline char* procXpkPathNormDupFs(const char* sPath)
{
	size_t iLenPath;
	char* sPathRet;

	if ( sPath == NULL ) {
		return NULL;
	}

#ifdef _WIN32
	size_t iPosIn;
	size_t iPosOut;
	int bSegStart;
	unsigned char iCh;

	iLenPath = strlen(sPath);
	sPathRet = (char*)xpkAllocInternal(iLenPath + 2);
	if ( sPathRet == NULL ) {
		return NULL;
	}

	iPosIn = 0;
	iPosOut = 0;
	bSegStart = TRUE;
	for ( ;; ) {
		for ( ;; ) {
			if ( bSegStart && sPath[iPosIn] == '.' &&
				(sPath[iPosIn + 1] == '/' || sPath[iPosIn + 1] == '\\') ) {
				iPosIn += 2;
				while ( sPath[iPosIn] == '/' || sPath[iPosIn] == '\\' ) {
					iPosIn++;
				}
				bSegStart = TRUE;
				continue;
			}
			if ( sPath[iPosIn] == '/' || sPath[iPosIn] == '\\' ) {
				while ( sPath[iPosIn] == '/' || sPath[iPosIn] == '\\' ) {
					iPosIn++;
				}
				if ( sPath[iPosIn] == '\0' ) {
					sPathRet[iPosOut] = '\0';
					return sPathRet;
				}
				sPathRet[iPosOut++] = '/';
				bSegStart = TRUE;
				break;
			}
			iCh = (unsigned char)tolower((unsigned char)sPath[iPosIn]);
			if ( iCh == '\0' ) {
				sPathRet[iPosOut] = '\0';
				return sPathRet;
			}
			sPathRet[iPosOut++] = (char)iCh;
			iPosIn++;
			bSegStart = FALSE;
			break;
		}
	}
#else
	iLenPath = strlen(sPath);
	sPathRet = (char*)xpkAllocInternal(iLenPath + 1);
	if ( sPathRet == NULL ) {
		return NULL;
	}
	memcpy(sPathRet, sPath, iLenPath + 1);
	return sPathRet;
#endif
}

static inline int procXpkPathInDirFs(xpkObject objXpk, const char* sPath, const char* sDir, int* pMatchRet)
{
	char* sPathNorm;
	char* sDirNorm;
	size_t iDirLen;

	if ( pMatchRet != NULL ) {
		*pMatchRet = FALSE;
	}
	if ( sPath == NULL || sDir == NULL || pMatchRet == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	sPathNorm = procXpkPathNormDupFs(sPath);
	if ( sPathNorm == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}
	sDirNorm = procXpkPathNormDupFs(sDir);
	if ( sDirNorm == NULL ) {
		xpkFreeInternal(sPathNorm);
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	iDirLen = strlen(sDirNorm);
	if ( iDirLen > 0 && strncmp(sPathNorm, sDirNorm, iDirLen) == 0 && sPathNorm[iDirLen] == '/' ) {
		*pMatchRet = TRUE;
	}

	xpkFreeInternal(sPathNorm);
	xpkFreeInternal(sDirNorm);
	return XPK_OK;
}

static inline int procXpkPathIsVolumeFamilyFs(xpkObject objXpk, const char* sPath, const char* sBasePath, int* pMatchRet)
{
	char* sPathNorm;
	char* sBaseNorm;
	size_t iBaseLen;
	const char* sSuffix;

	if ( pMatchRet != NULL ) {
		*pMatchRet = FALSE;
	}
	if ( sPath == NULL || sBasePath == NULL || pMatchRet == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	sPathNorm = procXpkPathNormDupFs(sPath);
	if ( sPathNorm == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}
	sBaseNorm = procXpkPathNormDupFs(sBasePath);
	if ( sBaseNorm == NULL ) {
		xpkFreeInternal(sPathNorm);
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	iBaseLen = strlen(sBaseNorm);
	if ( strncmp(sPathNorm, sBaseNorm, iBaseLen) == 0 ) {
		sSuffix = sPathNorm + iBaseLen;
		if ( *sSuffix == '.' ) {
			sSuffix++;
			if ( *sSuffix != '\0' ) {
				*pMatchRet = TRUE;
				for ( ; *sSuffix != '\0'; sSuffix++ ) {
					if ( *sSuffix < '0' || *sSuffix > '9' ) {
						*pMatchRet = FALSE;
						break;
					}
				}
			}
		}
	}

	xpkFreeInternal(sPathNorm);
	xpkFreeInternal(sBaseNorm);
	return XPK_OK;
}

static inline int procXpkBuildCopyMeta(xpkObject objDst, xpkObject objSrc)
{
	void* pMetaDup;

	if ( objSrc->pPackageMeta == NULL || objSrc->iPackageMetaSize == 0 ) {
		return XPK_OK;
	}

	pMetaDup = xpkAllocInternal(objSrc->iPackageMetaSize);
	if ( pMetaDup == NULL ) {
		return procXpkSetError(objDst, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	memcpy(pMetaDup, objSrc->pPackageMeta, objSrc->iPackageMetaSize);
	objDst->pPackageMeta = pMetaDup;
	objDst->iPackageMetaSize = objSrc->iPackageMetaSize;
	return XPK_OK;
}

static inline int procXpkBuildCopyConfig(xpkObject objDst, xpkObject objSrc)
{
	int iRet;

	iRet = procXpkApplyPackType(objDst, (xpkPackType)objSrc->objHead.packType);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	objDst->objHead.defComp = objSrc->objHead.defComp;
	objDst->objHead.metaComp = objSrc->objHead.metaComp;
	objDst->objHead.infoComp = objSrc->objHead.infoComp;
	objDst->objHead.infoExtSize = objSrc->objHead.infoExtSize;
	objDst->objHead.volumeMode = objSrc->objHead.volumeMode;
	objDst->objHead.volumeSize = objSrc->objHead.volumeSize;
	objDst->objHead.solidMode = objSrc->objHead.solidMode;
	objDst->objHead.createTime = objSrc->objHead.createTime;
	return procXpkBuildCopyMeta(objDst, objSrc);
}

static inline char* procXpkBuildDupEntryPath(xpkObject objDst, const char* sPath)
{
	char* sPathDup;
	int iErr;

	if ( sPath == NULL ) {
		return NULL;
	}
	if ( (objDst->objHead.packType != XPK_PACK_LINUX) && (objDst->objHead.packType != XPK_PACK_WIN32) ) {
		sPathDup = procXpkDupText(sPath);
		if ( sPathDup == NULL ) {
			procXpkSetError(objDst, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
		return sPathDup;
	}

	sPathDup = procXpkDupPathStoredText(objDst, sPath);
	if ( sPathDup != NULL ) {
		return sPathDup;
	}

	iErr = xpkLastError(objDst);
	if ( iErr == XPK_ERR_MEMORY ) {
		return NULL;
	}
	procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
	return NULL;
}

static inline int procXpkBuildInitSeed(xpkObject objDst, const xpkEntry* pEntrySrc, xpkEntry* pSeedRet)
{
	memset(pSeedRet, 0, sizeof(*pSeedRet));
	pSeedRet->iFileIndex = pEntrySrc->iFileIndex;
	pSeedRet->iPlatformAttr = pEntrySrc->iPlatformAttr;
	pSeedRet->tCreateTime = pEntrySrc->tCreateTime;
	pSeedRet->tModifyTime = pEntrySrc->tModifyTime;
	pSeedRet->tAccessTime = pEntrySrc->tAccessTime;
	if ( pEntrySrc->pInfoExt != NULL ) {
		pSeedRet->pInfoExt = procXpkDupInfoExt(objDst, pEntrySrc->pInfoExt);
		if ( pSeedRet->pInfoExt == NULL ) {
			return procXpkSetError(objDst, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
	}
	if ( pEntrySrc->sPath != NULL ) {
		pSeedRet->sPath = procXpkBuildDupEntryPath(objDst, pEntrySrc->sPath);
		if ( pSeedRet->sPath == NULL ) {
			if ( pSeedRet->pInfoExt != NULL ) {
				xpkFreeInternal(pSeedRet->pInfoExt);
				pSeedRet->pInfoExt = NULL;
			}
			return xpkLastError(objDst);
		}
	}

	return XPK_OK;
}

static inline int procXpkBuildCopyEntryData(xpkObject objDst, const xpkEntry* pEntrySrc, const void* pData, uint64_t iSize)
{
	xpkEntry objSeed;
	xpkEntry* pEntryDst;
	xpkWriteOptions objOpt;
	uint32_t iPosNew;
	int iRet;

	if ( pData == NULL && iSize > 0 ) {
		return procXpkSetError(objDst, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	iRet = procXpkBuildInitSeed(objDst, pEntrySrc, &objSeed);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.compLevel = (uint8_t)(pEntrySrc->iFlag & XPK_FLAG_COMP_MASK);
	objOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	objOpt.fileType = (uint8_t)((pEntrySrc->iFlag & XPK_FLAG_TYPE_MASK) >> 4);

	iRet = procXpkAddDataEntry(objDst, &objSeed, pData, iSize, &objOpt, &iPosNew);
	procXpkFreeEntryOwned(&objSeed);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	pEntryDst = procXpkGetEntryByPos(objDst, iPosNew);
	if ( pEntryDst == NULL ) {
		return procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
	}

	pEntryDst->iFlag = (pEntryDst->iFlag & (XPK_FLAG_COMP_MASK | XPK_FLAG_TYPE_MASK)) |
		(pEntrySrc->iFlag & ~(XPK_FLAG_COMP_MASK | XPK_FLAG_TYPE_MASK | XPK_FLAG_DELETED_MASK));
	pEntryDst->iPlatformAttr = pEntrySrc->iPlatformAttr;
	pEntryDst->tCreateTime = pEntrySrc->tCreateTime;
	pEntryDst->tModifyTime = pEntrySrc->tModifyTime;
	pEntryDst->tAccessTime = pEntrySrc->tAccessTime;
	return XPK_OK;
}

static inline int procXpkBuildCopyEntry(xpkObject objDst, xpkObject objSrc, const xpkEntry* pEntrySrc)
{
	void* pData;
	uint64_t iSize;
	int iRet;

	pData = procXpkReadEntryData(objSrc, (xpkEntry*)pEntrySrc, &iSize);
	if ( pData == NULL ) {
		return procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
	}

	iRet = procXpkBuildCopyEntryData(objDst, pEntrySrc, pData, iSize);
	xpkFree(pData);
	return iRet;
}

static inline int procXpkBuildAppendEntryOnly(xpkObject objDst, const xpkEntry* pEntrySrc, uint64_t iDataOffset, uint64_t iDataSize, uint64_t iFileSize)
{
	xpkEntry objEntry;
	xpkEntry* pEntryDst;
	int iRet;

	memset(&objEntry, 0, sizeof(objEntry));
	objEntry.iFlag = pEntrySrc->iFlag;
	objEntry.iFileHash = pEntrySrc->iFileHash;
	objEntry.iDataOffset = iDataOffset;
	objEntry.iDataSize = iDataSize;
	objEntry.iFileSize = iFileSize;
	objEntry.bStored = TRUE;
	objEntry.iFileIndex = pEntrySrc->iFileIndex;
	objEntry.iPlatformAttr = pEntrySrc->iPlatformAttr;
	objEntry.tCreateTime = pEntrySrc->tCreateTime;
	objEntry.tModifyTime = pEntrySrc->tModifyTime;
	objEntry.tAccessTime = pEntrySrc->tAccessTime;
	if ( pEntrySrc->pInfoExt != NULL ) {
		objEntry.pInfoExt = procXpkDupInfoExt(objDst, pEntrySrc->pInfoExt);
		if ( objEntry.pInfoExt == NULL ) {
			return procXpkSetError(objDst, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
	}
	if ( pEntrySrc->sPath != NULL ) {
		objEntry.sPath = procXpkBuildDupEntryPath(objDst, pEntrySrc->sPath);
		if ( objEntry.sPath == NULL ) {
			if ( objEntry.pInfoExt != NULL ) {
				xpkFreeInternal(objEntry.pInfoExt);
			}
			return xpkLastError(objDst);
		}
	}

	objEntry.iFlag = (objEntry.iFlag & ~XPK_FLAG_COMP_MASK) | procXpkSolidTargetCompLevel(objDst);
	iRet = procXpkAppendEntryOwned(objDst, &objEntry);
	procXpkFreeEntryOwned(&objEntry);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	pEntryDst = procXpkGetEntryByPos(objDst, objDst->iEntryCount);
	if ( pEntryDst == NULL ) {
		return procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
	}
	pEntryDst->bStored = TRUE;
	return XPK_OK;
}

static inline int procXpkBuildCopyLiveEntriesSolid(xpkObject objDst, xpkObject objSrc)
{
	uint32_t iPos;
	xpkEntry* pEntry;
	xpkEntry* pEntryDst;
	const uint8_t* pDataSrc;
	void* pData;
	void* pSolidRaw;
	void* pSolidComp;
	void* pSolidSrc;
	uint8_t* pSolidCur;
	uint64_t iDataSize;
	uint64_t iSolidRawSize64;
	uint64_t iOffsetSolid;
	uint64_t iSolidSrcSize;
	uint32_t iSolidCompSize;
	uint8_t iSolidLevel;
	xfile hFile;
	int iRet;

	pSolidRaw = NULL;
	pSolidComp = NULL;
	pSolidSrc = NULL;
	iSolidSrcSize = 0;
	iSolidRawSize64 = 0;
	for ( iPos = 1; iPos <= objSrc->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objSrc->arrEntry, iPos);
		if ( pEntry == NULL ) {
			return procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}
		iRet = procXpkValidateLiveEntryLookup(objSrc, pEntry);
		if ( iRet != XPK_OK ) {
			return procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
		}

		iSolidRawSize64 += pEntry->iFileSize;
	}
	if ( iSolidRawSize64 > UINT32_MAX ) {
		return procXpkSetError(objDst, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge);
	}

	if ( iSolidRawSize64 > 0 ) {
		pSolidRaw = xpkAllocInternal((size_t)iSolidRawSize64);
		if ( pSolidRaw == NULL ) {
			return procXpkSetError(objDst, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
	}
	if ( objSrc->bSolidApplied && (iSolidRawSize64 > 0) ) {
		iRet = procXpkReadSolidStream(objSrc, &pSolidSrc, &iSolidSrcSize);
		if ( iRet != XPK_OK ) {
			if ( pSolidRaw != NULL ) {
				xpkFreeInternal(pSolidRaw);
			}
			return procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
		}
	}

	pSolidCur = (uint8_t*)pSolidRaw;
	iOffsetSolid = 0;
	for ( iPos = 1; iPos <= objSrc->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objSrc->arrEntry, iPos);
		if ( pEntry == NULL ) {
			if ( pSolidRaw != NULL ) {
				xpkFreeInternal(pSolidRaw);
			}
			return procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}

		if ( pSolidSrc != NULL ) {
			if ( (pEntry->iDataOffset + pEntry->iFileSize) > iSolidSrcSize ) {
				xpkFreeInternal(pSolidSrc);
				if ( pSolidRaw != NULL ) {
					xpkFreeInternal(pSolidRaw);
				}
				return procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
			}
			pDataSrc = (const uint8_t*)pSolidSrc + pEntry->iDataOffset;
			iDataSize = pEntry->iFileSize;
		} else {
			pData = procXpkReadEntryData(objSrc, pEntry, &iDataSize);
			if ( pData == NULL ) {
				if ( pSolidRaw != NULL ) {
					xpkFreeInternal(pSolidRaw);
				}
				return procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
			}
			pDataSrc = (const uint8_t*)pData;
		}
		if ( iDataSize != pEntry->iFileSize ) {
			if ( pSolidSrc == NULL ) {
				xpkFree(pData);
			}
			if ( pSolidRaw != NULL ) {
				xpkFreeInternal(pSolidRaw);
			}
			if ( pSolidSrc != NULL ) {
				xpkFreeInternal(pSolidSrc);
			}
			return procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}

		if ( iDataSize > 0 ) {
			memcpy(pSolidCur, pDataSrc, (size_t)iDataSize);
			pSolidCur += iDataSize;
		}
		if ( pSolidSrc == NULL ) {
			xpkFree(pData);
		}

		iRet = procXpkBuildAppendEntryOnly(objDst, pEntry, iOffsetSolid, pEntry->iFileSize, pEntry->iFileSize);
		if ( iRet != XPK_OK ) {
			if ( pSolidSrc != NULL ) {
				xpkFreeInternal(pSolidSrc);
			}
			if ( pSolidRaw != NULL ) {
				xpkFreeInternal(pSolidRaw);
			}
			return iRet;
		}

		iOffsetSolid += pEntry->iFileSize;
	}

	iRet = procXpkRebuildLookup(objDst);
	if ( iRet != XPK_OK ) {
		if ( pSolidSrc != NULL ) {
			xpkFreeInternal(pSolidSrc);
		}
		if ( pSolidRaw != NULL ) {
			xpkFreeInternal(pSolidRaw);
		}
		return iRet;
	}
	if ( pSolidSrc != NULL ) {
		xpkFreeInternal(pSolidSrc);
	}

	iSolidLevel = procXpkSolidTargetCompLevel(objDst);
	iRet = procXpkCodecEncode(objDst, iSolidLevel, pSolidRaw, (uint32_t)iSolidRawSize64, &pSolidComp, &iSolidCompSize, &iSolidLevel);
	if ( pSolidRaw != NULL ) {
		xpkFreeInternal(pSolidRaw);
	}
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	hFile = xrtOpen(objDst->sPathPackage, FALSE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		if ( pSolidComp != NULL ) {
			xpkFreeInternal(pSolidComp);
		}
		return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	iRet = procXpkWriteAt(objDst, hFile, XPK_HEAD_SIZE, pSolidComp, iSolidCompSize);
	xrtClose(hFile);
	if ( pSolidComp != NULL ) {
		xpkFreeInternal(pSolidComp);
	}
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	objDst->objHead.defComp = iSolidLevel;
	for ( iPos = 1; iPos <= objDst->iEntryCount; iPos++ ) {
		pEntryDst = (xpkEntry*)xrtArrayGet(&objDst->arrEntry, iPos);
		if ( pEntryDst == NULL ) {
			return procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
		}
		pEntryDst->iFlag = (pEntryDst->iFlag & ~XPK_FLAG_COMP_MASK) | iSolidLevel;
	}
	objDst->bSolidApplied = TRUE;
	objDst->iAppendPos = XPK_HEAD_SIZE + iSolidCompSize;
	objDst->iFileSize = objDst->iAppendPos;
	objDst->bDirtyData = TRUE;
	objDst->bDirtyEntryTable = TRUE;
	objDst->bDirtyHead = TRUE;
	return XPK_OK;
}

static inline int procXpkBuildCopyLiveEntriesFromSolidSource(xpkObject objDst, xpkObject objSrc)
{
	uint32_t iPos;
	uint32_t iLiveCount;
	xpkEntry* pEntry;
	void* pSolidSrc;
	uint64_t iSolidSrcSize;
	int iRet;

	iRet = procXpkVisibleEntryCountStrict(objSrc, &iLiveCount);
	if ( iRet != XPK_OK ) {
		return procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
	}
	if ( iLiveCount == 0 ) {
		return XPK_OK;
	}

	pSolidSrc = NULL;
	iSolidSrcSize = 0;
	iRet = procXpkReadSolidStream(objSrc, &pSolidSrc, &iSolidSrcSize);
	if ( iRet != XPK_OK ) {
		return procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
	}

	for ( iPos = 1; iPos <= objSrc->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objSrc->arrEntry, iPos);
		if ( pEntry == NULL ) {
			xpkFreeInternal(pSolidSrc);
			return procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}
		iRet = procXpkValidateLiveEntryLookup(objSrc, pEntry);
		if ( iRet != XPK_OK ) {
			xpkFreeInternal(pSolidSrc);
			return procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
		}
		if ( (pEntry->iDataOffset + pEntry->iFileSize) > iSolidSrcSize ) {
			xpkFreeInternal(pSolidSrc);
			return procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}

		iRet = procXpkBuildCopyEntryData(objDst, pEntry, (const uint8_t*)pSolidSrc + pEntry->iDataOffset, pEntry->iFileSize);
		if ( iRet != XPK_OK ) {
			xpkFreeInternal(pSolidSrc);
			return iRet;
		}
	}

	xpkFreeInternal(pSolidSrc);
	return XPK_OK;
}

static inline int procXpkBuildCopyLiveEntries(xpkObject objDst, xpkObject objSrc)
{
	uint32_t iPos;
	xpkEntry* pEntry;
	int iRet;

	if ( objDst->objHead.solidMode ) {
		return procXpkBuildCopyLiveEntriesSolid(objDst, objSrc);
	}
	if ( objSrc->bSolidApplied ) {
		return procXpkBuildCopyLiveEntriesFromSolidSource(objDst, objSrc);
	}

	for ( iPos = 1; iPos <= objSrc->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objSrc->arrEntry, iPos);
		if ( pEntry == NULL ) {
			return procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}
		iRet = procXpkValidateLiveEntryLookup(objSrc, pEntry);
		if ( iRet != XPK_OK ) {
			return procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
		}

		iRet = procXpkBuildCopyEntry(objDst, objSrc, pEntry);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
	}

	return XPK_OK;
}

static inline char* procXpkBuildMakeTempPath(xpkObject objXpk, const xpkBuildOptions* pOpt, int bReplaceOriginal)
{
	size_t iPathLen;
	char* sTempPath;

	if ( (pOpt != NULL) && (pOpt->tempPath != NULL) && (pOpt->tempPath[0] != '\0') ) {
		if ( procXpkPathTextEqualFs(pOpt->tempPath, objXpk->sPathPackage) ) {
			procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
			return NULL;
		}
		sTempPath = procXpkDupText(pOpt->tempPath);
		if ( sTempPath == NULL ) {
			procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
			return NULL;
		}
		return sTempPath;
	}
	if ( !bReplaceOriginal ) {
		procXpkSetError(objXpk, XPK_ERR_PARAM, "tempPath is required when replaceOriginal is disabled");
		return NULL;
	}

	iPathLen = strlen(objXpk->sPathPackage);
	sTempPath = (char*)xpkAllocInternal(iPathLen + 11);
	if ( sTempPath == NULL ) {
		procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		return NULL;
	}

	memcpy(sTempPath, objXpk->sPathPackage, iPathLen);
	memcpy(sTempPath + iPathLen, ".build.tmp", 11);
	return sTempPath;
}

static inline int procXpkBuildEnsureTempPathUnused(xpkObject objXpk, const char* sTempPath)
{
	if ( sTempPath == NULL || sTempPath[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	return procXpkEnsureVolumePathUnusedText(objXpk, sTempPath, sXpkErrorTempPathExists);
}

static inline int procXpkBuildValidateOptions(xpkObject objXpk, const xpkBuildOptions* pOpt, int* pReplaceOriginalRet)
{
	int bReplaceOriginal;
	int iRet;
	int bPathInBackupDir;
	int bPathInBackupVolume;
	char* sBackupPath;
	size_t iBackupLen;
	unsigned char iFollow;

	bReplaceOriginal = TRUE;
	if ( pOpt != NULL ) {
		bReplaceOriginal = pOpt->replaceOriginal ? TRUE : FALSE;
	}

	if ( (pOpt != NULL) && (pOpt->tempPath != NULL) && (pOpt->tempPath[0] != '\0') ) {
		if ( procXpkPathTextEqualFs(pOpt->tempPath, objXpk->sPathPackage) ) {
			return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
		}
		if ( bReplaceOriginal ) {
			sBackupPath = procXpkPathSuffixDupText(objXpk->sPathPackage, ".replace.bak");
			if ( sBackupPath == NULL ) {
				return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
			}
			if ( procXpkPathTextEqualFs(pOpt->tempPath, sBackupPath) ) {
				xpkFreeInternal(sBackupPath);
				return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
			}
			iBackupLen = strlen(sBackupPath);
			if ( strncmp(pOpt->tempPath, sBackupPath, iBackupLen) == 0 ) {
				iFollow = (unsigned char)pOpt->tempPath[iBackupLen];
				if ( iFollow == '/' || iFollow == '\\' ) {
					xpkFreeInternal(sBackupPath);
					return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
				}
			}
			bPathInBackupDir = FALSE;
			iRet = procXpkPathInDirFs(objXpk, pOpt->tempPath, sBackupPath, &bPathInBackupDir);
			if ( iRet != XPK_OK ) {
				xpkFreeInternal(sBackupPath);
				return iRet;
			}
			if ( bPathInBackupDir ) {
				xpkFreeInternal(sBackupPath);
				return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
			}
			bPathInBackupVolume = FALSE;
			iRet = procXpkPathIsVolumeFamilyFs(objXpk, pOpt->tempPath, sBackupPath, &bPathInBackupVolume);
			if ( iRet != XPK_OK ) {
				xpkFreeInternal(sBackupPath);
				return iRet;
			}
			if ( bPathInBackupVolume ) {
				xpkFreeInternal(sBackupPath);
				return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
			}
			xpkFreeInternal(sBackupPath);
		}
		iRet = procXpkBuildEnsureTempPathUnused(objXpk, pOpt->tempPath);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
	} else if ( !bReplaceOriginal ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, "tempPath is required when replaceOriginal is disabled");
	}

	if ( pReplaceOriginalRet != NULL ) {
		*pReplaceOriginalRet = bReplaceOriginal;
	}
	return XPK_OK;
}

static inline int procXpkBuildResetTempPath(xpkObject objXpk, const char* sTempPath)
{
	int iRet;

	if ( sTempPath == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	if ( xrtFileExists((str)sTempPath) && !xrtFileDelete((str)sTempPath) ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
	}

	iRet = procXpkScanLooseVolumeFilesText(objXpk, sTempPath, TRUE, NULL);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	return XPK_OK;
}

static inline int procXpkBuildReloadSelf(xpkObject objXpk)
{
	xpkOpenOptions objOpt;
	xpkObject objReload;
	int iRet;

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.readonly = objXpk->bReadonly;
	objOpt.bufferedDefault = objXpk->bBufferedDefault;

	objReload = xpkOpen(objXpk->sPathPackage, &objOpt);
	if ( objReload == NULL ) {
		iRet = xpkLastError(NULL);
		if ( iRet == XPK_OK ) {
			iRet = XPK_ERR_IO;
		}
		return procXpkSetError(objXpk, iRet, xpkLastErrorMessage(NULL));
	}

	procXpkUnitWriteQueue(objXpk);
	procXpkUnitObject(objXpk);
	*objXpk = *objReload;
	xpkFreeInternal(objReload);
	return XPK_OK;
}

static inline int procXpkBuildPackage(xpkObject objXpk, const xpkBuildOptions* pOpt)
{
	int bReplaceOriginal;
	char* sTempPath;
	xpkObject objBuild;
	xpkOpenOptions objOpenOpt;
	char sBuildError[XPK_ERROR_TEXT_CAP];
	char sReplaceError[XPK_ERROR_TEXT_CAP];
	const char* sBuildText;
	size_t iReplaceTextSize;
	size_t iBuildTextSize;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	iRet = procXpkBuildValidateOptions(objXpk, pOpt, &bReplaceOriginal);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( procXpkValidateEntryCountState(objXpk) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( objXpk->bReadonly ) {
		return procXpkSetError(objXpk, XPK_ERR_READONLY, sXpkErrorReadonly);
	}

	sTempPath = procXpkBuildMakeTempPath(objXpk, pOpt, bReplaceOriginal);
	if ( sTempPath == NULL ) {
		return xpkLastError(objXpk);
	}
	iRet = procXpkBuildEnsureTempPathUnused(objXpk, sTempPath);
	if ( iRet != XPK_OK ) {
		xpkFreeInternal(sTempPath);
		return iRet;
	}

	iRet = procXpkBuildResetTempPath(objXpk, sTempPath);
	if ( iRet != XPK_OK ) {
		xpkFreeInternal(sTempPath);
		return iRet;
	}

	memset(&objOpenOpt, 0, sizeof(objOpenOpt));
	objOpenOpt.createIfMissing = TRUE;
	objBuild = xpkOpen(sTempPath, &objOpenOpt);
	if ( objBuild == NULL ) {
		sBuildText = xpkLastErrorMessage(NULL);
		iBuildTextSize = strlen(sBuildText);
		if ( iBuildTextSize >= XPK_ERROR_TEXT_CAP ) {
			iBuildTextSize = XPK_ERROR_TEXT_CAP - 1;
		}
		memcpy(sBuildError, sBuildText, iBuildTextSize);
		sBuildError[iBuildTextSize] = '\0';
		iRet = xpkLastError(NULL);
		if ( iRet == XPK_OK ) {
			iRet = XPK_ERR_IO;
			memcpy(sBuildError, sXpkErrorIoOpen, sizeof(sXpkErrorIoOpen));
		}
		xpkFreeInternal(sTempPath);
		return procXpkSetError(objXpk, iRet, sBuildError);
	}
	sBuildError[0] = '\0';

	iRet = procXpkBuildCopyConfig(objBuild, objXpk);
	if ( iRet == XPK_OK ) {
		iRet = procXpkBuildCopyLiveEntries(objBuild, objXpk);
	}
	if ( iRet == XPK_OK ) {
		iRet = procXpkSavePackage(objBuild);
	}
	if ( iRet != XPK_OK ) {
		sBuildText = xpkLastErrorMessage(objBuild);
		iBuildTextSize = strlen(sBuildText);
		if ( iBuildTextSize >= XPK_ERROR_TEXT_CAP ) {
			iBuildTextSize = XPK_ERROR_TEXT_CAP - 1;
		}
		memcpy(sBuildError, sBuildText, iBuildTextSize);
		sBuildError[iBuildTextSize] = '\0';
	}

	xpkClose(objBuild);
	if ( iRet != XPK_OK ) {
		(void)procXpkBuildResetTempPath(objXpk, sTempPath);
		xpkFreeInternal(sTempPath);
		return procXpkSetError(objXpk, iRet, sBuildError);
	}

	if ( !bReplaceOriginal ) {
		xpkFreeInternal(sTempPath);
		procXpkClearError(objXpk);
		return XPK_OK;
	}

	if ( procXpkMoveVolumeFilesText(objXpk, sTempPath, objXpk->sPathPackage) != XPK_OK ) {
		sBuildText = xpkLastErrorMessage(objXpk);
		iReplaceTextSize = strlen(sBuildText);
		if ( iReplaceTextSize >= XPK_ERROR_TEXT_CAP ) {
			iReplaceTextSize = XPK_ERROR_TEXT_CAP - 1;
		}
		memcpy(sReplaceError, sBuildText, iReplaceTextSize);
		sReplaceError[iReplaceTextSize] = '\0';
		iRet = xpkLastError(objXpk);
		(void)procXpkBuildResetTempPath(objXpk, sTempPath);
		xpkFreeInternal(sTempPath);
		return procXpkSetError(objXpk, iRet, sReplaceError);
	}

	xpkFreeInternal(sTempPath);
	iRet = procXpkBuildReloadSelf(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	procXpkClearError(objXpk);
	return XPK_OK;
}

#endif
