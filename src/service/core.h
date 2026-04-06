/*
	xPack 核心基础服务模块

	负责错误状态、路径归一化、条目管理、查找表维护与对象生命周期。
*/

#include <ctype.h>

static const char sXpkErrorInvalidObject[] = "invalid xpk object";
static const char sXpkErrorInvalidParam[] = "invalid parameter";
static const char sXpkErrorOutOfMemory[] = "out of memory";
static const char sXpkErrorNotImplemented[] = "function is not implemented yet";
static const char sXpkErrorNotFound[] = "target entry not found";
static const char sXpkErrorReadonly[] = "package is readonly";
static const char sXpkErrorIoOpen[] = "failed to open package file";
static const char sXpkErrorIoRead[] = "failed to read package file";
static const char sXpkErrorIoWrite[] = "failed to write package file";
static const char sXpkErrorIoSeek[] = "failed to seek package file";
static const char sXpkErrorBadHead[] = "invalid xpk file header";
static const char sXpkErrorBadFormat[] = "invalid xpk package format";
static const char sXpkErrorHashMismatch[] = "data hash verification failed";
static const char sXpkErrorVolumeUnsupported[] = "volume mode is not implemented yet";
static const char sXpkErrorCodecUnsupported[] = "compression level is not implemented yet";
static const char sXpkErrorSeekRange[] = "file offset exceeds current seek range";
static const char sXpkErrorBlockTooLarge[] = "single data block exceeds current xpk block limit";
static const char sXpkErrorCoreExtUnsupported[] = "custom core infoExtSize is not enabled";
static const char sXpkErrorPackTypeLocked[] = "pack type can only be changed on empty package";
static const char sXpkErrorBufferedPending[] = "buffered writes are pending";
static const char sXpkErrorExists[] = "target entry already exists";
static const char sXpkErrorDeleted[] = "target entry has been deleted";
static const char sXpkErrorPackTypeMismatch[] = "pack type does not match this api";
static const char sXpkErrorSolidBuildRequired[] = "solid mode change requires xpkBuild";
static const char sXpkErrorVolumeBuildRequired[] = "volume layout change requires xpkBuild";
static const char sXpkErrorInfoExtLocked[] = "infoExtSize can only be changed on empty core package";
static const char sXpkErrorInfoExtSizeMismatch[] = "infoExt buffer size does not match current infoExtSize";
static const char sXpkErrorPathTooLong[] = "package path exceeds 259 bytes";
static const char sXpkErrorPathEmpty[] = "package path is empty";
static const char sXpkErrorCompManaged[] = "compression flag is managed by xpk";
static const char sXpkErrorTempPathExists[] = "tempPath already exists";
static const char sXpkErrorBackupPathExists[] = "backup path is occupied";
static const char sXpkErrorPackagePathExists[] = "package path is occupied";
static XRT_TLS_STORAGE xpkErrorState g_objXpkErrorTls = { XPK_OK, { 0 } };

// 设置线程错误状态
static inline void procXpkSetThreadError(int iCode, const char* sText)
{
	size_t iSizeText;

	g_objXpkErrorTls.iCode = iCode;
	if ( sText == NULL ) {
		g_objXpkErrorTls.sText[0] = '\0';
		return;
	}

	iSizeText = strlen(sText);
	if ( iSizeText >= XPK_ERROR_TEXT_CAP ) {
		iSizeText = XPK_ERROR_TEXT_CAP - 1;
	}
	memcpy(g_objXpkErrorTls.sText, sText, iSizeText);
	g_objXpkErrorTls.sText[iSizeText] = '\0';
}

// 清理错误状态
static inline void procXpkClearError(xpkObject objXpk)
{
	procXpkSetThreadError(XPK_OK, NULL);
	if ( objXpk != NULL ) {
		objXpk->err.iCode = XPK_OK;
		objXpk->err.sText[0] = '\0';
	}
}

// 设置错误状态
static inline int procXpkSetError(xpkObject objXpk, int iCode, const char* sText)
{
	size_t iSizeText;

	procXpkSetThreadError(iCode, sText);
	if ( objXpk == NULL ) {
		return iCode;
	}

	objXpk->err.iCode = iCode;
	if ( sText == NULL ) {
		objXpk->err.sText[0] = '\0';
		return iCode;
	}

	iSizeText = strlen(sText);
	if ( iSizeText >= XPK_ERROR_TEXT_CAP ) {
		iSizeText = XPK_ERROR_TEXT_CAP - 1;
	}
	memcpy(objXpk->err.sText, sText, iSizeText);
	objXpk->err.sText[iSizeText] = '\0';
	return iCode;
}

