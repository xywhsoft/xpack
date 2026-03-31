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
			if ( bSegStartA && sPathA[iPosA] == '.' && sPathA[iPosA + 1] == '/' ) {
				iPosA += 2;
				while ( sPathA[iPosA] == '/' ) {
					iPosA++;
				}
				bSegStartA = TRUE;
				continue;
			}
			if ( sPathA[iPosA] == '/' ) {
				while ( sPathA[iPosA] == '/' ) {
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
			iChA = (unsigned char)sPathA[iPosA];
			if ( iChA != '\0' ) {
				iPosA++;
			}
			bSegStartA = FALSE;
			break;
		}

		for ( ;; ) {
			if ( bSegStartB && sPathB[iPosB] == '.' && sPathB[iPosB + 1] == '/' ) {
				iPosB += 2;
				while ( sPathB[iPosB] == '/' ) {
					iPosB++;
				}
				bSegStartB = TRUE;
				continue;
			}
			if ( sPathB[iPosB] == '/' ) {
				while ( sPathB[iPosB] == '/' ) {
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
			iChB = (unsigned char)sPathB[iPosB];
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
	size_t iPosIn;
	size_t iPosOut;
	int bSegStart;

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
			if ( bSegStart && sPath[iPosIn] == '.' && sPath[iPosIn + 1] == '/' ) {
				iPosIn += 2;
				while ( sPath[iPosIn] == '/' ) {
					iPosIn++;
				}
				bSegStart = TRUE;
				continue;
			}
			if ( sPath[iPosIn] == '/' ) {
				while ( sPath[iPosIn] == '/' ) {
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
			if ( sPath[iPosIn] == '\0' ) {
				sPathRet[iPosOut] = '\0';
				return sPathRet;
			}
			sPathRet[iPosOut++] = sPath[iPosIn++];
			bSegStart = FALSE;
			break;
		}
	}
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
	objDst->bVolumeApplied = objDst->objHead.volumeMode ? TRUE : FALSE;
	objDst->iVolumeSizeApplied = objDst->objHead.volumeSize;
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

static inline int procXpkBuildWriteAtChunked(xpkObject objDst, xfile hFile, uint64_t iOffset, const void* pData, uint64_t iSize)
{
	const uint8_t* pCur;
	uint64_t iSizeLeft;
	uint32_t iChunkSize;
	int iRet;

	if ( objDst == NULL ) {
		return procXpkReturnParamError(objDst);
	}
	if ( pData == NULL && iSize > 0 ) {
		return procXpkSetError(objDst, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	pCur = (const uint8_t*)pData;
	iSizeLeft = iSize;
	while ( iSizeLeft > 0 ) {
		iChunkSize = (iSizeLeft > XPK_WRITE_FILE_CHUNK_SIZE) ? XPK_WRITE_FILE_CHUNK_SIZE : (uint32_t)iSizeLeft;
		iRet = procXpkWriteAt(objDst, hFile, iOffset, pCur, iChunkSize);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		pCur += iChunkSize;
		iOffset += iChunkSize;
		iSizeLeft -= iChunkSize;
	}

	return XPK_OK;
}

static inline int procXpkBuildCleanupChunkFiles(void* pChunk, xfile hFileSrc, xfile hFileDst, int iRet)
{
	if ( pChunk != NULL ) {
		xpkFreeInternal(pChunk);
	}
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}
	if ( hFileDst != NULL ) {
		xrtClose(hFileDst);
	}
	return iRet;
}

static inline int procXpkBuildCopyStoredPayload(xpkObject objDst, xfile hFileDst, xpkObject objSrc, xfile hFileSrc, uint64_t iOffsetDst, uint64_t iOffsetSrc, uint64_t iSize, void* pChunkShared)
{
	void* pChunk;
	int bFreeChunk;
	uint64_t iSizeLeft;
	uint32_t iChunkSize;
	int iRet;

	pChunk = pChunkShared;
	bFreeChunk = FALSE;
	if ( pChunk == NULL ) {
		pChunk = xpkAllocInternal(XPK_WRITE_FILE_CHUNK_SIZE);
		if ( pChunk == NULL ) {
			return procXpkSetError(objDst, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
		bFreeChunk = TRUE;
	}

	iSizeLeft = iSize;
	while ( iSizeLeft > 0 ) {
		iChunkSize = (iSizeLeft > XPK_WRITE_FILE_CHUNK_SIZE) ? XPK_WRITE_FILE_CHUNK_SIZE : (uint32_t)iSizeLeft;
		iRet = procXpkReadAtBuffer(objSrc, hFileSrc, iOffsetSrc, pChunk, iChunkSize);
		if ( iRet != XPK_OK ) {
			if ( bFreeChunk ) {
				xpkFreeInternal(pChunk);
			}
			return iRet;
		}

		iRet = procXpkWriteAt(objDst, hFileDst, iOffsetDst, pChunk, iChunkSize);
		if ( iRet != XPK_OK ) {
			if ( bFreeChunk ) {
				xpkFreeInternal(pChunk);
			}
			return iRet;
		}

		iOffsetDst += iChunkSize;
		iOffsetSrc += iChunkSize;
		iSizeLeft -= iChunkSize;
	}

	if ( bFreeChunk ) {
		xpkFreeInternal(pChunk);
	}
	return XPK_OK;
}

static inline int procXpkBuildWriteSolidZstdFromRaw(xpkObject objDst, const void* pData, uint64_t iRawSize, uint8_t iLevel, uint32_t* pCompSizeRet, uint8_t* pLevelRet)
{
	xfile hFileTmp;
	xfile hFileDst;
	ZSTD_CCtx* pCtx;
	ZSTD_inBuffer objIn;
	ZSTD_outBuffer objOut;
	char* sPathTmp;
	void* pOutBuf;
	const uint8_t* pCur;
	uint64_t iSizeLeft;
	uint64_t iCompSize64;
	uint32_t iChunkSize;
	uint32_t iCompSize;
	size_t iOutCap;
	size_t iZstdRet;
	int bFallbackStore;
	int iRet;

	if ( pCompSizeRet != NULL ) {
		*pCompSizeRet = 0;
	}
	if ( pLevelRet != NULL ) {
		*pLevelRet = iLevel;
	}
	if ( objDst == NULL ) {
		return procXpkReturnParamError(objDst);
	}
	if ( pData == NULL && iRawSize > 0 ) {
		return procXpkSetError(objDst, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( procXpkCompLevelToAlg(iLevel) != XPK_ALG_ZSTD ) {
		return procXpkSetError(objDst, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( iRawSize == 0 ) {
		hFileDst = NULL;
		if ( !procXpkAppliedVolumeMode(objDst) ) {
			hFileDst = xrtOpen(objDst->sPathPackage, FALSE, XRT_CP_BINARY);
			if ( hFileDst == NULL ) {
				return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
			}
		}
		iRet = procXpkWriteAt(objDst, hFileDst, XPK_HEAD_SIZE, NULL, 0);
		if ( hFileDst != NULL ) {
			xrtClose(hFileDst);
		}
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		if ( pLevelRet != NULL ) {
			*pLevelRet = 0;
		}
		procXpkClearError(objDst);
		return XPK_OK;
	}

	hFileTmp = NULL;
	hFileDst = NULL;
	pCtx = NULL;
	sPathTmp = NULL;
	pOutBuf = NULL;
	iCompSize64 = 0;
	bFallbackStore = FALSE;

	sPathTmp = procXpkWriteTempPathDup(objDst, ".solid.zstd");
	if ( sPathTmp == NULL ) {
		return xpkLastError(objDst);
	}

	hFileTmp = xrtOpen((str)sPathTmp, FALSE, XRT_CP_BINARY);
	if ( hFileTmp == NULL ) {
		iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
		goto lblCleanup;
	}

	iOutCap = ZSTD_CStreamOutSize();
	if ( iOutCap == 0 ) {
		iOutCap = XPK_CODEC_STREAM_CHUNK_SIZE;
	}
	pOutBuf = xpkAllocInternal(iOutCap);
	if ( pOutBuf == NULL ) {
		iRet = procXpkSetError(objDst, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		goto lblCleanup;
	}

	pCtx = ZSTD_createCCtx();
	if ( pCtx == NULL ) {
		iRet = procXpkSetError(objDst, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		goto lblCleanup;
	}

	ZSTD_CCtx_reset(pCtx, ZSTD_reset_session_only);
	ZSTD_CCtx_setParameter(pCtx, ZSTD_c_checksumFlag, 0);
	ZSTD_CCtx_setParameter(pCtx, ZSTD_c_strategy, (ZSTD_strategy)procXpkCompLevelToNative(iLevel));
	ZSTD_CCtx_setPledgedSrcSize(pCtx, (unsigned long long)iRawSize);

	pCur = (const uint8_t*)pData;
	iSizeLeft = iRawSize;
	while ( iSizeLeft > 0 ) {
		iChunkSize = (uint32_t)((iSizeLeft > XPK_CODEC_STREAM_CHUNK_SIZE) ? XPK_CODEC_STREAM_CHUNK_SIZE : iSizeLeft);
		objIn.src = pCur;
		objIn.size = iChunkSize;
		objIn.pos = 0;

		while ( objIn.pos < objIn.size ) {
			objOut.dst = pOutBuf;
			objOut.size = iOutCap;
			objOut.pos = 0;
			iZstdRet = ZSTD_compressStream2(pCtx, &objOut, &objIn, ZSTD_e_continue);
			if ( ZSTD_isError(iZstdRet) ) {
				iRet = procXpkSetError(objDst, XPK_ERR_IO, "zstd compress failed");
				goto lblCleanup;
			}
			if ( objOut.pos > 0 ) {
				if ( xrtPut(hFileTmp, (ptr)pOutBuf, objOut.pos) != objOut.pos ) {
					iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoWrite);
					goto lblCleanup;
				}
				iCompSize64 += objOut.pos;
				if ( iCompSize64 >= iRawSize ) {
					bFallbackStore = TRUE;
					break;
				}
			}
		}
		if ( bFallbackStore ) {
			break;
		}

		pCur += iChunkSize;
		iSizeLeft -= iChunkSize;
	}

	if ( !bFallbackStore ) {
		objIn.src = NULL;
		objIn.size = 0;
		objIn.pos = 0;
		for ( ;; ) {
			objOut.dst = pOutBuf;
			objOut.size = iOutCap;
			objOut.pos = 0;
			iZstdRet = ZSTD_compressStream2(pCtx, &objOut, &objIn, ZSTD_e_end);
			if ( ZSTD_isError(iZstdRet) ) {
				iRet = procXpkSetError(objDst, XPK_ERR_IO, "zstd compress failed");
				goto lblCleanup;
			}
			if ( objOut.pos > 0 ) {
				if ( xrtPut(hFileTmp, (ptr)pOutBuf, objOut.pos) != objOut.pos ) {
					iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoWrite);
					goto lblCleanup;
				}
				iCompSize64 += objOut.pos;
				if ( iCompSize64 >= iRawSize ) {
					bFallbackStore = TRUE;
					break;
				}
			}
			if ( iZstdRet == 0 ) {
				break;
			}
		}
	}

	hFileDst = NULL;
	if ( !procXpkAppliedVolumeMode(objDst) ) {
		hFileDst = xrtOpen(objDst->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFileDst == NULL ) {
			iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
			goto lblCleanup;
		}
	}

	if ( bFallbackStore ) {
		iRet = procXpkBuildWriteAtChunked(objDst, hFileDst, XPK_HEAD_SIZE, pData, iRawSize);
		if ( iRet != XPK_OK ) {
			goto lblCleanup;
		}
		if ( pCompSizeRet != NULL ) {
			*pCompSizeRet = (uint32_t)iRawSize;
		}
		if ( pLevelRet != NULL ) {
			*pLevelRet = 0;
		}
		procXpkClearError(objDst);
		iRet = XPK_OK;
		goto lblCleanup;
	}

	if ( !xrtSetEOF(hFileTmp) ) {
		iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoWrite);
		goto lblCleanup;
	}

	iCompSize = (uint32_t)iCompSize64;
	iRet = procXpkCopySourceFileToPackage(objDst, hFileTmp, hFileDst, XPK_HEAD_SIZE, iCompSize);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	if ( pCompSizeRet != NULL ) {
		*pCompSizeRet = iCompSize;
	}
	if ( pLevelRet != NULL ) {
		*pLevelRet = iLevel;
	}
	procXpkClearError(objDst);
	iRet = XPK_OK;

lblCleanup:
	if ( hFileDst != NULL ) {
		xrtClose(hFileDst);
	}
	if ( hFileTmp != NULL ) {
		xrtClose(hFileTmp);
	}
	if ( pCtx != NULL ) {
		ZSTD_freeCCtx(pCtx);
	}
	if ( pOutBuf != NULL ) {
		xpkFreeInternal(pOutBuf);
	}
	if ( sPathTmp != NULL ) {
		if ( xrtFileExists((str)sPathTmp) ) {
			(void)xrtFileDelete((str)sPathTmp);
		}
		xpkFreeInternal(sPathTmp);
	}
	return iRet;
}

static inline int procXpkBuildWritePlainFileChunked(xpkObject objDst, xfile hFile, const void* pData, uint64_t iSize)
{
	const uint8_t* pCur;
	uint64_t iSizeLeft;
	size_t iChunkSize;
	size_t iWrite;

	if ( objDst == NULL || hFile == NULL ) {
		return procXpkReturnParamError(objDst);
	}
	if ( pData == NULL && iSize > 0 ) {
		return procXpkSetError(objDst, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	pCur = (const uint8_t*)pData;
	iSizeLeft = iSize;
	while ( iSizeLeft > 0 ) {
		iChunkSize = (iSizeLeft > XPK_WRITE_FILE_CHUNK_SIZE) ? XPK_WRITE_FILE_CHUNK_SIZE : (size_t)iSizeLeft;
		iWrite = xrtPut(hFile, (ptr)pCur, iChunkSize);
		if ( iWrite != iChunkSize ) {
			return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoWrite);
		}
		pCur += iChunkSize;
		iSizeLeft -= iChunkSize;
	}

	return XPK_OK;
}

static inline SRes procXpkBuildLzmaSeqInRead(ISeqInStreamPtr pStream, void* pData, size_t* pSize)
{
	xpkBuildLzmaSeqIn* pIn;
	size_t iWant;
	size_t iRead;

	if ( pStream == NULL || pSize == NULL ) {
		return SZ_ERROR_PARAM;
	}

	pIn = (xpkBuildLzmaSeqIn*)pStream;
	iWant = *pSize;
	if ( iWant == 0 ) {
		return SZ_OK;
	}
	if ( pIn->iRemain < (uint64_t)iWant ) {
		iWant = (size_t)pIn->iRemain;
	}
	if ( iWant == 0 ) {
		*pSize = 0;
		return SZ_OK;
	}

	iRead = xrtGetBuffer(pIn->hFile, pData, iWant);
	*pSize = iRead;
	if ( iRead != iWant ) {
		return SZ_ERROR_READ;
	}

	pIn->iRemain -= iRead;
	return SZ_OK;
}

static inline size_t procXpkBuildLzmaSeqOutWrite(ISeqOutStreamPtr pStream, const void* pData, size_t iSize)
{
	xpkBuildLzmaSeqOut* pOut;
	size_t iWrite;

	if ( pStream == NULL ) {
		return 0;
	}
	if ( iSize == 0 ) {
		return 0;
	}

	pOut = (xpkBuildLzmaSeqOut*)pStream;
	iWrite = xrtPut(pOut->hFile, (ptr)pData, iSize);
	pOut->iSize += iWrite;
	return iWrite;
}

static inline int procXpkBuildWriteSolidZstdFromFile(xpkObject objDst, const char* sPathSrc, uint64_t iRawSize, uint8_t iLevel, uint32_t* pCompSizeRet, uint8_t* pLevelRet)
{
	xfile hFileSrc;
	xfile hFileTmp;
	xfile hFileDst;
	ZSTD_CCtx* pCtx;
	ZSTD_inBuffer objIn;
	ZSTD_outBuffer objOut;
	char* sPathTmp;
	void* pInBuf;
	void* pOutBuf;
	uint64_t iCompSize64;
	uint64_t iOffsetRead;
	uint32_t iChunkRead;
	uint32_t iCompSize;
	size_t iInCap;
	size_t iOutCap;
	size_t iRead;
	size_t iZstdRet;
	int bFallbackStore;
	int iRet;

	if ( pCompSizeRet != NULL ) {
		*pCompSizeRet = 0;
	}
	if ( pLevelRet != NULL ) {
		*pLevelRet = iLevel;
	}
	if ( objDst == NULL ) {
		return procXpkReturnParamError(objDst);
	}
	if ( sPathSrc == NULL || sPathSrc[0] == '\0' ) {
		return procXpkSetError(objDst, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( procXpkCompLevelToAlg(iLevel) != XPK_ALG_ZSTD ) {
		return procXpkSetError(objDst, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( iRawSize == 0 ) {
		hFileDst = NULL;
		if ( !procXpkAppliedVolumeMode(objDst) ) {
			hFileDst = xrtOpen(objDst->sPathPackage, FALSE, XRT_CP_BINARY);
			if ( hFileDst == NULL ) {
				return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
			}
		}
		iRet = procXpkWriteAt(objDst, hFileDst, XPK_HEAD_SIZE, NULL, 0);
		if ( hFileDst != NULL ) {
			xrtClose(hFileDst);
		}
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		procXpkClearError(objDst);
		return XPK_OK;
	}

	hFileSrc = NULL;
	hFileTmp = NULL;
	hFileDst = NULL;
	pCtx = NULL;
	sPathTmp = NULL;
	pInBuf = NULL;
	pOutBuf = NULL;
	iCompSize64 = 0;
	iOffsetRead = 0;
	bFallbackStore = FALSE;

	hFileSrc = xrtOpen((str)sPathSrc, TRUE, XRT_CP_BINARY);
	if ( hFileSrc == NULL ) {
		return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	sPathTmp = procXpkWriteTempPathDup(objDst, ".solid.zstd");
	if ( sPathTmp == NULL ) {
		iRet = xpkLastError(objDst);
		goto lblCleanup;
	}

	hFileTmp = xrtOpen((str)sPathTmp, FALSE, XRT_CP_BINARY);
	if ( hFileTmp == NULL ) {
		iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
		goto lblCleanup;
	}

	iInCap = ZSTD_CStreamInSize();
	if ( iInCap == 0 ) {
		iInCap = XPK_CODEC_STREAM_CHUNK_SIZE;
	}
	iOutCap = ZSTD_CStreamOutSize();
	if ( iOutCap == 0 ) {
		iOutCap = XPK_CODEC_STREAM_CHUNK_SIZE;
	}

	pInBuf = xpkAllocInternal(iInCap);
	if ( pInBuf == NULL ) {
		iRet = procXpkSetError(objDst, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		goto lblCleanup;
	}
	pOutBuf = xpkAllocInternal(iOutCap);
	if ( pOutBuf == NULL ) {
		iRet = procXpkSetError(objDst, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		goto lblCleanup;
	}

	pCtx = ZSTD_createCCtx();
	if ( pCtx == NULL ) {
		iRet = procXpkSetError(objDst, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		goto lblCleanup;
	}

	ZSTD_CCtx_reset(pCtx, ZSTD_reset_session_only);
	ZSTD_CCtx_setParameter(pCtx, ZSTD_c_checksumFlag, 0);
	ZSTD_CCtx_setParameter(pCtx, ZSTD_c_strategy, (ZSTD_strategy)procXpkCompLevelToNative(iLevel));
	ZSTD_CCtx_setPledgedSrcSize(pCtx, (unsigned long long)iRawSize);

	while ( iOffsetRead < iRawSize ) {
		iChunkRead = (uint32_t)(((iRawSize - iOffsetRead) > (uint64_t)iInCap) ? iInCap : (iRawSize - iOffsetRead));
		iRead = xrtGetBuffer(hFileSrc, pInBuf, iChunkRead);
		if ( iRead != iChunkRead ) {
			iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoRead);
			goto lblCleanup;
		}
		iOffsetRead += iChunkRead;

		objIn.src = pInBuf;
		objIn.size = iChunkRead;
		objIn.pos = 0;
		while ( objIn.pos < objIn.size ) {
			objOut.dst = pOutBuf;
			objOut.size = iOutCap;
			objOut.pos = 0;
			iZstdRet = ZSTD_compressStream2(pCtx, &objOut, &objIn, ZSTD_e_continue);
			if ( ZSTD_isError(iZstdRet) ) {
				iRet = procXpkSetError(objDst, XPK_ERR_IO, "zstd compress failed");
				goto lblCleanup;
			}
			if ( objOut.pos > 0 ) {
				if ( xrtPut(hFileTmp, (ptr)pOutBuf, objOut.pos) != objOut.pos ) {
					iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoWrite);
					goto lblCleanup;
				}
				iCompSize64 += objOut.pos;
				if ( iCompSize64 >= iRawSize ) {
					bFallbackStore = TRUE;
					break;
				}
			}
		}
		if ( bFallbackStore ) {
			break;
		}
	}

	if ( !bFallbackStore ) {
		objIn.src = NULL;
		objIn.size = 0;
		objIn.pos = 0;
		for ( ;; ) {
			objOut.dst = pOutBuf;
			objOut.size = iOutCap;
			objOut.pos = 0;
			iZstdRet = ZSTD_compressStream2(pCtx, &objOut, &objIn, ZSTD_e_end);
			if ( ZSTD_isError(iZstdRet) ) {
				iRet = procXpkSetError(objDst, XPK_ERR_IO, "zstd compress failed");
				goto lblCleanup;
			}
			if ( objOut.pos > 0 ) {
				if ( xrtPut(hFileTmp, (ptr)pOutBuf, objOut.pos) != objOut.pos ) {
					iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoWrite);
					goto lblCleanup;
				}
				iCompSize64 += objOut.pos;
				if ( iCompSize64 >= iRawSize ) {
					bFallbackStore = TRUE;
					break;
				}
			}
			if ( iZstdRet == 0 ) {
				break;
			}
		}
	}

	hFileDst = NULL;
	if ( !procXpkAppliedVolumeMode(objDst) ) {
		hFileDst = xrtOpen(objDst->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFileDst == NULL ) {
			iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
			goto lblCleanup;
		}
	}

	if ( bFallbackStore ) {
		iRet = procXpkSeekFile(objDst, hFileSrc, 0);
		if ( iRet != XPK_OK ) {
			goto lblCleanup;
		}
		iRet = procXpkCopySourceFileToPackage(objDst, hFileSrc, hFileDst, XPK_HEAD_SIZE, iRawSize);
		if ( iRet != XPK_OK ) {
			goto lblCleanup;
		}
		if ( pCompSizeRet != NULL ) {
			*pCompSizeRet = (uint32_t)iRawSize;
		}
		if ( pLevelRet != NULL ) {
			*pLevelRet = 0;
		}
		procXpkClearError(objDst);
		iRet = XPK_OK;
		goto lblCleanup;
	}

	if ( !xrtSetEOF(hFileTmp) ) {
		iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoWrite);
		goto lblCleanup;
	}

	iCompSize = (uint32_t)iCompSize64;
	iRet = procXpkCopySourceFileToPackage(objDst, hFileTmp, hFileDst, XPK_HEAD_SIZE, iCompSize);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	if ( pCompSizeRet != NULL ) {
		*pCompSizeRet = iCompSize;
	}
	if ( pLevelRet != NULL ) {
		*pLevelRet = iLevel;
	}
	procXpkClearError(objDst);
	iRet = XPK_OK;

lblCleanup:
	if ( hFileDst != NULL ) {
		xrtClose(hFileDst);
	}
	if ( hFileTmp != NULL ) {
		xrtClose(hFileTmp);
	}
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}
	if ( pCtx != NULL ) {
		ZSTD_freeCCtx(pCtx);
	}
	if ( pOutBuf != NULL ) {
		xpkFreeInternal(pOutBuf);
	}
	if ( pInBuf != NULL ) {
		xpkFreeInternal(pInBuf);
	}
	if ( sPathTmp != NULL ) {
		if ( xrtFileExists((str)sPathTmp) ) {
			(void)xrtFileDelete((str)sPathTmp);
		}
		xpkFreeInternal(sPathTmp);
	}
	return iRet;
}

static inline int procXpkBuildWriteSolidLzma2FromFile(xpkObject objDst, const char* sPathSrc, uint64_t iRawSize, uint8_t iLevel, uint32_t* pCompSizeRet, uint8_t* pLevelRet)
{
	xfile hFileSrc;
	xfile hFileTmp;
	xfile hFileDst;
	CLzma2EncHandle hLzma2;
	CLzma2EncProps objProps;
	xpkBuildLzmaSeqIn objIn;
	xpkBuildLzmaSeqOut objOut;
	char* sPathTmp;
	Byte iPropByte;
	uint64_t iCompSize64;
	uint32_t iCompSize;
	SRes iLzmaRes;
	int bFallbackStore;
	int iRet;

	if ( pCompSizeRet != NULL ) {
		*pCompSizeRet = 0;
	}
	if ( pLevelRet != NULL ) {
		*pLevelRet = iLevel;
	}
	if ( objDst == NULL ) {
		return procXpkReturnParamError(objDst);
	}
	if ( sPathSrc == NULL || sPathSrc[0] == '\0' ) {
		return procXpkSetError(objDst, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( procXpkCompLevelToAlg(iLevel) != XPK_ALG_LZMA2 ) {
		return procXpkSetError(objDst, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( iRawSize == 0 ) {
		hFileDst = NULL;
		if ( !procXpkAppliedVolumeMode(objDst) ) {
			hFileDst = xrtOpen(objDst->sPathPackage, FALSE, XRT_CP_BINARY);
			if ( hFileDst == NULL ) {
				return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
			}
		}
		iRet = procXpkWriteAt(objDst, hFileDst, XPK_HEAD_SIZE, NULL, 0);
		if ( hFileDst != NULL ) {
			xrtClose(hFileDst);
		}
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		procXpkClearError(objDst);
		return XPK_OK;
	}

	hFileSrc = NULL;
	hFileTmp = NULL;
	hFileDst = NULL;
	hLzma2 = NULL;
	sPathTmp = NULL;
	iCompSize64 = 0;
	bFallbackStore = FALSE;

	hFileSrc = xrtOpen((str)sPathSrc, TRUE, XRT_CP_BINARY);
	if ( hFileSrc == NULL ) {
		return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	sPathTmp = procXpkWriteTempPathDup(objDst, ".solid.lzma2");
	if ( sPathTmp == NULL ) {
		iRet = xpkLastError(objDst);
		goto lblCleanup;
	}

	hFileTmp = xrtOpen((str)sPathTmp, FALSE, XRT_CP_BINARY);
	if ( hFileTmp == NULL ) {
		iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
		goto lblCleanup;
	}

	hLzma2 = Lzma2Enc_Create(&g_Alloc, &g_BigAlloc);
	if ( hLzma2 == NULL ) {
		iRet = procXpkSetError(objDst, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		goto lblCleanup;
	}

	Lzma2EncProps_Init(&objProps);
	objProps.lzmaProps.level = procXpkCompLevelToNative(iLevel);
	iLzmaRes = Lzma2Enc_SetProps(hLzma2, &objProps);
	if ( iLzmaRes != SZ_OK ) {
		iRet = procXpkSetError(objDst, XPK_ERR_IO, "lzma2 set props failed");
		goto lblCleanup;
	}
	Lzma2Enc_SetDataSize(hLzma2, (UInt64)iRawSize);

	iPropByte = Lzma2Enc_WriteProperties(hLzma2);
	if ( xrtPut(hFileTmp, (ptr)&iPropByte, 1) != 1 ) {
		iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoWrite);
		goto lblCleanup;
	}

	objIn.vt.Read = procXpkBuildLzmaSeqInRead;
	objIn.hFile = hFileSrc;
	objIn.iRemain = iRawSize;
	objOut.vt.Write = procXpkBuildLzmaSeqOutWrite;
	objOut.hFile = hFileTmp;
	objOut.iSize = 0;

	iLzmaRes = Lzma2Enc_Encode2(hLzma2, &objOut.vt, NULL, NULL, &objIn.vt, NULL, 0, NULL);
	if ( iLzmaRes != SZ_OK ) {
		if ( iLzmaRes == SZ_ERROR_READ ) {
			iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoRead);
		} else if ( iLzmaRes == SZ_ERROR_WRITE ) {
			iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoWrite);
		} else {
			iRet = procXpkSetError(objDst, XPK_ERR_IO, "lzma2 compress failed");
		}
		goto lblCleanup;
	}

	iCompSize64 = 1 + objOut.iSize;
	if ( iCompSize64 >= iRawSize ) {
		bFallbackStore = TRUE;
	}

	hFileDst = NULL;
	if ( !procXpkAppliedVolumeMode(objDst) ) {
		hFileDst = xrtOpen(objDst->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFileDst == NULL ) {
			iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
			goto lblCleanup;
		}
	}

	if ( bFallbackStore ) {
		iRet = procXpkSeekFile(objDst, hFileSrc, 0);
		if ( iRet != XPK_OK ) {
			goto lblCleanup;
		}
		iRet = procXpkCopySourceFileToPackage(objDst, hFileSrc, hFileDst, XPK_HEAD_SIZE, iRawSize);
		if ( iRet != XPK_OK ) {
			goto lblCleanup;
		}
		if ( pCompSizeRet != NULL ) {
			*pCompSizeRet = (uint32_t)iRawSize;
		}
		if ( pLevelRet != NULL ) {
			*pLevelRet = 0;
		}
		procXpkClearError(objDst);
		iRet = XPK_OK;
		goto lblCleanup;
	}

	if ( !xrtSetEOF(hFileTmp) ) {
		iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoWrite);
		goto lblCleanup;
	}

	iCompSize = (uint32_t)iCompSize64;
	iRet = procXpkCopySourceFileToPackage(objDst, hFileTmp, hFileDst, XPK_HEAD_SIZE, iCompSize);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	if ( pCompSizeRet != NULL ) {
		*pCompSizeRet = iCompSize;
	}
	if ( pLevelRet != NULL ) {
		*pLevelRet = iLevel;
	}
	procXpkClearError(objDst);
	iRet = XPK_OK;

lblCleanup:
	if ( hFileDst != NULL ) {
		xrtClose(hFileDst);
	}
	if ( hFileTmp != NULL ) {
		xrtClose(hFileTmp);
	}
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}
	if ( hLzma2 != NULL ) {
		Lzma2Enc_Destroy(hLzma2);
	}
	if ( sPathTmp != NULL ) {
		if ( xrtFileExists((str)sPathTmp) ) {
			(void)xrtFileDelete((str)sPathTmp);
		}
		xpkFreeInternal(sPathTmp);
	}
	return iRet;
}

static inline int procXpkBuildWriteSolidLz4FromFile(xpkObject objDst, const char* sPathSrc, uint64_t iRawSize, uint8_t iLevel, uint32_t* pCompSizeRet, uint8_t* pLevelRet)
{
	xfile hFileSrc;
	xfile hFileTmp;
	xfile hFileDst;
	xpkMappedFile objMapSrc;
	xpkMappedFile objMapTmp;
	char* sPathTmp;
	uint32_t iBound;
	uint32_t iCompSize;
	uint32_t iAlg;
	int iRetEnc;
	int iRet;

	if ( pCompSizeRet != NULL ) {
		*pCompSizeRet = 0;
	}
	if ( pLevelRet != NULL ) {
		*pLevelRet = iLevel;
	}
	if ( objDst == NULL ) {
		return procXpkReturnParamError(objDst);
	}
	if ( sPathSrc == NULL || sPathSrc[0] == '\0' ) {
		return procXpkSetError(objDst, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	iAlg = procXpkCompLevelToAlg(iLevel);
	if ( iAlg != XPK_ALG_LZ4 && iAlg != XPK_ALG_LZ4HC ) {
		return procXpkSetError(objDst, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( iRawSize == 0 ) {
		hFileDst = NULL;
		if ( !procXpkAppliedVolumeMode(objDst) ) {
			hFileDst = xrtOpen(objDst->sPathPackage, FALSE, XRT_CP_BINARY);
			if ( hFileDst == NULL ) {
				return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
			}
		}
		iRet = procXpkWriteAt(objDst, hFileDst, XPK_HEAD_SIZE, NULL, 0);
		if ( hFileDst != NULL ) {
			xrtClose(hFileDst);
		}
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		procXpkClearError(objDst);
		return XPK_OK;
	}

	hFileTmp = NULL;
	hFileSrc = xrtOpen((str)sPathSrc, TRUE, XRT_CP_BINARY);
	if ( hFileSrc == NULL ) {
		return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	memset(&objMapSrc, 0, sizeof(objMapSrc));
	memset(&objMapTmp, 0, sizeof(objMapTmp));
	sPathTmp = NULL;
	iRet = procXpkMapFileReadOnly(objDst, hFileSrc, iRawSize, &objMapSrc);
	xrtClose(hFileSrc);
	hFileSrc = NULL;
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	iCompSize = 0;
	iRet = procXpkCodecBound(objDst, iLevel, (uint32_t)iRawSize, &iBound);
	if ( iRet != XPK_OK ) {
		procXpkUnmapFile(&objMapSrc);
		return iRet;
	}

	sPathTmp = procXpkWriteTempPathDup(objDst, ".solid.lz4");
	if ( sPathTmp == NULL ) {
		procXpkUnmapFile(&objMapSrc);
		return xpkLastError(objDst);
	}

	hFileTmp = xrtOpen((str)sPathTmp, FALSE, XRT_CP_BINARY);
	if ( hFileTmp == NULL ) {
		procXpkUnmapFile(&objMapSrc);
		xpkFreeInternal(sPathTmp);
		return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	iRet = procXpkMapFileReadWrite(objDst, hFileTmp, iBound, &objMapTmp);
	if ( iRet != XPK_OK ) {
		procXpkUnmapFile(&objMapSrc);
		xrtClose(hFileTmp);
		xpkFreeInternal(sPathTmp);
		return iRet;
	}

	if ( iAlg == XPK_ALG_LZ4 ) {
		iRetEnc = LZ4_compress_fast((const char*)objMapSrc.pView, (char*)objMapTmp.pView, (int)iRawSize, (int)iBound, procXpkCompLevelToNative(iLevel));
	} else {
		iRetEnc = LZ4_compress_HC((const char*)objMapSrc.pView, (char*)objMapTmp.pView, (int)iRawSize, (int)iBound, procXpkCompLevelToNative(iLevel));
	}
	procXpkUnmapFile(&objMapSrc);
	if ( iRetEnc <= 0 ) {
		procXpkUnmapFile(&objMapTmp);
		xrtClose(hFileTmp);
		if ( xrtFileExists((str)sPathTmp) ) {
			(void)xrtFileDelete((str)sPathTmp);
		}
		xpkFreeInternal(sPathTmp);
		return procXpkSetError(objDst, XPK_ERR_IO, (iAlg == XPK_ALG_LZ4) ? "lz4 compress failed" : "lz4hc compress failed");
	}

	iCompSize = (uint32_t)iRetEnc;
	procXpkUnmapFile(&objMapTmp);
	hFileDst = NULL;
	if ( !procXpkAppliedVolumeMode(objDst) ) {
		hFileDst = xrtOpen(objDst->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFileDst == NULL ) {
			xrtClose(hFileTmp);
			if ( xrtFileExists((str)sPathTmp) ) {
				(void)xrtFileDelete((str)sPathTmp);
			}
			xpkFreeInternal(sPathTmp);
			return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
		}
	}

	if ( iCompSize >= iRawSize ) {
		hFileSrc = xrtOpen((str)sPathSrc, TRUE, XRT_CP_BINARY);
		if ( hFileSrc == NULL ) {
			xrtClose(hFileDst);
			xrtClose(hFileTmp);
			if ( xrtFileExists((str)sPathTmp) ) {
				(void)xrtFileDelete((str)sPathTmp);
			}
			xpkFreeInternal(sPathTmp);
			return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
		}
		iRet = procXpkCopySourceFileToPackage(objDst, hFileSrc, hFileDst, XPK_HEAD_SIZE, iRawSize);
		xrtClose(hFileSrc);
		hFileSrc = NULL;
		xrtClose(hFileDst);
		xrtClose(hFileTmp);
		if ( xrtFileExists((str)sPathTmp) ) {
			(void)xrtFileDelete((str)sPathTmp);
		}
		xpkFreeInternal(sPathTmp);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		if ( pCompSizeRet != NULL ) {
			*pCompSizeRet = (uint32_t)iRawSize;
		}
		if ( pLevelRet != NULL ) {
			*pLevelRet = 0;
		}
		procXpkClearError(objDst);
		return XPK_OK;
	}

	iRet = procXpkSeekFile(objDst, hFileTmp, iCompSize);
	if ( iRet != XPK_OK ) {
		xrtClose(hFileDst);
		xrtClose(hFileTmp);
		if ( xrtFileExists((str)sPathTmp) ) {
			(void)xrtFileDelete((str)sPathTmp);
		}
		xpkFreeInternal(sPathTmp);
		return iRet;
	}
	if ( !xrtSetEOF(hFileTmp) ) {
		xrtClose(hFileDst);
		xrtClose(hFileTmp);
		if ( xrtFileExists((str)sPathTmp) ) {
			(void)xrtFileDelete((str)sPathTmp);
		}
		xpkFreeInternal(sPathTmp);
		return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoWrite);
	}

	iRet = procXpkCopySourceFileToPackage(objDst, hFileTmp, hFileDst, XPK_HEAD_SIZE, iCompSize);
	xrtClose(hFileDst);
	xrtClose(hFileTmp);
	if ( xrtFileExists((str)sPathTmp) ) {
		(void)xrtFileDelete((str)sPathTmp);
	}
	xpkFreeInternal(sPathTmp);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	if ( pCompSizeRet != NULL ) {
		*pCompSizeRet = iCompSize;
	}
	if ( pLevelRet != NULL ) {
		*pLevelRet = iLevel;
	}
	procXpkClearError(objDst);
	return XPK_OK;
}

static inline int procXpkBuildCopyLiveEntriesSolidZstdStreaming(xpkObject objDst, xpkObject objSrc, uint64_t iSolidRawSize64)
{
	uint32_t iPos;
	xpkEntry* pEntry;
	xpkEntry* pEntryDst;
	xpkWriteNode* pNode;
	xfile hFileSrc;
	xfile hFileRaw;
	void* pData;
	char* sPathRaw;
	uint64_t iDataSize;
	uint64_t iOffsetSolid;
	uint32_t iSolidCompSize;
	uint8_t iSolidLevel;
	uint8_t iSolidLevelUsed;
	int iRet;

	hFileSrc = NULL;
	hFileRaw = NULL;
	pData = NULL;
	sPathRaw = NULL;
	iOffsetSolid = 0;
	iSolidCompSize = 0;
	iSolidLevel = procXpkSolidTargetCompLevel(objDst);
	iSolidLevelUsed = iSolidLevel;
	pNode = NULL;

	sPathRaw = procXpkWriteTempPathDup(objDst, ".solid.raw");
	if ( sPathRaw == NULL ) {
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		return xpkLastError(objDst);
	}

	hFileRaw = xrtOpen((str)sPathRaw, FALSE, XRT_CP_BINARY);
	if ( hFileRaw == NULL ) {
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xpkFreeInternal(sPathRaw);
		return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	for ( iPos = 1; iPos <= objSrc->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objSrc->arrEntry, iPos);
		if ( pEntry == NULL ) {
			iRet = procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
			goto lblCleanup;
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}
		iRet = procXpkValidateLiveEntryLookup(objSrc, pEntry);
		if ( iRet != XPK_OK ) {
			iRet = procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
			goto lblCleanup;
		}
		pNode = procXpkFindWriteNode(objSrc, pEntry->iPos, NULL);
		if ( pNode == NULL && !procXpkAppliedVolumeMode(objSrc) && hFileSrc == NULL ) {
			hFileSrc = xrtOpen(objSrc->sPathPackage, TRUE, XRT_CP_BINARY);
			if ( hFileSrc == NULL ) {
				iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
				goto lblCleanup;
			}
		}

		pData = procXpkReadEntryDataWithFile(objSrc, pEntry, &iDataSize, hFileSrc);
		if ( pData == NULL ) {
			iRet = procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
			goto lblCleanup;
		}
		if ( iDataSize != pEntry->iFileSize ) {
			iRet = procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
			goto lblCleanup;
		}

		iRet = procXpkBuildWritePlainFileChunked(objDst, hFileRaw, pData, iDataSize);
		xpkFree(pData);
		pData = NULL;
		if ( iRet != XPK_OK ) {
			goto lblCleanup;
		}

		iRet = procXpkBuildAppendEntryOnly(objDst, pEntry, iOffsetSolid, pEntry->iFileSize, pEntry->iFileSize);
		if ( iRet != XPK_OK ) {
			goto lblCleanup;
		}
		iOffsetSolid += pEntry->iFileSize;
	}

	if ( hFileRaw != NULL ) {
		xrtClose(hFileRaw);
		hFileRaw = NULL;
	}
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
		hFileSrc = NULL;
	}

	iRet = procXpkRebuildLookup(objDst);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	iRet = procXpkBuildWriteSolidZstdFromFile(objDst, sPathRaw, iSolidRawSize64, iSolidLevel, &iSolidCompSize, &iSolidLevelUsed);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	objDst->objHead.defComp = iSolidLevelUsed;
	for ( iPos = 1; iPos <= objDst->iEntryCount; iPos++ ) {
		pEntryDst = (xpkEntry*)xrtArrayGet(&objDst->arrEntry, iPos);
		if ( pEntryDst == NULL ) {
			iRet = procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
			goto lblCleanup;
		}
		pEntryDst->iFlag = (pEntryDst->iFlag & ~XPK_FLAG_COMP_MASK) | iSolidLevelUsed;
	}
	objDst->bSolidApplied = TRUE;
	objDst->iAppendPos = XPK_HEAD_SIZE + ((iSolidLevelUsed == 0) ? iSolidRawSize64 : iSolidCompSize);
	objDst->iFileSize = objDst->iAppendPos;
	objDst->bDirtyData = TRUE;
	objDst->bDirtyEntryTable = TRUE;
	objDst->bDirtyHead = TRUE;
	procXpkClearError(objDst);
	iRet = XPK_OK;

lblCleanup:
	if ( pData != NULL ) {
		xpkFree(pData);
	}
	if ( hFileRaw != NULL ) {
		xrtClose(hFileRaw);
	}
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}
	if ( sPathRaw != NULL ) {
		if ( xrtFileExists((str)sPathRaw) ) {
			(void)xrtFileDelete((str)sPathRaw);
		}
		xpkFreeInternal(sPathRaw);
	}
	return iRet;
}

static inline int procXpkBuildCopyLiveEntriesSolidLzma2Streaming(xpkObject objDst, xpkObject objSrc, uint64_t iSolidRawSize64)
{
	uint32_t iPos;
	xpkEntry* pEntry;
	xpkEntry* pEntryDst;
	xpkWriteNode* pNode;
	xfile hFileSrc;
	xfile hFileRaw;
	void* pData;
	char* sPathRaw;
	uint64_t iDataSize;
	uint64_t iOffsetSolid;
	uint32_t iSolidCompSize;
	uint8_t iSolidLevel;
	uint8_t iSolidLevelUsed;
	int iRet;

	hFileSrc = NULL;
	hFileRaw = NULL;
	pData = NULL;
	sPathRaw = NULL;
	iOffsetSolid = 0;
	iSolidCompSize = 0;
	iSolidLevel = procXpkSolidTargetCompLevel(objDst);
	iSolidLevelUsed = iSolidLevel;
	pNode = NULL;

	sPathRaw = procXpkWriteTempPathDup(objDst, ".solid.raw");
	if ( sPathRaw == NULL ) {
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		return xpkLastError(objDst);
	}

	hFileRaw = xrtOpen((str)sPathRaw, FALSE, XRT_CP_BINARY);
	if ( hFileRaw == NULL ) {
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xpkFreeInternal(sPathRaw);
		return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	for ( iPos = 1; iPos <= objSrc->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objSrc->arrEntry, iPos);
		if ( pEntry == NULL ) {
			iRet = procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
			goto lblCleanup;
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}
		iRet = procXpkValidateLiveEntryLookup(objSrc, pEntry);
		if ( iRet != XPK_OK ) {
			iRet = procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
			goto lblCleanup;
		}
		pNode = procXpkFindWriteNode(objSrc, pEntry->iPos, NULL);
		if ( pNode == NULL && !procXpkAppliedVolumeMode(objSrc) && hFileSrc == NULL ) {
			hFileSrc = xrtOpen(objSrc->sPathPackage, TRUE, XRT_CP_BINARY);
			if ( hFileSrc == NULL ) {
				iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
				goto lblCleanup;
			}
		}

		pData = procXpkReadEntryDataWithFile(objSrc, pEntry, &iDataSize, hFileSrc);
		if ( pData == NULL ) {
			iRet = procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
			goto lblCleanup;
		}
		if ( iDataSize != pEntry->iFileSize ) {
			iRet = procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
			goto lblCleanup;
		}

		iRet = procXpkBuildWritePlainFileChunked(objDst, hFileRaw, pData, iDataSize);
		xpkFree(pData);
		pData = NULL;
		if ( iRet != XPK_OK ) {
			goto lblCleanup;
		}

		iRet = procXpkBuildAppendEntryOnly(objDst, pEntry, iOffsetSolid, pEntry->iFileSize, pEntry->iFileSize);
		if ( iRet != XPK_OK ) {
			goto lblCleanup;
		}
		iOffsetSolid += pEntry->iFileSize;
	}

	if ( hFileRaw != NULL ) {
		xrtClose(hFileRaw);
		hFileRaw = NULL;
	}
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
		hFileSrc = NULL;
	}

	iRet = procXpkRebuildLookup(objDst);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	iRet = procXpkBuildWriteSolidLzma2FromFile(objDst, sPathRaw, iSolidRawSize64, iSolidLevel, &iSolidCompSize, &iSolidLevelUsed);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	objDst->objHead.defComp = iSolidLevelUsed;
	for ( iPos = 1; iPos <= objDst->iEntryCount; iPos++ ) {
		pEntryDst = (xpkEntry*)xrtArrayGet(&objDst->arrEntry, iPos);
		if ( pEntryDst == NULL ) {
			iRet = procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
			goto lblCleanup;
		}
		pEntryDst->iFlag = (pEntryDst->iFlag & ~XPK_FLAG_COMP_MASK) | iSolidLevelUsed;
	}
	objDst->bSolidApplied = TRUE;
	objDst->iAppendPos = XPK_HEAD_SIZE + ((iSolidLevelUsed == 0) ? iSolidRawSize64 : iSolidCompSize);
	objDst->iFileSize = objDst->iAppendPos;
	objDst->bDirtyData = TRUE;
	objDst->bDirtyEntryTable = TRUE;
	objDst->bDirtyHead = TRUE;
	procXpkClearError(objDst);
	iRet = XPK_OK;

lblCleanup:
	if ( pData != NULL ) {
		xpkFree(pData);
	}
	if ( hFileRaw != NULL ) {
		xrtClose(hFileRaw);
	}
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}
	if ( sPathRaw != NULL ) {
		if ( xrtFileExists((str)sPathRaw) ) {
			(void)xrtFileDelete((str)sPathRaw);
		}
		xpkFreeInternal(sPathRaw);
	}
	return iRet;
}

static inline int procXpkBuildCopyLiveEntriesSolidLz4Streaming(xpkObject objDst, xpkObject objSrc, uint64_t iSolidRawSize64)
{
	uint32_t iPos;
	xpkEntry* pEntry;
	xpkEntry* pEntryDst;
	xpkWriteNode* pNode;
	xfile hFileSrc;
	xfile hFileRaw;
	void* pData;
	char* sPathRaw;
	uint64_t iDataSize;
	uint64_t iOffsetSolid;
	uint32_t iSolidCompSize;
	uint8_t iSolidLevel;
	uint8_t iSolidLevelUsed;
	int iRet;

	hFileSrc = NULL;
	hFileRaw = NULL;
	pData = NULL;
	sPathRaw = NULL;
	iOffsetSolid = 0;
	iSolidCompSize = 0;
	iSolidLevel = procXpkSolidTargetCompLevel(objDst);
	iSolidLevelUsed = iSolidLevel;
	pNode = NULL;

	sPathRaw = procXpkWriteTempPathDup(objDst, ".solid.raw");
	if ( sPathRaw == NULL ) {
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		return xpkLastError(objDst);
	}

	hFileRaw = xrtOpen((str)sPathRaw, FALSE, XRT_CP_BINARY);
	if ( hFileRaw == NULL ) {
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xpkFreeInternal(sPathRaw);
		return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	for ( iPos = 1; iPos <= objSrc->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objSrc->arrEntry, iPos);
		if ( pEntry == NULL ) {
			iRet = procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
			goto lblCleanup;
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}
		iRet = procXpkValidateLiveEntryLookup(objSrc, pEntry);
		if ( iRet != XPK_OK ) {
			iRet = procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
			goto lblCleanup;
		}
		pNode = procXpkFindWriteNode(objSrc, pEntry->iPos, NULL);
		if ( pNode == NULL && !procXpkAppliedVolumeMode(objSrc) && hFileSrc == NULL ) {
			hFileSrc = xrtOpen(objSrc->sPathPackage, TRUE, XRT_CP_BINARY);
			if ( hFileSrc == NULL ) {
				iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
				goto lblCleanup;
			}
		}

		pData = procXpkReadEntryDataWithFile(objSrc, pEntry, &iDataSize, hFileSrc);
		if ( pData == NULL ) {
			iRet = procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
			goto lblCleanup;
		}
		if ( iDataSize != pEntry->iFileSize ) {
			iRet = procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
			goto lblCleanup;
		}

		iRet = procXpkBuildWritePlainFileChunked(objDst, hFileRaw, pData, iDataSize);
		xpkFree(pData);
		pData = NULL;
		if ( iRet != XPK_OK ) {
			goto lblCleanup;
		}

		iRet = procXpkBuildAppendEntryOnly(objDst, pEntry, iOffsetSolid, pEntry->iFileSize, pEntry->iFileSize);
		if ( iRet != XPK_OK ) {
			goto lblCleanup;
		}
		iOffsetSolid += pEntry->iFileSize;
	}

	if ( hFileRaw != NULL ) {
		xrtClose(hFileRaw);
		hFileRaw = NULL;
	}
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
		hFileSrc = NULL;
	}

	iRet = procXpkRebuildLookup(objDst);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	iRet = procXpkBuildWriteSolidLz4FromFile(objDst, sPathRaw, iSolidRawSize64, iSolidLevel, &iSolidCompSize, &iSolidLevelUsed);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	objDst->objHead.defComp = iSolidLevelUsed;
	for ( iPos = 1; iPos <= objDst->iEntryCount; iPos++ ) {
		pEntryDst = (xpkEntry*)xrtArrayGet(&objDst->arrEntry, iPos);
		if ( pEntryDst == NULL ) {
			iRet = procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
			goto lblCleanup;
		}
		pEntryDst->iFlag = (pEntryDst->iFlag & ~XPK_FLAG_COMP_MASK) | iSolidLevelUsed;
	}
	objDst->bSolidApplied = TRUE;
	objDst->iAppendPos = XPK_HEAD_SIZE + ((iSolidLevelUsed == 0) ? iSolidRawSize64 : iSolidCompSize);
	objDst->iFileSize = objDst->iAppendPos;
	objDst->bDirtyData = TRUE;
	objDst->bDirtyEntryTable = TRUE;
	objDst->bDirtyHead = TRUE;
	procXpkClearError(objDst);
	iRet = XPK_OK;

lblCleanup:
	if ( pData != NULL ) {
		xpkFree(pData);
	}
	if ( hFileRaw != NULL ) {
		xrtClose(hFileRaw);
	}
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}
	if ( sPathRaw != NULL ) {
		if ( xrtFileExists((str)sPathRaw) ) {
			(void)xrtFileDelete((str)sPathRaw);
		}
		xpkFreeInternal(sPathRaw);
	}
	return iRet;
}

static inline int procXpkBuildCopyLiveEntriesSolidStoreStreaming(xpkObject objDst, xpkObject objSrc, uint64_t iSolidRawSize64)
{
	uint32_t iPos;
	xpkEntry* pEntry;
	xpkEntry* pEntryDst;
	xpkWriteNode* pNode;
	xfile hFileSrc;
	xfile hFileRaw;
	xfile hFileRawSrc;
	xfile hFileDst;
	void* pData;
	char* sPathRaw;
	uint64_t iDataSize;
	uint64_t iOffsetSolid;
	int iRet;

	hFileSrc = NULL;
	hFileRaw = NULL;
	hFileRawSrc = NULL;
	hFileDst = NULL;
	pData = NULL;
	sPathRaw = NULL;
	iOffsetSolid = 0;
	pNode = NULL;

	sPathRaw = procXpkWriteTempPathDup(objDst, ".solid.raw");
	if ( sPathRaw == NULL ) {
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		return xpkLastError(objDst);
	}

	hFileRaw = xrtOpen((str)sPathRaw, FALSE, XRT_CP_BINARY);
	if ( hFileRaw == NULL ) {
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xpkFreeInternal(sPathRaw);
		return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	for ( iPos = 1; iPos <= objSrc->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objSrc->arrEntry, iPos);
		if ( pEntry == NULL ) {
			iRet = procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
			goto lblCleanup;
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}
		iRet = procXpkValidateLiveEntryLookup(objSrc, pEntry);
		if ( iRet != XPK_OK ) {
			iRet = procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
			goto lblCleanup;
		}
		pNode = procXpkFindWriteNode(objSrc, pEntry->iPos, NULL);
		if ( pNode == NULL && !procXpkAppliedVolumeMode(objSrc) && hFileSrc == NULL ) {
			hFileSrc = xrtOpen(objSrc->sPathPackage, TRUE, XRT_CP_BINARY);
			if ( hFileSrc == NULL ) {
				iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
				goto lblCleanup;
			}
		}

		pData = procXpkReadEntryDataWithFile(objSrc, pEntry, &iDataSize, hFileSrc);
		if ( pData == NULL ) {
			iRet = procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
			goto lblCleanup;
		}
		if ( iDataSize != pEntry->iFileSize ) {
			iRet = procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
			goto lblCleanup;
		}

		iRet = procXpkBuildWritePlainFileChunked(objDst, hFileRaw, pData, iDataSize);
		xpkFree(pData);
		pData = NULL;
		if ( iRet != XPK_OK ) {
			goto lblCleanup;
		}

		iRet = procXpkBuildAppendEntryOnly(objDst, pEntry, iOffsetSolid, pEntry->iFileSize, pEntry->iFileSize);
		if ( iRet != XPK_OK ) {
			goto lblCleanup;
		}
		iOffsetSolid += pEntry->iFileSize;
	}

	if ( hFileRaw != NULL ) {
		xrtClose(hFileRaw);
		hFileRaw = NULL;
	}
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
		hFileSrc = NULL;
	}

	iRet = procXpkRebuildLookup(objDst);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	hFileRawSrc = xrtOpen((str)sPathRaw, TRUE, XRT_CP_BINARY);
	if ( hFileRawSrc == NULL ) {
		iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
		goto lblCleanup;
	}
	if ( !procXpkAppliedVolumeMode(objDst) ) {
		hFileDst = xrtOpen(objDst->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFileDst == NULL ) {
			iRet = procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
			goto lblCleanup;
		}
	}

	iRet = procXpkCopySourceFileToPackage(objDst, hFileRawSrc, hFileDst, XPK_HEAD_SIZE, iSolidRawSize64);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	objDst->objHead.defComp = 0;
	for ( iPos = 1; iPos <= objDst->iEntryCount; iPos++ ) {
		pEntryDst = (xpkEntry*)xrtArrayGet(&objDst->arrEntry, iPos);
		if ( pEntryDst == NULL ) {
			iRet = procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
			goto lblCleanup;
		}
		pEntryDst->iFlag &= ~XPK_FLAG_COMP_MASK;
	}
	objDst->bSolidApplied = TRUE;
	objDst->iAppendPos = XPK_HEAD_SIZE + iSolidRawSize64;
	objDst->iFileSize = objDst->iAppendPos;
	objDst->bDirtyData = TRUE;
	objDst->bDirtyEntryTable = TRUE;
	objDst->bDirtyHead = TRUE;
	procXpkClearError(objDst);
	iRet = XPK_OK;

lblCleanup:
	if ( pData != NULL ) {
		xpkFree(pData);
	}
	if ( hFileDst != NULL ) {
		xrtClose(hFileDst);
	}
	if ( hFileRawSrc != NULL ) {
		xrtClose(hFileRawSrc);
	}
	if ( hFileRaw != NULL ) {
		xrtClose(hFileRaw);
	}
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}
	if ( sPathRaw != NULL ) {
		if ( xrtFileExists((str)sPathRaw) ) {
			(void)xrtFileDelete((str)sPathRaw);
		}
		xpkFreeInternal(sPathRaw);
	}
	return iRet;
}

static inline int procXpkBuildAppendCopiedEntry(xpkObject objDst, const xpkEntry* pEntrySrc, uint8_t iCompLevel, uint64_t iDataOffset, uint64_t iDataSize, uint64_t iFileSize)
{
	xpkEntry objSeed;
	int iRet;

	iRet = procXpkBuildInitSeed(objDst, pEntrySrc, &objSeed);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	objSeed.iFlag = (pEntrySrc->iFlag & ~XPK_FLAG_DELETED_MASK);
	objSeed.iFlag = (objSeed.iFlag & ~XPK_FLAG_COMP_MASK) | (uint32_t)(iCompLevel & XPK_FLAG_COMP_MASK);
	objSeed.iFileHash = pEntrySrc->iFileHash;
	objSeed.iDataOffset = iDataOffset;
	objSeed.iDataSize = iDataSize;
	objSeed.iFileSize = iFileSize;
	objSeed.bStored = TRUE;

	iRet = procXpkAppendEntryOwned(objDst, &objSeed);
	procXpkFreeEntryOwned(&objSeed);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	return XPK_OK;
}

static inline int procXpkBuildCopyEntryDirectWithFiles(xpkObject objDst, xpkObject objSrc, const xpkEntry* pEntrySrc, xfile hFileDst, xfile hFileSrc, void* pChunkShared)
{
	xpkWriteNode* pNode;
	xfile hFileSrcLocal;
	xfile hFileDstLocal;
	uint64_t iOffsetDst;
	uint64_t iDataEndSrc;
	uint64_t iCompSize;
	uint8_t iCompLevel;
	int iRet;

	if ( objDst == NULL || objSrc == NULL || pEntrySrc == NULL ) {
		return procXpkReturnParamError(objDst);
	}

	hFileSrcLocal = NULL;
	hFileDstLocal = NULL;
	iOffsetDst = objDst->iAppendPos;
	iDataEndSrc = procXpkCurrentDataEnd(objSrc);
	pNode = procXpkFindWriteNode(objSrc, pEntrySrc->iPos, NULL);
	if ( !procXpkAppliedVolumeMode(objDst) && hFileDst == NULL ) {
		hFileDstLocal = xrtOpen(objDst->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFileDstLocal == NULL ) {
			return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
		}
		hFileDst = hFileDstLocal;
	}

	if ( pNode != NULL ) {
		if ( pNode->iRawSize != pEntrySrc->iFileSize ) {
			if ( hFileDstLocal != NULL ) {
				xrtClose(hFileDstLocal);
			}
			return procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
		if ( pNode->pCompData == NULL && pNode->iCompSize > 0 ) {
			if ( hFileDstLocal != NULL ) {
				xrtClose(hFileDstLocal);
			}
			return procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}

		iCompLevel = pNode->iLevel;
		iCompSize = pNode->iCompSize;
		iRet = procXpkBuildWriteAtChunked(objDst, hFileDst, iOffsetDst, pNode->pCompData, iCompSize);
	} else {
		if ( pEntrySrc->iDataOffset < XPK_HEAD_SIZE ) {
			if ( hFileDstLocal != NULL ) {
				xrtClose(hFileDstLocal);
			}
			return procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
		if ( pEntrySrc->iDataOffset > iDataEndSrc ) {
			if ( hFileDstLocal != NULL ) {
				xrtClose(hFileDstLocal);
			}
			return procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
		if ( pEntrySrc->iDataSize > (iDataEndSrc - pEntrySrc->iDataOffset) ) {
			if ( hFileDstLocal != NULL ) {
				xrtClose(hFileDstLocal);
			}
			return procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}

		if ( !procXpkAppliedVolumeMode(objSrc) && hFileSrc == NULL ) {
			hFileSrcLocal = xrtOpen(objSrc->sPathPackage, TRUE, XRT_CP_BINARY);
			if ( hFileSrcLocal == NULL ) {
				if ( hFileDstLocal != NULL ) {
					xrtClose(hFileDstLocal);
				}
				return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
			}
			hFileSrc = hFileSrcLocal;
		}

		iCompLevel = (uint8_t)(pEntrySrc->iFlag & XPK_FLAG_COMP_MASK);
		iCompSize = pEntrySrc->iDataSize;
		iRet = procXpkBuildCopyStoredPayload(objDst, hFileDst, objSrc, hFileSrc, iOffsetDst, pEntrySrc->iDataOffset, iCompSize, pChunkShared);
	}

	if ( hFileSrcLocal != NULL ) {
		xrtClose(hFileSrcLocal);
	}
	if ( hFileDstLocal != NULL ) {
		xrtClose(hFileDstLocal);
	}
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	iRet = procXpkBuildAppendCopiedEntry(objDst, pEntrySrc, iCompLevel, iOffsetDst, iCompSize, pEntrySrc->iFileSize);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	objDst->iAppendPos = iOffsetDst + iCompSize;
	if ( objDst->iAppendPos > objDst->iFileSize ) {
		objDst->iFileSize = objDst->iAppendPos;
	}
	objDst->bDirtyData = TRUE;
	procXpkMarkDirtyEntryTable(objDst);
	procXpkClearError(objDst);
	return XPK_OK;
}

static inline int procXpkBuildCopyEntryDirect(xpkObject objDst, xpkObject objSrc, const xpkEntry* pEntrySrc)
{
	return procXpkBuildCopyEntryDirectWithFiles(objDst, objSrc, pEntrySrc, NULL, NULL, NULL);
}

static inline int procXpkBuildCopyEntry(xpkObject objDst, xpkObject objSrc, const xpkEntry* pEntrySrc)
{
	void* pData;
	uint64_t iSize;
	int iRet;

	if ( !objDst->objHead.solidMode && !objSrc->bSolidApplied ) {
		return procXpkBuildCopyEntryDirect(objDst, objSrc, pEntrySrc);
	}

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

static inline int procXpkBuildCheckSolidStoreDirect(xpkObject objDst, xpkObject objSrc, int* pCanCopyRet)
{
	uint32_t iPos;
	xpkEntry* pEntry;
	xpkWriteNode* pNode;

	if ( pCanCopyRet != NULL ) {
		*pCanCopyRet = FALSE;
	}
	if ( objDst == NULL || objSrc == NULL || pCanCopyRet == NULL ) {
		return procXpkReturnParamError(objDst);
	}
	if ( objSrc->bSolidApplied || !objDst->objHead.solidMode ) {
		return XPK_OK;
	}
	if ( procXpkSolidTargetCompLevel(objDst) != 0 ) {
		return XPK_OK;
	}

	for ( iPos = 1; iPos <= objSrc->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objSrc->arrEntry, iPos);
		if ( pEntry == NULL ) {
			return procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}
		if ( procXpkValidateLiveEntryLookup(objSrc, pEntry) != XPK_OK ) {
			return procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
		}

		pNode = procXpkFindWriteNode(objSrc, pEntry->iPos, NULL);
		if ( pNode != NULL ) {
			if ( pNode->iLevel != 0 ) {
				return XPK_OK;
			}
			if ( pNode->iCompSize != pNode->iRawSize || pNode->iRawSize != pEntry->iFileSize ) {
				return procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
			}
			if ( pNode->pCompData == NULL && pNode->iCompSize > 0 ) {
				return procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
			}
			continue;
		}

		if ( (pEntry->iFlag & XPK_FLAG_COMP_MASK) != 0 ) {
			return XPK_OK;
		}
		if ( pEntry->iDataSize != pEntry->iFileSize ) {
			return procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
		if ( pEntry->iDataOffset < XPK_HEAD_SIZE ) {
			return procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
		if ( pEntry->iDataOffset > procXpkCurrentDataEnd(objSrc) ) {
			return procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
		if ( pEntry->iDataSize > (procXpkCurrentDataEnd(objSrc) - pEntry->iDataOffset) ) {
			return procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
	}

	*pCanCopyRet = TRUE;
	return XPK_OK;
}

static inline int procXpkBuildCheckSingleEntrySolidDirectFromNormal(xpkObject objDst, xpkObject objSrc, xpkEntry** pEntryRet, xpkWriteNode** pNodeRet, uint64_t* pCompSizeRet, int* pCanCopyRet)
{
	uint32_t iPos;
	uint32_t iLiveCount;
	xpkEntry* pEntry;
	xpkWriteNode* pNode;
	uint8_t iSolidLevel;
	uint64_t iDataEnd;
	int iRet;

	if ( pEntryRet != NULL ) {
		*pEntryRet = NULL;
	}
	if ( pNodeRet != NULL ) {
		*pNodeRet = NULL;
	}
	if ( pCompSizeRet != NULL ) {
		*pCompSizeRet = 0;
	}
	if ( pCanCopyRet != NULL ) {
		*pCanCopyRet = FALSE;
	}
	if ( objDst == NULL || objSrc == NULL || pEntryRet == NULL || pNodeRet == NULL || pCompSizeRet == NULL || pCanCopyRet == NULL ) {
		return procXpkReturnParamError(objDst);
	}
	if ( objSrc->bSolidApplied || !objDst->objHead.solidMode ) {
		return XPK_OK;
	}

	iSolidLevel = procXpkSolidTargetCompLevel(objDst);
	if ( iSolidLevel == 0 ) {
		return XPK_OK;
	}

	iRet = procXpkVisibleEntryCountStrict(objSrc, &iLiveCount);
	if ( iRet != XPK_OK ) {
		return procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
	}
	if ( iLiveCount != 1 ) {
		return XPK_OK;
	}

	pEntry = NULL;
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
		break;
	}

	if ( pEntry == NULL || procXpkEntryDeleted(pEntry) ) {
		return procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
	}

	pNode = procXpkFindWriteNode(objSrc, pEntry->iPos, NULL);
	if ( pNode != NULL ) {
		if ( pNode->iLevel != iSolidLevel ) {
			return XPK_OK;
		}
		if ( pNode->iRawSize != pEntry->iFileSize ) {
			return procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
		if ( pNode->pCompData == NULL && pNode->iCompSize > 0 ) {
			return procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
		*pEntryRet = pEntry;
		*pNodeRet = pNode;
		*pCompSizeRet = pNode->iCompSize;
		*pCanCopyRet = TRUE;
		return XPK_OK;
	}

	if ( (pEntry->iFlag & XPK_FLAG_COMP_MASK) != iSolidLevel ) {
		return XPK_OK;
	}
	iDataEnd = procXpkCurrentDataEnd(objSrc);
	if ( pEntry->iDataOffset < XPK_HEAD_SIZE ) {
		return procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pEntry->iDataOffset > iDataEnd ) {
		return procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pEntry->iDataSize > (iDataEnd - pEntry->iDataOffset) ) {
		return procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	*pEntryRet = pEntry;
	*pNodeRet = NULL;
	*pCompSizeRet = pEntry->iDataSize;
	*pCanCopyRet = TRUE;
	return XPK_OK;
}

static inline int procXpkBuildCopySingleEntrySolidDirectFromNormal(xpkObject objDst, xpkObject objSrc)
{
	xpkEntry* pEntry;
	xpkWriteNode* pNode;
	xfile hFileSrc;
	xfile hFileDst;
	uint64_t iCompSize;
	uint8_t iSolidLevel;
	int bCanCopy;
	int iRet;

	pEntry = NULL;
	pNode = NULL;
	iCompSize = 0;
	bCanCopy = FALSE;
	iRet = procXpkBuildCheckSingleEntrySolidDirectFromNormal(objDst, objSrc, &pEntry, &pNode, &iCompSize, &bCanCopy);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( !bCanCopy || pEntry == NULL ) {
		return procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
	}

	hFileSrc = NULL;
	hFileDst = NULL;
	if ( !procXpkAppliedVolumeMode(objDst) ) {
		hFileDst = xrtOpen(objDst->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFileDst == NULL ) {
			return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
		}
	}

	if ( pNode != NULL ) {
		iRet = procXpkBuildWriteAtChunked(objDst, hFileDst, XPK_HEAD_SIZE, pNode->pCompData, iCompSize);
	} else {
		if ( !procXpkAppliedVolumeMode(objSrc) ) {
			hFileSrc = xrtOpen(objSrc->sPathPackage, TRUE, XRT_CP_BINARY);
			if ( hFileSrc == NULL ) {
				if ( hFileDst != NULL ) {
					xrtClose(hFileDst);
				}
				return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
			}
		}
		iRet = procXpkBuildCopyStoredPayload(objDst, hFileDst, objSrc, hFileSrc, XPK_HEAD_SIZE, pEntry->iDataOffset, iCompSize, NULL);
	}

	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}
	if ( hFileDst != NULL ) {
		xrtClose(hFileDst);
	}
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	iRet = procXpkBuildAppendEntryOnly(objDst, pEntry, 0, pEntry->iFileSize, pEntry->iFileSize);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	iRet = procXpkRebuildLookup(objDst);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	iSolidLevel = procXpkSolidTargetCompLevel(objDst);
	objDst->objHead.defComp = iSolidLevel;
	objDst->bSolidApplied = TRUE;
	objDst->iAppendPos = XPK_HEAD_SIZE + iCompSize;
	objDst->iFileSize = objDst->iAppendPos;
	objDst->bDirtyData = TRUE;
	objDst->bDirtyEntryTable = TRUE;
	objDst->bDirtyHead = TRUE;
	procXpkClearError(objDst);
	return XPK_OK;
}

static inline int procXpkBuildCopyLiveEntriesSolidStoreDirect(xpkObject objDst, xpkObject objSrc)
{
	uint32_t iPos;
	xpkEntry* pEntry;
	xpkWriteNode* pNode;
	xfile hFileSrc;
	xfile hFileDst;
	void* pChunkBuild;
	uint64_t iOffsetSolid;
	int iRet;

	hFileSrc = NULL;
	hFileDst = NULL;
	pChunkBuild = NULL;
	iOffsetSolid = 0;
	if ( !procXpkAppliedVolumeMode(objDst) ) {
		hFileDst = xrtOpen(objDst->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFileDst == NULL ) {
			return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
		}
	}

	for ( iPos = 1; iPos <= objSrc->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objSrc->arrEntry, iPos);
		if ( pEntry == NULL ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat));
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}

		pNode = procXpkFindWriteNode(objSrc, pEntry->iPos, NULL);
		if ( pNode != NULL ) {
			iRet = procXpkBuildWriteAtChunked(objDst, hFileDst, XPK_HEAD_SIZE + iOffsetSolid, pNode->pCompData, pNode->iCompSize);
		} else {
			if ( !procXpkAppliedVolumeMode(objSrc) && hFileSrc == NULL ) {
				hFileSrc = xrtOpen(objSrc->sPathPackage, TRUE, XRT_CP_BINARY);
				if ( hFileSrc == NULL ) {
					if ( hFileDst != NULL ) {
						xrtClose(hFileDst);
					}
					return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
				}
			}
			if ( pChunkBuild == NULL ) {
				pChunkBuild = xpkAllocInternal(XPK_WRITE_FILE_CHUNK_SIZE);
				if ( pChunkBuild == NULL ) {
					return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, XPK_ERR_MEMORY, sXpkErrorOutOfMemory));
				}
			}
			iRet = procXpkBuildCopyStoredPayload(objDst, hFileDst, objSrc, hFileSrc, XPK_HEAD_SIZE + iOffsetSolid, pEntry->iDataOffset, pEntry->iDataSize, pChunkBuild);
		}
		if ( iRet != XPK_OK ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, iRet);
		}

		iRet = procXpkBuildAppendEntryOnly(objDst, pEntry, iOffsetSolid, pEntry->iFileSize, pEntry->iFileSize);
		if ( iRet != XPK_OK ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, iRet);
		}

		iOffsetSolid += pEntry->iFileSize;
	}

	if ( pChunkBuild != NULL ) {
		xpkFreeInternal(pChunkBuild);
	}
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}
	if ( hFileDst != NULL ) {
		xrtClose(hFileDst);
	}

	iRet = procXpkRebuildLookup(objDst);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	objDst->objHead.defComp = 0;
	objDst->bSolidApplied = TRUE;
	objDst->iAppendPos = XPK_HEAD_SIZE + iOffsetSolid;
	objDst->iFileSize = objDst->iAppendPos;
	objDst->bDirtyData = TRUE;
	objDst->bDirtyEntryTable = TRUE;
	objDst->bDirtyHead = TRUE;
	procXpkClearError(objDst);
	return XPK_OK;
}

static inline int procXpkBuildCheckSolidDirectCopy(xpkObject objDst, xpkObject objSrc, uint64_t* pCompSizeRet, int* pCanCopyRet)
{
	uint32_t iPos;
	xpkEntry* pEntry;
	uint64_t iOffsetExpect;
	int iRet;

	if ( pCompSizeRet != NULL ) {
		*pCompSizeRet = 0;
	}
	if ( pCanCopyRet != NULL ) {
		*pCanCopyRet = FALSE;
	}
	if ( objDst == NULL || objSrc == NULL || pCompSizeRet == NULL || pCanCopyRet == NULL ) {
		return procXpkReturnParamError(objDst);
	}
	if ( !objSrc->bSolidApplied || !objDst->objHead.solidMode ) {
		return XPK_OK;
	}
	if ( procXpkSolidStoredCompLevel(objSrc) != procXpkSolidTargetCompLevel(objDst) ) {
		return XPK_OK;
	}
	if ( objSrc->objHead.dataOffset < XPK_HEAD_SIZE ) {
		return procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	iOffsetExpect = 0;
	for ( iPos = 1; iPos <= objSrc->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objSrc->arrEntry, iPos);
		if ( pEntry == NULL ) {
			return procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			return XPK_OK;
		}
		iRet = procXpkValidateLiveEntryLookup(objSrc, pEntry);
		if ( iRet != XPK_OK ) {
			return procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
		}
		if ( pEntry->iDataOffset != iOffsetExpect ) {
			return XPK_OK;
		}
		iOffsetExpect += pEntry->iFileSize;
	}

	*pCompSizeRet = objSrc->objHead.dataOffset - XPK_HEAD_SIZE;
	*pCanCopyRet = TRUE;
	return XPK_OK;
}

static inline int procXpkBuildCopySolidBlockDirect(xpkObject objDst, xpkObject objSrc)
{
	uint32_t iPos;
	xpkEntry* pEntry;
	uint64_t iCompSize;
	int bCanCopy;
	xfile hFileSrc;
	xfile hFileDst;
	uint8_t iSolidLevel;
	int iRet;

	iCompSize = 0;
	bCanCopy = FALSE;
	iRet = procXpkBuildCheckSolidDirectCopy(objDst, objSrc, &iCompSize, &bCanCopy);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( !bCanCopy ) {
		return procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
	}

	hFileSrc = NULL;
	hFileDst = NULL;
	if ( !procXpkAppliedVolumeMode(objSrc) ) {
		hFileSrc = xrtOpen(objSrc->sPathPackage, TRUE, XRT_CP_BINARY);
		if ( hFileSrc == NULL ) {
			return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
		}
	}
	if ( !procXpkAppliedVolumeMode(objDst) ) {
		hFileDst = xrtOpen(objDst->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFileDst == NULL ) {
			if ( hFileSrc != NULL ) {
				xrtClose(hFileSrc);
			}
			return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
		}
	}

	iRet = procXpkBuildCopyStoredPayload(objDst, hFileDst, objSrc, hFileSrc, XPK_HEAD_SIZE, XPK_HEAD_SIZE, iCompSize, NULL);
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}
	if ( hFileDst != NULL ) {
		xrtClose(hFileDst);
	}
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	for ( iPos = 1; iPos <= objSrc->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objSrc->arrEntry, iPos);
		if ( pEntry == NULL ) {
			return procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
		}
		iRet = procXpkBuildAppendEntryOnly(objDst, pEntry, pEntry->iDataOffset, pEntry->iFileSize, pEntry->iFileSize);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
	}

	iRet = procXpkRebuildLookup(objDst);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	iSolidLevel = procXpkSolidStoredCompLevel(objSrc);
	objDst->objHead.defComp = iSolidLevel;
	objDst->bSolidApplied = TRUE;
	objDst->iAppendPos = XPK_HEAD_SIZE + iCompSize;
	objDst->iFileSize = objDst->iAppendPos;
	objDst->bDirtyData = TRUE;
	objDst->bDirtyEntryTable = TRUE;
	objDst->bDirtyHead = TRUE;
	procXpkClearError(objDst);
	return XPK_OK;
}

static inline int procXpkBuildCopyLiveEntriesSolidStoreCompact(xpkObject objDst, xpkObject objSrc)
{
	uint32_t iPos;
	xpkEntry* pEntry;
	xfile hFileSrc;
	xfile hFileDst;
	void* pChunkBuild;
	uint64_t iOffsetSolid;
	uint64_t iSolidSrcSize;
	int iRet;

	iRet = procXpkCalcSolidStoredRawSize(objSrc, &iSolidSrcSize);
	if ( iRet != XPK_OK ) {
		return procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
	}

	hFileSrc = NULL;
	hFileDst = NULL;
	pChunkBuild = NULL;
	iOffsetSolid = 0;
	if ( !procXpkAppliedVolumeMode(objSrc) ) {
		hFileSrc = xrtOpen(objSrc->sPathPackage, TRUE, XRT_CP_BINARY);
		if ( hFileSrc == NULL ) {
			return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
		}
	}
	if ( !procXpkAppliedVolumeMode(objDst) ) {
		hFileDst = xrtOpen(objDst->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFileDst == NULL ) {
			if ( hFileSrc != NULL ) {
				xrtClose(hFileSrc);
			}
			return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
		}
	}

	for ( iPos = 1; iPos <= objSrc->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objSrc->arrEntry, iPos);
		if ( pEntry == NULL ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat));
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}
		iRet = procXpkValidateLiveEntryLookup(objSrc, pEntry);
		if ( iRet != XPK_OK ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc)));
		}
		if ( (pEntry->iDataOffset + pEntry->iFileSize) > iSolidSrcSize ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat));
		}

		if ( pChunkBuild == NULL ) {
			pChunkBuild = xpkAllocInternal(XPK_WRITE_FILE_CHUNK_SIZE);
			if ( pChunkBuild == NULL ) {
				return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, XPK_ERR_MEMORY, sXpkErrorOutOfMemory));
			}
		}
		iRet = procXpkBuildCopyStoredPayload(objDst, hFileDst, objSrc, hFileSrc, XPK_HEAD_SIZE + iOffsetSolid, XPK_HEAD_SIZE + pEntry->iDataOffset, pEntry->iFileSize, pChunkBuild);
		if ( iRet != XPK_OK ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, iRet);
		}

		iRet = procXpkBuildAppendEntryOnly(objDst, pEntry, iOffsetSolid, pEntry->iFileSize, pEntry->iFileSize);
		if ( iRet != XPK_OK ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, iRet);
		}

		iOffsetSolid += pEntry->iFileSize;
	}

	if ( pChunkBuild != NULL ) {
		xpkFreeInternal(pChunkBuild);
	}
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}
	if ( hFileDst != NULL ) {
		xrtClose(hFileDst);
	}

	iRet = procXpkRebuildLookup(objDst);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	objDst->objHead.defComp = 0;
	objDst->bSolidApplied = TRUE;
	objDst->iAppendPos = XPK_HEAD_SIZE + iOffsetSolid;
	objDst->iFileSize = objDst->iAppendPos;
	objDst->bDirtyData = TRUE;
	objDst->bDirtyEntryTable = TRUE;
	objDst->bDirtyHead = TRUE;
	procXpkClearError(objDst);
	return XPK_OK;
}

