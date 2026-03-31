static inline int procXpkEnsurePackageMutable(xpkObject objXpk)
{
	int iRet;

	iRet = procXpkEnsureWritable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( procXpkValidateEntryCountState(objXpk) != XPK_OK ) {
		return xpkLastError(objXpk);
	}
	return XPK_OK;
}

XPKAPI xpkObject xpkOpen(const char* sPackagePath, const xpkOpenOptions* pOpt)
{
	xpkObject objXpk;
	int iRet;

	if ( sPackagePath == NULL || sPackagePath[0] == '\0' ) {
		procXpkSetError(NULL, XPK_ERR_PARAM, sXpkErrorInvalidParam);
		return NULL;
	}

	objXpk = (xpkObject)xpkAllocInternal(sizeof(*objXpk));
	if ( objXpk == NULL ) {
		procXpkSetError(NULL, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		return NULL;
	}

	procXpkInitObject(objXpk, pOpt);
	objXpk->sPathPackage = procXpkDupText(sPackagePath);
	if ( objXpk->sPathPackage == NULL ) {
		procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		procXpkUnitObject(objXpk);
		xpkFreeInternal(objXpk);
		return NULL;
	}

	iRet = procXpkLoadPackage(objXpk, pOpt);
	if ( iRet != XPK_OK ) {
		procXpkUnitObject(objXpk);
		xpkFreeInternal(objXpk);
		return NULL;
	}

	return objXpk;
}

XPKAPI int xpkClose(xpkObject objXpk)
{
	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}

	procXpkClearError(objXpk);
	procXpkUnitWriteQueue(objXpk);
	procXpkUnitObject(objXpk);
	xpkFreeInternal(objXpk);
	return XPK_OK;
}

XPKAPI int xpkSave(xpkObject objXpk)
{
	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	return procXpkSavePackage(objXpk);
}

XPKAPI int xpkBuild(xpkObject objXpk, const xpkBuildOptions* pOpt)
{
	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	return procXpkBuildPackage(objXpk, pOpt);
}

XPKAPI int xpkGetPackType(xpkObject objXpk, xpkPackType* pTypeRet)
{
	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pTypeRet == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	*pTypeRet = (xpkPackType)objXpk->objHead.packType;
	procXpkClearError(objXpk);
	return XPK_OK;
}

XPKAPI int xpkSetPackType(xpkObject objXpk, xpkPackType iType)
{
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( (iType < XPK_PACK_CORE) || (iType > XPK_PACK_WIN32) ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	iRet = procXpkEnsurePackageMutable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( objXpk->objHead.packType == (uint32_t)iType ) {
		procXpkClearError(objXpk);
		return XPK_OK;
	}
	if ( objXpk->arrEntry.Count != 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeLocked);
	}

	procXpkApplyPackType(objXpk, iType);
	objXpk->bDirtyHead = TRUE;
	procXpkClearError(objXpk);
	return XPK_OK;
}

XPKAPI int xpkGetDefaultComp(xpkObject objXpk, uint8_t* pLevelRet)
{
	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pLevelRet == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	*pLevelRet = (uint8_t)objXpk->objHead.defComp;
	procXpkClearError(objXpk);
	return XPK_OK;
}

