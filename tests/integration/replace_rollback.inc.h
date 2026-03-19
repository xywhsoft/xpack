	objXpk = xpkOpen(sPathPkgReplace, &objOpt);
	if ( objXpk == NULL ) {
		return 422;
	}
	if ( xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN) != XPK_OK ) {
		xpkClose(objXpk);
		return 423;
	}

	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, sDataVolumeA, sizeof(sDataVolumeA), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 424;
	}
	if ( iPosRet != 1 ) {
		xpkClose(objXpk);
		return 425;
	}
	if ( xpkAddData(objXpk, sDataVolumeB, sizeof(sDataVolumeB), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 426;
	}
	if ( iPosRet != 2 ) {
		xpkClose(objXpk);
		return 427;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 428;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		xpkClose(objXpk);
		return 789;
	}
	iPosSaved = (objInfo.flag & XPK_FLAG_TYPE_MASK);
	objWriteOpt.compLevel = 16;
	objWriteOpt.fileType = 7;
	if ( xpkUpdateData(objXpk, 1, sDataReplaceNew, sizeof(sDataReplaceNew), &objWriteOpt) != XPK_ERR_PARAM ) {
		xpkClose(objXpk);
		return 790;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		xpkClose(objXpk);
		return 791;
	}
	if ( (objInfo.flag & XPK_FLAG_TYPE_MASK) != iPosSaved ) {
		xpkClose(objXpk);
		return 792;
	}
	objWriteOpt.compLevel = 0;
	objWriteOpt.fileType = 0;
	if ( xpkUpdateData(objXpk, 1, sDataReplaceNew, sizeof(sDataReplaceNew), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 429;
	}

	hFile = xrtOpen((str)sPathPkgReplace, FALSE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		xpkClose(objXpk);
		return 430;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_ERR_IO ) {
		xrtClose(hFile);
		xpkClose(objXpk);
		return 431;
	}
	xrtClose(hFile);
	iVolumeCount = 0;
	if ( procXpkCountVolumeFilesText(objXpk, sPathPkgReplaceBackup, &iVolumeCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 432;
	}
	if ( iVolumeCount != 0 ) {
		xpkClose(objXpk);
		return 433;
	}
	iVolumeCount = 0;
	if ( procXpkCountVolumeFilesText(objXpk, sPathPkgReplaceTemp, &iVolumeCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 442;
	}
	if ( iVolumeCount != 0 ) {
		xpkClose(objXpk);
		return 443;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgReplace, NULL);
	if ( objXpk == NULL ) {
		return 434;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 435;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 436;
	}
	if ( iDataSize != sizeof(sDataVolumeA) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 437;
	}
	if ( memcmp(pDataRead, sDataVolumeA, sizeof(sDataVolumeA)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 438;
	}
	xpkFree(pDataRead);
	pDataRead = xpkReadToMemory(objXpk, 2, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 439;
	}
	if ( iDataSize != sizeof(sDataVolumeB) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 440;
	}
	if ( memcmp(pDataRead, sDataVolumeB, sizeof(sDataVolumeB)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 441;
	}
	xpkFree(pDataRead);
	sPathKey = procXpkVolumePathDupText(sPathPkgReplaceBackup, 1);
	if ( sPathKey == NULL ) {
		xpkClose(objXpk);
		return 943;
	}
	if ( !procTestWriteBinaryFile(sPathKey, "KEEP", 4) ) {
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 944;
	}
	iRetCall = xpkBuild(objXpk, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_EXISTS, objXpk, XPK_ERR_EXISTS, sXpkErrorBackupPathExists) ) {
		procDeleteVolumeFiles(sPathPkgReplaceBackup);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_EXISTS) ? 946 : 947;
	}
	if ( !procTestFileContentEquals(sPathKey, "KEEP", 4) ) {
		procDeleteVolumeFiles(sPathPkgReplaceBackup);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 948;
	}
	procDeleteVolumeFiles(sPathPkgReplaceBackup);
	xpkFreeInternal(sPathKey);
	if ( !xrtDirCreate((str)sPathPkgReplaceBackup) ) {
		xpkClose(objXpk);
		return 470;
	}
	iRetCall = xpkBuild(objXpk, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_EXISTS, objXpk, XPK_ERR_EXISTS, sXpkErrorBackupPathExists) ) {
		xrtDirDelete((str)sPathPkgReplaceBackup);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_EXISTS) ? 471 : 472;
	}
	if ( !xrtDirExists((str)sPathPkgReplaceBackup) ) {
		xrtDirDelete((str)sPathPkgReplaceBackup);
		xpkClose(objXpk);
		return 473;
	}
	xrtDirDelete((str)sPathPkgReplaceBackup);
	xpkClose(objXpk);

	procDeleteVolumeFiles(sPathPkgReplace);
	procDeleteVolumeFiles(sPathPkgReplaceNew);
	procDeleteVolumeFiles(sPathPkgReplaceTemp);
	procDeleteVolumeFiles(sPathPkgReplaceBackup);
	xrtDirDelete((str)sPathPkgReplaceNew);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgReplace, &objOpt);
	if ( objXpk == NULL ) {
		return 444;
	}
	if ( xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN) != XPK_OK ) {
		xpkClose(objXpk);
		return 445;
	}

	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, sDataVolumeA, sizeof(sDataVolumeA), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 446;
	}
	if ( xpkAddData(objXpk, sDataVolumeB, sizeof(sDataVolumeB), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 447;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 448;
	}

	sPathVolume = procXpkVolumePathDupText(sPathPkgReplace, 2);
	if ( sPathVolume == NULL ) {
		xpkClose(objXpk);
		return 449;
	}
	if ( !xrtFileExists((str)sPathVolume) ) {
		xpkFreeInternal(sPathVolume);
		xpkClose(objXpk);
		return 450;
	}
	hFile = xrtOpen((str)sPathVolume, FALSE, XRT_CP_BINARY);
	xpkFreeInternal(sPathVolume);
	if ( hFile == NULL ) {
		xpkClose(objXpk);
		return 451;
	}
	iRetCall = xpkBuild(objXpk, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_IO, objXpk, XPK_ERR_IO, "failed to replace package and rollback") ) {
		xrtClose(hFile);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_IO) ? 452 : 453;
	}
	xrtClose(hFile);
	iVolumeCount = 0;
	if ( procXpkCountVolumeFilesText(objXpk, sPathPkgReplaceBackup, &iVolumeCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 454;
	}
	if ( iVolumeCount != 3 ) {
		xpkClose(objXpk);
		return 455;
	}
	sPathKey = procXpkVolumePathDupText(sPathPkgReplaceTemp, 2);
	if ( sPathKey == NULL ) {
		xpkClose(objXpk);
		return 456;
	}
	if ( xrtFileExists((str)sPathKey) ) {
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 457;
	}
	xpkFreeInternal(sPathKey);
	xpkClose(objXpk);
	procDeleteVolumeFiles(sPathPkgReplace);
	procDeleteVolumeFiles(sPathPkgReplaceTemp);
	procDeleteVolumeFiles(sPathPkgReplaceBackup);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgReplaceNew, &objOpt);
	if ( objXpk == NULL ) {
		return 458;
	}
	if ( xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN) != XPK_OK ) {
		xpkClose(objXpk);
		return 459;
	}

	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkAddData(objXpk, sDataVolumeA, sizeof(sDataVolumeA), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 460;
	}
	if ( xpkAddData(objXpk, sDataVolumeB, sizeof(sDataVolumeB), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 461;
	}
	if ( !xrtDirCreate((str)sPathPkgReplaceNew) ) {
		xpkClose(objXpk);
		return 462;
	}
	iRetCall = xpkBuild(objXpk, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_IO, objXpk, XPK_ERR_IO, "failed to replace package and cleanup") ) {
		xrtDirDelete((str)sPathPkgReplaceNew);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_IO) ? 463 : 464;
	}
	sPathVolume = procXpkVolumePathDupText(sPathPkgReplaceNew, 1);
	if ( sPathVolume == NULL ) {
		xrtDirDelete((str)sPathPkgReplaceNew);
		xpkClose(objXpk);
		return 465;
	}
	if ( xrtFileExists((str)sPathVolume) ) {
		xpkFreeInternal(sPathVolume);
		xrtDirDelete((str)sPathPkgReplaceNew);
		xpkClose(objXpk);
		return 466;
	}
	xpkFreeInternal(sPathVolume);
	sPathVolume = procXpkVolumePathDupText(sPathPkgReplaceNew, 2);
	if ( sPathVolume == NULL ) {
		xrtDirDelete((str)sPathPkgReplaceNew);
		xpkClose(objXpk);
		return 467;
	}
	if ( xrtFileExists((str)sPathVolume) ) {
		xpkFreeInternal(sPathVolume);
		xrtDirDelete((str)sPathPkgReplaceNew);
		xpkClose(objXpk);
		return 468;
	}
	xpkFreeInternal(sPathVolume);
	if ( !xrtDirExists((str)sPathPkgReplaceNew) ) {
		xrtDirDelete((str)sPathPkgReplaceNew);
		xpkClose(objXpk);
		return 469;
	}
	xrtDirDelete((str)sPathPkgReplaceNew);
	procDeleteVolumeFiles(sPathPkgReplaceNew);
	xpkClose(objXpk);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgBuild, &objOpt);
	if ( objXpk == NULL ) {
		return 476;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, sDataBuildA, sizeof(sDataBuildA), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 477;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 478;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		xpkClose(objXpk);
		return 479;
	}
	objBuildOpt.tempPath = sPathPkgBuildFailTemp;
	objBuildOpt.replaceOriginal = FALSE;
	objXpk->objHead.packType = XPK_PACK_WIN32;
	if ( xpkBuild(objXpk, &objBuildOpt) != XPK_ERR_FORMAT ) {
		xpkClose(objXpk);
		return 480;
	}
	objXpk->objHead.packType = XPK_PACK_CORE;
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		xpkClose(objXpk);
		return 481;
	}
	iVolumeCount = 0;
	if ( procXpkCountVolumeFilesText(objXpk, sPathPkgBuildFailTemp, &iVolumeCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 482;
	}
	if ( iVolumeCount != 0 ) {
		xpkClose(objXpk);
		return 483;
	}
	xpkClose(objXpk);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgSaveFail, &objOpt);
	if ( objXpk == NULL ) {
		return 452;
	}
	if ( xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN) != XPK_OK ) {
		xpkClose(objXpk);
		return 453;
	}

	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, sDataSaveFailBase, sizeof(sDataSaveFailBase), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 454;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 455;
	}

	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkAddData(objXpk, sDataSaveFailNew, sizeof(sDataSaveFailNew), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 456;
	}
	if ( iPosRet != 2 ) {
		xpkClose(objXpk);
		return 457;
	}
	if ( xpkGetInfo(objXpk, 2, &objInfo) != XPK_OK ) {
		xpkClose(objXpk);
		return 458;
	}
	if ( objInfo.dataOffset != 0 ) {
		xpkClose(objXpk);
		return 459;
	}
	sPathKey = procXpkVolumePathDupText(sPathPkgSaveFail, 5);
	if ( sPathKey == NULL ) {
		xpkClose(objXpk);
		return 951;
	}
	hFile = xrtOpen((str)sPathKey, FALSE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 952;
	}
	if ( xrtWrite(hFile, (str)"KEEP", 4) != 4 ) {
		xrtClose(hFile);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 953;
	}
	xrtClose(hFile);

	sPathVolume = procXpkVolumePathDupText(sPathPkgSaveFail, 1);
	if ( sPathVolume == NULL ) {
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 460;
	}
	hFile = xrtOpen((str)sPathVolume, FALSE, XRT_CP_BINARY);
	xpkFreeInternal(sPathVolume);
	if ( hFile == NULL ) {
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 461;
	}
	if ( xpkSave(objXpk) != XPK_ERR_IO ) {
		xrtClose(hFile);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 462;
	}
	objCheck = xpkOpen(sPathPkgSaveFail, NULL);
	if ( objCheck == NULL ) {
		xrtClose(hFile);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 507;
	}
	if ( xpkVerifyAll(objCheck) != XPK_OK ) {
		xpkClose(objCheck);
		xrtClose(hFile);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 508;
	}
	pDataRead = xpkReadToMemory(objCheck, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objCheck);
		xrtClose(hFile);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 509;
	}
	if ( iDataSize != sizeof(sDataSaveFailBase) ) {
		xpkFree(pDataRead);
		xpkClose(objCheck);
		xrtClose(hFile);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 510;
	}
	if ( memcmp(pDataRead, sDataSaveFailBase, sizeof(sDataSaveFailBase)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objCheck);
		xrtClose(hFile);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 511;
	}
	xpkFree(pDataRead);
	if ( xpkGetInfo(objCheck, 2, &objInfo) != XPK_ERR_NOT_FOUND ) {
		xpkClose(objCheck);
		xrtClose(hFile);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 512;
	}
	xpkClose(objCheck);
	hFileTmp = xrtOpen((str)sPathKey, TRUE, XRT_CP_BINARY);
	if ( hFileTmp == NULL ) {
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 954;
	}
	pDataRead = xrtRead(hFileTmp, 4, &iReadSize);
	xrtClose(hFileTmp);
	if ( pDataRead == NULL || iReadSize != 4 ) {
		if ( pDataRead != NULL ) {
			xrtFree(pDataRead);
		}
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 955;
	}
	if ( memcmp(pDataRead, "KEEP", 4) != 0 ) {
		xrtFree(pDataRead);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 956;
	}
	xrtFree(pDataRead);
	if ( xpkGetInfo(objXpk, 2, &objInfo) != XPK_OK ) {
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 463;
	}
	if ( objInfo.dataOffset != 0 ) {
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 464;
	}
	pDataRead = xpkReadToMemory(objXpk, 2, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 465;
	}
	if ( iDataSize != sizeof(sDataSaveFailNew) ) {
		xpkFree(pDataRead);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 466;
	}
	if ( memcmp(pDataRead, sDataSaveFailNew, sizeof(sDataSaveFailNew)) != 0 ) {
		xpkFree(pDataRead);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 467;
	}
	xpkFree(pDataRead);
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 468;
	}
	if ( xpkGetInfo(objXpk, 2, &objInfo) != XPK_OK ) {
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 469;
	}
	if ( objInfo.dataOffset == 0 ) {
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 470;
	}
	xpkFreeInternal(sPathKey);
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgSaveFail, NULL);
	if ( objXpk == NULL ) {
		return 471;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 472;
	}
	pDataRead = xpkReadToMemory(objXpk, 2, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 473;
	}
	if ( iDataSize != sizeof(sDataSaveFailNew) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 474;
	}
	if ( memcmp(pDataRead, sDataSaveFailNew, sizeof(sDataSaveFailNew)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 475;
	}
	xpkFree(pDataRead);
	xpkClose(objXpk);