static inline int procXpkBuildCopyLiveEntriesSolidStoreFromNormal(xpkObject objDst, xpkObject objSrc)
{
	uint32_t iPos;
	xpkEntry* pEntry;
	xpkWriteNode* pNode;
	xfile hFileSrc;
	xfile hFileDst;
	void* pData;
	void* pChunkBuild;
	uint64_t iDataSize;
	uint64_t iOffsetSolid;
	int iRet;

	hFileSrc = NULL;
	hFileDst = NULL;
	pChunkBuild = NULL;
	iOffsetSolid = 0;
	if ( !procXpkAppliedVolumeMode(objSrc) ) {
		hFileSrc = xrtOpen(objSrc->sPathPackage, TRUE, XRT_CP_BINARY);
		if ( hFileSrc == NULL ) {
			return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
		}
	}
	if ( !procXpkAppliedVolumeMode(objDst) ) {
		hFileDst = xrtOpen(objDst->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFileDst == NULL ) {
			if ( hFileSrc != NULL ) {
				xrtClose(hFileSrc);
			}
			return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
		}
	}

	for ( iPos = 1; iPos <= objSrc->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objSrc->arrEntry, iPos);
		if ( pEntry == NULL ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat));
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}
		iRet = procXpkValidateLiveEntryLookup(objSrc, pEntry);
		if ( iRet != XPK_OK ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc)));
		}

		pNode = procXpkFindWriteNode(objSrc, pEntry->iPos, NULL);
		if ( pNode != NULL && pNode->iLevel == 0 ) {
			if ( pNode->iCompSize != pNode->iRawSize || pNode->iRawSize != pEntry->iFileSize ) {
				return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat));
			}
			if ( pNode->pCompData == NULL && pNode->iCompSize > 0 ) {
				return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat));
			}
			iRet = procXpkBuildWriteAtChunked(objDst, hFileDst, XPK_HEAD_SIZE + iOffsetSolid, pNode->pCompData, pNode->iCompSize);
			iDataSize = pNode->iRawSize;
		} else if ( pNode == NULL && (pEntry->iFlag & XPK_FLAG_COMP_MASK) == 0 ) {
			if ( pEntry->iDataSize != pEntry->iFileSize ) {
				return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat));
			}
			if ( pEntry->iDataOffset < XPK_HEAD_SIZE ) {
				return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat));
			}
			if ( pEntry->iDataOffset > procXpkCurrentDataEnd(objSrc) ) {
				return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat));
			}
			if ( pEntry->iDataSize > (procXpkCurrentDataEnd(objSrc) - pEntry->iDataOffset) ) {
				return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat));
			}
			if ( pChunkBuild == NULL ) {
				pChunkBuild = xpkAllocInternal(XPK_WRITE_FILE_CHUNK_SIZE);
				if ( pChunkBuild == NULL ) {
					return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, XPK_ERR_MEMORY, sXpkErrorOutOfMemory));
				}
			}
			iRet = procXpkBuildCopyStoredPayload(objDst, hFileDst, objSrc, hFileSrc, XPK_HEAD_SIZE + iOffsetSolid, pEntry->iDataOffset, pEntry->iDataSize, pChunkBuild);
			iDataSize = pEntry->iFileSize;
		} else {
			pData = procXpkReadEntryDataWithFile(objSrc, pEntry, &iDataSize, hFileSrc);
			if ( pData == NULL ) {
				return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc)));
			}

			iRet = procXpkBuildWriteAtChunked(objDst, hFileDst, XPK_HEAD_SIZE + iOffsetSolid, pData, iDataSize);
			xpkFree(pData);
		}
		if ( iRet != XPK_OK ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, iRet);
		}

		iRet = procXpkBuildAppendEntryOnly(objDst, pEntry, iOffsetSolid, iDataSize, iDataSize);
		if ( iRet != XPK_OK ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, iRet);
		}

		iOffsetSolid += iDataSize;
	}

	if ( pChunkBuild != NULL ) {
		xpkFreeInternal(pChunkBuild);
	}
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}
	if ( hFileDst != NULL ) {
		xrtClose(hFileDst);
	}

	iRet = procXpkRebuildLookup(objDst);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	objDst->objHead.defComp = 0;
	objDst->bSolidApplied = TRUE;
	objDst->iAppendPos = XPK_HEAD_SIZE + iOffsetSolid;
	objDst->iFileSize = objDst->iAppendPos;
	objDst->bDirtyData = TRUE;
	objDst->bDirtyEntryTable = TRUE;
	objDst->bDirtyHead = TRUE;
	procXpkClearError(objDst);
	return XPK_OK;
}

