/*
	xPack 文件存储模块

	负责文件映射、分卷扫描、随机读写与物理文件操作。
*/

#include <stdio.h>
#if !defined(_WIN32) && !defined(_WIN64)
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

// 解除文件映射
static inline void procXpkUnmapFile(xpkMappedFile* pMap)
{
	if ( pMap == NULL ) {
		return;
	}

#if defined(_WIN32) || defined(_WIN64)
	if ( pMap->pView != NULL ) {
		UnmapViewOfFile(pMap->pView);
		pMap->pView = NULL;
	}
	if ( pMap->hMap != NULL ) {
		CloseHandle(pMap->hMap);
		pMap->hMap = NULL;
	}
#else
	if ( pMap->pView != NULL ) {
		munmap((void*)pMap->pView, (size_t)pMap->iSize);
		pMap->pView = NULL;
	}
	pMap->pMap = NULL;
#endif
	pMap->iSize = 0;
}

// 建立只读文件映射
static inline int procXpkMapFileReadOnly(xpkObject objXpk, xfile hFile, uint64_t iSize, xpkMappedFile* pMapRet)
{
	if ( pMapRet != NULL ) {
		memset(pMapRet, 0, sizeof(*pMapRet));
	}
	if ( objXpk == NULL || hFile == NULL || pMapRet == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( iSize == 0 ) {
		return XPK_OK;
	}
	if ( iSize > (uint64_t)SIZE_MAX ) {
		return procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge);
	}

#if defined(_WIN32) || defined(_WIN64)
	{
		LARGE_INTEGER iMapSize;

		iMapSize.QuadPart = iSize;
		pMapRet->hMap = CreateFileMapping((HANDLE)hFile->obj, NULL, PAGE_READONLY, iMapSize.HighPart, iMapSize.LowPart, NULL);
		if ( pMapRet->hMap == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoRead);
		}
		pMapRet->pView = MapViewOfFile(pMapRet->hMap, FILE_MAP_READ, 0, 0, 0);
		if ( pMapRet->pView == NULL ) {
			CloseHandle(pMapRet->hMap);
			pMapRet->hMap = NULL;
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoRead);
		}
	}
#else
	pMapRet->pMap = mmap(NULL, (size_t)iSize, PROT_READ, MAP_PRIVATE, hFile->idx, 0);
	if ( pMapRet->pMap == MAP_FAILED ) {
		pMapRet->pMap = NULL;
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoRead);
	}
	pMapRet->pView = pMapRet->pMap;
#endif

	pMapRet->iSize = iSize;
	return XPK_OK;
}

// 建立读写文件映射
static inline int procXpkMapFileReadWrite(xpkObject objXpk, xfile hFile, uint64_t iSize, xpkMappedFile* pMapRet)
{
	size_t iPos;

	if ( pMapRet != NULL ) {
		memset(pMapRet, 0, sizeof(*pMapRet));
	}
	if ( objXpk == NULL || hFile == NULL || pMapRet == NULL ) {
		return procXpkReturnParamError(objXpk);
	}
	if ( iSize == 0 ) {
		return XPK_OK;
	}
	if ( iSize > (uint64_t)SIZE_MAX ) {
		return procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge);
	}
	if ( iSize > (uint64_t)INT64_MAX ) {
		return procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorSeekRange);
	}

	iPos = xrtSeek(hFile, (int64)iSize, XRT_SEEK_SET);
	if ( (uint64_t)iPos != iSize ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoSeek);
	}
	if ( !xrtSetEOF(hFile) ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
	}

