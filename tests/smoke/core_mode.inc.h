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
	if ( bRunLargeIo ) {
		/* opt-in 4GB+ large-io regression: keep smoke fast by default */
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
	procTestDeletePathFamily(sPathPkgStoreHuge);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objCheck = xpkOpen(sPathPkgStoreHuge, &objOpt);
	if ( objCheck == NULL ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1210;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	iRetCall = xpkAddFile(objCheck, sPathFileHuge, &objWriteOpt, &iPosRet);
	if ( iRetCall != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objCheck);
		objCheck = NULL;
		xpkClose(objXpk);
		return 1211;
	}
	if ( iPosRet != 1 ) {
		remove(sPathFileHuge);
		xpkClose(objCheck);
		objCheck = NULL;
		xpkClose(objXpk);
		return 1212;
	}
	if ( xpkCount(objCheck) != 1 ) {
		remove(sPathFileHuge);
		xpkClose(objCheck);
		objCheck = NULL;
		xpkClose(objXpk);
		return 1213;
	}
	if ( xpkRemove(objCheck, 1) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objCheck);
		objCheck = NULL;
		xpkClose(objXpk);
		return 1214;
	}
	if ( xpkCount(objCheck) != 0 ) {
		remove(sPathFileHuge);
		xpkClose(objCheck);
		objCheck = NULL;
		xpkClose(objXpk);
		return 1215;
	}
	remove(sPathFileHuge);
	xpkClose(objCheck);
	objCheck = NULL;
	procTestDeletePathFamily(sPathPkgStoreHuge);
	xpkClose(objXpk);

	procTestDeletePathFamily(sPathPkgStoreHuge);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgStoreHuge, &objOpt);
	if ( objXpk == NULL ) {
		return 1135;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( !procTestCreateSparseFileSize(sPathFileHuge, (uint64_t)UINT32_MAX + 24680u) ) {
		xpkClose(objXpk);
		return 1136;
	}
	procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
	if ( !procTestCreateDirOccupy("release/x64/xpack_phase3_store_huge_pkg_dir") ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1137;
	}
	sPathSaved = objXpk->sPathPackage;
	objXpk->sPathPackage = procXpkDupText("release/x64/xpack_phase3_store_huge_pkg_dir");
	if ( objXpk->sPathPackage == NULL ) {
		objXpk->sPathPackage = sPathSaved;
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1138;
	}
	iRetCall = xpkAddFile(objXpk, sPathFileHuge, &objWriteOpt, &iPosRet);
	xpkFreeInternal(objXpk->sPathPackage);
	objXpk->sPathPackage = sPathSaved;
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_IO, objXpk, XPK_ERR_IO, sXpkErrorIoOpen) ) {
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_IO) ? 1139 : 1140;
	}
	if ( xpkCount(objXpk) != 0 ) {
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1141;
	}
	procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
	remove(sPathFileHuge);
	xpkClose(objXpk);
	remove(sPathPkgStoreHuge);

	procTestDeletePathFamily(sPathPkgStoreHuge);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgStoreHuge, &objOpt);
	if ( objXpk == NULL ) {
		return 1142;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, "KEEP-HUGE-IO", sizeof("KEEP-HUGE-IO"), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1143;
	}
	if ( !procTestCreateSparseFileSize(sPathFileHuge, (uint64_t)UINT32_MAX + 35791u) ) {
		xpkClose(objXpk);
		return 1144;
	}
	procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
	if ( !procTestCreateDirOccupy("release/x64/xpack_phase3_store_huge_pkg_dir") ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1145;
	}
	sPathSaved = objXpk->sPathPackage;
	objXpk->sPathPackage = procXpkDupText("release/x64/xpack_phase3_store_huge_pkg_dir");
	if ( objXpk->sPathPackage == NULL ) {
		objXpk->sPathPackage = sPathSaved;
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1146;
	}
	iRetCall = xpkUpdateFile(objXpk, 1, sPathFileHuge, &objWriteOpt);
	xpkFreeInternal(objXpk->sPathPackage);
	objXpk->sPathPackage = sPathSaved;
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_IO, objXpk, XPK_ERR_IO, sXpkErrorIoOpen) ) {
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_IO) ? 1147 : 1148;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1149;
	}
	if ( (iDataSize != sizeof("KEEP-HUGE-IO")) || (memcmp(pDataRead, "KEEP-HUGE-IO", sizeof("KEEP-HUGE-IO")) != 0) ) {
		xpkFree(pDataRead);
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1150;
	}
	xpkFree(pDataRead);
	procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_pkg_dir");
	remove(sPathFileHuge);
	xpkClose(objXpk);
	remove(sPathPkgStoreHuge);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgStoreHuge, &objOpt);
	if ( objXpk == NULL ) {
		return 995;
	}
	iDataLargeSize = (9u * 1024u * 1024u) + 654u;
	pDataLarge = xpkAllocInternal(iDataLargeSize);
	if ( pDataLarge == NULL ) {
		xpkClose(objXpk);
		return 996;
	}
	memset(pDataLarge, 'Q', iDataLargeSize);
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkAddData(objXpk, pDataLarge, iDataLargeSize, &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 997;
	}
	if ( iPosRet != 1 ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 998;
	}
	if ( xpkVerify(objXpk, 1) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1007;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1008;
	}
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 999;
	}
	if ( !procTestExpectLastError(objXpk, XPK_OK, "") ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1000;
	}
	if ( !procTestFileContentEquals(sPathFileExport, pDataLarge, iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1001;
	}
	if ( xpkCount(objXpk) != 1 ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1002;
	}
	xpkFreeInternal(pDataLarge);
	pDataLarge = NULL;
	xpkClose(objXpk);
	remove(sPathPkgStoreHuge);

	iDataLargeSize = (9u * 1024u * 1024u) + 987u;
	pDataLarge = xpkAllocInternal(iDataLargeSize);
	if ( pDataLarge == NULL ) {
		return 1701;
	}
	memset(pDataLarge, 'Y', iDataLargeSize);
	if ( !procTestWriteBinaryFile(sPathFileExport, pDataLarge, (size_t)iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1702;
	}

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgStoreHuge, &objOpt);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1703;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddFile(objXpk, sPathFileExport, &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1704;
	}
	if ( iPosRet != 1 ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1705;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1706;
	}
	if ( objInfo.fileSize != iDataLargeSize || objInfo.dataSize >= objInfo.fileSize || (objInfo.flag & XPK_FLAG_COMP_MASK) == 0 ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1707;
	}
	if ( xpkVerify(objXpk, 1) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1708;
	}

	memset(pDataLarge, 'Z', iDataLargeSize);
	if ( !procTestWriteBinaryFile(sPathFileExport, pDataLarge, (size_t)iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1709;
	}
	if ( xpkUpdateFile(objXpk, 1, sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1710;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1711;
	}
	if ( objInfo.fileSize != iDataLargeSize || objInfo.dataSize >= objInfo.fileSize || (objInfo.flag & XPK_FLAG_COMP_MASK) == 0 ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1712;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1713;
	}
	if ( !procTestFileContentEquals(sPathFileExport, pDataLarge, iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1714;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1715;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgStoreHuge, NULL);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1716;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1717;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1718;
	}
	if ( !procTestFileContentEquals(sPathFileExport, pDataLarge, iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1719;
	}
	xpkFreeInternal(pDataLarge);
	pDataLarge = NULL;
	xpkClose(objXpk);
	remove(sPathPkgStoreHuge);

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
	hFile = xrtOpen((str)sPathPkgStoreHuge, TRUE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		xpkClose(objXpk);
		return 1132;
	}
	memset(&objMap, 0, sizeof(objMap));
	if ( procXpkMapFileReadOnly(objXpk, hFile, XPK_HEAD_SIZE + iBuildSizeAfter, &objMap) != XPK_OK ) {
		xrtClose(hFile);
		xpkClose(objXpk);
		return 1133;
	}
	pEntry->iFileHash = xpkHash32Internal((const uint8_t*)objMap.pView + XPK_HEAD_SIZE, iBuildSizeAfter);
	procXpkUnmapFile(&objMap);
	xrtClose(hFile);
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
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( !procTestExpectNullResultError(pDataRead, objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge) ) {
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_dst");
		xpkClose(objXpk);
		return (pDataRead != NULL) ? 1003 : 1004;
	}
	iRetCall = xpkVerify(objXpk, 1);
	if ( !procTestExpectCallError(iRetCall, XPK_OK, objXpk, XPK_OK, "") ) {
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_dst");
		xpkClose(objXpk);
		return (iRetCall != XPK_OK) ? 1005 : 1006;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_dst");
		xpkClose(objXpk);
		return 1134;
	}
	if ( !xrtDirExists((str)"release/x64/xpack_phase3_store_huge_dst") ) {
		procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_dst");
		xpkClose(objXpk);
		return 994;
	}
	procTestDeletePathFamily("release/x64/xpack_phase3_store_huge_dst");
	xpkClose(objXpk);
	remove(sPathPkgStoreHuge);

	iDataLargeSize = (9u * 1024u * 1024u) + 777u;
	pDataLarge = xpkAllocInternal(iDataLargeSize);
	if ( pDataLarge == NULL ) {
		return 1053;
	}
	memset(pDataLarge, 'R', iDataLargeSize);
	if ( !procTestWriteBinaryFile(sPathFileExport, pDataLarge, (size_t)iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1054;
	}

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgStoreHuge, &objOpt);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1055;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddFile(objXpk, sPathFileExport, &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1056;
	}
	if ( iPosRet != 1 ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1057;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1058;
	}
	if ( !objXpk->bDirtyData || !objXpk->bDirtyEntryTable || !objXpk->bDirtyHead ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1410;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1059;
	}
	if ( iDataSize != iDataLargeSize || memcmp(pDataRead, pDataLarge, (size_t)iDataLargeSize) != 0 ) {
		fprintf(stderr, "core_mode store immediate mismatch: read=%u expect=%u first=%02X/%02X last=%02X/%02X\n",
			(unsigned int)iDataSize,
			(unsigned int)iDataLargeSize,
			(unsigned int)((const uint8_t*)pDataRead)[0],
			(unsigned int)((const uint8_t*)pDataLarge)[0],
			(unsigned int)((const uint8_t*)pDataRead)[iDataSize - 1],
			(unsigned int)((const uint8_t*)pDataLarge)[iDataLargeSize - 1]);
		xpkFree(pDataRead);
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1060;
	}
	xpkFree(pDataRead);

	memset(pDataLarge, 'S', iDataLargeSize);
	if ( !procTestWriteBinaryFile(sPathFileExport, pDataLarge, (size_t)iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1061;
	}
	if ( xpkUpdateFile(objXpk, 1, sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1062;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1063;
	}
	if ( !objXpk->bDirtyData || !objXpk->bDirtyEntryTable || !objXpk->bDirtyHead ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1411;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1064;
	}
	if ( iDataSize != iDataLargeSize || memcmp(pDataRead, pDataLarge, (size_t)iDataLargeSize) != 0 ) {
		xpkFree(pDataRead);
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1065;
	}
	xpkFree(pDataRead);
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1412;
	}
	if ( objXpk->bDirtyData || objXpk->bDirtyEntryTable || objXpk->bDirtyHead ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1413;
	}
	xpkClose(objXpk);
	objXpk = xpkOpen(sPathPkgStoreHuge, NULL);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1414;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1415;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1416;
	}
	if ( !procTestFileContentEquals(sPathFileExport, pDataLarge, iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1417;
	}
	xpkFreeInternal(pDataLarge);
	pDataLarge = NULL;
	xpkClose(objXpk);
	remove(sPathPkgStoreHuge);

	iDataLargeSize = (256u * 1024u) + 321u;
	pDataLarge = xpkAllocInternal(iDataLargeSize);
	if ( pDataLarge == NULL ) {
		return 1102;
	}
	memset(pDataLarge, 'T', iDataLargeSize);
	if ( !procTestWriteBinaryFile(sPathFileExport, pDataLarge, (size_t)iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1103;
	}

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgStoreHuge, &objOpt);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1104;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkAddFile(objXpk, sPathFileExport, &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1105;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1106;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1107;
	}
	if ( (iDataSize != iDataLargeSize) || (memcmp(pDataRead, pDataLarge, (size_t)iDataLargeSize) != 0) ) {
		xpkFree(pDataRead);
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1108;
	}
	xpkFree(pDataRead);

	memset(pDataLarge, 'U', iDataLargeSize);
	if ( !procTestWriteBinaryFile(sPathFileExport, pDataLarge, (size_t)iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1109;
	}
	if ( xpkUpdateFile(objXpk, 1, sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1110;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1111;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1635;
	}
	if ( !procTestFileContentEquals(sPathFileExport, pDataLarge, iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1636;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1112;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgStoreHuge, NULL);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1113;
	}
	if ( xpkVerify(objXpk, 1) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1332;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1331;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1114;
	}
	if ( (iDataSize != iDataLargeSize) || (memcmp(pDataRead, pDataLarge, (size_t)iDataLargeSize) != 0) ) {
		xpkFree(pDataRead);
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1115;
	}
	xpkFree(pDataRead);
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1624;
	}
	if ( !procTestFileContentEquals(sPathFileExport, pDataLarge, iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1625;
	}
	xpkFreeInternal(pDataLarge);
	pDataLarge = NULL;
	xpkClose(objXpk);
	remove(sPathPkgStoreHuge);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgStoreHuge, &objOpt);
	if ( objXpk == NULL ) {
		return 1116;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, "KEEP-STORE", sizeof("KEEP-STORE"), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1117;
	}
	if ( !procTestCreateSparseFileSize(sPathFileHuge, (uint64_t)UINT32_MAX + 22222u) ) {
		xpkClose(objXpk);
		return 1118;
	}
	if ( xpkUpdateFile(objXpk, 1, sPathFileHuge, &objWriteOpt) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1119;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1120;
	}
	if ( objInfo.fileSize != ((uint64_t)UINT32_MAX + 22222u) || objInfo.dataSize != objInfo.fileSize || (objInfo.flag & XPK_FLAG_COMP_MASK) != 0 ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1121;
	}
	if ( xpkVerify(objXpk, 1) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1122;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1123;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1124;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, (uint64_t)UINT32_MAX + 22222u) ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1125;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1472;
	}
	xpkClose(objXpk);
	objXpk = xpkOpen(sPathPkgStoreHuge, NULL);
	if ( objXpk == NULL ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		return 1473;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1474;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1475;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, (uint64_t)UINT32_MAX + 22222u) ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1476;
	}
	procTestDeletePathFamily(sPathFileExport);
	remove(sPathFileHuge);
	xpkClose(objXpk);
	remove(sPathPkgStoreHuge);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgStoreHuge, &objOpt);
	if ( objXpk == NULL ) {
		return 1401;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( !procTestCreateSparseFileSize(sPathFileHuge, (uint64_t)UINT32_MAX + 12345u) ) {
		xpkClose(objXpk);
		return 1402;
	}
	if ( xpkAddFile(objXpk, sPathFileHuge, &objWriteOpt, &iPosRet) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1403;
	}
	if ( iPosRet != 1 || xpkCount(objXpk) != 1 ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1404;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1405;
	}
	if ( objInfo.fileSize != ((uint64_t)UINT32_MAX + 12345u) || objInfo.dataSize != objInfo.fileSize || (objInfo.flag & XPK_FLAG_COMP_MASK) != 0 ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1406;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1407;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1408;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, (uint64_t)UINT32_MAX + 12345u) ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1409;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1477;
	}
	xpkClose(objXpk);
	objXpk = xpkOpen(sPathPkgStoreHuge, NULL);
	if ( objXpk == NULL ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		return 1478;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1479;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1480;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, (uint64_t)UINT32_MAX + 12345u) ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1481;
	}
	procTestDeletePathFamily(sPathFileExport);
	remove(sPathFileHuge);
	xpkClose(objXpk);
	remove(sPathPkgStoreHuge);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgStoreHuge, &objOpt);
	if ( objXpk == NULL ) {
		return 1124;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkAddData(objXpk, "KEEP-COMP", sizeof("KEEP-COMP"), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1125;
	}
	if ( !procTestCreateSparseFileSize(sPathFileHuge, (uint64_t)UINT32_MAX + 33333u) ) {
		xpkClose(objXpk);
		return 1126;
	}
	if ( xpkUpdateFile(objXpk, 1, sPathFileHuge, &objWriteOpt) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1127;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1128;
	}
	if ( objInfo.fileSize != ((uint64_t)UINT32_MAX + 33333u) || objInfo.dataSize != objInfo.fileSize || (objInfo.flag & XPK_FLAG_COMP_MASK) != 0 ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1129;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1130;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1131;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, (uint64_t)UINT32_MAX + 33333u) ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1132;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1133;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgStoreHuge, NULL);
	if ( objXpk == NULL ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		return 1134;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1135;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1136;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, (uint64_t)UINT32_MAX + 33333u) ) {
		procTestDeletePathFamily(sPathFileExport);
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1137;
	}
	remove(sPathFileHuge);
	xpkClose(objXpk);
	remove(sPathPkgStoreHuge);
	}

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

	procTestDeletePathFamily(sPathPkgTest);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgTest, &objOpt);
	if ( objXpk == NULL ) {
		return 1452;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, NULL, 0, &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1453;
	}
	if ( iPosRet != 1 ) {
		xpkClose(objXpk);
		return 1454;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		xpkClose(objXpk);
		return 1455;
	}
	if ( (objInfo.fileHash != 0) || (objInfo.fileSize != 0) || (objInfo.dataSize != 0) ) {
		xpkClose(objXpk);
		return 1456;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1457;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgTest, NULL);
	if ( objXpk == NULL ) {
		return 1458;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1459;
	}
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1460;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, 0) ) {
		xpkClose(objXpk);
		return 1461;
	}
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgTest);

	procTestDeletePathFamily(sPathPkgTest);
	procTestDeletePathFamily(sPathFileExport);
	if ( !procTestWriteBinaryFile(sPathFileExport, "CORE-ZERO-UPDATE", sizeof("CORE-ZERO-UPDATE")) ) {
		return 1528;
	}
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgTest, &objOpt);
	if ( objXpk == NULL ) {
		return 1529;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddFile(objXpk, sPathFileExport, &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1530;
	}
	if ( !procTestWriteBinaryFile(sPathFileExport, NULL, 0) ) {
		xpkClose(objXpk);
		return 1531;
	}
	if ( xpkUpdateFile(objXpk, 1, sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 1532;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		xpkClose(objXpk);
		return 1533;
	}
	if ( (objInfo.fileHash != 0) || (objInfo.fileSize != 0) || (objInfo.dataSize != 0) ) {
		xpkClose(objXpk);
		return 1534;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1535;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgTest, NULL);
	if ( objXpk == NULL ) {
		return 1536;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1537;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1538;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, 0) ) {
		xpkClose(objXpk);
		return 1539;
	}
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgTest);
