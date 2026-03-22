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
#define XPK_CODEC_STREAM_CHUNK_SIZE (256u * 1024u)

static inline uint8_t procXpkSolidStoredCompLevel(xpkObject objXpk);
static inline int procXpkCalcSolidRawSize(xpkObject objXpk, uint64_t* pSizeRet);
static inline int procXpkHashMappedFileRange(xpkObject objXpk, xfile hFile, uint64_t iOffset, uint64_t iSize, uint32_t* pHashRet);
static inline int procXpkCopyDecodedLz4BlockToFile(xpkObject objXpk, uint8_t iLevel, const void* pCompData, uint32_t iCompSize, uint64_t iRawSize, const char* sPathFile);
static inline int procXpkWriteImmediateStoreFile(xpkObject objXpk, xpkEntry* pEntry, const char* sSrcPath);
static inline int procXpkWriteImmediateLz4Data(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint32_t iSize, uint8_t iLevel);
static inline int procXpkWriteImmediateZstdData(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint32_t iSize, uint8_t iLevel);
static inline int procXpkWriteImmediateLzma2Data(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint32_t iSize, uint8_t iLevel);
static inline int procXpkWriteBufferedLz4Data(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint32_t iSize, uint8_t iLevel);
static inline int procXpkWriteBufferedZstdData(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint32_t iSize, uint8_t iLevel);
static inline int procXpkWriteBufferedLzma2Data(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint32_t iSize, uint8_t iLevel);

static inline int procXpkWriteAtChunkedPackage(xpkObject objXpk, xfile hFile, uint64_t iOffset, const void* pData, uint64_t iSize)
{
	const uint8_t* pCur;
	uint64_t iSizeLeft;
	uint32_t iChunkSize;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pData == NULL && iSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	pCur = (const uint8_t*)pData;
	iSizeLeft = iSize;
	while ( iSizeLeft > 0 ) {
		iChunkSize = (iSizeLeft > XPK_WRITE_FILE_CHUNK_SIZE) ? XPK_WRITE_FILE_CHUNK_SIZE : (uint32_t)iSizeLeft;
		iRet = procXpkWriteAt(objXpk, hFile, iOffset, pCur, iChunkSize);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		pCur += iChunkSize;
		iOffset += iChunkSize;
		iSizeLeft -= iChunkSize;
	}

	return XPK_OK;
}

