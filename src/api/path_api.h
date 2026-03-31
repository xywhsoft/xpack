/*
	xPack 路径 API 实现

	负责按包内路径访问条目的增删改查与属性操作。
*/

// 按路径检查条目是否存在
XPKAPI int xpkPathExists(xpkObject objXpk, const char* sPackagePath)
{
	xpkEntry* pEntry;
	int iRet;

	if ( objXpk == NULL ) {
		procXpkSetError(NULL, XPK_ERR_PARAM, sXpkErrorInvalidParam);
		return FALSE;
	}
	if ( (objXpk->objHead.packType != XPK_PACK_LINUX) && (objXpk->objHead.packType != XPK_PACK_WIN32) ) {
		procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
		return FALSE;
	}
	if ( procXpkValidateStoredPathText(objXpk, sPackagePath) != XPK_OK ) {
		return FALSE;
	}

	pEntry = procXpkLookupPathEntry(objXpk, sPackagePath);
	if ( pEntry == NULL ) {
		iRet = xpkLastError(objXpk);
		if ( iRet != XPK_OK ) {
			return FALSE;
		}
		procXpkClearError(objXpk);
		return FALSE;
	}

	procXpkClearError(objXpk);
	return TRUE;
}

// 按路径获取条目信息
XPKAPI int xpkPathGetInfo(xpkObject objXpk, const char* sPackagePath, xpkFileInfoPath* pInfoRet)
{
	xpkEntry* pEntry;
	size_t iPathLen;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( (objXpk->objHead.packType != XPK_PACK_LINUX) && (objXpk->objHead.packType != XPK_PACK_WIN32) ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
	}
	if ( pInfoRet == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( procXpkValidateStoredPathText(objXpk, sPackagePath) != XPK_OK ) {
		return xpkLastError(objXpk);
	}

	pEntry = procXpkLookupPathEntry(objXpk, sPackagePath);
	if ( pEntry == NULL ) {
		iRet = xpkLastError(objXpk);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorNotFound);
	}

	memset(pInfoRet, 0, sizeof(*pInfoRet));
	pInfoRet->flag = pEntry->iFlag;
	pInfoRet->fileHash = pEntry->iFileHash;
	pInfoRet->dataOffset = pEntry->iDataOffset;
	pInfoRet->dataSize = pEntry->iDataSize;
	pInfoRet->fileSize = pEntry->iFileSize;
	if ( pEntry->sPath != NULL ) {
		iPathLen = strlen(pEntry->sPath);
		if ( iPathLen >= XPK_PATH_BYTES ) {
			iPathLen = XPK_PATH_BYTES - 1;
		}
		memcpy(pInfoRet->pathBytes, pEntry->sPath, iPathLen);
	}
	pInfoRet->platformAttr = pEntry->iPlatformAttr;
	pInfoRet->createTime = pEntry->tCreateTime;
	pInfoRet->modifyTime = pEntry->tModifyTime;
	pInfoRet->accessTime = pEntry->tAccessTime;
	procXpkClearError(objXpk);
	return XPK_OK;
}

