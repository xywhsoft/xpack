/*
	xPack 保存模块

	负责将脏状态落盘，并在失败时执行回滚恢复。
*/

// 捕获刷写快照
static inline xpkFlushSnapshot* procXpkCaptureFlushSnapshots(xpkObject objXpk, uint32_t* pCountRet)
{
	xpkFlushSnapshot* arrSnapshot;
	xpkWriteNode* pNode;
	xpkEntry* pEntry;
	uint32_t iCount;
	uint32_t iNodePos;

	if ( pCountRet != NULL ) {
		*pCountRet = 0;
	}
	if ( objXpk == NULL ) {
		return NULL;
	}

	iCount = (uint32_t)procXpkWriteQueueCount(objXpk);
	if ( iCount == 0 ) {
		return NULL;
	}

	arrSnapshot = (xpkFlushSnapshot*)xpkAllocInternal(sizeof(*arrSnapshot) * iCount);
	if ( arrSnapshot == NULL ) {
		procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		return NULL;
	}

	for ( iNodePos = 1; iNodePos <= iCount; iNodePos++ ) {
		pNode = (xpkWriteNode*)xrtArrayGet(&objXpk->pWriteQueue->arrNode, iNodePos);
		if ( pNode == NULL ) {
			xpkFreeInternal(arrSnapshot);
			procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
			return NULL;
		}

		pEntry = procXpkGetEntryByPos(objXpk, pNode->iPos);
		if ( pEntry == NULL ) {
			xpkFreeInternal(arrSnapshot);
			procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
			return NULL;
		}

		arrSnapshot[iNodePos - 1].iPos = pNode->iPos;
		arrSnapshot[iNodePos - 1].iFlag = pEntry->iFlag;
		arrSnapshot[iNodePos - 1].iDataOffset = pEntry->iDataOffset;
		arrSnapshot[iNodePos - 1].iDataSize = pEntry->iDataSize;
		arrSnapshot[iNodePos - 1].iFileSize = pEntry->iFileSize;
	}

	if ( pCountRet != NULL ) {
		*pCountRet = iCount;
	}
	return arrSnapshot;
}

// 恢复刷写快照
static inline void procXpkRestoreFlushSnapshots(xpkObject objXpk, const xpkFlushSnapshot* arrSnapshot, uint32_t iCount, const xpkHead* pHeadSaved, uint64_t iAppendPosSaved, uint64_t iFileSizeSaved)
{
	xpkEntry* pEntry;
	uint32_t iIndex;

	if ( objXpk == NULL ) {
		return;
	}

	if ( pHeadSaved != NULL ) {
		objXpk->objHead = *pHeadSaved;
	}
	objXpk->iAppendPos = iAppendPosSaved;
	objXpk->iFileSize = iFileSizeSaved;

	if ( arrSnapshot == NULL ) {
		return;
	}

	for ( iIndex = 0; iIndex < iCount; iIndex++ ) {
		pEntry = procXpkGetEntryByPos(objXpk, arrSnapshot[iIndex].iPos);
		if ( pEntry == NULL ) {
			continue;
		}

		pEntry->iFlag = arrSnapshot[iIndex].iFlag;
		pEntry->iDataOffset = arrSnapshot[iIndex].iDataOffset;
		pEntry->iDataSize = arrSnapshot[iIndex].iDataSize;
		pEntry->iFileSize = arrSnapshot[iIndex].iFileSize;
	}
}

// 释放保存回滚
static inline void procXpkFreeSaveRollback(xpkSaveRollback* pRollback)
{
	if ( pRollback == NULL ) {
		return;
	}
	if ( pRollback->pTailData != NULL ) {
		xpkFreeInternal(pRollback->pTailData);
		pRollback->pTailData = NULL;
	}
	if ( pRollback->sTailPath != NULL ) {
		if ( !pRollback->bKeepTailFile && xrtFileExists((str)pRollback->sTailPath) ) {
			(void)xrtFileDelete((str)pRollback->sTailPath);
		}
		xpkFreeInternal(pRollback->sTailPath);
		pRollback->sTailPath = NULL;
	}
}

