	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgPath, &objOpt);
	if ( objXpk == NULL ) {
		return 45;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_WIN32) != XPK_OK ) {
		xpkClose(objXpk);
		return 46;
	}
	iRetCall = xpkAddData(objXpk, sDataPath, sizeof(sDataPath), NULL, &iPosRet);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 797 : 798;
	}
	iRetCall = xpkAddFile(objXpk, sPathPkgCore, NULL, &iPosRet);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 799 : 800;
	}
	iRetCall = xpkPathAddData(objXpk, "./", sDataPath, sizeof(sDataPath), NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorPathEmpty) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 350 : 351;
	}
	iRetCall = xpkPathAddData(objXpk, "", sDataPath, sizeof(sDataPath), NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorPathEmpty) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 444 : 445;
	}
	iRetCall = xpkPathAddData(objXpk, sPathTooLong, sDataPath, sizeof(sDataPath), NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorPathTooLong) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 336 : 337;
	}
	iCheckRet = procTestCheckFalseError(xpkPathExists(objXpk, "assets/missing.txt"), objXpk, XPK_OK, NULL);
	if ( iCheckRet != 0 ) {
		xpkClose(objXpk);
		return (iCheckRet == 2) ? 492 : 491;
	}
	iRetCall = xpkPathGetInfo(objXpk, "assets/missing.txt", &objInfoPath);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_NOT_FOUND, objXpk, XPK_ERR_NOT_FOUND, NULL) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_NOT_FOUND) ? 493 : 494;
	}
	pDataRead = xpkPathReadToMemory(objXpk, "assets/missing.txt", &iDataSize);
	if ( !procTestExpectNullResultError(pDataRead, objXpk, XPK_ERR_NOT_FOUND, NULL) ) {
		if ( pDataRead != NULL ) {
			xpkFree(pDataRead);
			xpkClose(objXpk);
			return 495;
		}
		xpkClose(objXpk);
		return 496;
	}

	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkPathAddData(objXpk, "assets/hero.png", sDataPath, sizeof(sDataPath), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 47;
	}
	if ( !xpkPathExists(objXpk, "assets/hero.png") ) {
		xpkClose(objXpk);
		return 48;
	}
	iRetCall = xpkPathRename(objXpk, "assets/hero.png", "");
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorPathEmpty) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 446 : 447;
	}
	if ( !xpkPathExists(objXpk, "assets/hero.png") ) {
		xpkClose(objXpk);
		return 448;
	}
	if ( xpkPathRename(objXpk, "assets/hero.png", "assets/hero_idle.png") != XPK_OK ) {
		xpkClose(objXpk);
		return 49;
	}
	if ( xpkPathSetAttr(objXpk, "assets/hero_idle.png", 0x20u) != XPK_OK ) {
		xpkClose(objXpk);
		return 50;
	}
	pDataRead = xpkPathReadToMemory(objXpk, "assets/hero_idle.png", &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 51;
	}
	if ( iDataSize != sizeof(sDataPath) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 52;
	}
	if ( memcmp(pDataRead, sDataPath, sizeof(sDataPath)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 53;
	}
	xpkFree(pDataRead);
	if ( !xpkPathExists(objXpk, "ASSETS/HERO_IDLE.PNG") ) {
		xpkClose(objXpk);
		return 230;
	}
	if ( xpkPathExists(objXpk, "./") ) {
		xpkClose(objXpk);
		return 381;
	}
	iCheckRet = procTestCheckLastError(objXpk, XPK_ERR_PARAM, sXpkErrorPathEmpty);
	if ( iCheckRet != 0 ) {
		xpkClose(objXpk);
		return (iCheckRet == 1) ? 382 : 383;
	}
	iRetCall = xpkPathGetInfo(objXpk, "./", &objInfoPath);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorPathEmpty) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 384 : 385;
	}
	pDataRead = xpkPathReadToMemory(objXpk, NULL, &iDataSize);
	iCheckRet = procTestCheckNullResultError(pDataRead, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	if ( iCheckRet != 0 ) {
		if ( pDataRead != NULL ) {
			xpkClose(objXpk);
			return 294;
		}
		xpkClose(objXpk);
		return (iCheckRet == 2) ? 295 : 296;
	}
	pDataRead = xpkPathReadToMemory(objXpk, "./", &iDataSize);
	iCheckRet = procTestCheckNullResultError(pDataRead, objXpk, XPK_ERR_PARAM, sXpkErrorPathEmpty);
	if ( iCheckRet != 0 ) {
		if ( pDataRead != NULL ) {
			xpkClose(objXpk);
			return 386;
		}
		xpkClose(objXpk);
		return (iCheckRet == 2) ? 387 : 388;
	}
	if ( !xpkPathExists(objXpk, ".\\ASSETS\\\\HERO_IDLE.PNG") ) {
		xpkClose(objXpk);
		return 231;
	}
	pDataRead = xpkPathReadToMemory(objXpk, ".\\ASSETS\\\\HERO_IDLE.PNG", &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 232;
	}
	if ( iDataSize != sizeof(sDataPath) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 233;
	}
	if ( memcmp(pDataRead, sDataPath, sizeof(sDataPath)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 234;
	}
	xpkFree(pDataRead);
	if ( xpkPathGetInfo(objXpk, ".\\ASSETS\\\\HERO_IDLE.PNG", &objInfoPath) != XPK_OK ) {
		xpkClose(objXpk);
		return 235;
	}
	if ( objInfoPath.platformAttr != 0x20u ) {
		xpkClose(objXpk);
		return 236;
	}
	if ( xpkPathRename(objXpk, ".\\ASSETS\\\\HERO_IDLE.PNG", ".\\ASSETS\\\\HERO_IDLE.PNG") != XPK_OK ) {
		xpkClose(objXpk);
		return 237;
	}
	if ( xpkPathGetInfo(objXpk, "assets/hero_idle.png", &objInfoPath) != XPK_OK ) {
		xpkClose(objXpk);
		return 238;
	}
	if ( strcmp((const char*)objInfoPath.pathBytes, "ASSETS/HERO_IDLE.PNG") != 0 ) {
		xpkClose(objXpk);
		return 239;
	}
	if ( xpkPathAddData(objXpk, ".\\ASSETS\\\\HERO_IDLE.PNG", sDataPath, sizeof(sDataPath), &objWriteOpt) != XPK_ERR_EXISTS ) {
		xpkClose(objXpk);
		return 240;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 54;
	}

	{
		const char* sPathPkgPathCodecBuffered;
		xpkObject objXpkCodec;

		sPathPkgPathCodecBuffered = "release/x64/xpack_phase3_path_codec_buffered.xpk";
		procTestDeletePathFamily(sPathPkgPathCodecBuffered);
		memset(&objOpt, 0, sizeof(objOpt));
		objOpt.createIfMissing = TRUE;
		objXpkCodec = xpkOpen(sPathPkgPathCodecBuffered, &objOpt);
		if ( objXpkCodec == NULL ) {
			xpkClose(objXpk);
			return 2145;
		}
		if ( xpkSetPackType(objXpkCodec, XPK_PACK_WIN32) != XPK_OK ) {
			xpkClose(objXpkCodec);
			xpkClose(objXpk);
			return 2146;
		}
		memset(&objWriteOpt, 0, sizeof(objWriteOpt));
		objWriteOpt.compLevel = 2;
		objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
		if ( xpkPathAddData(objXpkCodec, "assets/codec_buffered.bin", sDataBuildA, sizeof(sDataBuildA), &objWriteOpt) != XPK_OK ) {
			xpkClose(objXpkCodec);
			xpkClose(objXpk);
			return 2147;
		}
		if ( xpkSave(objXpkCodec) != XPK_OK ) {
			xpkClose(objXpkCodec);
			xpkClose(objXpk);
			return 2148;
		}
		xpkClose(objXpkCodec);

		objXpkCodec = xpkOpen(sPathPkgPathCodecBuffered, NULL);
		if ( objXpkCodec == NULL ) {
			xpkClose(objXpk);
			return 2149;
		}
		if ( xpkVerifyAll(objXpkCodec) != XPK_OK ) {
			xpkClose(objXpkCodec);
			xpkClose(objXpk);
			return 2150;
		}
		procTestDeletePathFamily(sPathFileExport);
		if ( xpkPathReadToFile(objXpkCodec, "assets/codec_buffered.bin", sPathFileExport) != XPK_OK ) {
			xpkClose(objXpkCodec);
			xpkClose(objXpk);
			return 2151;
		}
		if ( !procTestFileContentEquals(sPathFileExport, sDataBuildA, sizeof(sDataBuildA)) ) {
			xpkClose(objXpkCodec);
			xpkClose(objXpk);
			return 2152;
		}
		xpkClose(objXpkCodec);
		procTestDeletePathFamily(sPathPkgPathCodecBuffered);
	}

	iEachCount = 0;
	if ( xpkEach(objXpk, NULL, &iEachCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 55;
	}
	if ( iEachCount != 1 ) {
		xpkClose(objXpk);
		return 56;
	}
	iMatchCount = 0;
	if ( xpkEachMatch(objXpk, "assets/*.png", procEachCountTxt, &iMatchCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 57;
	}
	if ( iMatchCount != 1 ) {
		xpkClose(objXpk);
		return 58;
	}
	iRetCall = xpkEach(objXpk, procEachReturnObjectError, NULL);
	iCheckRet = procTestCheckCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	if ( iCheckRet != 0 ) {
		xpkClose(objXpk);
		return (iCheckRet == 1) ? 497 : ((iCheckRet == 2) ? 498 : 499);
	}
	if ( xpkGetInfo(objXpk, 0, &objInfo) != XPK_ERR_PARAM ) {
		xpkClose(objXpk);
		return 504;
	}
	if ( xpkEach(objXpk, procEachReturnCustomError, NULL) != 123 ) {
		xpkClose(objXpk);
		return 505;
	}
	if ( !procTestExpectLastError(objXpk, XPK_OK, NULL) ) {
		xpkClose(objXpk);
		return 506;
	}
	xpkClose(objXpk);

	procTestDeletePathFamily(sPathPkgTest);
	iDataLargeSize = (128u * 1024u) + 211u;
	pDataLarge = xpkAllocInternal(iDataLargeSize);
	if ( pDataLarge == NULL ) {
		return 1084;
	}
	memset(pDataLarge, 'P', iDataLargeSize);
	if ( !procTestWriteBinaryFile(sPathFileExport, pDataLarge, (size_t)iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1085;
	}

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgTest, &objOpt);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1086;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_WIN32) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1087;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkPathAddFile(objXpk, "assets/from_file.bin", sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1088;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1089;
	}
	pDataRead = xpkPathReadToMemory(objXpk, "ASSETS/FROM_FILE.BIN", &iDataSize);
	if ( pDataRead == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1090;
	}
	if ( (iDataSize != iDataLargeSize) || (memcmp(pDataRead, pDataLarge, (size_t)iDataLargeSize) != 0) ) {
		xpkFree(pDataRead);
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1091;
	}
	xpkFree(pDataRead);

	memset(pDataLarge, 'Q', iDataLargeSize);
	if ( !procTestWriteBinaryFile(sPathFileExport, pDataLarge, (size_t)iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1092;
	}
	if ( xpkPathUpdateFile(objXpk, ".\\ASSETS\\\\FROM_FILE.BIN", sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1093;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1094;
	}
	pDataRead = xpkPathReadToMemory(objXpk, "assets/from_file.bin", &iDataSize);
	if ( pDataRead == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1095;
	}
	if ( (iDataSize != iDataLargeSize) || (memcmp(pDataRead, pDataLarge, (size_t)iDataLargeSize) != 0) ) {
		xpkFree(pDataRead);
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1096;
	}
	xpkFree(pDataRead);
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1097;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgTest, NULL);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1098;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1099;
	}
	pDataRead = xpkPathReadToMemory(objXpk, "ASSETS/FROM_FILE.BIN", &iDataSize);
	if ( pDataRead == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1100;
	}
	if ( (iDataSize != iDataLargeSize) || (memcmp(pDataRead, pDataLarge, (size_t)iDataLargeSize) != 0) ) {
		xpkFree(pDataRead);
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1101;
	}
	xpkFree(pDataRead);
	xpkFreeInternal(pDataLarge);
	pDataLarge = NULL;
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgTest);

	procTestDeletePathFamily(sPathPkgTest);
	iDataLargeSize = (80u * 1024u) + 177u;
	pDataLarge = xpkAllocInternal(iDataLargeSize);
	if ( pDataLarge == NULL ) {
		return 1912;
	}
	memset(pDataLarge, 'X', iDataLargeSize);
	if ( !procTestWriteBinaryFile(sPathFileExport, pDataLarge, (size_t)iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1913;
	}

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgTest, &objOpt);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1914;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_WIN32) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1915;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 2;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkPathAddFile(objXpk, "assets/from_file_lz4.bin", sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1916;
	}

	memset(pDataLarge, 'Y', iDataLargeSize);
	if ( !procTestWriteBinaryFile(sPathFileExport, pDataLarge, (size_t)iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1917;
	}
	if ( xpkPathUpdateFile(objXpk, ".\\ASSETS\\\\FROM_FILE_LZ4.BIN", sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1918;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkPathReadToFile(objXpk, "assets/from_file_lz4.bin", sPathFileExport) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1919;
	}
	if ( !procTestFileContentEquals(sPathFileExport, pDataLarge, iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1920;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1921;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgTest, NULL);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1922;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1923;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkPathReadToFile(objXpk, "ASSETS/FROM_FILE_LZ4.BIN", sPathFileExport) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1924;
	}
	if ( !procTestFileContentEquals(sPathFileExport, pDataLarge, iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1925;
	}
	xpkFreeInternal(pDataLarge);
	pDataLarge = NULL;
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgTest);

	procTestDeletePathFamily(sPathPkgTest);
	procTestDeletePathFamily(sPathFileExport);
	if ( !procTestWriteBinaryFile(sPathFileExport, NULL, 0) ) {
		return 1493;
	}
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgTest, &objOpt);
	if ( objXpk == NULL ) {
		return 1494;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_WIN32) != XPK_OK ) {
		xpkClose(objXpk);
		return 1495;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkPathAddFile(objXpk, "assets/empty.bin", sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 1496;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1497;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgTest, NULL);
	if ( objXpk == NULL ) {
		return 1498;
	}
	if ( xpkPathGetInfo(objXpk, "ASSETS/EMPTY.BIN", &objInfoPath) != XPK_OK ) {
		xpkClose(objXpk);
		return 1499;
	}
	if ( (objInfoPath.fileHash != 0) || (objInfoPath.fileSize != 0) || (objInfoPath.dataSize != 0) ) {
		xpkClose(objXpk);
		return 1500;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1501;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkPathReadToFile(objXpk, "assets/empty.bin", sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1502;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, 0) ) {
		xpkClose(objXpk);
		return 1503;
	}
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgTest);

	procTestDeletePathFamily(sPathPkgTest);
	procTestDeletePathFamily(sPathFileExport);
	if ( !procTestWriteBinaryFile(sPathFileExport, "PATH-ZERO-UPDATE", sizeof("PATH-ZERO-UPDATE")) ) {
		return 1553;
	}
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgTest, &objOpt);
	if ( objXpk == NULL ) {
		return 1554;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_WIN32) != XPK_OK ) {
		xpkClose(objXpk);
		return 1555;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkPathAddFile(objXpk, "assets/empty_update.bin", sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 1556;
	}
	if ( !procTestWriteBinaryFile(sPathFileExport, NULL, 0) ) {
		xpkClose(objXpk);
		return 1557;
	}
	if ( xpkPathUpdateFile(objXpk, "ASSETS\\EMPTY_UPDATE.BIN", sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 1558;
	}
	if ( xpkPathGetInfo(objXpk, "assets/empty_update.bin", &objInfoPath) != XPK_OK ) {
		xpkClose(objXpk);
		return 1559;
	}
	if ( (objInfoPath.fileHash != 0) || (objInfoPath.fileSize != 0) || (objInfoPath.dataSize != 0) ) {
		xpkClose(objXpk);
		return 1560;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1561;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgTest, NULL);
	if ( objXpk == NULL ) {
		return 1562;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1563;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkPathReadToFile(objXpk, ".\\assets\\\\empty_update.bin", sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1564;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, 0) ) {
		xpkClose(objXpk);
		return 1565;
	}
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgTest);

	if ( bRunLargeIo ) {
		/* opt-in 4GB+ large-io regression: keep unit fast by default */
	procTestDeletePathFamily(sPathPkgTest);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgTest, &objOpt);
	if ( objXpk == NULL ) {
		return 1146;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_WIN32) != XPK_OK ) {
		xpkClose(objXpk);
		return 1147;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkPathAddData(objXpk, "assets/keep_huge.bin", "KEEP-PATH", sizeof("KEEP-PATH"), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 1148;
	}
	if ( !procTestCreateSparseFileSize(sPathFileHuge, (uint64_t)UINT32_MAX + 55555u) ) {
		xpkClose(objXpk);
		return 1149;
	}
	if ( xpkPathUpdateFile(objXpk, ".\\ASSETS\\\\KEEP_HUGE.BIN", sPathFileHuge, &objWriteOpt) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1150;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1151;
	}
	if ( objInfo.fileSize != ((uint64_t)UINT32_MAX + 55555u) || objInfo.dataSize != objInfo.fileSize || (objInfo.flag & XPK_FLAG_COMP_MASK) != 0 ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1152;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1153;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkPathReadToFile(objXpk, "assets/keep_huge.bin", sPathFileExport) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1154;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, (uint64_t)UINT32_MAX + 55555u) ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1155;
	}
	procTestDeletePathFamily(sPathFileExport);
	remove(sPathFileHuge);
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgTest);

	procTestDeletePathFamily(sPathPkgTest);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgTest, &objOpt);
	if ( objXpk == NULL ) {
		return 1161;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_WIN32) != XPK_OK ) {
		xpkClose(objXpk);
		return 1162;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( !procTestCreateSparseFileSize(sPathFileHuge, (uint64_t)UINT32_MAX + 99999u) ) {
		xpkClose(objXpk);
		return 1163;
	}
	procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
	if ( !procTestCreateDirOccupy("release/x64/xpack_phase3_store_huge_pkg_dir") ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1164;
	}
	sPathSaved = objXpk->sPathPackage;
	objXpk->sPathPackage = procXpkDupText("release/x64/xpack_phase3_store_huge_pkg_dir");
	if ( objXpk->sPathPackage == NULL ) {
		objXpk->sPathPackage = sPathSaved;
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1165;
	}
	iRetCall = xpkPathAddFile(objXpk, "assets/io_huge.bin", sPathFileHuge, &objWriteOpt);
	xpkFreeInternal(objXpk->sPathPackage);
	objXpk->sPathPackage = sPathSaved;
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_IO, objXpk, XPK_ERR_IO, sXpkErrorIoOpen) ) {
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_IO) ? 1166 : 1167;
	}
	if ( xpkCount(objXpk) != 0 ) {
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1168;
	}
	procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
	remove(sPathFileHuge);
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgTest);

	procTestDeletePathFamily(sPathPkgTest);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgTest, &objOpt);
	if ( objXpk == NULL ) {
		return 1169;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_WIN32) != XPK_OK ) {
		xpkClose(objXpk);
		return 1170;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkPathAddData(objXpk, "assets/keep_io.bin", "KEEP-PATH-IO", sizeof("KEEP-PATH-IO"), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 1171;
	}
	if ( !procTestCreateSparseFileSize(sPathFileHuge, (uint64_t)UINT32_MAX + 121212u) ) {
		xpkClose(objXpk);
		return 1172;
	}
	procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
	if ( !procTestCreateDirOccupy("release/x64/xpack_phase3_store_huge_pkg_dir") ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1173;
	}
	sPathSaved = objXpk->sPathPackage;
	objXpk->sPathPackage = procXpkDupText("release/x64/xpack_phase3_store_huge_pkg_dir");
	if ( objXpk->sPathPackage == NULL ) {
		objXpk->sPathPackage = sPathSaved;
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1174;
	}
	iRetCall = xpkPathUpdateFile(objXpk, "assets/keep_io.bin", sPathFileHuge, &objWriteOpt);
	xpkFreeInternal(objXpk->sPathPackage);
	objXpk->sPathPackage = sPathSaved;
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_IO, objXpk, XPK_ERR_IO, sXpkErrorIoOpen) ) {
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_IO) ? 1175 : 1176;
	}
	pDataRead = xpkPathReadToMemory(objXpk, "assets/keep_io.bin", &iDataSize);
	if ( pDataRead == NULL ) {
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1177;
	}
	if ( (iDataSize != sizeof("KEEP-PATH-IO")) || (memcmp(pDataRead, "KEEP-PATH-IO", sizeof("KEEP-PATH-IO")) != 0) ) {
		xpkFree(pDataRead);
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1178;
	}
	xpkFree(pDataRead);
	procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
	remove(sPathFileHuge);
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgTest);

	procTestDeletePathFamily(sPathPkgTest);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgTest, &objOpt);
	if ( objXpk == NULL ) {
		return 1155;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_WIN32) != XPK_OK ) {
		xpkClose(objXpk);
		return 1156;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( !procTestCreateSparseFileSize(sPathFileHuge, (uint64_t)UINT32_MAX + 77777u) ) {
		xpkClose(objXpk);
		return 1156;
	}
	if ( xpkPathAddFile(objXpk, "assets/too_big.bin", sPathFileHuge, &objWriteOpt) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1157;
	}
	if ( xpkCount(objXpk) != 1 ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1158;
	}
	if ( xpkPathGetInfo(objXpk, "ASSETS/TOO_BIG.BIN", &objInfoPath) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1159;
	}
	if ( objInfoPath.fileSize != ((uint64_t)UINT32_MAX + 77777u) || objInfoPath.dataSize != objInfoPath.fileSize || (objInfoPath.flag & XPK_FLAG_COMP_MASK) != 0 ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1160;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1161;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkPathReadToFile(objXpk, "assets/too_big.bin", sPathFileExport) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1162;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, (uint64_t)UINT32_MAX + 77777u) ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1163;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1467;
	}
	xpkClose(objXpk);
	objXpk = xpkOpen(sPathPkgTest, NULL);
	if ( objXpk == NULL ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		return 1468;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1469;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkPathReadToFile(objXpk, "ASSETS/TOO_BIG.BIN", sPathFileExport) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1470;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, (uint64_t)UINT32_MAX + 77777u) ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1471;
	}
	procTestDeletePathFamily(sPathFileExport);
	remove(sPathFileHuge);
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgTest);
	}

	procTestDeletePathFamily(sPathPkgTest);
	iDataLargeSize = (96u * 1024u) + 89u;
	pDataLarge = xpkAllocInternal(iDataLargeSize);
	if ( pDataLarge == NULL ) {
		return 1131;
	}
	memset(pDataLarge, 'V', iDataLargeSize);
	if ( !procTestWriteBinaryFile(sPathFileExport, pDataLarge, (size_t)iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1132;
	}

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgTest, &objOpt);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1133;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_WIN32) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1134;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkPathAddFile(objXpk, "assets/from_file_z.bin", sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1135;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1136;
	}
	pDataRead = xpkPathReadToMemory(objXpk, "ASSETS/FROM_FILE_Z.BIN", &iDataSize);
	if ( pDataRead == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1137;
	}
	if ( (iDataSize != iDataLargeSize) || (memcmp(pDataRead, pDataLarge, (size_t)iDataLargeSize) != 0) ) {
		xpkFree(pDataRead);
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1138;
	}
	xpkFree(pDataRead);

	memset(pDataLarge, 'W', iDataLargeSize);
	if ( !procTestWriteBinaryFile(sPathFileExport, pDataLarge, (size_t)iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1139;
	}
	if ( xpkPathUpdateFile(objXpk, ".\\ASSETS\\\\FROM_FILE_Z.BIN", sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1140;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkPathReadToFile(objXpk, "assets/from_file_z.bin", sPathFileExport) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1639;
	}
	if ( !procTestFileContentEquals(sPathFileExport, pDataLarge, iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1640;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1141;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgTest, NULL);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1142;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1143;
	}
	pDataRead = xpkPathReadToMemory(objXpk, "assets/from_file_z.bin", &iDataSize);
	if ( pDataRead == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1144;
	}
	if ( (iDataSize != iDataLargeSize) || (memcmp(pDataRead, pDataLarge, (size_t)iDataLargeSize) != 0) ) {
		xpkFree(pDataRead);
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1145;
	}
	xpkFree(pDataRead);
	xpkFreeInternal(pDataLarge);
	pDataLarge = NULL;
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgTest);
