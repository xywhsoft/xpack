	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgIndex, &objOpt);
	if ( objXpk == NULL ) {
		return 34;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_INDEX) != XPK_OK ) {
		xpkClose(objXpk);
		return 35;
	}
	iRetCall = xpkAddData(objXpk, sDataIndex, sizeof(sDataIndex), NULL, &iPosRet);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 793 : 794;
	}
	iRetCall = xpkAddFile(objXpk, sPathPkgCore, NULL, &iPosRet);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 795 : 796;
	}

	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkIndexAddData(objXpk, 1001, sDataIndex, sizeof(sDataIndex), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 36;
	}
	pDataRead = xpkIndexReadToMemory(objXpk, 1001, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 37;
	}
	if ( iDataSize != sizeof(sDataIndex) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 38;
	}
	if ( memcmp(pDataRead, sDataIndex, sizeof(sDataIndex)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 39;
	}
	xpkFree(pDataRead);
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 40;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgIndex, NULL);
	if ( objXpk == NULL ) {
		return 41;
	}
	pDataRead = xpkIndexReadToMemory(objXpk, 1001, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 42;
	}
	if ( iDataSize != sizeof(sDataIndex) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 43;
	}
	if ( memcmp(pDataRead, sDataIndex, sizeof(sDataIndex)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 44;
	}
	xpkFree(pDataRead);
	if ( xpkIndexRemove(objXpk, 1001) != XPK_OK ) {
		xpkClose(objXpk);
		return 299;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_INDEX) != XPK_OK ) {
		xpkClose(objXpk);
		return 815;
	}
	if ( xpkIndexAddData(objXpk, 1001, sDataIndex, sizeof(sDataIndex), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 300;
	}
	if ( xpkSetFlag(objXpk, 1, XPK_FLAG_DELETED_MASK, 0) != XPK_ERR_FORMAT ) {
		xpkClose(objXpk);
		return 301;
	}
	if ( xpkCount(objXpk) != 1 ) {
		xpkClose(objXpk);
		return 302;
	}
	if ( xpkIndexFind(objXpk, 1001, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 303;
	}
	if ( iPosRet != 2 ) {
		xpkClose(objXpk);
		return 304;
	}
	pMapEntry = (xpkEntry*)xrtListGet(&objXpk->lstEntry, 1001);
	if ( pMapEntry == NULL ) {
		xpkClose(objXpk);
		return 705;
	}
	iPosSaved = pMapEntry->iPos;
	pMapEntry->iPos = 0;
	iRetCall = xpkIndexFind(objXpk, 1001, &iPosRet);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		pMapEntry->iPos = iPosSaved;
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 706 : 707;
	}
	pMapEntry->iPos = iPosSaved;
	if ( !xrtListRemove(&objXpk->lstEntry, 1001) ) {
		xpkClose(objXpk);
		return 825;
	}
	iRetCall = xpkIndexFind(objXpk, 1001, &iPosRet);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 826 : 827;
	}
	if ( xpkCount(objXpk) != 0 ) {
		xpkClose(objXpk);
		return 756;
	}
	if ( !procTestExpectLastError(objXpk, XPK_ERR_STATE, NULL) ) {
		xpkClose(objXpk);
		return 757;
	}
	iRetCall = xpkVerifyAll(objXpk);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 729 : 730;
	}
	iRetCall = xpkVerify(objXpk, iPosSaved);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 737 : 738;
	}
	iRetCall = xpkStatGet(objXpk, &objStat);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBufferedPending) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 739 : 740;
	}
	iEachCount = 0;
	iRetCall = xpkEach(objXpk, NULL, &iEachCount);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 745 : 746;
	}
	iRetCall = xpkSave(objXpk);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 749 : 750;
	}
	iRetCall = xpkBuild(objXpk, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 731 : 732;
	}
	iRetCall = xpkIndexAddData(objXpk, 1001, sDataIndex, sizeof(sDataIndex), &objWriteOpt);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 725 : 726;
	}
	iRetCall = xpkIndexGetInfo(objXpk, 1001, (xpkFileInfoIndex*)&objInfoPath);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 708 : 709;
	}
	iRetCall = xpkGetInfo(objXpk, iPosSaved, &objInfo);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 773 : 774;
	}
	pMetaRead = xpkReadToMemory(objXpk, iPosSaved, &iDataSize);
	if ( !procTestExpectNullResultError(pMetaRead, objXpk, XPK_ERR_STATE, NULL) ) {
		if ( pMetaRead != NULL ) {
			xpkFree(pMetaRead);
			xpkClose(objXpk);
			return 775;
		}
		xpkClose(objXpk);
		return 776;
	}
	iRetCall = xpkUpdateData(objXpk, iPosSaved, sDataIndex, sizeof(sDataIndex), &objWriteOpt);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 777 : 778;
	}
	iRetCall = xpkUpdateFile(objXpk, iPosSaved, "release/x64/xpack_phase3_missing_update_src.bin", &objWriteOpt);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 785 : 786;
	}
	if ( procXpkRebuildLookup(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 828;
	}
	procXpkClearError(objXpk);
	if ( xpkCount(objXpk) != 1 ) {
		xpkClose(objXpk);
		return 1001;
	}
	objXpk->iEntryCount--;
	iRetCall = xpkIndexFind(objXpk, 1001, &iPosRet);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 840 : 841;
	}
	objXpk->iEntryCount++;
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 305;
	}
	if ( xpkIndexAddData(objXpk, 1002, sDataBuildA, sizeof(sDataBuildA), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 306;
	}
	if ( xpkIndexUpdateData(objXpk, 1001, sDataIndexUpdate, sizeof(sDataIndexUpdate), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 307;
	}
	iPackTypeSaved = objXpk->objHead.packType;
	objXpk->objHead.packType = XPK_PACK_WIN32;
	iRetCall = xpkRemove(objXpk, 2);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		objXpk->objHead.packType = iPackTypeSaved;
		procXpkRebuildLookup(objXpk);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 308 : 1002;
	}
	objXpk->objHead.packType = iPackTypeSaved;
	if ( procXpkRebuildLookup(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 309;
	}
	procXpkClearError(objXpk);
	if ( xpkCount(objXpk) != 2 ) {
		xpkClose(objXpk);
		return 310;
	}
	if ( xpkIndexFind(objXpk, 1001, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 311;
	}
	if ( iPosRet != 2 ) {
		xpkClose(objXpk);
		return 312;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 313;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgIndex, NULL);
	if ( objXpk == NULL ) {
		return 314;
	}
	pDataRead = xpkIndexReadToMemory(objXpk, 1001, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 315;
	}
	if ( iDataSize != sizeof(sDataIndexUpdate) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 316;
	}
	if ( memcmp(pDataRead, sDataIndexUpdate, sizeof(sDataIndexUpdate)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 317;
	}
	xpkFree(pDataRead);
	if ( xpkIndexFind(objXpk, 1002, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 318;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkIndexAddData(objXpk, 1003, sDataIndexPending, sizeof(sDataIndexPending), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 319;
	}
	iPackTypeSaved = objXpk->objHead.packType;
	objXpk->objHead.packType = XPK_PACK_WIN32;
	iRetCall = xpkRemove(objXpk, 4);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		objXpk->objHead.packType = iPackTypeSaved;
		procXpkRebuildLookup(objXpk);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 320 : 1003;
	}
	objXpk->objHead.packType = iPackTypeSaved;
	if ( procXpkRebuildLookup(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 321;
	}
	procXpkClearError(objXpk);
	if ( xpkCount(objXpk) != 3 ) {
		xpkClose(objXpk);
		return 322;
	}
	if ( xpkIndexFind(objXpk, 1003, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 323;
	}
	if ( iPosRet != 4 ) {
		xpkClose(objXpk);
		return 324;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 325;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgIndex, NULL);
	if ( objXpk == NULL ) {
		return 326;
	}
	pDataRead = xpkIndexReadToMemory(objXpk, 1003, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 327;
	}
	if ( iDataSize != sizeof(sDataIndexPending) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 328;
	}
	if ( memcmp(pDataRead, sDataIndexPending, sizeof(sDataIndexPending)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 329;
	}
	xpkFree(pDataRead);
	xpkClose(objXpk);