static inline int procXpkCopySourceFileToPackage(xpkObject objXpk, xfile hFileSrc, xfile hFileDst, uint64_t iOffsetDst, uint64_t iSize)
{
	void* pChunk;
	uint64_t iSizeLeft;
	uint32_t iChunkSize;
	size_t iRead;
	int iRet;

	if ( objXpk == NULL || hFileSrc == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( iSize == 0 ) {
		return XPK_OK;
	}

	pChunk = xpkAllocInternal(XPK_WRITE_FILE_CHUNK_SIZE);
	if ( pChunk == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	iRet = procXpkSeekFile(objXpk, hFileSrc, 0);
	if ( iRet != XPK_OK ) {
		xpkFreeInternal(pChunk);
		return iRet;
	}

	iSizeLeft = iSize;
	while ( iSizeLeft > 0 ) {
		iChunkSize = (iSizeLeft > XPK_WRITE_FILE_CHUNK_SIZE) ? XPK_WRITE_FILE_CHUNK_SIZE : (uint32_t)iSizeLeft;
		iRead = xrtGetBuffer(hFileSrc, pChunk, iChunkSize);
		if ( iRead != iChunkSize ) {
			xpkFreeInternal(pChunk);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoRead);
		}

		iRet = procXpkWriteAt(objXpk, hFileDst, iOffsetDst, pChunk, iChunkSize);
		if ( iRet != XPK_OK ) {
			xpkFreeInternal(pChunk);
			return iRet;
		}

		iOffsetDst += iChunkSize;
		iSizeLeft -= iChunkSize;
	}

	xpkFreeInternal(pChunk);
	return XPK_OK;
}

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

static inline int procXpkWriteImmediateStoreFileWithQueuedFallback(xpkObject objXpk, xpkEntry* pEntry, const char* sSrcPath)
{
	xpkWriteNode* pNode;
	xpkWriteNode objNodeSaved;
	xpkWriteNode* pNodeRestore;
	uint32_t iNodePos;
	uint32_t iInsertPos;
	int iRet;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}

	if ( procXpkWriteQueueCount(objXpk) == 0 ) {
		return procXpkWriteImmediateStoreFile(objXpk, pEntry, sSrcPath);
	}

	pNode = procXpkFindWriteNode(objXpk, pEntry->iPos, &iNodePos);
	if ( pNode == NULL || procXpkWriteQueueCount(objXpk) != 1 ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBufferedPending);
	}

	objNodeSaved = *pNode;
	pNode->pCompData = NULL;
	xrtArrayRemove(&objXpk->pWriteQueue->arrNode, iNodePos, 1);

	iRet = procXpkWriteImmediateStoreFile(objXpk, pEntry, sSrcPath);
	if ( iRet == XPK_OK ) {
		if ( objNodeSaved.pCompData != NULL ) {
			xpkFreeInternal(objNodeSaved.pCompData);
		}
		return XPK_OK;
	}

	iInsertPos = xrtArrayInsert(&objXpk->pWriteQueue->arrNode, iNodePos - 1, 1);
	if ( iInsertPos != iNodePos ) {
		if ( objNodeSaved.pCompData != NULL ) {
			xpkFreeInternal(objNodeSaved.pCompData);
		}
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	pNodeRestore = (xpkWriteNode*)xrtArrayGet(&objXpk->pWriteQueue->arrNode, iNodePos);
	if ( pNodeRestore == NULL ) {
		if ( objNodeSaved.pCompData != NULL ) {
			xpkFreeInternal(objNodeSaved.pCompData);
		}
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}

	*pNodeRestore = objNodeSaved;
	return iRet;
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

static inline int procXpkOpenMappedSourceFile(xpkObject objXpk, const char* sPathFile, xfile* pFileRet, uint64_t* pSizeRet, xpkMappedFile* pMapRet)
{
	xfile hFile;
	uint64_t iFileSize;
	int iRet;

	if ( pFileRet != NULL ) {
		*pFileRet = NULL;
	}
	if ( pSizeRet != NULL ) {
		*pSizeRet = 0;
	}
	if ( pMapRet != NULL ) {
		memset(pMapRet, 0, sizeof(*pMapRet));
	}
	if ( sPathFile == NULL || sPathFile[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( pFileRet == NULL || pSizeRet == NULL || pMapRet == NULL ) {
		return procXpkReturnParamError(objXpk);
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

	iRet = procXpkMapFileReadOnly(objXpk, hFile, iFileSize, pMapRet);
	if ( iRet != XPK_OK ) {
		xrtClose(hFile);
		return iRet;
	}

	*pFileRet = hFile;
	*pSizeRet = iFileSize;
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
	if ( procXpkSeekFile(objXpk, hFile, 0) != XPK_OK ) {
		xrtClose(hFile);
		return xpkLastError(objXpk);
	}

	pCur = (const uint8_t*)pData;
	iSizeLeft = iSize;
	while ( iSizeLeft > 0 ) {
		size_t iWrite;

		iChunkSize = (iSizeLeft > XPK_WRITE_FILE_CHUNK_SIZE) ? XPK_WRITE_FILE_CHUNK_SIZE : (size_t)iSizeLeft;
		iWrite = xrtPut(hFile, (ptr)pCur, iChunkSize);
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

static inline int procXpkCopyPackageRangeToFile(xpkObject objXpk, uint64_t iOffsetSrc, uint64_t iSize, const char* sPathFile)
{
	xfile hFileSrc;
	xfile hFileDst;
	void* pChunk;
	uint64_t iSizeLeft;
	uint32_t iChunkSize;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( sPathFile == NULL || sPathFile[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	hFileDst = xrtOpen((str)sPathFile, FALSE, XRT_CP_BINARY);
	if ( hFileDst == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}
	if ( procXpkSeekFile(objXpk, hFileDst, 0) != XPK_OK ) {
		xrtClose(hFileDst);
		return xpkLastError(objXpk);
	}

	hFileSrc = NULL;
	if ( !procXpkAppliedVolumeMode(objXpk) ) {
		hFileSrc = xrtOpen(objXpk->sPathPackage, TRUE, XRT_CP_BINARY);
		if ( hFileSrc == NULL ) {
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
		}
	}
	pChunk = xpkAllocInternal(XPK_WRITE_FILE_CHUNK_SIZE);
	if ( pChunk == NULL ) {
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	iSizeLeft = iSize;
	while ( iSizeLeft > 0 ) {
		size_t iWrite;

		iChunkSize = (iSizeLeft > XPK_WRITE_FILE_CHUNK_SIZE) ? XPK_WRITE_FILE_CHUNK_SIZE : (uint32_t)iSizeLeft;
		iRet = procXpkReadAtBuffer(objXpk, hFileSrc, iOffsetSrc, pChunk, iChunkSize);
		if ( iRet != XPK_OK ) {
			xpkFreeInternal(pChunk);
			if ( hFileSrc != NULL ) {
				xrtClose(hFileSrc);
			}
			xrtClose(hFileDst);
			return iRet;
		}

		iWrite = xrtPut(hFileDst, (ptr)pChunk, iChunkSize);
		if ( iWrite != iChunkSize ) {
			xpkFreeInternal(pChunk);
			if ( hFileSrc != NULL ) {
				xrtClose(hFileSrc);
			}
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		}

		iOffsetSrc += iChunkSize;
		iSizeLeft -= iChunkSize;
	}

	if ( !xrtSetEOF(hFileDst) ) {
		xpkFreeInternal(pChunk);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
	}

	xpkFreeInternal(pChunk);
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}
	xrtClose(hFileDst);
	procXpkClearError(objXpk);
	return XPK_OK;
}

static inline int procXpkCopyStoredEntryToFile(xpkObject objXpk, xpkEntry* pEntry, const char* sPathFile)
{
	uint32_t iLevel;
	uint64_t iDataEnd;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
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
	iDataEnd = procXpkCurrentDataEnd(objXpk);
	if ( pEntry->iDataSize != pEntry->iFileSize ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pEntry->iDataOffset < XPK_HEAD_SIZE ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pEntry->iDataOffset > iDataEnd ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pEntry->iDataSize > (iDataEnd - pEntry->iDataOffset) ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	return procXpkCopyPackageRangeToFile(objXpk, pEntry->iDataOffset, pEntry->iDataSize, sPathFile);
}

static inline int procXpkCopyStoredLz4EntryToFile(xpkObject objXpk, xpkEntry* pEntry, const char* sPathFile)
{
	xfile hFileSrc;
	xpkMappedFile objMap;
	void* pCompData;
	uint64_t iPkgSize;
	uint64_t iDataEnd;
	uint32_t iLevel;
	uint32_t iAlg;
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

	iLevel = (uint32_t)(pEntry->iFlag & XPK_FLAG_COMP_MASK);
	iAlg = procXpkCompLevelToAlg((uint8_t)iLevel);
	if ( iAlg != XPK_ALG_LZ4 && iAlg != XPK_ALG_LZ4HC ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}

	if ( !procXpkAppliedVolumeMode(objXpk) ) {
		memset(&objMap, 0, sizeof(objMap));
		hFileSrc = xrtOpen(objXpk->sPathPackage, TRUE, XRT_CP_BINARY);
		if ( hFileSrc == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
		}

		iPkgSize = xrtGetEOF(hFileSrc);
		if ( pEntry->iDataOffset > iPkgSize || pEntry->iDataSize > (iPkgSize - pEntry->iDataOffset) ) {
			xrtClose(hFileSrc);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
		iRet = procXpkMapFileReadOnly(objXpk, hFileSrc, iPkgSize, &objMap);
		xrtClose(hFileSrc);
		if ( iRet != XPK_OK ) {
			return iRet;
		}

		iRet = procXpkCopyDecodedLz4BlockToFile(objXpk, (uint8_t)iLevel, (const uint8_t*)objMap.pView + pEntry->iDataOffset, (uint32_t)pEntry->iDataSize, pEntry->iFileSize, sPathFile);
		procXpkUnmapFile(&objMap);
		return iRet;
	}

	if ( pEntry->iDataSize > UINT32_MAX ) {
		return procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge);
	}

	pCompData = NULL;
	iRet = procXpkReadAtAlloc(objXpk, NULL, pEntry->iDataOffset, (uint32_t)pEntry->iDataSize, &pCompData);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	iRet = procXpkCopyDecodedLz4BlockToFile(objXpk, (uint8_t)iLevel, pCompData, (uint32_t)pEntry->iDataSize, pEntry->iFileSize, sPathFile);
	xpkFreeInternal(pCompData);
	return iRet;
}

static inline int procXpkCopyStoredZstdEntryToFile(xpkObject objXpk, xpkEntry* pEntry, const char* sPathFile)
{
	xfile hFileSrc;
	xfile hFileDst;
	uint64_t iPkgSize;
	uint64_t iDataEnd;
	uint64_t iOffsetRead;
	uint64_t iSizeLeft;
	uint64_t iSizeWritten;
	ZSTD_DStream* pStream;
	ZSTD_inBuffer objIn;
	ZSTD_outBuffer objOut;
	void* pInBuf;
	void* pOutBuf;
	size_t iInCap;
	size_t iOutCap;
	size_t iZstdRet;
	uint32_t iLevel;
	uint32_t iChunkRead;
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
	if ( pEntry->iFileSize == 0 ) {
		return procXpkWriteFileData(objXpk, sPathFile, NULL, 0);
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

	iLevel = (uint32_t)(pEntry->iFlag & XPK_FLAG_COMP_MASK);
	if ( procXpkCompLevelToAlg((uint8_t)iLevel) != XPK_ALG_ZSTD ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}

	hFileSrc = NULL;
	hFileDst = xrtOpen((str)sPathFile, FALSE, XRT_CP_BINARY);
	if ( hFileDst == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}
	if ( procXpkSeekFile(objXpk, hFileDst, 0) != XPK_OK ) {
		xrtClose(hFileDst);
		return xpkLastError(objXpk);
	}

	if ( !procXpkAppliedVolumeMode(objXpk) ) {
		hFileSrc = xrtOpen(objXpk->sPathPackage, TRUE, XRT_CP_BINARY);
		if ( hFileSrc == NULL ) {
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
		}
		iPkgSize = xrtGetEOF(hFileSrc);
		if ( pEntry->iDataOffset > iPkgSize || pEntry->iDataSize > (iPkgSize - pEntry->iDataOffset) ) {
			xrtClose(hFileSrc);
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
	}

	iInCap = ZSTD_DStreamInSize();
	if ( iInCap == 0 ) {
		iInCap = 131072u;
	}
	iOutCap = ZSTD_DStreamOutSize();
	if ( iOutCap == 0 ) {
		iOutCap = 131072u;
	}

	pInBuf = xpkAllocInternal(iInCap);
	if ( pInBuf == NULL ) {
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}
	pOutBuf = xpkAllocInternal(iOutCap);
	if ( pOutBuf == NULL ) {
		xpkFreeInternal(pInBuf);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	pStream = ZSTD_createDStream();
	if ( pStream == NULL ) {
		xpkFreeInternal(pOutBuf);
		xpkFreeInternal(pInBuf);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	iZstdRet = ZSTD_initDStream(pStream);
	if ( ZSTD_isError(iZstdRet) ) {
		ZSTD_freeDStream(pStream);
		xpkFreeInternal(pOutBuf);
		xpkFreeInternal(pInBuf);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, "zstd decompress failed");
	}

	iOffsetRead = pEntry->iDataOffset;
	iSizeLeft = pEntry->iDataSize;
	iSizeWritten = 0;
	memset(&objIn, 0, sizeof(objIn));

	for ( ;; ) {
		if ( (objIn.pos == objIn.size) && (iSizeLeft > 0) ) {
			iChunkRead = (uint32_t)((iSizeLeft > (uint64_t)iInCap) ? iInCap : iSizeLeft);
			iRet = procXpkReadAtBuffer(objXpk, hFileSrc, iOffsetRead, pInBuf, iChunkRead);
			if ( iRet != XPK_OK ) {
				ZSTD_freeDStream(pStream);
				xpkFreeInternal(pOutBuf);
				xpkFreeInternal(pInBuf);
				if ( hFileSrc != NULL ) {
					xrtClose(hFileSrc);
				}
				xrtClose(hFileDst);
				return iRet;
			}

			objIn.src = pInBuf;
			objIn.size = iChunkRead;
			objIn.pos = 0;
			iOffsetRead += iChunkRead;
			iSizeLeft -= iChunkRead;
		} else if ( (objIn.pos == objIn.size) && (iSizeLeft == 0) ) {
			objIn.src = pInBuf;
			objIn.size = 0;
			objIn.pos = 0;
		}

		objOut.dst = pOutBuf;
		objOut.size = iOutCap;
		objOut.pos = 0;
		iZstdRet = ZSTD_decompressStream(pStream, &objOut, &objIn);
		if ( ZSTD_isError(iZstdRet) ) {
			ZSTD_freeDStream(pStream);
			xpkFreeInternal(pOutBuf);
			xpkFreeInternal(pInBuf);
			if ( hFileSrc != NULL ) {
				xrtClose(hFileSrc);
			}
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, "zstd decompress failed");
		}

		if ( objOut.pos > 0 ) {
			if ( xrtPut(hFileDst, (ptr)pOutBuf, objOut.pos) != objOut.pos ) {
				ZSTD_freeDStream(pStream);
				xpkFreeInternal(pOutBuf);
				xpkFreeInternal(pInBuf);
				if ( hFileSrc != NULL ) {
					xrtClose(hFileSrc);
				}
				xrtClose(hFileDst);
				return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
			}
			iSizeWritten += objOut.pos;
		}

		if ( (iSizeLeft == 0) && (objIn.pos == objIn.size) && (iZstdRet == 0) ) {
			break;
		}
		if ( (iSizeLeft == 0) && (objIn.pos == objIn.size) && (objOut.pos == 0) && (iZstdRet > 0) ) {
			ZSTD_freeDStream(pStream);
			xpkFreeInternal(pOutBuf);
			xpkFreeInternal(pInBuf);
			if ( hFileSrc != NULL ) {
				xrtClose(hFileSrc);
			}
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, "zstd decompress failed");
		}
	}

	if ( iSizeWritten != pEntry->iFileSize ) {
		ZSTD_freeDStream(pStream);
		xpkFreeInternal(pOutBuf);
		xpkFreeInternal(pInBuf);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( !xrtSetEOF(hFileDst) ) {
		ZSTD_freeDStream(pStream);
		xpkFreeInternal(pOutBuf);
		xpkFreeInternal(pInBuf);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
	}

	ZSTD_freeDStream(pStream);
	xpkFreeInternal(pOutBuf);
	xpkFreeInternal(pInBuf);
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}
	xrtClose(hFileDst);
	procXpkClearError(objXpk);
	return XPK_OK;
}

static inline int procXpkCopyStoredLzma2EntryToFile(xpkObject objXpk, xpkEntry* pEntry, const char* sPathFile)
{
	xfile hFileSrc;
	xfile hFileDst;
	uint64_t iPkgSize;
	uint64_t iDataEnd;
	uint64_t iOffsetRead;
	uint64_t iSizeLeft;
	uint64_t iSizeWritten;
	uint32_t iLevel;
	uint32_t iChunkRead;
	Byte iPropByte;
	CLzma2Dec objDec;
	ELzmaStatus iStatus;
	ELzmaFinishMode iFinishMode;
	void* pInBuf;
	void* pOutBuf;
	SizeT iInCap;
	SizeT iOutCap;
	SizeT iInputSize;
	SizeT iInputPos;
	SizeT iSrcLen;
	SizeT iDstLen;
	int iRet;
	SRes iLzmaRes;

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
	if ( pEntry->iFileSize == 0 ) {
		return procXpkWriteFileData(objXpk, sPathFile, NULL, 0);
	}
	if ( pEntry->iDataSize < 1 ) {
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

	iLevel = (uint32_t)(pEntry->iFlag & XPK_FLAG_COMP_MASK);
	if ( procXpkCompLevelToAlg((uint8_t)iLevel) != XPK_ALG_LZMA2 ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}

	hFileSrc = NULL;
	hFileDst = xrtOpen((str)sPathFile, FALSE, XRT_CP_BINARY);
	if ( hFileDst == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}
	if ( procXpkSeekFile(objXpk, hFileDst, 0) != XPK_OK ) {
		xrtClose(hFileDst);
		return xpkLastError(objXpk);
	}

	if ( !procXpkAppliedVolumeMode(objXpk) ) {
		hFileSrc = xrtOpen(objXpk->sPathPackage, TRUE, XRT_CP_BINARY);
		if ( hFileSrc == NULL ) {
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
		}
		iPkgSize = xrtGetEOF(hFileSrc);
		if ( pEntry->iDataOffset > iPkgSize || pEntry->iDataSize > (iPkgSize - pEntry->iDataOffset) ) {
			xrtClose(hFileSrc);
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
	}

	iRet = procXpkReadAtBuffer(objXpk, hFileSrc, pEntry->iDataOffset, &iPropByte, 1);
	if ( iRet != XPK_OK ) {
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		return iRet;
	}

	iInCap = XPK_CODEC_STREAM_CHUNK_SIZE;
	iOutCap = XPK_CODEC_STREAM_CHUNK_SIZE;
	pInBuf = xpkAllocInternal(iInCap);
	if ( pInBuf == NULL ) {
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}
	pOutBuf = xpkAllocInternal(iOutCap);
	if ( pOutBuf == NULL ) {
		xpkFreeInternal(pInBuf);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	Lzma2Dec_Construct(&objDec);
	iLzmaRes = Lzma2Dec_Allocate(&objDec, iPropByte, &g_Alloc);
	if ( iLzmaRes != SZ_OK ) {
		xpkFreeInternal(pOutBuf);
		xpkFreeInternal(pInBuf);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		if ( iLzmaRes == SZ_ERROR_MEM ) {
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, "lzma2 decompress failed");
	}
	Lzma2Dec_Init(&objDec);

	iOffsetRead = pEntry->iDataOffset + 1;
	iSizeLeft = pEntry->iDataSize - 1;
	iSizeWritten = 0;
	iInputSize = 0;
	iInputPos = 0;

	for ( ;; ) {
		if ( iSizeWritten >= pEntry->iFileSize ) {
			break;
		}
		if ( (iInputPos == iInputSize) && (iSizeLeft > 0) ) {
			iChunkRead = (uint32_t)((iSizeLeft > (uint64_t)iInCap) ? iInCap : iSizeLeft);
			iRet = procXpkReadAtBuffer(objXpk, hFileSrc, iOffsetRead, pInBuf, iChunkRead);
			if ( iRet != XPK_OK ) {
				Lzma2Dec_Free(&objDec, &g_Alloc);
				xpkFreeInternal(pOutBuf);
				xpkFreeInternal(pInBuf);
				if ( hFileSrc != NULL ) {
					xrtClose(hFileSrc);
				}
				xrtClose(hFileDst);
				return iRet;
			}
			iInputSize = iChunkRead;
			iInputPos = 0;
			iOffsetRead += iChunkRead;
			iSizeLeft -= iChunkRead;
		}

		iDstLen = (SizeT)(((pEntry->iFileSize - iSizeWritten) > (uint64_t)iOutCap) ? iOutCap : (pEntry->iFileSize - iSizeWritten));
		iSrcLen = iInputSize - iInputPos;
		iFinishMode = (((iSizeWritten + iDstLen) == pEntry->iFileSize) && (iSizeLeft == 0)) ? LZMA_FINISH_END : LZMA_FINISH_ANY;
		iStatus = LZMA_STATUS_NOT_SPECIFIED;
		iLzmaRes = Lzma2Dec_DecodeToBuf(&objDec, (Byte*)pOutBuf, &iDstLen, (const Byte*)pInBuf + iInputPos, &iSrcLen, iFinishMode, &iStatus);
		if ( iLzmaRes != SZ_OK ) {
			Lzma2Dec_Free(&objDec, &g_Alloc);
			xpkFreeInternal(pOutBuf);
			xpkFreeInternal(pInBuf);
			if ( hFileSrc != NULL ) {
				xrtClose(hFileSrc);
			}
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, "lzma2 decompress failed");
		}

		iInputPos += iSrcLen;
		if ( iDstLen > 0 ) {
			if ( xrtPut(hFileDst, (ptr)pOutBuf, iDstLen) != iDstLen ) {
				Lzma2Dec_Free(&objDec, &g_Alloc);
				xpkFreeInternal(pOutBuf);
				xpkFreeInternal(pInBuf);
				if ( hFileSrc != NULL ) {
					xrtClose(hFileSrc);
				}
				xrtClose(hFileDst);
				return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
			}
			iSizeWritten += iDstLen;
		}

		if ( (iDstLen == 0) && (iSrcLen == 0) ) {
			Lzma2Dec_Free(&objDec, &g_Alloc);
			xpkFreeInternal(pOutBuf);
			xpkFreeInternal(pInBuf);
			if ( hFileSrc != NULL ) {
				xrtClose(hFileSrc);
			}
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, "lzma2 decompress failed");
		}
	}

	if ( iSizeWritten != pEntry->iFileSize || iSizeLeft != 0 || iInputPos != iInputSize ) {
		Lzma2Dec_Free(&objDec, &g_Alloc);
		xpkFreeInternal(pOutBuf);
		xpkFreeInternal(pInBuf);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( !xrtSetEOF(hFileDst) ) {
		Lzma2Dec_Free(&objDec, &g_Alloc);
		xpkFreeInternal(pOutBuf);
		xpkFreeInternal(pInBuf);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
	}

	Lzma2Dec_Free(&objDec, &g_Alloc);
	xpkFreeInternal(pOutBuf);
	xpkFreeInternal(pInBuf);
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}
	xrtClose(hFileDst);
	procXpkClearError(objXpk);
	return XPK_OK;
}

static inline int procXpkCalcSolidStoredRawSize(xpkObject objXpk, uint64_t* pSizeRet)
{
	uint64_t iRawSize;

	if ( pSizeRet != NULL ) {
		*pSizeRet = 0;
	}
	if ( objXpk == NULL || pSizeRet == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( !objXpk->bSolidApplied ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}
	if ( objXpk->objHead.dataOffset < XPK_HEAD_SIZE ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( procXpkCalcSolidRawSize(objXpk, &iRawSize) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( procXpkSolidStoredCompLevel(objXpk) == 0 ) {
		if ( (objXpk->objHead.dataOffset - XPK_HEAD_SIZE) != iRawSize ) {
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
	}
	*pSizeRet = iRawSize;
	return XPK_OK;
}

static inline int procXpkCopyQueuedStoredEntryToFile(xpkObject objXpk, xpkEntry* pEntry, const char* sPathFile)
{
	xpkWriteNode* pNode;

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

	pNode = procXpkFindWriteNode(objXpk, pEntry->iPos, NULL);
	if ( pNode == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}
	if ( pNode->iLevel != 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}
	if ( pNode->iCompSize != pNode->iRawSize ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pEntry->iFileSize != pNode->iRawSize ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pNode->pCompData == NULL && pNode->iCompSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	return procXpkWriteFileData(objXpk, sPathFile, pNode->pCompData, pNode->iCompSize);
}

static inline int procXpkCopyDecodedLz4BlockToFile(xpkObject objXpk, uint8_t iLevel, const void* pCompData, uint32_t iCompSize, uint64_t iRawSize, const char* sPathFile)
{
	void* pRawData;
	int iDecRet;
	int iRet;
	uint32_t iAlg;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( sPathFile == NULL || sPathFile[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( iRawSize == 0 ) {
		return procXpkWriteFileData(objXpk, sPathFile, NULL, 0);
	}
	if ( pCompData == NULL || iCompSize == 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( iRawSize > (uint64_t)INT_MAX || iCompSize > (uint32_t)INT_MAX ) {
		return procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge);
	}

	iAlg = procXpkCompLevelToAlg(iLevel);
	if ( iAlg != XPK_ALG_LZ4 && iAlg != XPK_ALG_LZ4HC ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}

	pRawData = xpkAllocInternal((size_t)iRawSize);
	if ( pRawData == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	iDecRet = LZ4_decompress_safe((const char*)pCompData, (char*)pRawData, (int)iCompSize, (int)iRawSize);
	if ( iDecRet != (int)iRawSize ) {
		xpkFreeInternal(pRawData);
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, "lz4 decompress failed");
	}

	iRet = procXpkWriteFileData(objXpk, sPathFile, pRawData, iRawSize);
	xpkFreeInternal(pRawData);
	return iRet;
}

static inline int procXpkCopyQueuedLz4EntryToFile(xpkObject objXpk, xpkEntry* pEntry, const char* sPathFile)
{
	xpkWriteNode* pNode;
	uint32_t iAlg;

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

	pNode = procXpkFindWriteNode(objXpk, pEntry->iPos, NULL);
	if ( pNode == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}

	iAlg = procXpkCompLevelToAlg(pNode->iLevel);
	if ( iAlg != XPK_ALG_LZ4 && iAlg != XPK_ALG_LZ4HC ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}
	if ( pNode->pCompData == NULL && pNode->iCompSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pEntry->iFileSize != pNode->iRawSize ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	return procXpkCopyDecodedLz4BlockToFile(objXpk, pNode->iLevel, pNode->pCompData, pNode->iCompSize, pNode->iRawSize, sPathFile);
}

static inline int procXpkCopyQueuedZstdEntryToFile(xpkObject objXpk, xpkEntry* pEntry, const char* sPathFile)
{
	xpkWriteNode* pNode;
	xfile hFileDst;
	ZSTD_DStream* pStream;
	ZSTD_inBuffer objIn;
	ZSTD_outBuffer objOut;
	void* pOutBuf;
	size_t iOutCap;
	size_t iZstdRet;
	uint64_t iSizeWritten;

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

	pNode = procXpkFindWriteNode(objXpk, pEntry->iPos, NULL);
	if ( pNode == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}
	if ( procXpkCompLevelToAlg(pNode->iLevel) != XPK_ALG_ZSTD ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}
	if ( pNode->pCompData == NULL && pNode->iCompSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pEntry->iFileSize != pNode->iRawSize ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pEntry->iFileSize == 0 ) {
		return procXpkWriteFileData(objXpk, sPathFile, NULL, 0);
	}

	hFileDst = xrtOpen((str)sPathFile, FALSE, XRT_CP_BINARY);
	if ( hFileDst == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}
	if ( procXpkSeekFile(objXpk, hFileDst, 0) != XPK_OK ) {
		xrtClose(hFileDst);
		return xpkLastError(objXpk);
	}

	iOutCap = ZSTD_DStreamOutSize();
	if ( iOutCap == 0 ) {
		iOutCap = 131072u;
	}
	pOutBuf = xpkAllocInternal(iOutCap);
	if ( pOutBuf == NULL ) {
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	pStream = ZSTD_createDStream();
	if ( pStream == NULL ) {
		xpkFreeInternal(pOutBuf);
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}
	iZstdRet = ZSTD_initDStream(pStream);
	if ( ZSTD_isError(iZstdRet) ) {
		ZSTD_freeDStream(pStream);
		xpkFreeInternal(pOutBuf);
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, "zstd decompress failed");
	}

	objIn.src = pNode->pCompData;
	objIn.size = pNode->iCompSize;
	objIn.pos = 0;
	iSizeWritten = 0;
	for ( ;; ) {
		objOut.dst = pOutBuf;
		objOut.size = iOutCap;
		objOut.pos = 0;
		iZstdRet = ZSTD_decompressStream(pStream, &objOut, &objIn);
		if ( ZSTD_isError(iZstdRet) ) {
			ZSTD_freeDStream(pStream);
			xpkFreeInternal(pOutBuf);
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, "zstd decompress failed");
		}
		if ( objOut.pos > 0 ) {
			if ( xrtPut(hFileDst, (ptr)pOutBuf, objOut.pos) != objOut.pos ) {
				ZSTD_freeDStream(pStream);
				xpkFreeInternal(pOutBuf);
				xrtClose(hFileDst);
				return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
			}
			iSizeWritten += objOut.pos;
		}
		if ( (objIn.pos == objIn.size) && (iZstdRet == 0) ) {
			break;
		}
		if ( (objIn.pos == objIn.size) && (objOut.pos == 0) && (iZstdRet > 0) ) {
			ZSTD_freeDStream(pStream);
			xpkFreeInternal(pOutBuf);
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, "zstd decompress failed");
		}
	}

	if ( iSizeWritten != pEntry->iFileSize ) {
		ZSTD_freeDStream(pStream);
		xpkFreeInternal(pOutBuf);
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( !xrtSetEOF(hFileDst) ) {
		ZSTD_freeDStream(pStream);
		xpkFreeInternal(pOutBuf);
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
	}

	ZSTD_freeDStream(pStream);
	xpkFreeInternal(pOutBuf);
	xrtClose(hFileDst);
	procXpkClearError(objXpk);
	return XPK_OK;
}

static inline int procXpkCopyQueuedLzma2EntryToFile(xpkObject objXpk, xpkEntry* pEntry, const char* sPathFile)
{
	xpkWriteNode* pNode;
	xfile hFileDst;
	CLzma2Dec objDec;
	ELzmaStatus iStatus;
	ELzmaFinishMode iFinishMode;
	Byte iPropByte;
	void* pOutBuf;
	SizeT iOutCap;
	SizeT iInputPos;
	SizeT iInputSize;
	SizeT iSrcLen;
	SizeT iDstLen;
	uint64_t iSizeWritten;
	SRes iLzmaRes;

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

	pNode = procXpkFindWriteNode(objXpk, pEntry->iPos, NULL);
	if ( pNode == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}
	if ( procXpkCompLevelToAlg(pNode->iLevel) != XPK_ALG_LZMA2 ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}
	if ( pNode->pCompData == NULL && pNode->iCompSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pEntry->iFileSize != pNode->iRawSize ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pEntry->iFileSize == 0 ) {
		return procXpkWriteFileData(objXpk, sPathFile, NULL, 0);
	}
	if ( pNode->iCompSize < 1 ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	hFileDst = xrtOpen((str)sPathFile, FALSE, XRT_CP_BINARY);
	if ( hFileDst == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}
	if ( procXpkSeekFile(objXpk, hFileDst, 0) != XPK_OK ) {
		xrtClose(hFileDst);
		return xpkLastError(objXpk);
	}

	iOutCap = XPK_CODEC_STREAM_CHUNK_SIZE;
	pOutBuf = xpkAllocInternal(iOutCap);
	if ( pOutBuf == NULL ) {
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	iPropByte = ((const Byte*)pNode->pCompData)[0];
	Lzma2Dec_Construct(&objDec);
	iLzmaRes = Lzma2Dec_Allocate(&objDec, iPropByte, &g_Alloc);
	if ( iLzmaRes != SZ_OK ) {
		xpkFreeInternal(pOutBuf);
		xrtClose(hFileDst);
		if ( iLzmaRes == SZ_ERROR_MEM ) {
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, "lzma2 decompress failed");
	}
	Lzma2Dec_Init(&objDec);

	iInputPos = 1;
	iInputSize = pNode->iCompSize;
	iSizeWritten = 0;
	for ( ;; ) {
		if ( iSizeWritten >= pEntry->iFileSize ) {
			break;
		}

		iDstLen = (SizeT)(((pEntry->iFileSize - iSizeWritten) > (uint64_t)iOutCap) ? iOutCap : (pEntry->iFileSize - iSizeWritten));
		iSrcLen = iInputSize - iInputPos;
		iFinishMode = (((iSizeWritten + iDstLen) == pEntry->iFileSize) && (iInputPos + iSrcLen == iInputSize)) ? LZMA_FINISH_END : LZMA_FINISH_ANY;
		iStatus = LZMA_STATUS_NOT_SPECIFIED;
		iLzmaRes = Lzma2Dec_DecodeToBuf(&objDec, (Byte*)pOutBuf, &iDstLen, (const Byte*)pNode->pCompData + iInputPos, &iSrcLen, iFinishMode, &iStatus);
		if ( iLzmaRes != SZ_OK ) {
			Lzma2Dec_Free(&objDec, &g_Alloc);
			xpkFreeInternal(pOutBuf);
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, "lzma2 decompress failed");
		}

		iInputPos += iSrcLen;
		if ( iDstLen > 0 ) {
			if ( xrtPut(hFileDst, (ptr)pOutBuf, iDstLen) != iDstLen ) {
				Lzma2Dec_Free(&objDec, &g_Alloc);
				xpkFreeInternal(pOutBuf);
				xrtClose(hFileDst);
				return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
			}
			iSizeWritten += iDstLen;
		}

		if ( (iDstLen == 0) && (iSrcLen == 0) ) {
			Lzma2Dec_Free(&objDec, &g_Alloc);
			xpkFreeInternal(pOutBuf);
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, "lzma2 decompress failed");
		}
	}

	if ( iSizeWritten != pEntry->iFileSize || iInputPos != iInputSize ) {
		Lzma2Dec_Free(&objDec, &g_Alloc);
		xpkFreeInternal(pOutBuf);
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( !xrtSetEOF(hFileDst) ) {
		Lzma2Dec_Free(&objDec, &g_Alloc);
		xpkFreeInternal(pOutBuf);
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
	}

	Lzma2Dec_Free(&objDec, &g_Alloc);
	xpkFreeInternal(pOutBuf);
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

	if ( iLevel > 0 ) {
		if ( procXpkCompLevelToAlg(iLevel) == XPK_ALG_LZ4 || procXpkCompLevelToAlg(iLevel) == XPK_ALG_LZ4HC ) {
			return procXpkWriteImmediateLz4Data(objXpk, pEntry, pData, iSize, iLevel);
		}
		if ( procXpkCompLevelToAlg(iLevel) == XPK_ALG_ZSTD ) {
			return procXpkWriteImmediateZstdData(objXpk, pEntry, pData, iSize, iLevel);
		}
		if ( procXpkCompLevelToAlg(iLevel) == XPK_ALG_LZMA2 ) {
			return procXpkWriteImmediateLzma2Data(objXpk, pEntry, pData, iSize, iLevel);
		}
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
	pEntry->iFileHash = (iSize > 0) ? xpkHash32Internal(pData, iSize) : 0;
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
	if ( iSize > 0 ) {
		if ( procXpkCompLevelToAlg(iLevel) == XPK_ALG_LZ4 || procXpkCompLevelToAlg(iLevel) == XPK_ALG_LZ4HC ) {
			return procXpkWriteBufferedLz4Data(objXpk, pEntry, pData, iSize, iLevel);
		}
		if ( procXpkCompLevelToAlg(iLevel) == XPK_ALG_ZSTD ) {
			return procXpkWriteBufferedZstdData(objXpk, pEntry, pData, iSize, iLevel);
		}
		if ( procXpkCompLevelToAlg(iLevel) == XPK_ALG_LZMA2 ) {
			return procXpkWriteBufferedLzma2Data(objXpk, pEntry, pData, iSize, iLevel);
		}
	}

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
	pEntry->iFileHash = (iSize > 0) ? xpkHash32Internal(pData, iSize) : 0;
	pEntry->iDataSize = iCompSize;
	pEntry->iFileSize = iSize;
	return XPK_OK;
}

static inline char* procXpkWriteTempPathDup(xpkObject objXpk, const char* sTag)
{
	char sSuffix[128];
	char* sPathRet;
	uint64_t iStamp;
	uint64_t iTagObj;
	uint32_t iTry;
	int iSizePrint;

	if ( objXpk == NULL || objXpk->sPathPackage == NULL || sTag == NULL || sTag[0] == '\0' ) {
		procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
		return NULL;
	}

	iStamp = (uint64_t)xpkNowInternal();
	iTagObj = (uint64_t)(uintptr_t)objXpk;
	for ( iTry = 0; iTry < 32; iTry++ ) {
		iSizePrint = snprintf(
			sSuffix,
			sizeof(sSuffix),
			"%s.%llu.%llx.%u.tmp",
			sTag,
			(unsigned long long)iStamp,
			(unsigned long long)iTagObj,
			(unsigned int)iTry
		);
		if ( iSizePrint <= 0 || (size_t)iSizePrint >= sizeof(sSuffix) ) {
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite), NULL;
		}

		sPathRet = procXpkPathSuffixDupText(objXpk->sPathPackage, sSuffix);
		if ( sPathRet == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory), NULL;
		}
		if ( !xrtPathExists((str)sPathRet) ) {
			return sPathRet;
		}

		xpkFreeInternal(sPathRet);
	}

	return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite), NULL;
}

static inline int procXpkWriteBufferedLz4Data(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint32_t iSize, uint8_t iLevel)
{
	xfile hFileTmp;
	xpkMappedFile objMap;
	char* sPathTmp;
	void* pCompData;
	uint32_t iBound;
	uint32_t iCompSize;
	uint32_t iHash;
	int iAlg;
	int iCompRet;
	int iRet;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pData == NULL && iSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	iAlg = procXpkCompLevelToAlg(iLevel);
	if ( iAlg != XPK_ALG_LZ4 && iAlg != XPK_ALG_LZ4HC ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( iSize == 0 ) {
		return procXpkWriteBuffered(objXpk, pEntry, pData, iSize, 0);
	}
	if ( iSize > INT_MAX ) {
		return procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge);
	}

	hFileTmp = NULL;
	memset(&objMap, 0, sizeof(objMap));
	sPathTmp = NULL;
	pCompData = NULL;
	iBound = 0;
	iCompSize = 0;
	iHash = xpkHash32Internal(pData, iSize);

	iBound = (uint32_t)LZ4_compressBound((int)iSize);
	sPathTmp = procXpkWriteTempPathDup(objXpk, ".lz4.data.queue");
	if ( sPathTmp == NULL ) {
		return xpkLastError(objXpk);
	}

	hFileTmp = xrtOpen((str)sPathTmp, FALSE, XRT_CP_BINARY);
	if ( hFileTmp == NULL ) {
		xpkFreeInternal(sPathTmp);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}
	iRet = procXpkMapFileReadWrite(objXpk, hFileTmp, iBound, &objMap);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	if ( iAlg == XPK_ALG_LZ4 ) {
		iCompRet = LZ4_compress_fast((const char*)pData, (char*)objMap.pView, (int)iSize, (int)iBound, procXpkCompLevelToNative(iLevel));
	} else {
		iCompRet = LZ4_compress_HC((const char*)pData, (char*)objMap.pView, (int)iSize, (int)iBound, procXpkCompLevelToNative(iLevel));
	}
	if ( iCompRet <= 0 ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_IO, (iAlg == XPK_ALG_LZ4) ? "lz4 compress failed" : "lz4hc compress failed");
		goto lblCleanup;
	}
	iCompSize = (uint32_t)iCompRet;
	if ( iCompSize >= iSize ) {
		procXpkUnmapFile(&objMap);
		iRet = XPK_OK;
		goto lblFallback;
	}

	procXpkUnmapFile(&objMap);
	iRet = procXpkSeekFile(objXpk, hFileTmp, iCompSize);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}
	if ( !xrtSetEOF(hFileTmp) ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		goto lblCleanup;
	}

	iRet = procXpkReadAtAlloc(objXpk, hFileTmp, 0, iCompSize, &pCompData);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	iRet = procXpkQueueWrite(objXpk, pEntry->iPos, iLevel, pCompData, iCompSize, iSize);
	if ( iRet != XPK_OK ) {
		pCompData = NULL;
		goto lblCleanup;
	}
	pCompData = NULL;

	pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_COMP_MASK) | iLevel;
	pEntry->iFileHash = iHash;
	pEntry->iDataSize = iCompSize;
	pEntry->iFileSize = iSize;
	procXpkClearError(objXpk);
	iRet = XPK_OK;
	goto lblCleanup;

lblFallback:
	if ( hFileTmp != NULL ) {
		xrtClose(hFileTmp);
		hFileTmp = NULL;
	}
	if ( sPathTmp != NULL ) {
		if ( xrtFileExists((str)sPathTmp) ) {
			(void)xrtFileDelete((str)sPathTmp);
		}
		xpkFreeInternal(sPathTmp);
		sPathTmp = NULL;
	}
	return procXpkWriteBuffered(objXpk, pEntry, pData, iSize, 0);

lblCleanup:
	if ( pCompData != NULL ) {
		xpkFreeInternal(pCompData);
	}
	procXpkUnmapFile(&objMap);
	if ( hFileTmp != NULL ) {
		xrtClose(hFileTmp);
	}
	if ( sPathTmp != NULL ) {
		if ( xrtFileExists((str)sPathTmp) ) {
			(void)xrtFileDelete((str)sPathTmp);
		}
		xpkFreeInternal(sPathTmp);
	}
	return iRet;
}

static inline int procXpkWriteBufferedZstdData(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint32_t iSize, uint8_t iLevel)
{
	xfile hFileTmp;
	ZSTD_CCtx* pCtx;
	ZSTD_inBuffer objIn;
	ZSTD_outBuffer objOut;
	char* sPathTmp;
	void* pOutBuf;
	void* pCompData;
	const uint8_t* pCur;
	uint64_t iCompSize64;
	uint32_t iCompSize;
	uint32_t iHash;
	size_t iOutCap;
	size_t iChunkSize;
	size_t iSizeLeft;
	size_t iZstdRet;
	int bFallbackStore;
	int iRet;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pData == NULL && iSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( procXpkCompLevelToAlg(iLevel) != XPK_ALG_ZSTD ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( iSize == 0 ) {
		return procXpkWriteBuffered(objXpk, pEntry, pData, iSize, 0);
	}

	hFileTmp = NULL;
	pCtx = NULL;
	sPathTmp = NULL;
	pOutBuf = NULL;
	pCompData = NULL;
	iCompSize64 = 0;
	iCompSize = 0;
	iHash = xpkHash32Internal(pData, iSize);
	bFallbackStore = FALSE;

	sPathTmp = procXpkWriteTempPathDup(objXpk, ".zstd.data.queue");
	if ( sPathTmp == NULL ) {
		return xpkLastError(objXpk);
	}

	hFileTmp = xrtOpen((str)sPathTmp, FALSE, XRT_CP_BINARY);
	if ( hFileTmp == NULL ) {
		xpkFreeInternal(sPathTmp);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	iOutCap = ZSTD_CStreamOutSize();
	if ( iOutCap == 0 ) {
		iOutCap = XPK_CODEC_STREAM_CHUNK_SIZE;
	}

	pOutBuf = xpkAllocInternal(iOutCap);
	if ( pOutBuf == NULL ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		goto lblCleanup;
	}

	pCtx = ZSTD_createCCtx();
	if ( pCtx == NULL ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		goto lblCleanup;
	}

	ZSTD_CCtx_reset(pCtx, ZSTD_reset_session_only);
	ZSTD_CCtx_setParameter(pCtx, ZSTD_c_checksumFlag, 0);
	ZSTD_CCtx_setParameter(pCtx, ZSTD_c_strategy, (ZSTD_strategy)procXpkCompLevelToNative(iLevel));
	ZSTD_CCtx_setPledgedSrcSize(pCtx, (unsigned long long)iSize);

	pCur = (const uint8_t*)pData;
	iSizeLeft = iSize;
	while ( iSizeLeft > 0 ) {
		iChunkSize = (iSizeLeft > XPK_CODEC_STREAM_CHUNK_SIZE) ? XPK_CODEC_STREAM_CHUNK_SIZE : iSizeLeft;
		objIn.src = pCur;
		objIn.size = iChunkSize;
		objIn.pos = 0;
		while ( objIn.pos < objIn.size ) {
			objOut.dst = pOutBuf;
			objOut.size = iOutCap;
			objOut.pos = 0;
			iZstdRet = ZSTD_compressStream2(pCtx, &objOut, &objIn, ZSTD_e_continue);
			if ( ZSTD_isError(iZstdRet) ) {
				iRet = procXpkSetError(objXpk, XPK_ERR_IO, "zstd compress failed");
				goto lblCleanup;
			}
			if ( objOut.pos > 0 ) {
				if ( xrtPut(hFileTmp, (ptr)pOutBuf, objOut.pos) != objOut.pos ) {
					iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
					goto lblCleanup;
				}
				iCompSize64 += objOut.pos;
				if ( iCompSize64 >= iSize ) {
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
				iRet = procXpkSetError(objXpk, XPK_ERR_IO, "zstd compress failed");
				goto lblCleanup;
			}
			if ( objOut.pos > 0 ) {
				if ( xrtPut(hFileTmp, (ptr)pOutBuf, objOut.pos) != objOut.pos ) {
					iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
					goto lblCleanup;
				}
				iCompSize64 += objOut.pos;
				if ( iCompSize64 >= iSize ) {
					bFallbackStore = TRUE;
					break;
				}
			}
			if ( iZstdRet == 0 ) {
				break;
			}
		}
	}

	if ( bFallbackStore ) {
		iRet = XPK_OK;
		goto lblCleanup;
	}

	if ( !xrtSetEOF(hFileTmp) ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		goto lblCleanup;
	}

	iCompSize = (uint32_t)iCompSize64;
	iRet = procXpkReadAtAlloc(objXpk, hFileTmp, 0, iCompSize, &pCompData);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	iRet = procXpkQueueWrite(objXpk, pEntry->iPos, iLevel, pCompData, iCompSize, iSize);
	if ( iRet != XPK_OK ) {
		pCompData = NULL;
		goto lblCleanup;
	}
	pCompData = NULL;

	pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_COMP_MASK) | iLevel;
	pEntry->iFileHash = iHash;
	pEntry->iDataSize = iCompSize;
	pEntry->iFileSize = iSize;
	procXpkClearError(objXpk);
	iRet = XPK_OK;

lblCleanup:
	if ( pCompData != NULL ) {
		xpkFreeInternal(pCompData);
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
	if ( bFallbackStore ) {
		return procXpkWriteBuffered(objXpk, pEntry, pData, iSize, 0);
	}
	return iRet;
}


static inline int procXpkWriteImmediateLz4Data(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint32_t iSize, uint8_t iLevel)
{
	xfile hFileTmp;
	xfile hFileDst;
	xpkMappedFile objMap;
	char* sPathTmp;
	uint32_t iBound;
	uint32_t iCompSize;
	uint32_t iHash;
	int iAlg;
	int iCompRet;
	int iRet;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pData == NULL && iSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( objXpk->bSolidApplied ) {
		return procXpkSetError(objXpk, XPK_ERR_SOLID_DATA_WRITE, "solid mode does not allow file data writes");
	}
	if ( objXpk->bReadonly ) {
		return procXpkSetError(objXpk, XPK_ERR_READONLY, sXpkErrorReadonly);
	}
	if ( procXpkWriteQueueCount(objXpk) > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBufferedPending);
	}
	iAlg = procXpkCompLevelToAlg(iLevel);
	if ( iAlg != XPK_ALG_LZ4 && iAlg != XPK_ALG_LZ4HC ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( iSize == 0 ) {
		return procXpkWriteImmediate(objXpk, pEntry, pData, iSize, 0);
	}
	if ( iSize > INT_MAX ) {
		return procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge);
	}

	hFileTmp = NULL;
	hFileDst = NULL;
	memset(&objMap, 0, sizeof(objMap));
	sPathTmp = NULL;
	iBound = 0;
	iCompSize = 0;
	iHash = xpkHash32Internal(pData, iSize);

	iBound = (uint32_t)LZ4_compressBound((int)iSize);
	sPathTmp = procXpkWriteTempPathDup(objXpk, ".lz4.data");
	if ( sPathTmp == NULL ) {
		return xpkLastError(objXpk);
	}

	hFileTmp = xrtOpen((str)sPathTmp, FALSE, XRT_CP_BINARY);
	if ( hFileTmp == NULL ) {
		xpkFreeInternal(sPathTmp);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}
	iRet = procXpkMapFileReadWrite(objXpk, hFileTmp, iBound, &objMap);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	if ( iAlg == XPK_ALG_LZ4 ) {
		iCompRet = LZ4_compress_fast((const char*)pData, (char*)objMap.pView, (int)iSize, (int)iBound, procXpkCompLevelToNative(iLevel));
	} else {
		iCompRet = LZ4_compress_HC((const char*)pData, (char*)objMap.pView, (int)iSize, (int)iBound, procXpkCompLevelToNative(iLevel));
	}
	if ( iCompRet <= 0 ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_IO, (iAlg == XPK_ALG_LZ4) ? "lz4 compress failed" : "lz4hc compress failed");
		goto lblCleanup;
	}
	iCompSize = (uint32_t)iCompRet;
	if ( iCompSize >= iSize ) {
		procXpkUnmapFile(&objMap);
		iRet = XPK_OK;
		goto lblFallback;
	}

	procXpkUnmapFile(&objMap);
	iRet = procXpkSeekFile(objXpk, hFileTmp, iCompSize);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}
	if ( !xrtSetEOF(hFileTmp) ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		goto lblCleanup;
	}

	if ( !procXpkAppliedVolumeMode(objXpk) ) {
		hFileDst = xrtOpen(objXpk->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFileDst == NULL ) {
			iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
			goto lblCleanup;
		}
	}

	iRet = procXpkCopySourceFileToPackage(objXpk, hFileTmp, hFileDst, objXpk->iAppendPos, iCompSize);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_COMP_MASK) | iLevel;
	pEntry->iFileHash = iHash;
	pEntry->iDataOffset = objXpk->iAppendPos;
	pEntry->iDataSize = iCompSize;
	pEntry->iFileSize = iSize;
	objXpk->iAppendPos += iCompSize;
	if ( objXpk->iAppendPos > objXpk->iFileSize ) {
		objXpk->iFileSize = objXpk->iAppendPos;
	}

	iRet = XPK_OK;
	goto lblCleanup;

lblFallback:
	if ( hFileTmp != NULL ) {
		xrtClose(hFileTmp);
		hFileTmp = NULL;
	}
	if ( sPathTmp != NULL ) {
		if ( xrtFileExists((str)sPathTmp) ) {
			(void)xrtFileDelete((str)sPathTmp);
		}
		xpkFreeInternal(sPathTmp);
		sPathTmp = NULL;
	}
	return procXpkWriteImmediate(objXpk, pEntry, pData, iSize, 0);

lblCleanup:
	procXpkUnmapFile(&objMap);
	if ( hFileDst != NULL ) {
		xrtClose(hFileDst);
	}
	if ( hFileTmp != NULL ) {
		xrtClose(hFileTmp);
	}
	if ( sPathTmp != NULL ) {
		if ( xrtFileExists((str)sPathTmp) ) {
			(void)xrtFileDelete((str)sPathTmp);
		}
		xpkFreeInternal(sPathTmp);
	}
	if ( iRet == XPK_OK ) {
		procXpkClearError(objXpk);
	}
	return iRet;
}

static inline int procXpkWriteImmediateZstdData(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint32_t iSize, uint8_t iLevel)
{
	xfile hFileTmp;
	xfile hFileDst;
	ZSTD_CCtx* pCtx;
	ZSTD_inBuffer objIn;
	ZSTD_outBuffer objOut;
	char* sPathTmp;
	void* pOutBuf;
	const uint8_t* pCur;
	uint64_t iCompSize64;
	uint32_t iCompSize;
	uint32_t iHash;
	size_t iOutCap;
	size_t iChunkSize;
	size_t iSizeLeft;
	size_t iZstdRet;
	int bFallbackStore;
	int iRet;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pData == NULL && iSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( objXpk->bSolidApplied ) {
		return procXpkSetError(objXpk, XPK_ERR_SOLID_DATA_WRITE, "solid mode does not allow file data writes");
	}
	if ( objXpk->bReadonly ) {
		return procXpkSetError(objXpk, XPK_ERR_READONLY, sXpkErrorReadonly);
	}
	if ( procXpkWriteQueueCount(objXpk) > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBufferedPending);
	}
	if ( procXpkCompLevelToAlg(iLevel) != XPK_ALG_ZSTD ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( iSize == 0 ) {
		return procXpkWriteImmediate(objXpk, pEntry, pData, iSize, 0);
	}

	hFileTmp = NULL;
	hFileDst = NULL;
	pCtx = NULL;
	sPathTmp = NULL;
	pOutBuf = NULL;
	iCompSize64 = 0;
	iCompSize = 0;
	iHash = xpkHash32Internal(pData, iSize);
	bFallbackStore = FALSE;

	sPathTmp = procXpkWriteTempPathDup(objXpk, ".zstd.data");
	if ( sPathTmp == NULL ) {
		return xpkLastError(objXpk);
	}

	hFileTmp = xrtOpen((str)sPathTmp, FALSE, XRT_CP_BINARY);
	if ( hFileTmp == NULL ) {
		xpkFreeInternal(sPathTmp);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	iOutCap = ZSTD_CStreamOutSize();
	if ( iOutCap == 0 ) {
		iOutCap = XPK_CODEC_STREAM_CHUNK_SIZE;
	}

	pOutBuf = xpkAllocInternal(iOutCap);
	if ( pOutBuf == NULL ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		goto lblCleanup;
	}

	pCtx = ZSTD_createCCtx();
	if ( pCtx == NULL ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		goto lblCleanup;
	}

	ZSTD_CCtx_reset(pCtx, ZSTD_reset_session_only);
	ZSTD_CCtx_setParameter(pCtx, ZSTD_c_checksumFlag, 0);
	ZSTD_CCtx_setParameter(pCtx, ZSTD_c_strategy, (ZSTD_strategy)procXpkCompLevelToNative(iLevel));
	ZSTD_CCtx_setPledgedSrcSize(pCtx, (unsigned long long)iSize);

	pCur = (const uint8_t*)pData;
	iSizeLeft = iSize;
	while ( iSizeLeft > 0 ) {
		iChunkSize = (iSizeLeft > XPK_CODEC_STREAM_CHUNK_SIZE) ? XPK_CODEC_STREAM_CHUNK_SIZE : iSizeLeft;
		objIn.src = pCur;
		objIn.size = iChunkSize;
		objIn.pos = 0;
		while ( objIn.pos < objIn.size ) {
			objOut.dst = pOutBuf;
			objOut.size = iOutCap;
			objOut.pos = 0;
			iZstdRet = ZSTD_compressStream2(pCtx, &objOut, &objIn, ZSTD_e_continue);
			if ( ZSTD_isError(iZstdRet) ) {
				iRet = procXpkSetError(objXpk, XPK_ERR_IO, "zstd compress failed");
				goto lblCleanup;
			}
			if ( objOut.pos > 0 ) {
				if ( xrtPut(hFileTmp, (ptr)pOutBuf, objOut.pos) != objOut.pos ) {
					iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
					goto lblCleanup;
				}
				iCompSize64 += objOut.pos;
				if ( iCompSize64 >= iSize ) {
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
				iRet = procXpkSetError(objXpk, XPK_ERR_IO, "zstd compress failed");
				goto lblCleanup;
			}
			if ( objOut.pos > 0 ) {
				if ( xrtPut(hFileTmp, (ptr)pOutBuf, objOut.pos) != objOut.pos ) {
					iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
					goto lblCleanup;
				}
				iCompSize64 += objOut.pos;
				if ( iCompSize64 >= iSize ) {
					bFallbackStore = TRUE;
					break;
				}
			}
			if ( iZstdRet == 0 ) {
				break;
			}
		}
	}

	if ( bFallbackStore ) {
		iRet = XPK_OK;
		goto lblCleanup;
	}

	if ( !xrtSetEOF(hFileTmp) ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		goto lblCleanup;
	}

	if ( !procXpkAppliedVolumeMode(objXpk) ) {
		hFileDst = xrtOpen(objXpk->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFileDst == NULL ) {
			iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
			goto lblCleanup;
		}
	}

	iCompSize = (uint32_t)iCompSize64;
	iRet = procXpkCopySourceFileToPackage(objXpk, hFileTmp, hFileDst, objXpk->iAppendPos, iCompSize);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_COMP_MASK) | iLevel;
	pEntry->iFileHash = iHash;
	pEntry->iDataOffset = objXpk->iAppendPos;
	pEntry->iDataSize = iCompSize;
	pEntry->iFileSize = iSize;
	objXpk->iAppendPos += iCompSize;
	if ( objXpk->iAppendPos > objXpk->iFileSize ) {
		objXpk->iFileSize = objXpk->iAppendPos;
	}

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
	if ( bFallbackStore ) {
		return procXpkWriteImmediate(objXpk, pEntry, pData, iSize, 0);
	}
	if ( iRet == XPK_OK ) {
		procXpkClearError(objXpk);
	}
	return iRet;
}

typedef struct xpkWriteLzmaSeqIn {
	ISeqInStream vt;
	xfile hFile;
	uint64_t iRemain;
} xpkWriteLzmaSeqIn;

typedef struct xpkWriteLzmaSeqOut {
	ISeqOutStream vt;
	xfile hFile;
	uint64_t iSize;
} xpkWriteLzmaSeqOut;

typedef struct xpkWriteLzmaMemIn {
	ISeqInStream vt;
	const uint8_t* pData;
	uint64_t iSize;
	uint64_t iPos;
} xpkWriteLzmaMemIn;

static inline SRes procXpkWriteLzmaSeqInRead(ISeqInStreamPtr pStream, void* pData, size_t* pSize)
{
	xpkWriteLzmaSeqIn* pIn;
	size_t iWant;
	size_t iRead;

	if ( pStream == NULL || pSize == NULL ) {
		return SZ_ERROR_PARAM;
	}

	pIn = (xpkWriteLzmaSeqIn*)pStream;
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

static inline size_t procXpkWriteLzmaSeqOutWrite(ISeqOutStreamPtr pStream, const void* pData, size_t iSize)
{
	xpkWriteLzmaSeqOut* pOut;
	size_t iWrite;

	if ( pStream == NULL ) {
		return 0;
	}
	if ( iSize == 0 ) {
		return 0;
	}

	pOut = (xpkWriteLzmaSeqOut*)pStream;
	iWrite = xrtPut(pOut->hFile, (ptr)pData, iSize);
	pOut->iSize += iWrite;
	return iWrite;
}

static inline SRes procXpkWriteLzmaMemInRead(ISeqInStreamPtr pStream, void* pData, size_t* pSize)
{
	xpkWriteLzmaMemIn* pIn;
	size_t iWant;

	if ( pStream == NULL || pSize == NULL ) {
		return SZ_ERROR_PARAM;
	}

	pIn = (xpkWriteLzmaMemIn*)pStream;
	iWant = *pSize;
	if ( iWant == 0 ) {
		return SZ_OK;
	}
	if ( pIn->iPos >= pIn->iSize ) {
		*pSize = 0;
		return SZ_OK;
	}
	if ( (pIn->iSize - pIn->iPos) < (uint64_t)iWant ) {
		iWant = (size_t)(pIn->iSize - pIn->iPos);
	}
	if ( iWant == 0 ) {
		*pSize = 0;
		return SZ_OK;
	}

	memcpy(pData, pIn->pData + (size_t)pIn->iPos, iWant);
	pIn->iPos += iWant;
	*pSize = iWant;
	return SZ_OK;
}

static inline int procXpkWriteBufferedLzma2Data(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint32_t iSize, uint8_t iLevel)
{
	xfile hFileTmp;
	CLzma2EncHandle hLzma2;
	CLzma2EncProps objProps;
	xpkWriteLzmaMemIn objIn;
	xpkWriteLzmaSeqOut objOut;
	char* sPathTmp;
	void* pCompData;
	uint64_t iCompSize64;
	uint32_t iCompSize;
	uint32_t iHash;
	Byte iPropByte;
	SRes iLzmaRes;
	int bFallbackStore;
	int iRet;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pData == NULL && iSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( procXpkCompLevelToAlg(iLevel) != XPK_ALG_LZMA2 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( iSize == 0 ) {
		return procXpkWriteBuffered(objXpk, pEntry, pData, iSize, 0);
	}

	hFileTmp = NULL;
	hLzma2 = NULL;
	sPathTmp = NULL;
	pCompData = NULL;
	iCompSize64 = 0;
	iCompSize = 0;
	iHash = xpkHash32Internal(pData, iSize);
	bFallbackStore = FALSE;

	sPathTmp = procXpkWriteTempPathDup(objXpk, ".lzma2.data.queue");
	if ( sPathTmp == NULL ) {
		return xpkLastError(objXpk);
	}

	hFileTmp = xrtOpen((str)sPathTmp, FALSE, XRT_CP_BINARY);
	if ( hFileTmp == NULL ) {
		xpkFreeInternal(sPathTmp);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	hLzma2 = Lzma2Enc_Create(&g_Alloc, &g_BigAlloc);
	if ( hLzma2 == NULL ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		goto lblCleanup;
	}

	Lzma2EncProps_Init(&objProps);
	objProps.lzmaProps.level = procXpkCompLevelToNative(iLevel);
	iLzmaRes = Lzma2Enc_SetProps(hLzma2, &objProps);
	if ( iLzmaRes != SZ_OK ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_IO, "lzma2 set props failed");
		goto lblCleanup;
	}
	Lzma2Enc_SetDataSize(hLzma2, (UInt64)iSize);

	iPropByte = Lzma2Enc_WriteProperties(hLzma2);
	if ( xrtPut(hFileTmp, (ptr)&iPropByte, 1) != 1 ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		goto lblCleanup;
	}

	memset(&objIn, 0, sizeof(objIn));
	objIn.vt.Read = procXpkWriteLzmaMemInRead;
	objIn.pData = (const uint8_t*)pData;
	objIn.iSize = iSize;
	objIn.iPos = 0;

	memset(&objOut, 0, sizeof(objOut));
	objOut.vt.Write = procXpkWriteLzmaSeqOutWrite;
	objOut.hFile = hFileTmp;
	objOut.iSize = 1;

	iLzmaRes = Lzma2Enc_Encode2(hLzma2, &objOut.vt, NULL, NULL, &objIn.vt, NULL, 0, NULL);
	if ( iLzmaRes != SZ_OK ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_IO, "lzma2 encode failed");
		goto lblCleanup;
	}

	iCompSize64 = objOut.iSize;
	if ( iCompSize64 >= iSize ) {
		bFallbackStore = TRUE;
		iRet = XPK_OK;
		goto lblCleanup;
	}

	if ( !xrtSetEOF(hFileTmp) ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		goto lblCleanup;
	}

	iCompSize = (uint32_t)iCompSize64;
	iRet = procXpkReadAtAlloc(objXpk, hFileTmp, 0, iCompSize, &pCompData);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	iRet = procXpkQueueWrite(objXpk, pEntry->iPos, iLevel, pCompData, iCompSize, iSize);
	if ( iRet != XPK_OK ) {
		pCompData = NULL;
		goto lblCleanup;
	}
	pCompData = NULL;

	pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_COMP_MASK) | iLevel;
	pEntry->iFileHash = iHash;
	pEntry->iDataSize = iCompSize;
	pEntry->iFileSize = iSize;
	procXpkClearError(objXpk);
	iRet = XPK_OK;

lblCleanup:
	if ( pCompData != NULL ) {
		xpkFreeInternal(pCompData);
	}
	if ( hFileTmp != NULL ) {
		xrtClose(hFileTmp);
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
	if ( bFallbackStore ) {
		return procXpkWriteBuffered(objXpk, pEntry, pData, iSize, 0);
	}
	return iRet;
}

static inline int procXpkWriteImmediateLzma2Data(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint32_t iSize, uint8_t iLevel)
{
	xfile hFileTmp;
	xfile hFileDst;
	CLzma2EncHandle hLzma2;
	CLzma2EncProps objProps;
	xpkWriteLzmaMemIn objIn;
	xpkWriteLzmaSeqOut objOut;
	char* sPathTmp;
	uint64_t iCompSize64;
	uint32_t iCompSize;
	uint32_t iHash;
	Byte iPropByte;
	SRes iLzmaRes;
	int bFallbackStore;
	int iRet;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pData == NULL && iSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( objXpk->bSolidApplied ) {
		return procXpkSetError(objXpk, XPK_ERR_SOLID_DATA_WRITE, "solid mode does not allow file data writes");
	}
	if ( objXpk->bReadonly ) {
		return procXpkSetError(objXpk, XPK_ERR_READONLY, sXpkErrorReadonly);
	}
	if ( procXpkWriteQueueCount(objXpk) > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBufferedPending);
	}
	if ( procXpkCompLevelToAlg(iLevel) != XPK_ALG_LZMA2 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( iSize == 0 ) {
		return procXpkWriteImmediate(objXpk, pEntry, pData, iSize, 0);
	}

	hFileTmp = NULL;
	hFileDst = NULL;
	hLzma2 = NULL;
	sPathTmp = NULL;
	iCompSize64 = 0;
	iCompSize = 0;
	iHash = xpkHash32Internal(pData, iSize);
	bFallbackStore = FALSE;

	sPathTmp = procXpkWriteTempPathDup(objXpk, ".lzma2.data");
	if ( sPathTmp == NULL ) {
		return xpkLastError(objXpk);
	}

	hFileTmp = xrtOpen((str)sPathTmp, FALSE, XRT_CP_BINARY);
	if ( hFileTmp == NULL ) {
		xpkFreeInternal(sPathTmp);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	hLzma2 = Lzma2Enc_Create(&g_Alloc, &g_BigAlloc);
	if ( hLzma2 == NULL ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		goto lblCleanup;
	}

	Lzma2EncProps_Init(&objProps);
	objProps.lzmaProps.level = procXpkCompLevelToNative(iLevel);
	iLzmaRes = Lzma2Enc_SetProps(hLzma2, &objProps);
	if ( iLzmaRes != SZ_OK ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_IO, "lzma2 set props failed");
		goto lblCleanup;
	}
	Lzma2Enc_SetDataSize(hLzma2, (UInt64)iSize);

	iPropByte = Lzma2Enc_WriteProperties(hLzma2);
	if ( xrtPut(hFileTmp, (ptr)&iPropByte, 1) != 1 ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		goto lblCleanup;
	}

	objIn.vt.Read = procXpkWriteLzmaMemInRead;
	objIn.pData = (const uint8_t*)pData;
	objIn.iSize = iSize;
	objIn.iPos = 0;
	objOut.vt.Write = procXpkWriteLzmaSeqOutWrite;
	objOut.hFile = hFileTmp;
	objOut.iSize = 0;

	iLzmaRes = Lzma2Enc_Encode2(hLzma2, &objOut.vt, NULL, NULL, &objIn.vt, NULL, 0, NULL);
	if ( iLzmaRes != SZ_OK ) {
		if ( iLzmaRes == SZ_ERROR_WRITE ) {
			iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		} else {
			iRet = procXpkSetError(objXpk, XPK_ERR_IO, "lzma2 compress failed");
		}
		goto lblCleanup;
	}

	iCompSize64 = 1 + objOut.iSize;
	if ( iCompSize64 >= iSize ) {
		bFallbackStore = TRUE;
		iRet = XPK_OK;
		goto lblCleanup;
	}

	if ( !xrtSetEOF(hFileTmp) ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		goto lblCleanup;
	}

	if ( !procXpkAppliedVolumeMode(objXpk) ) {
		hFileDst = xrtOpen(objXpk->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFileDst == NULL ) {
			iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
			goto lblCleanup;
		}
	}

	iCompSize = (uint32_t)iCompSize64;
	iRet = procXpkCopySourceFileToPackage(objXpk, hFileTmp, hFileDst, objXpk->iAppendPos, iCompSize);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_COMP_MASK) | iLevel;
	pEntry->iFileHash = iHash;
	pEntry->iDataOffset = objXpk->iAppendPos;
	pEntry->iDataSize = iCompSize;
	pEntry->iFileSize = iSize;
	objXpk->iAppendPos += iCompSize;
	if ( objXpk->iAppendPos > objXpk->iFileSize ) {
		objXpk->iFileSize = objXpk->iAppendPos;
	}

	iRet = XPK_OK;

lblCleanup:
	if ( hFileDst != NULL ) {
		xrtClose(hFileDst);
	}
	if ( hFileTmp != NULL ) {
		xrtClose(hFileTmp);
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
	if ( bFallbackStore ) {
		return procXpkWriteImmediate(objXpk, pEntry, pData, iSize, 0);
	}
	if ( iRet == XPK_OK ) {
		procXpkClearError(objXpk);
	}
	return iRet;
}

static inline int procXpkWriteRawTempFileFromSource(xpkObject objXpk, const char* sSrcPath, const char* sTag, xfile* pFileTmpRet, char** pPathTmpRet, uint64_t* pFileSizeRet, uint32_t* pHashRet)
{
	xfile hFileSrc;
	xfile hFileTmp;
	char* sPathTmp;
	void* pChunk;
	uint64_t iFileSize;
	uint64_t iOffsetRead;
	uint32_t iChunkRead;
	size_t iRead;
	uint32_t iHash;
	int iRet;

	if ( pFileTmpRet != NULL ) {
		*pFileTmpRet = NULL;
	}
	if ( pPathTmpRet != NULL ) {
		*pPathTmpRet = NULL;
	}
	if ( pFileSizeRet != NULL ) {
		*pFileSizeRet = 0;
	}
	if ( pHashRet != NULL ) {
		*pHashRet = 0;
	}
	if ( objXpk == NULL || sSrcPath == NULL || sSrcPath[0] == '\0' || sTag == NULL || sTag[0] == '\0' ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pFileTmpRet == NULL || pPathTmpRet == NULL || pFileSizeRet == NULL || pHashRet == NULL ) {
		return procXpkReturnParamError(objXpk);
	}

	hFileSrc = NULL;
	hFileTmp = NULL;
	sPathTmp = NULL;
	pChunk = NULL;
	iFileSize = 0;
	iOffsetRead = 0;
	iHash = 0;

	hFileSrc = xrtOpen((str)sSrcPath, TRUE, XRT_CP_BINARY);
	if ( hFileSrc == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	iFileSize = xrtGetEOF(hFileSrc);
	if ( iFileSize == 0 ) {
		xrtClose(hFileSrc);
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	sPathTmp = procXpkWriteTempPathDup(objXpk, sTag);
	if ( sPathTmp == NULL ) {
		xrtClose(hFileSrc);
		return xpkLastError(objXpk);
	}

	hFileTmp = xrtOpen((str)sPathTmp, FALSE, XRT_CP_BINARY);
	if ( hFileTmp == NULL ) {
		xpkFreeInternal(sPathTmp);
		xrtClose(hFileSrc);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	pChunk = xpkAllocInternal(XPK_WRITE_FILE_CHUNK_SIZE);
	if ( pChunk == NULL ) {
		xrtClose(hFileTmp);
		(void)xrtFileDelete((str)sPathTmp);
		xpkFreeInternal(sPathTmp);
		xrtClose(hFileSrc);
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	while ( iOffsetRead < iFileSize ) {
		iChunkRead = (uint32_t)(((iFileSize - iOffsetRead) > XPK_WRITE_FILE_CHUNK_SIZE) ? XPK_WRITE_FILE_CHUNK_SIZE : (iFileSize - iOffsetRead));
		iRead = xrtGetBuffer(hFileSrc, pChunk, iChunkRead);
		if ( iRead != iChunkRead ) {
			xpkFreeInternal(pChunk);
			xrtClose(hFileTmp);
			(void)xrtFileDelete((str)sPathTmp);
			xpkFreeInternal(sPathTmp);
			xrtClose(hFileSrc);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoRead);
		}
		if ( xrtPut(hFileTmp, (ptr)pChunk, iChunkRead) != iChunkRead ) {
			xpkFreeInternal(pChunk);
			xrtClose(hFileTmp);
			(void)xrtFileDelete((str)sPathTmp);
			xpkFreeInternal(sPathTmp);
			xrtClose(hFileSrc);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		}
		iOffsetRead += iChunkRead;
	}

	xpkFreeInternal(pChunk);
	xrtClose(hFileSrc);
	hFileSrc = NULL;

	if ( !xrtSetEOF(hFileTmp) ) {
		xrtClose(hFileTmp);
		(void)xrtFileDelete((str)sPathTmp);
		xpkFreeInternal(sPathTmp);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
	}

	iRet = procXpkHashMappedFileRange(objXpk, hFileTmp, 0, iFileSize, &iHash);
	if ( iRet != XPK_OK ) {
		xrtClose(hFileTmp);
		(void)xrtFileDelete((str)sPathTmp);
		xpkFreeInternal(sPathTmp);
		return iRet;
	}

	*pFileTmpRet = hFileTmp;
	*pPathTmpRet = sPathTmp;
	*pFileSizeRet = iFileSize;
	*pHashRet = iHash;
	return XPK_OK;
}

static inline int procXpkEncodeMappedLz4Block(xpkObject objXpk, uint8_t iLevel, const xpkMappedFile* pMap, void** pCompDataRet, uint32_t* pCompSizeRet)
{
	void* pCompData;
	uint32_t iBound;
	uint32_t iAlg;
	int iCompSize;

	if ( pCompDataRet != NULL ) {
		*pCompDataRet = NULL;
	}
	if ( pCompSizeRet != NULL ) {
		*pCompSizeRet = 0;
	}
	if ( objXpk == NULL || pMap == NULL || pMap->pView == NULL || pCompDataRet == NULL || pCompSizeRet == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pMap->iSize > UINT32_MAX ) {
		return procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge);
	}

	iAlg = procXpkCompLevelToAlg(iLevel);
	if ( iAlg != XPK_ALG_LZ4 && iAlg != XPK_ALG_LZ4HC ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	iBound = (uint32_t)LZ4_compressBound((int)pMap->iSize);
	pCompData = xpkAllocInternal(iBound);
	if ( pCompData == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	if ( iAlg == XPK_ALG_LZ4 ) {
		iCompSize = LZ4_compress_fast((const char*)pMap->pView, (char*)pCompData, (int)pMap->iSize, (int)iBound, procXpkCompLevelToNative(iLevel));
	} else {
		iCompSize = LZ4_compress_HC((const char*)pMap->pView, (char*)pCompData, (int)pMap->iSize, (int)iBound, procXpkCompLevelToNative(iLevel));
	}
	if ( iCompSize <= 0 ) {
		xpkFreeInternal(pCompData);
		return procXpkSetError(objXpk, XPK_ERR_IO, "lz4 compress failed");
	}

	*pCompDataRet = pCompData;
	*pCompSizeRet = (uint32_t)iCompSize;
	return XPK_OK;
}

static inline int procXpkWriteImmediateZstdFile(xpkObject objXpk, xpkEntry* pEntry, const char* sSrcPath, uint8_t iLevel)
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
	uint64_t iFileSize;
	uint64_t iCompSize;
	uint64_t iOffsetRead;
	uint32_t iHash;
	size_t iInCap;
	size_t iOutCap;
	size_t iRead;
	size_t iZstdRet;
	uint32_t iChunkRead;
	int bFallbackStore;
	int iRet;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( sSrcPath == NULL || sSrcPath[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( objXpk->bSolidApplied ) {
		return procXpkSetError(objXpk, XPK_ERR_SOLID_DATA_WRITE, "solid mode does not allow file data writes");
	}
	if ( objXpk->bReadonly ) {
		return procXpkSetError(objXpk, XPK_ERR_READONLY, sXpkErrorReadonly);
	}
	if ( procXpkWriteQueueCount(objXpk) > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBufferedPending);
	}
	if ( procXpkCompLevelToAlg(iLevel) != XPK_ALG_ZSTD ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	hFileSrc = NULL;
	hFileTmp = NULL;
	hFileDst = NULL;
	pCtx = NULL;
	sPathTmp = NULL;
	pInBuf = NULL;
	pOutBuf = NULL;
	iFileSize = 0;
	iCompSize = 0;
	iOffsetRead = 0;
	iHash = 0;
	bFallbackStore = FALSE;

	hFileSrc = xrtOpen((str)sSrcPath, TRUE, XRT_CP_BINARY);
	if ( hFileSrc == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	iFileSize = xrtGetEOF(hFileSrc);
	if ( iFileSize == 0 ) {
		xrtClose(hFileSrc);
		return procXpkWriteImmediateStoreFile(objXpk, pEntry, sSrcPath);
	}

	sPathTmp = procXpkWriteTempPathDup(objXpk, ".zstd.file");
	if ( sPathTmp == NULL ) {
		xrtClose(hFileSrc);
		return xpkLastError(objXpk);
	}

	hFileTmp = xrtOpen((str)sPathTmp, FALSE, XRT_CP_BINARY);
	if ( hFileTmp == NULL ) {
		xpkFreeInternal(sPathTmp);
		xrtClose(hFileSrc);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
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
		iRet = procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		goto lblCleanup;
	}
	pOutBuf = xpkAllocInternal(iOutCap);
	if ( pOutBuf == NULL ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		goto lblCleanup;
	}

	pCtx = ZSTD_createCCtx();
	if ( pCtx == NULL ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		goto lblCleanup;
	}

	ZSTD_CCtx_reset(pCtx, ZSTD_reset_session_only);
	ZSTD_CCtx_setParameter(pCtx, ZSTD_c_checksumFlag, 0);
	ZSTD_CCtx_setParameter(pCtx, ZSTD_c_strategy, (ZSTD_strategy)procXpkCompLevelToNative(iLevel));
	ZSTD_CCtx_setPledgedSrcSize(pCtx, (unsigned long long)iFileSize);

	while ( iOffsetRead < iFileSize ) {
		iChunkRead = (uint32_t)(((iFileSize - iOffsetRead) > (uint64_t)iInCap) ? iInCap : (iFileSize - iOffsetRead));
		if ( iChunkRead == 0 ) {
			break;
		}

		iRead = xrtGetBuffer(hFileSrc, pInBuf, iChunkRead);
		if ( iRead != iChunkRead ) {
			iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoRead);
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
				iRet = procXpkSetError(objXpk, XPK_ERR_IO, "zstd compress failed");
				goto lblCleanup;
			}
			if ( objOut.pos > 0 ) {
				if ( xrtPut(hFileTmp, (ptr)pOutBuf, objOut.pos) != objOut.pos ) {
					iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
					goto lblCleanup;
				}
				iCompSize += objOut.pos;
				if ( iCompSize >= iFileSize ) {
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
				iRet = procXpkSetError(objXpk, XPK_ERR_IO, "zstd compress failed");
				goto lblCleanup;
			}
			if ( objOut.pos > 0 ) {
				if ( xrtPut(hFileTmp, (ptr)pOutBuf, objOut.pos) != objOut.pos ) {
					iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
					goto lblCleanup;
				}
				iCompSize += objOut.pos;
				if ( iCompSize >= iFileSize ) {
					bFallbackStore = TRUE;
					break;
				}
			}
			if ( iZstdRet == 0 ) {
				break;
			}
		}
	}

	if ( bFallbackStore ) {
		iRet = XPK_OK;
		goto lblCleanup;
	}

	if ( !xrtSetEOF(hFileTmp) ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		goto lblCleanup;
	}

	iRet = procXpkHashMappedFileRange(objXpk, hFileSrc, 0, iFileSize, &iHash);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	if ( !procXpkAppliedVolumeMode(objXpk) ) {
		hFileDst = xrtOpen(objXpk->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFileDst == NULL ) {
			iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
			goto lblCleanup;
		}
	}

	iRet = procXpkCopySourceFileToPackage(objXpk, hFileTmp, hFileDst, objXpk->iAppendPos, iCompSize);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_COMP_MASK) | iLevel;
	pEntry->iFileHash = iHash;
	pEntry->iDataOffset = objXpk->iAppendPos;
	pEntry->iDataSize = iCompSize;
	pEntry->iFileSize = iFileSize;
	objXpk->iAppendPos += iCompSize;
	if ( objXpk->iAppendPos > objXpk->iFileSize ) {
		objXpk->iFileSize = objXpk->iAppendPos;
	}

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
	if ( bFallbackStore ) {
		return procXpkWriteImmediateStoreFile(objXpk, pEntry, sSrcPath);
	}
	if ( iRet == XPK_OK ) {
		procXpkClearError(objXpk);
	}
	return iRet;
}

static inline int procXpkWriteImmediateLz4File(xpkObject objXpk, xpkEntry* pEntry, const char* sSrcPath, uint8_t iLevel)
{
	xfile hFileTmp;
	xfile hFileDst;
	xpkMappedFile objMap;
	char* sPathTmp;
	void* pCompData;
	uint64_t iFileSize;
	uint32_t iHash;
	uint32_t iCompSize;
	int iRet;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( sSrcPath == NULL || sSrcPath[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( objXpk->bSolidApplied ) {
		return procXpkSetError(objXpk, XPK_ERR_SOLID_DATA_WRITE, "solid mode does not allow file data writes");
	}
	if ( objXpk->bReadonly ) {
		return procXpkSetError(objXpk, XPK_ERR_READONLY, sXpkErrorReadonly);
	}
	if ( procXpkWriteQueueCount(objXpk) > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBufferedPending);
	}
	if ( procXpkCompLevelToAlg(iLevel) != XPK_ALG_LZ4 && procXpkCompLevelToAlg(iLevel) != XPK_ALG_LZ4HC ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	hFileTmp = NULL;
	hFileDst = NULL;
	memset(&objMap, 0, sizeof(objMap));
	sPathTmp = NULL;
	pCompData = NULL;
	iFileSize = 0;
	iHash = 0;
	iCompSize = 0;

	iRet = procXpkWriteRawTempFileFromSource(objXpk, sSrcPath, ".lz4.file", &hFileTmp, &sPathTmp, &iFileSize, &iHash);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( iFileSize > UINT32_MAX ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge);
		goto lblCleanup;
	}

	iRet = procXpkMapFileReadOnly(objXpk, hFileTmp, iFileSize, &objMap);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	iRet = procXpkEncodeMappedLz4Block(objXpk, iLevel, &objMap, &pCompData, &iCompSize);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	if ( iCompSize >= iFileSize ) {
		if ( !procXpkAppliedVolumeMode(objXpk) ) {
			hFileDst = xrtOpen(objXpk->sPathPackage, FALSE, XRT_CP_BINARY);
			if ( hFileDst == NULL ) {
				iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
				goto lblCleanup;
			}
		}
		iRet = procXpkCopySourceFileToPackage(objXpk, hFileTmp, hFileDst, objXpk->iAppendPos, iFileSize);
		if ( iRet != XPK_OK ) {
			goto lblCleanup;
		}

		pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_COMP_MASK);
		pEntry->iFileHash = iHash;
		pEntry->iDataOffset = objXpk->iAppendPos;
		pEntry->iDataSize = iFileSize;
		pEntry->iFileSize = iFileSize;
		objXpk->iAppendPos += iFileSize;
	} else {
		if ( !procXpkAppliedVolumeMode(objXpk) ) {
			hFileDst = xrtOpen(objXpk->sPathPackage, FALSE, XRT_CP_BINARY);
			if ( hFileDst == NULL ) {
				iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
				goto lblCleanup;
			}
		}
		iRet = procXpkWriteAtChunkedPackage(objXpk, hFileDst, objXpk->iAppendPos, pCompData, iCompSize);
		if ( iRet != XPK_OK ) {
			goto lblCleanup;
		}

		pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_COMP_MASK) | iLevel;
		pEntry->iFileHash = iHash;
		pEntry->iDataOffset = objXpk->iAppendPos;
		pEntry->iDataSize = iCompSize;
		pEntry->iFileSize = iFileSize;
		objXpk->iAppendPos += iCompSize;
	}
	if ( objXpk->iAppendPos > objXpk->iFileSize ) {
		objXpk->iFileSize = objXpk->iAppendPos;
	}

	iRet = XPK_OK;

lblCleanup:
	if ( pCompData != NULL ) {
		xpkFreeInternal(pCompData);
	}
	procXpkUnmapFile(&objMap);
	if ( hFileDst != NULL ) {
		xrtClose(hFileDst);
	}
	if ( hFileTmp != NULL ) {
		xrtClose(hFileTmp);
	}
	if ( sPathTmp != NULL ) {
		if ( xrtFileExists((str)sPathTmp) ) {
			(void)xrtFileDelete((str)sPathTmp);
		}
		xpkFreeInternal(sPathTmp);
	}
	if ( iRet == XPK_OK ) {
		procXpkClearError(objXpk);
	}
	return iRet;
}

static inline int procXpkWriteImmediateLzma2File(xpkObject objXpk, xpkEntry* pEntry, const char* sSrcPath, uint8_t iLevel)
{
	xfile hFileSrc;
	xfile hFileTmp;
	xfile hFileDst;
	CLzma2EncHandle hLzma2;
	CLzma2EncProps objProps;
	xpkWriteLzmaSeqIn objIn;
	xpkWriteLzmaSeqOut objOut;
	char* sPathTmp;
	uint64_t iFileSize;
	uint64_t iCompSize64;
	uint32_t iCompSize;
	uint32_t iHash;
	Byte iPropByte;
	SRes iLzmaRes;
	int bFallbackStore;
	int iRet;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( sSrcPath == NULL || sSrcPath[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( objXpk->bSolidApplied ) {
		return procXpkSetError(objXpk, XPK_ERR_SOLID_DATA_WRITE, "solid mode does not allow file data writes");
	}
	if ( objXpk->bReadonly ) {
		return procXpkSetError(objXpk, XPK_ERR_READONLY, sXpkErrorReadonly);
	}
	if ( procXpkWriteQueueCount(objXpk) > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBufferedPending);
	}
	if ( procXpkCompLevelToAlg(iLevel) != XPK_ALG_LZMA2 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	hFileSrc = NULL;
	hFileTmp = NULL;
	hFileDst = NULL;
	hLzma2 = NULL;
	sPathTmp = NULL;
	iFileSize = 0;
	iCompSize64 = 0;
	iCompSize = 0;
	iHash = 0;
	bFallbackStore = FALSE;

	hFileSrc = xrtOpen((str)sSrcPath, TRUE, XRT_CP_BINARY);
	if ( hFileSrc == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	iFileSize = xrtGetEOF(hFileSrc);
	if ( iFileSize == 0 ) {
		xrtClose(hFileSrc);
		return procXpkWriteImmediateStoreFile(objXpk, pEntry, sSrcPath);
	}

	sPathTmp = procXpkWriteTempPathDup(objXpk, ".lzma2.file");
	if ( sPathTmp == NULL ) {
		xrtClose(hFileSrc);
		return xpkLastError(objXpk);
	}

	hFileTmp = xrtOpen((str)sPathTmp, FALSE, XRT_CP_BINARY);
	if ( hFileTmp == NULL ) {
		xpkFreeInternal(sPathTmp);
		xrtClose(hFileSrc);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	hLzma2 = Lzma2Enc_Create(&g_Alloc, &g_BigAlloc);
	if ( hLzma2 == NULL ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		goto lblCleanup;
	}

	Lzma2EncProps_Init(&objProps);
	objProps.lzmaProps.level = procXpkCompLevelToNative(iLevel);
	iLzmaRes = Lzma2Enc_SetProps(hLzma2, &objProps);
	if ( iLzmaRes != SZ_OK ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_IO, "lzma2 set props failed");
		goto lblCleanup;
	}
	Lzma2Enc_SetDataSize(hLzma2, (UInt64)iFileSize);

	iPropByte = Lzma2Enc_WriteProperties(hLzma2);
	if ( xrtPut(hFileTmp, (ptr)&iPropByte, 1) != 1 ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		goto lblCleanup;
	}

	objIn.vt.Read = procXpkWriteLzmaSeqInRead;
	objIn.hFile = hFileSrc;
	objIn.iRemain = iFileSize;
	objOut.vt.Write = procXpkWriteLzmaSeqOutWrite;
	objOut.hFile = hFileTmp;
	objOut.iSize = 0;

	iLzmaRes = Lzma2Enc_Encode2(hLzma2, &objOut.vt, NULL, NULL, &objIn.vt, NULL, 0, NULL);
	if ( iLzmaRes != SZ_OK ) {
		if ( iLzmaRes == SZ_ERROR_READ ) {
			iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoRead);
		} else if ( iLzmaRes == SZ_ERROR_WRITE ) {
			iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		} else {
			iRet = procXpkSetError(objXpk, XPK_ERR_IO, "lzma2 compress failed");
		}
		goto lblCleanup;
	}

	iCompSize64 = 1 + objOut.iSize;
	if ( iCompSize64 >= iFileSize ) {
		bFallbackStore = TRUE;
		iRet = XPK_OK;
		goto lblCleanup;
	}

	if ( !xrtSetEOF(hFileTmp) ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		goto lblCleanup;
	}

	iRet = procXpkHashMappedFileRange(objXpk, hFileSrc, 0, iFileSize, &iHash);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	if ( !procXpkAppliedVolumeMode(objXpk) ) {
		hFileDst = xrtOpen(objXpk->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFileDst == NULL ) {
			iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
			goto lblCleanup;
		}
	}

	iCompSize = (uint32_t)iCompSize64;
	iRet = procXpkCopySourceFileToPackage(objXpk, hFileTmp, hFileDst, objXpk->iAppendPos, iCompSize);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_COMP_MASK) | iLevel;
	pEntry->iFileHash = iHash;
	pEntry->iDataOffset = objXpk->iAppendPos;
	pEntry->iDataSize = iCompSize;
	pEntry->iFileSize = iFileSize;
	objXpk->iAppendPos += iCompSize;
	if ( objXpk->iAppendPos > objXpk->iFileSize ) {
		objXpk->iFileSize = objXpk->iAppendPos;
	}

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
	if ( bFallbackStore ) {
		return procXpkWriteImmediateStoreFile(objXpk, pEntry, sSrcPath);
	}
	if ( iRet == XPK_OK ) {
		procXpkClearError(objXpk);
	}
	return iRet;
}

static inline int procXpkWriteBufferedZstdFile(xpkObject objXpk, xpkEntry* pEntry, const char* sSrcPath, uint8_t iLevel)
{
	xfile hFileSrc;
	xfile hFileTmp;
	ZSTD_CCtx* pCtx;
	ZSTD_inBuffer objIn;
	ZSTD_outBuffer objOut;
	char* sPathTmp;
	void* pInBuf;
	void* pOutBuf;
	void* pCompData;
	void* pRawData;
	uint64_t iFileSize;
	uint64_t iCompSize64;
	uint64_t iOffsetRead;
	uint32_t iHash;
	uint32_t iCompSize;
	uint32_t iChunkRead;
	size_t iInCap;
	size_t iOutCap;
	size_t iRead;
	size_t iZstdRet;
	int bFallbackStore;
	int iRet;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( sSrcPath == NULL || sSrcPath[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( objXpk->bSolidApplied ) {
		return procXpkSetError(objXpk, XPK_ERR_SOLID_DATA_WRITE, "solid mode does not allow file data writes");
	}
	if ( objXpk->bReadonly ) {
		return procXpkSetError(objXpk, XPK_ERR_READONLY, sXpkErrorReadonly);
	}
	if ( procXpkCompLevelToAlg(iLevel) != XPK_ALG_ZSTD ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	hFileSrc = NULL;
	hFileTmp = NULL;
	pCtx = NULL;
	sPathTmp = NULL;
	pInBuf = NULL;
	pOutBuf = NULL;
	pCompData = NULL;
	pRawData = NULL;
	iFileSize = 0;
	iCompSize64 = 0;
	iOffsetRead = 0;
	iHash = 0;
	iCompSize = 0;
	bFallbackStore = FALSE;

	hFileSrc = xrtOpen((str)sSrcPath, TRUE, XRT_CP_BINARY);
	if ( hFileSrc == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	iFileSize = xrtGetEOF(hFileSrc);
	if ( iFileSize > UINT32_MAX ) {
		xrtClose(hFileSrc);
		return procXpkWriteImmediateStoreFileWithQueuedFallback(objXpk, pEntry, sSrcPath);
	}
	if ( iFileSize == 0 ) {
		xrtClose(hFileSrc);
		return procXpkWriteBuffered(objXpk, pEntry, NULL, 0, iLevel);
	}

	sPathTmp = procXpkWriteTempPathDup(objXpk, ".zstd.queue");
	if ( sPathTmp == NULL ) {
		xrtClose(hFileSrc);
		return xpkLastError(objXpk);
	}

	hFileTmp = xrtOpen((str)sPathTmp, FALSE, XRT_CP_BINARY);
	if ( hFileTmp == NULL ) {
		xpkFreeInternal(sPathTmp);
		xrtClose(hFileSrc);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
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
		iRet = procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		goto lblCleanup;
	}
	pOutBuf = xpkAllocInternal(iOutCap);
	if ( pOutBuf == NULL ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		goto lblCleanup;
	}

	pCtx = ZSTD_createCCtx();
	if ( pCtx == NULL ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		goto lblCleanup;
	}

	ZSTD_CCtx_reset(pCtx, ZSTD_reset_session_only);
	ZSTD_CCtx_setParameter(pCtx, ZSTD_c_checksumFlag, 0);
	ZSTD_CCtx_setParameter(pCtx, ZSTD_c_strategy, (ZSTD_strategy)procXpkCompLevelToNative(iLevel));
	ZSTD_CCtx_setPledgedSrcSize(pCtx, (unsigned long long)iFileSize);

	while ( iOffsetRead < iFileSize ) {
		iChunkRead = (uint32_t)(((iFileSize - iOffsetRead) > (uint64_t)iInCap) ? iInCap : (iFileSize - iOffsetRead));
		if ( iChunkRead == 0 ) {
			break;
		}

		iRead = xrtGetBuffer(hFileSrc, pInBuf, iChunkRead);
		if ( iRead != iChunkRead ) {
			iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoRead);
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
				iRet = procXpkSetError(objXpk, XPK_ERR_IO, "zstd compress failed");
				goto lblCleanup;
			}
			if ( objOut.pos > 0 ) {
				if ( xrtPut(hFileTmp, (ptr)pOutBuf, objOut.pos) != objOut.pos ) {
					iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
					goto lblCleanup;
				}
				iCompSize64 += objOut.pos;
				if ( iCompSize64 >= iFileSize ) {
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
				iRet = procXpkSetError(objXpk, XPK_ERR_IO, "zstd compress failed");
				goto lblCleanup;
			}
			if ( objOut.pos > 0 ) {
				if ( xrtPut(hFileTmp, (ptr)pOutBuf, objOut.pos) != objOut.pos ) {
					iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
					goto lblCleanup;
				}
				iCompSize64 += objOut.pos;
				if ( iCompSize64 >= iFileSize ) {
					bFallbackStore = TRUE;
					break;
				}
			}
			if ( iZstdRet == 0 ) {
				break;
			}
		}
	}

	if ( bFallbackStore ) {
		pRawData = xpkAllocInternal((size_t)iFileSize);
		if ( pRawData == NULL ) {
			iRet = procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
			goto lblCleanup;
		}
		iRet = procXpkSeekFile(objXpk, hFileSrc, 0);
		if ( iRet != XPK_OK ) {
			goto lblCleanup;
		}
		iRead = xrtGetBuffer(hFileSrc, pRawData, (size_t)iFileSize);
		if ( iRead != (size_t)iFileSize ) {
			iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoRead);
			goto lblCleanup;
		}

		iHash = xpkHash32Internal(pRawData, iFileSize);
		iRet = procXpkQueueWrite(objXpk, pEntry->iPos, 0, pRawData, (uint32_t)iFileSize, iFileSize);
		if ( iRet != XPK_OK ) {
			pRawData = NULL;
			goto lblCleanup;
		}
		pRawData = NULL;

		pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_COMP_MASK);
		pEntry->iFileHash = iHash;
		pEntry->iDataSize = iFileSize;
		pEntry->iFileSize = iFileSize;
		procXpkClearError(objXpk);
		iRet = XPK_OK;
		goto lblCleanup;
	}

	if ( !xrtSetEOF(hFileTmp) ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		goto lblCleanup;
	}

	iRet = procXpkHashMappedFileRange(objXpk, hFileSrc, 0, iFileSize, &iHash);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	iCompSize = (uint32_t)iCompSize64;
	iRet = procXpkReadAtAlloc(objXpk, hFileTmp, 0, iCompSize, &pCompData);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	iRet = procXpkQueueWrite(objXpk, pEntry->iPos, iLevel, pCompData, iCompSize, iFileSize);
	if ( iRet != XPK_OK ) {
		pCompData = NULL;
		goto lblCleanup;
	}
	pCompData = NULL;

	pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_COMP_MASK) | iLevel;
	pEntry->iFileHash = iHash;
	pEntry->iDataSize = iCompSize;
	pEntry->iFileSize = iFileSize;
	procXpkClearError(objXpk);
	iRet = XPK_OK;

lblCleanup:
	if ( pCompData != NULL ) {
		xpkFreeInternal(pCompData);
	}
	if ( pRawData != NULL ) {
		xpkFreeInternal(pRawData);
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

static inline int procXpkWriteBufferedLzma2File(xpkObject objXpk, xpkEntry* pEntry, const char* sSrcPath, uint8_t iLevel)
{
	xfile hFileSrc;
	xfile hFileTmp;
	CLzma2EncHandle hLzma2;
	CLzma2EncProps objProps;
	xpkWriteLzmaSeqIn objIn;
	xpkWriteLzmaSeqOut objOut;
	char* sPathTmp;
	void* pCompData;
	void* pRawData;
	uint64_t iFileSize;
	uint64_t iCompSize64;
	uint32_t iHash;
	uint32_t iCompSize;
	Byte iPropByte;
	size_t iRead;
	SRes iLzmaRes;
	int bFallbackStore;
	int iRet;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( sSrcPath == NULL || sSrcPath[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( objXpk->bSolidApplied ) {
		return procXpkSetError(objXpk, XPK_ERR_SOLID_DATA_WRITE, "solid mode does not allow file data writes");
	}
	if ( objXpk->bReadonly ) {
		return procXpkSetError(objXpk, XPK_ERR_READONLY, sXpkErrorReadonly);
	}
	if ( procXpkCompLevelToAlg(iLevel) != XPK_ALG_LZMA2 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	hFileSrc = NULL;
	hFileTmp = NULL;
	hLzma2 = NULL;
	sPathTmp = NULL;
	pCompData = NULL;
	pRawData = NULL;
	iFileSize = 0;
	iCompSize64 = 0;
	iHash = 0;
	iCompSize = 0;
	bFallbackStore = FALSE;

	hFileSrc = xrtOpen((str)sSrcPath, TRUE, XRT_CP_BINARY);
	if ( hFileSrc == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	iFileSize = xrtGetEOF(hFileSrc);
	if ( iFileSize > UINT32_MAX ) {
		xrtClose(hFileSrc);
		return procXpkWriteImmediateStoreFileWithQueuedFallback(objXpk, pEntry, sSrcPath);
	}
	if ( iFileSize == 0 ) {
		xrtClose(hFileSrc);
		return procXpkWriteBuffered(objXpk, pEntry, NULL, 0, iLevel);
	}

	sPathTmp = procXpkWriteTempPathDup(objXpk, ".lzma2.queue");
	if ( sPathTmp == NULL ) {
		xrtClose(hFileSrc);
		return xpkLastError(objXpk);
	}

	hFileTmp = xrtOpen((str)sPathTmp, FALSE, XRT_CP_BINARY);
	if ( hFileTmp == NULL ) {
		xpkFreeInternal(sPathTmp);
		xrtClose(hFileSrc);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	hLzma2 = Lzma2Enc_Create(&g_Alloc, &g_BigAlloc);
	if ( hLzma2 == NULL ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		goto lblCleanup;
	}

	Lzma2EncProps_Init(&objProps);
	objProps.lzmaProps.level = procXpkCompLevelToNative(iLevel);
	iLzmaRes = Lzma2Enc_SetProps(hLzma2, &objProps);
	if ( iLzmaRes != SZ_OK ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_IO, "lzma2 set props failed");
		goto lblCleanup;
	}
	Lzma2Enc_SetDataSize(hLzma2, (UInt64)iFileSize);

	iPropByte = Lzma2Enc_WriteProperties(hLzma2);
	if ( xrtPut(hFileTmp, (ptr)&iPropByte, 1) != 1 ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		goto lblCleanup;
	}

	objIn.vt.Read = procXpkWriteLzmaSeqInRead;
	objIn.hFile = hFileSrc;
	objIn.iRemain = iFileSize;
	objOut.vt.Write = procXpkWriteLzmaSeqOutWrite;
	objOut.hFile = hFileTmp;
	objOut.iSize = 0;

	iLzmaRes = Lzma2Enc_Encode2(hLzma2, &objOut.vt, NULL, NULL, &objIn.vt, NULL, 0, NULL);
	if ( iLzmaRes != SZ_OK ) {
		if ( iLzmaRes == SZ_ERROR_READ ) {
			iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoRead);
		} else if ( iLzmaRes == SZ_ERROR_WRITE ) {
			iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		} else {
			iRet = procXpkSetError(objXpk, XPK_ERR_IO, "lzma2 compress failed");
		}
		goto lblCleanup;
	}

	iCompSize64 = 1 + objOut.iSize;
	if ( iCompSize64 >= iFileSize ) {
		bFallbackStore = TRUE;
	}

	if ( bFallbackStore ) {
		pRawData = xpkAllocInternal((size_t)iFileSize);
		if ( pRawData == NULL ) {
			iRet = procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
			goto lblCleanup;
		}
		iRet = procXpkSeekFile(objXpk, hFileSrc, 0);
		if ( iRet != XPK_OK ) {
			goto lblCleanup;
		}
		iRead = xrtGetBuffer(hFileSrc, pRawData, (size_t)iFileSize);
		if ( iRead != (size_t)iFileSize ) {
			iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoRead);
			goto lblCleanup;
		}

		iHash = xpkHash32Internal(pRawData, iFileSize);
		iRet = procXpkQueueWrite(objXpk, pEntry->iPos, 0, pRawData, (uint32_t)iFileSize, iFileSize);
		if ( iRet != XPK_OK ) {
			pRawData = NULL;
			goto lblCleanup;
		}
		pRawData = NULL;

		pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_COMP_MASK);
		pEntry->iFileHash = iHash;
		pEntry->iDataSize = iFileSize;
		pEntry->iFileSize = iFileSize;
		procXpkClearError(objXpk);
		iRet = XPK_OK;
		goto lblCleanup;
	}

	if ( !xrtSetEOF(hFileTmp) ) {
		iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		goto lblCleanup;
	}

	iRet = procXpkHashMappedFileRange(objXpk, hFileSrc, 0, iFileSize, &iHash);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	iCompSize = (uint32_t)iCompSize64;
	iRet = procXpkReadAtAlloc(objXpk, hFileTmp, 0, iCompSize, &pCompData);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	iRet = procXpkQueueWrite(objXpk, pEntry->iPos, iLevel, pCompData, iCompSize, iFileSize);
	if ( iRet != XPK_OK ) {
		pCompData = NULL;
		goto lblCleanup;
	}
	pCompData = NULL;

	pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_COMP_MASK) | iLevel;
	pEntry->iFileHash = iHash;
	pEntry->iDataSize = iCompSize;
	pEntry->iFileSize = iFileSize;
	procXpkClearError(objXpk);
	iRet = XPK_OK;

lblCleanup:
	if ( pCompData != NULL ) {
		xpkFreeInternal(pCompData);
	}
	if ( pRawData != NULL ) {
		xpkFreeInternal(pRawData);
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

static inline int procXpkWriteBufferedLz4File(xpkObject objXpk, xpkEntry* pEntry, const char* sSrcPath, uint8_t iLevel)
{
	xfile hFileTmp;
	xpkMappedFile objMap;
	char* sPathTmp;
	void* pCompData;
	void* pRawData;
	uint64_t iFileSize;
	uint32_t iHash;
	uint32_t iCompSize;
	int iRet;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( sSrcPath == NULL || sSrcPath[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( objXpk->bSolidApplied ) {
		return procXpkSetError(objXpk, XPK_ERR_SOLID_DATA_WRITE, "solid mode does not allow file data writes");
	}
	if ( objXpk->bReadonly ) {
		return procXpkSetError(objXpk, XPK_ERR_READONLY, sXpkErrorReadonly);
	}
	if ( procXpkCompLevelToAlg(iLevel) != XPK_ALG_LZ4 && procXpkCompLevelToAlg(iLevel) != XPK_ALG_LZ4HC ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	hFileTmp = NULL;
	memset(&objMap, 0, sizeof(objMap));
	sPathTmp = NULL;
	pCompData = NULL;
	pRawData = NULL;
	iFileSize = 0;
	iHash = 0;
	iCompSize = 0;

	iRet = procXpkWriteRawTempFileFromSource(objXpk, sSrcPath, ".lz4.queue", &hFileTmp, &sPathTmp, &iFileSize, &iHash);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( iFileSize > UINT32_MAX ) {
		iRet = procXpkWriteImmediateStoreFileWithQueuedFallback(objXpk, pEntry, sSrcPath);
		goto lblCleanup;
	}

	iRet = procXpkMapFileReadOnly(objXpk, hFileTmp, iFileSize, &objMap);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	iRet = procXpkEncodeMappedLz4Block(objXpk, iLevel, &objMap, &pCompData, &iCompSize);
	if ( iRet != XPK_OK ) {
		goto lblCleanup;
	}

	if ( iCompSize >= iFileSize ) {
		pRawData = xpkAllocInternal((size_t)iFileSize);
		if ( pRawData == NULL ) {
			iRet = procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
			goto lblCleanup;
		}
		memcpy(pRawData, objMap.pView, (size_t)iFileSize);

		iRet = procXpkQueueWrite(objXpk, pEntry->iPos, 0, pRawData, (uint32_t)iFileSize, iFileSize);
		if ( iRet != XPK_OK ) {
			pRawData = NULL;
			goto lblCleanup;
		}
		pRawData = NULL;

		pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_COMP_MASK);
		pEntry->iFileHash = iHash;
		pEntry->iDataSize = iFileSize;
		pEntry->iFileSize = iFileSize;
		procXpkClearError(objXpk);
		iRet = XPK_OK;
		goto lblCleanup;
	}

	iRet = procXpkQueueWrite(objXpk, pEntry->iPos, iLevel, pCompData, iCompSize, iFileSize);
	if ( iRet != XPK_OK ) {
		pCompData = NULL;
		goto lblCleanup;
	}
	pCompData = NULL;

	pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_COMP_MASK) | iLevel;
	pEntry->iFileHash = iHash;
	pEntry->iDataSize = iCompSize;
	pEntry->iFileSize = iFileSize;
	procXpkClearError(objXpk);
	iRet = XPK_OK;

lblCleanup:
	if ( pCompData != NULL ) {
		xpkFreeInternal(pCompData);
	}
	if ( pRawData != NULL ) {
		xpkFreeInternal(pRawData);
	}
	procXpkUnmapFile(&objMap);
	if ( hFileTmp != NULL ) {
		xrtClose(hFileTmp);
	}
	if ( sPathTmp != NULL ) {
		if ( xrtFileExists((str)sPathTmp) ) {
			(void)xrtFileDelete((str)sPathTmp);
		}
		xpkFreeInternal(sPathTmp);
	}
	return iRet;
}

static inline int procXpkWriteImmediateStoreFile(xpkObject objXpk, xpkEntry* pEntry, const char* sSrcPath)
{
	xfile hFileSrc;
	xfile hFileDst;
	xpkMappedFile objMap;
	uint64_t iFileSize;
	uint32_t iHash;
	const void* pHashData;
	int iRet;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( sSrcPath == NULL || sSrcPath[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( objXpk->bSolidApplied ) {
		return procXpkSetError(objXpk, XPK_ERR_SOLID_DATA_WRITE, "solid mode does not allow file data writes");
	}
	if ( objXpk->bReadonly ) {
		return procXpkSetError(objXpk, XPK_ERR_READONLY, sXpkErrorReadonly);
	}
	if ( procXpkWriteQueueCount(objXpk) > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBufferedPending);
	}

	hFileSrc = NULL;
	memset(&objMap, 0, sizeof(objMap));
	iFileSize = 0;
	hFileSrc = xrtOpen((str)sSrcPath, TRUE, XRT_CP_BINARY);
	if ( hFileSrc == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}
	iFileSize = xrtGetEOF(hFileSrc);
	if ( !procXpkAppliedVolumeMode(objXpk) ) {
		hFileDst = xrtOpen(objXpk->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFileDst == NULL ) {
			xrtClose(hFileSrc);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
		}

		iRet = procXpkCopySourceFileToPackage(objXpk, hFileSrc, hFileDst, objXpk->iAppendPos, iFileSize);
		if ( iRet == XPK_OK ) {
			if ( iFileSize == 0 ) {
				iHash = 0;
			} else {
				iRet = procXpkHashMappedFileRange(objXpk, hFileDst, objXpk->iAppendPos, iFileSize, &iHash);
			}
		}
		xrtClose(hFileDst);
		xrtClose(hFileSrc);
		if ( iRet != XPK_OK ) {
			return iRet;
		}

		pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_COMP_MASK);
		pEntry->iFileHash = iHash;
		pEntry->iDataOffset = objXpk->iAppendPos;
		pEntry->iDataSize = iFileSize;
		pEntry->iFileSize = iFileSize;
		objXpk->iAppendPos += iFileSize;
		if ( objXpk->iAppendPos > objXpk->iFileSize ) {
			objXpk->iFileSize = objXpk->iAppendPos;
		}

		procXpkClearError(objXpk);
		return XPK_OK;
	}

	iRet = procXpkMapFileReadOnly(objXpk, hFileSrc, iFileSize, &objMap);
	if ( iRet != XPK_OK ) {
		xrtClose(hFileSrc);
		return iRet;
	}

	hFileDst = NULL;
	if ( !procXpkAppliedVolumeMode(objXpk) ) {
		hFileDst = xrtOpen(objXpk->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFileDst == NULL ) {
			procXpkUnmapFile(&objMap);
			xrtClose(hFileSrc);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
		}
	}

	iRet = procXpkWriteAtChunkedPackage(objXpk, hFileDst, objXpk->iAppendPos, objMap.pView, iFileSize);
	if ( hFileDst != NULL ) {
		xrtClose(hFileDst);
	}
	if ( iRet != XPK_OK ) {
		procXpkUnmapFile(&objMap);
		xrtClose(hFileSrc);
		return iRet;
	}

	if ( iFileSize == 0 ) {
		iHash = 0;
	} else {
		pHashData = (objMap.pView != NULL) ? objMap.pView : "";
		iHash = xpkHash32Internal(pHashData, iFileSize);
	}
	pEntry->iFlag = (pEntry->iFlag & ~XPK_FLAG_COMP_MASK);
	pEntry->iFileHash = iHash;
	pEntry->iDataOffset = objXpk->iAppendPos;
	pEntry->iDataSize = iFileSize;
	pEntry->iFileSize = iFileSize;
	objXpk->iAppendPos += iFileSize;
	if ( objXpk->iAppendPos > objXpk->iFileSize ) {
		objXpk->iFileSize = objXpk->iAppendPos;
	}

	procXpkUnmapFile(&objMap);
	xrtClose(hFileSrc);
	procXpkClearError(objXpk);
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

static inline int procXpkReadSolidLz4SliceAlloc(xpkObject objXpk, xpkEntry* pEntry, void** pDataRet, uint64_t* pSizeRet)
{
	xfile hFileSrc;
	xpkMappedFile objMap;
	void* pCompData;
	void* pPrefixBuf;
	void* pSliceBuf;
	const void* pCompPtr;
	uint64_t iPkgSize;
	uint64_t iSolidSize;
	uint64_t iSliceEnd;
	uint32_t iCompSize;
	int iDecRet;
	int iRet;

	if ( pDataRet != NULL ) {
		*pDataRet = NULL;
	}
	if ( pSizeRet != NULL ) {
		*pSizeRet = 0;
	}
	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pEntry->iFileSize == 0 ) {
		pSliceBuf = xpkAllocInternal(1);
		if ( pSliceBuf == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
		if ( pDataRet != NULL ) {
			*pDataRet = pSliceBuf;
		}
		if ( pSizeRet != NULL ) {
			*pSizeRet = 0;
		}
		procXpkClearError(objXpk);
		return XPK_OK;
	}

	iRet = procXpkCalcSolidRawSize(objXpk, &iSolidSize);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( (pEntry->iDataOffset + pEntry->iFileSize) > iSolidSize ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( objXpk->objHead.dataOffset < XPK_HEAD_SIZE ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	iCompSize = (uint32_t)(objXpk->objHead.dataOffset - XPK_HEAD_SIZE);
	if ( iCompSize == 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	iSliceEnd = pEntry->iDataOffset + pEntry->iFileSize;
	if ( iSliceEnd > 2147483647ULL || iCompSize > 2147483647U ) {
		return procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge);
	}

	hFileSrc = NULL;
	memset(&objMap, 0, sizeof(objMap));
	pCompData = NULL;
	pCompPtr = NULL;
	pPrefixBuf = NULL;
	pSliceBuf = NULL;

	if ( !procXpkAppliedVolumeMode(objXpk) ) {
		hFileSrc = xrtOpen(objXpk->sPathPackage, TRUE, XRT_CP_BINARY);
		if ( hFileSrc == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
		}
		iPkgSize = xrtGetEOF(hFileSrc);
		if ( objXpk->objHead.dataOffset > iPkgSize ) {
			xrtClose(hFileSrc);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
		iRet = procXpkMapFileReadOnly(objXpk, hFileSrc, iPkgSize, &objMap);
		xrtClose(hFileSrc);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		pCompPtr = (const uint8_t*)objMap.pView + XPK_HEAD_SIZE;
	} else {
		iRet = procXpkReadAtAlloc(objXpk, NULL, XPK_HEAD_SIZE, iCompSize, &pCompData);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		pCompPtr = pCompData;
	}

	pPrefixBuf = xpkAllocInternal((size_t)iSliceEnd);
	if ( pPrefixBuf == NULL ) {
		if ( pCompData != NULL ) {
			xpkFreeInternal(pCompData);
		}
		procXpkUnmapFile(&objMap);
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	iDecRet = LZ4_decompress_safe_partial((const char*)pCompPtr, (char*)pPrefixBuf, (int)iCompSize, (int)iSliceEnd, (int)iSliceEnd);
	if ( iDecRet != (int)iSliceEnd ) {
		xpkFreeInternal(pPrefixBuf);
		if ( pCompData != NULL ) {
			xpkFreeInternal(pCompData);
		}
		procXpkUnmapFile(&objMap);
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, "lz4 decompress failed");
	}

	pSliceBuf = xpkAllocInternal((size_t)pEntry->iFileSize);
	if ( pSliceBuf == NULL ) {
		xpkFreeInternal(pPrefixBuf);
		if ( pCompData != NULL ) {
			xpkFreeInternal(pCompData);
		}
		procXpkUnmapFile(&objMap);
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	memcpy(pSliceBuf, (const uint8_t*)pPrefixBuf + (size_t)pEntry->iDataOffset, (size_t)pEntry->iFileSize);
	xpkFreeInternal(pPrefixBuf);
	if ( pCompData != NULL ) {
		xpkFreeInternal(pCompData);
	}
	procXpkUnmapFile(&objMap);

	if ( pDataRet != NULL ) {
		*pDataRet = pSliceBuf;
	}
	if ( pSizeRet != NULL ) {
		*pSizeRet = pEntry->iFileSize;
	}
	procXpkClearError(objXpk);
	return XPK_OK;
}

static inline int procXpkCopySolidLz4EntryToFile(xpkObject objXpk, xpkEntry* pEntry, const char* sPathFile)
{
	xfile hFileSrc;
	xpkMappedFile objMap;
	void* pCompData;
	void* pPrefixBuf;
	const void* pCompPtr;
	uint64_t iPkgSize;
	uint64_t iSolidSize;
	uint64_t iSliceEnd;
	uint32_t iCompSize;
	int iDecRet;
	int iRet;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( sPathFile == NULL || sPathFile[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( pEntry->iFileSize == 0 ) {
		return procXpkWriteFileData(objXpk, sPathFile, NULL, 0);
	}

	iRet = procXpkCalcSolidRawSize(objXpk, &iSolidSize);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( (pEntry->iDataOffset + pEntry->iFileSize) > iSolidSize ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( objXpk->objHead.dataOffset < XPK_HEAD_SIZE ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	iCompSize = (uint32_t)(objXpk->objHead.dataOffset - XPK_HEAD_SIZE);
	if ( iCompSize == 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	iSliceEnd = pEntry->iDataOffset + pEntry->iFileSize;
	if ( iSliceEnd > 2147483647ULL || iCompSize > 2147483647U ) {
		return procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge);
	}

	hFileSrc = NULL;
	memset(&objMap, 0, sizeof(objMap));
	pCompData = NULL;
	pCompPtr = NULL;
	pPrefixBuf = NULL;

	if ( !procXpkAppliedVolumeMode(objXpk) ) {
		hFileSrc = xrtOpen(objXpk->sPathPackage, TRUE, XRT_CP_BINARY);
		if ( hFileSrc == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
		}
		iPkgSize = xrtGetEOF(hFileSrc);
		if ( objXpk->objHead.dataOffset > iPkgSize ) {
			xrtClose(hFileSrc);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
		iRet = procXpkMapFileReadOnly(objXpk, hFileSrc, iPkgSize, &objMap);
		xrtClose(hFileSrc);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		pCompPtr = (const uint8_t*)objMap.pView + XPK_HEAD_SIZE;
	} else {
		iRet = procXpkReadAtAlloc(objXpk, NULL, XPK_HEAD_SIZE, iCompSize, &pCompData);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		pCompPtr = pCompData;
	}

	pPrefixBuf = xpkAllocInternal((size_t)iSliceEnd);
	if ( pPrefixBuf == NULL ) {
		if ( pCompData != NULL ) {
			xpkFreeInternal(pCompData);
		}
		procXpkUnmapFile(&objMap);
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	iDecRet = LZ4_decompress_safe_partial((const char*)pCompPtr, (char*)pPrefixBuf, (int)iCompSize, (int)iSliceEnd, (int)iSliceEnd);
	if ( iDecRet != (int)iSliceEnd ) {
		xpkFreeInternal(pPrefixBuf);
		if ( pCompData != NULL ) {
			xpkFreeInternal(pCompData);
		}
		procXpkUnmapFile(&objMap);
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, "lz4 decompress failed");
	}

	iRet = procXpkWriteFileData(objXpk, sPathFile, (const uint8_t*)pPrefixBuf + (size_t)pEntry->iDataOffset, pEntry->iFileSize);
	xpkFreeInternal(pPrefixBuf);
	if ( pCompData != NULL ) {
		xpkFreeInternal(pCompData);
	}
	procXpkUnmapFile(&objMap);
	return iRet;
}

static inline int procXpkReadSolidZstdSliceAlloc(xpkObject objXpk, xpkEntry* pEntry, void** pDataRet, uint64_t* pSizeRet)
{
	xfile hFileSrc;
	uint64_t iPkgSize;
	uint64_t iSolidSize;
	uint64_t iOffsetRead;
	uint64_t iSizeLeft;
	uint64_t iRawPos;
	uint64_t iSliceWritten;
	uint64_t iSliceStart;
	uint64_t iSliceEnd;
	uint64_t iCopyStart;
	uint64_t iCopyEnd;
	ZSTD_DStream* pStream;
	ZSTD_inBuffer objIn;
	ZSTD_outBuffer objOut;
	void* pInBuf;
	void* pOutBuf;
	void* pSliceBuf;
	size_t iInCap;
	size_t iOutCap;
	size_t iZstdRet;
	uint32_t iCompSize;
	uint32_t iChunkRead;
	int iRet;

	if ( pDataRet != NULL ) {
		*pDataRet = NULL;
	}
	if ( pSizeRet != NULL ) {
		*pSizeRet = 0;
	}
	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pEntry->iFileSize == 0 ) {
		pSliceBuf = xpkAllocInternal(1);
		if ( pSliceBuf == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
		if ( pDataRet != NULL ) {
			*pDataRet = pSliceBuf;
		}
		if ( pSizeRet != NULL ) {
			*pSizeRet = 0;
		}
		procXpkClearError(objXpk);
		return XPK_OK;
	}

	iRet = procXpkCalcSolidRawSize(objXpk, &iSolidSize);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( (pEntry->iDataOffset + pEntry->iFileSize) > iSolidSize ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( objXpk->objHead.dataOffset < XPK_HEAD_SIZE ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	iCompSize = (uint32_t)(objXpk->objHead.dataOffset - XPK_HEAD_SIZE);
	if ( iCompSize == 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	hFileSrc = NULL;
	if ( !procXpkAppliedVolumeMode(objXpk) ) {
		hFileSrc = xrtOpen(objXpk->sPathPackage, TRUE, XRT_CP_BINARY);
		if ( hFileSrc == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
		}
		iPkgSize = xrtGetEOF(hFileSrc);
		if ( objXpk->objHead.dataOffset > iPkgSize ) {
			xrtClose(hFileSrc);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
	}

	pSliceBuf = xpkAllocInternal((size_t)pEntry->iFileSize);
	if ( pSliceBuf == NULL ) {
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	iInCap = ZSTD_DStreamInSize();
	if ( iInCap == 0 ) {
		iInCap = 131072u;
	}
	iOutCap = ZSTD_DStreamOutSize();
	if ( iOutCap == 0 ) {
		iOutCap = 131072u;
	}

	pInBuf = xpkAllocInternal(iInCap);
	if ( pInBuf == NULL ) {
		xpkFreeInternal(pSliceBuf);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}
	pOutBuf = xpkAllocInternal(iOutCap);
	if ( pOutBuf == NULL ) {
		xpkFreeInternal(pInBuf);
		xpkFreeInternal(pSliceBuf);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	pStream = ZSTD_createDStream();
	if ( pStream == NULL ) {
		xpkFreeInternal(pOutBuf);
		xpkFreeInternal(pInBuf);
		xpkFreeInternal(pSliceBuf);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	iZstdRet = ZSTD_initDStream(pStream);
	if ( ZSTD_isError(iZstdRet) ) {
		ZSTD_freeDStream(pStream);
		xpkFreeInternal(pOutBuf);
		xpkFreeInternal(pInBuf);
		xpkFreeInternal(pSliceBuf);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, "zstd decompress failed");
	}

	iOffsetRead = XPK_HEAD_SIZE;
	iSizeLeft = iCompSize;
	iRawPos = 0;
	iSliceWritten = 0;
	iSliceStart = pEntry->iDataOffset;
	iSliceEnd = pEntry->iDataOffset + pEntry->iFileSize;
	memset(&objIn, 0, sizeof(objIn));

	for ( ;; ) {
		if ( (objIn.pos == objIn.size) && (iSizeLeft > 0) ) {
			iChunkRead = (uint32_t)((iSizeLeft > (uint64_t)iInCap) ? iInCap : iSizeLeft);
			iRet = procXpkReadAtBuffer(objXpk, hFileSrc, iOffsetRead, pInBuf, iChunkRead);
			if ( iRet != XPK_OK ) {
				ZSTD_freeDStream(pStream);
				xpkFreeInternal(pOutBuf);
				xpkFreeInternal(pInBuf);
				xpkFreeInternal(pSliceBuf);
				if ( hFileSrc != NULL ) {
					xrtClose(hFileSrc);
				}
				return iRet;
			}

			objIn.src = pInBuf;
			objIn.size = iChunkRead;
			objIn.pos = 0;
			iOffsetRead += iChunkRead;
			iSizeLeft -= iChunkRead;
		} else if ( (objIn.pos == objIn.size) && (iSizeLeft == 0) ) {
			objIn.src = pInBuf;
			objIn.size = 0;
			objIn.pos = 0;
		}

		objOut.dst = pOutBuf;
		objOut.size = iOutCap;
		objOut.pos = 0;
		iZstdRet = ZSTD_decompressStream(pStream, &objOut, &objIn);
		if ( ZSTD_isError(iZstdRet) ) {
			ZSTD_freeDStream(pStream);
			xpkFreeInternal(pOutBuf);
			xpkFreeInternal(pInBuf);
			xpkFreeInternal(pSliceBuf);
			if ( hFileSrc != NULL ) {
				xrtClose(hFileSrc);
			}
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, "zstd decompress failed");
		}

		if ( objOut.pos > 0 ) {
			iCopyStart = (iRawPos > iSliceStart) ? iRawPos : iSliceStart;
			iCopyEnd = ((iRawPos + objOut.pos) < iSliceEnd) ? (iRawPos + objOut.pos) : iSliceEnd;
			if ( iCopyEnd > iCopyStart ) {
				memcpy((uint8_t*)pSliceBuf + (size_t)(iCopyStart - iSliceStart), (const uint8_t*)pOutBuf + (size_t)(iCopyStart - iRawPos), (size_t)(iCopyEnd - iCopyStart));
				iSliceWritten += (iCopyEnd - iCopyStart);
			}
			iRawPos += objOut.pos;
		}

		if ( (iSizeLeft == 0) && (objIn.pos == objIn.size) && (iZstdRet == 0) ) {
			break;
		}
		if ( (iSizeLeft == 0) && (objIn.pos == objIn.size) && (objOut.pos == 0) && (iZstdRet > 0) ) {
			ZSTD_freeDStream(pStream);
			xpkFreeInternal(pOutBuf);
			xpkFreeInternal(pInBuf);
			xpkFreeInternal(pSliceBuf);
			if ( hFileSrc != NULL ) {
				xrtClose(hFileSrc);
			}
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, "zstd decompress failed");
		}
	}

	ZSTD_freeDStream(pStream);
	xpkFreeInternal(pOutBuf);
	xpkFreeInternal(pInBuf);
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}

	if ( iRawPos != iSolidSize || iSliceWritten != pEntry->iFileSize ) {
		xpkFreeInternal(pSliceBuf);
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	if ( pDataRet != NULL ) {
		*pDataRet = pSliceBuf;
	}
	if ( pSizeRet != NULL ) {
		*pSizeRet = pEntry->iFileSize;
	}
	procXpkClearError(objXpk);
	return XPK_OK;
}

static inline int procXpkCopySolidZstdEntryToFile(xpkObject objXpk, xpkEntry* pEntry, const char* sPathFile)
{
	xfile hFileSrc;
	xfile hFileDst;
	uint64_t iPkgSize;
	uint64_t iSolidSize;
	uint64_t iOffsetRead;
	uint64_t iSizeLeft;
	uint64_t iRawPos;
	uint64_t iSliceWritten;
	uint64_t iSliceStart;
	uint64_t iSliceEnd;
	uint64_t iCopyStart;
	uint64_t iCopyEnd;
	ZSTD_DStream* pStream;
	ZSTD_inBuffer objIn;
	ZSTD_outBuffer objOut;
	void* pInBuf;
	void* pOutBuf;
	size_t iInCap;
	size_t iOutCap;
	size_t iZstdRet;
	uint32_t iCompSize;
	uint32_t iChunkRead;
	int iRet;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( sPathFile == NULL || sPathFile[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( pEntry->iFileSize == 0 ) {
		return procXpkWriteFileData(objXpk, sPathFile, NULL, 0);
	}

	iRet = procXpkCalcSolidRawSize(objXpk, &iSolidSize);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( (pEntry->iDataOffset + pEntry->iFileSize) > iSolidSize ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( objXpk->objHead.dataOffset < XPK_HEAD_SIZE ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	iCompSize = (uint32_t)(objXpk->objHead.dataOffset - XPK_HEAD_SIZE);
	if ( iCompSize == 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	hFileSrc = NULL;
	hFileDst = xrtOpen((str)sPathFile, FALSE, XRT_CP_BINARY);
	if ( hFileDst == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}
	if ( procXpkSeekFile(objXpk, hFileDst, 0) != XPK_OK ) {
		xrtClose(hFileDst);
		return xpkLastError(objXpk);
	}
	if ( !procXpkAppliedVolumeMode(objXpk) ) {
		hFileSrc = xrtOpen(objXpk->sPathPackage, TRUE, XRT_CP_BINARY);
		if ( hFileSrc == NULL ) {
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
		}
		iPkgSize = xrtGetEOF(hFileSrc);
		if ( objXpk->objHead.dataOffset > iPkgSize ) {
			xrtClose(hFileSrc);
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
	}

	iInCap = ZSTD_DStreamInSize();
	if ( iInCap == 0 ) {
		iInCap = 131072u;
	}
	iOutCap = ZSTD_DStreamOutSize();
	if ( iOutCap == 0 ) {
		iOutCap = 131072u;
	}
	pInBuf = xpkAllocInternal(iInCap);
	if ( pInBuf == NULL ) {
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}
	pOutBuf = xpkAllocInternal(iOutCap);
	if ( pOutBuf == NULL ) {
		xpkFreeInternal(pInBuf);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	pStream = ZSTD_createDStream();
	if ( pStream == NULL ) {
		xpkFreeInternal(pOutBuf);
		xpkFreeInternal(pInBuf);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}
	iZstdRet = ZSTD_initDStream(pStream);
	if ( ZSTD_isError(iZstdRet) ) {
		ZSTD_freeDStream(pStream);
		xpkFreeInternal(pOutBuf);
		xpkFreeInternal(pInBuf);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, "zstd decompress failed");
	}

	iOffsetRead = XPK_HEAD_SIZE;
	iSizeLeft = iCompSize;
	iRawPos = 0;
	iSliceWritten = 0;
	iSliceStart = pEntry->iDataOffset;
	iSliceEnd = pEntry->iDataOffset + pEntry->iFileSize;
	memset(&objIn, 0, sizeof(objIn));
	for ( ;; ) {
		if ( (objIn.pos == objIn.size) && (iSizeLeft > 0) ) {
			iChunkRead = (uint32_t)((iSizeLeft > (uint64_t)iInCap) ? iInCap : iSizeLeft);
			iRet = procXpkReadAtBuffer(objXpk, hFileSrc, iOffsetRead, pInBuf, iChunkRead);
			if ( iRet != XPK_OK ) {
				ZSTD_freeDStream(pStream);
				xpkFreeInternal(pOutBuf);
				xpkFreeInternal(pInBuf);
				if ( hFileSrc != NULL ) {
					xrtClose(hFileSrc);
				}
				xrtClose(hFileDst);
				return iRet;
			}
			objIn.src = pInBuf;
			objIn.size = iChunkRead;
			objIn.pos = 0;
			iOffsetRead += iChunkRead;
			iSizeLeft -= iChunkRead;
		} else if ( (objIn.pos == objIn.size) && (iSizeLeft == 0) ) {
			objIn.src = pInBuf;
			objIn.size = 0;
			objIn.pos = 0;
		}

		objOut.dst = pOutBuf;
		objOut.size = iOutCap;
		objOut.pos = 0;
		iZstdRet = ZSTD_decompressStream(pStream, &objOut, &objIn);
		if ( ZSTD_isError(iZstdRet) ) {
			ZSTD_freeDStream(pStream);
			xpkFreeInternal(pOutBuf);
			xpkFreeInternal(pInBuf);
			if ( hFileSrc != NULL ) {
				xrtClose(hFileSrc);
			}
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, "zstd decompress failed");
		}

		if ( objOut.pos > 0 ) {
			iCopyStart = (iRawPos > iSliceStart) ? iRawPos : iSliceStart;
			iCopyEnd = ((iRawPos + objOut.pos) < iSliceEnd) ? (iRawPos + objOut.pos) : iSliceEnd;
			if ( iCopyEnd > iCopyStart ) {
				if ( xrtPut(hFileDst, (ptr)((const uint8_t*)pOutBuf + (size_t)(iCopyStart - iRawPos)), (size_t)(iCopyEnd - iCopyStart)) != (size_t)(iCopyEnd - iCopyStart) ) {
					ZSTD_freeDStream(pStream);
					xpkFreeInternal(pOutBuf);
					xpkFreeInternal(pInBuf);
					if ( hFileSrc != NULL ) {
						xrtClose(hFileSrc);
					}
					xrtClose(hFileDst);
					return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
				}
				iSliceWritten += (iCopyEnd - iCopyStart);
			}
			iRawPos += objOut.pos;
		}

		if ( (iSizeLeft == 0) && (objIn.pos == objIn.size) && (iZstdRet == 0) ) {
			break;
		}
		if ( (iSizeLeft == 0) && (objIn.pos == objIn.size) && (objOut.pos == 0) && (iZstdRet > 0) ) {
			ZSTD_freeDStream(pStream);
			xpkFreeInternal(pOutBuf);
			xpkFreeInternal(pInBuf);
			if ( hFileSrc != NULL ) {
				xrtClose(hFileSrc);
			}
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, "zstd decompress failed");
		}
	}

	ZSTD_freeDStream(pStream);
	xpkFreeInternal(pOutBuf);
	xpkFreeInternal(pInBuf);
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}

	if ( iRawPos != iSolidSize || iSliceWritten != pEntry->iFileSize ) {
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( !xrtSetEOF(hFileDst) ) {
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
	}

	xrtClose(hFileDst);
	procXpkClearError(objXpk);
	return XPK_OK;
}

static inline int procXpkReadSolidLzma2SliceAlloc(xpkObject objXpk, xpkEntry* pEntry, void** pDataRet, uint64_t* pSizeRet)
{
	xfile hFileSrc;
	uint64_t iPkgSize;
	uint64_t iSolidSize;
	uint64_t iOffsetRead;
	uint64_t iSizeLeft;
	uint64_t iRawPos;
	uint64_t iSliceWritten;
	uint64_t iSliceStart;
	uint64_t iSliceEnd;
	uint64_t iCopyStart;
	uint64_t iCopyEnd;
	Byte iPropByte;
	CLzma2Dec objDec;
	ELzmaStatus iStatus;
	ELzmaFinishMode iFinishMode;
	void* pInBuf;
	void* pOutBuf;
	void* pSliceBuf;
	SizeT iInCap;
	SizeT iOutCap;
	SizeT iInputSize;
	SizeT iInputPos;
	SizeT iSrcLen;
	SizeT iDstLen;
	uint32_t iCompSize;
	uint32_t iChunkRead;
	int iRet;
	SRes iLzmaRes;

	if ( pDataRet != NULL ) {
		*pDataRet = NULL;
	}
	if ( pSizeRet != NULL ) {
		*pSizeRet = 0;
	}
	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pEntry->iFileSize == 0 ) {
		pSliceBuf = xpkAllocInternal(1);
		if ( pSliceBuf == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
		if ( pDataRet != NULL ) {
			*pDataRet = pSliceBuf;
		}
		if ( pSizeRet != NULL ) {
			*pSizeRet = 0;
		}
		procXpkClearError(objXpk);
		return XPK_OK;
	}

	iRet = procXpkCalcSolidRawSize(objXpk, &iSolidSize);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( (pEntry->iDataOffset + pEntry->iFileSize) > iSolidSize ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( objXpk->objHead.dataOffset < XPK_HEAD_SIZE ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	iCompSize = (uint32_t)(objXpk->objHead.dataOffset - XPK_HEAD_SIZE);
	if ( iCompSize < 1 ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	hFileSrc = NULL;
	if ( !procXpkAppliedVolumeMode(objXpk) ) {
		hFileSrc = xrtOpen(objXpk->sPathPackage, TRUE, XRT_CP_BINARY);
		if ( hFileSrc == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
		}
		iPkgSize = xrtGetEOF(hFileSrc);
		if ( objXpk->objHead.dataOffset > iPkgSize ) {
			xrtClose(hFileSrc);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
	}

	pSliceBuf = xpkAllocInternal((size_t)pEntry->iFileSize);
	if ( pSliceBuf == NULL ) {
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	iRet = procXpkReadAtBuffer(objXpk, hFileSrc, XPK_HEAD_SIZE, &iPropByte, 1);
	if ( iRet != XPK_OK ) {
		xpkFreeInternal(pSliceBuf);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		return iRet;
	}

	iInCap = XPK_CODEC_STREAM_CHUNK_SIZE;
	iOutCap = XPK_CODEC_STREAM_CHUNK_SIZE;
	pInBuf = xpkAllocInternal(iInCap);
	if ( pInBuf == NULL ) {
		xpkFreeInternal(pSliceBuf);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}
	pOutBuf = xpkAllocInternal(iOutCap);
	if ( pOutBuf == NULL ) {
		xpkFreeInternal(pInBuf);
		xpkFreeInternal(pSliceBuf);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	Lzma2Dec_Construct(&objDec);
	iLzmaRes = Lzma2Dec_Allocate(&objDec, iPropByte, &g_Alloc);
	if ( iLzmaRes != SZ_OK ) {
		xpkFreeInternal(pOutBuf);
		xpkFreeInternal(pInBuf);
		xpkFreeInternal(pSliceBuf);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		if ( iLzmaRes == SZ_ERROR_MEM ) {
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, "lzma2 decompress failed");
	}
	Lzma2Dec_Init(&objDec);

	iOffsetRead = XPK_HEAD_SIZE + 1;
	iSizeLeft = iCompSize - 1;
	iRawPos = 0;
	iSliceWritten = 0;
	iSliceStart = pEntry->iDataOffset;
	iSliceEnd = pEntry->iDataOffset + pEntry->iFileSize;
	iInputSize = 0;
	iInputPos = 0;

	for ( ;; ) {
		if ( iRawPos >= iSolidSize ) {
			break;
		}
		if ( (iInputPos == iInputSize) && (iSizeLeft > 0) ) {
			iChunkRead = (uint32_t)((iSizeLeft > (uint64_t)iInCap) ? iInCap : iSizeLeft);
			iRet = procXpkReadAtBuffer(objXpk, hFileSrc, iOffsetRead, pInBuf, iChunkRead);
			if ( iRet != XPK_OK ) {
				Lzma2Dec_Free(&objDec, &g_Alloc);
				xpkFreeInternal(pOutBuf);
				xpkFreeInternal(pInBuf);
				xpkFreeInternal(pSliceBuf);
				if ( hFileSrc != NULL ) {
					xrtClose(hFileSrc);
				}
				return iRet;
			}
			iInputSize = iChunkRead;
			iInputPos = 0;
			iOffsetRead += iChunkRead;
			iSizeLeft -= iChunkRead;
		}

		iDstLen = (SizeT)(((iSolidSize - iRawPos) > (uint64_t)iOutCap) ? iOutCap : (iSolidSize - iRawPos));
		iSrcLen = iInputSize - iInputPos;
		iFinishMode = (((iRawPos + iDstLen) == iSolidSize) && (iSizeLeft == 0)) ? LZMA_FINISH_END : LZMA_FINISH_ANY;
		iStatus = LZMA_STATUS_NOT_SPECIFIED;
		iLzmaRes = Lzma2Dec_DecodeToBuf(&objDec, (Byte*)pOutBuf, &iDstLen, (const Byte*)pInBuf + iInputPos, &iSrcLen, iFinishMode, &iStatus);
		if ( iLzmaRes != SZ_OK ) {
			Lzma2Dec_Free(&objDec, &g_Alloc);
			xpkFreeInternal(pOutBuf);
			xpkFreeInternal(pInBuf);
			xpkFreeInternal(pSliceBuf);
			if ( hFileSrc != NULL ) {
				xrtClose(hFileSrc);
			}
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, "lzma2 decompress failed");
		}

		iInputPos += iSrcLen;
		if ( iDstLen > 0 ) {
			iCopyStart = (iRawPos > iSliceStart) ? iRawPos : iSliceStart;
			iCopyEnd = ((iRawPos + iDstLen) < iSliceEnd) ? (iRawPos + iDstLen) : iSliceEnd;
			if ( iCopyEnd > iCopyStart ) {
				memcpy((uint8_t*)pSliceBuf + (size_t)(iCopyStart - iSliceStart), (const uint8_t*)pOutBuf + (size_t)(iCopyStart - iRawPos), (size_t)(iCopyEnd - iCopyStart));
				iSliceWritten += (iCopyEnd - iCopyStart);
			}
			iRawPos += iDstLen;
		}

		if ( (iDstLen == 0) && (iSrcLen == 0) ) {
			Lzma2Dec_Free(&objDec, &g_Alloc);
			xpkFreeInternal(pOutBuf);
			xpkFreeInternal(pInBuf);
			xpkFreeInternal(pSliceBuf);
			if ( hFileSrc != NULL ) {
				xrtClose(hFileSrc);
			}
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, "lzma2 decompress failed");
		}
	}

	Lzma2Dec_Free(&objDec, &g_Alloc);
	xpkFreeInternal(pOutBuf);
	xpkFreeInternal(pInBuf);
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}

	if ( iRawPos != iSolidSize || iSizeLeft != 0 || iInputPos != iInputSize || iSliceWritten != pEntry->iFileSize ) {
		xpkFreeInternal(pSliceBuf);
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	if ( pDataRet != NULL ) {
		*pDataRet = pSliceBuf;
	}
	if ( pSizeRet != NULL ) {
		*pSizeRet = pEntry->iFileSize;
	}
	procXpkClearError(objXpk);
	return XPK_OK;
}

static inline int procXpkCopySolidLzma2EntryToFile(xpkObject objXpk, xpkEntry* pEntry, const char* sPathFile)
{
	xfile hFileSrc;
	xfile hFileDst;
	uint64_t iPkgSize;
	uint64_t iSolidSize;
	uint64_t iOffsetRead;
	uint64_t iSizeLeft;
	uint64_t iRawPos;
	uint64_t iSliceWritten;
	uint64_t iSliceStart;
	uint64_t iSliceEnd;
	uint64_t iCopyStart;
	uint64_t iCopyEnd;
	Byte iPropByte;
	CLzma2Dec objDec;
	ELzmaStatus iStatus;
	ELzmaFinishMode iFinishMode;
	void* pInBuf;
	void* pOutBuf;
	SizeT iInCap;
	SizeT iOutCap;
	SizeT iInputSize;
	SizeT iInputPos;
	SizeT iSrcLen;
	SizeT iDstLen;
	uint32_t iCompSize;
	uint32_t iChunkRead;
	int iRet;
	SRes iLzmaRes;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( sPathFile == NULL || sPathFile[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( pEntry->iFileSize == 0 ) {
		return procXpkWriteFileData(objXpk, sPathFile, NULL, 0);
	}

	iRet = procXpkCalcSolidRawSize(objXpk, &iSolidSize);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( (pEntry->iDataOffset + pEntry->iFileSize) > iSolidSize ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( objXpk->objHead.dataOffset < XPK_HEAD_SIZE ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	iCompSize = (uint32_t)(objXpk->objHead.dataOffset - XPK_HEAD_SIZE);
	if ( iCompSize < 1 ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	hFileSrc = NULL;
	hFileDst = xrtOpen((str)sPathFile, FALSE, XRT_CP_BINARY);
	if ( hFileDst == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}
	if ( procXpkSeekFile(objXpk, hFileDst, 0) != XPK_OK ) {
		xrtClose(hFileDst);
		return xpkLastError(objXpk);
	}
	if ( !procXpkAppliedVolumeMode(objXpk) ) {
		hFileSrc = xrtOpen(objXpk->sPathPackage, TRUE, XRT_CP_BINARY);
		if ( hFileSrc == NULL ) {
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
		}
		iPkgSize = xrtGetEOF(hFileSrc);
		if ( objXpk->objHead.dataOffset > iPkgSize ) {
			xrtClose(hFileSrc);
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
	}

	iRet = procXpkReadAtBuffer(objXpk, hFileSrc, XPK_HEAD_SIZE, &iPropByte, 1);
	if ( iRet != XPK_OK ) {
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		return iRet;
	}

	iInCap = XPK_CODEC_STREAM_CHUNK_SIZE;
	iOutCap = XPK_CODEC_STREAM_CHUNK_SIZE;
	pInBuf = xpkAllocInternal(iInCap);
	if ( pInBuf == NULL ) {
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}
	pOutBuf = xpkAllocInternal(iOutCap);
	if ( pOutBuf == NULL ) {
		xpkFreeInternal(pInBuf);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	Lzma2Dec_Construct(&objDec);
	iLzmaRes = Lzma2Dec_Allocate(&objDec, iPropByte, &g_Alloc);
	if ( iLzmaRes != SZ_OK ) {
		xpkFreeInternal(pOutBuf);
		xpkFreeInternal(pInBuf);
		if ( hFileSrc != NULL ) {
			xrtClose(hFileSrc);
		}
		xrtClose(hFileDst);
		if ( iLzmaRes == SZ_ERROR_MEM ) {
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, "lzma2 decompress failed");
	}
	Lzma2Dec_Init(&objDec);

	iOffsetRead = XPK_HEAD_SIZE + 1;
	iSizeLeft = iCompSize - 1;
	iRawPos = 0;
	iSliceWritten = 0;
	iSliceStart = pEntry->iDataOffset;
	iSliceEnd = pEntry->iDataOffset + pEntry->iFileSize;
	iInputSize = 0;
	iInputPos = 0;
	for ( ;; ) {
		if ( iRawPos >= iSolidSize ) {
			break;
		}
		if ( (iInputPos == iInputSize) && (iSizeLeft > 0) ) {
			iChunkRead = (uint32_t)((iSizeLeft > (uint64_t)iInCap) ? iInCap : iSizeLeft);
			iRet = procXpkReadAtBuffer(objXpk, hFileSrc, iOffsetRead, pInBuf, iChunkRead);
			if ( iRet != XPK_OK ) {
				Lzma2Dec_Free(&objDec, &g_Alloc);
				xpkFreeInternal(pOutBuf);
				xpkFreeInternal(pInBuf);
				if ( hFileSrc != NULL ) {
					xrtClose(hFileSrc);
				}
				xrtClose(hFileDst);
				return iRet;
			}
			iInputSize = iChunkRead;
			iInputPos = 0;
			iOffsetRead += iChunkRead;
			iSizeLeft -= iChunkRead;
		}

		iDstLen = (SizeT)(((iSolidSize - iRawPos) > (uint64_t)iOutCap) ? iOutCap : (iSolidSize - iRawPos));
		iSrcLen = iInputSize - iInputPos;
		iFinishMode = (((iRawPos + iDstLen) == iSolidSize) && (iSizeLeft == 0)) ? LZMA_FINISH_END : LZMA_FINISH_ANY;
		iStatus = LZMA_STATUS_NOT_SPECIFIED;
		iLzmaRes = Lzma2Dec_DecodeToBuf(&objDec, (Byte*)pOutBuf, &iDstLen, (const Byte*)pInBuf + iInputPos, &iSrcLen, iFinishMode, &iStatus);
		if ( iLzmaRes != SZ_OK ) {
			Lzma2Dec_Free(&objDec, &g_Alloc);
			xpkFreeInternal(pOutBuf);
			xpkFreeInternal(pInBuf);
			if ( hFileSrc != NULL ) {
				xrtClose(hFileSrc);
			}
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, "lzma2 decompress failed");
		}

		iInputPos += iSrcLen;
		if ( iDstLen > 0 ) {
			iCopyStart = (iRawPos > iSliceStart) ? iRawPos : iSliceStart;
			iCopyEnd = ((iRawPos + iDstLen) < iSliceEnd) ? (iRawPos + iDstLen) : iSliceEnd;
			if ( iCopyEnd > iCopyStart ) {
				if ( xrtPut(hFileDst, (ptr)((const uint8_t*)pOutBuf + (size_t)(iCopyStart - iRawPos)), (size_t)(iCopyEnd - iCopyStart)) != (size_t)(iCopyEnd - iCopyStart) ) {
					Lzma2Dec_Free(&objDec, &g_Alloc);
					xpkFreeInternal(pOutBuf);
					xpkFreeInternal(pInBuf);
					if ( hFileSrc != NULL ) {
						xrtClose(hFileSrc);
					}
					xrtClose(hFileDst);
					return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
				}
				iSliceWritten += (iCopyEnd - iCopyStart);
			}
			iRawPos += iDstLen;
		}

		if ( (iDstLen == 0) && (iSrcLen == 0) ) {
			Lzma2Dec_Free(&objDec, &g_Alloc);
			xpkFreeInternal(pOutBuf);
			xpkFreeInternal(pInBuf);
			if ( hFileSrc != NULL ) {
				xrtClose(hFileSrc);
			}
			xrtClose(hFileDst);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, "lzma2 decompress failed");
		}
	}

	Lzma2Dec_Free(&objDec, &g_Alloc);
	xpkFreeInternal(pOutBuf);
	xpkFreeInternal(pInBuf);
	if ( hFileSrc != NULL ) {
		xrtClose(hFileSrc);
	}

	if ( iRawPos != iSolidSize || iSizeLeft != 0 || iInputPos != iInputSize || iSliceWritten != pEntry->iFileSize ) {
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( !xrtSetEOF(hFileDst) ) {
		xrtClose(hFileDst);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
	}

	xrtClose(hFileDst);
	procXpkClearError(objXpk);
	return XPK_OK;
}

static inline void* procXpkReadSolidEntryData(xpkObject objXpk, xpkEntry* pEntry, uint64_t* pSizeRet)
{
	void* pFileData;
	uint64_t iSolidSize;
	uint32_t iAlg;
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
	if ( procXpkSolidStoredCompLevel(objXpk) == 0 ) {
		iRet = procXpkCalcSolidStoredRawSize(objXpk, &iSolidSize);
		if ( iRet != XPK_OK ) {
			return NULL;
		}
		if ( pEntry->iFileSize > UINT32_MAX ) {
			procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge);
			return NULL;
		}
		if ( (pEntry->iDataOffset + pEntry->iFileSize) > iSolidSize ) {
			procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
			return NULL;
		}
		iRet = procXpkReadAtAlloc(objXpk, NULL, XPK_HEAD_SIZE + pEntry->iDataOffset, (uint32_t)pEntry->iFileSize, &pFileData);
		if ( iRet != XPK_OK ) {
			return NULL;
		}
		if ( pSizeRet != NULL ) {
			*pSizeRet = pEntry->iFileSize;
		}
		procXpkClearError(objXpk);
		return pFileData;
	}

	iAlg = procXpkCompLevelToAlg(procXpkSolidStoredCompLevel(objXpk));
	if ( iAlg == XPK_ALG_LZ4 || iAlg == XPK_ALG_LZ4HC ) {
		iRet = procXpkReadSolidLz4SliceAlloc(objXpk, pEntry, &pFileData, pSizeRet);
		if ( iRet != XPK_OK ) {
			return NULL;
		}
		return pFileData;
	}
	if ( iAlg == XPK_ALG_ZSTD ) {
		iRet = procXpkReadSolidZstdSliceAlloc(objXpk, pEntry, &pFileData, pSizeRet);
		if ( iRet != XPK_OK ) {
			return NULL;
		}
		return pFileData;
	}
	if ( iAlg == XPK_ALG_LZMA2 ) {
		iRet = procXpkReadSolidLzma2SliceAlloc(objXpk, pEntry, &pFileData, pSizeRet);
		if ( iRet != XPK_OK ) {
			return NULL;
		}
		return pFileData;
	}

	procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	return NULL;
}

static inline int procXpkCopySolidEntryToFile(xpkObject objXpk, xpkEntry* pEntry, const char* sPathFile)
{
	uint64_t iSolidSize;
	uint32_t iAlg;
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
	if ( !objXpk->bSolidApplied ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}
	if ( pEntry->iFileSize == 0 ) {
		return procXpkWriteFileData(objXpk, sPathFile, NULL, 0);
	}
	if ( procXpkSolidStoredCompLevel(objXpk) == 0 ) {
		iRet = procXpkCalcSolidStoredRawSize(objXpk, &iSolidSize);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		if ( (pEntry->iDataOffset + pEntry->iFileSize) > iSolidSize ) {
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
		return procXpkCopyPackageRangeToFile(objXpk, XPK_HEAD_SIZE + pEntry->iDataOffset, pEntry->iFileSize, sPathFile);
	}

	iAlg = procXpkCompLevelToAlg(procXpkSolidStoredCompLevel(objXpk));
	if ( iAlg == XPK_ALG_LZ4 || iAlg == XPK_ALG_LZ4HC ) {
		return procXpkCopySolidLz4EntryToFile(objXpk, pEntry, sPathFile);
	}
	if ( iAlg == XPK_ALG_ZSTD ) {
		return procXpkCopySolidZstdEntryToFile(objXpk, pEntry, sPathFile);
	}
	if ( iAlg == XPK_ALG_LZMA2 ) {
		return procXpkCopySolidLzma2EntryToFile(objXpk, pEntry, sPathFile);
	}

	return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
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

static inline void* procXpkReadEntryDataWithFile(xpkObject objXpk, xpkEntry* pEntry, uint64_t* pSizeRet, xfile hFile)
{
	xpkWriteNode* pNode;
	xfile hFileLocal;
	xpkMappedFile objMap;
	uint64_t iFileSize;
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
		if ( pEntry->iFileSize > UINT32_MAX ) {
			procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge);
			return NULL;
		}
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

	hFileLocal = NULL;
	memset(&objMap, 0, sizeof(objMap));
	iFileSize = 0;
	if ( hFile == NULL && !procXpkAppliedVolumeMode(objXpk) ) {
		hFileLocal = xrtOpen(objXpk->sPathPackage, TRUE, XRT_CP_BINARY);
		if ( hFileLocal == NULL ) {
			procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
			return NULL;
		}
		hFile = hFileLocal;
	}
	if ( pEntry->iDataSize > UINT32_MAX || pEntry->iFileSize > UINT32_MAX ) {
		if ( hFileLocal != NULL ) {
			xrtClose(hFileLocal);
		}
		procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge);
		return NULL;
	}
	if ( !procXpkAppliedVolumeMode(objXpk) && iLevel != 0 ) {
		iFileSize = xrtGetEOF(hFile);
		if ( pEntry->iDataOffset > iFileSize || pEntry->iDataSize > (iFileSize - pEntry->iDataOffset) ) {
			if ( hFileLocal != NULL ) {
				xrtClose(hFileLocal);
			}
			procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
			return NULL;
		}

		iRet = procXpkMapFileReadOnly(objXpk, hFile, iFileSize, &objMap);
		if ( hFileLocal != NULL ) {
			xrtClose(hFileLocal);
		}
		if ( iRet != XPK_OK ) {
			return NULL;
		}

		iRet = procXpkCodecDecode(objXpk, iLevel, (const uint8_t*)objMap.pView + pEntry->iDataOffset, (uint32_t)pEntry->iDataSize, (uint32_t)pEntry->iFileSize, &pRawData);
		procXpkUnmapFile(&objMap);
		if ( iRet != XPK_OK ) {
			return NULL;
		}
		if ( pSizeRet != NULL ) {
			*pSizeRet = pEntry->iFileSize;
		}
		procXpkClearError(objXpk);
		return pRawData;
	}

	pCompData = NULL;
	iRet = procXpkReadAtAlloc(objXpk, hFile, pEntry->iDataOffset, (uint32_t)pEntry->iDataSize, &pCompData);
	if ( hFileLocal != NULL ) {
		xrtClose(hFileLocal);
	}
	if ( iRet != XPK_OK ) {
		return NULL;
	}

	if ( iLevel == 0 ) {
		if ( pEntry->iDataSize != pEntry->iFileSize ) {
			xpkFreeInternal(pCompData);
			procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
			return NULL;
		}
		if ( pSizeRet != NULL ) {
			*pSizeRet = pEntry->iFileSize;
		}
		procXpkClearError(objXpk);
		return pCompData;
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

static inline void* procXpkReadEntryData(xpkObject objXpk, xpkEntry* pEntry, uint64_t* pSizeRet)
{
	return procXpkReadEntryDataWithFile(objXpk, pEntry, pSizeRet, NULL);
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
