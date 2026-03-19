	objXpk = xpkOpen(sPathPkgLayout, &objOpt);
	if ( objXpk == NULL ) {
		return 206;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_WIN32) != XPK_OK ) {
		xpkClose(objXpk);
		return 601;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkPathAddData(objXpk, "assets/all_deleted.txt", sDataSolidA, sizeof(sDataSolidA), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 602;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 603;
	}
	if ( xpkSetSolidMode(objXpk, TRUE) != XPK_OK ) {
		xpkClose(objXpk);
		return 604;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 605;
	}
	if ( xpkPathRemove(objXpk, "assets/all_deleted.txt") != XPK_OK ) {
		xpkClose(objXpk);
		return 606;
	}
	pEntry = (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, 1);
	if ( pEntry == NULL || pEntry->sPath == NULL ) {
		xpkClose(objXpk);
		return 753;
	}
	iCorruptByte = (uint8_t)pEntry->sPath[0];
	pEntry->sPath[0] = '\0';
	iRetCall = xpkSave(objXpk);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_FORMAT, objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat) ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_FORMAT) ? 754 : 755;
	}
	pEntry->sPath[0] = (char)iCorruptByte;
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 607;
	}
	if ( xpkCount(objXpk) != 0 ) {
		xpkClose(objXpk);
		return 608;
	}
	if ( xpkSetDefaultComp(objXpk, 14) != XPK_OK ) {
		xpkClose(objXpk);
		return 609;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 610;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 611;
	}
	if ( xpkSetSolidMode(objXpk, FALSE) != XPK_OK ) {
		xpkClose(objXpk);
		return 612;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 613;
	}
	if ( xpkGetSolidMode(objXpk, &iEachCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 614;
	}
	if ( iEachCount != FALSE ) {
		xpkClose(objXpk);
		return 615;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 616;
	}
	if ( xpkCount(objXpk) != 0 ) {
		xpkClose(objXpk);
		return 617;
	}
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgLayout);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgLayout, &objOpt);
	if ( objXpk == NULL ) {
		return 206;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, sDataVolumeB, sizeof(sDataVolumeB), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 207;
	}
	if ( iPosRet != 1 ) {
		xpkClose(objXpk);
		return 208;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 209;
	}
	if ( xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN) != XPK_OK ) {
		xpkClose(objXpk);
		return 210;
	}
	if ( xpkSave(objXpk) != XPK_ERR_STATE ) {
		xpkClose(objXpk);
		return 211;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 212;
	}
	if ( xpkGetVolumeSize(objXpk, &iVolumeSize) != XPK_OK ) {
		xpkClose(objXpk);
		return 213;
	}
	if ( iVolumeSize != XPK_VOLUME_MIN ) {
		xpkClose(objXpk);
		return 214;
	}
	iVolumeCount = 0;
	if ( procXpkCountVolumeFilesText(objXpk, objXpk->sPathPackage, &iVolumeCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 215;
	}
	if ( iVolumeCount < 2 ) {
		xpkClose(objXpk);
		return 216;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 217;
	}
	if ( iDataSize != sizeof(sDataVolumeB) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 218;
	}
	if ( memcmp(pDataRead, sDataVolumeB, sizeof(sDataVolumeB)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 219;
	}
	xpkFree(pDataRead);
	if ( xpkSetVolumeSize(objXpk, 0) != XPK_OK ) {
		xpkClose(objXpk);
		return 220;
	}
	if ( xpkSave(objXpk) != XPK_ERR_STATE ) {
		xpkClose(objXpk);
		return 221;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 222;
	}
	if ( xpkGetVolumeSize(objXpk, &iVolumeSize) != XPK_OK ) {
		xpkClose(objXpk);
		return 223;
	}
	if ( iVolumeSize != 0 ) {
		xpkClose(objXpk);
		return 224;
	}
	iVolumeCount = 0;
	if ( procXpkCountVolumeFilesText(objXpk, objXpk->sPathPackage, &iVolumeCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 225;
	}
	if ( iVolumeCount != 1 ) {
		xpkClose(objXpk);
		return 226;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 227;
	}
	if ( iDataSize != sizeof(sDataVolumeB) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 228;
	}
	if ( memcmp(pDataRead, sDataVolumeB, sizeof(sDataVolumeB)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 229;
	}
	xpkFree(pDataRead);
	xpkClose(objXpk);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgReadonly, &objOpt);
