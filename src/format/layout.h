static inline void procXpkWrite32LE(uint8_t* pBuf, uint32_t iValue)
{
	pBuf[0] = (uint8_t)(iValue & 0xFFu);
	pBuf[1] = (uint8_t)((iValue >> 8) & 0xFFu);
	pBuf[2] = (uint8_t)((iValue >> 16) & 0xFFu);
	pBuf[3] = (uint8_t)((iValue >> 24) & 0xFFu);
}

static inline void procXpkWrite64LE(uint8_t* pBuf, uint64_t iValue)
{
	procXpkWrite32LE(pBuf, (uint32_t)(iValue & 0xFFFFFFFFu));
	procXpkWrite32LE(pBuf + 4, (uint32_t)(iValue >> 32));
}

static inline uint32_t procXpkRead32LE(const uint8_t* pBuf)
{
	return ((uint32_t)pBuf[0]) |
		((uint32_t)pBuf[1] << 8) |
		((uint32_t)pBuf[2] << 16) |
		((uint32_t)pBuf[3] << 24);
}

static inline uint64_t procXpkRead64LE(const uint8_t* pBuf)
{
	return ((uint64_t)procXpkRead32LE(pBuf)) |
		((uint64_t)procXpkRead32LE(pBuf + 4) << 32);
}

