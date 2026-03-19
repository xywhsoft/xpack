	objXpk = xpkOpen(sPathPkgReadonly, &objOpt);
	if ( objXpk == NULL ) {
		return 352;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_WIN32) != XPK_OK ) {
		xpkClose(objXpk);
		return 353;
	}
	if ( xpkMetaSet(objXpk, sMetaReadonly, sizeof(sMetaReadonly), 6) != XPK_OK ) {
		xpkClose(objXpk);
		return 354;
	}
	if ( xpkPathAddData(objXpk, "assets/readonly.txt", sDataReadonly, sizeof(sDataReadonly), NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 355;
	}
	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 356;
	}
	xpkClose(objXpk);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.readonly = TRUE;
	objXpk = xpkOpen(sPathPkgReadonly, &objOpt);
	if ( objXpk == NULL ) {
		return 357;
	}
	iRetCall = xpkSetDefaultComp(objXpk, 9);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_READONLY, objXpk, XPK_ERR_READONLY, sXpkErrorReadonly) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_READONLY) ? 358 : 359;
	}
	if ( xpkGetDefaultComp(objXpk, &objWriteOpt.compLevel) != XPK_OK ) {
		xpkClose(objXpk);
		return 360;
	}
	if ( objWriteOpt.compLevel != 6 ) {
		xpkClose(objXpk);
		return 361;
	}
	if ( xpkMetaClear(objXpk) != XPK_ERR_READONLY ) {
		xpkClose(objXpk);
		return 362;
	}
	pMetaRead = xpkMetaGet(objXpk, &iMetaSize);
	if ( pMetaRead == NULL ) {
		xpkClose(objXpk);
		return 363;
	}
	if ( iMetaSize != sizeof(sMetaReadonly) ) {
		xpkFree(pMetaRead);
		xpkClose(objXpk);
		return 364;
	}
	if ( memcmp(pMetaRead, sMetaReadonly, sizeof(sMetaReadonly)) != 0 ) {
		xpkFree(pMetaRead);
		xpkClose(objXpk);
		return 365;
	}
	xpkFree(pMetaRead);
	if ( xpkPathSetAttr(objXpk, "assets/readonly.txt", 0x20u) != XPK_ERR_READONLY ) {
		xpkClose(objXpk);
		return 366;
	}
	iRetCall = xpkSetDefaultComp(objXpk, 16);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 591 : 592;
	}
	iRetCall = xpkSetVolumeSize(objXpk, 1);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, "volume size is below minimum 64KB") ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 593 : 594;
	}
	iRetCall = xpkMetaSet(objXpk, NULL, 1, 6);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 595 : 596;
	}
	iRetCall = xpkSetPackType(objXpk, (xpkPackType)9);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 597 : 598;
	}
	iRetCall = xpkGetInfoExt(objXpk, 0, arrInfoExtCoreRead, sizeof(arrInfoExtCoreRead));
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorCoreExtUnsupported) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 682 : 683;
	}
	iRetCall = xpkSetInfoExt(objXpk, 0, arrInfoExtCoreRead, sizeof(arrInfoExtCoreRead));
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorCoreExtUnsupported) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 684 : 685;
	}
	iRetCall = xpkReadToFile(objXpk, 1, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 651 : 652;
	}
	iRetCall = xpkPathReadToFile(objXpk, "assets/readonly.txt", NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 653 : 654;
	}
	iRetCall = xpkAddFile(objXpk, NULL, NULL, &iPosRet);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 618 : 619;
	}
	iRetCall = xpkAddData(objXpk, NULL, 1, NULL, &iPosRet);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 620 : 621;
	}
	iRetCall = xpkUpdateFile(objXpk, 1, NULL, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 622 : 623;
	}
	iRetCall = xpkUpdateData(objXpk, 1, NULL, 1, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 624 : 625;
	}
	iRetCall = xpkPathAddFile(objXpk, "assets/readonly_new.txt", NULL, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 626 : 627;
	}
	iRetCall = xpkPathAddData(objXpk, "./", NULL, 1, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorPathEmpty) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 665 : 666;
	}
	iRetCall = xpkPathAddData(objXpk, "assets/readonly_new.txt", NULL, 1, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 628 : 629;
	}
	iRetCall = xpkPathUpdateFile(objXpk, "assets/readonly.txt", NULL, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 630 : 631;
	}
	iRetCall = xpkPathUpdateFile(objXpk, sPathTooLong, NULL, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorPathTooLong) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 667 : 668;
	}
	iRetCall = xpkPathUpdateData(objXpk, "assets/readonly.txt", NULL, 1, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 632 : 633;
	}
	iRetCall = xpkPathRename(objXpk, NULL, "assets/readonly_new.txt");
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 643 : 644;
	}
	iRetCall = xpkPathRemove(objXpk, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 645 : 646;
	}
	iRetCall = xpkPathSetAttr(objXpk, NULL, 0x20u);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 647 : 648;
	}
	iRetCall = xpkPathReadToFile(objXpk, "./", NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorPathEmpty) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 669 : 670;
	}
	if ( xpkPathRename(objXpk, "assets/readonly.txt", "assets/readonly_new.txt") != XPK_ERR_READONLY ) {
		xpkClose(objXpk);
		return 367;
	}
	iRetCall = xpkPathRename(objXpk, "./", "assets/readonly_new.txt");
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorPathEmpty) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 671 : 672;
	}
	memset(&objBuildOpt, 0, sizeof(objBuildOpt));
	objBuildOpt.replaceOriginal = FALSE;
	iRetCall = xpkBuild(objXpk, &objBuildOpt);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, "tempPath is required when replaceOriginal is disabled") ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 673 : 674;
	}
	memset(&objBuildOpt, 0, sizeof(objBuildOpt));
	objBuildOpt.replaceOriginal = FALSE;
	objBuildOpt.tempPath = sPathPkgReadonly;
	iRetCall = xpkBuild(objXpk, &objBuildOpt);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 675 : 676;
	}
	memset(&objBuildOpt, 0, sizeof(objBuildOpt));
	objBuildOpt.replaceOriginal = FALSE;
	objBuildOpt.tempPath = sPathReadonlyAlt;
	iRetCall = xpkBuild(objXpk, &objBuildOpt);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 680 : 681;
	}
	sPathKey = procXpkPathSuffixDupText(sPathPkgReadonly, ".replace.bak");
	if ( sPathKey == NULL ) {
		xpkClose(objXpk);
		return 933;
	}
	memset(&objBuildOpt, 0, sizeof(objBuildOpt));
	objBuildOpt.replaceOriginal = TRUE;
	objBuildOpt.tempPath = sPathKey;
	iRetCall = xpkBuild(objXpk, &objBuildOpt);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 934 : 935;
	}
	xpkFreeInternal(sPathKey);
	sPathKey = procXpkPathSuffixDupText(sPathReadonlyAlt, ".replace.bak\\temp.xpk");
	if ( sPathKey == NULL ) {
		xpkClose(objXpk);
		return 936;
	}
	memset(&objBuildOpt, 0, sizeof(objBuildOpt));
	objBuildOpt.replaceOriginal = TRUE;
	objBuildOpt.tempPath = sPathKey;
	iRetCall = xpkBuild(objXpk, &objBuildOpt);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 937 : 938;
	}
	xpkFreeInternal(sPathKey);
	sPathKey = procXpkPathSuffixDupText(sPathReadonlyAlt, ".replace.bak.001");
	if ( sPathKey == NULL ) {
		xpkClose(objXpk);
		return 939;
	}
	memset(&objBuildOpt, 0, sizeof(objBuildOpt));
	objBuildOpt.replaceOriginal = TRUE;
	objBuildOpt.tempPath = sPathKey;
	iRetCall = xpkBuild(objXpk, &objBuildOpt);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkFreeInternal(sPathKey);
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 940 : 941;
	}
	xpkFreeInternal(sPathKey);
	if ( xpkAddData(objXpk, sDataReadonly, sizeof(sDataReadonly), NULL, &iPosRet) != XPK_ERR_READONLY ) {
		xpkClose(objXpk);
		return 368;
	}
	if ( xpkPathAddData(objXpk, "assets/readonly.txt", sDataReadonly, sizeof(sDataReadonly), NULL) != XPK_ERR_READONLY ) {
		xpkClose(objXpk);
		return 379;
	}
	if ( xpkPathRemove(objXpk, "assets/readonly.txt") != XPK_ERR_READONLY ) {
		xpkClose(objXpk);
		return 380;
	}
	iRetCall = xpkUpdateData(objXpk, 0, sDataReadonly, sizeof(sDataReadonly), NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 583 : 584;
	}
	iRetCall = xpkRemove(objXpk, 0);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 585 : 586;
	}
	iRetCall = xpkSetFlag(objXpk, 0, XPK_FLAG_TYPE_MASK, 0x10u);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 587 : 588;
	}
	iRetCall = xpkIndexRemove(objXpk, 1);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 574 : 575;
	}
	iRetCall = xpkIndexUpdateData(objXpk, 1, sDataReadonly, sizeof(sDataReadonly), NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 576 : 577;
	}
	if ( xpkCount(objXpk) != 1 ) {
		xpkClose(objXpk);
		return 369;
	}
	if ( !xpkPathExists(objXpk, "assets/readonly.txt") ) {
		xpkClose(objXpk);
		return 370;
	}
	if ( xpkPathExists(objXpk, "assets/readonly_new.txt") ) {
		xpkClose(objXpk);
		return 371;
	}
	if ( xpkPathGetInfo(objXpk, "assets/readonly.txt", &objInfoPath) != XPK_OK ) {
		xpkClose(objXpk);
		return 372;
	}
	if ( objInfoPath.platformAttr != 0 ) {
		xpkClose(objXpk);
		return 373;
	}
	if ( xpkSetFlag(objXpk, 1, XPK_FLAG_TYPE_MASK, 0x30u) != XPK_ERR_READONLY ) {
		xpkClose(objXpk);
		return 374;
	}
	if ( xpkGetInfo(objXpk, 1, &objInfo) != XPK_OK ) {
		xpkClose(objXpk);
		return 375;
	}
	if ( (objInfo.flag & XPK_FLAG_TYPE_MASK) != 0 ) {
		xpkClose(objXpk);
		return 376;
	}
	if ( xpkMetaClear(objXpk) != XPK_ERR_READONLY ) {
		xpkClose(objXpk);
		return 571;
	}
	xpkClose(objXpk);
	iCheckRet = procTestCheckLastError(NULL, XPK_OK, "");
	if ( iCheckRet != 0 ) {
		return (iCheckRet == 1) ? 572 : 573;
	}
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.readonly = TRUE;
	objXpk = xpkOpen(sPathPkgCore, &objOpt);
	if ( objXpk == NULL ) {
		return 578;
	}
	iRetCall = xpkPathRemove(objXpk, "assets/readonly.txt");
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 579 : 580;
	}
	iRetCall = xpkPathSetAttr(objXpk, "assets/readonly.txt", 0x20u);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 581 : 582;
	}
	iRetCall = xpkSetInfoExtSize(objXpk, 8);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 677 : 678;
	}
	iRetCall = xpkSetInfoExt(objXpk, 0, arrInfoExtCoreRead, sizeof(arrInfoExtCoreRead));
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 589 : 590;
	}
	iRetCall = xpkSetInfoExt(objXpk, 1, NULL, sizeof(arrInfoExtCoreRead));
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 649 : 650;
	}
	iDataSize = 123;
	pMetaRead = xpkPathReadToMemory(objXpk, NULL, &iDataSize);
	iCheckRet = procTestCheckNullResultError(pMetaRead, objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch);
	if ( iCheckRet != 0 ) {
		if ( pMetaRead != NULL ) {
			xpkFree(pMetaRead);
			xpkClose(objXpk);
			return 657;
		}
		xpkClose(objXpk);
		return (iCheckRet == 2) ? 658 : 659;
	}
	if ( iDataSize != 0 ) {
		xpkClose(objXpk);
		return 660;
	}
	iRetCall = xpkPathReadToFile(objXpk, "assets/readonly.txt", NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 661 : 662;
	}
	iRetCall = xpkIndexReadToFile(objXpk, 1, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_STATE, objXpk, XPK_ERR_STATE, sXpkErrorPackTypeMismatch) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_STATE) ? 663 : 664;
	}
	xpkClose(objXpk);

	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.readonly = TRUE;
	objXpk = xpkOpen(sPathPkgIndex, &objOpt);
	if ( objXpk == NULL ) {
		return 634;
	}
	iRetCall = xpkIndexAddFile(objXpk, 1999, NULL, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 635 : 636;
	}
	iRetCall = xpkIndexAddData(objXpk, 1999, NULL, 1, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 637 : 638;
	}
	iRetCall = xpkIndexUpdateFile(objXpk, 1001, NULL, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 639 : 640;
	}
	iRetCall = xpkIndexUpdateData(objXpk, 1001, NULL, 1, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 641 : 642;
	}
	iRetCall = xpkIndexReadToFile(objXpk, 1001, NULL);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_PARAM, objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam) ) {
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_PARAM) ? 655 : 656;
	}
	xpkClose(objXpk);

	if ( xpkAddFile(NULL, "missing.file", NULL, NULL) != XPK_ERR_PARAM ) {
		return 377;
	}
	if ( xpkUpdateFile(NULL, 1, "missing.file", NULL) != XPK_ERR_PARAM ) {
		return 378;
	}
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objOpt.readonly = TRUE;
	objXpk = xpkOpen("release/x64/xpack_phase3_missing_readonly.xpk", &objOpt);
	if ( objXpk != NULL ) {
		xpkClose(objXpk);
		return 405;
	}
	iCheckRet = procTestCheckLastError(NULL, XPK_ERR_READONLY, sXpkErrorReadonly);
	if ( iCheckRet != 0 ) {
		return (iCheckRet == 1) ? 412 : 413;
	}

	sPathFile = "release/x64/xpack_phase3_cleanup_check.xpk";
	remove(sPathFile);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathFile, &objOpt);
	if ( objXpk == NULL ) {
		return 874;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_WIN32) != XPK_OK ) {
		xpkClose(objXpk);
		return 875;
	}
	memset(&objWriteOpt, 0, sizeof(objWriteOpt));
	objWriteOpt.compLevel = 1;
	objWriteOpt.writePolicy = XPK_WRITE_BUFFERED;
	if ( xpkPathAddData(objXpk, "assets/cleanup.txt", sDataPath, sizeof(sDataPath), &objWriteOpt) != XPK_OK ) {
		xpkClose(objXpk);
		return 876;
	}
	pEntry = (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, 1);
	if ( pEntry == NULL || pEntry->sPath == NULL ) {
		xpkClose(objXpk);
		return 877;
	}
	objXpk->iEntryCount = 0;
	procXpkFreeEntryText(objXpk);
	if ( pEntry->sPath != NULL ) {
		xpkClose(objXpk);
		return 878;
	}
	xpkClose(objXpk);
	remove(sPathFile);

	sPathFile = "release/x64/xpack_phase3_lookup_preserve.xpk";
	remove(sPathFile);
	memset(&objOpt, 0, sizeof(objOpt));
	objOpt.createIfMissing = TRUE;
	objXpk = xpkOpen(sPathFile, &objOpt);
	if ( objXpk == NULL ) {
		return 888;
	}
	if ( xpkSetPackType(objXpk, XPK_PACK_WIN32) != XPK_OK ) {
		xpkClose(objXpk);
		return 889;
	}
	if ( xpkPathAddData(objXpk, "assets/a.txt", sDataPath, sizeof(sDataPath), NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 890;
	}
	if ( xpkPathAddData(objXpk, "assets/b.txt", sDataPath, sizeof(sDataPath), NULL) != XPK_OK ) {
		xpkClose(objXpk);
		return 891;
	}
	pEntry = (xpkEntry*)xrtArrayGet(&objXpk->arrEntry, 2);
	if ( pEntry == NULL || pEntry->sPath == NULL ) {
		xpkClose(objXpk);
		return 892;
	}
	iCorruptByte = (uint8_t)pEntry->sPath[0];
	pEntry->sPath[0] = '\0';
	iRetCall = procXpkRebuildLookup(objXpk);
	if ( !procTestExpectCallError(iRetCall, XPK_ERR_FORMAT, objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat) ) {
		pEntry->sPath[0] = (char)iCorruptByte;
		xpkClose(objXpk);
		return (iRetCall != XPK_ERR_FORMAT) ? 893 : 894;
	}
	pEntry->sPath[0] = (char)iCorruptByte;
	if ( !xpkPathExists(objXpk, "assets/a.txt") ) {
		xpkClose(objXpk);
		return 895;
	}
	if ( !procTestExpectLastError(objXpk, XPK_OK, NULL) ) {
		xpkClose(objXpk);
		return 896;
	}
	xpkClose(objXpk);
