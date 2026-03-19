#ifndef XPK_STORAGE_FILEIO_H
#define XPK_STORAGE_FILEIO_H

#include <stdio.h>

static inline char* procXpkVolumePathDupText(const char* sPathPackage, uint32_t iVolume)
{
	char sSuffix[32];
	size_t iSizePath;
	size_t iSizeSuffix;
	char* sPathRet;
	int iSizePrint;

	if ( sPathPackage == NULL ) {
		return NULL;
	}
	if ( iVolume == 0 ) {
		return procXpkDupText(sPathPackage);
	}

	iSizePrint = snprintf(sSuffix, sizeof(sSuffix), ".%03u", (unsigned int)iVolume);
	if ( iSizePrint <= 0 || (size_t)iSizePrint >= sizeof(sSuffix) ) {
		return NULL;
	}

	iSizePath = strlen(sPathPackage);
	iSizeSuffix = (size_t)iSizePrint;
	sPathRet = (char*)xpkAllocInternal(iSizePath + iSizeSuffix + 1);
	if ( sPathRet == NULL ) {
		return NULL;
	}

	memcpy(sPathRet, sPathPackage, iSizePath);
	memcpy(sPathRet + iSizePath, sSuffix, iSizeSuffix + 1);
	return sPathRet;
}

static inline char* procXpkPathSuffixDupText(const char* sPathBase, const char* sSuffix)
{
	size_t iSizeBase;
	size_t iSizeSuffix;
	char* sPathRet;

	if ( sPathBase == NULL || sSuffix == NULL ) {
		return NULL;
	}

	iSizeBase = strlen(sPathBase);
	iSizeSuffix = strlen(sSuffix);
	sPathRet = (char*)xpkAllocInternal(iSizeBase + iSizeSuffix + 1);
	if ( sPathRet == NULL ) {
		return NULL;
	}

	memcpy(sPathRet, sPathBase, iSizeBase);
	memcpy(sPathRet + iSizeBase, sSuffix, iSizeSuffix + 1);
	return sPathRet;
}

typedef struct xpkLooseVolumeScan {
	xpkObject objXpk;
	const char* sNameBase;
	size_t iNameSize;
	uint32_t iStartVolume;
	uint32_t iCount;
	int bDelete;
	int iError;
} xpkLooseVolumeScan;

static inline const char* procXpkPathNameText(const char* sPath)
{
	const char* sName;

	if ( sPath == NULL ) {
		return NULL;
	}

	sName = sPath;
	for ( ; *sPath != '\0'; sPath++ ) {
		if ( (*sPath == '/') || (*sPath == '\\') ) {
			sName = sPath + 1;
		}
	}
	return sName;
}

static inline char* procXpkPathDirDupText(const char* sPath)
{
	const char* sSep;
	const char* sCur;
	size_t iSize;
	char* sRet;

	if ( sPath == NULL ) {
		return NULL;
	}

	sSep = NULL;
	for ( sCur = sPath; *sCur != '\0'; sCur++ ) {
		if ( (*sCur == '/') || (*sCur == '\\') ) {
			sSep = sCur;
		}
	}

	if ( sSep == NULL ) {
		return procXpkDupText(".");
	}

	iSize = (size_t)(sSep - sPath + 1);
	sRet = (char*)xpkAllocInternal(iSize + 1);
	if ( sRet == NULL ) {
		return NULL;
	}

	memcpy(sRet, sPath, iSize);
	sRet[iSize] = '\0';
	return sRet;
}