static inline int procXpkApplyPackType(xpkObject objXpk, xpkPackType iType)
{
	uint32_t iInfoExtSize;

	iInfoExtSize = procXpkInfoExtSizeByPackType(iType);
	if ( iInfoExtSize == UINT32_MAX ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	objXpk->objHead.packType = (uint32_t)iType;
	objXpk->objHead.infoExtSize = iInfoExtSize;
	return XPK_OK;
}

static inline int procXpkValidateHead(xpkObject objXpk, const xpkHead* pHead)
{
	uint32_t iInfoExtSize;

	if ( pHead->fileHead != XPK_FILE_HEAD ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadHead);
	}
	if ( pHead->packType > XPK_PACK_WIN32 ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	iInfoExtSize = procXpkInfoExtSizeByPackType((xpkPackType)pHead->packType);
	if ( iInfoExtSize == UINT32_MAX ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pHead->infoExtSize != iInfoExtSize ) {
		if ( pHead->packType == XPK_PACK_CORE ) {
			if ( pHead->infoExtSize > 0x3FFFFu ) {
				return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
			}
		} else {
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
	}
	if ( pHead->dataOffset < XPK_HEAD_SIZE ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}
	if ( pHead->volumeMode ) {
		if ( pHead->volumeSize < XPK_VOLUME_MIN ) {
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
	}
	if ( (!pHead->volumeMode) && (pHead->volumeSize != 0) ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	return XPK_OK;
}

static inline void procXpkEncodeHead(const xpkHead* pHead, uint8_t sBuf[XPK_HEAD_SIZE])
{
	uint32_t iFlags1;
	uint32_t iFlags2;

	memset(sBuf, 0, XPK_HEAD_SIZE);

	iFlags1 = (pHead->packType & 0x3u) |
		((pHead->defComp & 0xFu) << 2) |
		((pHead->metaComp & 0xFu) << 6) |
		((pHead->infoComp & 0xFu) << 10) |
		((pHead->infoExtSize & 0x3FFFFu) << 14);

	iFlags2 = (pHead->solidMode & 0x1u) |
		((pHead->volumeMode & 0x1u) << 1);

	procXpkWrite32LE(sBuf + 0, pHead->fileHead);
	procXpkWrite32LE(sBuf + 4, pHead->fileCount);
	procXpkWrite32LE(sBuf + 8, iFlags1);
	procXpkWrite32LE(sBuf + 12, iFlags2);
	procXpkWrite64LE(sBuf + 16, pHead->dataOffset);
	procXpkWrite32LE(sBuf + 24, pHead->volumeSize);
	procXpkWrite32LE(sBuf + 28, pHead->metaRawSize);
	procXpkWrite32LE(sBuf + 32, pHead->metaCompSize);
	procXpkWrite32LE(sBuf + 36, pHead->metaHash);
	procXpkWrite32LE(sBuf + 40, pHead->infoCompSize);
	procXpkWrite32LE(sBuf + 44, pHead->infoHash);
	procXpkWrite64LE(sBuf + 48, (uint64_t)pHead->createTime);
	procXpkWrite64LE(sBuf + 56, (uint64_t)pHead->changeTime);
}

static inline void procXpkDecodeHead(const uint8_t sBuf[XPK_HEAD_SIZE], xpkHead* pHead)
{
	uint32_t iFlags1;
	uint32_t iFlags2;

	memset(pHead, 0, sizeof(*pHead));

	iFlags1 = procXpkRead32LE(sBuf + 8);
	iFlags2 = procXpkRead32LE(sBuf + 12);

	pHead->fileHead = procXpkRead32LE(sBuf + 0);
	pHead->fileCount = procXpkRead32LE(sBuf + 4);
	pHead->packType = iFlags1 & 0x3u;
	pHead->defComp = (iFlags1 >> 2) & 0xFu;
	pHead->metaComp = (iFlags1 >> 6) & 0xFu;
	pHead->infoComp = (iFlags1 >> 10) & 0xFu;
	pHead->infoExtSize = (iFlags1 >> 14) & 0x3FFFFu;
	pHead->solidMode = iFlags2 & 0x1u;
	pHead->volumeMode = (iFlags2 >> 1) & 0x1u;
	pHead->dataOffset = procXpkRead64LE(sBuf + 16);
	pHead->volumeSize = procXpkRead32LE(sBuf + 24);
	pHead->metaRawSize = procXpkRead32LE(sBuf + 28);
	pHead->metaCompSize = procXpkRead32LE(sBuf + 32);
	pHead->metaHash = procXpkRead32LE(sBuf + 36);
	pHead->infoCompSize = procXpkRead32LE(sBuf + 40);
	pHead->infoHash = procXpkRead32LE(sBuf + 44);
	pHead->createTime = (xtime)procXpkRead64LE(sBuf + 48);
	pHead->changeTime = (xtime)procXpkRead64LE(sBuf + 56);
}

static inline int procXpkEncodeEntryTable(xpkObject objXpk, void** pDataRet, uint32_t* pSizeRet)
{
	uint32_t iPos;
	uint32_t iStride;
	uint64_t iRawSize64;
	uint32_t iRawSize;
	uint8_t* pData;
	uint8_t* pRow;
	xpkEntry* pEntry;
	size_t iPathLen;

	if ( (pDataRet == NULL) || (pSizeRet == NULL) ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	*pDataRet = NULL;
	*pSizeRet = 0;
	if ( procXpkValidateEntryCountState(objXpk) != XPK_OK ) {
		return xpkLastError(objXpk);
	}

	iStride = procXpkEntryStride(&objXpk->objHead);
	iRawSize64 = (uint64_t)objXpk->iEntryCount * (uint64_t)iStride;
	if ( iRawSize64 > UINT32_MAX ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}

	iRawSize = (uint32_t)iRawSize64;
	if ( iRawSize == 0 ) {
		return XPK_OK;
	}

	pData = (uint8_t*)xpkAllocInternal(iRawSize);
	if ( pData == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}
	memset(pData, 0, iRawSize);

	for ( iPos = 1; iPos <= objXpk->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, iPos);
		if ( pEntry == NULL ) {
			xpkFreeInternal(pData);
			return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		}
		if ( !procXpkEntryDeleted(pEntry) ) {
			if ( procXpkValidateLiveEntryLookup(objXpk, pEntry) != XPK_OK ) {
				xpkFreeInternal(pData);
				return xpkLastError(objXpk);
			}
		}

		pRow = pData + ((iPos - 1) * iStride);
		procXpkWrite32LE(pRow + 0, pEntry->iFlag);
		procXpkWrite32LE(pRow + 4, pEntry->iFileHash);
		procXpkWrite64LE(pRow + 8, pEntry->iDataOffset);
		procXpkWrite64LE(pRow + 16, pEntry->iDataSize);
		procXpkWrite64LE(pRow + 24, pEntry->iFileSize);

		if ( (objXpk->objHead.packType == XPK_PACK_CORE) && (objXpk->objHead.infoExtSize > 0) ) {
			if ( pEntry->pInfoExt != NULL ) {
				memcpy(pRow + 32, pEntry->pInfoExt, objXpk->objHead.infoExtSize);
			}
		} else if ( objXpk->objHead.packType == XPK_PACK_INDEX ) {
			procXpkWrite64LE(pRow + 32, (uint64_t)pEntry->iFileIndex);
		} else if ( (objXpk->objHead.packType == XPK_PACK_LINUX) || (objXpk->objHead.packType == XPK_PACK_WIN32) ) {
			if ( (pEntry->sPath == NULL) || (pEntry->sPath[0] == '\0') ) {
				xpkFreeInternal(pData);
				return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
			}

			iPathLen = strlen(pEntry->sPath);
			if ( iPathLen >= XPK_PATH_BYTES ) {
				xpkFreeInternal(pData);
				return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPathTooLong);
			}
			memcpy(pRow + 32, pEntry->sPath, iPathLen);
			procXpkWrite32LE(pRow + 32 + XPK_PATH_BYTES, pEntry->iPlatformAttr);
			procXpkWrite64LE(pRow + 32 + XPK_PATH_BYTES + 4, pEntry->tCreateTime);
			procXpkWrite64LE(pRow + 32 + XPK_PATH_BYTES + 12, pEntry->tModifyTime);
			procXpkWrite64LE(pRow + 32 + XPK_PATH_BYTES + 20, pEntry->tAccessTime);
		}
	}

	*pDataRet = pData;
	*pSizeRet = iRawSize;
	return XPK_OK;
}

static inline int procXpkDecodeEntryTable(xpkObject objXpk, const void* pData, uint32_t iSize)
{
	uint32_t iPos;
	uint32_t iStride;
	const uint8_t* pRow;
	xpkEntry objEntry;
	size_t iPathLen;
	int iRet;

	if ( iSize == 0 ) {
		procXpkResetEntries(objXpk);
		return XPK_OK;
	}
	if ( pData == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	iStride = procXpkEntryStride(&objXpk->objHead);
	if ( ((uint64_t)objXpk->objHead.fileCount * (uint64_t)iStride) != iSize ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	procXpkResetEntries(objXpk);
	for ( iPos = 0; iPos < objXpk->objHead.fileCount; iPos++ ) {
		memset(&objEntry, 0, sizeof(objEntry));
		pRow = (const uint8_t*)pData + (iPos * iStride);

		objEntry.iFlag = procXpkRead32LE(pRow + 0);
		objEntry.iFileHash = procXpkRead32LE(pRow + 4);
		objEntry.iDataOffset = procXpkRead64LE(pRow + 8);
		objEntry.iDataSize = procXpkRead64LE(pRow + 16);
		objEntry.iFileSize = procXpkRead64LE(pRow + 24);
		objEntry.bStored = TRUE;

		if ( (objXpk->objHead.packType == XPK_PACK_CORE) && (objXpk->objHead.infoExtSize > 0) ) {
			objEntry.pInfoExt = procXpkAllocInfoExt(objXpk);
			if ( objEntry.pInfoExt == NULL ) {
				procXpkResetEntries(objXpk);
				return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
			}
			memcpy(objEntry.pInfoExt, pRow + 32, objXpk->objHead.infoExtSize);
		} else if ( objXpk->objHead.packType == XPK_PACK_INDEX ) {
			objEntry.iFileIndex = (int64_t)procXpkRead64LE(pRow + 32);
		} else if ( (objXpk->objHead.packType == XPK_PACK_LINUX) || (objXpk->objHead.packType == XPK_PACK_WIN32) ) {
			for ( iPathLen = 0; iPathLen < XPK_PATH_BYTES; iPathLen++ ) {
				if ( pRow[32 + iPathLen] == '\0' ) {
					break;
				}
			}
			if ( iPathLen == 0 ) {
				procXpkResetEntries(objXpk);
				return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
			}
			if ( iPathLen >= XPK_PATH_BYTES ) {
				procXpkResetEntries(objXpk);
				return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
			}
			objEntry.sPath = procXpkDupPathTextN(objXpk, (const char*)(pRow + 32), iPathLen);
			if ( (iPathLen > 0) && (objEntry.sPath == NULL) ) {
				procXpkResetEntries(objXpk);
				return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
			}
			if ( objEntry.sPath == NULL || objEntry.sPath[0] == '\0' ) {
				procXpkFreeEntryOwned(&objEntry);
				procXpkResetEntries(objXpk);
				return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
			}
			objEntry.iPlatformAttr = procXpkRead32LE(pRow + 32 + XPK_PATH_BYTES);
			objEntry.tCreateTime = procXpkRead64LE(pRow + 32 + XPK_PATH_BYTES + 4);
			objEntry.tModifyTime = procXpkRead64LE(pRow + 32 + XPK_PATH_BYTES + 12);
			objEntry.tAccessTime = procXpkRead64LE(pRow + 32 + XPK_PATH_BYTES + 20);
		}

		iRet = procXpkAppendEntryOwned(objXpk, &objEntry);
		if ( iRet != XPK_OK ) {
			procXpkFreeEntryOwned(&objEntry);
			procXpkResetEntries(objXpk);
			return iRet;
		}
	}

	iRet = procXpkRebuildLookup(objXpk);
	if ( iRet != XPK_OK ) {
		procXpkResetEntries(objXpk);
		return iRet;
	}

	return XPK_OK;
}