// 保存回滚路径复制
static inline char* procXpkSaveRollbackPathDup(xpkObject objXpk)
{
	char sSuffix[96];
	char* sPathRet;
	uint64_t iStamp;
	uint64_t iTag;
	uint32_t iTry;
	int iSizePrint;

	if ( objXpk == NULL || objXpk->sPathPackage == NULL ) {
		procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
		return NULL;
	}

	iStamp = (uint64_t)xpkNowInternal();
	iTag = (uint64_t)(uintptr_t)objXpk;
	for ( iTry = 0; iTry < 32; iTry++ ) {
		iSizePrint = snprintf(
			sSuffix,
			sizeof(sSuffix),
			".save.rollback.%llu.%llx.%u.tmp",
			(unsigned long long)iStamp,
			(unsigned long long)iTag,
			(unsigned int)iTry
		);
		if ( iSizePrint <= 0 || (size_t)iSizePrint >= sizeof(sSuffix) ) {
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite), NULL;
		}

		sPathRet = procXpkPathSuffixDupText(objXpk->sPathPackage, sSuffix);
		if ( sPathRet == NULL ) {
			procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
			return NULL;
		}
		if ( !xrtPathExists((str)sPathRet) ) {
			return sPathRet;
		}

		xpkFreeInternal(sPathRet);
	}

	procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
	return NULL;
}

