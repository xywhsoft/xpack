	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgTest, &objOpt);
	if ( objXpk == NULL ) {
		fprintf(stderr, "runtime_open first open failed: last=%d text=%s\n", xpkLastError(NULL), xpkLastErrorMessage(NULL));
		return 16;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_INDEX) != XPK_OK ) {
		xpkClose(objXpk);
		return 17;
	}
	if ( xpkMetaSet(objXpk, sMetaTest, sizeof(sMetaTest), 6) != XPK_OK ) {
		xpkClose(objXpk);
		return 18;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		fprintf(stderr, "runtime_open phase1 save failed: last=%d text=%s\n", xpkLastError(objXpk), xpkLastErrorMessage(objXpk));
		xpkClose(objXpk);
		return 19;
	}
	xpkClose(objXpk);

	memset(&objOpt, 0, sizeof(objOpt));
	objXpk = xpkOpen(sPathPkgTest, &objOpt);
	if ( objXpk == NULL ) {
		return 20;
	}
	if ( xpkGetPackType(objXpk, &iPackType) != XPK_OK ) {
		xpkClose(objXpk);
		return 21;
	}
	if ( iPackType != XPK_PACK_INDEX ) {
		xpkClose(objXpk);
		return 22;
	}

	pMetaRead = xpkMetaGet(objXpk, &iMetaSize);
	if ( pMetaRead == NULL ) {
		xpkClose(objXpk);
		return 23;
	}
	if ( iMetaSize != sizeof(sMetaTest) ) {
		xpkFree(pMetaRead);
		xpkClose(objXpk);
		return 24;
	}
	if ( memcmp(pMetaRead, sMetaTest, sizeof(sMetaTest)) != 0 ) {
		xpkFree(pMetaRead);
		xpkClose(objXpk);
		return 25;
	}

	xpkFree(pMetaRead);
	xpkClose(objXpk);
	remove(sPathPkgTest);

	objXpk = xpkOpen(NULL, NULL);
	iCheckRet = procTestCheckNullResultError(objXpk, NULL, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	if ( iCheckRet != 0 ) {
		if ( objXpk != NULL ) {
			xpkClose(objXpk);
			return 406;
		}
		return (iCheckRet == 2) ? 407 : 408;
	}
	if ( xpkCount(NULL) != 0 ) {
		return 414;
	}
	if ( !procTestExpectLastError(NULL, XPK_ERR_PARAM, NULL) ) {
		return 415;
	}
	pMetaRead = xpkMetaGet(NULL, &iMetaSize);
	if ( !procTestExpectNullResultError(pMetaRead, NULL, XPK_ERR_PARAM, NULL) ) {
		if ( pMetaRead != NULL ) {
			xpkFree(pMetaRead);
			return 416;
		}
		return 417;
	}
	pDataRead = xpkReadToMemory(NULL, 1, &iDataSize);
	if ( !procTestExpectNullResultError(pDataRead, NULL, XPK_ERR_PARAM, NULL) ) {
		if ( pDataRead != NULL ) {
			xpkFree(pDataRead);
			return 418;
		}
		return 419;
	}
	iCheckRet = procTestCheckFalseError(xpkPathExists(NULL, "assets/null.txt"), NULL, XPK_ERR_PARAM, NULL);
	if ( iCheckRet != 0 ) {
		return (iCheckRet == 2) ? 421 : 420;
	}

	objXpk = xpkOpen("release/x64/xpack_phase3_missing_open.xpk", NULL);
	iCheckRet = procTestCheckNullResultError(objXpk, NULL, XPK_ERR_NOT_FOUND, sXpkErrorNotFound);
	if ( iCheckRet != 0 ) {
		if ( objXpk != NULL ) {
			xpkClose(objXpk);
			return 409;
		}
		return (iCheckRet == 2) ? 410 : 411;
	}
	if ( !procTestCreateDirOccupy(sPathPkgOpenDir) ) {
		return 925;
	}
	objXpk = xpkOpen(sPathPkgOpenDir, NULL);
	iCheckRet = procTestCheckNullResultError(objXpk, NULL, XPK_ERR_IO, sXpkErrorIoOpen);
	if ( iCheckRet != 0 ) {
		if ( objXpk != NULL ) {
			xpkClose(objXpk);
			procTestDeletePathFamily(sPathPkgOpenDir);
			return 926;
		}
		procTestDeletePathFamily(sPathPkgOpenDir);
		return (iCheckRet == 2) ? 927 : 928;
	}

	procTestDeletePathFamily(sPathPkgTest);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgOpenDir, &objOpt);
	iCheckRet = procTestCheckNullResultError(objXpk, NULL, XPK_ERR_IO, sXpkErrorIoOpen);
	if ( iCheckRet != 0 ) {
		if ( objXpk != NULL ) {
			xpkClose(objXpk);
			procTestDeletePathFamily(sPathPkgOpenDir);
			return 929;
		}
		procTestDeletePathFamily(sPathPkgOpenDir);
		return (iCheckRet == 2) ? 930 : 931;
	}
	if ( !xrtDirExists((str)sPathPkgOpenDir) ) {
		procTestDeletePathFamily(sPathPkgOpenDir);
		return 932;
	}
	procTestDeletePathFamily(sPathPkgOpenDir);
	sPathKey = procXpkVolumePathDupText(sPathPkgOpenSparse, 5);
	if ( sPathKey == NULL ) {
		return 957;
	}
	if ( !procTestWriteVolumeBinaryFile(sPathPkgOpenSparse, 5, "KEEP", 4) ) {
		xpkFreeInternal(sPathKey);
		return 958;
	}

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgOpenSparse, &objOpt);
	if ( objXpk != NULL ) {
		xpkClose(objXpk);
		procTestDeletePathFamily(sPathPkgOpenSparse);
		xpkFreeInternal(sPathKey);
		return 960;
	}
	if ( !procTestExpectLastError(NULL, XPK_ERR_EXISTS, sXpkErrorPackagePathExists) ) {
		procTestDeletePathFamily(sPathPkgOpenSparse);
		xpkFreeInternal(sPathKey);
		return 961;
	}
	if ( !procTestFileContentEquals(sPathKey, "KEEP", 4) ) {
		procTestDeletePathFamily(sPathPkgOpenSparse);
		xpkFreeInternal(sPathKey);
		return 962;
	}
	procTestDeletePathFamily(sPathPkgOpenSparse);
	xpkFreeInternal(sPathKey);
	hFile = xrtOpen((str)sPathPkgOpenZeroSparse, FALSE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		return 966;
	}
	xrtClose(hFile);
	sPathKey = procXpkVolumePathDupText(sPathPkgOpenZeroSparse, 5);
	if ( sPathKey == NULL ) {
		remove(sPathPkgOpenZeroSparse);
		return 967;
	}
	if ( !procTestWriteVolumeBinaryFile(sPathPkgOpenZeroSparse, 5, "KEEP", 4) ) {
		remove(sPathPkgOpenZeroSparse);
		xpkFreeInternal(sPathKey);
		return 968;
	}
	objXpk = xpkOpen(sPathPkgOpenZeroSparse, NULL);
	if ( objXpk != NULL ) {
		xpkClose(objXpk);
		remove(sPathPkgOpenZeroSparse);
		procTestDeletePathFamily(sPathPkgOpenZeroSparse);
		xpkFreeInternal(sPathKey);
		return 970;
	}
	if ( !procTestExpectLastError(NULL, XPK_ERR_EXISTS, sXpkErrorPackagePathExists) ) {
		fprintf(stderr, "runtime_open zero_sparse last=%d text=%s\n", xpkLastError(NULL), xpkLastErrorMessage(NULL));
		remove(sPathPkgOpenZeroSparse);
		procTestDeletePathFamily(sPathPkgOpenZeroSparse);
		xpkFreeInternal(sPathKey);
		return 971;
	}
	if ( !procTestFileContentEquals(sPathKey, "KEEP", 4) ) {
		remove(sPathPkgOpenZeroSparse);
		procTestDeletePathFamily(sPathPkgOpenZeroSparse);
		xpkFreeInternal(sPathKey);
		return 972;
	}
	remove(sPathPkgOpenZeroSparse);
	procTestDeletePathFamily(sPathPkgOpenZeroSparse);
	xpkFreeInternal(sPathKey);