#if defined(_WIN32) || defined(_WIN64)
	{
		LARGE_INTEGER iMapSize;

		iMapSize.QuadPart = iSize;
		pMapRet->hMap = CreateFileMapping((HANDLE)hFile->obj, NULL, PAGE_READWRITE, iMapSize.HighPart, iMapSize.LowPart, NULL);
		if ( pMapRet->hMap == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		}
		pMapRet->pView = MapViewOfFile(pMapRet->hMap, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
		if ( pMapRet->pView == NULL ) {
			CloseHandle(pMapRet->hMap);
			pMapRet->hMap = NULL;
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		}
	}
#else
	pMapRet->pMap = mmap(NULL, (size_t)iSize, PROT_READ | PROT_WRITE, MAP_SHARED, hFile->idx, 0);
	if ( pMapRet->pMap == MAP_FAILED ) {
		pMapRet->pMap = NULL;
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
	}
	pMapRet->pView = pMapRet->pMap;
#endif

	pMapRet->iSize = iSize;
	return XPK_OK;
}

// 生成分卷路径文本
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

// 生成带后缀的路径文本
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

// 匹配松散分卷文件名
static inline int procXpkLooseVolumeMatchName(const xpkLooseVolumeScan* pScan, const char* sName, uint32_t* pVolumeRet)
{
	const char* sDigit;
	uint64_t iVolume;

	if ( pVolumeRet != NULL ) {
		*pVolumeRet = 0;
	}
	if ( pScan == NULL || sName == NULL ) {
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
		iVolume = (iVolume * 10u) + (uint64_t)(*sDigit - '0');
		if ( iVolume > UINT32_MAX ) {
			return FALSE;
		}
	}
	if ( iVolume < pScan->iStartVolume ) {
		return FALSE;
	}

	if ( pVolumeRet != NULL ) {
		*pVolumeRet = (uint32_t)iVolume;
	}
	return TRUE;
}

// 拼接路径文本
static inline char* procXpkPathJoinDupText(const char* sDirPath, const char* sName)
{
	size_t iSizeDir;
	size_t iSizeName;
	size_t iSizeSep;
	char* sPathRet;

	if ( sDirPath == NULL || sName == NULL ) {
		return NULL;
	}
	if ( strcmp(sDirPath, ".") == 0 ) {
		return procXpkDupText(sName);
	}

	iSizeDir = strlen(sDirPath);
	iSizeName = strlen(sName);
	iSizeSep = 0;
	if ( iSizeDir > 0 && sDirPath[iSizeDir - 1] != '/' && sDirPath[iSizeDir - 1] != '\\' ) {
		iSizeSep = 1;
	}

	sPathRet = (char*)xpkAllocInternal(iSizeDir + iSizeSep + iSizeName + 1);
	if ( sPathRet == NULL ) {
		return NULL;
	}

	memcpy(sPathRet, sDirPath, iSizeDir);
	if ( iSizeSep != 0 ) {
		sPathRet[iSizeDir] = '/';
	}
	memcpy(sPathRet + iSizeDir + iSizeSep, sName, iSizeName + 1);
	return sPathRet;
}

