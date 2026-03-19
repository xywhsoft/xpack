	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgFormat, &objOpt);
	if ( objXpk == NULL ) {
		return 250;
	}
	if ( xpkMetaSet(objXpk, sMetaTest, sizeof(sMetaTest), 6) != XPK_OK ) {
		xpkClose(objXpk);
		return 251;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 252;
	}
	xpkClose(objXpk);

	hFile = xrtOpen((str)sPathPkgFormat, FALSE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		return 253;
	}
	xrtSeek(hFile, 32, XRT_SEEK_SET);
	iMetaSize = 0;
	if ( xrtWrite(hFile, (str)&iMetaSize, sizeof(iMetaSize)) != sizeof(iMetaSize) ) {
		xrtClose(hFile);
		return 254;
	}
	xrtClose(hFile);

	objXpk = xpkOpen(sPathPkgFormat, NULL);
	if ( objXpk != NULL ) {
		xpkClose(objXpk);
		return 255;
	}
	remove(sPathPkgFormat);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgFormat, &objOpt);
	if ( objXpk == NULL ) {
		return 256;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_WIN32) != XPK_OK ) {
		xpkClose(objXpk);
		return 257;
	}
	if ( xpkSetInfoComp(objXpk, 0) != XPK_OK ) {
		xpkClose(objXpk);
		return 258;
	}
	if ( xpkPathAddData(objXpk, "assets/format.txt", sDataPath, sizeof(sDataPath), NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 259;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 260;
	}
	iBuildSizeBefore = objXpk->objHead.dataOffset + objXpk->objHead.metaCompSize;
	xpkClose(objXpk);

	hFile = xrtOpen((str)sPathPkgFormat, FALSE, XRT_CP_BINARY);
	if ( hFile == NULL ) {
		return 261;
	}
	xrtSeek(hFile, (long)(iBuildSizeBefore + 32), XRT_SEEK_SET);
	if ( xrtWrite(hFile, (str)".", 1) != 1 ) {
		xrtClose(hFile);
		return 262;
	}
	if ( xrtWrite(hFile, (str)"\0", 1) != 1 ) {
		xrtClose(hFile);
		return 263;
	}
	xrtClose(hFile);

	objXpk = xpkOpen(sPathPkgFormat, NULL);
	if ( objXpk != NULL ) {
		xpkClose(objXpk);
		return 264;
	}
	remove(sPathPkgFormat);
