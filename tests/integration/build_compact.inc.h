	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgBuild, &objOpt);
	if ( objXpk == NULL ) {
		return 66;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_INDEX) != XPK_OK ) {
		xpkClose(objXpk);
		return 67;
	}

	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkIndexAddData(objXpk, 2001, sDataBuildA, sizeof(sDataBuildA), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 68;
	}
	if ( xpkIndexAddData(objXpk, 2002, sDataBuildB, sizeof(sDataBuildB), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 69;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 70;
	}
	iMatchCount = 0;
	if ( xpkEachMatch(objXpk, "200?", NULL, &iMatchCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 71;
	}
	if ( iMatchCount != 2 ) {
		xpkClose(objXpk);
		return 72;
	}
	if ( xpkIndexRemove(objXpk, 2001) != XPK_OK ) {
		xpkClose(objXpk);
		return 73;
	}
	if ( xpkCount(objXpk) != 1 ) {
		xpkClose(objXpk);
		return 297;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_ERR_NOT_FOUND ) {
		xpkClose(objXpk);
		return 298;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 74;
	}
	if ( xpkStatGet(objXpk, &objStat) != XPK_OK ) {
		xpkClose(objXpk);
		return 75;
	}
	if ( objStat.fileCount != 1 ) {
		xpkClose(objXpk);
		return 76;
	}
	if ( objStat.liveDataBytes != sizeof(sDataBuildB) ) {
		xpkClose(objXpk);
		return 77;
	}
	if ( objStat.holeBytes != sizeof(sDataBuildA) ) {
		xpkClose(objXpk);
		return 78;
	}

	hFile = xrtOpen((str)sPathPkgBuild, TRUE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		xpkClose(objXpk);
		return 79;
	}
	iBuildSizeBefore = xrtGetEOF(hFile);
	xrtClose(hFile);

	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 80;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 81;
	}
	if ( xpkStatGet(objXpk, &objStat) != XPK_OK ) {
		xpkClose(objXpk);
		return 82;
	}
	if ( objStat.fileCount != 1 ) {
		xpkClose(objXpk);
		return 83;
	}
	if ( objStat.liveDataBytes != sizeof(sDataBuildB) ) {
		xpkClose(objXpk);
		return 84;
	}
	if ( objStat.holeBytes != 0 ) {
		xpkClose(objXpk);
		return 85;
	}
	if ( xpkCount(objXpk) != 1 ) {
		xpkClose(objXpk);
		return 86;
	}
	if ( xpkIndexFind(objXpk, 2001, &iPosRet) != XPK_ERR_NOT_FOUND ) {
		xpkClose(objXpk);
		return 87;
	}
	pDataRead = xpkIndexReadToMemory(objXpk, 2002, &iDataSize);
	if ( pDataRead == NULL ) {
		xpkClose(objXpk);
		return 88;
	}
	if ( iDataSize != sizeof(sDataBuildB) ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 89;
	}
	if ( memcmp(pDataRead, sDataBuildB, sizeof(sDataBuildB)) != 0 ) {
		xpkFree(pDataRead);
		xpkClose(objXpk);
		return 90;
	}
	xpkFree(pDataRead);

	hFile = xrtOpen((str)sPathPkgBuild, TRUE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		xpkClose(objXpk);
		return 91;
	}
	iBuildSizeAfter = xrtGetEOF(hFile);
	xrtClose(hFile);
	if ( iBuildSizeAfter >= iBuildSizeBefore ) {
		xpkClose(objXpk);
		return 92;
	}
	if ( !procTestWriteBinaryFile(sPathPkgBuildExistsTemp, "KEEP", 4) ) {
		xpkClose(objXpk);
		return 801;
	}

	memset(&objBuildOpt, 0, sizeof(objBuildOpt));
	objBuildOpt.tempPath = sPathPkgBuildExistsTemp;
	objBuildOpt.replaceOriginal = FALSE;
	iRetCall = xpkBuild(objXpk, &objBuildOpt);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_EXISTS, objXpk, XPK_ERR_EXISTS, sXpkErrorTempPathExists) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_EXISTS) ? 803 : 804;
	}
	if ( !procTestFileContentEquals(sPathPkgBuildExistsTemp, "KEEP", 4) ) {
		xpkClose(objXpk);
		return 805;
	}
	sPathKey = procXpkPathSuffixDupText(sPathPkgBuild, ".build.tmp");
	if ( sPathKey == NULL ) {
		xpkClose(objXpk);
		return 897;
	}
	if ( !procTestWriteBinaryFile(sPathKey, "KEEP", 4) ) {
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 898;
	}
	iRetCall = xpkBuild(objXpk, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_EXISTS, objXpk, XPK_ERR_EXISTS, sXpkErrorTempPathExists) ) {
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_EXISTS) ? 900 : 901;
	}
	if ( !procTestFileContentEquals(sPathKey, "KEEP", 4) ) {
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 902;
	}
	sPathVolume = procXpkVolumePathDupText(sPathKey, 2);
	if ( sPathVolume == NULL ) {
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 905;
	}
	if ( !procTestWriteBinaryFile(sPathVolume, "SPARSE", 6) ) {
		xpkFreeInternal(sPathVolume);
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 906;
	}
	iRetCall = xpkBuild(objXpk, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_EXISTS, objXpk, XPK_ERR_EXISTS, sXpkErrorTempPathExists) ) {
		xpkFreeInternal(sPathVolume);
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_EXISTS) ? 908 : 909;
	}
	if ( !procTestFileContentEquals(sPathVolume, "SPARSE", 6) ) {
		xpkFreeInternal(sPathVolume);
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 910;
	}
	xpkFreeInternal(sPathVolume);
	sPathVolume = procXpkVolumePathDupText(sPathKey, 1);
	if ( sPathVolume == NULL ) {
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 917;
	}
	if ( !procTestCreateDirOccupy(sPathVolume) ) {
		xpkFreeInternal(sPathVolume);
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 918;
	}
	iRetCall = xpkBuild(objXpk, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_EXISTS, objXpk, XPK_ERR_EXISTS, sXpkErrorTempPathExists) ) {
		procTestDeletePathFamily(sPathVolume);
		xpkFreeInternal(sPathVolume);
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_EXISTS) ? 919 : 920;
	}
	if ( !xrtDirExists((str)sPathVolume) ) {
		procTestDeletePathFamily(sPathVolume);
		xpkFreeInternal(sPathVolume);
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 921;
	}
	procTestDeletePathFamily(sPathVolume);
	if ( !procTestCreateDirOccupy(sPathPkgBuildExistsDir) ) {
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 913;
	}
	memset(&objBuildOpt, 0, sizeof(objBuildOpt));
	objBuildOpt.tempPath = sPathPkgBuildExistsDir;
	objBuildOpt.replaceOriginal = FALSE;
	iRetCall = xpkBuild(objXpk, &objBuildOpt);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_EXISTS, objXpk, XPK_ERR_EXISTS, sXpkErrorTempPathExists) ) {
		procTestDeletePathFamily(sPathPkgBuildExistsDir);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_EXISTS) ? 914 : 915;
	}
	if ( !xrtDirExists((str)sPathPkgBuildExistsDir) ) {
		procTestDeletePathFamily(sPathPkgBuildExistsDir);
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return 916;
	}
	procTestDeletePathFamily(sPathPkgBuildExistsDir);
	xpkFreeInternal(sPathVolume);
	procTestDeletePathFamily(sPathKey);
	xpkFreeInternal(sPathKey);
	xpkClose(objXpk);
	sPathKey = procXpkPathSuffixDupText(sPathPkgBuild, ".buffered");
	if ( sPathKey == NULL ) {
		return 1003;
	}
	procTestDeletePathFamily(sPathKey);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathKey, &objOpt);
	if ( objXpk == NULL ) {
		xpkFreeInternal(sPathKey);
		return 1004;
	}
	iDataLargeSize = (9u * 1024u * 1024u) + 777u;
	pDataLarge = xpkAllocInternal(iDataLargeSize);
	if ( pDataLarge == NULL ) {
		xpkClose(objXpk);
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		return 1005;
	}
	memset(pDataLarge, 'R', iDataLargeSize);
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 0;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkAddData(objXpk, pDataLarge, iDataLargeSize, &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		return 1006;
	}
	if ( iPosRet != 1 ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		return 1007;
	}
	if ( xpkBuild(objXpk, NULL) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		return 1008;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		return 1009;
	}
	if ( xpkCount(objXpk) != 1 ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		return 1010;
	}
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		return 1011;
	}
	if ( !procTestFileContentEquals(sPathFileExport, pDataLarge, iDataLargeSize) ) {
		xpkFreeInternal(pDataLarge);
		pDataLarge = NULL;
		xpkClose(objXpk);
		procTestDeletePathFamily(sPathKey);
		xpkFreeInternal(sPathKey);
		return 1012;
	}
	xpkFreeInternal(pDataLarge);
	pDataLarge = NULL;
	xpkClose(objXpk);
	procTestDeletePathFamily(sPathKey);
	xpkFreeInternal(sPathKey);
	{
		xpkObject objStress;
		char* sPathStressPkg;
		uint8_t* pDataStress;
		uint32_t iStressIndex;
		uint32_t iStressRound;
		uint32_t iStressSize;
		char sPathStressEntry[64];
		char iStressFill;
		sPathStressPkg = procXpkPathSuffixDupText(sPathPkgBuild, ".stress");
		if ( sPathStressPkg == NULL ) {
			return 2194;
		}
		procTestDeletePathFamily(sPathStressPkg);
		pDataStress = (uint8_t*)xpkAllocInternal(131072u);
		if ( pDataStress == NULL ) {
			xpkFreeInternal(sPathStressPkg);
			return 2195;
		}

		memset(&objOpt, 0, sizeof(objOpt));
		objOpt.createIfMissing = TRUE;
		objStress = xpkOpen(sPathStressPkg, &objOpt);
		if ( objStress == NULL ) {
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2196;
		}
		if ( xpkSetPackType(objStress, XPK_PACK_WIN32) != XPK_OK ) {
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2197;
		}

		for ( iStressIndex = 0; iStressIndex < 12; iStressIndex++ ) {
			iStressSize = 65536u + (iStressIndex * 321u);
			iStressFill = (char)('A' + (iStressIndex % 26u));
			memset(pDataStress, iStressFill, iStressSize);
			memset(sPathStressEntry, 0, sizeof(sPathStressEntry));
			snprintf(sPathStressEntry, sizeof(sPathStressEntry), "assets/stress_%02u.bin", iStressIndex);

			memset(&objWriteOpt, 0, sizeof(objWriteOpt));
			switch ( iStressIndex & 3u ) {
				case 0:
					objWriteOpt.compLevel = 0;
					break;
				case 1:
					objWriteOpt.compLevel = 2;
					break;
				case 2:
					objWriteOpt.compLevel = 6;
					break;
				default:
					objWriteOpt.compLevel = 14;
					break;
			}
			objWriteOpt.writePolicy = (iStressIndex < 6u) ? XPK_WRITE_IMMEDIATE : XPK_WRITE_BUFFERED;
			if ( xpkPathAddData(objStress, sPathStressEntry, pDataStress, iStressSize, &objWriteOpt) != XPK_OK ) {
				xpkClose(objStress);
				procTestDeletePathFamily(sPathStressPkg);
				xpkFreeInternal(pDataStress);
				xpkFreeInternal(sPathStressPkg);
				return 2198;
			}
		}

		if ( xpkSave(objStress) != XPK_OK ) {
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2199;
		}
		xpkClose(objStress);

		objStress = xpkOpen(sPathStressPkg, NULL);
		if ( objStress == NULL ) {
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2200;
		}
		if ( xpkVerifyAll(objStress) != XPK_OK ) {
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2201;
		}
		if ( xpkSetSolidMode(objStress, TRUE) != XPK_OK ) {
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2202;
		}
		if ( xpkSetDefaultComp(objStress, 6) != XPK_OK ) {
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2203;
		}
		if ( xpkBuild(objStress, NULL) != XPK_OK ) {
			fprintf(stderr, "stress volume build failed: last=%d text=%s solid=%d volume=%u appliedSolid=%d appliedVolume=%d appliedVolumeSize=%u count=%u append=%llu file=%llu\n",
				xpkLastError(objStress),
				xpkLastErrorMessage(objStress),
				objStress->objHead.solidMode,
				objStress->objHead.volumeSize,
				objStress->bSolidApplied,
				objStress->bVolumeApplied,
				objStress->iVolumeSizeApplied,
				objStress->iEntryCount,
				(unsigned long long)objStress->iAppendPos,
				(unsigned long long)objStress->iFileSize);
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2204;
		}
		if ( xpkVerifyAll(objStress) != XPK_OK ) {
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2205;
		}
		if ( xpkSetDefaultComp(objStress, 14) != XPK_OK ) {
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2206;
		}
		if ( xpkBuild(objStress, NULL) != XPK_OK ) {
			fprintf(stderr, "stress volume build failed: last=%d text=%s solid=%d volume=%u appliedSolid=%d appliedVolume=%d appliedVolumeSize=%u count=%u append=%llu file=%llu\n",
				xpkLastError(objStress),
				xpkLastErrorMessage(objStress),
				objStress->objHead.solidMode,
				objStress->objHead.volumeSize,
				objStress->bSolidApplied,
				objStress->bVolumeApplied,
				objStress->iVolumeSizeApplied,
				objStress->iEntryCount,
				(unsigned long long)objStress->iAppendPos,
				(unsigned long long)objStress->iFileSize);
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2207;
		}
		if ( xpkVerifyAll(objStress) != XPK_OK ) {
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2208;
		}
		if ( xpkSetSolidMode(objStress, FALSE) != XPK_OK ) {
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2209;
		}
		if ( xpkBuild(objStress, NULL) != XPK_OK ) {
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2210;
		}
		if ( xpkVerifyAll(objStress) != XPK_OK ) {
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2211;
		}
		for ( iStressIndex = 0; iStressIndex < 131072u; iStressIndex++ ) {
			pDataStress[iStressIndex] = (uint8_t)((iStressIndex * 17u) + 3u);
		}
		memset(&objWriteOpt, 0, sizeof(objWriteOpt));
		objWriteOpt.compLevel = 0;
		objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
		if ( xpkPathAddData(objStress, "assets/stress_store.bin", pDataStress, 131072u, &objWriteOpt) != XPK_OK ) {
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2212;
		}
		if ( xpkSetVolumeSize(objStress, XPK_VOLUME_MIN) != XPK_OK ) {
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2213;
		}
		if ( xpkBuild(objStress, NULL) != XPK_OK ) {
			fprintf(stderr, "stress volume build failed: last=%d text=%s solid=%d volume=%u appliedSolid=%d appliedVolume=%d appliedVolumeSize=%u count=%u append=%llu file=%llu\n",
				xpkLastError(objStress),
				xpkLastErrorMessage(objStress),
				objStress->objHead.solidMode,
				objStress->objHead.volumeSize,
				objStress->bSolidApplied,
				objStress->bVolumeApplied,
				objStress->iVolumeSizeApplied,
				objStress->iEntryCount,
				(unsigned long long)objStress->iAppendPos,
				(unsigned long long)objStress->iFileSize);
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2214;
		}
		if ( xpkVerifyAll(objStress) != XPK_OK ) {
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2215;
		}
		if ( procXpkCountVolumeFilesText(objStress, objStress->sPathPackage, &iVolumeCount) != XPK_OK ) {
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2216;
		}
		if ( iVolumeCount < 2 ) {
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2217;
		}

		memset(pDataStress, 'D', 65536u + (3u * 321u));
		if ( xpkPathReadToFile(objStress, "assets/stress_03.bin", sPathFileExport) != XPK_OK ) {
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2218;
		}
		if ( !procTestFileContentEquals(sPathFileExport, pDataStress, 65536u + (3u * 321u)) ) {
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2219;
		}
		memset(pDataStress, 'L', 65536u + (11u * 321u));
		if ( xpkPathReadToFile(objStress, "assets/stress_11.bin", sPathFileExport) != XPK_OK ) {
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2220;
		}
		if ( !procTestFileContentEquals(sPathFileExport, pDataStress, 65536u + (11u * 321u)) ) {
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2221;
		}
		for ( iStressIndex = 0; iStressIndex < 131072u; iStressIndex++ ) {
			pDataStress[iStressIndex] = (uint8_t)((iStressIndex * 17u) + 3u);
		}
		if ( xpkPathReadToFile(objStress, "assets/stress_store.bin", sPathFileExport) != XPK_OK ) {
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2222;
		}
		if ( !procTestFileContentEquals(sPathFileExport, pDataStress, 131072u) ) {
			xpkClose(objStress);
			procTestDeletePathFamily(sPathStressPkg);
			xpkFreeInternal(pDataStress);
			xpkFreeInternal(sPathStressPkg);
			return 2223;
		}

		for ( iStressRound = 0; iStressRound < 5; iStressRound++ ) {
			xpkClose(objStress);
			objStress = xpkOpen(sPathStressPkg, NULL);
			if ( objStress == NULL ) {
				procTestDeletePathFamily(sPathStressPkg);
				xpkFreeInternal(pDataStress);
				xpkFreeInternal(sPathStressPkg);
				return 2221;
			}
			if ( xpkVerifyAll(objStress) != XPK_OK ) {
				xpkClose(objStress);
				procTestDeletePathFamily(sPathStressPkg);
				xpkFreeInternal(pDataStress);
				xpkFreeInternal(sPathStressPkg);
				return 2222;
			}

			switch ( iStressRound ) {
				case 0:
					if ( xpkSetVolumeSize(objStress, 0) != XPK_OK ) {
						xpkClose(objStress);
						procTestDeletePathFamily(sPathStressPkg);
						xpkFreeInternal(pDataStress);
						xpkFreeInternal(sPathStressPkg);
						return 2223;
					}
					if ( xpkBuild(objStress, NULL) != XPK_OK ) {
						xpkClose(objStress);
						procTestDeletePathFamily(sPathStressPkg);
						xpkFreeInternal(pDataStress);
						xpkFreeInternal(sPathStressPkg);
						return 2224;
					}
					break;

				case 1:
					if ( xpkSetSolidMode(objStress, TRUE) != XPK_OK ) {
						xpkClose(objStress);
						procTestDeletePathFamily(sPathStressPkg);
						xpkFreeInternal(pDataStress);
						xpkFreeInternal(sPathStressPkg);
						return 2225;
					}
					if ( xpkSetDefaultComp(objStress, 2) != XPK_OK ) {
						xpkClose(objStress);
						procTestDeletePathFamily(sPathStressPkg);
						xpkFreeInternal(pDataStress);
						xpkFreeInternal(sPathStressPkg);
						return 2226;
					}
					if ( xpkBuild(objStress, NULL) != XPK_OK ) {
						xpkClose(objStress);
						procTestDeletePathFamily(sPathStressPkg);
						xpkFreeInternal(pDataStress);
						xpkFreeInternal(sPathStressPkg);
						return 2227;
					}
					break;

				case 2:
					if ( xpkSetDefaultComp(objStress, 6) != XPK_OK ) {
						xpkClose(objStress);
						procTestDeletePathFamily(sPathStressPkg);
						xpkFreeInternal(pDataStress);
						xpkFreeInternal(sPathStressPkg);
						return 2228;
					}
					if ( xpkBuild(objStress, NULL) != XPK_OK ) {
						xpkClose(objStress);
						procTestDeletePathFamily(sPathStressPkg);
						xpkFreeInternal(pDataStress);
						xpkFreeInternal(sPathStressPkg);
						return 2229;
					}
					break;

				case 3:
					if ( xpkSetDefaultComp(objStress, 14) != XPK_OK ) {
						xpkClose(objStress);
						procTestDeletePathFamily(sPathStressPkg);
						xpkFreeInternal(pDataStress);
						xpkFreeInternal(sPathStressPkg);
						return 2230;
					}
					if ( xpkBuild(objStress, NULL) != XPK_OK ) {
						xpkClose(objStress);
						procTestDeletePathFamily(sPathStressPkg);
						xpkFreeInternal(pDataStress);
						xpkFreeInternal(sPathStressPkg);
						return 2231;
					}
					break;

				default:
					if ( xpkSetSolidMode(objStress, FALSE) != XPK_OK ) {
						xpkClose(objStress);
						procTestDeletePathFamily(sPathStressPkg);
						xpkFreeInternal(pDataStress);
						xpkFreeInternal(sPathStressPkg);
						return 2232;
					}
					if ( xpkBuild(objStress, NULL) != XPK_OK ) {
						xpkClose(objStress);
						procTestDeletePathFamily(sPathStressPkg);
						xpkFreeInternal(pDataStress);
						xpkFreeInternal(sPathStressPkg);
						return 2233;
					}
					break;
			}

			if ( xpkVerifyAll(objStress) != XPK_OK ) {
				xpkClose(objStress);
				procTestDeletePathFamily(sPathStressPkg);
				xpkFreeInternal(pDataStress);
				xpkFreeInternal(sPathStressPkg);
				return 2234;
			}
			if ( xpkPathReadToFile(objStress, "assets/stress_00.bin", sPathFileExport) != XPK_OK ) {
				xpkClose(objStress);
				procTestDeletePathFamily(sPathStressPkg);
				xpkFreeInternal(pDataStress);
				xpkFreeInternal(sPathStressPkg);
				return 2235;
			}
			memset(pDataStress, 'A', 65536u);
			if ( !procTestFileContentEquals(sPathFileExport, pDataStress, 65536u) ) {
				xpkClose(objStress);
				procTestDeletePathFamily(sPathStressPkg);
				xpkFreeInternal(pDataStress);
				xpkFreeInternal(sPathStressPkg);
				return 2236;
			}
			if ( xpkPathReadToFile(objStress, "assets/stress_07.bin", sPathFileExport) != XPK_OK ) {
				xpkClose(objStress);
				procTestDeletePathFamily(sPathStressPkg);
				xpkFreeInternal(pDataStress);
				xpkFreeInternal(sPathStressPkg);
				return 2237;
			}
			memset(pDataStress, 'H', 65536u + (7u * 321u));
			if ( !procTestFileContentEquals(sPathFileExport, pDataStress, 65536u + (7u * 321u)) ) {
				xpkClose(objStress);
				procTestDeletePathFamily(sPathStressPkg);
				xpkFreeInternal(pDataStress);
				xpkFreeInternal(sPathStressPkg);
				return 2238;
			}
		}

		xpkClose(objStress);
		procTestDeletePathFamily(sPathStressPkg);
		xpkFreeInternal(pDataStress);
		xpkFreeInternal(sPathStressPkg);
	}
	{
		xpkObject objMutate;
		char* sPathMutatePkg;
		uint8_t* pDataMutate;
		uint32_t iMutateIndex;

		sPathMutatePkg = procXpkPathSuffixDupText(sPathPkgBuild, ".mutate");
		if ( sPathMutatePkg == NULL ) {
			return 2239;
		}
		procTestDeletePathFamily(sPathMutatePkg);
		pDataMutate = (uint8_t*)xpkAllocInternal(131072u);
		if ( pDataMutate == NULL ) {
			xpkFreeInternal(sPathMutatePkg);
			return 2240;
		}

		memset(&objOpt, 0, sizeof(objOpt));
		objOpt.createIfMissing = TRUE;
		objMutate = xpkOpen(sPathMutatePkg, &objOpt);
		if ( objMutate == NULL ) {
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2241;
		}
		if ( xpkSetPackType(objMutate, XPK_PACK_WIN32) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2242;
		}

		memset(pDataMutate, 'A', 40000);
		memset(&objWriteOpt, 0, sizeof(objWriteOpt));
		objWriteOpt.compLevel = 0;
		objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
		if ( xpkPathAddData(objMutate, "assets/m0.bin", pDataMutate, 40000, &objWriteOpt) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2243;
		}

		memset(pDataMutate, 'B', 45000);
		memset(&objWriteOpt, 0, sizeof(objWriteOpt));
		objWriteOpt.compLevel = 0;
		objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
		if ( xpkPathAddData(objMutate, "assets/m1.bin", pDataMutate, 45000, &objWriteOpt) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2244;
		}

		memset(pDataMutate, 'C', 50000);
		memset(&objWriteOpt, 0, sizeof(objWriteOpt));
		objWriteOpt.compLevel = 14;
		objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
		if ( xpkPathAddData(objMutate, "assets/m2.bin", pDataMutate, 50000, &objWriteOpt) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2245;
		}
		if ( xpkSave(objMutate) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2246;
		}
		xpkClose(objMutate);

		objMutate = xpkOpen(sPathMutatePkg, NULL);
		if ( objMutate == NULL ) {
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2247;
		}
		if ( xpkVerifyAll(objMutate) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2248;
		}

		memset(pDataMutate, 'D', 47000);
		memset(&objWriteOpt, 0, sizeof(objWriteOpt));
		objWriteOpt.compLevel = 6;
		objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
		if ( xpkPathUpdateData(objMutate, "assets/m1.bin", pDataMutate, 47000, &objWriteOpt) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2249;
		}
		if ( xpkPathRename(objMutate, "assets/m2.bin", "assets/m2r.bin") != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2250;
		}
		if ( xpkPathRemove(objMutate, "assets/m0.bin") != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2251;
		}
		if ( xpkSave(objMutate) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2252;
		}
		xpkClose(objMutate);

		objMutate = xpkOpen(sPathMutatePkg, NULL);
		if ( objMutate == NULL ) {
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2253;
		}
		if ( xpkVerifyAll(objMutate) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2254;
		}
		if ( xpkCount(objMutate) != 2 ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2255;
		}
		if ( xpkPathExists(objMutate, "assets/m0.bin") ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2256;
		}
		if ( !xpkPathExists(objMutate, "assets/m2r.bin") ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2257;
		}
		memset(pDataMutate, 'D', 47000);
		if ( xpkPathReadToFile(objMutate, "assets/m1.bin", sPathFileExport) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2258;
		}
		if ( !procTestFileContentEquals(sPathFileExport, pDataMutate, 47000) ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2259;
		}

		if ( xpkSetSolidMode(objMutate, TRUE) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2260;
		}
		if ( xpkSetDefaultComp(objMutate, 6) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2261;
		}
		if ( xpkBuild(objMutate, NULL) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2262;
		}
		if ( xpkVerifyAll(objMutate) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2263;
		}
		memset(pDataMutate, 'C', 50000);
		if ( xpkPathReadToFile(objMutate, "assets/m2r.bin", sPathFileExport) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2264;
		}
		if ( !procTestFileContentEquals(sPathFileExport, pDataMutate, 50000) ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2265;
		}
		if ( xpkSetDefaultComp(objMutate, 14) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2266;
		}
		if ( xpkBuild(objMutate, NULL) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2267;
		}
		if ( xpkVerifyAll(objMutate) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2268;
		}
		if ( xpkSetSolidMode(objMutate, FALSE) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2269;
		}
		if ( xpkBuild(objMutate, NULL) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2270;
		}
		for ( iMutateIndex = 0; iMutateIndex < 131072u; iMutateIndex++ ) {
			pDataMutate[iMutateIndex] = (uint8_t)((iMutateIndex * 29u) + 5u);
		}
		memset(&objWriteOpt, 0, sizeof(objWriteOpt));
		objWriteOpt.compLevel = 0;
		objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
		if ( xpkPathAddData(objMutate, "assets/mv.bin", pDataMutate, 131072u, &objWriteOpt) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2283;
		}
		if ( xpkSetVolumeSize(objMutate, XPK_VOLUME_MIN) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2271;
		}
		if ( xpkBuild(objMutate, NULL) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2272;
		}
		if ( xpkVerifyAll(objMutate) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2273;
		}
		if ( procXpkCountVolumeFilesText(objMutate, objMutate->sPathPackage, &iVolumeCount) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2274;
		}
		if ( iVolumeCount < 2 ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2275;
		}

		memset(pDataMutate, 'D', 47000);
		if ( xpkPathReadToFile(objMutate, "assets/m1.bin", sPathFileExport) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2276;
		}
		if ( !procTestFileContentEquals(sPathFileExport, pDataMutate, 47000) ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2277;
		}
		memset(pDataMutate, 'C', 50000);
		if ( xpkPathReadToFile(objMutate, "assets/m2r.bin", sPathFileExport) != XPK_OK ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2278;
		}
		if ( !procTestFileContentEquals(sPathFileExport, pDataMutate, 50000) ) {
			xpkClose(objMutate);
			procTestDeletePathFamily(sPathMutatePkg);
			xpkFreeInternal(pDataMutate);
			xpkFreeInternal(sPathMutatePkg);
			return 2279;
		}

		xpkClose(objMutate);
		procTestDeletePathFamily(sPathMutatePkg);
		xpkFreeInternal(pDataMutate);
		xpkFreeInternal(sPathMutatePkg);
	}
	objXpk = xpkOpen(sPathPkgBuild, NULL);
	if ( objXpk == NULL ) {
		return 93;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		xpkClose(objXpk);
		return 94;
	}
	iBuildSizeBefore = objInfo.dataOffset;
	xpkClose(objXpk);

	hFile = xrtOpen((str)sPathPkgBuild, FALSE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		return 941;
	}
	if ( xrtSeek(hFile, (int64)iBuildSizeBefore, XRT_SEEK_SET) != iBuildSizeBefore ) {
		xrtClose(hFile);
		return 942;
	}
	iCorruptByte = (uint8_t)'C';
	if ( xrtWrite(hFile, (str)&iCorruptByte, 1) != 1 ) {
		xrtClose(hFile);
		return 943;
	}
	xrtClose(hFile);

	objXpk = xpkOpen(sPathPkgBuild, NULL);
	if ( objXpk == NULL ) {
		return 95;
	}
	if ( xpkVerify(objXpk, 1) != XPK_ERR_HASH ) {
		xpkClose(objXpk);
		return 96;
	}
	xpkClose(objXpk);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
