	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathPkgLinux, &objOpt);
	if ( objXpk == NULL ) {
		return 246;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_LINUX) != XPK_OK ) {
		xpkClose(objXpk);
		return 247;
	}

	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 6;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkPathAddData(objXpk, "./assets//hero.txt", sDataPath, sizeof(sDataPath), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 248;
	}
	if ( !xpkPathExists(objXpk, "assets/hero.txt") ) {
		xpkClose(objXpk);
		return 249;
	}
	if ( !xpkPathExists(objXpk, "./assets//hero.txt") ) {
		xpkClose(objXpk);
		return 250;
	}
	if ( xpkPathExists(objXpk, "ASSETS/HERO.TXT") ) {
		xpkClose(objXpk);
		return 251;
	}
	if ( xpkPathExists(objXpk, "assets\\hero.txt") ) {
		xpkClose(objXpk);
		return 252;
	}
	if ( xpkPathGetInfo(objXpk, "assets/hero.txt", &objInfoPath) != XPK_OK ) {
		xpkClose(objXpk);
		return 253;
	}
	if ( strcmp((const char*)objInfoPath.pathBytes, "assets/hero.txt") != 0 ) {
		xpkClose(objXpk);
		return 254;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 255;
	}
	xpkClose(objXpk);

	objXpk = xpkOpen(sPathPkgLinux, NULL);
	if ( objXpk == NULL ) {
		return 256;
	}
	if ( !xpkPathExists(objXpk, "assets/hero.txt") ) {
		xpkClose(objXpk);
		return 257;
	}
	if ( !xpkPathExists(objXpk, "./assets//hero.txt") ) {
		xpkClose(objXpk);
		return 258;
	}
	if ( xpkPathExists(objXpk, "ASSETS/HERO.TXT") ) {
		xpkClose(objXpk);
		return 259;
	}
	if ( xpkPathExists(objXpk, "assets\\hero.txt") ) {
		xpkClose(objXpk);
		return 260;
	}
	if ( xpkPathGetInfo(objXpk, "assets/hero.txt", &objInfoPath) != XPK_OK ) {
		xpkClose(objXpk);
		return 261;
	}
	if ( strcmp((const char*)objInfoPath.pathBytes, "assets/hero.txt") != 0 ) {
		xpkClose(objXpk);
		return 262;
	}
	iMatchCount = 0;
	if ( xpkEachMatch(objXpk, "assets/*.txt", NULL, &iMatchCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 263;
	}
	if ( iMatchCount != 1 ) {
		xpkClose(objXpk);
		return 264;
	}
	iMatchCount = 0;
	if ( xpkEachMatch(objXpk, "*.TXT", NULL, &iMatchCount) != XPK_OK ) {
		xpkClose(objXpk);
		return 265;
	}
	if ( iMatchCount != 0 ) {
		xpkClose(objXpk);
		return 266;
	}
	xpkClose(objXpk);