// 将回滚尾段写入文件
static inline int procXpkCaptureRollbackTailToFile(xpkObject objXpk, uint64_t iOffset, uint64_t iSize, const char* sPathTail)
{
	xfile hFile;
	void* pChunk;
	uint64_t iRemain;
	uint64_t iOffsetCur;
	uint32_t iChunk;
	size_t iWrite;
	int iRet;

	if ( objXpk == NULL || sPathTail == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	hFile = xrtOpen((str)sPathTail, FALSE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	iRemain = iSize;
	iOffsetCur = iOffset;
	while ( iRemain > 0 ) {
		iChunk = (uint32_t)((iRemain > XPK_SAVE_ROLLBACK_CHUNK_SIZE) ? XPK_SAVE_ROLLBACK_CHUNK_SIZE : iRemain);
		pChunk = NULL;
		iRet = procXpkReadAtAlloc(objXpk, NULL, iOffsetCur, iChunk, &pChunk);
		if ( iRet != XPK_OK ) {
			xrtClose(hFile);
			return iRet;
		}

		iWrite = xrtPut(hFile, (ptr)pChunk, iChunk);
		xpkFreeInternal(pChunk);
		if ( iWrite != iChunk ) {
			xrtClose(hFile);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		}

		iOffsetCur += iChunk;
		iRemain -= iChunk;
	}

	if ( !xrtSetEOF(hFile) ) {
		xrtClose(hFile);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
	}

	xrtClose(hFile);
	return XPK_OK;
}

// 从文件恢复回滚尾段
static inline int procXpkRestoreRollbackTailFromFile(xpkObject objXpk, const xpkSaveRollback* pRollback)
{
	xfile hFile;
	void* pChunk;
	uint64_t iRemain;
	uint64_t iOffsetCur;
	uint64_t iBackupSize;
	uint32_t iChunk;
	size_t iRead;
	int iRet;

	if ( objXpk == NULL || pRollback == NULL || pRollback->sTailPath == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	hFile = xrtOpen((str)pRollback->sTailPath, TRUE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	iBackupSize = (uint64_t)xrtGetEOF(hFile);
	if ( iBackupSize != pRollback->iTailSize ) {
		xrtClose(hFile);
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoRead);
	}

	iRemain = pRollback->iTailSize;
	iOffsetCur = pRollback->iDataOffset;
	while ( iRemain > 0 ) {
		iChunk = (uint32_t)((iRemain > XPK_SAVE_ROLLBACK_CHUNK_SIZE) ? XPK_SAVE_ROLLBACK_CHUNK_SIZE : iRemain);
		pChunk = xpkAllocInternal(iChunk);
		if ( pChunk == NULL ) {
			xrtClose(hFile);
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
		iRead = xrtGetBuffer(hFile, pChunk, iChunk);
		if ( iRead != iChunk ) {
			xpkFreeInternal(pChunk);
			xrtClose(hFile);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoRead);
		}

		iRet = procXpkWriteAt(objXpk, NULL, iOffsetCur, pChunk, iChunk);
		xpkFreeInternal(pChunk);
		if ( iRet != XPK_OK ) {
			xrtClose(hFile);
			return iRet;
		}

		iOffsetCur += iChunk;
		iRemain -= iChunk;
	}

	xrtClose(hFile);
	return XPK_OK;
}

// 捕获保存回滚上下文
static inline int procXpkCaptureSaveRollback(xpkObject objXpk, xpkSaveRollback* pRollback)
{
	void* pHeadData;
	uint64_t iLogicalSize;
	uint64_t iTailSize64;
	int iRet;

	if ( objXpk == NULL || pRollback == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	memset(pRollback, 0, sizeof(*pRollback));
	pRollback->bFileExisted = xrtFileExists((str)objXpk->sPathPackage) ? TRUE : FALSE;
	if ( !pRollback->bFileExisted ) {
		return XPK_OK;
	}

	iLogicalSize = 0;
	if ( procXpkAppliedVolumeMode(objXpk) ) {
		iRet = procXpkCalcLogicalFileSize(objXpk, &objXpk->objHead, &iLogicalSize);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
	} else {
		iLogicalSize = (uint64_t)xrtFileGetSize((str)objXpk->sPathPackage);
	}
	pRollback->iLogicalSize = iLogicalSize;
	pRollback->iDataOffset = objXpk->objHead.dataOffset;

	if ( iLogicalSize >= XPK_HEAD_SIZE ) {
		pHeadData = NULL;
		iRet = procXpkReadAtAlloc(objXpk, NULL, 0, XPK_HEAD_SIZE, &pHeadData);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		memcpy(pRollback->sHeadBuf, pHeadData, XPK_HEAD_SIZE);
		xpkFreeInternal(pHeadData);
		pRollback->bHeadValid = TRUE;
	}

	if ( iLogicalSize <= pRollback->iDataOffset ) {
		return XPK_OK;
	}

	iTailSize64 = iLogicalSize - pRollback->iDataOffset;
	pRollback->iTailSize = iTailSize64;
	if ( iTailSize64 <= XPK_SAVE_ROLLBACK_MEM_LIMIT ) {
		iRet = procXpkReadAtAlloc(objXpk, NULL, pRollback->iDataOffset, (uint32_t)iTailSize64, &pRollback->pTailData);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		return XPK_OK;
	}

	pRollback->sTailPath = procXpkSaveRollbackPathDup(objXpk);
	if ( pRollback->sTailPath == NULL ) {
		return xpkLastError(objXpk);
	}
	iRet = procXpkCaptureRollbackTailToFile(objXpk, pRollback->iDataOffset, iTailSize64, pRollback->sTailPath);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	pRollback->bTailInFile = TRUE;
	return XPK_OK;
}

// 恢复保存回滚上下文
static inline int procXpkRestoreSaveRollback(xpkObject objXpk, const xpkSaveRollback* pRollback)
{
	int iRet;
	uint32_t iVolumeLast;
	uint32_t iVolume;
	uint32_t iVolumeSize;
	uint64_t iLastSize;
	xfile hFile;

	if ( objXpk == NULL || pRollback == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	if ( !pRollback->bFileExisted ) {
		if ( xrtFileExists((str)objXpk->sPathPackage) && !xrtFileDelete((str)objXpk->sPathPackage) ) {
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		}
		return procXpkScanLooseVolumeFilesText(objXpk, objXpk->sPathPackage, TRUE, NULL);
	}

	if ( pRollback->iTailSize > 0 ) {
		if ( pRollback->bTailInFile ) {
			iRet = procXpkRestoreRollbackTailFromFile(objXpk, pRollback);
			if ( iRet != XPK_OK ) {
				((xpkSaveRollback*)pRollback)->bKeepTailFile = TRUE;
				return iRet;
			}
		} else {
			iRet = procXpkWriteAt(objXpk, NULL, pRollback->iDataOffset, pRollback->pTailData, (uint32_t)pRollback->iTailSize);
			if ( iRet != XPK_OK ) {
				return iRet;
			}
		}
	}

	if ( procXpkAppliedVolumeMode(objXpk) ) {
		iVolumeSize = procXpkAppliedVolumeSize(objXpk);
		if ( iVolumeSize == 0 ) {
			return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		}

		if ( pRollback->iLogicalSize == 0 ) {
			iVolumeLast = 0;
			iLastSize = 0;
		} else {
			iVolumeLast = (uint32_t)((pRollback->iLogicalSize - 1) / iVolumeSize);
			iLastSize = pRollback->iLogicalSize - ((uint64_t)iVolumeLast * (uint64_t)iVolumeSize);
		}

		for ( iVolume = 0; iVolume < iVolumeLast; iVolume++ ) {
			hFile = NULL;
			iRet = procXpkOpenVolumeText(objXpk, objXpk->sPathPackage, iVolume, FALSE, &hFile);
			if ( iRet != XPK_OK ) {
				return iRet;
			}

			iRet = procXpkSeekFile(objXpk, hFile, iVolumeSize);
			if ( iRet != XPK_OK ) {
				xrtClose(hFile);
				return iRet;
			}
			if ( !xrtSetEOF(hFile) ) {
				xrtClose(hFile);
				return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
			}

			xrtClose(hFile);
		}

		hFile = NULL;
		iRet = procXpkOpenVolumeText(objXpk, objXpk->sPathPackage, iVolumeLast, FALSE, &hFile);
		if ( iRet != XPK_OK ) {
			return iRet;
		}

		iRet = procXpkSeekFile(objXpk, hFile, iLastSize);
		if ( iRet != XPK_OK ) {
			xrtClose(hFile);
			return iRet;
		}
		if ( !xrtSetEOF(hFile) ) {
			xrtClose(hFile);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		}

		xrtClose(hFile);
		iRet = procXpkDeleteVolumeFilesContiguousText(objXpk, objXpk->sPathPackage, iVolumeLast + 1);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
	} else {
		iRet = procXpkSetEOFAt(objXpk, NULL, pRollback->iLogicalSize);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
	}
	if ( pRollback->bHeadValid ) {
		iRet = procXpkWriteAt(objXpk, NULL, 0, pRollback->sHeadBuf, XPK_HEAD_SIZE);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
	}

	return XPK_OK;
}

// 执行保存落盘流程
static inline int procXpkSavePackage(xpkObject objXpk)
{
	xfile hFile;
	void* pMetaComp;
	void* pEntryRaw;
	void* pEntryComp;
	xpkFlushSnapshot* arrSnapshot;
	xpkSaveRollback objRollback;
	uint32_t iMetaCompSize;
	uint32_t iEntryRawSize;
	uint32_t iEntryCompSize;
	uint32_t iSnapshotCount;
	uint8_t iMetaLevel;
	uint8_t iInfoLevel;
	uint64_t iTailPos;
	uint64_t iAppendPosSaved;
	uint64_t iFileSizeSaved;
	uint8_t sHeadBuf[XPK_HEAD_SIZE];
	xpkHead objHeadSaved;
	int iRet;
	uint32_t iPos;
	xpkEntry* pEntry;

	// 先确认当前对象允许直接保存，布局变更需要交给 build 路径处理。
	if ( objXpk->bReadonly ) {
		return procXpkSetError(objXpk, XPK_ERR_READONLY, sXpkErrorReadonly);
	}
	if ( procXpkValidateEntryCountState(objXpk) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( procXpkSolidLayoutChanged(objXpk) && (objXpk->iEntryCount != 0) ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorSolidBuildRequired);
	}
	if ( procXpkVolumeLayoutChanged(objXpk) && !procXpkCanAdoptTargetLayout(objXpk) ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorVolumeBuildRequired);
	}

	pMetaComp = NULL;
	pEntryRaw = NULL;
	pEntryComp = NULL;
	arrSnapshot = NULL;
	memset(&objRollback, 0, sizeof(objRollback));
	iMetaCompSize = 0;
	iEntryRawSize = 0;
	iEntryCompSize = 0;
	iSnapshotCount = 0;
	iMetaLevel = (uint8_t)objXpk->objHead.metaComp;
	iInfoLevel = (uint8_t)objXpk->objHead.infoComp;
	objHeadSaved = objXpk->objHead;
	iAppendPosSaved = objXpk->iAppendPos;
	iFileSizeSaved = objXpk->iFileSize;

	// 预编码元数据，并提前抓取刷写快照与回滚上下文，保证中途失败后还能恢复现场。
	iRet = procXpkCodecEncode(objXpk, (uint8_t)objXpk->objHead.metaComp, objXpk->pPackageMeta, objXpk->iPackageMetaSize, &pMetaComp, &iMetaCompSize, &iMetaLevel);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	arrSnapshot = procXpkCaptureFlushSnapshots(objXpk, &iSnapshotCount);
	if ( (iSnapshotCount > 0) && (arrSnapshot == NULL) ) {
		if ( pMetaComp != NULL ) {
			xpkFreeInternal(pMetaComp);
		}
		return xpkLastError(objXpk);
	}
	iRet = procXpkCaptureSaveRollback(objXpk, &objRollback);
	if ( iRet != XPK_OK ) {
		if ( pMetaComp != NULL ) {
			xpkFreeInternal(pMetaComp);
		}
		if ( arrSnapshot != NULL ) {
			xpkFreeInternal(arrSnapshot);
		}
		return iRet;
	}

	hFile = NULL;
	if ( !procXpkAppliedVolumeMode(objXpk) ) {
		hFile = xrtOpen(objXpk->sPathPackage, FALSE, XRT_CP_BINARY);
		if ( hFile == NULL ) {
			iRet = procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
		} else {
			iRet = XPK_OK;
		}
	} else {
		iRet = XPK_OK;
	}
	if ( iRet != XPK_OK ) {
		if ( pMetaComp != NULL ) {
			xpkFreeInternal(pMetaComp);
		}
		if ( pEntryRaw != NULL ) {
			xpkFreeInternal(pEntryRaw);
		}
		if ( pEntryComp != NULL ) {
			xpkFreeInternal(pEntryComp);
		}
		if ( arrSnapshot != NULL ) {
			xpkFreeInternal(arrSnapshot);
		}
		procXpkFreeSaveRollback(&objRollback);
		return iRet;
	}

	// 先把延迟写入队列刷入数据区，再重新编码条目表，保证偏移信息使用的是最终值。
	iRet = procXpkFlushQueuedWrites(objXpk, hFile);
	if ( iRet != XPK_OK ) {
		if ( hFile != NULL ) {
			xrtClose(hFile);
		}
		procXpkRestoreFlushSnapshots(objXpk, arrSnapshot, iSnapshotCount, &objHeadSaved, iAppendPosSaved, iFileSizeSaved);
		if ( procXpkRestoreSaveRollback(objXpk, &objRollback) != XPK_OK ) {
			iRet = xpkLastError(objXpk);
		}
		if ( pMetaComp != NULL ) {
			xpkFreeInternal(pMetaComp);
		}
		if ( pEntryRaw != NULL ) {
			xpkFreeInternal(pEntryRaw);
		}
		if ( pEntryComp != NULL ) {
			xpkFreeInternal(pEntryComp);
		}
		if ( arrSnapshot != NULL ) {
			xpkFreeInternal(arrSnapshot);
		}
		procXpkFreeSaveRollback(&objRollback);
		return iRet;
	}

	iRet = procXpkEncodeEntryTable(objXpk, &pEntryRaw, &iEntryRawSize);
	if ( iRet != XPK_OK ) {
		if ( hFile != NULL ) {
			xrtClose(hFile);
		}
		procXpkRestoreFlushSnapshots(objXpk, arrSnapshot, iSnapshotCount, &objHeadSaved, iAppendPosSaved, iFileSizeSaved);
		if ( procXpkRestoreSaveRollback(objXpk, &objRollback) != XPK_OK ) {
			iRet = xpkLastError(objXpk);
		}
		if ( pMetaComp != NULL ) {
			xpkFreeInternal(pMetaComp);
		}
		if ( arrSnapshot != NULL ) {
			xpkFreeInternal(arrSnapshot);
		}
		procXpkFreeSaveRollback(&objRollback);
		return iRet;
	}

	iRet = procXpkCodecEncode(objXpk, (uint8_t)objXpk->objHead.infoComp, pEntryRaw, iEntryRawSize, &pEntryComp, &iEntryCompSize, &iInfoLevel);
	if ( iRet != XPK_OK ) {
		if ( hFile != NULL ) {
			xrtClose(hFile);
		}
		procXpkRestoreFlushSnapshots(objXpk, arrSnapshot, iSnapshotCount, &objHeadSaved, iAppendPosSaved, iFileSizeSaved);
		if ( procXpkRestoreSaveRollback(objXpk, &objRollback) != XPK_OK ) {
			iRet = xpkLastError(objXpk);
		}
		if ( pMetaComp != NULL ) {
			xpkFreeInternal(pMetaComp);
		}
		if ( pEntryRaw != NULL ) {
			xpkFreeInternal(pEntryRaw);
		}
		if ( arrSnapshot != NULL ) {
			xpkFreeInternal(arrSnapshot);
		}
		procXpkFreeSaveRollback(&objRollback);
		return iRet;
	}

	// 组装新的包头与尾段布局，并把元数据、条目表和头部一次性落盘。
	objXpk->objHead.fileCount = objXpk->iEntryCount;
	objXpk->objHead.dataOffset = objXpk->iAppendPos;
	objXpk->objHead.metaComp = iMetaLevel;
	objXpk->objHead.infoComp = iInfoLevel;
	objXpk->objHead.metaRawSize = objXpk->iPackageMetaSize;
	objXpk->objHead.metaCompSize = iMetaCompSize;
	objXpk->objHead.metaHash = (objXpk->iPackageMetaSize > 0) ? xpkHash32Internal(objXpk->pPackageMeta, objXpk->iPackageMetaSize) : 0;
	objXpk->objHead.infoCompSize = iEntryCompSize;
	objXpk->objHead.infoHash = (iEntryRawSize > 0) ? xpkHash32Internal(pEntryRaw, iEntryRawSize) : 0;
	if ( objXpk->objHead.createTime == 0 ) {
		objXpk->objHead.createTime = xpkNowInternal();
	}
	objXpk->objHead.changeTime = xpkNowInternal();

	iRet = procXpkWriteAt(objXpk, hFile, objXpk->objHead.dataOffset, pMetaComp, iMetaCompSize);
	if ( iRet == XPK_OK ) {
		iRet = procXpkWriteAt(objXpk, hFile, objXpk->objHead.dataOffset + iMetaCompSize, pEntryComp, iEntryCompSize);
	}
	if ( iRet == XPK_OK ) {
		iTailPos = objXpk->objHead.dataOffset + iMetaCompSize + iEntryCompSize;
		iRet = procXpkSetEOFAt(objXpk, hFile, iTailPos);
		if ( iRet == XPK_OK ) {
			objXpk->iFileSize = iTailPos;
		}
	}
	if ( iRet == XPK_OK ) {
		procXpkEncodeHead(&objXpk->objHead, sHeadBuf);
		iRet = procXpkWriteAt(objXpk, hFile, 0, sHeadBuf, XPK_HEAD_SIZE);
	}

	// 写盘完成后统一释放临时缓冲；失败则回滚快照和原始尾段。
	if ( hFile != NULL ) {
		xrtClose(hFile);
	}
	if ( pMetaComp != NULL ) {
		xpkFreeInternal(pMetaComp);
	}
	if ( pEntryRaw != NULL ) {
		xpkFreeInternal(pEntryRaw);
	}
	if ( pEntryComp != NULL ) {
		xpkFreeInternal(pEntryComp);
	}

	if ( iRet != XPK_OK ) {
		procXpkRestoreFlushSnapshots(objXpk, arrSnapshot, iSnapshotCount, &objHeadSaved, iAppendPosSaved, iFileSizeSaved);
		if ( procXpkRestoreSaveRollback(objXpk, &objRollback) != XPK_OK ) {
			iRet = xpkLastError(objXpk);
		}
		if ( arrSnapshot != NULL ) {
			xpkFreeInternal(arrSnapshot);
		}
		procXpkFreeSaveRollback(&objRollback);
		return iRet;
	}

	// 保存成功后把条目状态标记为已落盘，并清理脏标记。
	for ( iPos = 1; iPos <= objXpk->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, iPos);
		if ( pEntry != NULL ) {
			pEntry->bStored = TRUE;
		}
	}
	procXpkUnitWriteQueue(objXpk);
	if ( arrSnapshot != NULL ) {
		xpkFreeInternal(arrSnapshot);
	}
	procXpkFreeSaveRollback(&objRollback);

	procXpkMarkAppliedLayout(objXpk);
	procXpkMarkClean(objXpk);
	procXpkClearError(objXpk);
	return XPK_OK;
}
