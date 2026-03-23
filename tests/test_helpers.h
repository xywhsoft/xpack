#ifdef __linux__
#include <dirent.h>
#endif

static bool procTestWriteBinaryFile(const char* sPathFile, const void* pData, size_t iSize)
{
	xfile hFile;
	int iWriteRet;

	if ( sPathFile == NULL ) {
		return FALSE;
	}
	if ( (pData == NULL) && (iSize > 0) ) {
		return FALSE;
	}

	hFile = xrtOpen((str)sPathFile, FALSE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		return FALSE;
	}

	if ( iSize > 0 ) {
		iWriteRet = xrtPut(hFile, (ptr)pData, iSize);
		if ( (iWriteRet < 0) || ((size_t)iWriteRet != iSize) ) {
			xrtClose(hFile);
			return FALSE;
		}
	}
	if ( !xrtSetEOF(hFile) ) {
		xrtClose(hFile);
		return FALSE;
	}

	xrtClose(hFile);
	return TRUE;
}


static bool procTestCreateSparseFileSize(const char* sPathFile, uint64_t iSize)
{
	xfile hFile;
	size_t iPos;

	if ( sPathFile == NULL ) {
		return FALSE;
	}

	hFile = xrtOpen((str)sPathFile, FALSE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		return FALSE;
	}
	if ( iSize == 0 ) {
		if ( !xrtSetEOF(hFile) ) {
			xrtClose(hFile);
			return FALSE;
		}
		xrtClose(hFile);
		return TRUE;
	}

	iPos = xrtSeek(hFile, (int64)(iSize - 1), XRT_SEEK_SET);
	if ( iPos < (iSize - 1) ) {
		xrtClose(hFile);
		return FALSE;
	}
	if ( xrtPut(hFile, (ptr)"\0", 1) != 1 ) {
		xrtClose(hFile);
		return FALSE;
	}
	if ( !xrtSetEOF(hFile) ) {
		xrtClose(hFile);
		return FALSE;
	}

	xrtClose(hFile);
	return TRUE;
}

static bool procTestFileSizeEquals(const char* sPathFile, uint64_t iSize)
{
	xfile hFile;
	uint64_t iFileSize;

	if ( sPathFile == NULL ) {
		return FALSE;
	}

	hFile = xrtOpen((str)sPathFile, TRUE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		return FALSE;
	}

	iFileSize = xrtGetEOF(hFile);
	xrtClose(hFile);
	return (iFileSize == iSize) ? TRUE : FALSE;
}


static int procTestCountOpenFd(void)
{
#ifdef __linux__
	DIR* hDir;
	struct dirent* pItem;
	int iCount;

	hDir = opendir("/proc/self/fd");
	if ( hDir == NULL ) {
		return -1;
	}

	iCount = 0;
	for ( ;; ) {
		pItem = readdir(hDir);
		if ( pItem == NULL ) {
			break;
		}
		if ( strcmp(pItem->d_name, ".") == 0 || strcmp(pItem->d_name, "..") == 0 ) {
			continue;
		}
		iCount++;
	}

	closedir(hDir);
	return iCount;
#else
	return -1;
#endif
}


static bool procTestWriteVolumeBinaryFile(const char* sPathBase, uint32_t iVolume, const void* pData, size_t iSize)
{
	char* sPathVolume;
	bool bRet;

	if ( sPathBase == NULL ) {
		return FALSE;
	}

	sPathVolume = procXpkVolumePathDupText(sPathBase, iVolume);
	if ( sPathVolume == NULL ) {
		return FALSE;
	}

	bRet = procTestWriteBinaryFile(sPathVolume, pData, iSize);
	xpkFreeInternal(sPathVolume);
	return bRet;
}


static bool procTestCreateDirOccupy(const char* sPathDir)
{
	if ( sPathDir == NULL ) {
		return FALSE;
	}

	if ( xrtDirExists((str)sPathDir) ) {
		return TRUE;
	}

	return xrtDirCreate((str)sPathDir) ? TRUE : FALSE;
}


