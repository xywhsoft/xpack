static bool procTestWriteBinaryFile(const char* sPathFile, const void* pData, size_t iSize)
{
	xfile hFile;
	long iWriteRet;

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
		iWriteRet = xrtWrite(hFile, (str)pData, iSize);
		if ( (iWriteRet < 0) || ((size_t)iWriteRet != iSize) ) {
			xrtClose(hFile);
			return FALSE;
		}
	}

	xrtClose(hFile);
	return TRUE;
}


static bool procTestCreateSparseFileSize(const char* sPathFile, uint64_t iSize)
{
	xfile hFile;

	if ( sPathFile == NULL ) {
		return FALSE;
	}

	hFile = xrtOpen((str)sPathFile, FALSE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		return FALSE;
	}

	xrtSeek(hFile, (int64)iSize, XRT_SEEK_SET);
	if ( xrtTell(hFile) != iSize ) {
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
	if ( sPathBase == NULL ) {
		return;
	}

	remove(sPathBase);
	(void)procXpkDeleteVolumeFilesText(NULL, sPathBase, 0);
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

	pReadData = xrtRead(hFile, iSize, &iReadRet);
	xrtClose(hFile);
	if ( pReadData == NULL ) {
		return FALSE;
	}

	bMatch = (iReadRet == iSize) && (memcmp(pReadData, pData, iSize) == 0);
	xrtFree(pReadData);
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
