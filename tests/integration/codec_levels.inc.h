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
	procTestDeletePathFamily(sPathFileExport);
	if ( !procTestWriteBinaryFile(sPathFileExport, "OLD-LZ4-EXPORT-CONTENT-TO-TRUNCATE", sizeof("OLD-LZ4-EXPORT-CONTENT-TO-TRUNCATE")) ) {
		xpkClose(objXpk);
		return 2153;
	}
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1647;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataBuildA, sizeof(sDataBuildA)) ) {
		xpkClose(objXpk);
		return 1648;
	}

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
	procTestDeletePathFamily(sPathFileExport);
	if ( !procTestWriteBinaryFile(sPathFileExport, "OLD-LZMA2-EXPORT-CONTENT-TO-TRUNCATE", sizeof("OLD-LZMA2-EXPORT-CONTENT-TO-TRUNCATE")) ) {
		xpkClose(objXpk);
		return 2154;
	}
	if ( xpkReadToFile(objXpk, 2, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1641;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataBuildB, sizeof(sDataBuildB)) ) {
		xpkClose(objXpk);
		return 1642;
	}
	iRetCall = xpkSetFlag(objXpk, 1, XPK_FLAG_COMP_MASK, 14);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorCompManaged) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 763 : 764;
	}
	xpkClose(objXpk);

	procTestDeletePathFamily(sPathPkgCodec);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgCodec, &objOpt);
	if ( objXpk == NULL ) {
		return 1643;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 14;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkAddData(objXpk, sDataBuildB, sizeof(sDataBuildB), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1644;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( !procTestWriteBinaryFile(sPathFileExport, "OLD-QUEUED-LZMA2-EXPORT-CONTENT", sizeof("OLD-QUEUED-LZMA2-EXPORT-CONTENT")) ) {
		xpkClose(objXpk);
		return 2155;
	}
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1645;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataBuildB, sizeof(sDataBuildB)) ) {
		xpkClose(objXpk);
		return 1646;
	}
	xpkClose(objXpk);

	procTestDeletePathFamily(sPathPkgCodec);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgCodec, &objOpt);
	if ( objXpk == NULL ) {
		return 1653;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 14;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkAddData(objXpk, sDataBuildB, sizeof(sDataBuildB), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1654;
	}
	if ( iPosRet != 1 ) {
		xpkClose(objXpk);
		return 1655;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 2132;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgCodec, NULL);
	if ( objXpk == NULL ) {
		return 2133;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 2134;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 2135;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataBuildB, sizeof(sDataBuildB)) ) {
		xpkClose(objXpk);
		return 2136;
	}
	xpkClose(objXpk);

	procTestDeletePathFamily(sPathPkgCodec);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgCodec, &objOpt);
	if ( objXpk == NULL ) {
		return 1649;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 2;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkAddData(objXpk, sDataBuildA, sizeof(sDataBuildA), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1650;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( !procTestWriteBinaryFile(sPathFileExport, "OLD-QUEUED-LZ4-EXPORT-CONTENT", sizeof("OLD-QUEUED-LZ4-EXPORT-CONTENT")) ) {
		xpkClose(objXpk);
		return 2156;
	}
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1651;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataBuildA, sizeof(sDataBuildA)) ) {
		xpkClose(objXpk);
		return 1652;
	}
	xpkClose(objXpk);

	procTestDeletePathFamily(sPathPkgCodec);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgCodec, &objOpt);
	if ( objXpk == NULL ) {
		return 2124;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 2;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkAddData(objXpk, sDataBuildA, sizeof(sDataBuildA), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 2125;
	}
	if ( iPosRet != 1 ) {
		xpkClose(objXpk);
		return 2126;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 2127;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgCodec, NULL);
	if ( objXpk == NULL ) {
		return 2128;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 2129;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 2130;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataBuildA, sizeof(sDataBuildA)) ) {
		xpkClose(objXpk);
		return 2131;
	}
	xpkClose(objXpk);

	procTestDeletePathFamily(sPathPkgCodec);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgCodec, &objOpt);
	if ( objXpk == NULL ) {
		return 2100;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, sDataBuildB, sizeof(sDataBuildB), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 2101;
	}
	if ( iPosRet != 1 ) {
		xpkClose(objXpk);
		return 2102;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 2103;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgCodec, NULL);
	if ( objXpk == NULL ) {
		return 2104;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 2105;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 2106;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataBuildB, sizeof(sDataBuildB)) ) {
		xpkClose(objXpk);
		return 2107;
	}
	xpkClose(objXpk);

	procTestDeletePathFamily(sPathPkgCodec);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgCodec, &objOpt);
	if ( objXpk == NULL ) {
		return 2116;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkAddData(objXpk, sDataBuildB, sizeof(sDataBuildB), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 2117;
	}
	if ( iPosRet != 1 ) {
		xpkClose(objXpk);
		return 2118;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 2119;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgCodec, NULL);
	if ( objXpk == NULL ) {
		return 2120;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 2121;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 2122;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataBuildB, sizeof(sDataBuildB)) ) {
		xpkClose(objXpk);
		return 2123;
	}
	xpkClose(objXpk);

	{
		void* pDataCodecLarge;
		uint32_t iDataCodecLarge;

		pDataCodecLarge = NULL;
		iDataCodecLarge = (3u * 1024u * 1024u) + 123u;

		procTestDeletePathFamily(sPathPkgCodec);
		memset(&objOpt, 0, sizeof(objOpt));
		objOpt.createIfMissing = TRUE;
		objXpk = xpkOpen(sPathPkgCodec, &objOpt);
		if ( objXpk == NULL ) {
			return 2153;
		}

		pDataCodecLarge = xpkAllocInternal(iDataCodecLarge);
		if ( pDataCodecLarge == NULL ) {
			xpkClose(objXpk);
			return 2154;
		}
		memset(pDataCodecLarge, 'Z', iDataCodecLarge);

		memset(&objWriteOpt, 0, sizeof(objWriteOpt));
		objWriteOpt.compLevel = 6;
		objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
		if ( xpkAddData(objXpk, pDataCodecLarge, iDataCodecLarge, &objWriteOpt, &iPosRet) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2155;
		}
		if ( xpkSave(objXpk) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2156;
		}
		xpkClose(objXpk);

		objXpk = xpkOpen(sPathPkgCodec, NULL);
		if ( objXpk == NULL ) {
			xpkFreeInternal(pDataCodecLarge);
			return 2157;
		}
		if ( xpkVerifyAll(objXpk) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2158;
		}
		procTestDeletePathFamily(sPathFileExport);
		if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2159;
		}
		if ( !procTestFileContentEquals(sPathFileExport, pDataCodecLarge, iDataCodecLarge) ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2160;
		}
		xpkFreeInternal(pDataCodecLarge);
		xpkClose(objXpk);
	}

	{
		void* pDataCodecLarge;
		uint32_t iDataCodecLarge;

		pDataCodecLarge = NULL;
		iDataCodecLarge = (3u * 1024u * 1024u) + 913u;

		procTestDeletePathFamily(sPathPkgCodec);
		memset(&objOpt, 0, sizeof(objOpt));
		objOpt.createIfMissing = TRUE;
		objXpk = xpkOpen(sPathPkgCodec, &objOpt);
		if ( objXpk == NULL ) {
			return 2177;
		}

		pDataCodecLarge = xpkAllocInternal(iDataCodecLarge);
		if ( pDataCodecLarge == NULL ) {
			xpkClose(objXpk);
			return 2178;
		}
		memset(pDataCodecLarge, 'U', iDataCodecLarge);

		memset(&objWriteOpt, 0, sizeof(objWriteOpt));
		objWriteOpt.compLevel = 0;
		objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
		if ( xpkAddData(objXpk, "KEEP-UPDATE-ZSTD", sizeof("KEEP-UPDATE-ZSTD"), &objWriteOpt, &iPosRet) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2179;
		}
		objWriteOpt.compLevel = 6;
		if ( xpkUpdateData(objXpk, 1, pDataCodecLarge, iDataCodecLarge, &objWriteOpt) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2180;
		}
		if ( xpkSave(objXpk) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2181;
		}
		xpkClose(objXpk);

		objXpk = xpkOpen(sPathPkgCodec, NULL);
		if ( objXpk == NULL ) {
			xpkFreeInternal(pDataCodecLarge);
			return 2182;
		}
		if ( xpkVerifyAll(objXpk) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2183;
		}
		procTestDeletePathFamily(sPathFileExport);
		if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2184;
		}
		if ( !procTestFileContentEquals(sPathFileExport, pDataCodecLarge, iDataCodecLarge) ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2185;
		}
		xpkFreeInternal(pDataCodecLarge);
		xpkClose(objXpk);
	}

	{
		void* pDataCodecLarge;
		uint32_t iDataCodecLarge;

		pDataCodecLarge = NULL;
		iDataCodecLarge = (3u * 1024u * 1024u) + 517u;

		procTestDeletePathFamily(sPathPkgCodec);
		memset(&objOpt, 0, sizeof(objOpt));
		objOpt.createIfMissing = TRUE;
		objXpk = xpkOpen(sPathPkgCodec, &objOpt);
		if ( objXpk == NULL ) {
			return 2186;
		}

		pDataCodecLarge = xpkAllocInternal(iDataCodecLarge);
		if ( pDataCodecLarge == NULL ) {
			xpkClose(objXpk);
			return 2187;
		}
		memset(pDataCodecLarge, 'V', iDataCodecLarge);

		memset(&objWriteOpt, 0, sizeof(objWriteOpt));
		objWriteOpt.compLevel = 0;
		objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
		if ( xpkAddData(objXpk, "KEEP-UPDATE-LZ4", sizeof("KEEP-UPDATE-LZ4"), &objWriteOpt, &iPosRet) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2188;
		}
		objWriteOpt.compLevel = 2;
		objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
		if ( xpkUpdateData(objXpk, 1, pDataCodecLarge, iDataCodecLarge, &objWriteOpt) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2189;
		}
		if ( xpkSave(objXpk) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2190;
		}
		xpkClose(objXpk);

		objXpk = xpkOpen(sPathPkgCodec, NULL);
		if ( objXpk == NULL ) {
			xpkFreeInternal(pDataCodecLarge);
			return 2191;
		}
		if ( xpkVerifyAll(objXpk) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2192;
		}
		procTestDeletePathFamily(sPathFileExport);
		if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2193;
		}
		if ( !procTestFileContentEquals(sPathFileExport, pDataCodecLarge, iDataCodecLarge) ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2194;
		}
		xpkFreeInternal(pDataCodecLarge);
		xpkClose(objXpk);
	}

	{
		void* pDataCodecLarge;
		uint32_t iDataCodecLarge;

		pDataCodecLarge = NULL;
		iDataCodecLarge = (1024u * 1024u) + 991u;

		procTestDeletePathFamily(sPathPkgCodec);
		memset(&objOpt, 0, sizeof(objOpt));
		objOpt.createIfMissing = TRUE;
		objXpk = xpkOpen(sPathPkgCodec, &objOpt);
		if ( objXpk == NULL ) {
			return 2195;
		}

		pDataCodecLarge = xpkAllocInternal(iDataCodecLarge);
		if ( pDataCodecLarge == NULL ) {
			xpkClose(objXpk);
			return 2196;
		}
		memset(pDataCodecLarge, 'W', iDataCodecLarge);

		memset(&objWriteOpt, 0, sizeof(objWriteOpt));
		objWriteOpt.compLevel = 0;
		objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
		if ( xpkAddData(objXpk, "KEEP-UPDATE-LZMA2", sizeof("KEEP-UPDATE-LZMA2"), &objWriteOpt, &iPosRet) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2197;
		}
		objWriteOpt.compLevel = 14;
		objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
		if ( xpkUpdateData(objXpk, 1, pDataCodecLarge, iDataCodecLarge, &objWriteOpt) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2198;
		}
		if ( xpkSave(objXpk) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2199;
		}
		xpkClose(objXpk);

		objXpk = xpkOpen(sPathPkgCodec, NULL);
		if ( objXpk == NULL ) {
			xpkFreeInternal(pDataCodecLarge);
			return 2200;
		}
		if ( xpkVerifyAll(objXpk) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2201;
		}
		procTestDeletePathFamily(sPathFileExport);
		if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2202;
		}
		if ( !procTestFileContentEquals(sPathFileExport, pDataCodecLarge, iDataCodecLarge) ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2203;
		}
		xpkFreeInternal(pDataCodecLarge);
		xpkClose(objXpk);
	}

	{
		void* pDataCodecLarge;
		uint32_t iDataCodecLarge;

		pDataCodecLarge = NULL;
		iDataCodecLarge = (3u * 1024u * 1024u) + 321u;

		procTestDeletePathFamily(sPathPkgCodec);
		memset(&objOpt, 0, sizeof(objOpt));
		objOpt.createIfMissing = TRUE;
		objXpk = xpkOpen(sPathPkgCodec, &objOpt);
		if ( objXpk == NULL ) {
			return 2161;
		}

		pDataCodecLarge = xpkAllocInternal(iDataCodecLarge);
		if ( pDataCodecLarge == NULL ) {
			xpkClose(objXpk);
			return 2162;
		}
		memset(pDataCodecLarge, 'L', iDataCodecLarge);

		memset(&objWriteOpt, 0, sizeof(objWriteOpt));
		objWriteOpt.compLevel = 2;
		objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
		if ( xpkAddData(objXpk, pDataCodecLarge, iDataCodecLarge, &objWriteOpt, &iPosRet) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2163;
		}
		if ( xpkSave(objXpk) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2164;
		}
		xpkClose(objXpk);

		objXpk = xpkOpen(sPathPkgCodec, NULL);
		if ( objXpk == NULL ) {
			xpkFreeInternal(pDataCodecLarge);
			return 2165;
		}
		if ( xpkVerifyAll(objXpk) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2166;
		}
		procTestDeletePathFamily(sPathFileExport);
		if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2167;
		}
		if ( !procTestFileContentEquals(sPathFileExport, pDataCodecLarge, iDataCodecLarge) ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2168;
		}
		xpkFreeInternal(pDataCodecLarge);
		xpkClose(objXpk);
	}

	{
		void* pDataCodecLarge;
		uint32_t iDataCodecLarge;

		pDataCodecLarge = NULL;
		iDataCodecLarge = (1024u * 1024u) + 777u;

		procTestDeletePathFamily(sPathPkgCodec);
		memset(&objOpt, 0, sizeof(objOpt));
		objOpt.createIfMissing = TRUE;
		objXpk = xpkOpen(sPathPkgCodec, &objOpt);
		if ( objXpk == NULL ) {
			return 2169;
		}

		pDataCodecLarge = xpkAllocInternal(iDataCodecLarge);
		if ( pDataCodecLarge == NULL ) {
			xpkClose(objXpk);
			return 2170;
		}
		memset(pDataCodecLarge, 'M', iDataCodecLarge);

		memset(&objWriteOpt, 0, sizeof(objWriteOpt));
		objWriteOpt.compLevel = 14;
		objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
		if ( xpkAddData(objXpk, pDataCodecLarge, iDataCodecLarge, &objWriteOpt, &iPosRet) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2171;
		}
		if ( xpkSave(objXpk) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2172;
		}
		xpkClose(objXpk);

		objXpk = xpkOpen(sPathPkgCodec, NULL);
		if ( objXpk == NULL ) {
			xpkFreeInternal(pDataCodecLarge);
			return 2173;
		}
		if ( xpkVerifyAll(objXpk) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2174;
		}
		procTestDeletePathFamily(sPathFileExport);
		if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2175;
		}
		if ( !procTestFileContentEquals(sPathFileExport, pDataCodecLarge, iDataCodecLarge) ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2176;
		}
		xpkFreeInternal(pDataCodecLarge);
		xpkClose(objXpk);
	}

	procTestDeletePathFamily(sPathPkgCodec);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgCodec, &objOpt);
	if ( objXpk == NULL ) {
		return 2108;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 4;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddData(objXpk, sDataBuildA, sizeof(sDataBuildA), &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 2109;
	}
	if ( iPosRet != 1 ) {
		xpkClose(objXpk);
		return 2110;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 2111;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgCodec, NULL);
	if ( objXpk == NULL ) {
		return 2112;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 2113;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 2114;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataBuildA, sizeof(sDataBuildA)) ) {
		xpkClose(objXpk);
		return 2115;
	}
	xpkClose(objXpk);

	procTestDeletePathFamily(sPathPkgCodec);
	procTestDeletePathFamily(sPathFileCodecSrc);
	if ( !procTestWriteBinaryFile(sPathFileCodecSrc, sDataBuildB, sizeof(sDataBuildB)) ) {
		return 1656;
	}
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgCodec, &objOpt);
	if ( objXpk == NULL ) {
		return 1657;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 14;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddFile(objXpk, sPathFileCodecSrc, &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1658;
	}
	if ( iPosRet != 1 ) {
		xpkClose(objXpk);
		return 1659;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1660;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgCodec, NULL);
	if ( objXpk == NULL ) {
		return 1661;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1662;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		xpkClose(objXpk);
		return 1663;
	}
	if ( (objInfo.flag & XPK_FLAG_COMP_MASK) != 14 ) {
		xpkClose(objXpk);
		return 1664;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1665;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataBuildB, sizeof(sDataBuildB)) ) {
		xpkClose(objXpk);
		return 1666;
	}
	xpkClose(objXpk);

	procTestDeletePathFamily(sPathPkgCodec);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgCodec, &objOpt);
	if ( objXpk == NULL ) {
		return 1685;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 2;
	objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
	if ( xpkAddFile(objXpk, sPathFileCodecSrc, &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1686;
	}
	if ( iPosRet != 1 ) {
		xpkClose(objXpk);
		return 1687;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1688;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgCodec, NULL);
	if ( objXpk == NULL ) {
		return 1689;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1690;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		xpkClose(objXpk);
		return 1691;
	}
	if ( (objInfo.flag & XPK_FLAG_COMP_MASK) != 2 ) {
		xpkClose(objXpk);
		return 1692;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1693;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataBuildB, sizeof(sDataBuildB)) ) {
		xpkClose(objXpk);
		return 1694;
	}
	if ( !procTestWriteBinaryFile(sPathFileCodecSrc, sDataBuildA, sizeof(sDataBuildA)) ) {
		xpkClose(objXpk);
		return 1699;
	}
	if ( xpkUpdateFile(objXpk, 1, sPathFileCodecSrc, &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 1700;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1701;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgCodec, NULL);
	if ( objXpk == NULL ) {
		return 1702;
	}
	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 1703;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1704;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataBuildA, sizeof(sDataBuildA)) ) {
		xpkClose(objXpk);
		return 1705;
	}
	xpkClose(objXpk);

	procTestDeletePathFamily(sPathPkgCodec);
	if ( !procTestWriteBinaryFile(sPathFileCodecSrc, sDataBuildB, sizeof(sDataBuildB)) ) {
		return 1710;
	}
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgCodec, &objOpt);
	if ( objXpk == NULL ) {
		return 1667;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 14;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkAddFile(objXpk, sPathFileCodecSrc, &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1668;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1669;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataBuildB, sizeof(sDataBuildB)) ) {
		xpkClose(objXpk);
		return 1670;
	}
	xpkClose(objXpk);

	procTestDeletePathFamily(sPathPkgCodec);
	if ( !procTestWriteBinaryFile(sPathFileCodecSrc, sDataBuildB, sizeof(sDataBuildB)) ) {
		return 1711;
	}
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgCodec, &objOpt);
	if ( objXpk == NULL ) {
		return 1695;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 2;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkAddFile(objXpk, sPathFileCodecSrc, &objWriteOpt, &iPosRet) != XPK_OK ) {
		xpkClose(objXpk);
		return 1696;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1697;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataBuildB, sizeof(sDataBuildB)) ) {
		xpkClose(objXpk);
		return 1698;
	}
	if ( !procTestWriteBinaryFile(sPathFileCodecSrc, sDataBuildA, sizeof(sDataBuildA)) ) {
		xpkClose(objXpk);
		return 1706;
	}
	if ( xpkUpdateFile(objXpk, 1, sPathFileCodecSrc, &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 1707;
	}
	procTestDeletePathFamily(sPathFileExport);
	if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 1708;
	}
	if ( !procTestFileContentEquals(sPathFileExport, sDataBuildA, sizeof(sDataBuildA)) ) {
		xpkClose(objXpk);
		return 1709;
	}
	xpkClose(objXpk);

	{
		void* pDataCodecLarge;
		uint32_t iDataCodecLarge;

		pDataCodecLarge = NULL;
		iDataCodecLarge = (3u * 1024u * 1024u) + 645u;

		pDataCodecLarge = xpkAllocInternal(iDataCodecLarge);
		if ( pDataCodecLarge == NULL ) {
			return 2204;
		}
		memset(pDataCodecLarge, 'Y', iDataCodecLarge);

		procTestDeletePathFamily(sPathPkgCodec);
		procTestDeletePathFamily(sPathFileCodecSrc);
		if ( !procTestWriteBinaryFile(sPathFileCodecSrc, sDataBuildA, sizeof(sDataBuildA)) ) {
			xpkFreeInternal(pDataCodecLarge);
			return 2205;
		}
		memset(&objOpt, 0, sizeof(objOpt));
		objOpt.createIfMissing = TRUE;
		objXpk = xpkOpen(sPathPkgCodec, &objOpt);
		if ( objXpk == NULL ) {
			xpkFreeInternal(pDataCodecLarge);
			return 2206;
		}
		memset(&objWriteOpt, 0, sizeof(objWriteOpt));
		objWriteOpt.compLevel = 0;
		objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
		if ( xpkAddFile(objXpk, sPathFileCodecSrc, &objWriteOpt, &iPosRet) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2207;
		}
		if ( !procTestWriteBinaryFile(sPathFileCodecSrc, pDataCodecLarge, iDataCodecLarge) ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2208;
		}
		objWriteOpt.compLevel = 6;
		if ( xpkUpdateFile(objXpk, 1, sPathFileCodecSrc, &objWriteOpt) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2209;
		}
		if ( xpkSave(objXpk) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2210;
		}
		xpkClose(objXpk);

		objXpk = xpkOpen(sPathPkgCodec, NULL);
		if ( objXpk == NULL ) {
			xpkFreeInternal(pDataCodecLarge);
			return 2211;
		}
		if ( xpkVerifyAll(objXpk) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2212;
		}
		procTestDeletePathFamily(sPathFileExport);
		if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2213;
		}
		if ( !procTestFileContentEquals(sPathFileExport, pDataCodecLarge, iDataCodecLarge) ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2214;
		}
		xpkFreeInternal(pDataCodecLarge);
		xpkClose(objXpk);
	}

	{
		void* pDataCodecLarge;
		uint32_t iDataCodecLarge;

		pDataCodecLarge = NULL;
		iDataCodecLarge = (1024u * 1024u) + 433u;

		pDataCodecLarge = xpkAllocInternal(iDataCodecLarge);
		if ( pDataCodecLarge == NULL ) {
			return 2215;
		}
		memset(pDataCodecLarge, 'Z', iDataCodecLarge);

		procTestDeletePathFamily(sPathPkgCodec);
		procTestDeletePathFamily(sPathFileCodecSrc);
		if ( !procTestWriteBinaryFile(sPathFileCodecSrc, sDataBuildB, sizeof(sDataBuildB)) ) {
			xpkFreeInternal(pDataCodecLarge);
			return 2216;
		}
		memset(&objOpt, 0, sizeof(objOpt));
		objOpt.createIfMissing = TRUE;
		objXpk = xpkOpen(sPathPkgCodec, &objOpt);
		if ( objXpk == NULL ) {
			xpkFreeInternal(pDataCodecLarge);
			return 2217;
		}
		memset(&objWriteOpt, 0, sizeof(objWriteOpt));
		objWriteOpt.compLevel = 0;
		objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
		if ( xpkAddFile(objXpk, sPathFileCodecSrc, &objWriteOpt, &iPosRet) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2218;
		}
		if ( !procTestWriteBinaryFile(sPathFileCodecSrc, pDataCodecLarge, iDataCodecLarge) ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2219;
		}
		objWriteOpt.compLevel = 14;
		objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
		if ( xpkUpdateFile(objXpk, 1, sPathFileCodecSrc, &objWriteOpt) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2220;
		}
		if ( xpkSave(objXpk) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2221;
		}
		xpkClose(objXpk);

		objXpk = xpkOpen(sPathPkgCodec, NULL);
		if ( objXpk == NULL ) {
			xpkFreeInternal(pDataCodecLarge);
			return 2222;
		}
		if ( xpkVerifyAll(objXpk) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2223;
		}
		procTestDeletePathFamily(sPathFileExport);
		if ( xpkReadToFile(objXpk, 1, sPathFileExport) != XPK_OK ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2224;
		}
		if ( !procTestFileContentEquals(sPathFileExport, pDataCodecLarge, iDataCodecLarge) ) {
			xpkFreeInternal(pDataCodecLarge);
			xpkClose(objXpk);
			return 2225;
		}
		xpkFreeInternal(pDataCodecLarge);
		xpkClose(objXpk);
	}

	if ( bRunStress ) {
		xpkObject objCodecStress;
		char* sPathCodecStress;
		void* pDataCodecStress;
		uint32_t iCodecStressRound;
		uint32_t iCodecStressSize;

		objCodecStress = NULL;
		sPathCodecStress = procXpkPathSuffixDupText(sPathPkgCodec, ".stressloop");
		pDataCodecStress = NULL;
		if ( sPathCodecStress == NULL ) {
			return 2320;
		}
		procTestDeletePathFamily(sPathCodecStress);

		pDataCodecStress = xpkAllocInternal(262144u);
		if ( pDataCodecStress == NULL ) {
			xpkFreeInternal(sPathCodecStress);
			return 2321;
		}

		memset(&objOpt, 0, sizeof(objOpt));
		objOpt.createIfMissing = TRUE;
		objCodecStress = xpkOpen(sPathCodecStress, &objOpt);
		if ( objCodecStress == NULL ) {
			xpkFreeInternal(pDataCodecStress);
			xpkFreeInternal(sPathCodecStress);
			return 2322;
		}

		iCodecStressSize = 70000u;
		memset(pDataCodecStress, 'A', iCodecStressSize);
		memset(&objWriteOpt, 0, sizeof(objWriteOpt));
		objWriteOpt.compLevel = 2;
		objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
		if ( xpkAddData(objCodecStress, pDataCodecStress, iCodecStressSize, &objWriteOpt, &iPosRet) != XPK_OK ) {
			xpkClose(objCodecStress);
			procTestDeletePathFamily(sPathCodecStress);
			xpkFreeInternal(pDataCodecStress);
			xpkFreeInternal(sPathCodecStress);
			return 2323;
		}

		iCodecStressSize = 80000u;
		memset(pDataCodecStress, 'B', iCodecStressSize);
		memset(&objWriteOpt, 0, sizeof(objWriteOpt));
		objWriteOpt.compLevel = 6;
		objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
		if ( xpkAddData(objCodecStress, pDataCodecStress, iCodecStressSize, &objWriteOpt, &iPosRet) != XPK_OK ) {
			xpkClose(objCodecStress);
			procTestDeletePathFamily(sPathCodecStress);
			xpkFreeInternal(pDataCodecStress);
			xpkFreeInternal(sPathCodecStress);
			return 2324;
		}

		iCodecStressSize = 90000u;
		memset(pDataCodecStress, 'C', iCodecStressSize);
		memset(&objWriteOpt, 0, sizeof(objWriteOpt));
		objWriteOpt.compLevel = 14;
		objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
		if ( xpkAddData(objCodecStress, pDataCodecStress, iCodecStressSize, &objWriteOpt, &iPosRet) != XPK_OK ) {
			xpkClose(objCodecStress);
			procTestDeletePathFamily(sPathCodecStress);
			xpkFreeInternal(pDataCodecStress);
			xpkFreeInternal(sPathCodecStress);
			return 2325;
		}
		if ( xpkSave(objCodecStress) != XPK_OK ) {
			xpkClose(objCodecStress);
			procTestDeletePathFamily(sPathCodecStress);
			xpkFreeInternal(pDataCodecStress);
			xpkFreeInternal(sPathCodecStress);
			return 2326;
		}
		xpkClose(objCodecStress);

		for ( iCodecStressRound = 0; iCodecStressRound < 3u; iCodecStressRound++ ) {
			objCodecStress = xpkOpen(sPathCodecStress, NULL);
			if ( objCodecStress == NULL ) {
				procTestDeletePathFamily(sPathCodecStress);
				xpkFreeInternal(pDataCodecStress);
				xpkFreeInternal(sPathCodecStress);
				return 2327;
			}
			if ( xpkVerifyAll(objCodecStress) != XPK_OK ) {
				xpkClose(objCodecStress);
				procTestDeletePathFamily(sPathCodecStress);
				xpkFreeInternal(pDataCodecStress);
				xpkFreeInternal(sPathCodecStress);
				return 2328;
			}

			iCodecStressSize = 100000u + (iCodecStressRound * 257u);
			memset(pDataCodecStress, (int)('D' + iCodecStressRound), iCodecStressSize);
			memset(&objWriteOpt, 0, sizeof(objWriteOpt));
			objWriteOpt.compLevel = 2;
			objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
			if ( xpkUpdateData(objCodecStress, 1, pDataCodecStress, iCodecStressSize, &objWriteOpt) != XPK_OK ) {
				xpkClose(objCodecStress);
				procTestDeletePathFamily(sPathCodecStress);
				xpkFreeInternal(pDataCodecStress);
				xpkFreeInternal(sPathCodecStress);
				return 2329;
			}

			iCodecStressSize = 110000u + (iCodecStressRound * 257u);
			memset(pDataCodecStress, (int)('G' + iCodecStressRound), iCodecStressSize);
			memset(&objWriteOpt, 0, sizeof(objWriteOpt));
			objWriteOpt.compLevel = 6;
			objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
			if ( xpkUpdateData(objCodecStress, 2, pDataCodecStress, iCodecStressSize, &objWriteOpt) != XPK_OK ) {
				xpkClose(objCodecStress);
				procTestDeletePathFamily(sPathCodecStress);
				xpkFreeInternal(pDataCodecStress);
				xpkFreeInternal(sPathCodecStress);
				return 2330;
			}

			iCodecStressSize = 120000u + (iCodecStressRound * 257u);
			memset(pDataCodecStress, (int)('J' + iCodecStressRound), iCodecStressSize);
			memset(&objWriteOpt, 0, sizeof(objWriteOpt));
			objWriteOpt.compLevel = 14;
			objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
			if ( xpkUpdateData(objCodecStress, 3, pDataCodecStress, iCodecStressSize, &objWriteOpt) != XPK_OK ) {
				xpkClose(objCodecStress);
				procTestDeletePathFamily(sPathCodecStress);
				xpkFreeInternal(pDataCodecStress);
				xpkFreeInternal(sPathCodecStress);
				return 2331;
			}

			if ( xpkSave(objCodecStress) != XPK_OK ) {
				xpkClose(objCodecStress);
				procTestDeletePathFamily(sPathCodecStress);
				xpkFreeInternal(pDataCodecStress);
				xpkFreeInternal(sPathCodecStress);
				return 2332;
			}
			xpkClose(objCodecStress);

			objCodecStress = xpkOpen(sPathCodecStress, NULL);
			if ( objCodecStress == NULL ) {
				procTestDeletePathFamily(sPathCodecStress);
				xpkFreeInternal(pDataCodecStress);
				xpkFreeInternal(sPathCodecStress);
				return 2333;
			}
			if ( xpkVerifyAll(objCodecStress) != XPK_OK ) {
				xpkClose(objCodecStress);
				procTestDeletePathFamily(sPathCodecStress);
				xpkFreeInternal(pDataCodecStress);
				xpkFreeInternal(sPathCodecStress);
				return 2334;
			}
			iCodecStressSize = 120000u + (iCodecStressRound * 257u);
			memset(pDataCodecStress, (int)('J' + iCodecStressRound), iCodecStressSize);
			procTestDeletePathFamily(sPathFileExport);
			if ( xpkReadToFile(objCodecStress, 3, sPathFileExport) != XPK_OK ) {
				xpkClose(objCodecStress);
				procTestDeletePathFamily(sPathCodecStress);
				xpkFreeInternal(pDataCodecStress);
				xpkFreeInternal(sPathCodecStress);
				return 2335;
			}
			if ( !procTestFileContentEquals(sPathFileExport, pDataCodecStress, iCodecStressSize) ) {
				xpkClose(objCodecStress);
				procTestDeletePathFamily(sPathCodecStress);
				xpkFreeInternal(pDataCodecStress);
				xpkFreeInternal(sPathCodecStress);
				return 2336;
			}
			xpkClose(objCodecStress);
		}

		procTestDeletePathFamily(sPathCodecStress);
		procTestDeletePathFamily(sPathFileCodecSrc);
		if ( !procTestWriteBinaryFile(sPathFileCodecSrc, sDataBuildA, sizeof(sDataBuildA)) ) {
			xpkFreeInternal(pDataCodecStress);
			xpkFreeInternal(sPathCodecStress);
			return 2337;
		}
		memset(&objOpt, 0, sizeof(objOpt));
		objOpt.createIfMissing = TRUE;
		objCodecStress = xpkOpen(sPathCodecStress, &objOpt);
		if ( objCodecStress == NULL ) {
			xpkFreeInternal(pDataCodecStress);
			xpkFreeInternal(sPathCodecStress);
			return 2338;
		}
		memset(&objWriteOpt, 0, sizeof(objWriteOpt));
		objWriteOpt.compLevel = 6;
		objWriteOpt.writePolicy = XPK_WRITE_IMMEDIATE;
		if ( xpkAddFile(objCodecStress, sPathFileCodecSrc, &objWriteOpt, &iPosRet) != XPK_OK ) {
			xpkClose(objCodecStress);
			procTestDeletePathFamily(sPathCodecStress);
			xpkFreeInternal(pDataCodecStress);
			xpkFreeInternal(sPathCodecStress);
			return 2339;
		}
		if ( xpkSave(objCodecStress) != XPK_OK ) {
			xpkClose(objCodecStress);
			procTestDeletePathFamily(sPathCodecStress);
			xpkFreeInternal(pDataCodecStress);
			xpkFreeInternal(sPathCodecStress);
			return 2340;
		}
		xpkClose(objCodecStress);

		for ( iCodecStressRound = 0; iCodecStressRound < 2u; iCodecStressRound++ ) {
			iCodecStressSize = 131072u + (iCodecStressRound * 4096u);
			memset(pDataCodecStress, (int)('P' + iCodecStressRound), iCodecStressSize);
			if ( !procTestWriteBinaryFile(sPathFileCodecSrc, pDataCodecStress, iCodecStressSize) ) {
				procTestDeletePathFamily(sPathCodecStress);
				xpkFreeInternal(pDataCodecStress);
				xpkFreeInternal(sPathCodecStress);
				return 2341;
			}

			objCodecStress = xpkOpen(sPathCodecStress, NULL);
			if ( objCodecStress == NULL ) {
				procTestDeletePathFamily(sPathCodecStress);
				xpkFreeInternal(pDataCodecStress);
				xpkFreeInternal(sPathCodecStress);
				return 2342;
			}
			if ( xpkVerifyAll(objCodecStress) != XPK_OK ) {
				xpkClose(objCodecStress);
				procTestDeletePathFamily(sPathCodecStress);
				xpkFreeInternal(pDataCodecStress);
				xpkFreeInternal(sPathCodecStress);
				return 2343;
			}

			memset(&objWriteOpt, 0, sizeof(objWriteOpt));
			objWriteOpt.compLevel = (iCodecStressRound == 0u) ? 2 : 14;
			objWriteOpt.writePolicy = (iCodecStressRound == 0u) ? XPK_WRITE_IMMEDIATE : XPK_WRITE_BUFFERED;
			if ( xpkUpdateFile(objCodecStress, 1, sPathFileCodecSrc, &objWriteOpt) != XPK_OK ) {
				xpkClose(objCodecStress);
				procTestDeletePathFamily(sPathCodecStress);
				xpkFreeInternal(pDataCodecStress);
				xpkFreeInternal(sPathCodecStress);
				return 2344;
			}
			if ( xpkSave(objCodecStress) != XPK_OK ) {
				xpkClose(objCodecStress);
				procTestDeletePathFamily(sPathCodecStress);
				xpkFreeInternal(pDataCodecStress);
				xpkFreeInternal(sPathCodecStress);
				return 2345;
			}
			xpkClose(objCodecStress);

			objCodecStress = xpkOpen(sPathCodecStress, NULL);
			if ( objCodecStress == NULL ) {
				procTestDeletePathFamily(sPathCodecStress);
				xpkFreeInternal(pDataCodecStress);
				xpkFreeInternal(sPathCodecStress);
				return 2346;
			}
			if ( xpkVerifyAll(objCodecStress) != XPK_OK ) {
				xpkClose(objCodecStress);
				procTestDeletePathFamily(sPathCodecStress);
				xpkFreeInternal(pDataCodecStress);
				xpkFreeInternal(sPathCodecStress);
				return 2347;
			}
			procTestDeletePathFamily(sPathFileExport);
			if ( xpkReadToFile(objCodecStress, 1, sPathFileExport) != XPK_OK ) {
				xpkClose(objCodecStress);
				procTestDeletePathFamily(sPathCodecStress);
				xpkFreeInternal(pDataCodecStress);
				xpkFreeInternal(sPathCodecStress);
				return 2348;
			}
			if ( !procTestFileContentEquals(sPathFileExport, pDataCodecStress, iCodecStressSize) ) {
				xpkClose(objCodecStress);
				procTestDeletePathFamily(sPathCodecStress);
				xpkFreeInternal(pDataCodecStress);
				xpkFreeInternal(sPathCodecStress);
				return 2349;
			}
			xpkClose(objCodecStress);
		}

		procTestDeletePathFamily(sPathCodecStress);
		xpkFreeInternal(pDataCodecStress);
		xpkFreeInternal(sPathCodecStress);
	}

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgVolume, &objOpt);
