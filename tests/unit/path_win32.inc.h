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
