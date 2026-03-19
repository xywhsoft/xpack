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
	if ( xpkSetSolidMode(objXpk, TRUE) != XPK_OK ) {
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
	if ( xpkPathAddData(objXpk, "assets/solid_d.txt", sDataSolidA, sizeof(sDataSolidA), &objWriteOpt) != XPK_ERR_SOLID_DATA_WRITE ) {
		xpkClose(objXpk);
		return 175;
	}
	if ( xpkPathUpdateData(objXpk, "assets/solid_c.txt", sDataSolidA, sizeof(sDataSolidA), &objWriteOpt) != XPK_ERR_SOLID_DATA_WRITE ) {
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
	if ( (objInfo.flag & XPK_FLAG_COMP_MASK) != 14 ) {
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
	xpkClose(objXpk);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgLayout, &objOpt);
