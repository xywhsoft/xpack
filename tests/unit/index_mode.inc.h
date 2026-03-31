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

	{
		const char* sPathPkgIndexCodecBuffered;
		xpkObject objXpkCodec;

		sPathPkgIndexCodecBuffered = "release/x64/xpack_phase3_index_codec_buffered.xpk";
		procTestDeletePathFamily(sPathPkgIndexCodecBuffered);
		memset(&objOpt, 0, sizeof(objOpt));
		objOpt.createIfMissing = TRUE;
		objXpkCodec = xpkOpen(sPathPkgIndexCodecBuffered, &objOpt);
		if ( objXpkCodec == NULL ) {
			return 2137;
		}
		if ( xpkSetPackType(objXpkCodec, XPK_PACK_INDEX) != XPK_OK ) {
			xpkClose(objXpkCodec);
			return 2138;
		}
		memset(&objWriteOpt, 0, sizeof(objWriteOpt));
		objWriteOpt.compLevel = 14;
		objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
		if ( xpkIndexAddData(objXpkCodec, 1401, sDataBuildB, sizeof(sDataBuildB), &objWriteOpt) != XPK_OK ) {
			xpkClose(objXpkCodec);
			return 2139;
		}
		if ( xpkSave(objXpkCodec) != XPK_OK ) {
			xpkClose(objXpkCodec);
			return 2140;
		}
		xpkClose(objXpkCodec);

		objXpkCodec = xpkOpen(sPathPkgIndexCodecBuffered, NULL);
		if ( objXpkCodec == NULL ) {
			return 2141;
		}
		if ( xpkVerifyAll(objXpkCodec) != XPK_OK ) {
			xpkClose(objXpkCodec);
			return 2142;
		}
		procTestDeletePathFamily(sPathFileExport);
		if ( xpkIndexReadToFile(objXpkCodec, 1401, sPathFileExport) != XPK_OK ) {
			xpkClose(objXpkCodec);
			return 2143;
		}
		if ( !procTestFileContentEquals(sPathFileExport, sDataBuildB, sizeof(sDataBuildB)) ) {
			xpkClose(objXpkCodec);
			return 2144;
		}
		xpkClose(objXpkCodec);
		procTestDeletePathFamily(sPathPkgIndexCodecBuffered);
	}

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

	procTestDeletePathFamily(sPathPkgTest);
	iDataLargeSize = (128u * 1024u) + 123u;
	pDataLarge = xpkAllocInternal(iDataLargeSize);
	if ( pDataLarge == NULL ) {
		return 1066;
	}
	memset(pDataLarge, 'I', iDataLargeSize);
	if ( !procTestWriteBinaryFile(sPathFileExport, pDataLarge, (size_t)iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1067;
	}

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgTest, &objOpt);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1068;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_INDEX) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1069;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkIndexAddFile(objXpk, 2001, sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1070;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1071;
	}
	pDataRead = xpkIndexReadToMemory(objXpk, 2001, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1072;
	}
	if ( (iDataSize != iDataLargeSize) || (memcmp(pDataRead, pDataLarge, (size_t)iDataLargeSize) != 0) ) {
		xpkFree(pDataRead);
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1073;
	}
	xpkFree(pDataRead);

	memset(pDataLarge, 'J', iDataLargeSize);
	if ( !procTestWriteBinaryFile(sPathFileExport, pDataLarge, (size_t)iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1074;
	}
	if ( xpkIndexUpdateFile(objXpk, 2001, sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1075;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1076;
	}
	pDataRead = xpkIndexReadToMemory(objXpk, 2001, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1077;
	}
	if ( (iDataSize != iDataLargeSize) || (memcmp(pDataRead, pDataLarge, (size_t)iDataLargeSize) != 0) ) {
		xpkFree(pDataRead);
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1078;
	}
	xpkFree(pDataRead);
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1079;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgTest, NULL);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1080;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1081;
	}
	pDataRead = xpkIndexReadToMemory(objXpk, 2001, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1082;
	}
	if ( (iDataSize != iDataLargeSize) || (memcmp(pDataRead, pDataLarge, (size_t)iDataLargeSize) != 0) ) {
		xpkFree(pDataRead);
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1083;
	}
	xpkFree(pDataRead);
	xpkFreeInternal(pDataLarge);
	pDataLarge = NULL;
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgTest);

	procTestDeletePathFamily(sPathPkgTest);
	iDataLargeSize = (72u * 1024u) + 123u;
	pDataLarge = xpkAllocInternal(iDataLargeSize);
	if ( pDataLarge == NULL ) {
		return 1900;
	}
	memset(pDataLarge, 'L', iDataLargeSize);
	if ( !procTestWriteBinaryFile(sPathFileExport, pDataLarge, (size_t)iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1901;
	}

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgTest, &objOpt);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1902;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_INDEX) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1903;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 2;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkIndexAddFile(objXpk, 8101, sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1904;
	}

	memset(pDataLarge, 'M', iDataLargeSize);
	if ( !procTestWriteBinaryFile(sPathFileExport, pDataLarge, (size_t)iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1905;
	}
	if ( xpkIndexUpdateFile(objXpk, 8101, sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1906;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1907;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgTest, NULL);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1908;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1909;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkIndexReadToFile(objXpk, 8101, sPathFileExport) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1910;
	}
	if ( !procTestFileContentEquals(sPathFileExport, pDataLarge, iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1911;
	}
	xpkFreeInternal(pDataLarge);
	pDataLarge = NULL;
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgTest);

	procTestDeletePathFamily(sPathPkgTest);
	procTestDeletePathFamily(sPathFileExport);
	if ( !procTestWriteBinaryFile(sPathFileExport, NULL, 0) ) {
		return 1482;
	}
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgTest, &objOpt);
	if ( objXpk == NULL ) {
		return 1483;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_INDEX) != XPK_OK ) {
		xpkClose(objXpk);
		return 1484;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkIndexAddFile(objXpk, 7001, sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 1485;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1486;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgTest, NULL);
	if ( objXpk == NULL ) {
		return 1487;
	}
	if ( xpkIndexGetInfo(objXpk, 7001, &objInfoIndex) != XPK_OK ) {
		xpkClose(objXpk);
		return 1488;
	}
	if ( (objInfoIndex.fileHash != 0) || (objInfoIndex.fileSize != 0) || (objInfoIndex.dataSize != 0) ) {
		xpkClose(objXpk);
		return 1489;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1490;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkIndexReadToFile(objXpk, 7001, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1491;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, 0) ) {
		xpkClose(objXpk);
		return 1492;
	}
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgTest);

	procTestDeletePathFamily(sPathPkgTest);
	procTestDeletePathFamily(sPathFileExport);
	if ( !procTestWriteBinaryFile(sPathFileExport, "INDEX-ZERO-UPDATE", sizeof("INDEX-ZERO-UPDATE")) ) {
		return 1540;
	}
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgTest, &objOpt);
	if ( objXpk == NULL ) {
		return 1541;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_INDEX) != XPK_OK ) {
		xpkClose(objXpk);
		return 1542;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkIndexAddFile(objXpk, 7002, sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 1543;
	}
	if ( !procTestWriteBinaryFile(sPathFileExport, NULL, 0) ) {
		xpkClose(objXpk);
		return 1544;
	}
	if ( xpkIndexUpdateFile(objXpk, 7002, sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 1545;
	}
	if ( xpkIndexGetInfo(objXpk, 7002, &objInfoIndex) != XPK_OK ) {
		xpkClose(objXpk);
		return 1546;
	}
	if ( (objInfoIndex.fileHash != 0) || (objInfoIndex.fileSize != 0) || (objInfoIndex.dataSize != 0) ) {
		xpkClose(objXpk);
		return 1547;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1548;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgTest, NULL);
	if ( objXpk == NULL ) {
		return 1549;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1550;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkIndexReadToFile(objXpk, 7002, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1551;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, 0) ) {
		xpkClose(objXpk);
		return 1552;
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
		return 1131;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_INDEX) != XPK_OK ) {
		xpkClose(objXpk);
		return 1132;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkIndexAddData(objXpk, 4001, "KEEP-INDEX", sizeof("KEEP-INDEX"), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 1133;
	}
	if ( !procTestCreateSparseFileSize(sPathFileHuge, (uint64_t)UINT32_MAX + 44444u) ) {
		xpkClose(objXpk);
		return 1134;
	}
	if ( xpkIndexUpdateFile(objXpk, 4001, sPathFileHuge, &objWriteOpt) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1135;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1136;
	}
	if ( objInfo.fileSize != ((uint64_t)UINT32_MAX + 44444u) || objInfo.dataSize != objInfo.fileSize || (objInfo.flag & XPK_FLAG_COMP_MASK) != 0 ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1137;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1138;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkIndexReadToFile(objXpk, 4001, sPathFileExport) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1139;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, (uint64_t)UINT32_MAX + 44444u) ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1140;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1462;
	}
	xpkClose(objXpk);
	objXpk = xpkOpen(sPathPkgTest, NULL);
	if ( objXpk == NULL ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		return 1463;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1464;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkIndexReadToFile(objXpk, 4001, sPathFileExport) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1465;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, (uint64_t)UINT32_MAX + 44444u) ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1466;
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
		return 1146;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_INDEX) != XPK_OK ) {
		xpkClose(objXpk);
		return 1147;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( !procTestCreateSparseFileSize(sPathFileHuge, (uint64_t)UINT32_MAX + 88888u) ) {
		xpkClose(objXpk);
		return 1148;
	}
	procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
	if ( !procTestCreateDirOccupy("release/x64/xpack_phase3_store_huge_pkg_dir") ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1149;
	}
	sPathSaved = objXpk->sPathPackage;
	objXpk->sPathPackage = procXpkDupText("release/x64/xpack_phase3_store_huge_pkg_dir");
	if ( objXpk->sPathPackage == NULL ) {
		objXpk->sPathPackage = sPathSaved;
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1150;
	}
	iRetCall = xpkIndexAddFile(objXpk, 6001, sPathFileHuge, &objWriteOpt);
	xpkFreeInternal(objXpk->sPathPackage);
	objXpk->sPathPackage = sPathSaved;
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_IO, objXpk, XPK_ERR_IO, sXpkErrorIoOpen) ) {
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_IO) ? 1151 : 1152;
	}
	if ( xpkCount(objXpk) != 0 ) {
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1153;
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
		return 1154;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_INDEX) != XPK_OK ) {
		xpkClose(objXpk);
		return 1155;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkIndexAddData(objXpk, 6002, "KEEP-INDEX-IO", sizeof("KEEP-INDEX-IO"), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 1156;
	}
	if ( !procTestCreateSparseFileSize(sPathFileHuge, (uint64_t)UINT32_MAX + 101010u) ) {
		xpkClose(objXpk);
		return 1157;
	}
	procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
	if ( !procTestCreateDirOccupy("release/x64/xpack_phase3_store_huge_pkg_dir") ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1158;
	}
	sPathSaved = objXpk->sPathPackage;
	objXpk->sPathPackage = procXpkDupText("release/x64/xpack_phase3_store_huge_pkg_dir");
	if ( objXpk->sPathPackage == NULL ) {
		objXpk->sPathPackage = sPathSaved;
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1159;
	}
	iRetCall = xpkIndexUpdateFile(objXpk, 6002, sPathFileHuge, &objWriteOpt);
	xpkFreeInternal(objXpk->sPathPackage);
	objXpk->sPathPackage = sPathSaved;
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_IO, objXpk, XPK_ERR_IO, sXpkErrorIoOpen) ) {
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_IO) ? 1160 : 1161;
	}
	pDataRead = xpkIndexReadToMemory(objXpk, 6002, &iDataSize);
	if ( pDataRead == NULL ) {
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1162;
	}
	if ( (iDataSize != sizeof("KEEP-INDEX-IO")) || (memcmp(pDataRead, "KEEP-INDEX-IO", sizeof("KEEP-INDEX-IO")) != 0) ) {
		xpkFree(pDataRead);
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1163;
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
		return 1140;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_INDEX) != XPK_OK ) {
		xpkClose(objXpk);
		return 1141;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( !procTestCreateSparseFileSize(sPathFileHuge, (uint64_t)UINT32_MAX + 66666u) ) {
		xpkClose(objXpk);
		return 1142;
	}
	if ( xpkIndexAddFile(objXpk, 5001, sPathFileHuge, &objWriteOpt) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1143;
	}
	if ( xpkCount(objXpk) != 1 ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1144;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1145;
	}
	if ( objInfo.fileSize != ((uint64_t)UINT32_MAX + 66666u) || objInfo.dataSize != objInfo.fileSize || (objInfo.flag & XPK_FLAG_COMP_MASK) != 0 ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1146;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1147;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkIndexReadToFile(objXpk, 5001, sPathFileExport) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1148;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, (uint64_t)UINT32_MAX + 66666u) ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1149;
	}
	procTestDeletePathFamily(sPathFileExport);
	remove(sPathFileHuge);
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgTest);
	}

	procTestDeletePathFamily(sPathPkgTest);
	iDataLargeSize = (96u * 1024u) + 57u;
	pDataLarge = xpkAllocInternal(iDataLargeSize);
	if ( pDataLarge == NULL ) {
		return 1116;
	}
	memset(pDataLarge, 'K', iDataLargeSize);
	if ( !procTestWriteBinaryFile(sPathFileExport, pDataLarge, (size_t)iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1117;
	}

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgTest, &objOpt);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1118;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_INDEX) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1119;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkIndexAddFile(objXpk, 3001, sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1120;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1121;
	}
	pDataRead = xpkIndexReadToMemory(objXpk, 3001, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1122;
	}
	if ( (iDataSize != iDataLargeSize) || (memcmp(pDataRead, pDataLarge, (size_t)iDataLargeSize) != 0) ) {
		xpkFree(pDataRead);
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1123;
	}
	xpkFree(pDataRead);

	memset(pDataLarge, 'L', iDataLargeSize);
	if ( !procTestWriteBinaryFile(sPathFileExport, pDataLarge, (size_t)iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1124;
	}
	if ( xpkIndexUpdateFile(objXpk, 3001, sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1125;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkIndexReadToFile(objXpk, 3001, sPathFileExport) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1637;
	}
	if ( !procTestFileContentEquals(sPathFileExport, pDataLarge, iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1638;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1126;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgTest, NULL);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1127;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1128;
	}
	pDataRead = xpkIndexReadToMemory(objXpk, 3001, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1129;
	}
	if ( (iDataSize != iDataLargeSize) || (memcmp(pDataRead, pDataLarge, (size_t)iDataLargeSize) != 0) ) {
		xpkFree(pDataRead);
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1130;
	}
	xpkFree(pDataRead);
	xpkFreeInternal(pDataLarge);
	pDataLarge = NULL;
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgTest);