static inline int procXpkLooseVolumeScanProc(ptr sPath, size_t iSize, int bDir, ptr pData, ptr Param)
{
	xpkLooseVolumeScan* pScan;
	const char* sName;
	const char* sDigit;
	uint64_t iVolume;

	(void)iSize;
	(void)pData;
	if ( sPath == NULL || Param == NULL ) {
		return FALSE;
	}
	if ( bDir == 2 ) {
		return FALSE;
	}
	if ( bDir != 0 && bDir != 1 ) {
		return FALSE;
	}

	pScan = (xpkLooseVolumeScan*)Param;
	sName = procXpkPathNameText((const char*)sPath);
	if ( sName == NULL ) {
		return FALSE;
	}
	if ( strncmp(sName, pScan->sNameBase, pScan->iNameSize) != 0 ) {
		return FALSE;
	}
	if ( sName[pScan->iNameSize] != '.' ) {
		return FALSE;
	}

	sDigit = sName + pScan->iNameSize + 1;
	if ( *sDigit == '\0' ) {
		return FALSE;
	}
	iVolume = 0;
	for ( ; *sDigit != '\0'; sDigit++ ) {
		if ( (*sDigit < '0') || (*sDigit > '9') ) {
			return FALSE;
		}
		if ( iVolume < UINT32_MAX ) {
			iVolume = (iVolume * 10u) + (uint64_t)(*sDigit - '0');
			if ( iVolume > UINT32_MAX ) {
				iVolume = UINT32_MAX;
			}
		}
	}
	if ( iVolume < pScan->iStartVolume ) {
		return FALSE;
	}

	pScan->iCount++;
	if ( pScan->bDelete ) {
		if ( bDir != 0 ) {
			pScan->iError = procXpkSetError(pScan->objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
			return TRUE;
		}
		if ( !xrtFileDelete((str)sPath) ) {
			pScan->iError = procXpkSetError(pScan->objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
			return TRUE;
		}
	}

	return FALSE;
}

static inline int procXpkScanLooseVolumeFilesFromText(xpkObject objXpk, const char* sPathPackage, uint32_t iStartVolume, int bDelete, uint32_t* pCountRet)
{
	xpkLooseVolumeScan objScan;
	const char* sNameBase;
	char* sDirPath;

	if ( pCountRet != NULL ) {
		*pCountRet = 0;
	}
	if ( sPathPackage == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	sNameBase = procXpkPathNameText(sPathPackage);
	if ( sNameBase == NULL || sNameBase[0] == '\0' ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	sDirPath = procXpkPathDirDupText(sPathPackage);
	if ( sDirPath == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}
	if ( strcmp(sDirPath, ".") != 0 && !xrtDirExists((str)sDirPath) ) {
		xpkFreeInternal(sDirPath);
		return XPK_OK;
	}

	memset(&objScan, 0, sizeof(objScan));
	objScan.objXpk = objXpk;
	objScan.sNameBase = sNameBase;
	objScan.iNameSize = strlen(sNameBase);
	objScan.iStartVolume = iStartVolume;
	objScan.bDelete = bDelete ? TRUE : FALSE;
	objScan.iError = XPK_OK;
	xrtDirScan((str)sDirPath, FALSE, procXpkLooseVolumeScanProc, &objScan);
	xpkFreeInternal(sDirPath);
	if ( objScan.iError != XPK_OK ) {
		return objScan.iError;
	}

	if ( pCountRet != NULL ) {
		*pCountRet = objScan.iCount;
	}
	return XPK_OK;
}

static inline int procXpkScanLooseVolumeFilesText(xpkObject objXpk, const char* sPathPackage, int bDelete, uint32_t* pCountRet)
{
	return procXpkScanLooseVolumeFilesFromText(objXpk, sPathPackage, 0, bDelete, pCountRet);
}

static inline int procXpkEnsureVolumePathUnusedText(xpkObject objXpk, const char* sPathPackage, const char* sErrorText)
{
	uint32_t iLooseVolumeCount;
	int iRet;

	if ( sPathPackage == NULL || sErrorText == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( xrtPathExists((str)sPathPackage) ) {
		return procXpkSetError(objXpk, XPK_ERR_EXISTS, sErrorText);
	}

	iLooseVolumeCount = 0;
	iRet = procXpkScanLooseVolumeFilesText(objXpk, sPathPackage, FALSE, &iLooseVolumeCount);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( iLooseVolumeCount > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_EXISTS, sErrorText);
	}

	return XPK_OK;
}

static inline int procXpkOpenVolumeText(xpkObject objXpk, const char* sPathPackage, uint32_t iVolume, int bReadonly, xfile* pFileRet)
{
	char* sPathVolume;
	xfile hFile;

	if ( pFileRet == NULL || sPathPackage == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	*pFileRet = NULL;
	sPathVolume = procXpkVolumePathDupText(sPathPackage, iVolume);
	if ( sPathVolume == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	hFile = xrtOpen((str)sPathVolume, bReadonly ? TRUE : FALSE, XRT_CP_BINARY);
	xpkFreeInternal(sPathVolume);
	if ( hFile == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	*pFileRet = hFile;
	return XPK_OK;
}

static inline int procXpkGetVolumePhysicalSizeText(xpkObject objXpk, const char* sPathPackage, uint32_t iVolume, uint64_t* pSizeRet, int* pExistsRet)
{
	char* sPathVolume;
	xfile hFile;

	if ( pSizeRet != NULL ) {
		*pSizeRet = 0;
	}
	if ( pExistsRet != NULL ) {
		*pExistsRet = FALSE;
	}
	if ( sPathPackage == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	sPathVolume = procXpkVolumePathDupText(sPathPackage, iVolume);
	if ( sPathVolume == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	if ( !xrtFileExists((str)sPathVolume) ) {
		xpkFreeInternal(sPathVolume);
		return XPK_OK;
	}

	hFile = xrtOpen((str)sPathVolume, TRUE, XRT_CP_BINARY);
	xpkFreeInternal(sPathVolume);
	if ( hFile == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}

	if ( pSizeRet != NULL ) {
		*pSizeRet = xrtGetEOF(hFile);
	}
	if ( pExistsRet != NULL ) {
		*pExistsRet = TRUE;
	}
	xrtClose(hFile);
	return XPK_OK;
}

static inline int procXpkCountVolumeFilesText(xpkObject objXpk, const char* sPathPackage, uint32_t* pCountRet)
{
	uint32_t iVolume;
	char* sPathVolume;

	if ( pCountRet == NULL || sPathPackage == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	*pCountRet = 0;
	for ( iVolume = 0; ; iVolume++ ) {
		sPathVolume = procXpkVolumePathDupText(sPathPackage, iVolume);
		if ( sPathVolume == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}

		if ( !xrtFileExists((str)sPathVolume) ) {
			xpkFreeInternal(sPathVolume);
			break;
		}

		xpkFreeInternal(sPathVolume);
		(*pCountRet)++;
	}

	return XPK_OK;
}

static inline int procXpkDeleteVolumeFilesText(xpkObject objXpk, const char* sPathPackage, uint32_t iStartVolume)
{
	uint32_t iVolume;
	char* sPathVolume;
	int iRet;

	if ( sPathPackage == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	for ( iVolume = iStartVolume; ; iVolume++ ) {
		sPathVolume = procXpkVolumePathDupText(sPathPackage, iVolume);
		if ( sPathVolume == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}

		if ( !xrtFileExists((str)sPathVolume) ) {
			xpkFreeInternal(sPathVolume);
			break;
		}
		if ( !xrtFileDelete((str)sPathVolume) ) {
			xpkFreeInternal(sPathVolume);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		}

		xpkFreeInternal(sPathVolume);
	}

	iRet = procXpkScanLooseVolumeFilesFromText(objXpk, sPathPackage, (iStartVolume == 0) ? 1u : iStartVolume, TRUE, NULL);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	return XPK_OK;
}

static inline int procXpkDeleteVolumeFilesContiguousText(xpkObject objXpk, const char* sPathPackage, uint32_t iStartVolume)
{
	uint32_t iVolume;
	char* sPathVolume;

	if ( sPathPackage == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	for ( iVolume = iStartVolume; ; iVolume++ ) {
		sPathVolume = procXpkVolumePathDupText(sPathPackage, iVolume);
		if ( sPathVolume == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}

		if ( !xrtFileExists((str)sPathVolume) ) {
			xpkFreeInternal(sPathVolume);
			break;
		}
		if ( !xrtFileDelete((str)sPathVolume) ) {
			xpkFreeInternal(sPathVolume);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		}

		xpkFreeInternal(sPathVolume);
	}

	return XPK_OK;
}

static inline int procXpkDeleteVolumeFilesExactText(xpkObject objXpk, const char* sPathPackage, uint32_t iVolumeCount)
{
	uint32_t iVolume;
	char* sPathVolume;

	if ( sPathPackage == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	for ( iVolume = 0; iVolume < iVolumeCount; iVolume++ ) {
		sPathVolume = procXpkVolumePathDupText(sPathPackage, iVolume);
		if ( sPathVolume == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}

		if ( xrtFileExists((str)sPathVolume) && !xrtFileDelete((str)sPathVolume) ) {
			xpkFreeInternal(sPathVolume);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		}

		xpkFreeInternal(sPathVolume);
	}

	return XPK_OK;
}

static inline int procXpkTransferVolumeFilesText(xpkObject objXpk, const char* sPathSrc, const char* sPathDst, int bMove, int bRewrite)
{
	uint32_t iVolumeCount;
	uint32_t iVolume;
	char* sSrcVolume;
	char* sDstVolume;
	int bOk;
	int iRet;

	if ( sPathSrc == NULL || sPathDst == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	iRet = procXpkCountVolumeFilesText(objXpk, sPathSrc, &iVolumeCount);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( iVolumeCount == 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorNotFound);
	}

	for ( iVolume = 1; iVolume < iVolumeCount; iVolume++ ) {
		sSrcVolume = procXpkVolumePathDupText(sPathSrc, iVolume);
		sDstVolume = procXpkVolumePathDupText(sPathDst, iVolume);
		if ( sSrcVolume == NULL || sDstVolume == NULL ) {
			if ( sSrcVolume != NULL ) {
				xpkFreeInternal(sSrcVolume);
			}
			if ( sDstVolume != NULL ) {
				xpkFreeInternal(sDstVolume);
			}
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}

		if ( bMove ) {
			bOk = xrtFileMove((str)sSrcVolume, (str)sDstVolume, bRewrite ? TRUE : FALSE);
		} else {
			bOk = xrtFileCopy((str)sSrcVolume, (str)sDstVolume, bRewrite ? TRUE : FALSE);
		}
		if ( !bOk ) {
			xpkFreeInternal(sSrcVolume);
			xpkFreeInternal(sDstVolume);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		}

		xpkFreeInternal(sSrcVolume);
		xpkFreeInternal(sDstVolume);
	}

	if ( bMove ) {
		bOk = xrtFileMove((str)sPathSrc, (str)sPathDst, bRewrite ? TRUE : FALSE);
	} else {
		bOk = xrtFileCopy((str)sPathSrc, (str)sPathDst, bRewrite ? TRUE : FALSE);
	}
	if ( !bOk ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
	}

	return XPK_OK;
}

static inline int procXpkMoveVolumeFilesText(xpkObject objXpk, const char* sPathSrc, const char* sPathDst)
{
	uint32_t iVolumeCountSrc;
	uint32_t iVolumeCountDst;
	char* sPathBackup;
	int iRet;
	int iRetRollback;

	if ( sPathSrc == NULL || sPathDst == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	iRet = procXpkCountVolumeFilesText(objXpk, sPathSrc, &iVolumeCountSrc);
	if ( iRet != XPK_OK ) {
		return iRet;
	}
	if ( iVolumeCountSrc == 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_NOT_FOUND, sXpkErrorNotFound);
	}

	iRet = procXpkCountVolumeFilesText(objXpk, sPathDst, &iVolumeCountDst);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	sPathBackup = procXpkPathSuffixDupText(sPathDst, ".replace.bak");
	if ( sPathBackup == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}
	iRet = procXpkEnsureVolumePathUnusedText(objXpk, sPathBackup, sXpkErrorBackupPathExists);
	if ( iRet != XPK_OK ) {
		xpkFreeInternal(sPathBackup);
		return iRet;
	}

	if ( iVolumeCountDst > 0 ) {
		iRet = procXpkTransferVolumeFilesText(objXpk, sPathDst, sPathBackup, FALSE, FALSE);
		if ( iRet != XPK_OK ) {
			(void)procXpkDeleteVolumeFilesExactText(objXpk, sPathBackup, iVolumeCountDst);
			xpkFreeInternal(sPathBackup);
			return iRet;
		}
	}

	iRet = procXpkTransferVolumeFilesText(objXpk, sPathSrc, sPathDst, FALSE, TRUE);
	if ( iRet == XPK_OK ) {
		iRet = procXpkDeleteVolumeFilesText(objXpk, sPathDst, iVolumeCountSrc);
	}
	if ( iRet != XPK_OK ) {
		if ( iVolumeCountDst > 0 ) {
			iRetRollback = procXpkTransferVolumeFilesText(objXpk, sPathBackup, sPathDst, FALSE, TRUE);
			if ( iRetRollback == XPK_OK ) {
				iRetRollback = procXpkDeleteVolumeFilesText(objXpk, sPathDst, iVolumeCountDst);
			}
			if ( iRetRollback != XPK_OK ) {
				iRet = procXpkSetError(objXpk, XPK_ERR_IO, "failed to replace package and rollback");
			} else {
				(void)procXpkDeleteVolumeFilesExactText(objXpk, sPathBackup, iVolumeCountDst);
			}
		} else {
			iRetRollback = procXpkDeleteVolumeFilesText(objXpk, sPathDst, 0);
			if ( iRetRollback != XPK_OK ) {
				iRet = procXpkSetError(objXpk, XPK_ERR_IO, "failed to replace package and cleanup");
			}
		}
		(void)procXpkDeleteVolumeFilesExactText(objXpk, sPathSrc, iVolumeCountSrc);
		xpkFreeInternal(sPathBackup);
		return iRet;
	}

	(void)procXpkDeleteVolumeFilesExactText(objXpk, sPathSrc, iVolumeCountSrc);
	(void)procXpkDeleteVolumeFilesExactText(objXpk, sPathBackup, iVolumeCountDst);
	xpkFreeInternal(sPathBackup);
	return XPK_OK;
}

static inline int procXpkSeekFile(xpkObject objXpk, xfile hFile, uint64_t iOffset)
{
	if ( hFile == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}
	if ( iOffset > LONG_MAX ) {
		return procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorSeekRange);
	}

	xrtSeek(hFile, (long)iOffset, XRT_SEEK_SET);
	return XPK_OK;
}

static inline int procXpkCalcLogicalFileSize(xpkObject objXpk, const xpkHead* pHead, uint64_t* pSizeRet)
{
	uint32_t iVolume;
	uint64_t iSizeVolume;
	uint64_t iSizeUse;
	uint64_t iSizeTotal;
	int bExists;
	int iRet;

	if ( pSizeRet == NULL || pHead == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	*pSizeRet = 0;
	if ( !pHead->volumeMode ) {
		return procXpkGetVolumePhysicalSizeText(objXpk, objXpk->sPathPackage, 0, pSizeRet, NULL);
	}

	iSizeTotal = 0;
	for ( iVolume = 0; ; iVolume++ ) {
		iRet = procXpkGetVolumePhysicalSizeText(objXpk, objXpk->sPathPackage, iVolume, &iSizeVolume, &bExists);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		if ( !bExists ) {
			break;
		}

		iSizeUse = iSizeVolume;
		if ( iSizeUse > pHead->volumeSize ) {
			iSizeUse = pHead->volumeSize;
		}

		iSizeTotal += iSizeUse;
		if ( iSizeUse < pHead->volumeSize ) {
			break;
		}
	}

	*pSizeRet = iSizeTotal;
	return XPK_OK;
}

static inline int procXpkRawRead(xpkObject objXpk, uint64_t iOffset, void* pData, uint32_t iSize)
{
	uint8_t* pCur;
	uint64_t iOffsetVolume;
	uint64_t iRemainVolume;
	uint32_t iVolumeSize;
	uint32_t iVolume;
	uint32_t iSizePart;
	xfile hFile;
	size_t iRead;
	void* pPart;
	int iRet;

	if ( iSize == 0 ) {
		return XPK_OK;
	}
	if ( pData == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	iVolumeSize = procXpkAppliedVolumeSize(objXpk);
	if ( iVolumeSize == 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}

	pCur = (uint8_t*)pData;
	while ( iSize > 0 ) {
		iVolume = (uint32_t)(iOffset / iVolumeSize);
		iOffsetVolume = iOffset % iVolumeSize;
		iRemainVolume = (uint64_t)iVolumeSize - iOffsetVolume;
		iSizePart = (uint32_t)((iRemainVolume < (uint64_t)iSize) ? iRemainVolume : (uint64_t)iSize);

		hFile = NULL;
		iRet = procXpkOpenVolumeText(objXpk, objXpk->sPathPackage, iVolume, TRUE, &hFile);
		if ( iRet != XPK_OK ) {
			return iRet;
		}

		iRet = procXpkSeekFile(objXpk, hFile, iOffsetVolume);
		if ( iRet != XPK_OK ) {
			xrtClose(hFile);
			return iRet;
		}

		pPart = xrtRead(hFile, iSizePart, &iRead);
		xrtClose(hFile);
		if ( (pPart == NULL) || (iRead != iSizePart) ) {
			if ( pPart != NULL ) {
				xrtFree(pPart);
			}
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoRead);
		}

		memcpy(pCur, pPart, iSizePart);
		xrtFree(pPart);

		pCur += iSizePart;
		iOffset += iSizePart;
		iSize -= iSizePart;
	}

	return XPK_OK;
}

static inline int procXpkRawWrite(xpkObject objXpk, uint64_t iOffset, const void* pData, uint32_t iSize)
{
	const uint8_t* pCur;
	uint64_t iOffsetVolume;
	uint64_t iRemainVolume;
	uint32_t iVolumeSize;
	uint32_t iVolume;
	uint32_t iSizePart;
	xfile hFile;
	size_t iWrite;
	int iRet;

	if ( iSize == 0 ) {
		return XPK_OK;
	}
	if ( pData == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	iVolumeSize = procXpkAppliedVolumeSize(objXpk);
	if ( iVolumeSize == 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}

	pCur = (const uint8_t*)pData;
	while ( iSize > 0 ) {
		iVolume = (uint32_t)(iOffset / iVolumeSize);
		iOffsetVolume = iOffset % iVolumeSize;
		iRemainVolume = (uint64_t)iVolumeSize - iOffsetVolume;
		iSizePart = (uint32_t)((iRemainVolume < (uint64_t)iSize) ? iRemainVolume : (uint64_t)iSize);

		hFile = NULL;
		iRet = procXpkOpenVolumeText(objXpk, objXpk->sPathPackage, iVolume, FALSE, &hFile);
		if ( iRet != XPK_OK ) {
			return iRet;
		}

		iRet = procXpkSeekFile(objXpk, hFile, iOffsetVolume);
		if ( iRet != XPK_OK ) {
			xrtClose(hFile);
			return iRet;
		}

		iWrite = xrtWrite(hFile, (str)pCur, iSizePart);
		xrtClose(hFile);
		if ( iWrite != iSizePart ) {
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		}

		pCur += iSizePart;
		iOffset += iSizePart;
		iSize -= iSizePart;
	}

	return XPK_OK;
}

static inline int procXpkReadAtAlloc(xpkObject objXpk, xfile hFile, uint64_t iOffset, uint32_t iSize, void** pDataRet)
{
	size_t iRead;
	void* pData;
	int iRet;

	if ( pDataRet == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	*pDataRet = NULL;
	if ( iSize == 0 ) {
		return XPK_OK;
	}

	if ( procXpkAppliedVolumeMode(objXpk) ) {
		pData = xpkAllocInternal(iSize);
		if ( pData == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}

		iRet = procXpkRawRead(objXpk, iOffset, pData, iSize);
		if ( iRet != XPK_OK ) {
			xpkFreeInternal(pData);
			return iRet;
		}

		*pDataRet = pData;
		return XPK_OK;
	}

	if ( hFile == NULL ) {
		iRet = procXpkOpenVolumeText(objXpk, objXpk->sPathPackage, 0, TRUE, &hFile);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		iRet = procXpkSeekFile(objXpk, hFile, iOffset);
		if ( iRet != XPK_OK ) {
			xrtClose(hFile);
			return iRet;
		}

		pData = xrtRead(hFile, iSize, &iRead);
		xrtClose(hFile);
	} else {
		iRet = procXpkSeekFile(objXpk, hFile, iOffset);
		if ( iRet != XPK_OK ) {
			return iRet;
		}

		pData = xrtRead(hFile, iSize, &iRead);
	}

	if ( (pData == NULL) || (iRead != iSize) ) {
		if ( pData != NULL ) {
			xrtFree(pData);
		}
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoRead);
	}

	*pDataRet = pData;
	return XPK_OK;
}

static inline int procXpkWriteAt(xpkObject objXpk, xfile hFile, uint64_t iOffset, const void* pData, uint32_t iSize)
{
	size_t iWrite;
	int iRet;

	if ( iSize == 0 ) {
		return XPK_OK;
	}
	if ( pData == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	if ( procXpkAppliedVolumeMode(objXpk) ) {
		(void)hFile;
		return procXpkRawWrite(objXpk, iOffset, pData, iSize);
	}

	if ( hFile == NULL ) {
		iRet = procXpkOpenVolumeText(objXpk, objXpk->sPathPackage, 0, FALSE, &hFile);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		iRet = procXpkSeekFile(objXpk, hFile, iOffset);
		if ( iRet != XPK_OK ) {
			xrtClose(hFile);
			return iRet;
		}

		iWrite = xrtWrite(hFile, (str)pData, iSize);
		xrtClose(hFile);
	} else {
		iRet = procXpkSeekFile(objXpk, hFile, iOffset);
		if ( iRet != XPK_OK ) {
			return iRet;
		}

		iWrite = xrtWrite(hFile, (str)pData, iSize);
	}

	if ( iWrite != iSize ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
	}

	return XPK_OK;
}

static inline int procXpkSetEOFAt(xpkObject objXpk, xfile hFile, uint64_t iOffset)
{
	uint32_t iVolumeLast;
	uint32_t iVolume;
	uint32_t iVolumeSize;
	uint64_t iLastSize;
	int iRet;

	if ( !procXpkAppliedVolumeMode(objXpk) ) {
		if ( hFile == NULL ) {
			iRet = procXpkOpenVolumeText(objXpk, objXpk->sPathPackage, 0, FALSE, &hFile);
			if ( iRet != XPK_OK ) {
				return iRet;
			}
			iRet = procXpkSeekFile(objXpk, hFile, iOffset);
			if ( iRet != XPK_OK ) {
				xrtClose(hFile);
				return iRet;
			}
			if ( !xrtSetEOF(hFile) ) {
				xrtClose(hFile);
				return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
			}
			xrtClose(hFile);
			return XPK_OK;
		}

		iRet = procXpkSeekFile(objXpk, hFile, iOffset);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		if ( !xrtSetEOF(hFile) ) {
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		}

		return XPK_OK;
	}
	iVolumeSize = procXpkAppliedVolumeSize(objXpk);
	if ( iVolumeSize == 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}

	if ( iOffset == 0 ) {
		iVolumeLast = 0;
		iLastSize = 0;
	} else {
		iVolumeLast = (uint32_t)((iOffset - 1) / iVolumeSize);
		iLastSize = iOffset - ((uint64_t)iVolumeLast * (uint64_t)iVolumeSize);
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
	return procXpkDeleteVolumeFilesText(objXpk, objXpk->sPathPackage, iVolumeLast + 1);
}

#endif