// 处理松散分卷条目
static inline int procXpkHandleLooseVolumeEntry(xpkLooseVolumeScan* pScan, const char* sDirPath, const char* sName, int bDir)
{
	char* sPathEntry;
	uint32_t iVolume;

	if ( pScan == NULL || sDirPath == NULL || sName == NULL ) {
		return procXpkSetError((pScan != NULL) ? pScan->objXpk : NULL, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( !procXpkLooseVolumeMatchName(pScan, sName, &iVolume) ) {
		return XPK_OK;
	}

	(void)iVolume;
	pScan->iCount++;
	if ( !pScan->bDelete ) {
		return XPK_OK;
	}

	sPathEntry = procXpkPathJoinDupText(sDirPath, sName);
	if ( sPathEntry == NULL ) {
		return procXpkSetError(pScan->objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}
	if ( bDir ) {
		xpkFreeInternal(sPathEntry);
		return procXpkSetError(pScan->objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
	}
	if ( !xrtFileDelete((str)sPathEntry) ) {
		xpkFreeInternal(sPathEntry);
		return procXpkSetError(pScan->objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
	}

	xpkFreeInternal(sPathEntry);
	return XPK_OK;
}

// 路径名称文本
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

// 路径目录复制文本
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

// 松散分卷扫描回调
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

// 扫描松散分卷文件从文本
static inline int procXpkScanLooseVolumeFilesFromText(xpkObject objXpk, const char* sPathPackage, uint32_t iStartVolume, int bDelete, uint32_t* pCountRet)
{
	xpkLooseVolumeScan objScan;
	const char* sNameBase;
	char* sDirPath;
	int iRet;

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

#if defined(_WIN32) || defined(_WIN64)
	{
		WIN32_FIND_DATAA objFindData;
		HANDLE hFind;
		char* sPattern;

		sPattern = procXpkPathJoinDupText(sDirPath, "*");
		if ( sPattern == NULL ) {
			xpkFreeInternal(sDirPath);
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}

		hFind = FindFirstFileA(sPattern, &objFindData);
		xpkFreeInternal(sPattern);
		if ( hFind != INVALID_HANDLE_VALUE ) {
			do {
				if ( strcmp(objFindData.cFileName, ".") == 0 || strcmp(objFindData.cFileName, "..") == 0 ) {
					continue;
				}
				iRet = procXpkHandleLooseVolumeEntry(&objScan, sDirPath, objFindData.cFileName,
					(objFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE);
				if ( iRet != XPK_OK ) {
					objScan.iError = iRet;
					break;
				}
			} while ( FindNextFileA(hFind, &objFindData) );
			FindClose(hFind);
		}
	}
#else
	{
		DIR* pDir;
		struct dirent* pEnt;

		pDir = opendir(sDirPath);
		if ( pDir != NULL ) {
			while ( (pEnt = readdir(pDir)) != NULL ) {
				int bDirEntry;

				if ( strcmp(pEnt->d_name, ".") == 0 || strcmp(pEnt->d_name, "..") == 0 ) {
					continue;
				}
#if defined(DT_DIR)
				if ( pEnt->d_type == DT_DIR ) {
					bDirEntry = TRUE;
				} else if ( pEnt->d_type == DT_REG ) {
					bDirEntry = FALSE;
				} else
#endif
				{
					struct stat objStat;
					char* sPathEntry;

					sPathEntry = procXpkPathJoinDupText(sDirPath, pEnt->d_name);
					if ( sPathEntry == NULL ) {
						objScan.iError = procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
						break;
					}
					bDirEntry = (stat(sPathEntry, &objStat) == 0 && S_ISDIR(objStat.st_mode)) ? TRUE : FALSE;
					xpkFreeInternal(sPathEntry);
				}

				iRet = procXpkHandleLooseVolumeEntry(&objScan, sDirPath, pEnt->d_name, bDirEntry);
				if ( iRet != XPK_OK ) {
					objScan.iError = iRet;
					break;
				}
			}
			closedir(pDir);
		}
	}
#endif

	xpkFreeInternal(sDirPath);
	if ( objScan.iError != XPK_OK ) {
		return objScan.iError;
	}

	if ( pCountRet != NULL ) {
		*pCountRet = objScan.iCount;
	}
	return XPK_OK;
}

// 扫描松散分卷文件文本
static inline int procXpkScanLooseVolumeFilesText(xpkObject objXpk, const char* sPathPackage, int bDelete, uint32_t* pCountRet)
{
	return procXpkScanLooseVolumeFilesFromText(objXpk, sPathPackage, 0, bDelete, pCountRet);
}

// 确保分卷路径未占用文本
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

// 打开分卷文本
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

// 获取分卷物理大小文本
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

// 数量分卷文件文本
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

// 删除分卷文件文本
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

// 删除分卷文件连续文本
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

// 删除分卷文件精确文本
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

// 转移分卷文件文本
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

// 移动分卷文件文本
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

// 定位文件
static inline int procXpkSeekFile(xpkObject objXpk, xfile hFile, uint64_t iOffset)
{
	size_t iPos;

	if ( hFile == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
	}
	if ( iOffset > (uint64_t)INT64_MAX ) {
		return procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorSeekRange);
	}

	iPos = xrtSeek(hFile, (int64)iOffset, XRT_SEEK_SET);
	if ( (uint64_t)iPos != iOffset ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoSeek);
	}
	return XPK_OK;
}

// 计算逻辑文件大小
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

// 读取分卷片段文本
static inline int procXpkReadVolumePartText(xpkObject objXpk, const char* sPathPackage, uint32_t iVolume, uint64_t iOffsetVolume, void* pData, uint32_t iSize)
{
	char* sPathVolume;

	if ( sPathPackage == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( pData == NULL && iSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( iSize == 0 ) {
		return XPK_OK;
	}

	sPathVolume = procXpkVolumePathDupText(sPathPackage, iVolume);
	if ( sPathVolume == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

#if defined(_WIN32) || defined(_WIN64)
	{
		u16str sPathW;
		HANDLE hFile;
		LARGE_INTEGER iSeek;
		DWORD iRead;

		sPathW = xrtUTF8to16((str)sPathVolume, 0, NULL);
		if ( sPathW == NULL ) {
			xpkFreeInternal(sPathVolume);
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}

		hFile = CreateFileW(sPathW, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		xrtFree(sPathW);
		xpkFreeInternal(sPathVolume);
		if ( hFile == INVALID_HANDLE_VALUE ) {
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
		}

		iSeek.QuadPart = (LONGLONG)iOffsetVolume;
		if ( !SetFilePointerEx(hFile, iSeek, NULL, FILE_BEGIN) ) {
			CloseHandle(hFile);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoSeek);
		}

		iRead = 0;
		if ( !ReadFile(hFile, pData, (DWORD)iSize, &iRead, NULL) || iRead != (DWORD)iSize ) {
			CloseHandle(hFile);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoRead);
		}

		CloseHandle(hFile);
	}
#else
	{
		int hFile;
		ssize_t iRead;

		hFile = open(sPathVolume, O_RDONLY);
		xpkFreeInternal(sPathVolume);
		if ( hFile < 0 ) {
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
		}
		if ( lseek(hFile, (off_t)iOffsetVolume, SEEK_SET) < 0 ) {
			close(hFile);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoSeek);
		}
		iRead = read(hFile, pData, (size_t)iSize);
		if ( iRead != (ssize_t)iSize ) {
			close(hFile);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoRead);
		}
		close(hFile);
	}
#endif

	return XPK_OK;
}

// 写入分卷片段文本
static inline int procXpkWriteVolumePartText(xpkObject objXpk, const char* sPathPackage, uint32_t iVolume, uint64_t iOffsetVolume, const void* pData, uint32_t iSize)
{
	char* sPathVolume;

	if ( sPathPackage == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( pData == NULL && iSize > 0 ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( iSize == 0 ) {
		return XPK_OK;
	}

	sPathVolume = procXpkVolumePathDupText(sPathPackage, iVolume);
	if ( sPathVolume == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

#if defined(_WIN32) || defined(_WIN64)
	{
		u16str sPathW;
		HANDLE hFile;
		LARGE_INTEGER iSeek;
		DWORD iWrite;

		sPathW = xrtUTF8to16((str)sPathVolume, 0, NULL);
		if ( sPathW == NULL ) {
			xpkFreeInternal(sPathVolume);
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}

		hFile = CreateFileW(sPathW, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		xrtFree(sPathW);
		xpkFreeInternal(sPathVolume);
		if ( hFile == INVALID_HANDLE_VALUE ) {
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
		}

		iSeek.QuadPart = (LONGLONG)iOffsetVolume;
		if ( !SetFilePointerEx(hFile, iSeek, NULL, FILE_BEGIN) ) {
			CloseHandle(hFile);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoSeek);
		}

		iWrite = 0;
		if ( !WriteFile(hFile, pData, (DWORD)iSize, &iWrite, NULL) || iWrite != (DWORD)iSize ) {
			CloseHandle(hFile);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		}

		CloseHandle(hFile);
	}
#else
	{
		int hFile;
		ssize_t iWrite;

		hFile = open(sPathVolume, O_RDWR | O_CREAT, 0644);
		xpkFreeInternal(sPathVolume);
		if ( hFile < 0 ) {
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoOpen);
		}
		if ( lseek(hFile, (off_t)iOffsetVolume, SEEK_SET) < 0 ) {
			close(hFile);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoSeek);
		}
		iWrite = write(hFile, pData, (size_t)iSize);
		if ( iWrite != (ssize_t)iSize ) {
			close(hFile);
			return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
		}
		close(hFile);
	}
#endif

	return XPK_OK;
}

// 原始读取
static inline int procXpkRawRead(xpkObject objXpk, uint64_t iOffset, void* pData, uint32_t iSize)
{
	uint8_t* pCur;
	uint64_t iOffsetVolume;
	uint64_t iRemainVolume;
	uint32_t iVolumeSize;
	uint32_t iVolume;
	uint32_t iSizePart;
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

		iRet = procXpkReadVolumePartText(objXpk, objXpk->sPathPackage, iVolume, iOffsetVolume, pCur, iSizePart);
		if ( iRet != XPK_OK ) {
			return iRet;
		}

		pCur += iSizePart;
		iOffset += iSizePart;
		iSize -= iSizePart;
	}

	return XPK_OK;
}

// 原始写入
static inline int procXpkRawWrite(xpkObject objXpk, uint64_t iOffset, const void* pData, uint32_t iSize)
{
	const uint8_t* pCur;
	uint64_t iOffsetVolume;
	uint64_t iRemainVolume;
	uint32_t iVolumeSize;
	uint32_t iVolume;
	uint32_t iSizePart;
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

		iRet = procXpkWriteVolumePartText(objXpk, objXpk->sPathPackage, iVolume, iOffsetVolume, pCur, iSizePart);
		if ( iRet != XPK_OK ) {
			return iRet;
		}

		pCur += iSizePart;
		iOffset += iSizePart;
		iSize -= iSizePart;
	}

	return XPK_OK;
}

// 读取按位置分配
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
		pData = xpkAllocInternal(iSize);
		if ( pData == NULL ) {
			xrtClose(hFile);
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
		iRead = xrtGetBuffer(hFile, pData, iSize);
		xrtClose(hFile);
	} else {
		iRet = procXpkSeekFile(objXpk, hFile, iOffset);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		pData = xpkAllocInternal(iSize);
		if ( pData == NULL ) {
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
		iRead = xrtGetBuffer(hFile, pData, iSize);
	}

	if ( (pData == NULL) || (iRead != iSize) ) {
		if ( pData != NULL ) {
			xpkFreeInternal(pData);
		}
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoRead);
	}

	*pDataRet = pData;
	return XPK_OK;
}

// 读取按位置缓冲
static inline int procXpkReadAtBuffer(xpkObject objXpk, xfile hFile, uint64_t iOffset, void* pData, uint32_t iSize)
{
	size_t iRead;
	int iRet;

	if ( objXpk == NULL || pData == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( iSize == 0 ) {
		return XPK_OK;
	}

	if ( procXpkAppliedVolumeMode(objXpk) ) {
		return procXpkRawRead(objXpk, iOffset, pData, iSize);
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
		iRead = xrtGetBuffer(hFile, pData, iSize);
		xrtClose(hFile);
	} else {
		iRet = procXpkSeekFile(objXpk, hFile, iOffset);
		if ( iRet != XPK_OK ) {
			return iRet;
		}
		iRead = xrtGetBuffer(hFile, pData, iSize);
	}

	if ( iRead != iSize ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoRead);
	}

	return XPK_OK;
}

// 写入按位置
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

		iWrite = xrtPut(hFile, (ptr)pData, iSize);
		xrtClose(hFile);
	} else {
		iRet = procXpkSeekFile(objXpk, hFile, iOffset);
		if ( iRet != XPK_OK ) {
			return iRet;
		}

		iWrite = xrtPut(hFile, (ptr)pData, iSize);
	}

	if ( iWrite != iSize ) {
		return procXpkSetError(objXpk, XPK_ERR_IO, sXpkErrorIoWrite);
	}

	return XPK_OK;
}

// 在指定位置设置文件 EOF
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
