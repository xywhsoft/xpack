	objXpk = xpkOpen(sPathPkgVolume, &objOpt);
	if ( objXpk == NULL ) {
		return 117;
	}
	if ( xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN) != XPK_OK ) {
		xpkClose(objXpk);
		return 118;
	}

	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, sDataVolumeA, sizeof(sDataVolumeA), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 119;
	}
	if ( iPosRet != 1 ) {
		xpkClose(objXpk);
		return 120;
	}
	if ( xpkAddData(objXpk, sDataVolumeB, sizeof(sDataVolumeB), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 121;
	}
	if ( iPosRet != 2 ) {
		xpkClose(objXpk);
		return 122;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 123;
	}
	xpkClose(objXpk);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgRaw64, &objOpt);
	if ( objXpk == NULL ) {
		return 976;
	}
	if ( xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN) != XPK_OK ) {
		xpkClose(objXpk);
		return 977;
	}
	if ( !objXpk->bVolumeApplied || procXpkAppliedVolumeSize(objXpk) != XPK_VOLUME_MIN ) {
		xpkClose(objXpk);
		return 978;
	}
	iBuildSizeBefore = ((uint64_t)XPK_VOLUME_MIN * 65536u) + 123u;
	if ( procXpkRawWrite(objXpk, iBuildSizeBefore, "SEEK64", 6) != XPK_OK ) {
		xpkClose(objXpk);
		return 979;
	}
	memset(arrInfoExtCoreRead, 0, sizeof(arrInfoExtCoreRead));
	if ( procXpkRawRead(objXpk, iBuildSizeBefore, arrInfoExtCoreRead, 6) != XPK_OK ) {
		xpkClose(objXpk);
		return 980;
	}
	if ( memcmp(arrInfoExtCoreRead, "SEEK64", 6) != 0 ) {
		xpkClose(objXpk);
		return 981;
	}
	sPathVolume = procXpkVolumePathDupText(sPathPkgRaw64, 65536u);
	if ( sPathVolume == NULL ) {
		xpkClose(objXpk);
		return 982;
	}
	if ( !xrtFileExists((str)sPathVolume) ) {
		xpkFreeInternal(sPathVolume);
		xpkClose(objXpk);
		return 983;
	}
	xpkFreeInternal(sPathVolume);
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgRaw64);

	sPathVolume = procXpkVolumePathDupText(sPathPkgVolume, 1);
	if ( sPathVolume == NULL ) {
		return 124;
	}
	if ( !xrtFileExists((str)sPathVolume) ) {
		xpkFreeInternal(sPathVolume);
		return 125;
	}
	xpkFreeInternal(sPathVolume);

	hFile = xrtOpen((str)sPathPkgVolume, FALSE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		return 126;
	}
	xrtSeek(hFile, (long)XPK_VOLUME_MIN, XRT_SEEK_SET);
	iCorruptByte = (uint8_t)'Z';
	if ( xrtWrite(hFile, (str)&iCorruptByte, 1) != 1 ) {
		xrtClose(hFile);
		return 127;
	}
	xrtClose(hFile);

	objXpk = xpkOpen(sPathPkgVolume, NULL);
	if ( objXpk == NULL ) {
		return 128;
	}
	if ( xpkGetVolumeSize(objXpk, &iVolumeSize) != XPK_OK ) {
		xpkClose(objXpk);
		return 129;
	}
	if ( iVolumeSize != XPK_VOLUME_MIN ) {
		xpkClose(objXpk);
		return 130;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 131;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 132;
	}
	if ( iDataSize != sizeof(sDataVolumeA) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 133;
	}
	if ( memcmp(pDataRead, sDataVolumeA, sizeof(sDataVolumeA)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 134;
	}
	xpkFree(pDataRead);
	memset(&objBuildOpt, 0, sizeof(objBuildOpt));
	objBuildOpt.tempPath = sPathPkgVolumeTemp;
	objBuildOpt.replaceOriginal = FALSE;
	if ( xpkBuild(objXpk, &objBuildOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 330;
	}
	iVolumeCount = 0;
	if ( procXpkCountVolumeFilesText(objXpk, sPathPkgVolumeTemp, &iVolumeCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 331;
	}
	if ( iVolumeCount != 3 ) {
		xpkClose(objXpk);
		return 332;
	}

	if ( xpkRemove(objXpk, 1) != XPK_OK ) {
		xpkClose(objXpk);
		return 135;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 136;
	}
	if ( xpkStatGet(objXpk, &objStat) != XPK_OK ) {
		xpkClose(objXpk);
		return 137;
	}
	if ( objStat.fileCount != 1 ) {
		xpkClose(objXpk);
		return 138;
	}
	if ( objStat.holeBytes != sizeof(sDataVolumeA) ) {
		xpkClose(objXpk);
		return 139;
	}
	if ( xpkBuild(objXpk, &objBuildOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 333;
	}
	iVolumeCount = 0;
	if ( procXpkCountVolumeFilesText(objXpk, sPathPkgVolumeTemp, &iVolumeCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 334;
	}
	if ( iVolumeCount != 2 ) {
		xpkClose(objXpk);
		return 335;
	}
	sPathVolume = procXpkVolumePathDupText(sPathPkgVolume, 5);
	if ( sPathVolume == NULL ) {
		xpkClose(objXpk);
		return 336;
	}
	if ( !procTestWriteVolumeBinaryFile(sPathPkgVolume, 5, "STALE", 5) ) {
		xpkFreeInternal(sPathVolume);
		xpkClose(objXpk);
		return 337;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkFreeInternal(sPathVolume);
		xpkClose(objXpk);
		return 140;
	}
	if ( xrtFileExists((str)sPathVolume) ) {
		xpkFreeInternal(sPathVolume);
		xpkClose(objXpk);
		return 339;
	}
	xpkFreeInternal(sPathVolume);
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 141;
	}
	if ( xpkStatGet(objXpk, &objStat) != XPK_OK ) {
		xpkClose(objXpk);
		return 142;
	}
	if ( objStat.fileCount != 1 ) {
		xpkClose(objXpk);
		return 143;
	}
	if ( objStat.holeBytes != 0 ) {
		xpkClose(objXpk);
		return 144;
	}
	if ( xpkGetVolumeSize(objXpk, &iVolumeSize) != XPK_OK ) {
		xpkClose(objXpk);
		return 145;
	}
	if ( iVolumeSize != XPK_VOLUME_MIN ) {
		xpkClose(objXpk);
		return 146;
	}
	iVolumeCount = 0;
	if ( procXpkCountVolumeFilesText(objXpk, objXpk->sPathPackage, &iVolumeCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 147;
	}
	if ( iVolumeCount < 2 ) {
		xpkClose(objXpk);
		return 148;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 149;
	}
	if ( iDataSize != sizeof(sDataVolumeB) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 150;
	}
	if ( memcmp(pDataRead, sDataVolumeB, sizeof(sDataVolumeB)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 151;
	}
	xpkFree(pDataRead);
	xpkClose(objXpk);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgReplace, &objOpt);