static inline int procXpkBuildCopyLiveEntriesSolid(xpkObject objDst, xpkObject objSrc)
{
	uint32_t iPos;
	xpkEntry* pEntry;
	uint64_t iSolidRawSize64;
	uint64_t iSolidSrcSize;
	uint32_t iSolidSrcAlg;
	int bCanCopyDirect;
	int bCanStoreDirect;
	int iRet;

	iSolidSrcSize = 0;
	iSolidRawSize64 = 0;
	iSolidSrcAlg = procXpkCompLevelToAlg(procXpkSolidStoredCompLevel(objSrc));
	bCanCopyDirect = FALSE;
	bCanStoreDirect = FALSE;
	if ( !objSrc->bSolidApplied ) {
		iRet = procXpkBuildCheckSolidStoreDirect(objDst, objSrc, &bCanStoreDirect);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		if ( bCanStoreDirect ) {
			return procXpkBuildCopyLiveEntriesSolidStoreDirect(objDst, objSrc);
		}
		if ( procXpkSolidTargetCompLevel(objDst) == 0 ) {
			return procXpkBuildCopyLiveEntriesSolidStoreFromNormal(objDst, objSrc);
		}
	}
	if ( objSrc->bSolidApplied ) {
		iRet = procXpkBuildCheckSolidDirectCopy(objDst, objSrc, &iSolidSrcSize, &bCanCopyDirect);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		if ( bCanCopyDirect ) {
			return procXpkBuildCopySolidBlockDirect(objDst, objSrc);
		}
	}
	if ( objSrc->bSolidApplied && procXpkSolidStoredCompLevel(objSrc) == 0 && procXpkSolidTargetCompLevel(objDst) == 0 ) {
		return procXpkBuildCopyLiveEntriesSolidStoreCompact(objDst, objSrc);
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

		iSolidRawSize64 += pEntry->iFileSize;
	}
	if ( iSolidRawSize64 > UINT32_MAX ) {
		return procXpkSetError(objDst, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge);
	}
	if ( procXpkSolidTargetCompLevel(objDst) == 0 ) {
		return procXpkBuildCopyLiveEntriesSolidStoreStreaming(objDst, objSrc, iSolidRawSize64);
	}
	if ( procXpkCompLevelToAlg(procXpkSolidTargetCompLevel(objDst)) == XPK_ALG_LZ4
		|| procXpkCompLevelToAlg(procXpkSolidTargetCompLevel(objDst)) == XPK_ALG_LZ4HC ) {
		return procXpkBuildCopyLiveEntriesSolidLz4Streaming(objDst, objSrc, iSolidRawSize64);
	}
	if ( procXpkCompLevelToAlg(procXpkSolidTargetCompLevel(objDst)) == XPK_ALG_ZSTD ) {
		return procXpkBuildCopyLiveEntriesSolidZstdStreaming(objDst, objSrc, iSolidRawSize64);
	}
	if ( procXpkCompLevelToAlg(procXpkSolidTargetCompLevel(objDst)) == XPK_ALG_LZMA2 ) {
		return procXpkBuildCopyLiveEntriesSolidLzma2Streaming(objDst, objSrc, iSolidRawSize64);
	}

	return procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
}

