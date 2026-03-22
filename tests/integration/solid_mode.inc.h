	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgSolid, &objOpt);
	if ( objXpk == NULL ) {
		return 152;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_WIN32) != XPK_OK ) {
		xpkClose(objXpk);
		return 153;
	}

	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkPathAddData(objXpk, "assets/solid_a.txt", sDataSolidA, sizeof(sDataSolidA), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 154;
	}
	if ( xpkPathAddData(objXpk, "assets/solid_b.txt", sDataSolidB, sizeof(sDataSolidB), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 155;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 156;
	}
	iRetCall = xpkSetSolidMode(objXpk, TRUE);
	if ( iRetCall != XPK_OK ) {
		xpkClose(objXpk);
		return 157;
	}
	if ( xpkSave(objXpk) != XPK_ERR_STATE ) {
		xpkClose(objXpk);
		return 158;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 159;
	}
	if ( xpkGetSolidMode(objXpk, &iEachCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 160;
	}
	if ( iEachCount != TRUE ) {
		xpkClose(objXpk);
		return 161;
	}
	if ( xpkVerify(objXpk, 1) != XPK_OK ) {
		xpkClose(objXpk);
		return 1005;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 162;
	}
	pDataRead = xpkPathReadToMemory(objXpk, "assets/solid_a.txt", &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 163;
	}
	if ( iDataSize != sizeof(sDataSolidA) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 164;
	}
	if ( memcmp(pDataRead, sDataSolidA, sizeof(sDataSolidA)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 165;
	}
	xpkFree(pDataRead);
	if ( xpkMetaSet(objXpk, sMetaSolid, sizeof(sMetaSolid), 6) != XPK_OK ) {
		xpkClose(objXpk);
		return 166;
	}
	if ( xpkPathRename(objXpk, "assets/solid_b.txt", "assets/solid_c.txt") != XPK_OK ) {
		xpkClose(objXpk);
		return 167;
	}
	if ( xpkPathSetAttr(objXpk, "assets/solid_c.txt", 0x40u) != XPK_OK ) {
		xpkClose(objXpk);
		return 168;
	}
	if ( xpkPathRemove(objXpk, "assets/solid_a.txt") != XPK_OK ) {
		xpkClose(objXpk);
		return 169;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 170;
	}
	if ( xpkStatGet(objXpk, &objStat) != XPK_OK ) {
		xpkClose(objXpk);
		return 171;
	}
	if ( objStat.fileCount != 1 ) {
		xpkClose(objXpk);
		return 172;
	}
	if ( objStat.liveDataBytes != sizeof(sDataSolidB) ) {
		xpkClose(objXpk);
		return 173;
	}
	if ( objStat.holeBytes != sizeof(sDataSolidA) ) {
		xpkClose(objXpk);
		return 174;
	}
	iRetCall = xpkPathAddData(objXpk, "assets/solid_d.txt", sDataSolidA, sizeof(sDataSolidA), &objWriteOpt);
	if ( iRetCall != XPK_ERR_SOLID_DATA_WRITE ) {
		xpkClose(objXpk);
		return 175;
	}
	iRetCall = xpkPathUpdateData(objXpk, "assets/solid_c.txt", sDataSolidA, sizeof(sDataSolidA), &objWriteOpt);
	if ( iRetCall != XPK_ERR_SOLID_DATA_WRITE ) {
		xpkClose(objXpk);
		return 176;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgSolid, NULL);
	if ( objXpk == NULL ) {
		return 177;
	}
	if ( !xpkPathExists(objXpk, "assets/solid_c.txt") ) {
		xpkClose(objXpk);
		return 178;
	}
	if ( xpkPathExists(objXpk, "assets/solid_a.txt") ) {
		xpkClose(objXpk);
		return 179;
	}
	pMetaRead = xpkMetaGet(objXpk, &iMetaSize);
	if ( pMetaRead == NULL ) {
		xpkClose(objXpk);
		return 180;
	}
	if ( iMetaSize != sizeof(sMetaSolid) ) {
		xpkFree(pMetaRead);
		xpkClose(objXpk);
		return 181;
	}
	if ( memcmp(pMetaRead, sMetaSolid, sizeof(sMetaSolid)) != 0 ) {
		xpkFree(pMetaRead);
		xpkClose(objXpk);
		return 182;
	}
	xpkFree(pMetaRead);
	if ( xpkPathGetInfo(objXpk, "assets/solid_c.txt", &objInfoPath) != XPK_OK ) {
		xpkClose(objXpk);
		return 183;
	}
	if ( objInfoPath.platformAttr != 0x40u ) {
		xpkClose(objXpk);
		return 184;
	}
	if ( xpkStatGet(objXpk, &objStat) != XPK_OK ) {
		xpkClose(objXpk);
		return 185;
	}
	if ( objStat.fileCount != 1 ) {
		xpkClose(objXpk);
		return 186;
	}
	if ( objStat.liveDataBytes != sizeof(sDataSolidB) ) {
		xpkClose(objXpk);
		return 187;
	}
	if ( objStat.holeBytes != sizeof(sDataSolidA) ) {
		xpkClose(objXpk);
		return 188;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 189;
	}
	if ( xpkSetFlag(objXpk, 1, XPK_FLAG_COMP_MASK, 14) != XPK_OK ) {
		xpkClose(objXpk);
		return 599;
	}
	pDataRead = xpkPathReadToMemory(objXpk, "assets/solid_c.txt", &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 190;
	}
	if ( iDataSize != sizeof(sDataSolidB) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 191;
	}
	if ( memcmp(pDataRead, sDataSolidB, sizeof(sDataSolidB)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 192;
	}
	xpkFree(pDataRead);
	procTestDeletePathFamily(sPathFileExport);
	if ( !procTestWriteBinaryFile(sPathFileExport, "OLD-SOLID-EXPORT-CONTENT-TO-TRUNCATE", sizeof("OLD-SOLID-EXPORT-CONTENT-TO-TRUNCATE")) ) {
		xpkClose(objXpk);
		return 1685;
	}
	if ( xpkPathReadToFile(objXpk, "assets/solid_c.txt", sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1003;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataSolidB, sizeof(sDataSolidB)) ) {
		xpkClose(objXpk);
		return 1004;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 600;
	}
	if ( xpkSetDefaultComp(objXpk, 14) != XPK_OK ) {
		xpkClose(objXpk);
		return 560;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 561;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgSolid, NULL);
	if ( objXpk == NULL ) {
		return 562;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 563;
	}
	pDataRead = xpkPathReadToMemory(objXpk, "assets/solid_c.txt", &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 564;
	}
	if ( iDataSize != sizeof(sDataSolidB) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 565;
	}
	if ( memcmp(pDataRead, sDataSolidB, sizeof(sDataSolidB)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 566;
	}
	xpkFree(pDataRead);
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 567;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		xpkClose(objXpk);
		return 568;
	}
	if ( ((objInfo.flag & XPK_FLAG_COMP_MASK) != 14) && ((objInfo.flag & XPK_FLAG_COMP_MASK) != 0) ) {
		xpkClose(objXpk);
		return 569;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 570;
	}
	if ( xpkSetSolidMode(objXpk, FALSE) != XPK_OK ) {
		xpkClose(objXpk);
		return 193;
	}
	if ( xpkSave(objXpk) != XPK_ERR_STATE ) {
		xpkClose(objXpk);
		return 194;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 195;
	}
	if ( xpkGetSolidMode(objXpk, &iEachCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 196;
	}
	if ( iEachCount != FALSE ) {
		xpkClose(objXpk);
		return 197;
	}
	if ( xpkStatGet(objXpk, &objStat) != XPK_OK ) {
		xpkClose(objXpk);
		return 198;
	}
	if ( objStat.holeBytes != 0 ) {
		xpkClose(objXpk);
		return 199;
	}
	if ( xpkPathUpdateData(objXpk, "assets/solid_c.txt", sDataSolidUpdate, sizeof(sDataSolidUpdate), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 200;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 201;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgSolid, NULL);
	if ( objXpk == NULL ) {
		return 202;
	}
	pDataRead = xpkPathReadToMemory(objXpk, "assets/solid_c.txt", &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 203;
	}
	if ( iDataSize != sizeof(sDataSolidUpdate) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 204;
	}
	if ( memcmp(pDataRead, sDataSolidUpdate, sizeof(sDataSolidUpdate)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 205;
	}
	xpkFree(pDataRead);
	xpkClose(objXpk);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgSolid, &objOpt);
	if ( objXpk == NULL ) {
		return 389;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_WIN32) != XPK_OK ) {
		xpkClose(objXpk);
		return 390;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkPathAddData(objXpk, "assets/pending_a.txt", sDataSolidA, sizeof(sDataSolidA), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 391;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 392;
	}
	if ( xpkSetSolidMode(objXpk, TRUE) != XPK_OK ) {
		xpkClose(objXpk);
		return 393;
	}
	if ( xpkPathAddData(objXpk, "assets/pending_b.txt", sDataSolidB, sizeof(sDataSolidB), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 394;
	}
	if ( xpkPathUpdateData(objXpk, "assets/pending_a.txt", sDataSolidUpdate, sizeof(sDataSolidUpdate), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 395;
	}
	if ( xpkSave(objXpk) != XPK_ERR_STATE ) {
		xpkClose(objXpk);
		return 396;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 397;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 398;
	}
	pDataRead = xpkPathReadToMemory(objXpk, "assets/pending_a.txt", &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 399;
	}
	if ( iDataSize != sizeof(sDataSolidUpdate) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 400;
	}
	if ( memcmp(pDataRead, sDataSolidUpdate, sizeof(sDataSolidUpdate)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 401;
	}
	xpkFree(pDataRead);
	pDataRead = xpkPathReadToMemory(objXpk, "assets/pending_b.txt", &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 402;
	}
	if ( iDataSize != sizeof(sDataSolidB) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 403;
	}
	if ( memcmp(pDataRead, sDataSolidB, sizeof(sDataSolidB)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 404;
	}
	xpkFree(pDataRead);
	xpkClose(objXpk);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgSolidStore, &objOpt);
	if ( objXpk == NULL ) {
		return 338;
	}
	if ( xpkSetDefaultComp(objXpk, 15) != XPK_OK ) {
		xpkClose(objXpk);
		return 339;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, sDataSolidStoreA, sizeof(sDataSolidStoreA), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 340;
	}
	if ( xpkAddData(objXpk, sDataSolidStoreB, sizeof(sDataSolidStoreB), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 341;
	}
	if ( xpkSetSolidMode(objXpk, TRUE) != XPK_OK ) {
		xpkClose(objXpk);
		return 342;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 343;
	}
	if ( xpkGetDefaultComp(objXpk, &objWriteOpt.compLevel) != XPK_OK ) {
		xpkClose(objXpk);
		return 344;
	}
	if ( objWriteOpt.compLevel != 0 ) {
		xpkClose(objXpk);
		return 345;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		xpkClose(objXpk);
		return 346;
	}
	if ( (objInfo.flag & XPK_FLAG_COMP_MASK) != 0 ) {
		xpkClose(objXpk);
		return 347;
	}
	if ( xpkGetInfo(objXpk, 2, &objInfo) != XPK_OK ) {
		xpkClose(objXpk);
		return 348;
	}
	if ( (objInfo.flag & XPK_FLAG_COMP_MASK) != 0 ) {
		xpkClose(objXpk);
		return 349;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 1013;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1014;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 1015;
	}
	if ( iDataSize != sizeof(sDataSolidStoreA) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1016;
	}
	if ( memcmp(pDataRead, sDataSolidStoreA, sizeof(sDataSolidStoreA)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1017;
	}
	xpkFree(pDataRead);
	pDataRead = xpkReadToMemory(objXpk, 2, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 1018;
	}
	if ( iDataSize != sizeof(sDataSolidStoreB) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1019;
	}
	if ( memcmp(pDataRead, sDataSolidStoreB, sizeof(sDataSolidStoreB)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1020;
	}
	xpkFree(pDataRead);
	if ( xpkSetDefaultComp(objXpk, 14) != XPK_OK ) {
		xpkClose(objXpk);
		return 1021;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 1022;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1023;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		xpkClose(objXpk);
		return 1024;
	}
	if ( ((objInfo.flag & XPK_FLAG_COMP_MASK) != 14) && ((objInfo.flag & XPK_FLAG_COMP_MASK) != 0) ) {
		xpkClose(objXpk);
		return 1025;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 1026;
	}
	if ( iDataSize != sizeof(sDataSolidStoreA) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1027;
	}
	if ( memcmp(pDataRead, sDataSolidStoreA, sizeof(sDataSolidStoreA)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1028;
	}
	xpkFree(pDataRead);
	pDataRead = xpkReadToMemory(objXpk, 2, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 1029;
	}
	if ( iDataSize != sizeof(sDataSolidStoreB) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1030;
	}
	if ( memcmp(pDataRead, sDataSolidStoreB, sizeof(sDataSolidStoreB)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1031;
	}
	xpkFree(pDataRead);
	if ( xpkSetDefaultComp(objXpk, 2) != XPK_OK ) {
		xpkClose(objXpk);
		return 1671;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 1672;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1673;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		xpkClose(objXpk);
		return 1674;
	}
	if ( ((objInfo.flag & XPK_FLAG_COMP_MASK) != 2) && ((objInfo.flag & XPK_FLAG_COMP_MASK) != 0) ) {
		xpkClose(objXpk);
		return 1675;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 1676;
	}
	if ( iDataSize != sizeof(sDataSolidStoreA) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1677;
	}
	if ( memcmp(pDataRead, sDataSolidStoreA, sizeof(sDataSolidStoreA)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1678;
	}
	xpkFree(pDataRead);
	pDataRead = xpkReadToMemory(objXpk, 2, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 1679;
	}
	if ( iDataSize != sizeof(sDataSolidStoreB) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1680;
	}
	if ( memcmp(pDataRead, sDataSolidStoreB, sizeof(sDataSolidStoreB)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1681;
	}
	xpkFree(pDataRead);
	if ( xpkVerify(objXpk, 1) != XPK_OK ) {
		xpkClose(objXpk);
		return 1682;
	}
	if ( xpkReadToFile(objXpk, 2, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1683;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataSolidStoreB, sizeof(sDataSolidStoreB)) ) {
		xpkClose(objXpk);
		return 1684;
	}
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgSolid);

	procTestDeletePathFamily(sPathPkgSolid);
	procTestDeletePathFamily(sPathPkgSolid);
	procTestDeletePathFamily(sPathPkgSolidStore);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgSolid, &objOpt);
	if ( objXpk == NULL ) {
		return 1195;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_WIN32) != XPK_OK ) {
		xpkClose(objXpk);
		return 1196;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	iEachCount = xpkPathAddData(objXpk, "assets/single_comp.txt", sDataSolidStoreA, sizeof(sDataSolidStoreA), &objWriteOpt);
	if ( iEachCount != XPK_OK ) {
		xpkClose(objXpk);
		return 1197;
	}
	if ( xpkSetSolidMode(objXpk, TRUE) != XPK_OK ) {
		xpkClose(objXpk);
		return 1198;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 1199;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1200;
	}
	if ( xpkSetSolidMode(objXpk, FALSE) != XPK_OK ) {
		xpkClose(objXpk);
		return 1201;
	}
	if ( xpkSave(objXpk) != XPK_ERR_STATE ) {
		xpkClose(objXpk);
		return 1202;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 1203;
	}
	if ( xpkGetSolidMode(objXpk, &iEachCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 1204;
	}
	if ( iEachCount != FALSE ) {
		xpkClose(objXpk);
		return 1205;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1206;
	}
	if ( !xpkPathExists(objXpk, "assets/single_comp.txt") ) {
		xpkClose(objXpk);
		return 1207;
	}
	if ( xpkPathGetInfo(objXpk, "assets/single_comp.txt", &objInfoPath) != XPK_OK ) {
		xpkClose(objXpk);
		return 1208;
	}
	if ( ((objInfoPath.flag & XPK_FLAG_COMP_MASK) != 6) && ((objInfoPath.flag & XPK_FLAG_COMP_MASK) != 0) ) {
		xpkClose(objXpk);
		return 1209;
	}
	pDataRead = xpkPathReadToMemory(objXpk, "assets/single_comp.txt", &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 1210;
	}
	if ( iDataSize != sizeof(sDataSolidStoreA) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1211;
	}
	if ( memcmp(pDataRead, sDataSolidStoreA, sizeof(sDataSolidStoreA)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1212;
	}
	xpkFree(pDataRead);
	if ( xpkPathReadToFile(objXpk, "assets/single_comp.txt", sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1213;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataSolidStoreA, sizeof(sDataSolidStoreA)) ) {
		xpkClose(objXpk);
		return 1214;
	}
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgSolid);

	procTestDeletePathFamily(sPathPkgSolid);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgSolid, &objOpt);
	if ( objXpk == NULL ) {
		return 1215;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_WIN32) != XPK_OK ) {
		xpkClose(objXpk);
		return 1216;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkPathAddData(objXpk, "assets/single_to_solid.txt", sDataSolidStoreA, sizeof(sDataSolidStoreA), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 1217;
	}
	if ( xpkSetSolidMode(objXpk, TRUE) != XPK_OK ) {
		xpkClose(objXpk);
		return 1218;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 1219;
	}
	if ( xpkGetSolidMode(objXpk, &iEachCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 1220;
	}
	if ( iEachCount != TRUE ) {
		xpkClose(objXpk);
		return 1221;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1222;
	}
	if ( xpkPathGetInfo(objXpk, "assets/single_to_solid.txt", &objInfoPath) != XPK_OK ) {
		xpkClose(objXpk);
		return 1223;
	}
	if ( ((objInfoPath.flag & XPK_FLAG_COMP_MASK) != 6) && ((objInfoPath.flag & XPK_FLAG_COMP_MASK) != 0) ) {
		xpkClose(objXpk);
		return 1224;
	}
	pDataRead = xpkPathReadToMemory(objXpk, "assets/single_to_solid.txt", &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 1225;
	}
	if ( iDataSize != sizeof(sDataSolidStoreA) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1226;
	}
	if ( memcmp(pDataRead, sDataSolidStoreA, sizeof(sDataSolidStoreA)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1227;
	}
	xpkFree(pDataRead);
	if ( xpkPathReadToFile(objXpk, "assets/single_to_solid.txt", sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1228;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataSolidStoreA, sizeof(sDataSolidStoreA)) ) {
		xpkClose(objXpk);
		return 1229;
	}
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgSolid);
	procTestDeletePathFamily(sPathPkgSolidStore);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgSolidStore, &objOpt);
	if ( objXpk == NULL ) {
		return 1199;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, sDataSolidStoreA, sizeof(sDataSolidStoreA), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1200;
	}
	if ( xpkSetSolidMode(objXpk, TRUE) != XPK_OK ) {
		xpkClose(objXpk);
		return 1201;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 1202;
	}
	pEntry = procXpkGetEntryByPos(objXpk, 1);
	if ( pEntry == NULL ) {
		xpkClose(objXpk);
		return 1203;
	}
	iBuildSizeAfter = (uint64_t)UINT32_MAX + 88888u;
	if ( !procTestCreateSparseFileSize(sPathPkgSolidStore, XPK_HEAD_SIZE + iBuildSizeAfter) ) {
		xpkClose(objXpk);
		return 1204;
	}
	hFile = xrtOpen((str)sPathPkgSolidStore, TRUE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		xpkClose(objXpk);
		return 1205;
	}
	memset(&objMap, 0, sizeof(objMap));
	if ( procXpkMapFileReadOnly(objXpk, hFile, XPK_HEAD_SIZE + iBuildSizeAfter, &objMap) != XPK_OK ) {
		xrtClose(hFile);
		xpkClose(objXpk);
		return 1206;
	}
	pEntry->iFileHash = xpkHash32Internal((const uint8_t*)objMap.pView + XPK_HEAD_SIZE, iBuildSizeAfter);
	procXpkUnmapFile(&objMap);
	xrtClose(hFile);
	objXpk->objHead.dataOffset = XPK_HEAD_SIZE + iBuildSizeAfter;
	pEntry->iFlag &= ~XPK_FLAG_COMP_MASK;
	pEntry->iDataOffset = 0;
	pEntry->iDataSize = iBuildSizeAfter;
	pEntry->iFileSize = iBuildSizeAfter;
	if ( xpkVerify(objXpk, 1) != XPK_OK ) {
		xpkClose(objXpk);
		return 1207;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1208;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( !procTestExpectNullResultError(pDataRead, objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge) ) {
		if ( pDataRead != NULL ) {
			xpkFree(pDataRead);
			xpkClose(objXpk);
			return 1209;
		}
		xpkClose(objXpk);
		return 1210;
	}
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgSolidStore);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgSolidStore, &objOpt);
	if ( objXpk == NULL ) {
		return 1188;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, sDataSolidStoreA, sizeof(sDataSolidStoreA), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1189;
	}
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkAddData(objXpk, sDataSolidStoreB, sizeof(sDataSolidStoreB), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1190;
	}
	if ( xpkSetDefaultComp(objXpk, 0) != XPK_OK ) {
		xpkClose(objXpk);
		return 1191;
	}
	if ( xpkSetSolidMode(objXpk, TRUE) != XPK_OK ) {
		xpkClose(objXpk);
		return 1192;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 1193;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1194;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 1195;
	}
	if ( (iDataSize != sizeof(sDataSolidStoreA)) || (memcmp(pDataRead, sDataSolidStoreA, sizeof(sDataSolidStoreA)) != 0) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1196;
	}
	xpkFree(pDataRead);
	pDataRead = xpkReadToMemory(objXpk, 2, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 1197;
	}
	if ( (iDataSize != sizeof(sDataSolidStoreB)) || (memcmp(pDataRead, sDataSolidStoreB, sizeof(sDataSolidStoreB)) != 0) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1198;
	}
	xpkFree(pDataRead);
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgSolidStore);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgSolidStore, &objOpt);
	if ( objXpk == NULL ) {
		return 1175;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, sDataSolidStoreA, sizeof(sDataSolidStoreA), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1176;
	}
	if ( xpkAddData(objXpk, sDataSolidStoreB, sizeof(sDataSolidStoreB), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1177;
	}
	if ( xpkSetDefaultComp(objXpk, 0) != XPK_OK ) {
		xpkClose(objXpk);
		return 1178;
	}
	if ( xpkSetSolidMode(objXpk, TRUE) != XPK_OK ) {
		xpkClose(objXpk);
		return 1179;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 1180;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1181;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 1182;
	}
	if ( iDataSize != sizeof(sDataSolidStoreA) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1183;
	}
	if ( memcmp(pDataRead, sDataSolidStoreA, sizeof(sDataSolidStoreA)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1184;
	}
	xpkFree(pDataRead);
	pDataRead = xpkReadToMemory(objXpk, 2, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 1185;
	}
	if ( iDataSize != sizeof(sDataSolidStoreB) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1186;
	}
	if ( memcmp(pDataRead, sDataSolidStoreB, sizeof(sDataSolidStoreB)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1187;
	}
	xpkFree(pDataRead);
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgSolidStore);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgSolidStore, &objOpt);
	if ( objXpk == NULL ) {
		return 1032;
	}
	if ( xpkSetDefaultComp(objXpk, 14) != XPK_OK ) {
		xpkClose(objXpk);
		return 1033;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, sDataSolidStoreA, sizeof(sDataSolidStoreA), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1034;
	}
	if ( xpkSetSolidMode(objXpk, TRUE) != XPK_OK ) {
		xpkClose(objXpk);
		return 1035;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 1036;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 1037;
	}
	if ( iDataSize != sizeof(sDataSolidStoreA) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1038;
	}
	if ( memcmp(pDataRead, sDataSolidStoreA, sizeof(sDataSolidStoreA)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1039;
	}
	xpkFree(pDataRead);
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1040;
	}
	if ( xpkVerify(objXpk, 1) != XPK_OK ) {
		xpkClose(objXpk);
		return 1653;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1654;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataSolidStoreA, sizeof(sDataSolidStoreA)) ) {
		xpkClose(objXpk);
		return 1655;
	}
	if ( xpkSetDefaultComp(objXpk, 0) != XPK_OK ) {
		xpkClose(objXpk);
		return 1146;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 1147;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		xpkClose(objXpk);
		return 1148;
	}
	if ( (objInfo.flag & XPK_FLAG_COMP_MASK) != 0 ) {
		xpkClose(objXpk);
		return 1149;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1150;
	}
	if ( xpkVerify(objXpk, 1) != XPK_OK ) {
		xpkClose(objXpk);
		return 1154;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 1151;
	}
	if ( iDataSize != sizeof(sDataSolidStoreA) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1152;
	}
	if ( memcmp(pDataRead, sDataSolidStoreA, sizeof(sDataSolidStoreA)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1153;
	}
	xpkFree(pDataRead);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1155;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataSolidStoreA, sizeof(sDataSolidStoreA)) ) {
		xpkClose(objXpk);
		return 1156;
	}
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgSolidStore);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgSolidStore, &objOpt);
	if ( objXpk == NULL ) {
		return 1157;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, sDataSolidStoreA, sizeof(sDataSolidStoreA), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1158;
	}
	if ( xpkAddData(objXpk, sDataSolidStoreB, sizeof(sDataSolidStoreB), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1159;
	}
	if ( xpkSetSolidMode(objXpk, TRUE) != XPK_OK ) {
		xpkClose(objXpk);
		return 1160;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 1161;
	}
	if ( xpkRemove(objXpk, 1) != XPK_OK ) {
		xpkClose(objXpk);
		return 1162;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1163;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1164;
	}
	pDataRead = xpkReadToMemory(objXpk, 2, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 1165;
	}
	if ( iDataSize != sizeof(sDataSolidStoreB) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1166;
	}
	if ( memcmp(pDataRead, sDataSolidStoreB, sizeof(sDataSolidStoreB)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1167;
	}
	xpkFree(pDataRead);
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 1168;
	}
	if ( xpkStatGet(objXpk, &objStat) != XPK_OK ) {
		xpkClose(objXpk);
		return 1169;
	}
	if ( objStat.holeBytes != 0 ) {
		xpkClose(objXpk);
		return 1170;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1171;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 1172;
	}
	if ( iDataSize != sizeof(sDataSolidStoreB) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1173;
	}
	if ( memcmp(pDataRead, sDataSolidStoreB, sizeof(sDataSolidStoreB)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1174;
	}
	xpkFree(pDataRead);
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgSolidStore);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgSolidStore, &objOpt);
	if ( objXpk == NULL ) {
		return 1175;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, sDataSolidStoreA, sizeof(sDataSolidStoreA), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1176;
	}
	if ( xpkAddData(objXpk, sDataSolidStoreB, sizeof(sDataSolidStoreB), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1177;
	}
	if ( xpkSetSolidMode(objXpk, TRUE) != XPK_OK ) {
		xpkClose(objXpk);
		return 1178;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 1179;
	}
	if ( xpkRemove(objXpk, 1) != XPK_OK ) {
		xpkClose(objXpk);
		return 1180;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1181;
	}
	if ( xpkSetSolidMode(objXpk, FALSE) != XPK_OK ) {
		xpkClose(objXpk);
		return 1182;
	}
	if ( xpkSave(objXpk) != XPK_ERR_STATE ) {
		xpkClose(objXpk);
		return 1183;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 1184;
	}
	if ( xpkGetSolidMode(objXpk, &iEachCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 1185;
	}
	if ( iEachCount != FALSE ) {
		xpkClose(objXpk);
		return 1186;
	}
	if ( xpkStatGet(objXpk, &objStat) != XPK_OK ) {
		xpkClose(objXpk);
		return 1187;
	}
	if ( objStat.holeBytes != 0 ) {
		xpkClose(objXpk);
		return 1188;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1189;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 1190;
	}
	if ( iDataSize != sizeof(sDataSolidStoreB) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1191;
	}
	if ( memcmp(pDataRead, sDataSolidStoreB, sizeof(sDataSolidStoreB)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1192;
	}
	xpkFree(pDataRead);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1193;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataSolidStoreB, sizeof(sDataSolidStoreB)) ) {
		xpkClose(objXpk);
		return 1194;
	}
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgSolidStore);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgSolidStore, &objOpt);
	if ( objXpk == NULL ) {
		return 1041;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkAddData(objXpk, sDataSolidStoreA, sizeof(sDataSolidStoreA), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1042;
	}
	if ( xpkAddData(objXpk, sDataSolidStoreB, sizeof(sDataSolidStoreB), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1043;
	}
	if ( xpkSetSolidMode(objXpk, TRUE) != XPK_OK ) {
		xpkClose(objXpk);
		return 1044;
	}
	iEachCount = xpkBuild(objXpk, NULL);
	if ( iEachCount != XPK_OK ) {
		xpkClose(objXpk);
		return 1045 - iEachCount;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1046;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 1047;
	}
	if ( iDataSize != sizeof(sDataSolidStoreA) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1048;
	}
	if ( memcmp(pDataRead, sDataSolidStoreA, sizeof(sDataSolidStoreA)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1049;
	}
	xpkFree(pDataRead);
	pDataRead = xpkReadToMemory(objXpk, 2, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 1050;
	}
	if ( iDataSize != sizeof(sDataSolidStoreB) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1051;
	}
	if ( memcmp(pDataRead, sDataSolidStoreB, sizeof(sDataSolidStoreB)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1052;
	}
	xpkFree(pDataRead);
	xpkClose(objXpk);

	{
		const char* sPathPkgSolidLarge;
		xpkObject objXpkLarge;
		void* pDataSolidLargeA;
		void* pDataSolidLargeB;
		uint32_t iDataSolidLargeA;
		uint32_t iDataSolidLargeB;

		sPathPkgSolidLarge = "release/x64/xpack_phase3_solid_large.xpk";
		objXpkLarge = NULL;
		pDataSolidLargeA = NULL;
		pDataSolidLargeB = NULL;
		iDataSolidLargeA = (2u * 1024u * 1024u) + 321u;
		iDataSolidLargeB = (3u * 1024u * 1024u) + 123u;

		procTestDeletePathFamily(sPathPkgSolidLarge);
		pDataSolidLargeA = xpkAllocInternal(iDataSolidLargeA);
		if ( pDataSolidLargeA == NULL ) {
			return 2177;
		}
		pDataSolidLargeB = xpkAllocInternal(iDataSolidLargeB);
		if ( pDataSolidLargeB == NULL ) {
			xpkFreeInternal(pDataSolidLargeA);
			return 2178;
		}
		memset(pDataSolidLargeA, 'S', iDataSolidLargeA);
		memset(pDataSolidLargeB, 'T', iDataSolidLargeB);

		memset(&objOpt, 0, sizeof(objOpt));
		objOpt.createIfMissing = TRUE;
		objXpkLarge = xpkOpen(sPathPkgSolidLarge, &objOpt);
		if ( objXpkLarge == NULL ) {
			xpkFreeInternal(pDataSolidLargeA);
			xpkFreeInternal(pDataSolidLargeB);
			return 2179;
		}
		if ( xpkSetPackType(objXpkLarge, XPK_PACK_WIN32) != XPK_OK ) {
			xpkFreeInternal(pDataSolidLargeA);
			xpkFreeInternal(pDataSolidLargeB);
			xpkClose(objXpkLarge);
			return 2180;
		}

		memset(&objWriteOpt, 0, sizeof(objWriteOpt));
		objWriteOpt.compLevel = 0;
		objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
		if ( xpkPathAddData(objXpkLarge, "assets/solid_large_a.bin", pDataSolidLargeA, iDataSolidLargeA, &objWriteOpt) != XPK_OK ) {
			xpkFreeInternal(pDataSolidLargeA);
			xpkFreeInternal(pDataSolidLargeB);
			xpkClose(objXpkLarge);
			return 2181;
		}
		if ( xpkPathAddData(objXpkLarge, "assets/solid_large_b.bin", pDataSolidLargeB, iDataSolidLargeB, &objWriteOpt) != XPK_OK ) {
			xpkFreeInternal(pDataSolidLargeA);
			xpkFreeInternal(pDataSolidLargeB);
			xpkClose(objXpkLarge);
			return 2182;
		}
		if ( xpkSave(objXpkLarge) != XPK_OK ) {
			xpkFreeInternal(pDataSolidLargeA);
			xpkFreeInternal(pDataSolidLargeB);
			xpkClose(objXpkLarge);
			return 2183;
		}
		if ( xpkSetSolidMode(objXpkLarge, TRUE) != XPK_OK ) {
			xpkFreeInternal(pDataSolidLargeA);
			xpkFreeInternal(pDataSolidLargeB);
			xpkClose(objXpkLarge);
			return 2184;
		}
		if ( xpkBuild(objXpkLarge, NULL) != XPK_OK ) {
			xpkFreeInternal(pDataSolidLargeA);
			xpkFreeInternal(pDataSolidLargeB);
			xpkClose(objXpkLarge);
			return 2185;
		}
		if ( xpkVerifyAll(objXpkLarge) != XPK_OK ) {
			xpkFreeInternal(pDataSolidLargeA);
			xpkFreeInternal(pDataSolidLargeB);
			xpkClose(objXpkLarge);
			return 2186;
		}
		procTestDeletePathFamily(sPathFileExport);
		if ( xpkPathReadToFile(objXpkLarge, "assets/solid_large_a.bin", sPathFileExport) != XPK_OK ) {
			xpkFreeInternal(pDataSolidLargeA);
			xpkFreeInternal(pDataSolidLargeB);
			xpkClose(objXpkLarge);
			return 2187;
		}
		if ( !procTestFileContentEquals(sPathFileExport, pDataSolidLargeA, iDataSolidLargeA) ) {
			xpkFreeInternal(pDataSolidLargeA);
			xpkFreeInternal(pDataSolidLargeB);
			xpkClose(objXpkLarge);
			return 2188;
		}

		if ( xpkSetDefaultComp(objXpkLarge, 14) != XPK_OK ) {
			xpkFreeInternal(pDataSolidLargeA);
			xpkFreeInternal(pDataSolidLargeB);
			xpkClose(objXpkLarge);
			return 2189;
		}
		if ( xpkBuild(objXpkLarge, NULL) != XPK_OK ) {
			xpkFreeInternal(pDataSolidLargeA);
			xpkFreeInternal(pDataSolidLargeB);
			xpkClose(objXpkLarge);
			return 2190;
		}
		if ( xpkVerifyAll(objXpkLarge) != XPK_OK ) {
			xpkFreeInternal(pDataSolidLargeA);
			xpkFreeInternal(pDataSolidLargeB);
			xpkClose(objXpkLarge);
			return 2191;
		}
		procTestDeletePathFamily(sPathFileExport);
		if ( xpkPathReadToFile(objXpkLarge, "assets/solid_large_b.bin", sPathFileExport) != XPK_OK ) {
			xpkFreeInternal(pDataSolidLargeA);
			xpkFreeInternal(pDataSolidLargeB);
			xpkClose(objXpkLarge);
			return 2192;
		}
		if ( !procTestFileContentEquals(sPathFileExport, pDataSolidLargeB, iDataSolidLargeB) ) {
			xpkFreeInternal(pDataSolidLargeA);
			xpkFreeInternal(pDataSolidLargeB);
			xpkClose(objXpkLarge);
			return 2193;
		}
		if ( xpkSetDefaultComp(objXpkLarge, 2) != XPK_OK ) {
			xpkFreeInternal(pDataSolidLargeA);
			xpkFreeInternal(pDataSolidLargeB);
			xpkClose(objXpkLarge);
			return 2194;
		}
		if ( xpkBuild(objXpkLarge, NULL) != XPK_OK ) {
			xpkFreeInternal(pDataSolidLargeA);
			xpkFreeInternal(pDataSolidLargeB);
			xpkClose(objXpkLarge);
			return 2195;
		}
		if ( xpkVerifyAll(objXpkLarge) != XPK_OK ) {
			xpkFreeInternal(pDataSolidLargeA);
			xpkFreeInternal(pDataSolidLargeB);
			xpkClose(objXpkLarge);
			return 2196;
		}
		procTestDeletePathFamily(sPathFileExport);
		if ( xpkPathReadToFile(objXpkLarge, "assets/solid_large_a.bin", sPathFileExport) != XPK_OK ) {
			xpkFreeInternal(pDataSolidLargeA);
			xpkFreeInternal(pDataSolidLargeB);
			xpkClose(objXpkLarge);
			return 2197;
		}
		if ( !procTestFileContentEquals(sPathFileExport, pDataSolidLargeA, iDataSolidLargeA) ) {
			xpkFreeInternal(pDataSolidLargeA);
			xpkFreeInternal(pDataSolidLargeB);
			xpkClose(objXpkLarge);
			return 2198;
		}

		xpkFreeInternal(pDataSolidLargeA);
		xpkFreeInternal(pDataSolidLargeB);
		xpkClose(objXpkLarge);
		procTestDeletePathFamily(sPathPkgSolidLarge);
	}

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgLayout, &objOpt);