static void procTestDeletePathFamily(const char* sPathBase)
{
	char* sPathVolume;
	uint32_t iVolume;
	uint32_t iMissCount;

	if ( sPathBase == NULL ) {
		return;
	}

	remove(sPathBase);
	(void)procXpkDeleteVolumeFilesText(NULL, sPathBase, 0);
	(void)procXpkDeleteVolumeFilesContiguousText(NULL, sPathBase, 1);
	iMissCount = 0;
	for ( iVolume = 1; iVolume < 4096u && iMissCount < 32u; iVolume++ ) {
		sPathVolume = procXpkVolumePathDupText(sPathBase, iVolume);
		if ( sPathVolume == NULL ) {
			break;
		}
		if ( xrtFileExists((str)sPathVolume) ) {
			(void)xrtFileDelete((str)sPathVolume);
			iMissCount = 0;
		} else {
			iMissCount++;
		}
		xpkFreeInternal(sPathVolume);
	}
	(void)procXpkScanLooseVolumeFilesText(NULL, sPathBase, TRUE, NULL);
	if ( xrtDirExists((str)sPathBase) ) {
		(void)xrtDirDelete((str)sPathBase);
	}
}


static bool procTestFileContentEquals(const char* sPathFile, const void* pData, size_t iSize)
{
	xfile hFile;
	void* pReadData;
	size_t iReadRet;
	bool bMatch;

	if ( sPathFile == NULL ) {
		return FALSE;
	}
	if ( (pData == NULL) && (iSize > 0) ) {
		return FALSE;
	}

	hFile = xrtOpen((str)sPathFile, TRUE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		return FALSE;
	}

	pReadData = NULL;
	iReadRet = 0;
	if ( iSize > 0 ) {
		pReadData = xpkAllocInternal(iSize);
		if ( pReadData == NULL ) {
			xrtClose(hFile);
			return FALSE;
		}
		iReadRet = xrtGetBuffer(hFile, pReadData, iSize);
	}
	xrtClose(hFile);
	if ( (iSize > 0) && (pReadData == NULL) ) {
		return FALSE;
	}

	bMatch = (iReadRet == iSize) && (memcmp(pReadData, pData, iSize) == 0);
	if ( pReadData != NULL ) {
		xpkFreeInternal(pReadData);
	}
	return bMatch;
}


static bool procTestExpectLastError(xpkObject objXpk, int iError, const char* sErrorText)
{
	if ( xpkLastError(objXpk) != iError ) {
		return FALSE;
	}
	if ( sErrorText == NULL ) {
		return TRUE;
	}
	return strcmp(xpkLastErrorMessage(objXpk), sErrorText) == 0;
}


static int procTestCheckLastError(xpkObject objXpk, int iError, const char* sErrorText)
{
	if ( xpkLastError(objXpk) != iError ) {
		return 1;
	}
	if ( sErrorText == NULL ) {
		return 0;
	}
	return (strcmp(xpkLastErrorMessage(objXpk), sErrorText) == 0) ? 0 : 2;
}


static bool procTestExpectCallError(int iRetActual, int iRetExpected, xpkObject objXpk, int iError, const char* sErrorText)
{
	if ( iRetActual != iRetExpected ) {
		return FALSE;
	}

	return procTestExpectLastError(objXpk, iError, sErrorText);
}


static int procTestCheckCallError(int iRetActual, int iRetExpected, xpkObject objXpk, int iError, const char* sErrorText)
{
	int iCheckRet;

	if ( iRetActual != iRetExpected ) {
		return 1;
	}

	iCheckRet = procTestCheckLastError(objXpk, iError, sErrorText);
	return (iCheckRet == 0) ? 0 : (iCheckRet + 1);
}


static bool procTestExpectNullResultError(const void* pRetActual, xpkObject objXpk, int iError, const char* sErrorText)
{
	if ( pRetActual != NULL ) {
		return FALSE;
	}

	return procTestExpectLastError(objXpk, iError, sErrorText);
}


static int procTestCheckNullResultError(const void* pRetActual, xpkObject objXpk, int iError, const char* sErrorText)
{
	int iCheckRet;

	if ( pRetActual != NULL ) {
		return 1;
	}

	iCheckRet = procTestCheckLastError(objXpk, iError, sErrorText);
	return (iCheckRet == 0) ? 0 : (iCheckRet + 1);
}


static bool procTestExpectFalseError(bool bActual, xpkObject objXpk, int iError, const char* sErrorText)
{
	if ( bActual ) {
		return FALSE;
	}

	return procTestExpectLastError(objXpk, iError, sErrorText);
}


static int procTestCheckFalseError(bool bActual, xpkObject objXpk, int iError, const char* sErrorText)
{
	int iCheckRet;

	if ( bActual ) {
		return 1;
	}

	iCheckRet = procTestCheckLastError(objXpk, iError, sErrorText);
	return (iCheckRet == 0) ? 0 : (iCheckRet + 1);
}
