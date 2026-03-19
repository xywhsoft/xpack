	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgBuild, &objOpt);
	if ( objXpk == NULL ) {
		return 66;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_INDEX) != XPK_OK ) {
		xpkClose(objXpk);
		return 67;
	}

	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkIndexAddData(objXpk, 2001, sDataBuildA, sizeof(sDataBuildA), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 68;
	}
	if ( xpkIndexAddData(objXpk, 2002, sDataBuildB, sizeof(sDataBuildB), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 69;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 70;
	}
	iMatchCount = 0;
	if ( xpkEachMatch(objXpk, "200?", NULL, &iMatchCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 71;
	}
	if ( iMatchCount != 2 ) {
		xpkClose(objXpk);
		return 72;
	}
	if ( xpkIndexRemove(objXpk, 2001) != XPK_OK ) {
		xpkClose(objXpk);
		return 73;
	}
	if ( xpkCount(objXpk) != 1 ) {
		xpkClose(objXpk);
		return 297;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_ERR_NOT_FOUND ) {
		xpkClose(objXpk);
		return 298;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 74;
	}
	if ( xpkStatGet(objXpk, &objStat) != XPK_OK ) {
		xpkClose(objXpk);
		return 75;
	}
	if ( objStat.fileCount != 1 ) {
		xpkClose(objXpk);
		return 76;
	}
	if ( objStat.liveDataBytes != sizeof(sDataBuildB) ) {
		xpkClose(objXpk);
		return 77;
	}
	if ( objStat.holeBytes != sizeof(sDataBuildA) ) {
		xpkClose(objXpk);
		return 78;
	}

	hFile = xrtOpen((str)sPathPkgBuild, TRUE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		xpkClose(objXpk);
		return 79;
	}
	iBuildSizeBefore = xrtGetEOF(hFile);
	xrtClose(hFile);

	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 80;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 81;
	}
	if ( xpkStatGet(objXpk, &objStat) != XPK_OK ) {
		xpkClose(objXpk);
		return 82;
	}
	if ( objStat.fileCount != 1 ) {
		xpkClose(objXpk);
		return 83;
	}
	if ( objStat.liveDataBytes != sizeof(sDataBuildB) ) {
		xpkClose(objXpk);
		return 84;
	}
	if ( objStat.holeBytes != 0 ) {
		xpkClose(objXpk);
		return 85;
	}
	if ( xpkCount(objXpk) != 1 ) {
		xpkClose(objXpk);
		return 86;
	}
	if ( xpkIndexFind(objXpk, 2001, &iPosRet) != XPK_ERR_NOT_FOUND ) {
		xpkClose(objXpk);
		return 87;
	}
	pDataRead = xpkIndexReadToMemory(objXpk, 2002, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 88;
	}
	if ( iDataSize != sizeof(sDataBuildB) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 89;
	}
	if ( memcmp(pDataRead, sDataBuildB, sizeof(sDataBuildB)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 90;
	}
	xpkFree(pDataRead);

	hFile = xrtOpen((str)sPathPkgBuild, TRUE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		xpkClose(objXpk);
		return 91;
	}
	iBuildSizeAfter = xrtGetEOF(hFile);
	xrtClose(hFile);
	if ( iBuildSizeAfter >= iBuildSizeBefore ) {
		xpkClose(objXpk);
		return 92;
	}
	if ( !procTestWriteBinaryFile(sPathPkgBuildExistsTemp, "KEEP", 4) ) {
		xpkClose(objXpk);
		return 801;
	}

	memset(&objBuildOpt, 0, sizeof(objBuildOpt));
	objBuildOpt.tempPath = sPathPkgBuildExistsTemp;
	objBuildOpt.replaceOriginal = FALSE;
	iRetCall = xpkBuild(objXpk, &objBuildOpt);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_EXISTS, objXpk, XPK_ERR_EXISTS, sXpkErrorTempPathExists) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_EXISTS) ? 803 : 804;
	}
	if ( !procTestFileContentEquals(sPathPkgBuildExistsTemp, "KEEP", 4) ) {
		xpkClose(objXpk);
		return 805;
	}
	sPathKey = procXpkPathSuffixDupText(sPathPkgBuild, ".build.tmp");
	if ( sPathKey == NULL ) {
		xpkClose(objXpk);
		return 897;
	}
	if ( !procTestWriteBinaryFile(sPathKey, "KEEP", 4) ) {
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 898;
	}
	iRetCall = xpkBuild(objXpk, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_EXISTS, objXpk, XPK_ERR_EXISTS, sXpkErrorTempPathExists) ) {
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_EXISTS) ? 900 : 901;
	}
	if ( !procTestFileContentEquals(sPathKey, "KEEP", 4) ) {
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 902;
	}
	sPathVolume = procXpkVolumePathDupText(sPathKey, 2);
	if ( sPathVolume == NULL ) {
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 905;
	}
	if ( !procTestWriteBinaryFile(sPathVolume, "SPARSE", 6) ) {
		xpkFreeInternal(sPathVolume);
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 906;
	}
	iRetCall = xpkBuild(objXpk, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_EXISTS, objXpk, XPK_ERR_EXISTS, sXpkErrorTempPathExists) ) {
		xpkFreeInternal(sPathVolume);
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_EXISTS) ? 908 : 909;
	}
	if ( !procTestFileContentEquals(sPathVolume, "SPARSE", 6) ) {
		xpkFreeInternal(sPathVolume);
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 910;
	}
	xpkFreeInternal(sPathVolume);
	sPathVolume = procXpkVolumePathDupText(sPathKey, 1);
	if ( sPathVolume == NULL ) {
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 917;
	}
	if ( !procTestCreateDirOccupy(sPathVolume) ) {
		xpkFreeInternal(sPathVolume);
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 918;
	}
	iRetCall = xpkBuild(objXpk, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_EXISTS, objXpk, XPK_ERR_EXISTS, sXpkErrorTempPathExists) ) {
		procTestDeletePathFamily(sPathVolume);
		xpkFreeInternal(sPathVolume);
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_EXISTS) ? 919 : 920;
	}
	if ( !xrtDirExists((str)sPathVolume) ) {
		procTestDeletePathFamily(sPathVolume);
		xpkFreeInternal(sPathVolume);
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 921;
	}
	procTestDeletePathFamily(sPathVolume);
	if ( !procTestCreateDirOccupy(sPathPkgBuildExistsDir) ) {
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 913;
	}
	memset(&objBuildOpt, 0, sizeof(objBuildOpt));
	objBuildOpt.tempPath = sPathPkgBuildExistsDir;
	objBuildOpt.replaceOriginal = FALSE;
	iRetCall = xpkBuild(objXpk, &objBuildOpt);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_EXISTS, objXpk, XPK_ERR_EXISTS, sXpkErrorTempPathExists) ) {
		procTestDeletePathFamily(sPathPkgBuildExistsDir);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_EXISTS) ? 914 : 915;
	}
	if ( !xrtDirExists((str)sPathPkgBuildExistsDir) ) {
		procTestDeletePathFamily(sPathPkgBuildExistsDir);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 916;
	}
	procTestDeletePathFamily(sPathPkgBuildExistsDir);
	xpkFreeInternal(sPathVolume);
	procTestDeletePathFamily(sPathKey);
	xpkFreeInternal(sPathKey);
	xpkClose(objXpk);

	hFile = xrtOpen((str)sPathPkgBuild, FALSE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		return 93;
	}
	xrtSeek(hFile, (long)XPK_HEAD_SIZE, XRT_SEEK_SET);
	iCorruptByte = (uint8_t)'C';
	if ( xrtWrite(hFile, (str)&iCorruptByte, 1) != 1 ) {
		xrtClose(hFile);
		return 94;
	}
	xrtClose(hFile);

	objXpk = xpkOpen(sPathPkgBuild, NULL);
	if ( objXpk == NULL ) {
		return 95;
	}
	if ( xpkVerify(objXpk, 1) != XPK_ERR_HASH ) {
		xpkClose(objXpk);
		return 96;
	}
	xpkClose(objXpk);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