static inline int procXpkBuildCopyLiveEntriesFromSolidSource(xpkObject objDst, xpkObject objSrc)
{
	uint32_t iPos;
	uint32_t iLiveCount;
	xpkEntry* pEntry;
	void* pData;
	uint64_t iDataSize;
	int iRet;

	iRet = procXpkVisibleEntryCountStrict(objSrc, &iLiveCount);
	if ( iRet != XPK_OK ) {
		return procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
	}
	if ( iLiveCount == 0 ) {
		return XPK_OK;
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

		pData = procXpkReadEntryData(objSrc, pEntry, &iDataSize);
		if ( pData == NULL ) {
			return procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
		}
		if ( iDataSize != pEntry->iFileSize ) {
			xpkFree(pData);
			return procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
		iRet = procXpkBuildCopyEntryData(objDst, pEntry, pData, iDataSize);
		xpkFree(pData);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
	}

	return XPK_OK;
}

static inline int procXpkBuildCopyLiveEntriesFromSolidStoredSource(xpkObject objDst, xpkObject objSrc)
{
	uint32_t iPos;
	uint32_t iLiveCount;
	xpkEntry* pEntry;
	xfile hFileSrc;
	xfile hFileDst;
	void* pChunkBuild;
	uint64_t iSolidRawSize;
	uint64_t iOffsetDst;
	int iRet;

	iRet = procXpkVisibleEntryCountStrict(objSrc, &iLiveCount);
	if ( iRet != XPK_OK ) {
		return procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
	}
	if ( iLiveCount == 0 ) {
		return XPK_OK;
	}

	iRet = procXpkCalcSolidStoredRawSize(objSrc, &iSolidRawSize);
	if ( iRet != XPK_OK ) {
		return procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
	}

	hFileSrc = NULL;
	hFileDst = NULL;
	pChunkBuild = NULL;
	if ( !procXpkAppliedVolumeMode(objSrc) ) {
		hFileSrc = xrtOpen(objSrc->sPathPackage, TRUE, XRT_CP_BINARY);
		if ( hFileSrc == NULL ) {
			return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
		}
	}
	if ( !procXpkAppliedVolumeMode(objDst) ) {
		hFileDst = xrtOpen(objDst->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFileDst == NULL ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen));
		}
	}

	for ( iPos = 1; iPos <= objSrc->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objSrc->arrEntry, iPos);
		if ( pEntry == NULL ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat));
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}
		iRet = procXpkValidateLiveEntryLookup(objSrc, pEntry);
		if ( iRet != XPK_OK ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc)));
		}
		if ( (pEntry->iFlag & XPK_FLAG_COMP_MASK) != 0 ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat));
		}
		if ( pEntry->iDataSize != pEntry->iFileSize ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat));
		}
		if ( (pEntry->iDataOffset + pEntry->iFileSize) > iSolidRawSize ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat));
		}

		iOffsetDst = objDst->iAppendPos;
		if ( pChunkBuild == NULL ) {
			pChunkBuild = xpkAllocInternal(XPK_WRITE_FILE_CHUNK_SIZE);
			if ( pChunkBuild == NULL ) {
				return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, XPK_ERR_MEMORY, sXpkErrorOutOfMemory));
			}
		}
		iRet = procXpkBuildCopyStoredPayload(objDst, hFileDst, objSrc, hFileSrc, iOffsetDst, XPK_HEAD_SIZE + pEntry->iDataOffset, pEntry->iFileSize, pChunkBuild);
		if ( iRet != XPK_OK ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, iRet);
		}

		iRet = procXpkBuildAppendCopiedEntry(objDst, pEntry, 0, iOffsetDst, pEntry->iFileSize, pEntry->iFileSize);
		if ( iRet != XPK_OK ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, iRet);
		}

		objDst->iAppendPos = iOffsetDst + pEntry->iFileSize;
		if ( objDst->iAppendPos > objDst->iFileSize ) {
			objDst->iFileSize = objDst->iAppendPos;
		}
		objDst->bDirtyData = TRUE;
		procXpkMarkDirtyEntryTable(objDst);
	}

	if ( pChunkBuild != NULL ) {
		xpkFreeInternal(pChunkBuild);
	}
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}
	if ( hFileDst != NULL ) {
		xrtClose(hFileDst);
	}

	procXpkClearError(objDst);
	return XPK_OK;
}

