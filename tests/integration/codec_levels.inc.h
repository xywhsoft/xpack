	objXpk = xpkOpen(sPathPkgCodec, &objOpt);
	if ( objXpk == NULL ) {
		return 97;
	}

	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 2;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, sDataBuildA, sizeof(sDataBuildA), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 98;
	}
	if ( iPosRet != 1 ) {
		xpkClose(objXpk);
		return 99;
	}

	objWriteOpt.compLevel = 14;
	if ( xpkAddData(objXpk, sDataBuildB, sizeof(sDataBuildB), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 100;
	}
	if ( iPosRet != 2 ) {
		xpkClose(objXpk);
		return 101;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 102;
	}
	iMatchCount = 0;
	if ( xpkEachMatch(objXpk, "?", NULL, &iMatchCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 103;
	}
	if ( iMatchCount != 2 ) {
		xpkClose(objXpk);
		return 104;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgCodec, NULL);
	if ( objXpk == NULL ) {
		return 105;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 106;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		xpkClose(objXpk);
		return 107;
	}
	if ( (objInfo.flag & XPK_FLAG_COMP_MASK) != 2 ) {
		xpkClose(objXpk);
		return 108;
	}
	if ( xpkGetInfo(objXpk, 2, &objInfo) != XPK_OK ) {
		xpkClose(objXpk);
		return 109;
	}
	if ( (objInfo.flag & XPK_FLAG_COMP_MASK) != 14 ) {
		xpkClose(objXpk);
		return 110;
	}

	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 111;
	}
	if ( iDataSize != sizeof(sDataBuildA) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 112;
	}
	if ( memcmp(pDataRead, sDataBuildA, sizeof(sDataBuildA)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 113;
	}
	xpkFree(pDataRead);

	pDataRead = xpkReadToMemory(objXpk, 2, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 114;
	}
	if ( iDataSize != sizeof(sDataBuildB) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 115;
	}
	if ( memcmp(pDataRead, sDataBuildB, sizeof(sDataBuildB)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 116;
	}
	xpkFree(pDataRead);
	iRetCall = xpkSetFlag(objXpk, 1, XPK_FLAG_COMP_MASK, 14);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorCompManaged) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 763 : 764;
	}
	xpkClose(objXpk);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgVolume, &objOpt);