// 返回参数错误
static inline int procXpkReturnParamError(xpkObject objXpk)
{
	if ( objXpk == NULL ) {
		return procXpkSetError(NULL, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
}

// 确保对象可写
static inline int procXpkEnsureWritable(xpkObject objXpk)
{
	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( objXpk->bReadonly ) {
		return procXpkSetError(objXpk, XPK_ERR_READONLY, sXpkErrorReadonly);
	}
	return XPK_OK;
}

// 在对象有效时设置参数错误
static inline void procXpkSetParamErrorIfObject(xpkObject objXpk)
{
	if ( objXpk == NULL ) {
		return;
	}
	procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
}

// 复制指定长度文本
static inline char* procXpkDupTextN(const char* sText, size_t iSizeText)
{
	char* sRet;

	sRet = (char*)xpkAllocInternal(iSizeText + 1);
	if ( sRet == NULL ) {
		return NULL;
	}
	if ( (sText != NULL) && (iSizeText > 0) ) {
		memcpy(sRet, sText, iSizeText);
	}
	sRet[iSizeText] = '\0';
	return sRet;
}

// 复制文本
static inline char* procXpkDupText(const char* sText)
{
	if ( sText == NULL ) {
		return NULL;
	}
	return procXpkDupTextN(sText, strlen(sText));
}

// 判断 Core 信息扩展是否启用
static inline int procXpkCoreInfoExtEnabled(xpkObject objXpk)
{
	if ( objXpk == NULL ) {
		return FALSE;
	}
	if ( objXpk->objHead.packType != XPK_PACK_CORE ) {
		return FALSE;
	}
	return (objXpk->objHead.infoExtSize > 0) ? TRUE : FALSE;
}

// 分配信息扩展缓冲
static inline void* procXpkAllocInfoExt(xpkObject objXpk)
{
	void* pInfoExt;

	if ( !procXpkCoreInfoExtEnabled(objXpk) ) {
		return NULL;
	}

	pInfoExt = xpkAllocInternal(objXpk->objHead.infoExtSize);
	if ( pInfoExt == NULL ) {
		return NULL;
	}
	memset(pInfoExt, 0, objXpk->objHead.infoExtSize);
	return pInfoExt;
}

// 复制信息扩展缓冲
static inline void* procXpkDupInfoExt(xpkObject objXpk, const void* pInfoExtSrc)
{
	void* pInfoExt;

	if ( !procXpkCoreInfoExtEnabled(objXpk) ) {
		return NULL;
	}

	pInfoExt = procXpkAllocInfoExt(objXpk);
	if ( pInfoExt == NULL ) {
		return NULL;
	}
	if ( pInfoExtSrc != NULL ) {
		memcpy(pInfoExt, pInfoExtSrc, objXpk->objHead.infoExtSize);
	}
	return pInfoExt;
}

// 释放条目自有资源
static inline void procXpkFreeEntryOwned(xpkEntry* pEntry)
{
	if ( pEntry == NULL ) {
		return;
	}
	if ( pEntry->pInfoExt != NULL ) {
		xpkFreeInternal(pEntry->pInfoExt);
		pEntry->pInfoExt = NULL;
	}
	if ( pEntry->sPath != NULL ) {
		xpkFreeInternal(pEntry->sPath);
		pEntry->sPath = NULL;
	}
}

// 判断路径是否忽略大小写
static inline int procXpkPathIgnoreCase(xpkObject objXpk)
{
	if ( objXpk == NULL ) {
		return FALSE;
	}
	return (objXpk->objHead.packType == XPK_PACK_WIN32) ? TRUE : FALSE;
}

// 判断字符是否为路径分隔符
static inline int procXpkPathIsSep(xpkObject objXpk, char ch)
{
	if ( ch == '/' ) {
		return TRUE;
	}
	if ( procXpkPathIgnoreCase(objXpk) && ch == '\\' ) {
		return TRUE;
	}
	return FALSE;
}

// 复制指定长度路径文本
static inline char* procXpkDupPathTextN(xpkObject objXpk, const char* sPath, size_t iSizePath)
{
	size_t iPosDst;
	size_t iPosSrc;
	char* sText;
	int bLastSep;

	if ( sPath == NULL ) {
		return NULL;
	}

	sText = procXpkDupTextN(sPath, iSizePath);
	if ( sText == NULL ) {
		return NULL;
	}

	iPosSrc = 0;
	while ( (iPosSrc + 1) < iSizePath ) {
		if ( sPath[iPosSrc] != '.' ) {
			break;
		}
		if ( !procXpkPathIsSep(objXpk, sPath[iPosSrc + 1]) ) {
			break;
		}

		iPosSrc += 2;
		while ( (iPosSrc < iSizePath) && procXpkPathIsSep(objXpk, sPath[iPosSrc]) ) {
			iPosSrc++;
		}
	}

	iPosDst = 0;
	bLastSep = FALSE;
	for ( ; iPosSrc < iSizePath; iPosSrc++ ) {
		if ( procXpkPathIsSep(objXpk, sPath[iPosSrc]) ) {
			if ( bLastSep ) {
				continue;
			}
			sText[iPosDst++] = '/';
			bLastSep = TRUE;
			continue;
		}

		if ( sPath[iPosSrc] == '.' ) {
			if ( ((iPosDst == 0) || bLastSep) &&
				(((iPosSrc + 1) == iSizePath) || procXpkPathIsSep(objXpk, sPath[iPosSrc + 1])) ) {
				continue;
			}
		}

		sText[iPosDst++] = sPath[iPosSrc];
		bLastSep = FALSE;
	}

	if ( (iPosDst > 1) && (sText[iPosDst - 1] == '/') ) {
		iPosDst--;
	}
	sText[iPosDst] = '\0';
	return sText;
}

// 复制路径文本
static inline char* procXpkDupPathText(xpkObject objXpk, const char* sPath)
{
	if ( sPath == NULL ) {
		return NULL;
	}
	return procXpkDupPathTextN(objXpk, sPath, strlen(sPath));
}

// 复制路径原样文本
static inline char* procXpkDupPathStoredText(xpkObject objXpk, const char* sPath)
{
	char* sPathRet;

	if ( sPath == NULL ) {
		return NULL;
	}

	sPathRet = procXpkDupPathText(objXpk, sPath);
	if ( sPathRet == NULL ) {
		if ( sPath[0] != '\0' ) {
			procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
		return NULL;
	}
	if ( strlen(sPathRet) >= XPK_PATH_BYTES ) {
		xpkFreeInternal(sPathRet);
		procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorPathTooLong);
		return NULL;
	}
	if ( sPathRet[0] == '\0' ) {
		xpkFreeInternal(sPathRet);
		procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorPathEmpty);
		return NULL;
	}

	return sPathRet;
}

// 校验原样路径文本
static inline int procXpkValidateStoredPathText(xpkObject objXpk, const char* sPath)
{
	char* sPathDup;

	if ( sPath == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	sPathDup = procXpkDupPathStoredText(objXpk, sPath);
	if ( sPathDup == NULL ) {
		return xpkLastError(objXpk);
	}

	xpkFreeInternal(sPathDup);
	procXpkClearError(objXpk);
	return XPK_OK;
}

// 复制路径键
static inline char* procXpkDupPathKey(xpkObject objXpk, const char* sPath)
{
	size_t iPos;
	size_t iSizePath;
	char* sKey;

	sKey = procXpkDupPathText(objXpk, sPath);
	if ( sKey == NULL ) {
		return NULL;
	}
	if ( !procXpkPathIgnoreCase(objXpk) ) {
		return sKey;
	}

	iSizePath = strlen(sKey);
	for ( iPos = 0; iPos < iSizePath; iPos++ ) {
		sKey[iPos] = (char)tolower((unsigned char)sKey[iPos]);
	}
	return sKey;
}

// 复制路径键已校验
static inline char* procXpkDupPathKeyChecked(xpkObject objXpk, const char* sPath)
{
	size_t iPos;
	size_t iSizePath;
	char* sKey;

	sKey = procXpkDupPathStoredText(objXpk, sPath);
	if ( sKey == NULL ) {
		return NULL;
	}
	if ( !procXpkPathIgnoreCase(objXpk) ) {
		return sKey;
	}

	iSizePath = strlen(sKey);
	for ( iPos = 0; iPos < iSizePath; iPos++ ) {
		sKey[iPos] = (char)tolower((unsigned char)sKey[iPos]);
	}
	return sKey;
}

// 查找路径条目
static inline xpkEntry* procXpkLookupPathEntry(xpkObject objXpk, const char* sPath)
{
	char* sKey;
	char* sEntryKey;
	xpkEntry* pMap;
	xpkEntry* pEntry;

	if ( objXpk == NULL || sPath == NULL ) {
		return NULL;
	}

	procXpkClearError(objXpk);
	if ( procXpkValidateEntryCountState(objXpk) != XPK_OK ) {
		return NULL;
	}
	sKey = procXpkDupPathKeyChecked(objXpk, sPath);
	if ( sKey == NULL ) {
		return NULL;
	}

	pMap = (xpkEntry*)xrtDictGet(&objXpk->tblEntry, (ptr)sKey, (uint32_t)strlen(sKey));
	if ( pMap == NULL ) {
		uint32_t iPosScan;
		for ( iPosScan = 1; iPosScan <= objXpk->iEntryCount; iPosScan++ ) {
			pEntry = (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, iPosScan);
			if ( pEntry == NULL ) {
				if ( sKey != NULL ) {
					xpkFreeInternal(sKey);
				}
				procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
				return NULL;
			}
			if ( ((pEntry->iFlag & XPK_FLAG_DELETED_MASK) != 0) ) {
				continue;
			}
			if ( (pEntry->sPath == NULL) || (pEntry->sPath[0] == '\0') || (strlen(pEntry->sPath) >= XPK_PATH_BYTES) ) {
				if ( sKey != NULL ) {
					xpkFreeInternal(sKey);
				}
				procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
				return NULL;
			}

			sEntryKey = procXpkDupPathKey(objXpk, pEntry->sPath);
			if ( sEntryKey == NULL ) {
				if ( sKey != NULL ) {
					xpkFreeInternal(sKey);
				}
				procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
				return NULL;
			}
			if ( strcmp(sEntryKey, sKey) == 0 ) {
				xpkFreeInternal(sEntryKey);
				if ( sKey != NULL ) {
					xpkFreeInternal(sKey);
				}
				procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
				return NULL;
			}
			xpkFreeInternal(sEntryKey);
		}
		if ( sKey != NULL ) {
			xpkFreeInternal(sKey);
		}
		return NULL;
	}

	if ( pMap->iPos == 0 ) {
		if ( sKey != NULL ) {
			xpkFreeInternal(sKey);
		}
		procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		return NULL;
	}
	pEntry = (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, pMap->iPos);
	if ( pEntry == NULL || ((pEntry->iFlag & XPK_FLAG_DELETED_MASK) != 0) ||
		(pEntry->sPath == NULL) || (pEntry->sPath[0] == '\0') ) {
		if ( sKey != NULL ) {
			xpkFreeInternal(sKey);
		}
		procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		return NULL;
	}
	sEntryKey = procXpkDupPathKey(objXpk, pEntry->sPath);
	if ( sEntryKey == NULL ) {
		if ( sKey != NULL ) {
			xpkFreeInternal(sKey);
		}
		procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		return NULL;
	}
	if ( strcmp(sEntryKey, sKey) != 0 ) {
		xpkFreeInternal(sEntryKey);
		if ( sKey != NULL ) {
			xpkFreeInternal(sKey);
		}
		procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		return NULL;
	}
	xpkFreeInternal(sEntryKey);
	if ( sKey != NULL ) {
		xpkFreeInternal(sKey);
	}
	return pEntry;
}

// 信息扩展大小按包类型
static inline uint32_t procXpkInfoExtSizeByPackType(xpkPackType iType)
{
	switch ( iType ) {
		case XPK_PACK_CORE:
			return 0;
		case XPK_PACK_INDEX:
			return XPK_INFO_EXT_INDEX;
		case XPK_PACK_LINUX:
		case XPK_PACK_WIN32:
			return XPK_INFO_EXT_PATH;
		default:
			return UINT32_MAX;
	}
}

// 条目步长
static inline uint32_t procXpkEntryStride(const xpkHead* pHead)
{
	return XPK_ENTRY_BASE_SIZE + pHead->infoExtSize;
}

// 条目删除
static inline int procXpkEntryDeleted(const xpkEntry* pEntry)
{
	return ((pEntry->iFlag & XPK_FLAG_DELETED_MASK) != 0) ? TRUE : FALSE;
}

// 条目表原始大小
static inline uint64_t procXpkEntryTableRawSize(const xpkHead* pHead)
{
	return (uint64_t)pHead->fileCount * (uint64_t)procXpkEntryStride(pHead);
}

// 获取条目按位置
static inline xpkEntry* procXpkGetEntryByPos(xpkObject objXpk, uint32_t iPos)
{
	if ( (objXpk == NULL) || (iPos == 0) ) {
		return NULL;
	}
	return (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, iPos);
}

// 校验条目数量状态
static inline int procXpkValidateEntryCountState(xpkObject objXpk)
{
	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( objXpk->iEntryCount != objXpk->arrEntry.Count ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}
	return XPK_OK;
}

// 获取公开条目按位置
static inline int procXpkGetPublicEntryByPos(xpkObject objXpk, uint32_t iPos, xpkEntry** ppEntryRet)
{
	xpkEntry* pEntry;

	if ( ppEntryRet != NULL ) {
		*ppEntryRet = NULL;
	}
	if ( objXpk == NULL || iPos == 0 ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( procXpkValidateEntryCountState(objXpk) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	if ( iPos > objXpk->iEntryCount ) {
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorNotFound);
	}

	pEntry = procXpkGetEntryByPos(objXpk, iPos);
	if ( pEntry == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}

	if ( ppEntryRet != NULL ) {
		*ppEntryRet = pEntry;
	}
	return XPK_OK;
}

// 可见条目数量
static inline uint32_t procXpkVisibleEntryCount(xpkObject objXpk)
{
	uint32_t iCount;
	uint32_t iPos;
	xpkEntry* pEntry;

	if ( objXpk == NULL ) {
		return 0;
	}
	if ( objXpk->iEntryCount != objXpk->arrEntry.Count ) {
		return 0;
	}

	iCount = 0;
	for ( iPos = 1; iPos <= objXpk->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, iPos);
		if ( pEntry == NULL ) {
			continue;
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}
		iCount++;
	}
	return iCount;
}

// 可见条目数量严格
static inline int procXpkVisibleEntryCountStrict(xpkObject objXpk, uint32_t* pCountRet)
{
	uint32_t iCount;
	uint32_t iPos;
	xpkEntry* pEntry;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pCountRet == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( procXpkValidateEntryCountState(objXpk) != XPK_OK ) {
		*pCountRet = 0;
		return xpkLastError(objXpk);
	}

	iCount = 0;
	for ( iPos = 1; iPos <= objXpk->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, iPos);
		if ( pEntry == NULL ) {
			*pCountRet = 0;
			return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		}
		if ( ((objXpk->objHead.packType == XPK_PACK_LINUX) || (objXpk->objHead.packType == XPK_PACK_WIN32)) &&
			((pEntry->sPath == NULL) || (pEntry->sPath[0] == '\0')) ) {
			*pCountRet = 0;
			return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		}
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}
		if ( procXpkValidateLiveEntryLookup(objXpk, pEntry) != XPK_OK ) {
			*pCountRet = 0;
			return xpkLastError(objXpk);
		}
		iCount++;
	}

	*pCountRet = iCount;
	procXpkClearError(objXpk);
	return XPK_OK;
}

