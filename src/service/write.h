#ifndef XPK_SERVICE_WRITE_H
#define XPK_SERVICE_WRITE_H

typedef struct xpkWriteNode {
	uint32_t iPos;
	uint8_t iLevel;
	uint8_t reserved0[3];
	uint32_t iCompSize;
	uint64_t iRawSize;
	void* pCompData;
} xpkWriteNode;

struct xpkWriteQueue {
	xarray_struct arrNode;
};

#define XPK_WRITE_FILE_CHUNK_SIZE (8u * 1024u * 1024u)

static inline int procXpkWriteQueueCount(xpkObject objXpk)
{
	if ( (objXpk == NULL) || (objXpk->pWriteQueue == NULL) ) {
		return 0;
	}
	return (int)objXpk->pWriteQueue->arrNode.Count;
}

static inline int procXpkInitWriteQueue(xpkObject objXpk)
{
	if ( objXpk->pWriteQueue != NULL ) {
		return XPK_OK;
	}

	objXpk->pWriteQueue = (xpkWriteQueue*)xpkAllocInternal(sizeof(*objXpk->pWriteQueue));
	if ( objXpk->pWriteQueue == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	memset(objXpk->pWriteQueue, 0, sizeof(*objXpk->pWriteQueue));
	xrtArrayInit(&objXpk->pWriteQueue->arrNode, sizeof(xpkWriteNode), XRT_OBJMODE_LOCAL);
	return XPK_OK;
}

static inline void procXpkUnitWriteQueue(xpkObject objXpk)
{
	uint32_t iPos;
	xpkWriteNode* pNode;

	if ( (objXpk == NULL) || (objXpk->pWriteQueue == NULL) ) {
		return;
	}

	for ( iPos = 1; iPos <= objXpk->pWriteQueue->arrNode.Count; iPos++ ) {
		pNode = (xpkWriteNode*)xrtArrayGet(&objXpk->pWriteQueue->arrNode, iPos);
		if ( (pNode != NULL) && (pNode->pCompData != NULL) ) {
			xpkFreeInternal(pNode->pCompData);
			pNode->pCompData = NULL;
		}
	}

	xrtArrayUnit(&objXpk->pWriteQueue->arrNode);
	xpkFreeInternal(objXpk->pWriteQueue);
	objXpk->pWriteQueue = NULL;
}

static inline xpkWriteNode* procXpkFindWriteNode(xpkObject objXpk, uint32_t iPos, uint32_t* pNodePosRet)
{
	uint32_t iNodePos;
	xpkWriteNode* pNode;

	if ( pNodePosRet != NULL ) {
		*pNodePosRet = 0;
	}
	if ( (objXpk == NULL) || (objXpk->pWriteQueue == NULL) ) {
		return NULL;
	}

	for ( iNodePos = 1; iNodePos <= objXpk->pWriteQueue->arrNode.Count; iNodePos++ ) {
		pNode = (xpkWriteNode*)xrtArrayGet(&objXpk->pWriteQueue->arrNode, iNodePos);
		if ( (pNode != NULL) && (pNode->iPos == iPos) ) {
			if ( pNodePosRet != NULL ) {
				*pNodePosRet = iNodePos;
			}
			return pNode;
		}
	}

	return NULL;
}

static inline void procXpkRemoveQueuedWriteAt(xpkObject objXpk, uint32_t iNodePos)
{
	xpkWriteNode* pNode;

	if ( (objXpk == NULL) || (objXpk->pWriteQueue == NULL) || (iNodePos == 0) ) {
		return;
	}

	pNode = (xpkWriteNode*)xrtArrayGet(&objXpk->pWriteQueue->arrNode, iNodePos);
	if ( (pNode != NULL) && (pNode->pCompData != NULL) ) {
		xpkFreeInternal(pNode->pCompData);
		pNode->pCompData = NULL;
	}
	xrtArrayRemove(&objXpk->pWriteQueue->arrNode, iNodePos, 1);
}

static inline void procXpkDropQueuedWrite(xpkObject objXpk, uint32_t iPos)
{
	uint32_t iNodePos;

	procXpkFindWriteNode(objXpk, iPos, &iNodePos);
	if ( iNodePos != 0 ) {
		procXpkRemoveQueuedWriteAt(objXpk, iNodePos);
	}
}

static inline void procXpkShiftQueuedWritePos(xpkObject objXpk, uint32_t iPosRemoved)
{
	uint32_t iNodePos;
	xpkWriteNode* pNode;

	if ( (objXpk == NULL) || (objXpk->pWriteQueue == NULL) ) {
		return;
	}

	for ( iNodePos = 1; iNodePos <= objXpk->pWriteQueue->arrNode.Count; iNodePos++ ) {
		pNode = (xpkWriteNode*)xrtArrayGet(&objXpk->pWriteQueue->arrNode, iNodePos);
		if ( (pNode != NULL) && (pNode->iPos > iPosRemoved) ) {
			pNode->iPos--;
		}
	}
}

static inline int procXpkQueueWrite(xpkObject objXpk, uint32_t iPos, uint8_t iLevel, void* pCompData, uint32_t iCompSize, uint64_t iRawSize)
{
	uint32_t iNodePos;
	xpkWriteNode* pNode;

	if ( pCompData == NULL && iCompSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	if ( procXpkInitWriteQueue(objXpk) != XPK_OK ) {
		if ( pCompData != NULL ) {
			xpkFreeInternal(pCompData);
		}
		return XPK_ERR_MEMORY;
	}

	pNode = procXpkFindWriteNode(objXpk, iPos, &iNodePos);
	if ( pNode == NULL ) {
		iNodePos = xrtArrayAppend(&objXpk->pWriteQueue->arrNode, 1);
		if ( iNodePos == 0 ) {
			if ( pCompData != NULL ) {
				xpkFreeInternal(pCompData);
			}
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
		pNode = (xpkWriteNode*)xrtArrayGet(&objXpk->pWriteQueue->arrNode, iNodePos);
		if ( pNode == NULL ) {
			if ( pCompData != NULL ) {
				xpkFreeInternal(pCompData);
			}
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
		memset(pNode, 0, sizeof(*pNode));
		pNode->iPos = iPos;
	} else if ( pNode->pCompData != NULL ) {
		xpkFreeInternal(pNode->pCompData);
	}

	pNode->iLevel = iLevel;
	pNode->iCompSize = iCompSize;
	pNode->iRawSize = iRawSize;
	pNode->pCompData = pCompData;
	return XPK_OK;
}

static inline int procXpkResolveCompLevel(xpkObject objXpk, const xpkWriteOptions* pOpt, uint8_t* pLevelRet)
{
	uint8_t iLevel;

	if ( pLevelRet == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	if ( pOpt == NULL ) {
		iLevel = (uint8_t)objXpk->objHead.defComp;
	} else if ( pOpt->compLevel == 0xFFu ) {
		iLevel = (uint8_t)objXpk->objHead.defComp;
	} else {
		iLevel = pOpt->compLevel;
	}

	if ( iLevel > 15 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	*pLevelRet = iLevel;
	return XPK_OK;
}

static inline uint8_t procXpkResolveWritePolicy(xpkObject objXpk, const xpkWriteOptions* pOpt)
{
	if ( pOpt == NULL ) {
		return objXpk->bBufferedDefault ? XPK_WRITE_BUFFERED : XPK_WRITE_IMMEDIATE;
	}
	return pOpt->writePolicy;
}

static inline int procXpkLoadFileData(xpkObject objXpk, const char* sPathFile, void** pDataRet, uint64_t* pSizeRet)
{
	xfile hFile;
	uint64_t iFileSize;
	size_t iReadSize;
	void* pData;

	if ( pDataRet != NULL ) {
		*pDataRet = NULL;
	}
	if ( pSizeRet != NULL ) {
		*pSizeRet = 0;
	}
	if ( sPathFile == NULL || sPathFile[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	hFile = xrtOpen((str)sPathFile, TRUE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	iFileSize = xrtGetEOF(hFile);
	if ( iFileSize > UINT32_MAX ) {
		xrtClose(hFile);
		return procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge);
	}

	pData = xrtRead(hFile, (size_t)iFileSize, &iReadSize);
	xrtClose(hFile);
	if ( (iFileSize > 0) && ((pData == NULL) || (iReadSize != iFileSize)) ) {
		if ( pData != NULL ) {
			xrtFree(pData);
		}
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoRead);
	}

	if ( pDataRet != NULL ) {
		*pDataRet = pData;
	}
	if ( pSizeRet != NULL ) {
		*pSizeRet = iFileSize;
	}
	return XPK_OK;
}

static inline int procXpkWriteFileData(xpkObject objXpk, const char* sPathFile, const void* pData, uint64_t iSize)
{
	xfile hFile;
	size_t iChunkSize;
	const uint8_t* pCur;
	uint64_t iSizeLeft;

	if ( sPathFile == NULL || sPathFile[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( pData == NULL && iSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	hFile = xrtOpen((str)sPathFile, FALSE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	pCur = (const uint8_t*)pData;
	iSizeLeft = iSize;
	while ( iSizeLeft > 0 ) {
		size_t iWrite;

		iChunkSize = (iSizeLeft > XPK_WRITE_FILE_CHUNK_SIZE) ? XPK_WRITE_FILE_CHUNK_SIZE : (size_t)iSizeLeft;
		iWrite = xrtWrite(hFile, (str)pCur, iChunkSize);
		if ( iWrite != iChunkSize ) {
			xrtClose(hFile);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		}
		pCur += iChunkSize;
		iSizeLeft -= iChunkSize;
	}
	if ( !xrtSetEOF(hFile) ) {
		xrtClose(hFile);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
	}

	xrtClose(hFile);
	procXpkClearError(objXpk);
	return XPK_OK;
}

static inline int procXpkCopyStoredEntryToFile(xpkObject objXpk, xpkEntry* pEntry, const char* sPathFile)
{
	xfile hFileSrc;
	xfile hFileDst;
	void* pChunk;
	uint64_t iOffsetCur;
	uint64_t iSizeLeft;
	uint32_t iChunkSize;
	uint32_t iLevel;
	int iRet;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( sPathFile == NULL || sPathFile[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( procXpkEntryDeleted(pEntry) ) {
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorDeleted);
	}
	if ( objXpk->bSolidApplied ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}
	if ( procXpkFindWriteNode(objXpk, pEntry->iPos, NULL) != NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBufferedPending);
	}

	iLevel = (uint32_t)(pEntry->iFlag & XPK_FLAG_COMP_MASK);
	if ( iLevel != 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}
	if ( pEntry->iDataSize != pEntry->iFileSize ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pEntry->iDataOffset < XPK_HEAD_SIZE ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pEntry->iDataOffset > objXpk->objHead.dataOffset ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pEntry->iDataSize > (objXpk->objHead.dataOffset - pEntry->iDataOffset) ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	hFileDst = xrtOpen((str)sPathFile, FALSE, XRT_CP_BINARY);
	if ( hFileDst == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	hFileSrc = NULL;
	if ( !procXpkAppliedVolumeMode(objXpk) ) {
		hFileSrc = xrtOpen(objXpk->sPathPackage, TRUE, XRT_CP_BINARY);
		if ( hFileSrc == NULL ) {
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
		}
	}

	iOffsetCur = pEntry->iDataOffset;
	iSizeLeft = pEntry->iDataSize;
	while ( iSizeLeft > 0 ) {
		size_t iWrite;

		iChunkSize = (iSizeLeft > XPK_WRITE_FILE_CHUNK_SIZE) ? XPK_WRITE_FILE_CHUNK_SIZE : (uint32_t)iSizeLeft;
		pChunk = NULL;
		iRet = procXpkReadAtAlloc(objXpk, hFileSrc, iOffsetCur, iChunkSize, &pChunk);
		if ( iRet != XPK_OK ) {
			if ( hFileSrc != NULL ) {
				xrtClose(hFileSrc);
			}
			xrtClose(hFileDst);
			return iRet;
		}

		iWrite = xrtWrite(hFileDst, (str)pChunk, iChunkSize);
		xpkFreeInternal(pChunk);
		if ( iWrite != iChunkSize ) {
			if ( hFileSrc != NULL ) {
				xrtClose(hFileSrc);
			}
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		}

		iOffsetCur += iChunkSize;
		iSizeLeft -= iChunkSize;
	}

	if ( !xrtSetEOF(hFileDst) ) {
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
	}

	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}
	xrtClose(hFileDst);
	procXpkClearError(objXpk);
	return XPK_OK;
}

static inline int procXpkWriteImmediate(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint32_t iSize, uint8_t iLevel)
{
	xfile hFile;
	void* pCompData;
	uint32_t iCompSize;
	uint8_t iUsedLevel;
	int iRet;

	if ( procXpkWriteQueueCount(objXpk) > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBufferedPending);
	}

	iRet = procXpkCodecEncode(objXpk, iLevel, pData, iSize, &pCompData, &iCompSize, &iUsedLevel);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	hFile = NULL;
	if ( !procXpkAppliedVolumeMode(objXpk) ) {
		hFile = xrtOpen(objXpk->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFile == NULL ) {
			xpkFreeInternal(pCompData);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
		}
	}

	iRet = procXpkWriteAt(objXpk, hFile, objXpk->iAppendPos, pCompData, iCompSize);
	if ( hFile != NULL ) {
		xrtClose(hFile);
	}
	if ( iRet != XPK_OK ) {
		xpkFreeInternal(pCompData);
		return iRet;
	}

	pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_COMP_MASK) | iUsedLevel;
	pEntry->iFileHash = xpkHash32Internal(pData, iSize);
	pEntry->iDataOffset = objXpk->iAppendPos;
	pEntry->iDataSize = iCompSize;
	pEntry->iFileSize = iSize;
	objXpk->iAppendPos += iCompSize;
	if ( objXpk->iAppendPos > objXpk->iFileSize ) {
		objXpk->iFileSize = objXpk->iAppendPos;
	}

	xpkFreeInternal(pCompData);
	return XPK_OK;
}

static inline int procXpkWriteBuffered(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint32_t iSize, uint8_t iLevel)
{
	void* pCompData;
	uint32_t iCompSize;
	uint8_t iUsedLevel;
	int iRet;

	iRet = procXpkCodecEncode(objXpk, iLevel, pData, iSize, &pCompData, &iCompSize, &iUsedLevel);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	iRet = procXpkQueueWrite(objXpk, pEntry->iPos, iUsedLevel, pCompData, iCompSize, iSize);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_COMP_MASK) | iUsedLevel;
	pEntry->iFileHash = xpkHash32Internal(pData, iSize);
	pEntry->iDataSize = iCompSize;
	pEntry->iFileSize = iSize;
	return XPK_OK;
}

static inline uint8_t procXpkSolidTargetCompLevel(xpkObject objXpk)
{
	if ( objXpk == NULL ) {
		return 0;
	}
	return (uint8_t)objXpk->objHead.defComp;
}

static inline uint8_t procXpkSolidStoredCompLevel(xpkObject objXpk)
{
	uint32_t iPos;
	xpkEntry* pEntry;

	if ( objXpk == NULL ) {
		return 0;
	}
	if ( !objXpk->bSolidApplied ) {
		return procXpkSolidTargetCompLevel(objXpk);
	}

	for ( iPos = 1; iPos <= objXpk->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, iPos);
		if ( (pEntry != NULL) && !procXpkEntryDeleted(pEntry) ) {
			return (uint8_t)(pEntry->iFlag & XPK_FLAG_COMP_MASK);
		}
	}

	return procXpkSolidTargetCompLevel(objXpk);
}

static inline int procXpkCalcSolidRawSize(xpkObject objXpk, uint64_t* pSizeRet)
{
	uint32_t iPos;
	xpkEntry* pEntry;
	uint64_t iEndPos;
	uint64_t iSizeMax;

	if ( pSizeRet == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	*pSizeRet = 0;
	iSizeMax = 0;
	for ( iPos = 1; iPos <= objXpk->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, iPos);
		if ( pEntry == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		}

		iEndPos = pEntry->iDataOffset + pEntry->iFileSize;
		if ( iEndPos > iSizeMax ) {
			iSizeMax = iEndPos;
		}
	}

	*pSizeRet = iSizeMax;
	return XPK_OK;
}

static inline int procXpkReadSolidStream(xpkObject objXpk, void** pDataRet, uint64_t* pSizeRet)
{
	xfile hFile;
	void* pCompData;
	void* pRawData;
	uint64_t iRawSize64;
	uint32_t iCompSize;
	uint8_t iLevel;
	int iRet;

	if ( pDataRet != NULL ) {
		*pDataRet = NULL;
	}
	if ( pSizeRet != NULL ) {
		*pSizeRet = 0;
	}
	if ( objXpk == NULL || !objXpk->bSolidApplied ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}

	iRet = procXpkCalcSolidRawSize(objXpk, &iRawSize64);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( iRawSize64 > UINT32_MAX ) {
		return procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge);
	}
	if ( objXpk->objHead.dataOffset < XPK_HEAD_SIZE ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	iCompSize = (uint32_t)(objXpk->objHead.dataOffset - XPK_HEAD_SIZE);
	if ( iCompSize == 0 ) {
		if ( iRawSize64 != 0 ) {
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}

		pRawData = xpkAllocInternal(1);
		if ( pRawData == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
		if ( pDataRet != NULL ) {
			*pDataRet = pRawData;
		}
		if ( pSizeRet != NULL ) {
			*pSizeRet = 0;
		}
		return XPK_OK;
	}

	hFile = xrtOpen(objXpk->sPathPackage, TRUE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	pCompData = NULL;
	iRet = procXpkReadAtAlloc(objXpk, hFile, XPK_HEAD_SIZE, iCompSize, &pCompData);
	xrtClose(hFile);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	iLevel = procXpkSolidStoredCompLevel(objXpk);
	iRet = procXpkCodecDecode(objXpk, iLevel, pCompData, iCompSize, (uint32_t)iRawSize64, &pRawData);
	xrtFree(pCompData);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	if ( pDataRet != NULL ) {
		*pDataRet = pRawData;
	}
	if ( pSizeRet != NULL ) {
		*pSizeRet = iRawSize64;
	}
	return XPK_OK;
}

static inline void* procXpkReadSolidEntryData(xpkObject objXpk, xpkEntry* pEntry, uint64_t* pSizeRet)
{
	void* pSolidRaw;
	void* pFileData;
	uint64_t iSolidSize;
	int iRet;

	if ( pSizeRet != NULL ) {
		*pSizeRet = 0;
	}
	if ( pEntry == NULL ) {
		procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
		return NULL;
	}
	if ( pEntry->iFileSize == 0 ) {
		pFileData = xpkAllocInternal(1);
		if ( pFileData == NULL ) {
			procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
			return NULL;
		}
		procXpkClearError(objXpk);
		return pFileData;
	}

	iRet = procXpkReadSolidStream(objXpk, &pSolidRaw, &iSolidSize);
	if ( iRet != XPK_OK ) {
		return NULL;
	}
	if ( (pEntry->iDataOffset + pEntry->iFileSize) > iSolidSize ) {
		xpkFreeInternal(pSolidRaw);
		procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		return NULL;
	}

	pFileData = xpkAllocInternal((size_t)pEntry->iFileSize);
	if ( pFileData == NULL ) {
		xpkFreeInternal(pSolidRaw);
		procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		return NULL;
	}

	memcpy(pFileData, (const uint8_t*)pSolidRaw + pEntry->iDataOffset, (size_t)pEntry->iFileSize);
	xpkFreeInternal(pSolidRaw);
	if ( pSizeRet != NULL ) {
		*pSizeRet = pEntry->iFileSize;
	}
	procXpkClearError(objXpk);
	return pFileData;
}

static inline int procXpkStoreEntryData(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint64_t iSize, const xpkWriteOptions* pOpt)
{
	uint8_t iLevel;
	uint8_t iWritePolicy;
	int iRet;

	if ( objXpk->bSolidApplied ) {
		return procXpkSetError(objXpk, XPK_ERR_SOLID_DATA_WRITE, "solid mode does not allow file data writes");
	}
	if ( objXpk->bReadonly ) {
		return procXpkSetError(objXpk, XPK_ERR_READONLY, sXpkErrorReadonly);
	}
	if ( iSize > UINT32_MAX ) {
		return procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge);
	}

	iRet = procXpkResolveCompLevel(objXpk, pOpt, &iLevel);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	iWritePolicy = procXpkResolveWritePolicy(objXpk, pOpt);
	if ( iWritePolicy == XPK_WRITE_IMMEDIATE ) {
		iRet = procXpkWriteImmediate(objXpk, pEntry, pData, (uint32_t)iSize, iLevel);
	} else {
		iRet = procXpkWriteBuffered(objXpk, pEntry, pData, (uint32_t)iSize, iLevel);
	}
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

static inline void* procXpkReadEntryData(xpkObject objXpk, xpkEntry* pEntry, uint64_t* pSizeRet)
{
	xpkWriteNode* pNode;
	xfile hFile;
	void* pCompData;
	void* pRawData;
	int iRet;
	uint8_t iLevel;

	if ( pSizeRet != NULL ) {
		*pSizeRet = 0;
	}
	if ( pEntry == NULL ) {
		procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
		return NULL;
	}
	if ( procXpkEntryDeleted(pEntry) ) {
		procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorDeleted);
		return NULL;
	}
	if ( pEntry->iFileSize == 0 ) {
		pRawData = xpkAllocInternal(1);
		if ( pRawData == NULL ) {
			procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
			return NULL;
		}
		if ( pSizeRet != NULL ) {
			*pSizeRet = 0;
		}
		procXpkClearError(objXpk);
		return pRawData;
	}
	if ( objXpk->bSolidApplied ) {
		return procXpkReadSolidEntryData(objXpk, pEntry, pSizeRet);
	}

	pNode = procXpkFindWriteNode(objXpk, pEntry->iPos, NULL);
	iLevel = (uint8_t)(pEntry->iFlag & XPK_FLAG_COMP_MASK);
	if ( pNode != NULL ) {
		iRet = procXpkCodecDecode(objXpk, pNode->iLevel, pNode->pCompData, pNode->iCompSize, (uint32_t)pEntry->iFileSize, &pRawData);
		if ( iRet != XPK_OK ) {
			return NULL;
		}
		if ( pSizeRet != NULL ) {
			*pSizeRet = pEntry->iFileSize;
		}
		procXpkClearError(objXpk);
		return pRawData;
	}

	hFile = xrtOpen(objXpk->sPathPackage, TRUE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
		return NULL;
	}

	pCompData = NULL;
	iRet = procXpkReadAtAlloc(objXpk, hFile, pEntry->iDataOffset, (uint32_t)pEntry->iDataSize, &pCompData);
	xrtClose(hFile);
	if ( iRet != XPK_OK ) {
		return NULL;
	}

	iRet = procXpkCodecDecode(objXpk, iLevel, pCompData, (uint32_t)pEntry->iDataSize, (uint32_t)pEntry->iFileSize, &pRawData);
	xrtFree(pCompData);
	if ( iRet != XPK_OK ) {
		return NULL;
	}
	if ( pSizeRet != NULL ) {
		*pSizeRet = pEntry->iFileSize;
	}
	procXpkClearError(objXpk);
	return pRawData;
}

static inline int procXpkFlushQueuedWrites(xpkObject objXpk, xfile hFile)
{
	uint32_t iNodePos;
	xpkWriteNode* pNode;
	xpkEntry* pEntry;
	int iRet;

	if ( (objXpk == NULL) || (objXpk->pWriteQueue == NULL) || (objXpk->pWriteQueue->arrNode.Count == 0) ) {
		return XPK_OK;
	}

	for ( iNodePos = 1; iNodePos <= objXpk->pWriteQueue->arrNode.Count; iNodePos++ ) {
		pNode = (xpkWriteNode*)xrtArrayGet(&objXpk->pWriteQueue->arrNode, iNodePos);
		if ( pNode == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		}

		pEntry = procXpkGetEntryByPos(objXpk, pNode->iPos);
		if ( pEntry == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}

		iRet = procXpkWriteAt(objXpk, hFile, objXpk->iAppendPos, pNode->pCompData, pNode->iCompSize);
		if ( iRet != XPK_OK ) {
			return iRet;
		}

		pEntry->iDataOffset = objXpk->iAppendPos;
		pEntry->iDataSize = pNode->iCompSize;
		pEntry->iFileSize = pNode->iRawSize;
		pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_COMP_MASK) | pNode->iLevel;
		objXpk->iAppendPos += pNode->iCompSize;
		if ( objXpk->iAppendPos > objXpk->iFileSize ) {
			objXpk->iFileSize = objXpk->iAppendPos;
		}
	}

	return XPK_OK;
}

#endif