// 按路径添加文件条目
XPKAPI int xpkPathAddFile(xpkObject objXpk, const char* sPackagePath, const char* sSrcPath, const xpkWriteOptions* pOpt)
{
	xpkEntry objEntry;
	char* sPathText;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( (objXpk->objHead.packType != XPK_PACK_LINUX) && (objXpk->objHead.packType != XPK_PACK_WIN32) ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
	}
	if ( procXpkValidateStoredPathText(objXpk, sPackagePath) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( sSrcPath == NULL || sSrcPath[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( xpkPathExists(objXpk, sPackagePath) ) {
		return procXpkSetError(objXpk, XPK_ERR_EXISTS, sXpkErrorExists);
	}
	iRet = xpkLastError(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	procXpkClearError(objXpk);

	memset(&objEntry, 0, sizeof(objEntry));
	sPathText = procXpkDupPathStoredText(objXpk, sPackagePath);
	objEntry.sPath = sPathText;
	if ( objEntry.sPath == NULL ) {
		return xpkLastError(objXpk);
	}

	iRet = procXpkAddFileEntry(objXpk, &objEntry, sSrcPath, pOpt, NULL);
	if ( objEntry.sPath != NULL ) {
		xpkFreeInternal(objEntry.sPath);
	}
	return iRet;
}

// 按路径添加内存数据条目
XPKAPI int xpkPathAddData(xpkObject objXpk, const char* sPackagePath, const void* pData, uint64_t iSize, const xpkWriteOptions* pOpt)
{
	xpkEntry objEntry;
	char* sPathText;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( (objXpk->objHead.packType != XPK_PACK_LINUX) && (objXpk->objHead.packType != XPK_PACK_WIN32) ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
	}
	if ( procXpkValidateStoredPathText(objXpk, sPackagePath) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( pData == NULL && iSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( xpkPathExists(objXpk, sPackagePath) ) {
		return procXpkSetError(objXpk, XPK_ERR_EXISTS, sXpkErrorExists);
	}
	iRet = xpkLastError(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	procXpkClearError(objXpk);

	memset(&objEntry, 0, sizeof(objEntry));
	sPathText = procXpkDupPathStoredText(objXpk, sPackagePath);
	objEntry.sPath = sPathText;
	if ( objEntry.sPath == NULL ) {
		return xpkLastError(objXpk);
	}

	iRet = procXpkAddDataEntry(objXpk, &objEntry, pData, iSize, pOpt, NULL);
	if ( objEntry.sPath != NULL ) {
		xpkFreeInternal(objEntry.sPath);
	}
	return iRet;
}

// 按路径读取条目到文件
XPKAPI int xpkPathReadToFile(xpkObject objXpk, const char* sPackagePath, const char* sDstPath)
{
	xpkEntry* pEntry;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( (objXpk->objHead.packType != XPK_PACK_LINUX) && (objXpk->objHead.packType != XPK_PACK_WIN32) ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
	}
	if ( procXpkValidateStoredPathText(objXpk, sPackagePath) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( sDstPath == NULL || sDstPath[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	pEntry = procXpkLookupPathEntry(objXpk, sPackagePath);
	if ( pEntry == NULL ) {
		iRet = xpkLastError(objXpk);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorNotFound);
	}
	return xpkReadToFile(objXpk, pEntry->iPos, sDstPath);
}

// 按路径读取条目到内存
XPKAPI void* xpkPathReadToMemory(xpkObject objXpk, const char* sPackagePath, uint64_t* pSizeRet)
{
	xpkEntry* pEntry;
	int iRet;

	if ( pSizeRet != NULL ) {
		*pSizeRet = 0;
	}
	if ( objXpk == NULL ) {
		procXpkSetError(NULL, XPK_ERR_PARAM, sXpkErrorInvalidParam);
		return NULL;
	}
	if ( (objXpk->objHead.packType != XPK_PACK_LINUX) && (objXpk->objHead.packType != XPK_PACK_WIN32) ) {
		procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
		return NULL;
	}
	if ( procXpkValidateStoredPathText(objXpk, sPackagePath) != XPK_OK ) {
		return NULL;
	}

	pEntry = procXpkLookupPathEntry(objXpk, sPackagePath);
	if ( pEntry == NULL ) {
		iRet = xpkLastError(objXpk);
		if ( iRet != XPK_OK ) {
			return NULL;
		}
		procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorNotFound);
		return NULL;
	}
	return xpkReadToMemory(objXpk, pEntry->iPos, pSizeRet);
}

// 按路径更新文件条目
XPKAPI int xpkPathUpdateFile(xpkObject objXpk, const char* sPackagePath, const char* sSrcPath, const xpkWriteOptions* pOpt)
{
	xpkEntry* pEntry;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( (objXpk->objHead.packType != XPK_PACK_LINUX) && (objXpk->objHead.packType != XPK_PACK_WIN32) ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
	}
	if ( procXpkValidateStoredPathText(objXpk, sPackagePath) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( sSrcPath == NULL || sSrcPath[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	pEntry = procXpkLookupPathEntry(objXpk, sPackagePath);
	if ( pEntry == NULL ) {
		iRet = xpkLastError(objXpk);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorNotFound);
	}
	return xpkUpdateFile(objXpk, pEntry->iPos, sSrcPath, pOpt);
}

// 按路径更新内存数据条目
XPKAPI int xpkPathUpdateData(xpkObject objXpk, const char* sPackagePath, const void* pData, uint64_t iSize, const xpkWriteOptions* pOpt)
{
	xpkEntry* pEntry;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( (objXpk->objHead.packType != XPK_PACK_LINUX) && (objXpk->objHead.packType != XPK_PACK_WIN32) ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
	}
	if ( procXpkValidateStoredPathText(objXpk, sPackagePath) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( pData == NULL && iSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	pEntry = procXpkLookupPathEntry(objXpk, sPackagePath);
	if ( pEntry == NULL ) {
		iRet = xpkLastError(objXpk);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorNotFound);
	}
	return xpkUpdateData(objXpk, pEntry->iPos, pData, iSize, pOpt);
}

// 按路径重命名条目
XPKAPI int xpkPathRename(xpkObject objXpk, const char* sPathOld, const char* sPathNew)
{
	xpkEntry* pEntry;
	xpkEntry* pEntryNew;
	char* sPathOldDup;
	char* sPathDup;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( (objXpk->objHead.packType != XPK_PACK_LINUX) && (objXpk->objHead.packType != XPK_PACK_WIN32) ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
	}
	if ( sPathOld == NULL || sPathNew == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( procXpkValidateStoredPathText(objXpk, sPathOld) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( procXpkValidateStoredPathText(objXpk, sPathNew) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	pEntry = procXpkLookupPathEntry(objXpk, sPathOld);
	if ( pEntry == NULL ) {
		iRet = xpkLastError(objXpk);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorNotFound);
	}

	pEntryNew = procXpkLookupPathEntry(objXpk, sPathNew);
	if ( pEntryNew == NULL ) {
		iRet = xpkLastError(objXpk);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		procXpkClearError(objXpk);
	} else if ( pEntryNew != pEntry ) {
		return procXpkSetError(objXpk, XPK_ERR_EXISTS, sXpkErrorExists);
	}

	sPathDup = procXpkDupPathStoredText(objXpk, sPathNew);
	if ( sPathDup == NULL ) {
		return xpkLastError(objXpk);
	}

	sPathOldDup = pEntry->sPath;
	pEntry->sPath = sPathDup;
	iRet = procXpkRebuildLookup(objXpk);
	if ( iRet != XPK_OK ) {
		pEntry->sPath = sPathOldDup;
		if ( sPathDup != NULL ) {
			xpkFreeInternal(sPathDup);
		}
		procXpkRebuildLookup(objXpk);
		return iRet;
	}
	if ( sPathOldDup != NULL ) {
		xpkFreeInternal(sPathOldDup);
	}
	objXpk->bDirtyEntryTable = TRUE;
	objXpk->bDirtyHead = TRUE;
	procXpkClearError(objXpk);
	return XPK_OK;
}

// 按路径移除条目
XPKAPI int xpkPathRemove(xpkObject objXpk, const char* sPackagePath)
{
	xpkEntry* pEntry;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( (objXpk->objHead.packType != XPK_PACK_LINUX) && (objXpk->objHead.packType != XPK_PACK_WIN32) ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
	}
	if ( procXpkValidateStoredPathText(objXpk, sPackagePath) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	pEntry = procXpkLookupPathEntry(objXpk, sPackagePath);
	if ( pEntry == NULL ) {
		iRet = xpkLastError(objXpk);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorNotFound);
	}
	return xpkRemove(objXpk, pEntry->iPos);
}

// 按路径设置平台属性
XPKAPI int xpkPathSetAttr(xpkObject objXpk, const char* sPackagePath, uint32_t iPlatformAttr)
{
	xpkEntry* pEntry;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( (objXpk->objHead.packType != XPK_PACK_LINUX) && (objXpk->objHead.packType != XPK_PACK_WIN32) ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
	}
	if ( procXpkValidateStoredPathText(objXpk, sPackagePath) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	pEntry = procXpkLookupPathEntry(objXpk, sPackagePath);
	if ( pEntry == NULL ) {
		iRet = xpkLastError(objXpk);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorNotFound);
	}

	pEntry->iPlatformAttr = iPlatformAttr;
	objXpk->bDirtyEntryTable = TRUE;
	objXpk->bDirtyHead = TRUE;
	procXpkClearError(objXpk);
	return XPK_OK;
}