static inline int procXpkBuildCheckSolidSingleEntryDirect(xpkObject objDst, xpkObject objSrc, xpkEntry** pEntryRet, uint64_t* pCompSizeRet, int* pCanCopyRet)
{
	uint32_t iPos;
	uint32_t iLiveCount;
	xpkEntry* pEntry;
	uint64_t iSolidRawSize;
	int iRet;

	if ( pEntryRet != NULL ) {
		*pEntryRet = NULL;
	}
	if ( pCompSizeRet != NULL ) {
		*pCompSizeRet = 0;
	}
	if ( pCanCopyRet != NULL ) {
		*pCanCopyRet = FALSE;
	}
	if ( objDst == NULL || objSrc == NULL || pEntryRet == NULL || pCompSizeRet == NULL || pCanCopyRet == NULL ) {
		return procXpkReturnParamError(objDst);
	}
	if ( objDst->objHead.solidMode || !objSrc->bSolidApplied ) {
		return XPK_OK;
	}

	iRet = procXpkVisibleEntryCountStrict(objSrc, &iLiveCount);
	if ( iRet != XPK_OK ) {
		return procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
	}
	if ( iLiveCount != 1 ) {
		return XPK_OK;
	}

	pEntry = NULL;
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
		break;
	}

	if ( pEntry == NULL || procXpkEntryDeleted(pEntry) ) {
		return procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
	}
	if ( objSrc->objHead.dataOffset < XPK_HEAD_SIZE ) {
		return procXpkSetError(objDst, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	iRet = procXpkCalcSolidStoredRawSize(objSrc, &iSolidRawSize);
	if ( iRet != XPK_OK ) {
		return procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc));
	}
	if ( pEntry->iDataOffset != 0 || pEntry->iFileSize != iSolidRawSize ) {
		return XPK_OK;
	}

	*pEntryRet = pEntry;
	*pCompSizeRet = objSrc->objHead.dataOffset - XPK_HEAD_SIZE;
	*pCanCopyRet = TRUE;
	return XPK_OK;
}