XPKAPI int xpkSetDefaultComp(xpkObject objXpk, uint8_t iLevel)
{
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( iLevel > 15 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	iRet = procXpkEnsurePackageMutable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	objXpk->objHead.defComp = iLevel;
	objXpk->bDirtyHead = TRUE;
	procXpkClearError(objXpk);
	return XPK_OK;
}

XPKAPI int xpkGetMetaComp(xpkObject objXpk, uint8_t* pLevelRet)
{
	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pLevelRet == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	*pLevelRet = (uint8_t)objXpk->objHead.metaComp;
	procXpkClearError(objXpk);
	return XPK_OK;
}

XPKAPI int xpkSetMetaComp(xpkObject objXpk, uint8_t iLevel)
{
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( iLevel > 15 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	iRet = procXpkEnsurePackageMutable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	objXpk->objHead.metaComp = iLevel;
	objXpk->bDirtyHead = TRUE;
	procXpkClearError(objXpk);
	return XPK_OK;
}

XPKAPI int xpkGetInfoComp(xpkObject objXpk, uint8_t* pLevelRet)
{
	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pLevelRet == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	*pLevelRet = (uint8_t)objXpk->objHead.infoComp;
	procXpkClearError(objXpk);
	return XPK_OK;
}

XPKAPI int xpkSetInfoComp(xpkObject objXpk, uint8_t iLevel)
{
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( iLevel > 15 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	iRet = procXpkEnsurePackageMutable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	objXpk->objHead.infoComp = iLevel;
	objXpk->bDirtyHead = TRUE;
	procXpkClearError(objXpk);
	return XPK_OK;
}

XPKAPI int xpkGetInfoExtSize(xpkObject objXpk, uint32_t* pSizeRet)
{
	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pSizeRet == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	*pSizeRet = objXpk->objHead.infoExtSize;
	procXpkClearError(objXpk);
	return XPK_OK;
}

XPKAPI int xpkSetInfoExtSize(xpkObject objXpk, uint32_t iSize)
{
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( iSize > 0x3FFFFu ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( objXpk->objHead.packType != XPK_PACK_CORE ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
	}
	iRet = procXpkEnsurePackageMutable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( objXpk->objHead.infoExtSize == iSize ) {
		procXpkClearError(objXpk);
		return XPK_OK;
	}
	if ( objXpk->arrEntry.Count != 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorInfoExtLocked);
	}

	objXpk->objHead.infoExtSize = iSize;
	objXpk->bDirtyHead = TRUE;
	procXpkClearError(objXpk);
	return XPK_OK;
}

XPKAPI int xpkGetVolumeSize(xpkObject objXpk, uint32_t* pSizeRet)
{
	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pSizeRet == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	*pSizeRet = objXpk->objHead.volumeSize;
	procXpkClearError(objXpk);
	return XPK_OK;
}

XPKAPI int xpkSetVolumeSize(xpkObject objXpk, uint32_t iSize)
{
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( iSize != 0 && iSize < XPK_VOLUME_MIN ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, "volume size is below minimum 64KB");
	}
	iRet = procXpkEnsurePackageMutable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	objXpk->objHead.volumeSize = iSize;
	objXpk->objHead.volumeMode = (iSize != 0) ? 1 : 0;
	if ( procXpkCanAdoptTargetLayout(objXpk) ) {
		objXpk->bVolumeApplied = objXpk->objHead.volumeMode ? TRUE : FALSE;
		objXpk->iVolumeSizeApplied = objXpk->objHead.volumeSize;
	}
	objXpk->bDirtyHead = TRUE;
	procXpkClearError(objXpk);
	return XPK_OK;
}

XPKAPI int xpkGetSolidMode(xpkObject objXpk, int* pEnabledRet)
{
	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( pEnabledRet == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	*pEnabledRet = objXpk->objHead.solidMode ? TRUE : FALSE;
	procXpkClearError(objXpk);
	return XPK_OK;
}

XPKAPI int xpkSetSolidMode(xpkObject objXpk, int bEnabled)
{
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	iRet = procXpkEnsurePackageMutable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	objXpk->objHead.solidMode = bEnabled ? 1 : 0;
	if ( procXpkCanAdoptTargetLayout(objXpk) ) {
		objXpk->bSolidApplied = objXpk->objHead.solidMode ? TRUE : FALSE;
	}
	objXpk->bDirtyHead = TRUE;
	procXpkClearError(objXpk);
	return XPK_OK;
}

XPKAPI void* xpkMetaGet(xpkObject objXpk, uint32_t* pSizeRet)
{
	void* pRet;

	if ( pSizeRet ) {
		*pSizeRet = 0;
	}
	if ( objXpk == NULL ) {
		procXpkSetError(NULL, XPK_ERR_PARAM, sXpkErrorInvalidParam);
		return NULL;
	}
	if ( objXpk->pPackageMeta == NULL || objXpk->iPackageMetaSize == 0 ) {
		procXpkClearError(objXpk);
		return NULL;
	}

	pRet = xpkAllocInternal(objXpk->iPackageMetaSize);
	if ( pRet == NULL ) {
		procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		return NULL;
	}
	memcpy(pRet, objXpk->pPackageMeta, objXpk->iPackageMetaSize);
	if ( pSizeRet ) {
		*pSizeRet = objXpk->iPackageMetaSize;
	}
	procXpkClearError(objXpk);
	return pRet;
}

XPKAPI int xpkMetaSet(xpkObject objXpk, const void* pData, uint32_t iSize, uint8_t iCompLevel)
{
	void* pMetaNew;
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( iCompLevel > 15 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( iSize > 0 && pData == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	iRet = procXpkEnsurePackageMutable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	pMetaNew = NULL;
	if ( iSize > 0 ) {
		pMetaNew = xpkAllocInternal(iSize);
		if ( pMetaNew == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
		memcpy(pMetaNew, pData, iSize);
	}

	if ( objXpk->pPackageMeta ) {
		xpkFreeInternal(objXpk->pPackageMeta);
	}
	objXpk->pPackageMeta = pMetaNew;
	objXpk->iPackageMetaSize = iSize;
	objXpk->objHead.metaComp = iCompLevel;
	objXpk->bDirtyPackageMeta = TRUE;
	objXpk->bDirtyHead = TRUE;
	procXpkClearError(objXpk);
	return XPK_OK;
}

XPKAPI int xpkMetaClear(xpkObject objXpk)
{
	int iRet;

	if ( objXpk == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	iRet = procXpkEnsurePackageMutable(objXpk);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( objXpk->pPackageMeta ) {
		xpkFreeInternal(objXpk->pPackageMeta);
	}
	objXpk->pPackageMeta = NULL;
	objXpk->iPackageMetaSize = 0;
	objXpk->bDirtyPackageMeta = TRUE;
	objXpk->bDirtyHead = TRUE;
	procXpkClearError(objXpk);
	return XPK_OK;
}
