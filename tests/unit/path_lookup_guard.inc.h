	objXpk = xpkOpen(sPathPkgPath, NULL);
	if ( objXpk == NULL ) {
		return 59;
	}
	if ( !xpkPathExists(objXpk, "assets/hero_idle.png") ) {
		xpkClose(objXpk);
		return 60;
	}
	if ( !xpkPathExists(objXpk, "ASSETS/HERO_IDLE.PNG") ) {
		xpkClose(objXpk);
		return 241;
	}
	if ( !xpkPathExists(objXpk, ".\\assets\\\\hero_idle.png") ) {
		xpkClose(objXpk);
		return 242;
	}
	pDataRead = xpkPathReadToMemory(objXpk, "assets/hero_idle.png", &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 61;
	}
	if ( iDataSize != sizeof(sDataPath) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 62;
	}
	if ( memcmp(pDataRead, sDataPath, sizeof(sDataPath)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 63;
	}
	if ( xpkPathGetInfo(objXpk, "ASSETS/HERO_IDLE.PNG", &objInfoPath) != XPK_OK ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 243;
	}
	if ( objInfoPath.platformAttr != 0x20u ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 244;
	}
	if ( strcmp((const char*)objInfoPath.pathBytes, "ASSETS/HERO_IDLE.PNG") != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 245;
	}
	iMatchCount = 0;
	if ( xpkEachMatch(objXpk, "*.PNG", NULL, &iMatchCount) != XPK_OK ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 64;
	}
	if ( iMatchCount != 1 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 65;
	}
	sPathKey = procXpkDupPathKey(objXpk, "assets/hero_idle.png");
	if ( sPathKey == NULL ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 710;
	}
	pMapEntry = (xpkEntry*)xrtDictGet(&objXpk->tblEntry, (ptr)sPathKey, (uint32_t)strlen(sPathKey));
	xpkFreeInternal(sPathKey);
	if ( pMapEntry == NULL ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 711;
	}
	iPosSaved = pMapEntry->iPos;
	pMapEntry->iPos = 0;
	if ( xpkPathExists(objXpk, "assets/hero_idle.png") ) {
		pMapEntry->iPos = iPosSaved;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 712;
	}
	iCheckRet = procTestCheckLastError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	if ( iCheckRet != 0 ) {
		pMapEntry->iPos = iPosSaved;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iCheckRet == 1) ? 713 : 714;
	}
	iRetCall = xpkPathAddData(objXpk, "assets/hero_idle.png", sDataPath, sizeof(sDataPath), &objWriteOpt);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		pMapEntry->iPos = iPosSaved;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 727 : 728;
	}
	iRetCall = xpkPathGetInfo(objXpk, "assets/hero_idle.png", &objInfoPath);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		pMapEntry->iPos = iPosSaved;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 715 : 716;
	}
	pMapEntry->iPos = iPosSaved;
	sPathKey = procXpkDupPathKey(objXpk, "assets/hero_idle.png");
	if ( sPathKey == NULL ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 829;
	}
	if ( !xrtDictRemove(&objXpk->tblEntry, (ptr)sPathKey, (uint32_t)strlen(sPathKey)) ) {
		xpkFreeInternal(sPathKey);
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 830;
	}
	xpkFreeInternal(sPathKey);
	if ( xpkPathExists(objXpk, "assets/hero_idle.png") ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 831;
	}
	iCheckRet = procTestCheckLastError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	if ( iCheckRet != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iCheckRet == 1) ? 832 : 833;
	}
	if ( procXpkRebuildLookup(objXpk) != XPK_OK ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 834;
	}
	pEntry = procXpkGetEntryByPos(objXpk, 1);
	if ( pEntry == NULL ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 500;
	}
	sPathSaved = pEntry->sPath;
	pEntry->sPath = NULL;
	iRetCall = xpkEachMatch(objXpk, "*.PNG", NULL, &iMatchCount);
	iCheckRet = procTestCheckCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	if ( iCheckRet != 0 ) {
		pEntry->sPath = sPathSaved;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iCheckRet == 1) ? 501 : ((iCheckRet == 2) ? 502 : 503);
	}
	pEntry->sPath = sPathSaved;
	iCorruptByte = (uint8_t)pEntry->sPath[0];
	pEntry->sPath[0] = '\0';
	if ( xpkCount(objXpk) != 0 ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 690;
	}
	iCheckRet = procTestCheckLastError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	if ( iCheckRet != 0 ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iCheckRet == 1) ? 691 : 692;
	}
	iRetCall = xpkEachMatch(objXpk, "*.PNG", NULL, &iMatchCount);
	iCheckRet = procTestCheckCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	if ( iCheckRet != 0 ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iCheckRet == 1) ? 693 : ((iCheckRet == 2) ? 694 : 695);
	}
	iRetCall = procXpkRebuildLookup(objXpk);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_FORMAT, objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat) ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_FORMAT) ? 760 : 761;
	}
	pEntry->sPath[0] = (char)iCorruptByte;
	if ( procXpkRebuildLookup(objXpk) != XPK_OK ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 762;
	}
	iCorruptByte = (uint8_t)pEntry->sPath[0];
	pEntry->sPath[0] = '\0';
	iRetCall = xpkBuild(objXpk, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 723 : 724;
	}
	if ( xpkPathExists(objXpk, "assets/hero_idle.png") ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 696;
	}
	iCheckRet = procTestCheckLastError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	if ( iCheckRet != 0 ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iCheckRet == 1) ? 697 : 698;
	}
	iRetCall = xpkPathGetInfo(objXpk, "assets/hero_idle.png", &objInfoPath);
	iCheckRet = procTestCheckCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	if ( iCheckRet != 0 ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iCheckRet == 1) ? 699 : ((iCheckRet == 2) ? 700 : 701);
	}
	pMetaRead = xpkPathReadToMemory(objXpk, "assets/hero_idle.png", &iDataSize);
	if ( !procTestExpectNullResultError(pMetaRead, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		if ( pMetaRead != NULL ) {
			pEntry->sPath[0] = (char)iCorruptByte;
			xpkFree(pMetaRead);
			xpkFree(pDataRead);
			xpkClose(objXpk);
			return 702;
		}
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 703;
	}
	pEntry->sPath[0] = (char)iCorruptByte;
	sPathSaved = pEntry->sPath;
	pEntry->sPath = (char*)xpkAllocInternal(XPK_PATH_BYTES + 8);
	if ( pEntry->sPath == NULL ) {
		pEntry->sPath = sPathSaved;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 766;
	}
	memset(pEntry->sPath, 'a', XPK_PATH_BYTES + 7);
	pEntry->sPath[XPK_PATH_BYTES + 7] = '\0';
	if ( xpkCount(objXpk) != 0 ) {
		xpkFreeInternal(pEntry->sPath);
		pEntry->sPath = sPathSaved;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 767;
	}
	iCheckRet = procTestCheckLastError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	if ( iCheckRet != 0 ) {
		xpkFreeInternal(pEntry->sPath);
		pEntry->sPath = sPathSaved;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iCheckRet == 1) ? 768 : 769;
	}
	iRetCall = procXpkRebuildLookup(objXpk);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_FORMAT, objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat) ) {
		xpkFreeInternal(pEntry->sPath);
		pEntry->sPath = sPathSaved;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_FORMAT) ? 770 : 771;
	}
	xpkFreeInternal(pEntry->sPath);
	pEntry->sPath = sPathSaved;
	iRetCall = procXpkRebuildLookup(objXpk);
	if ( iRetCall != XPK_OK ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iRetCall == XPK_ERR_FORMAT) ? 1772 : ((iRetCall == XPK_ERR_STATE) ? 2772 : 3772);
	}
	procXpkClearError(objXpk);
	iCorruptByte = (uint8_t)pEntry->sPath[0];
	pEntry->sPath[0] = 'B';
	if ( xpkPathExists(objXpk, "assets/hero_idle.png") ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 717;
	}
	iCheckRet = procTestCheckLastError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	if ( iCheckRet != 0 ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iCheckRet == 1) ? 718 : 719;
	}
	if ( xpkCount(objXpk) != 0 ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 758;
	}
	if ( !procTestExpectLastError(objXpk, XPK_ERR_STATE, NULL) ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 759;
	}
	iRetCall = xpkVerifyAll(objXpk);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 733 : 734;
	}
	iRetCall = xpkVerify(objXpk, pEntry->iPos);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 741 : 742;
	}
	pEntry->sPath[0] = (char)iCorruptByte;
	objXpk->iEntryCount--;
	if ( xpkPathExists(objXpk, "assets/hero_idle.png") ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 842;
	}
	iCheckRet = procTestCheckLastError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	if ( iCheckRet != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iCheckRet == 1) ? 843 : 844;
	}
	if ( procXpkRebuildLookup(objXpk) != XPK_ERR_STATE ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 879;
	}
	objXpk->iEntryCount++;
	if ( procXpkRebuildLookup(objXpk) != XPK_OK ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 880;
	}
	if ( !xpkPathExists(objXpk, "assets/hero_idle.png") ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1004;
	}
	if ( !procTestExpectLastError(objXpk, XPK_OK, NULL) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 881;
	}
	iCorruptByte = (uint8_t)pEntry->sPath[0];
	pEntry->sPath[0] = 'B';
	iRetCall = xpkGetInfo(objXpk, pEntry->iPos, &objInfo);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 779 : 780;
	}
	pMetaRead = xpkReadToMemory(objXpk, pEntry->iPos, &iDataSize);
	if ( !procTestExpectNullResultError(pMetaRead, objXpk, XPK_ERR_STATE, NULL) ) {
		if ( pMetaRead != NULL ) {
			pEntry->sPath[0] = (char)iCorruptByte;
			xpkFree(pMetaRead);
			xpkFree(pDataRead);
			xpkClose(objXpk);
			return 781;
		}
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 782;
	}
	iRetCall = xpkUpdateData(objXpk, pEntry->iPos, sDataPath, sizeof(sDataPath), &objWriteOpt);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 783 : 784;
	}
	iRetCall = xpkUpdateFile(objXpk, pEntry->iPos, "release/x64/xpack_phase3_missing_update_src.bin", &objWriteOpt);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 787 : 788;
	}
	iRetCall = xpkStatGet(objXpk, &objStat);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 743 : 744;
	}
	iEachCount = 0;
	iRetCall = xpkEach(objXpk, NULL, &iEachCount);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 747 : 748;
	}
	iRetCall = xpkSave(objXpk);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 751 : 752;
	}
	iRetCall = xpkBuild(objXpk, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 735 : 736;
	}
	iRetCall = xpkPathGetInfo(objXpk, "assets/hero_idle.png", &objInfoPath);
	iCheckRet = procTestCheckCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	if ( iCheckRet != 0 ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return (iCheckRet == 1) ? 720 : ((iCheckRet == 2) ? 721 : 722);
	}
	pEntry->sPath[0] = (char)iCorruptByte;
	pEntry->sPath[0] = (char)iCorruptByte;
	xpkFree(pDataRead);
	xpkClose(objXpk);