static inline int procXpkBuildCopySingleSolidEntryDirect(xpkObject objDst, xpkObject objSrc)
{
	xpkEntry* pEntry;
	xfile hFileSrc;
	xfile hFileDst;
	uint64_t iCompSize;
	uint64_t iOffsetDst;
	uint8_t iCompLevel;
	int bCanCopy;
	int iRet;

	pEntry = NULL;
	iCompSize = 0;
	bCanCopy = FALSE;
	iRet = procXpkBuildCheckSolidSingleEntryDirect(objDst, objSrc, &pEntry, &iCompSize, &bCanCopy);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( !bCanCopy || pEntry == NULL ) {
		return procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat);
	}

	hFileSrc = NULL;
	hFileDst = NULL;
	if ( !procXpkAppliedVolumeMode(objSrc) ) {
		hFileSrc = xrtOpen(objSrc->sPathPackage, TRUE, XRT_CP_BINARY);
		if ( hFileSrc == NULL ) {
			return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
		}
	}
	if ( !procXpkAppliedVolumeMode(objDst) ) {
		hFileDst = xrtOpen(objDst->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFileDst == NULL ) {
			if ( hFileSrc != NULL ) {
				xrtClose(hFileSrc);
			}
			return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
		}
	}

	iOffsetDst = objDst->iAppendPos;
	iRet = procXpkBuildCopyStoredPayload(objDst, hFileDst, objSrc, hFileSrc, iOffsetDst, XPK_HEAD_SIZE, iCompSize, NULL);
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}
	if ( hFileDst != NULL ) {
		xrtClose(hFileDst);
	}
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	iCompLevel = procXpkSolidStoredCompLevel(objSrc);
	iRet = procXpkBuildAppendCopiedEntry(objDst, pEntry, iCompLevel, iOffsetDst, iCompSize, pEntry->iFileSize);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	iRet = procXpkRebuildLookup(objDst);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	objDst->iAppendPos = iOffsetDst + iCompSize;
	if ( objDst->iAppendPos > objDst->iFileSize ) {
		objDst->iFileSize = objDst->iAppendPos;
	}
	objDst->bDirtyData = TRUE;
	procXpkMarkDirtyEntryTable(objDst);
	procXpkClearError(objDst);
	return XPK_OK;
}

