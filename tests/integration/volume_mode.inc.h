	objXpk = xpkOpen(sPathPkgVolume, &objOpt);
	if ( objXpk == NULL ) {
		return 117;
	}
	if ( xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN) != XPK_OK ) {
		xpkClose(objXpk);
		return 118;
	}

	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, sDataVolumeA, sizeof(sDataVolumeA), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 119;
	}
	if ( iPosRet != 1 ) {
		xpkClose(objXpk);
		return 120;
	}
	if ( xpkAddData(objXpk, sDataVolumeB, sizeof(sDataVolumeB), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 121;
	}
	if ( iPosRet != 2 ) {
		xpkClose(objXpk);
		return 122;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 123;
	}
	xpkClose(objXpk);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgRaw64, &objOpt);
	if ( objXpk == NULL ) {
		return 976;
	}
	if ( xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN) != XPK_OK ) {
		xpkClose(objXpk);
		return 977;
	}
	if ( !objXpk->bVolumeApplied || procXpkAppliedVolumeSize(objXpk) != XPK_VOLUME_MIN ) {
		xpkClose(objXpk);
		return 978;
	}
	iBuildSizeBefore = ((uint64_t)XPK_VOLUME_MIN * 65536u) + 123u;
	if ( procXpkRawWrite(objXpk, iBuildSizeBefore, "SEEK64", 6) != XPK_OK ) {
		xpkClose(objXpk);
		return 979;
	}
	memset(arrInfoExtCoreRead, 0, sizeof(arrInfoExtCoreRead));
	if ( procXpkRawRead(objXpk, iBuildSizeBefore, arrInfoExtCoreRead, 6) != XPK_OK ) {
		xpkClose(objXpk);
		return 980;
	}
	if ( memcmp(arrInfoExtCoreRead, "SEEK64", 6) != 0 ) {
		xpkClose(objXpk);
		return 981;
	}
	sPathVolume = procXpkVolumePathDupText(sPathPkgRaw64, 65536u);
	if ( sPathVolume == NULL ) {
		xpkClose(objXpk);
		return 982;
	}
	if ( !xrtFileExists((str)sPathVolume) ) {
		xpkFreeInternal(sPathVolume);
		xpkClose(objXpk);
		return 983;
	}
	xpkFreeInternal(sPathVolume);
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgRaw64);

	procTestDeletePathFamily(sPathPkgVolumeLarge);
	procTestDeletePathFamily(sPathFileExport);
	if ( !procTestWriteBinaryFile(sPathFileExport, NULL, 0) ) {
		return 1440;
	}
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgVolumeLarge, &objOpt);
	if ( objXpk == NULL ) {
		return 1441;
	}
	if ( xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN) != XPK_OK ) {
		xpkClose(objXpk);
		return 1442;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddFile(objXpk, sPathFileExport, &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1443;
	}
	if ( iPosRet != 1 ) {
		xpkClose(objXpk);
		return 1444;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1445;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgVolumeLarge, NULL);
	if ( objXpk == NULL ) {
		return 1446;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		xpkClose(objXpk);
		return 1447;
	}
	if ( (objInfo.fileHash != 0) || (objInfo.fileSize != 0) || (objInfo.dataSize != 0) ) {
		xpkClose(objXpk);
		return 1448;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1449;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1450;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, 0) ) {
		xpkClose(objXpk);
		return 1451;
	}
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgVolumeLarge);

	procTestDeletePathFamily(sPathPkgVolumeLarge);
	procTestDeletePathFamily(sPathFileExport);
	if ( !procTestWriteBinaryFile(sPathFileExport, "VOL-CORE-ZERO", sizeof("VOL-CORE-ZERO")) ) {
		return 1566;
	}
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgVolumeLarge, &objOpt);
	if ( objXpk == NULL ) {
		return 1567;
	}
	if ( xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN) != XPK_OK ) {
		xpkClose(objXpk);
		return 1568;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddFile(objXpk, sPathFileExport, &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1569;
	}
	if ( !procTestWriteBinaryFile(sPathFileExport, NULL, 0) ) {
		xpkClose(objXpk);
		return 1570;
	}
	if ( xpkUpdateFile(objXpk, 1, sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 1571;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		xpkClose(objXpk);
		return 1572;
	}
	if ( (objInfo.fileHash != 0) || (objInfo.fileSize != 0) || (objInfo.dataSize != 0) ) {
		xpkClose(objXpk);
		return 1573;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1574;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgVolumeLarge, NULL);
	if ( objXpk == NULL ) {
		return 1575;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1576;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1577;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, 0) ) {
		xpkClose(objXpk);
		return 1578;
	}
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgVolumeLarge);

	procTestDeletePathFamily(sPathPkgVolumeLarge);
	procTestDeletePathFamily(sPathFileExport);
	if ( !procTestWriteBinaryFile(sPathFileExport, NULL, 0) ) {
		return 1504;
	}
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgVolumeLarge, &objOpt);
	if ( objXpk == NULL ) {
		return 1505;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_WIN32) != XPK_OK ) {
		xpkClose(objXpk);
		return 1506;
	}
	if ( xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN) != XPK_OK ) {
		xpkClose(objXpk);
		return 1507;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkPathAddFile(objXpk, "assets/empty_volume.bin", sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 1508;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1509;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgVolumeLarge, NULL);
	if ( objXpk == NULL ) {
		return 1510;
	}
	if ( xpkPathGetInfo(objXpk, "ASSETS\\EMPTY_VOLUME.BIN", &objInfoPath) != XPK_OK ) {
		xpkClose(objXpk);
		return 1511;
	}
	if ( (objInfoPath.fileHash != 0) || (objInfoPath.fileSize != 0) || (objInfoPath.dataSize != 0) ) {
		xpkClose(objXpk);
		return 1512;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1513;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkPathReadToFile(objXpk, ".\\assets\\\\empty_volume.bin", sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1514;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, 0) ) {
		xpkClose(objXpk);
		return 1515;
	}
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgVolumeLarge);

	procTestDeletePathFamily(sPathPkgVolumeLarge);
	procTestDeletePathFamily(sPathFileExport);
	if ( !procTestWriteBinaryFile(sPathFileExport, "VOL-PATH-ZERO", sizeof("VOL-PATH-ZERO")) ) {
		return 1579;
	}
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgVolumeLarge, &objOpt);
	if ( objXpk == NULL ) {
		return 1580;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_WIN32) != XPK_OK ) {
		xpkClose(objXpk);
		return 1581;
	}
	if ( xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN) != XPK_OK ) {
		xpkClose(objXpk);
		return 1582;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkPathAddFile(objXpk, "assets/empty_volume_update.bin", sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 1583;
	}
	if ( !procTestWriteBinaryFile(sPathFileExport, NULL, 0) ) {
		xpkClose(objXpk);
		return 1584;
	}
	if ( xpkPathUpdateFile(objXpk, "ASSETS\\EMPTY_VOLUME_UPDATE.BIN", sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 1585;
	}
	if ( xpkPathGetInfo(objXpk, ".\\assets\\\\empty_volume_update.bin", &objInfoPath) != XPK_OK ) {
		xpkClose(objXpk);
		return 1586;
	}
	if ( (objInfoPath.fileHash != 0) || (objInfoPath.fileSize != 0) || (objInfoPath.dataSize != 0) ) {
		xpkClose(objXpk);
		return 1587;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1588;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgVolumeLarge, NULL);
	if ( objXpk == NULL ) {
		return 1589;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1590;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkPathReadToFile(objXpk, "assets/empty_volume_update.bin", sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1591;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, 0) ) {
		xpkClose(objXpk);
		return 1592;
	}
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgVolumeLarge);

	procTestDeletePathFamily(sPathPkgVolumeLarge);
	procTestDeletePathFamily(sPathFileExport);
	if ( !procTestWriteBinaryFile(sPathFileExport, NULL, 0) ) {
		return 1516;
	}
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgVolumeLarge, &objOpt);
	if ( objXpk == NULL ) {
		return 1517;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_INDEX) != XPK_OK ) {
		xpkClose(objXpk);
		return 1518;
	}
	if ( xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN) != XPK_OK ) {
		xpkClose(objXpk);
		return 1519;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkIndexAddFile(objXpk, 8101, sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 1520;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1521;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgVolumeLarge, NULL);
	if ( objXpk == NULL ) {
		return 1522;
	}
	if ( xpkIndexGetInfo(objXpk, 8101, &objInfoIndex) != XPK_OK ) {
		xpkClose(objXpk);
		return 1523;
	}
	if ( (objInfoIndex.fileHash != 0) || (objInfoIndex.fileSize != 0) || (objInfoIndex.dataSize != 0) ) {
		xpkClose(objXpk);
		return 1524;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1525;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkIndexReadToFile(objXpk, 8101, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1526;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, 0) ) {
		xpkClose(objXpk);
		return 1527;
	}
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgVolumeLarge);

	procTestDeletePathFamily(sPathPkgVolumeLarge);
	procTestDeletePathFamily(sPathFileExport);
	if ( !procTestWriteBinaryFile(sPathFileExport, "VOL-INDEX-ZERO", sizeof("VOL-INDEX-ZERO")) ) {
		return 1593;
	}
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgVolumeLarge, &objOpt);
	if ( objXpk == NULL ) {
		return 1594;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_INDEX) != XPK_OK ) {
		xpkClose(objXpk);
		return 1595;
	}
	if ( xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN) != XPK_OK ) {
		xpkClose(objXpk);
		return 1596;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkIndexAddFile(objXpk, 8102, sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 1597;
	}
	if ( !procTestWriteBinaryFile(sPathFileExport, NULL, 0) ) {
		xpkClose(objXpk);
		return 1598;
	}
	if ( xpkIndexUpdateFile(objXpk, 8102, sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 1599;
	}
	if ( xpkIndexGetInfo(objXpk, 8102, &objInfoIndex) != XPK_OK ) {
		xpkClose(objXpk);
		return 1600;
	}
	if ( (objInfoIndex.fileHash != 0) || (objInfoIndex.fileSize != 0) || (objInfoIndex.dataSize != 0) ) {
		xpkClose(objXpk);
		return 1601;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1602;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgVolumeLarge, NULL);
	if ( objXpk == NULL ) {
		return 1603;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1604;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkIndexReadToFile(objXpk, 8102, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1605;
	}
	if ( !procTestFileSizeEquals(sPathFileExport, 0) ) {
		xpkClose(objXpk);
		return 1606;
	}
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgVolumeLarge);

	procTestDeletePathFamily(sPathPkgVolumeLarge);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgVolumeLarge, &objOpt);
	if ( objXpk == NULL ) {
		return 1607;
	}
	if ( xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN) != XPK_OK ) {
		xpkClose(objXpk);
		return 1608;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, "VOL-COMP-KEEP", sizeof("VOL-COMP-KEEP"), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1609;
	}
	if ( !procTestCreateSparseFileSize(sPathFileHuge, (uint64_t)UINT32_MAX + 13579u) ) {
		xpkClose(objXpk);
		return 1610;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	iRetCall = xpkUpdateFile(objXpk, 1, sPathFileHuge, &objWriteOpt);
	if ( iRetCall != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1611;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1612;
	}
	if ( objInfo.fileSize != ((uint64_t)UINT32_MAX + 13579u) || objInfo.dataSize >= objInfo.fileSize || (objInfo.flag & XPK_FLAG_COMP_MASK) != 6 ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1613;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( !procTestExpectNullResultError(pDataRead, objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge) ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1614;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1615;
	}
	xpkClose(objXpk);
	objXpk = xpkOpen(sPathPkgVolumeLarge, NULL);
	if ( objXpk == NULL ) {
		remove(sPathFileHuge);
		return 1616;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1617;
	}
	if ( objInfo.fileSize != ((uint64_t)UINT32_MAX + 13579u) || objInfo.dataSize >= objInfo.fileSize || (objInfo.flag & XPK_FLAG_COMP_MASK) != 6 ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1618;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( !procTestExpectNullResultError(pDataRead, objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge) ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1619;
	}
	remove(sPathFileHuge);
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgVolumeLarge);

	procTestDeletePathFamily(sPathPkgVolumeLarge);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgVolumeLarge, &objOpt);
	if ( objXpk == NULL ) {
		return 1615;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_WIN32) != XPK_OK ) {
		xpkClose(objXpk);
		return 1616;
	}
	if ( xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN) != XPK_OK ) {
		xpkClose(objXpk);
		return 1617;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkPathAddData(objXpk, "assets/keep_comp.bin", "VOL-PATH-COMP-KEEP", sizeof("VOL-PATH-COMP-KEEP"), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 1618;
	}
	if ( !procTestCreateSparseFileSize(sPathFileHuge, (uint64_t)UINT32_MAX + 24680u) ) {
		xpkClose(objXpk);
		return 1619;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	iRetCall = xpkPathUpdateFile(objXpk, "ASSETS\\KEEP_COMP.BIN", sPathFileHuge, &objWriteOpt);
	if ( iRetCall != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1620;
	}
	if ( xpkPathGetInfo(objXpk, "assets/keep_comp.bin", &objInfoPath) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1621;
	}
	if ( objInfoPath.fileSize != ((uint64_t)UINT32_MAX + 24680u) || objInfoPath.dataSize >= objInfoPath.fileSize || (objInfoPath.flag & XPK_FLAG_COMP_MASK) != 6 ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1622;
	}
	pDataRead = xpkPathReadToMemory(objXpk, "assets/keep_comp.bin", &iDataSize);
	if ( !procTestExpectNullResultError(pDataRead, objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge) ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1623;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1624;
	}
	xpkClose(objXpk);
	objXpk = xpkOpen(sPathPkgVolumeLarge, NULL);
	if ( objXpk == NULL ) {
		remove(sPathFileHuge);
		return 1625;
	}
	if ( xpkPathGetInfo(objXpk, "assets/keep_comp.bin", &objInfoPath) != XPK_OK ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1626;
	}
	if ( objInfoPath.fileSize != ((uint64_t)UINT32_MAX + 24680u) || objInfoPath.dataSize >= objInfoPath.fileSize || (objInfoPath.flag & XPK_FLAG_COMP_MASK) != 6 ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1627;
	}
	pDataRead = xpkPathReadToMemory(objXpk, "assets/keep_comp.bin", &iDataSize);
	if ( !procTestExpectNullResultError(pDataRead, objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorBlockTooLarge) ) {
		remove(sPathFileHuge);
		xpkClose(objXpk);
		return 1628;
	}
	remove(sPathFileHuge);
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgVolumeLarge);

	sPathVolume = procXpkVolumePathDupText(sPathPkgVolume, 1);
	if ( sPathVolume == NULL ) {
		return 124;
	}
	if ( !xrtFileExists((str)sPathVolume) ) {
		xpkFreeInternal(sPathVolume);
		return 125;
	}
	xpkFreeInternal(sPathVolume);

	hFile = xrtOpen((str)sPathPkgVolume, FALSE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		return 126;
	}
	xrtSeek(hFile, (long)XPK_VOLUME_MIN, XRT_SEEK_SET);
	iCorruptByte = (uint8_t)'Z';
	if ( xrtWrite(hFile, (str)&iCorruptByte, 1) != 1 ) {
		xrtClose(hFile);
		return 127;
	}
	xrtClose(hFile);

	objXpk = xpkOpen(sPathPkgVolume, NULL);
	if ( objXpk == NULL ) {
		return 128;
	}
	if ( xpkGetVolumeSize(objXpk, &iVolumeSize) != XPK_OK ) {
		xpkClose(objXpk);
		return 129;
	}
	if ( iVolumeSize != XPK_VOLUME_MIN ) {
		xpkClose(objXpk);
		return 130;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 131;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 132;
	}
	if ( iDataSize != sizeof(sDataVolumeA) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 133;
	}
	if ( memcmp(pDataRead, sDataVolumeA, sizeof(sDataVolumeA)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 134;
	}
	xpkFree(pDataRead);
	memset(&objBuildOpt, 0, sizeof(objBuildOpt));
	objBuildOpt.tempPath = sPathPkgVolumeTemp;
	objBuildOpt.replaceOriginal = FALSE;
	if ( xpkBuild(objXpk, &objBuildOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 330;
	}
	iVolumeCount = 0;
	if ( procXpkCountVolumeFilesText(objXpk, sPathPkgVolumeTemp, &iVolumeCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 331;
	}
	if ( iVolumeCount != 3 ) {
		xpkClose(objXpk);
		return 332;
	}
	procTestDeletePathFamily(sPathPkgVolumeTemp);

	if ( xpkRemove(objXpk, 1) != XPK_OK ) {
		xpkClose(objXpk);
		return 135;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 136;
	}
	if ( xpkStatGet(objXpk, &objStat) != XPK_OK ) {
		xpkClose(objXpk);
		return 137;
	}
	if ( objStat.fileCount != 1 ) {
		xpkClose(objXpk);
		return 138;
	}
	if ( objStat.holeBytes != sizeof(sDataVolumeA) ) {
		xpkClose(objXpk);
		return 139;
	}
	if ( xpkBuild(objXpk, &objBuildOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 333;
	}
	iVolumeCount = 0;
	if ( procXpkCountVolumeFilesText(objXpk, sPathPkgVolumeTemp, &iVolumeCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 334;
	}
	if ( iVolumeCount != 2 ) {
		xpkClose(objXpk);
		return 335;
	}
	procTestDeletePathFamily(sPathPkgVolumeTemp);
	sPathVolume = procXpkVolumePathDupText(sPathPkgVolume, 5);
	if ( sPathVolume == NULL ) {
		xpkClose(objXpk);
		return 336;
	}
	if ( !procTestWriteVolumeBinaryFile(sPathPkgVolume, 5, "STALE", 5) ) {
		xpkFreeInternal(sPathVolume);
		xpkClose(objXpk);
		return 337;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkFreeInternal(sPathVolume);
		xpkClose(objXpk);
		return 140;
	}
	if ( xrtFileExists((str)sPathVolume) ) {
		xpkFreeInternal(sPathVolume);
		xpkClose(objXpk);
		return 339;
	}
	xpkFreeInternal(sPathVolume);
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 141;
	}
	if ( xpkStatGet(objXpk, &objStat) != XPK_OK ) {
		xpkClose(objXpk);
		return 142;
	}
	if ( objStat.fileCount != 1 ) {
		xpkClose(objXpk);
		return 143;
	}
	if ( objStat.holeBytes != 0 ) {
		xpkClose(objXpk);
		return 144;
	}
	if ( xpkGetVolumeSize(objXpk, &iVolumeSize) != XPK_OK ) {
		xpkClose(objXpk);
		return 145;
	}
	if ( iVolumeSize != XPK_VOLUME_MIN ) {
		xpkClose(objXpk);
		return 146;
	}
	iVolumeCount = 0;
	if ( procXpkCountVolumeFilesText(objXpk, objXpk->sPathPackage, &iVolumeCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 147;
	}
	if ( iVolumeCount < 2 ) {
		xpkClose(objXpk);
		return 148;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 149;
	}
	if ( iDataSize != sizeof(sDataVolumeB) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 150;
	}
	if ( memcmp(pDataRead, sDataVolumeB, sizeof(sDataVolumeB)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 151;
	}
	xpkFree(pDataRead);
	xpkClose(objXpk);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgVolumeLarge, &objOpt);
	if ( objXpk == NULL ) {
		return 984;
	}
	if ( xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN) != XPK_OK ) {
		xpkClose(objXpk);
		return 985;
	}
	iDataLargeSize = (9u * 1024u * 1024u) + 321u;
	pDataLarge = xpkAllocInternal(iDataLargeSize);
	if ( pDataLarge == NULL ) {
		xpkClose(objXpk);
		return 986;
	}
	memset(pDataLarge, 'V', iDataLargeSize);
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, pDataLarge, iDataLargeSize, &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 987;
	}
	if ( iPosRet != 1 ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 988;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 989;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgVolumeLarge, NULL);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 990;
	}
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 991;
	}
	if ( !procTestFileContentEquals(sPathFileExport, pDataLarge, iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 992;
	}
	iVolumeCount = 0;
	if ( procXpkCountVolumeFilesText(objXpk, objXpk->sPathPackage, &iVolumeCount) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 993;
	}
	if ( iVolumeCount < 100 ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 994;
	}
	xpkFreeInternal(pDataLarge);
	pDataLarge = NULL;
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgVolumeLarge);

	iDataLargeSize = (9u * 1024u * 1024u) + 543u;
	pDataLarge = xpkAllocInternal(iDataLargeSize);
	if ( pDataLarge == NULL ) {
		return 1626;
	}
	memset(pDataLarge, 'Z', iDataLargeSize);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgVolumeLarge, &objOpt);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1627;
	}
	if ( xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1628;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, pDataLarge, iDataLargeSize, &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1629;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1630;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgVolumeLarge, NULL);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1631;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1632;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1633;
	}
	if ( !procTestFileContentEquals(sPathFileExport, pDataLarge, iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1634;
	}
	xpkFreeInternal(pDataLarge);
	pDataLarge = NULL;
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgVolumeLarge);

	iDataLargeSize = (9u * 1024u * 1024u) + 765u;
	pDataLarge = xpkAllocInternal(iDataLargeSize);
	if ( pDataLarge == NULL ) {
		return 1420;
	}
	memset(pDataLarge, 'W', iDataLargeSize);
	if ( !procTestWriteBinaryFile(sPathFileExport, pDataLarge, (size_t)iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1421;
	}

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgVolumeLarge, &objOpt);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1422;
	}
	if ( xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1423;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddFile(objXpk, sPathFileExport, &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1424;
	}
	if ( iPosRet != 1 ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1425;
	}
	if ( !objXpk->bDirtyData || !objXpk->bDirtyEntryTable || !objXpk->bDirtyHead ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1426;
	}

	memset(pDataLarge, 'X', iDataLargeSize);
	if ( !procTestWriteBinaryFile(sPathFileExport, pDataLarge, (size_t)iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1427;
	}
	if ( xpkUpdateFile(objXpk, 1, sPathFileExport, &objWriteOpt) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1428;
	}
	if ( !objXpk->bDirtyData || !objXpk->bDirtyEntryTable || !objXpk->bDirtyHead ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1429;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1430;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgVolumeLarge, NULL);
	if ( objXpk == NULL ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		return 1431;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1432;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1433;
	}
	if ( !procTestFileContentEquals(sPathFileExport, pDataLarge, iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1434;
	}
	iVolumeCount = 0;
	if ( procXpkCountVolumeFilesText(objXpk, objXpk->sPathPackage, &iVolumeCount) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1435;
	}
	if ( iVolumeCount < 100 ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		return 1436;
	}
	xpkFreeInternal(pDataLarge);
	pDataLarge = NULL;
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathPkgVolumeLarge);
	procTestDeletePathFamily(sPathPkgVolumeSolid);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgVolumeSolid, &objOpt);
	if ( objXpk == NULL ) {
		return 995;
	}
	if ( xpkSetVolumeSize(objXpk, XPK_VOLUME_MIN) != XPK_OK ) {
		xpkClose(objXpk);
		return 996;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, sDataVolumeA, sizeof(sDataVolumeA), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 997;
	}
	if ( xpkAddData(objXpk, sDataVolumeB, sizeof(sDataVolumeB), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 998;
	}
	if ( xpkSetSolidMode(objXpk, TRUE) != XPK_OK ) {
		xpkClose(objXpk);
		return 999;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		fprintf(
			stderr,
			"volume solid build failed: last=%d text=%s\n",
			xpkLastError(objXpk),
			xpkLastErrorMessage(objXpk) != NULL ? xpkLastErrorMessage(objXpk) : "(null)"
		);
		xpkClose(objXpk);
		return 1000;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1001;
	}
	if ( xpkReadToFile(objXpk, 2, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1002;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataVolumeB, sizeof(sDataVolumeB)) ) {
		xpkClose(objXpk);
		return 1003;
	}
	iVolumeCount = 0;
	if ( procXpkCountVolumeFilesText(objXpk, objXpk->sPathPackage, &iVolumeCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 1004;
	}
	if ( !objXpk->bVolumeApplied || (procXpkAppliedVolumeSize(objXpk) != XPK_VOLUME_MIN) || (iVolumeCount < 1) ) {
		xpkClose(objXpk);
		return 1005;
	}
	if ( xpkRemove(objXpk, 1) != XPK_OK ) {
		xpkClose(objXpk);
		return 1006;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1007;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1008;
	}
	pDataRead = xpkReadToMemory(objXpk, 2, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 1009;
	}
	if ( iDataSize != sizeof(sDataVolumeB) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1010;
	}
	if ( memcmp(pDataRead, sDataVolumeB, sizeof(sDataVolumeB)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1011;
	}
	xpkFree(pDataRead);
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 1012;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1013;
	}
	if ( xpkStatGet(objXpk, &objStat) != XPK_OK ) {
		xpkClose(objXpk);
		return 1014;
	}
	if ( objStat.holeBytes != 0 ) {
		xpkClose(objXpk);
		return 1015;
	}
	pDataRead = xpkReadToMemory(objXpk, 1, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 1016;
	}
	if ( iDataSize != sizeof(sDataVolumeB) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1017;
	}
	if ( memcmp(pDataRead, sDataVolumeB, sizeof(sDataVolumeB)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 1018;
	}
	xpkFree(pDataRead);
	xpkClose(objXpk);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgReplace, &objOpt);