// 校验有效条目查找
static inline int procXpkValidateLiveEntryLookup(xpkObject objXpk, const xpkEntry* pEntry)
{
	xpkEntry* pMap;
	char* sKey;

	if ( objXpk == NULL || pEntry == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( procXpkEntryDeleted(pEntry) ) {
		return XPK_OK;
	}

	if ( objXpk->objHead.packType == XPK_PACK_INDEX ) {
		pMap = (xpkEntry*)xrtListGet(&objXpk->lstEntry, pEntry->iFileIndex);
		if ( pMap == NULL || pMap->iPos != pEntry->iPos || pMap->iFileIndex != pEntry->iFileIndex ) {
			return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		}
		return XPK_OK;
	}

	if ( (objXpk->objHead.packType != XPK_PACK_LINUX) && (objXpk->objHead.packType != XPK_PACK_WIN32) ) {
		return XPK_OK;
	}
	if ( pEntry->sPath == NULL || pEntry->sPath[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}
	if ( strlen(pEntry->sPath) >= XPK_PATH_BYTES ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}

	sKey = procXpkDupPathKey(objXpk, pEntry->sPath);
	if ( sKey == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}
	pMap = (xpkEntry*)xrtDictGet(&objXpk->tblEntry, (ptr)sKey, (uint32_t)strlen(sKey));
	xpkFreeInternal(sKey);
	if ( pMap == NULL || pMap->iPos != pEntry->iPos ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}

	return XPK_OK;
}

// 释放条目文本
static inline void procXpkFreeEntryText(xpkObject objXpk)
{
	uint32_t iPos;
	xpkEntry* pEntry;

	if ( objXpk == NULL ) {
		return;
	}

	for ( iPos = 1; iPos <= objXpk->arrEntry.Count; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, iPos);
		procXpkFreeEntryOwned(pEntry);
	}
}

// 重置查找
static inline void procXpkResetLookup(xpkObject objXpk)
{
	xrtListUnit(&objXpk->lstEntry);
	xrtDictUnit(&objXpk->tblEntry);
	memset(&objXpk->lstEntry, 0, sizeof(objXpk->lstEntry));
	memset(&objXpk->tblEntry, 0, sizeof(objXpk->tblEntry));

	xrtListInit(&objXpk->lstEntry, sizeof(xpkEntry), XRT_OBJMODE_LOCAL);
	xrtDictInit(&objXpk->tblEntry, sizeof(xpkEntry), XRT_OBJMODE_LOCAL);
}

// 重建查找
static inline int procXpkRebuildLookup(xpkObject objXpk)
{
	uint32_t iPos;
	bool bNew;
	char* sKey;
	xpkEntry* pEntry;
	xpkEntry* pMap;
	if ( objXpk == NULL ) {
		return XPK_ERR_PARAM;
	}
	if ( procXpkValidateEntryCountState(objXpk) != XPK_OK ) {
		return xpkLastError(objXpk);
	}

	procXpkResetLookup(objXpk);
	for ( iPos = 1; iPos <= objXpk->iEntryCount; iPos++ ) {
		pEntry = (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, iPos);
		if ( pEntry == NULL ) {
			procXpkResetLookup(objXpk);
			return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
		}

		pEntry->iPos = iPos;
		if ( procXpkEntryDeleted(pEntry) ) {
			continue;
		}
		if ( objXpk->objHead.packType == XPK_PACK_INDEX ) {
			pMap = (xpkEntry*)xrtListSet(&objXpk->lstEntry, pEntry->iFileIndex, &bNew);
			if ( (pMap == NULL) || !bNew ) {
				procXpkResetLookup(objXpk);
				return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
			}
			*pMap = *pEntry;
		} else if ( (objXpk->objHead.packType == XPK_PACK_LINUX) || (objXpk->objHead.packType == XPK_PACK_WIN32) ) {
			if ( pEntry->sPath == NULL || pEntry->sPath[0] == '\0' ) {
				procXpkResetLookup(objXpk);
				return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
			}
			if ( strlen(pEntry->sPath) >= XPK_PATH_BYTES ) {
				procXpkResetLookup(objXpk);
				return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
			}

			sKey = procXpkDupPathKey(objXpk, pEntry->sPath);
			if ( sKey == NULL ) {
				procXpkResetLookup(objXpk);
				return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
			}

			pMap = (xpkEntry*)xrtDictSet(&objXpk->tblEntry, (ptr)sKey, (uint32_t)strlen(sKey), &bNew);
			if ( sKey != NULL ) {
				xpkFreeInternal(sKey);
			}
			if ( (pMap == NULL) || !bNew ) {
				procXpkResetLookup(objXpk);
				return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
			}
			*pMap = *pEntry;
		}
	}
	return XPK_OK;
}

// 重置条目
static inline void procXpkResetEntries(xpkObject objXpk)
{
	procXpkFreeEntryText(objXpk);

	xrtArrayUnit(&objXpk->arrEntry);
	xrtListUnit(&objXpk->lstEntry);
	xrtDictUnit(&objXpk->tblEntry);
	memset(&objXpk->arrEntry, 0, sizeof(objXpk->arrEntry));
	memset(&objXpk->lstEntry, 0, sizeof(objXpk->lstEntry));
	memset(&objXpk->tblEntry, 0, sizeof(objXpk->tblEntry));

	xrtArrayInit(&objXpk->arrEntry, sizeof(xpkEntry), XRT_OBJMODE_LOCAL);
	xrtListInit(&objXpk->lstEntry, sizeof(xpkEntry), XRT_OBJMODE_LOCAL);
	xrtDictInit(&objXpk->tblEntry, sizeof(xpkEntry), XRT_OBJMODE_LOCAL);

	objXpk->iEntryCount = 0;
}

// 重置已加载状态
static inline void procXpkResetLoadedState(xpkObject objXpk)
{
	if ( objXpk == NULL ) {
		return;
	}

	procXpkUnitWriteQueue(objXpk);
	if ( objXpk->pPackageMeta != NULL ) {
		xpkFreeInternal(objXpk->pPackageMeta);
		objXpk->pPackageMeta = NULL;
	}
	objXpk->iPackageMetaSize = 0;
	procXpkResetEntries(objXpk);
	procXpkInitHead(objXpk);
	objXpk->iAppendPos = XPK_HEAD_SIZE;
	objXpk->iFileSize = XPK_HEAD_SIZE;
	procXpkMarkClean(objXpk);
	procXpkClearError(objXpk);
}

// 追加条目自有
static inline int procXpkAppendEntryOwned(xpkObject objXpk, xpkEntry* pEntrySrc)
{
	uint32_t iPos;
	xpkEntry* pEntry;

	iPos = xrtArrayAppend(&objXpk->arrEntry, 1);
	if ( iPos == 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	pEntry = (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, iPos);
	if ( pEntry == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	memset(pEntry, 0, sizeof(*pEntry));
	*pEntry = *pEntrySrc;
	pEntry->iPos = iPos;

	pEntrySrc->pInfoExt = NULL;
	pEntrySrc->sPath = NULL;
	objXpk->iEntryCount = iPos;
	return XPK_OK;
}

// 标记对象为干净状态
static inline void procXpkMarkClean(xpkObject objXpk)
{
	objXpk->bDirtyHead = FALSE;
	objXpk->bDirtyPackageMeta = FALSE;
	objXpk->bDirtyEntryTable = FALSE;
	objXpk->bDirtyData = FALSE;
}

// 标记脏条目表
static inline void procXpkMarkDirtyEntryTable(xpkObject objXpk)
{
	objXpk->bDirtyEntryTable = TRUE;
	objXpk->bDirtyHead = TRUE;
}

// 初始化默认包头
static inline void procXpkInitHead(xpkObject objXpk)
{
	xtime tNow;

	tNow = xpkNowInternal();
	memset(&objXpk->objHead, 0, sizeof(objXpk->objHead));
	objXpk->objHead.fileHead = XPK_FILE_HEAD;
	objXpk->objHead.packType = XPK_PACK_CORE;
	objXpk->objHead.defComp = 6;
	objXpk->objHead.metaComp = 6;
	objXpk->objHead.infoComp = 6;
	objXpk->objHead.infoExtSize = 0;
	objXpk->objHead.dataOffset = XPK_HEAD_SIZE;
	objXpk->objHead.createTime = tNow;
	objXpk->objHead.changeTime = tNow;
}

// 标记已应用布局
static inline void procXpkMarkAppliedLayout(xpkObject objXpk)
{
	if ( objXpk == NULL ) {
		return;
	}

	objXpk->bSolidApplied = objXpk->objHead.solidMode ? TRUE : FALSE;
	objXpk->bVolumeApplied = objXpk->objHead.volumeMode ? TRUE : FALSE;
	objXpk->iVolumeSizeApplied = objXpk->objHead.volumeSize;
}

// 已应用分卷大小
static inline uint32_t procXpkAppliedVolumeSize(xpkObject objXpk)
{
	if ( objXpk == NULL ) {
		return 0;
	}
	return objXpk->iVolumeSizeApplied;
}

// 已应用分卷模式
static inline int procXpkAppliedVolumeMode(xpkObject objXpk)
{
	if ( objXpk == NULL ) {
		return FALSE;
	}
	return objXpk->bVolumeApplied ? TRUE : FALSE;
}

// 当前数据结束
static inline uint64_t procXpkCurrentDataEnd(xpkObject objXpk)
{
	uint64_t iDataEnd;

	if ( objXpk == NULL ) {
		return 0;
	}

	iDataEnd = objXpk->objHead.dataOffset;
	if ( objXpk->iAppendPos > iDataEnd ) {
		iDataEnd = objXpk->iAppendPos;
	}
	if ( iDataEnd < XPK_HEAD_SIZE ) {
		iDataEnd = XPK_HEAD_SIZE;
	}
	return iDataEnd;
}

// 判断分卷布局是否变化
static inline int procXpkVolumeLayoutChanged(xpkObject objXpk)
{
	if ( objXpk == NULL ) {
		return FALSE;
	}
	if ( procXpkAppliedVolumeMode(objXpk) != (objXpk->objHead.volumeMode ? TRUE : FALSE) ) {
		return TRUE;
	}
	if ( procXpkAppliedVolumeSize(objXpk) != objXpk->objHead.volumeSize ) {
		return TRUE;
	}
	return FALSE;
}

// 判断 Solid 布局是否变化
static inline int procXpkSolidLayoutChanged(xpkObject objXpk)
{
	if ( objXpk == NULL ) {
		return FALSE;
	}
	return (objXpk->bSolidApplied ? TRUE : FALSE) != (objXpk->objHead.solidMode ? TRUE : FALSE);
}

// 可采用目标布局
static inline int procXpkCanAdoptTargetLayout(xpkObject objXpk)
{
	if ( objXpk == NULL ) {
		return FALSE;
	}
	if ( objXpk->iEntryCount != 0 || objXpk->arrEntry.Count != 0 ) {
		return FALSE;
	}
	if ( objXpk->iAppendPos != XPK_HEAD_SIZE ) {
		return FALSE;
	}
	if ( objXpk->iFileSize != XPK_HEAD_SIZE ) {
		return FALSE;
	}
	if ( procXpkWriteQueueCount(objXpk) > 0 ) {
		return FALSE;
	}
	return TRUE;
}

// 初始化对象
static inline void procXpkInitObject(xpkObject objXpk, const xpkOpenOptions* pOpt)
{
	memset(objXpk, 0, sizeof(*objXpk));
	procXpkInitHead(objXpk);
	objXpk->bReadonly = (pOpt != NULL) ? pOpt->readonly : FALSE;
	objXpk->bBufferedDefault = (pOpt != NULL) ? pOpt->bufferedDefault : FALSE;
	objXpk->iAppendPos = XPK_HEAD_SIZE;
	objXpk->iFileSize = XPK_HEAD_SIZE;
	xrtArrayInit(&objXpk->arrEntry, sizeof(xpkEntry), XRT_OBJMODE_LOCAL);
	xrtListInit(&objXpk->lstEntry, sizeof(xpkEntry), XRT_OBJMODE_LOCAL);
	xrtDictInit(&objXpk->tblEntry, sizeof(xpkEntry), XRT_OBJMODE_LOCAL);
	procXpkMarkAppliedLayout(objXpk);
	procXpkClearError(objXpk);
}

// 释放对象
static inline void procXpkUnitObject(xpkObject objXpk)
{
	if ( objXpk == NULL ) {
		return;
	}

	if ( objXpk->sPathPackage != NULL ) {
		xpkFreeInternal(objXpk->sPathPackage);
		objXpk->sPathPackage = NULL;
	}
	if ( objXpk->pPackageMeta != NULL ) {
		xpkFreeInternal(objXpk->pPackageMeta);
		objXpk->pPackageMeta = NULL;
		objXpk->iPackageMetaSize = 0;
	}

	procXpkFreeEntryText(objXpk);
	xrtArrayUnit(&objXpk->arrEntry);
	xrtListUnit(&objXpk->lstEntry);
	xrtDictUnit(&objXpk->tblEntry);
	memset(&objXpk->arrEntry, 0, sizeof(objXpk->arrEntry));
	memset(&objXpk->lstEntry, 0, sizeof(objXpk->lstEntry));
	memset(&objXpk->tblEntry, 0, sizeof(objXpk->tblEntry));
}