static inline int procXpkBuildCopyLiveEntries(xpkObject objDst, xpkObject objSrc)
{
	uint32_t iPos;
	xpkEntry* pEntry;
	xpkEntry* pEntryDirect;
	xpkWriteNode* pNode;
	xfile hFileSrc;
	xfile hFileDst;
	void* pChunkBuild;
	uint64_t iCompSizeDirect;
	int bCanCopyDirect;
	int iRet;

	if ( objDst->objHead.solidMode ) {
		return procXpkBuildCopyLiveEntriesSolid(objDst, objSrc);
	}
	if ( objSrc->bSolidApplied ) {
		pEntryDirect = NULL;
		iCompSizeDirect = 0;
		bCanCopyDirect = FALSE;
		iRet = procXpkBuildCheckSolidSingleEntryDirect(objDst, objSrc, &pEntryDirect, &iCompSizeDirect, &bCanCopyDirect);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		if ( bCanCopyDirect ) {
			return procXpkBuildCopySingleSolidEntryDirect(objDst, objSrc);
		}
		if ( procXpkSolidStoredCompLevel(objSrc) == 0 ) {
			return procXpkBuildCopyLiveEntriesFromSolidStoredSource(objDst, objSrc);
		}
		return procXpkBuildCopyLiveEntriesFromSolidSource(objDst, objSrc);
	}

	hFileSrc = NULL;
	hFileDst = NULL;
	pChunkBuild = NULL;
	if ( !procXpkAppliedVolumeMode(objDst) ) {
		hFileDst = xrtOpen(objDst->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFileDst == NULL ) {
			return procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen);
		}
	}

	for ( iPos = 1; iPos <= objSrc->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objSrc->arrEntry, iPos);
		if ( pEntry == NULL ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, XPK_ERR_STATE, sXpkErrorBadFormat));
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}
		iRet = procXpkValidateLiveEntryLookup(objSrc, pEntry);
		if ( iRet != XPK_OK ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, xpkLastError(objSrc), xpkLastErrorMessage(objSrc)));
		}

		pNode = procXpkFindWriteNode(objSrc, pEntry->iPos, NULL);
		if ( pNode == NULL && !procXpkAppliedVolumeMode(objSrc) && hFileSrc == NULL ) {
			hFileSrc = xrtOpen(objSrc->sPathPackage, TRUE, XRT_CP_BINARY);
			if ( hFileSrc == NULL ) {
				return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, XPK_ERR_IO, sXpkErrorIoOpen));
			}
		}
		if ( pNode == NULL && pChunkBuild == NULL ) {
			pChunkBuild = xpkAllocInternal(XPK_WRITE_FILE_CHUNK_SIZE);
			if ( pChunkBuild == NULL ) {
				return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, procXpkSetError(objDst, XPK_ERR_MEMORY, sXpkErrorOutOfMemory));
			}
		}

		iRet = procXpkBuildCopyEntryDirectWithFiles(objDst, objSrc, pEntry, hFileDst, hFileSrc, pChunkBuild);
		if ( iRet != XPK_OK ) {
			return procXpkBuildCleanupChunkFiles(pChunkBuild, hFileSrc, hFileDst, iRet);
		}
	}

	if ( pChunkBuild != NULL ) {
		xpkFreeInternal(pChunkBuild);
	}
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}
	if ( hFileDst != NULL ) {
		xrtClose(hFileDst);
	}

	iRet = procXpkRebuildLookup(objDst);
	if ( iRet != XPK_OK ) {
		return iRet;
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
	uint32_t iTry;

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.readonly = objXpk->bReadonly;
	objOpt.bufferedDefault = objXpk->bBufferedDefault;

	objReload = NULL;
	for ( iTry = 0; iTry < 8u; iTry++ ) {
		objReload = xpkOpen(objXpk->sPathPackage, &objOpt);
		if ( objReload != NULL ) {
			break;
		}
		if ( iTry + 1u < 8u ) {
			xrtSleep(10);
		}
	}
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
	char* sFinalPath;
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
	sFinalPath = NULL;

	iRet = procXpkBuildCopyConfig(objBuild, objXpk);
	if ( iRet == XPK_OK ) {
		iRet = procXpkBuildCopyLiveEntries(objBuild, objXpk);
	}
	if ( iRet == XPK_OK ) {
		iRet = procXpkSavePackage(objBuild);
	}
	if ( iRet != XPK_OK ) {
		sBuildText = xpkLastErrorMessage(objBuild);
		if ( sBuildText[0] == '\0' ) {
			sBuildText = xpkLastErrorMessage(objXpk);
		}
		if ( sBuildText[0] == '\0' ) {
			sBuildText = xpkLastErrorMessage(NULL);
		}
		if ( sBuildText[0] == '\0' ) {
			sBuildText = (iRet == XPK_ERR_IO) ? sXpkErrorIoWrite : sXpkErrorInvalidParam;
		}
		iBuildTextSize = strlen(sBuildText);
		if ( iBuildTextSize >= XPK_ERROR_TEXT_CAP ) {
			iBuildTextSize = XPK_ERROR_TEXT_CAP - 1;
		}
		memcpy(sBuildError, sBuildText, iBuildTextSize);
		sBuildError[iBuildTextSize] = '\0';
	}

	if ( iRet != XPK_OK ) {
		xpkClose(objBuild);
		(void)procXpkBuildResetTempPath(objXpk, sTempPath);
		xpkFreeInternal(sTempPath);
		return procXpkSetError(objXpk, iRet, sBuildError);
	}

	if ( !bReplaceOriginal ) {
		xpkClose(objBuild);
		xpkFreeInternal(sTempPath);
		procXpkClearError(objXpk);
		return XPK_OK;
	}

	if ( procXpkMoveVolumeFilesText(objXpk, sTempPath, objXpk->sPathPackage) != XPK_OK ) {
		iRet = xpkLastError(objXpk);
		sBuildText = xpkLastErrorMessage(objXpk);
		if ( sBuildText[0] == '\0' ) {
			sBuildText = xpkLastErrorMessage(NULL);
		}
		if ( sBuildText[0] == '\0' ) {
			sBuildText = (iRet == XPK_ERR_IO) ? sXpkErrorIoWrite : sXpkErrorInvalidParam;
		}
		iReplaceTextSize = strlen(sBuildText);
		if ( iReplaceTextSize >= XPK_ERROR_TEXT_CAP ) {
			iReplaceTextSize = XPK_ERROR_TEXT_CAP - 1;
		}
		memcpy(sReplaceError, sBuildText, iReplaceTextSize);
		sReplaceError[iReplaceTextSize] = '\0';
		xpkClose(objBuild);
		(void)procXpkBuildResetTempPath(objXpk, sTempPath);
		xpkFreeInternal(sTempPath);
		return procXpkSetError(objXpk, iRet, sReplaceError);
	}

	sFinalPath = procXpkDupText(objXpk->sPathPackage);
	if ( sFinalPath == NULL ) {
		xpkClose(objBuild);
		xpkFreeInternal(sTempPath);
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	xpkFreeInternal(objBuild->sPathPackage);
	objBuild->sPathPackage = sFinalPath;
	sFinalPath = NULL;
	objBuild->err.iCode = XPK_OK;
	objBuild->err.sText[0] = '\0';

	procXpkUnitWriteQueue(objXpk);
	procXpkUnitObject(objXpk);
	*objXpk = *objBuild;
	xpkFreeInternal(objBuild);

	xpkFreeInternal(sTempPath);
	procXpkClearError(objXpk);
	return XPK_OK;
}
