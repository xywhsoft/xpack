	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgCore, &objOpt);
	if ( objXpk == NULL ) {
		return 26;
	}
	if ( xpkSetInfoExtSize(objXpk, sizeof(arrInfoExtCoreSet)) != XPK_OK ) {
		xpkClose(objXpk);
		return 270;
	}
	if ( xpkGetInfoExtSize(objXpk, &iMetaSize) != XPK_OK ) {
		xpkClose(objXpk);
		return 271;
	}
	if ( iMetaSize != sizeof(arrInfoExtCoreSet) ) {
		xpkClose(objXpk);
		return 272;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_CORE) != XPK_OK ) {
		xpkClose(objXpk);
		return 808;
	}
	if ( xpkGetInfoExtSize(objXpk, &iMetaSize) != XPK_OK ) {
		xpkClose(objXpk);
		return 809;
	}
	if ( iMetaSize != sizeof(arrInfoExtCoreSet) ) {
		xpkClose(objXpk);
		return 810;
	}

	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 1;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, sDataCore, sizeof(sDataCore), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 273;
	}
	if ( iPosRet != 1 ) {
		xpkClose(objXpk);
		return 274;
	}
	if ( xpkSetInfoExt(objXpk, 1, arrInfoExtCoreSet, sizeof(arrInfoExtCoreSet)) != XPK_OK ) {
		xpkClose(objXpk);
		return 275;
	}
	if ( xpkSetInfoExtSize(objXpk, sizeof(arrInfoExtCoreSet)) != XPK_OK ) {
		xpkClose(objXpk);
		return 811;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_CORE) != XPK_OK ) {
		xpkClose(objXpk);
		return 812;
	}
	if ( xpkGetInfoExtSize(objXpk, &iMetaSize) != XPK_OK ) {
		xpkClose(objXpk);
		return 813;
	}
	if ( iMetaSize != sizeof(arrInfoExtCoreSet) ) {
		xpkClose(objXpk);
		return 814;
	}
	iRetCall = xpkGetInfo(objXpk, 0, &objInfo);
	iCheckRet = procTestCheckCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	if ( iCheckRet != 0 ) {
		xpkClose(objXpk);
		return (iCheckRet == 1) ? 291 : ((iCheckRet == 2) ? 292 : 293);
	}
	if ( xpkCount(objXpk) != 1 ) {
		xpkClose(objXpk);
		return 449;
	}
	iCheckRet = procTestCheckLastError(objXpk, XPK_OK, "");
	if ( iCheckRet != 0 ) {
		xpkClose(objXpk);
		return (iCheckRet == 1) ? 450 : 451;
	}
	objXpk->iEntryCount++;
	if ( xpkCount(objXpk) != 0 ) {
		xpkClose(objXpk);
		return 686;
	}
	iCheckRet = procTestCheckLastError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	if ( iCheckRet != 0 ) {
		xpkClose(objXpk);
		return (iCheckRet == 1) ? 687 : 688;
	}
	iRetCall = xpkGetInfo(objXpk, 2, &objInfo);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 816 : 817;
	}
	pDataRead = xpkReadToMemory(objXpk, 2, &iDataSize);
	iCheckRet = procTestCheckNullResultError(pDataRead, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	if ( iCheckRet != 0 ) {
		if ( pDataRead != NULL ) {
			xpkFree(pDataRead);
			xpkClose(objXpk);
			return 818;
		}
		xpkClose(objXpk);
		return (iCheckRet == 2) ? 819 : 820;
	}
	iRetCall = xpkVerify(objXpk, 2);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 821 : 822;
	}
	iRetCall = xpkSetInfoExt(objXpk, 2, arrInfoExtCoreSet, sizeof(arrInfoExtCoreSet));
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 823 : 824;
	}
	objXpk->iEntryCount--;
	if ( xpkCount(objXpk) != 1 ) {
		xpkClose(objXpk);
		return 689;
	}
	objXpk->iEntryCount--;
	if ( xpkCount(objXpk) != 0 ) {
		xpkClose(objXpk);
		return 835;
	}
	iCheckRet = procTestCheckLastError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	if ( iCheckRet != 0 ) {
		xpkClose(objXpk);
		return (iCheckRet == 1) ? 836 : 837;
	}
	iRetCall = xpkGetInfo(objXpk, 1, &objInfo);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 838 : 839;
	}
	iRetCall = xpkSetDefaultComp(objXpk, 7);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 849 : 850;
	}
	iRetCall = xpkMetaSet(objXpk, sMetaTest, sizeof(sMetaTest), 6);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 851 : 852;
	}
	iRetCall = xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 853 : 854;
	}
	iRetCall = xpkSetSolidMode(objXpk, TRUE);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 855 : 856;
	}
	iRetCall = xpkSetInfoExtSize(objXpk, 0);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 857 : 858;
	}
	iRetCall = xpkSetPackType(objXpk, XPK_PACK_INDEX);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 859 : 860;
	}
	iRetCall = xpkAddData(objXpk, sDataCore, sizeof(sDataCore), NULL, &iPosRet);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 861 : 862;
	}
	if ( objXpk->arrEntry.Count != 1 || objXpk->iEntryCount != 0 ) {
		xpkClose(objXpk);
		return 863;
	}
	iRetCall = xpkSave(objXpk);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 845 : 846;
	}
	iRetCall = xpkBuild(objXpk, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorBadFormat) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 847 : 848;
	}
	objXpk->iEntryCount++;
	if ( xpkGetInfo(objXpk, 0, &objInfo) != XPK_ERR_PARAM ) {
		xpkClose(objXpk);
		return 484;
	}
	iBuildSizeBefore = objXpk->iAppendPos;
	iBuildSizeAfter = objXpk->iFileSize;
	objXpk->iAppendPos = XPK_HEAD_SIZE;
	objXpk->iFileSize = XPK_HEAD_SIZE;
	if ( xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN) != XPK_OK ) {
		xpkClose(objXpk);
		return 864;
	}
	if ( objXpk->bVolumeApplied ) {
		xpkClose(objXpk);
		return 865;
	}
	iRetCall = xpkSave(objXpk);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorVolumeBuildRequired) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 866 : 867;
	}
	objXpk->iAppendPos = iBuildSizeBefore;
	objXpk->iFileSize = iBuildSizeAfter;
	if ( xpkSetVolumeSize(objXpk, 0) != XPK_OK ) {
		xpkClose(objXpk);
		return 868;
	}
	objXpk->iAppendPos = XPK_HEAD_SIZE;
	objXpk->iFileSize = XPK_HEAD_SIZE;
	if ( xpkSetSolidMode(objXpk, TRUE) != XPK_OK ) {
		xpkClose(objXpk);
		return 869;
	}
	if ( objXpk->bSolidApplied ) {
		xpkClose(objXpk);
		return 870;
	}
	iRetCall = xpkSave(objXpk);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorSolidBuildRequired) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 871 : 872;
	}
	objXpk->iAppendPos = iBuildSizeBefore;
	objXpk->iFileSize = iBuildSizeAfter;
	if ( xpkSetSolidMode(objXpk, FALSE) != XPK_OK ) {
		xpkClose(objXpk);
		return 873;
	}
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 485;
	}
	if ( !procTestExpectLastError(objXpk, XPK_OK, "") ) {
		xpkClose(objXpk);
		return 486;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataCore, sizeof(sDataCore)) ) {
		xpkClose(objXpk);
		return 490;
	}
	memset(arrInfoExtCoreRead, 0, sizeof(arrInfoExtCoreRead));
	if ( xpkGetInfoExt(objXpk, 1, arrInfoExtCoreRead, sizeof(arrInfoExtCoreRead)) != XPK_OK ) {
		xpkClose(objXpk);
		return 276;
	}
	if ( memcmp(arrInfoExtCoreRead, arrInfoExtCoreSet, sizeof(arrInfoExtCoreSet)) != 0 ) {
		xpkClose(objXpk);
		return 277;
	}
	if ( xpkSetInfoExtSize(objXpk, 8) != XPK_ERR_STATE ) {
		xpkClose(objXpk);
		return 278;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 279;
	}
	hFile = xrtOpen((str)sPathPkgCore, FALSE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		xpkClose(objXpk);
		return 973;
	}
	iBuildSizeBefore = (uint64_t)UINT32_MAX + 4096u;
	if ( procXpkSeekFile(objXpk, hFile, iBuildSizeBefore) != XPK_OK ) {
		xrtClose(hFile);
		xpkClose(objXpk);
		return 974;
	}
	if ( xrtTell(hFile) != iBuildSizeBefore ) {
		xrtClose(hFile);
		xpkClose(objXpk);
		return 975;
	}
	iRetCall = procXpkSeekFile(objXpk, hFile, (uint64_t)INT64_MAX + 1u);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_UNSUPPORTED, objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorSeekRange) ) {
		xrtClose(hFile);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_UNSUPPORTED) ? 983 : 984;
	}
	if ( xrtTell(hFile) != iBuildSizeBefore ) {
		xrtClose(hFile);
		xpkClose(objXpk);
		return 985;
	}
	xrtClose(hFile);
	iDataLargeSize = (9u * 1024u * 1024u) + 321u;
	pDataLarge = xpkAllocInternal(iDataLargeSize);
	if ( pDataLarge == NULL ) {
		xpkClose(objXpk);
		return 976;
	}
	memset(pDataLarge, 'E', iDataLargeSize);
	if ( procXpkWriteFileData(objXpk, sPathFileExport, pDataLarge, iDataLargeSize) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 977;
	}
	if ( !procTestFileContentEquals(sPathFileExport, pDataLarge, iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 978;
	}
	xpkFreeInternal(pDataLarge);
	pDataLarge = NULL;
	if ( !procTestCreateSparseFileSize(sPathFileHuge, (uint64_t)UINT32_MAX + 12345u) ) {
		xpkClose(objXpk);
		return 979;
	}
	iRetCall = xpkAddFile(objXpk, sPathFileHuge, NULL, &iPosRet);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_UNSUPPORTED, objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge) ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_UNSUPPORTED) ? 980 : 981;
	}
	if ( xpkCount(objXpk) != 1 ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 982;
	}
	remove(sPathFileHuge);
	xpkClose(objXpk);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgStoreHuge, &objOpt);
	if ( objXpk == NULL ) {
		return 986;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, "S", 1, &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 987;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 988;
	}
	pEntry = procXpkGetEntryByPos(objXpk, 1);
	if ( pEntry == NULL ) {
		xpkClose(objXpk);
		return 989;
	}
	iBuildSizeAfter = (uint64_t)UINT32_MAX + 65537u;
	if ( !procTestCreateSparseFileSize(sPathPkgStoreHuge, XPK_HEAD_SIZE + iBuildSizeAfter) ) {
		xpkClose(objXpk);
		return 990;
	}
	objXpk->objHead.dataOffset = XPK_HEAD_SIZE + iBuildSizeAfter;
	pEntry->iFlag &= ~XPK_FLAG_COMP_MASK;
	pEntry->iDataOffset = XPK_HEAD_SIZE;
	pEntry->iDataSize = iBuildSizeAfter;
	pEntry->iFileSize = iBuildSizeAfter;
	procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_dst");
	if ( !procTestCreateDirOccupy("release/x64/xpack_phase3_store_huge_dst") ) {
		xpkClose(objXpk);
		return 991;
	}
	iRetCall = xpkReadToFile(objXpk, 1, "release/x64/xpack_phase3_store_huge_dst");
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_IO, objXpk, XPK_ERR_IO, sXpkErrorIoOpen) ) {
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_dst");
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_IO) ? 992 : 993;
	}
	if ( !xrtDirExists((str)"release/x64/xpack_phase3_store_huge_dst") ) {
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_dst");
		xpkClose(objXpk);
		return 994;
	}
	procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_dst");
	xpkClose(objXpk);
	remove(sPathPkgStoreHuge);

	objXpk = xpkOpen(sPathPkgCore, NULL);
	if ( objXpk == NULL ) {
		return 280;
	}
	if ( xpkGetInfoExtSize(objXpk, &iMetaSize) != XPK_OK ) {
		xpkClose(objXpk);
		return 281;
	}
	if ( iMetaSize != sizeof(arrInfoExtCoreSet) ) {
		xpkClose(objXpk);
		return 282;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 283;
	}
	if ( iDataSize != sizeof(sDataCore) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 284;
	}
	if ( memcmp(pDataRead, sDataCore, sizeof(sDataCore)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 285;
	}
	memset(arrInfoExtCoreRead, 0, sizeof(arrInfoExtCoreRead));
	if ( xpkGetInfoExt(objXpk, 1, arrInfoExtCoreRead, sizeof(arrInfoExtCoreRead)) != XPK_OK ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 286;
	}
	if ( memcmp(arrInfoExtCoreRead, arrInfoExtCoreSet, sizeof(arrInfoExtCoreSet)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 287;
	}
	sPathSaved = objXpk->sPathPackage;
	objXpk->sPathPackage = procXpkDupText("release/x64/xpack_phase3_reload_missing.xpk");
	if ( objXpk->sPathPackage == NULL ) {
		objXpk->sPathPackage = sPathSaved;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 882;
	}
	if ( procXpkBuildReloadSelf(objXpk) != XPK_ERR_NOT_FOUND ) {
		xpkFreeInternal(objXpk->sPathPackage);
		objXpk->sPathPackage = sPathSaved;
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 883;
	}
	xpkFreeInternal(objXpk->sPathPackage);
	objXpk->sPathPackage = sPathSaved;
	if ( xpkCount(objXpk) != 1 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 884;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 885;
	}
	memset(arrInfoExtCoreRead, 0, sizeof(arrInfoExtCoreRead));
	if ( xpkGetInfoExt(objXpk, 1, arrInfoExtCoreRead, sizeof(arrInfoExtCoreRead)) != XPK_OK ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 886;
	}
	if ( memcmp(arrInfoExtCoreRead, arrInfoExtCoreSet, sizeof(arrInfoExtCoreSet)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 887;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 288;
	}
	memset(arrInfoExtCoreRead, 0, sizeof(arrInfoExtCoreRead));
	if ( xpkGetInfoExt(objXpk, 1, arrInfoExtCoreRead, sizeof(arrInfoExtCoreRead)) != XPK_OK ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 289;
	}
	if ( memcmp(arrInfoExtCoreRead, arrInfoExtCoreSet, sizeof(arrInfoExtCoreSet)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 290;
	}
	xpkFree(pDataRead);
	xpkClose(objXpk);
