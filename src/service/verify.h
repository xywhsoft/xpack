/*
	xPack 校验与统计模块

	负责条目校验、整包校验以及当前统计信息计算。
*/

// 哈希映射文件范围
static inline int procXpkHashMappedFileRange(xpkObject objXpk, xfile hFile, uint64_t iOffset, uint64_t iSize, uint32_t* pHashRet)
{
	xpkMappedFile objMap;
	uint64_t iFileSize;

	if ( objXpk == NULL || hFile == NULL || pHashRet == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	iFileSize = xrtGetEOF(hFile);
	if ( iOffset > iFileSize || iSize > (iFileSize - iOffset) ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	memset(&objMap, 0, sizeof(objMap));
	if ( procXpkMapFileReadOnly(objXpk, hFile, iFileSize, &objMap) != XPK_OK ) {
		return xpkLastError(objXpk);
	}

	*pHashRet = (iSize > 0) ? xpkHash32Internal((const uint8_t*)objMap.pView + iOffset, iSize) : 0;
	procXpkUnmapFile(&objMap);
	procXpkClearError(objXpk);
	return XPK_OK;
}

// 校验哈希匹配
static inline int procXpkVerifyHashMatch(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint64_t iSize)
{
	uint32_t iHash;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( pData == NULL && iSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	iHash = (iSize > 0) ? xpkHash32Internal(pData, iSize) : 0;
	if ( iHash != pEntry->iFileHash ) {
		return procXpkSetError(objXpk, XPK_ERR_HASH, sXpkErrorHashMismatch);
	}

	procXpkClearError(objXpk);
	return XPK_OK;
}

// 校验 Solid 条目
static inline int procXpkVerifySolidEntry(xpkObject objXpk, xpkEntry* pEntry)
{
	void* pData;
	xfile hFile;
	uint32_t iHash;
	uint32_t iAlg;
	uint64_t iSize;
	uint64_t iSolidSize;
	int iRet;

	if ( procXpkSolidStoredCompLevel(objXpk) == 0 ) {
		if ( !procXpkAppliedVolumeMode(objXpk) ) {
			iRet = procXpkCalcSolidStoredRawSize(objXpk, &iSolidSize);
			if ( iRet != XPK_OK ) {
				return iRet;
			}
			if ( (pEntry->iDataOffset + pEntry->iFileSize) > iSolidSize ) {
				return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
			}
			hFile = xrtOpen(objXpk->sPathPackage, TRUE, XRT_CP_BINARY);
			if ( hFile == NULL ) {
				return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
			}
			iRet = procXpkHashMappedFileRange(objXpk, hFile, XPK_HEAD_SIZE + pEntry->iDataOffset, pEntry->iFileSize, &iHash);
			xrtClose(hFile);
			if ( iRet != XPK_OK ) {
				return iRet;
			}
			if ( iHash != pEntry->iFileHash ) {
				return procXpkSetError(objXpk, XPK_ERR_HASH, sXpkErrorHashMismatch);
			}
			procXpkClearError(objXpk);
			return XPK_OK;
		}
		if ( pEntry->iFileSize > UINT32_MAX ) {
			return procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge);
		}
		pData = procXpkReadSolidEntryData(objXpk, pEntry, &iSize);
		if ( pData == NULL ) {
			return xpkLastError(objXpk);
		}
		iRet = procXpkVerifyHashMatch(objXpk, pEntry, pData, iSize);
		xpkFree(pData);
		return iRet;
	}

	iAlg = procXpkCompLevelToAlg(procXpkSolidStoredCompLevel(objXpk));
	if ( iAlg == XPK_ALG_LZ4 || iAlg == XPK_ALG_LZ4HC || iAlg == XPK_ALG_ZSTD || iAlg == XPK_ALG_LZMA2 ) {
		pData = procXpkReadSolidEntryData(objXpk, pEntry, &iSize);
		if ( pData == NULL ) {
			return xpkLastError(objXpk);
		}
		iRet = procXpkVerifyHashMatch(objXpk, pEntry, pData, iSize);
		xpkFree(pData);
		return iRet;
	}

	return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
}

// 校验原样条目
static inline int procXpkVerifyStoredEntry(xpkObject objXpk, xpkEntry* pEntry)
{
	return procXpkVerifyStoredEntryWithFile(objXpk, pEntry, NULL);
}

// 校验原样条目带文件
static inline int procXpkVerifyStoredEntryWithFile(xpkObject objXpk, xpkEntry* pEntry, xfile hFile)
{
	xpkWriteNode* pNode;
	xfile hFileLocal;
	xpkMappedFile objMap;
	uint64_t iDataEnd;
	int iRet;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	pNode = procXpkFindWriteNode(objXpk, pEntry->iPos, NULL);
	if ( pNode != NULL ) {
		if ( pNode->iLevel != 0 ) {
			return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		}
		if ( pNode->iRawSize != pEntry->iFileSize || pNode->iCompSize != pEntry->iFileSize ) {
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
		if ( pNode->pCompData == NULL && pNode->iCompSize > 0 ) {
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
		return procXpkVerifyHashMatch(objXpk, pEntry, pNode->pCompData, pNode->iCompSize);
	}

	if ( pEntry->iDataSize != pEntry->iFileSize ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	iDataEnd = procXpkCurrentDataEnd(objXpk);
	if ( pEntry->iDataOffset < XPK_HEAD_SIZE ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pEntry->iDataOffset > iDataEnd ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pEntry->iDataSize > (iDataEnd - pEntry->iDataOffset) ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( procXpkAppliedVolumeMode(objXpk) ) {
		void* pCompData;

		if ( pEntry->iDataSize > UINT32_MAX ) {
			return procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge);
		}

		pCompData = NULL;
		iRet = procXpkReadAtAlloc(objXpk, hFile, pEntry->iDataOffset, (uint32_t)pEntry->iDataSize, &pCompData);
		if ( iRet != XPK_OK ) {
			return iRet;
		}

		iRet = procXpkVerifyHashMatch(objXpk, pEntry, pCompData, pEntry->iDataSize);
		xpkFreeInternal(pCompData);
		return iRet;
	}

	hFileLocal = NULL;
	if ( hFile == NULL ) {
		hFileLocal = xrtOpen(objXpk->sPathPackage, TRUE, XRT_CP_BINARY);
		if ( hFileLocal == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
		}
		hFile = hFileLocal;
	}

	memset(&objMap, 0, sizeof(objMap));
	iRet = procXpkMapFileReadOnly(objXpk, hFile, xrtGetEOF(hFile), &objMap);
	if ( iRet != XPK_OK ) {
		if ( hFileLocal != NULL ) {
			xrtClose(hFileLocal);
		}
		return iRet;
	}

	iRet = procXpkVerifyStoredEntryMapped(objXpk, pEntry, &objMap);
	procXpkUnmapFile(&objMap);
	if ( hFileLocal != NULL ) {
		xrtClose(hFileLocal);
	}
	return iRet;
}

// 校验原样条目映射
static inline int procXpkVerifyStoredEntryMapped(xpkObject objXpk, xpkEntry* pEntry, const xpkMappedFile* pMap)
{
	uint64_t iDataEnd;

	if ( objXpk == NULL || pEntry == NULL || pMap == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( pMap->pView == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}
	if ( pEntry->iDataSize != pEntry->iFileSize ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	iDataEnd = procXpkCurrentDataEnd(objXpk);
	if ( pEntry->iDataOffset < XPK_HEAD_SIZE ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pEntry->iDataOffset > iDataEnd ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pEntry->iDataSize > (iDataEnd - pEntry->iDataOffset) ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pEntry->iDataOffset > pMap->iSize || pEntry->iDataSize > (pMap->iSize - pEntry->iDataOffset) ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	return procXpkVerifyHashMatch(objXpk, pEntry, (const uint8_t*)pMap->pView + pEntry->iDataOffset, pEntry->iDataSize);
}

// 基于文件句柄校验解码条目
static inline int procXpkVerifyDecodedEntryWithFile(xpkObject objXpk, xpkEntry* pEntry, xfile hFile)
{
	xpkWriteNode* pNode;
	xfile hFileLocal;
	xpkMappedFile objMap;
	void* pData;
	uint64_t iSize;
	int iRet;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	pNode = procXpkFindWriteNode(objXpk, pEntry->iPos, NULL);
	if ( pNode == NULL && !procXpkAppliedVolumeMode(objXpk) ) {
		hFileLocal = NULL;
		if ( hFile == NULL ) {
			hFileLocal = xrtOpen(objXpk->sPathPackage, TRUE, XRT_CP_BINARY);
			if ( hFileLocal == NULL ) {
				return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
			}
			hFile = hFileLocal;
		}

		memset(&objMap, 0, sizeof(objMap));
		iRet = procXpkMapFileReadOnly(objXpk, hFile, xrtGetEOF(hFile), &objMap);
		if ( iRet != XPK_OK ) {
			if ( hFileLocal != NULL ) {
				xrtClose(hFileLocal);
			}
			return iRet;
		}

		iRet = procXpkVerifyDecodedEntryMapped(objXpk, pEntry, &objMap);
		procXpkUnmapFile(&objMap);
		if ( hFileLocal != NULL ) {
			xrtClose(hFileLocal);
		}
		return iRet;
	}

	pData = procXpkReadEntryDataWithFile(objXpk, pEntry, &iSize, hFile);
	if ( pData == NULL ) {
		return xpkLastError(objXpk);
	}
	iRet = procXpkVerifyHashMatch(objXpk, pEntry, pData, iSize);
	xpkFree(pData);
	return iRet;
}

// 基于映射校验解码条目
static inline int procXpkVerifyDecodedEntryMapped(xpkObject objXpk, xpkEntry* pEntry, const xpkMappedFile* pMap)
{
	void* pData;
	uint64_t iDataEnd;
	int iRet;

	if ( objXpk == NULL || pEntry == NULL || pMap == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( pMap->pView == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}
	if ( pEntry->iDataSize > UINT32_MAX || pEntry->iFileSize > UINT32_MAX ) {
		return procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge);
	}
	iDataEnd = procXpkCurrentDataEnd(objXpk);
	if ( pEntry->iDataOffset < XPK_HEAD_SIZE ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pEntry->iDataOffset > iDataEnd ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pEntry->iDataSize > (iDataEnd - pEntry->iDataOffset) ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pEntry->iDataOffset > pMap->iSize || pEntry->iDataSize > (pMap->iSize - pEntry->iDataOffset) ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	pData = NULL;
	iRet = procXpkCodecDecode(objXpk, (uint8_t)(pEntry->iFlag & XPK_FLAG_COMP_MASK), (const uint8_t*)pMap->pView + pEntry->iDataOffset, (uint32_t)pEntry->iDataSize, (uint32_t)pEntry->iFileSize, &pData);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	iRet = procXpkVerifyHashMatch(objXpk, pEntry, pData, pEntry->iFileSize);
	xpkFree(pData);
	return iRet;
}

// 校验全部确保数据文件
static inline int procXpkVerifyAllEnsureDataFile(xpkObject objXpk, xfile* pFileRet)
{
	if ( objXpk == NULL || pFileRet == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( procXpkAppliedVolumeMode(objXpk) ) {
		return XPK_OK;
	}
	if ( *pFileRet != NULL ) {
		return XPK_OK;
	}

	*pFileRet = xrtOpen(objXpk->sPathPackage, TRUE, XRT_CP_BINARY);
	if ( *pFileRet == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	return XPK_OK;
}

// 校验条目
static inline int procXpkVerifyEntry(xpkObject objXpk, xpkEntry* pEntry)
{
	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( procXpkEntryDeleted(pEntry) ) {
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorDeleted);
	}
	if ( objXpk->bSolidApplied ) {
		return procXpkVerifySolidEntry(objXpk, pEntry);
	}
	if ( (pEntry->iFlag & XPK_FLAG_COMP_MASK) == 0 ) {
		return procXpkVerifyStoredEntry(objXpk, pEntry);
	}

	return procXpkVerifyDecodedEntryWithFile(objXpk, pEntry, NULL);
}

// 逐条校验全部 Solid 压缩条目
static inline int procXpkVerifyAllSolidCompressedEntriesByEntry(xpkObject objXpk)
{
	uint32_t iPos;
	uint32_t iLiveCount;
	xpkEntry* pEntry;
	int iRet;

	iRet = procXpkVisibleEntryCountStrict(objXpk, &iLiveCount);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( iLiveCount == 0 ) {
		procXpkClearError(objXpk);
		return XPK_OK;
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
		iRet = procXpkVerifySolidEntry(objXpk, pEntry);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
	}

	procXpkClearError(objXpk);
	return XPK_OK;
}

// 基于映射校验全部 Solid 原样条目
static inline int procXpkVerifyAllSolidStoredEntriesMapped(xpkObject objXpk)
{
	uint32_t iPos;
	uint32_t iLiveCount;
	xpkEntry* pEntry;
	xfile hFile;
	xpkMappedFile objMap;
	uint64_t iRawSize;
	uint64_t iFileSize;
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
	if ( procXpkAppliedVolumeMode(objXpk) ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}
	iRet = procXpkCalcSolidStoredRawSize(objXpk, &iRawSize);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	hFile = xrtOpen(objXpk->sPathPackage, TRUE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}
	iFileSize = xrtGetEOF(hFile);
	if ( objXpk->objHead.dataOffset > iFileSize ) {
		xrtClose(hFile);
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	memset(&objMap, 0, sizeof(objMap));
	iRet = procXpkMapFileReadOnly(objXpk, hFile, iFileSize, &objMap);
	if ( iRet != XPK_OK ) {
		xrtClose(hFile);
		return iRet;
	}

	for ( iPos = 1; iPos <= objXpk->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, iPos);
		if ( pEntry == NULL ) {
			procXpkUnmapFile(&objMap);
			xrtClose(hFile);
			return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}
		iRet = procXpkValidateLiveEntryLookup(objXpk, pEntry);
		if ( iRet != XPK_OK ) {
			procXpkUnmapFile(&objMap);
			xrtClose(hFile);
			return iRet;
		}
		if ( (pEntry->iDataOffset + pEntry->iFileSize) > iRawSize ) {
			procXpkUnmapFile(&objMap);
			xrtClose(hFile);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}

		iHash = (pEntry->iFileSize > 0) ? xpkHash32Internal((const uint8_t*)objMap.pView + XPK_HEAD_SIZE + pEntry->iDataOffset, pEntry->iFileSize) : 0;
		if ( iHash != pEntry->iFileHash ) {
			procXpkUnmapFile(&objMap);
			xrtClose(hFile);
			return procXpkSetError(objXpk, XPK_ERR_HASH, sXpkErrorHashMismatch);
		}
	}

	procXpkUnmapFile(&objMap);
	xrtClose(hFile);
	procXpkClearError(objXpk);
	return XPK_OK;
}

// 逐条校验全部 Solid 原样条目
static inline int procXpkVerifyAllSolidStoredEntriesByEntry(xpkObject objXpk)
{
	uint32_t iPos;
	uint32_t iLiveCount;
	xpkEntry* pEntry;
	int iRet;

	iRet = procXpkVisibleEntryCountStrict(objXpk, &iLiveCount);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( iLiveCount == 0 ) {
		procXpkClearError(objXpk);
		return XPK_OK;
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
		iRet = procXpkVerifySolidEntry(objXpk, pEntry);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
	}

	procXpkClearError(objXpk);
	return XPK_OK;
}

// 校验全部条目
static inline int procXpkVerifyAllEntries(xpkObject objXpk)
{
	uint32_t iPos;
	xpkEntry* pEntry;
	xpkWriteNode* pNode;
	xfile hFileData;
	xpkMappedFile objMapData;
	int iRet;

	// 先处理对象状态和 solid 专用校验路径，这些分支能避免通用逐条遍历。
	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( procXpkValidateEntryCountState(objXpk) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( objXpk->bSolidApplied ) {
		if ( procXpkSolidStoredCompLevel(objXpk) == 0 ) {
			if ( !procXpkAppliedVolumeMode(objXpk) ) {
				return procXpkVerifyAllSolidStoredEntriesMapped(objXpk);
			}
			return procXpkVerifyAllSolidStoredEntriesByEntry(objXpk);
		}
		if (
			procXpkCompLevelToAlg(procXpkSolidStoredCompLevel(objXpk)) == XPK_ALG_LZ4 ||
			procXpkCompLevelToAlg(procXpkSolidStoredCompLevel(objXpk)) == XPK_ALG_LZ4HC ||
			procXpkCompLevelToAlg(procXpkSolidStoredCompLevel(objXpk)) == XPK_ALG_ZSTD ||
			procXpkCompLevelToAlg(procXpkSolidStoredCompLevel(objXpk)) == XPK_ALG_LZMA2
		) {
			return procXpkVerifyAllSolidCompressedEntriesByEntry(objXpk);
		}
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}
	hFileData = NULL;
	memset(&objMapData, 0, sizeof(objMapData));

	// 普通布局下复用同一个数据文件句柄和映射视图，避免每个条目都重复打开文件。
	for ( iPos = 1; iPos <= objXpk->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, iPos);
		if ( pEntry == NULL ) {
			procXpkUnmapFile(&objMapData);
			if ( hFileData != NULL ) {
				xrtClose(hFileData);
			}
			return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}
		iRet = procXpkValidateLiveEntryLookup(objXpk, pEntry);
		if ( iRet != XPK_OK ) {
			procXpkUnmapFile(&objMapData);
			if ( hFileData != NULL ) {
				xrtClose(hFileData);
			}
			return iRet;
		}

		pNode = procXpkFindWriteNode(objXpk, pEntry->iPos, NULL);
		if ( pNode == NULL ) {
			iRet = procXpkVerifyAllEnsureDataFile(objXpk, &hFileData);
			if ( iRet != XPK_OK ) {
				procXpkUnmapFile(&objMapData);
				if ( hFileData != NULL ) {
					xrtClose(hFileData);
				}
				return iRet;
			}
		}

		if ( pNode == NULL && !procXpkAppliedVolumeMode(objXpk) && objMapData.pView == NULL ) {
			iRet = procXpkMapFileReadOnly(objXpk, hFileData, xrtGetEOF(hFileData), &objMapData);
			if ( iRet != XPK_OK ) {
				if ( hFileData != NULL ) {
					xrtClose(hFileData);
				}
				return iRet;
			}
		}

		// 按条目当前存储形态选择原样校验或解码校验路径。
		if ( (pEntry->iFlag & XPK_FLAG_COMP_MASK) == 0 ) {
			if ( pNode == NULL && objMapData.pView != NULL ) {
				iRet = procXpkVerifyStoredEntryMapped(objXpk, pEntry, &objMapData);
			} else {
				iRet = procXpkVerifyStoredEntryWithFile(objXpk, pEntry, hFileData);
			}
		} else {
			if ( pNode == NULL && objMapData.pView != NULL ) {
				iRet = procXpkVerifyDecodedEntryMapped(objXpk, pEntry, &objMapData);
			} else {
				iRet = procXpkVerifyDecodedEntryWithFile(objXpk, pEntry, hFileData);
			}
		}
		if ( iRet != XPK_OK ) {
			procXpkUnmapFile(&objMapData);
			if ( hFileData != NULL ) {
				xrtClose(hFileData);
			}
			return iRet;
		}
	}

	// 校验成功后统一释放共享资源并清理错误状态。
	procXpkUnmapFile(&objMapData);
	if ( hFileData != NULL ) {
		xrtClose(hFileData);
	}
	procXpkClearError(objXpk);
	return XPK_OK;
}

// 计算当前元数据字节数
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

// 计算当前条目字节数
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

// 统计当前
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
