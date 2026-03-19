static void procDeleteVolumeFiles(const char* sPathPackage)
{
	procTestDeletePathFamily(sPathPackage);
}

static int procEachCountTxt(xpkObject objXpk, uint32_t iPos, const void* pInfo, void* pArg)
{
	int* pCount;

	(void)objXpk;
	(void)iPos;
	(void)pInfo;
	pCount = (int*)pArg;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
	return XPK_OK;
}

static int procEachReturnObjectError(xpkObject objXpk, uint32_t iPos, const void* pInfo, void* pArg)
{
	xpkFileInfo objInfo;

	(void)iPos;
	(void)pInfo;
	(void)pArg;
	return xpkGetInfo(objXpk, 0, &objInfo);
}

static int procEachReturnCustomError(xpkObject objXpk, uint32_t iPos, const void* pInfo, void* pArg)
{
	(void)objXpk;
	(void)iPos;
	(void)pInfo;
	(void)pArg;
	return 123;
}

static bool procTestFilterEquals(const char* sFilter, const char* sGroup, const char* sBlock)
{
	if ( (sFilter == NULL) || (sFilter[0] == '\0') ) {
		return TRUE;
	}
	if ( strcmp(sFilter, "all") == 0 ) {
		return TRUE;
	}
	if ( (sGroup != NULL) && (strcmp(sFilter, sGroup) == 0) ) {
		return TRUE;
	}
	if ( (sBlock != NULL) && (strcmp(sFilter, sBlock) == 0) ) {
		return TRUE;
	}
	if ( (sGroup != NULL) && (sBlock != NULL) ) {
		char sScope[64];

		memset(sScope, 0, sizeof(sScope));
		snprintf(sScope, sizeof(sScope), "%s/%s", sGroup, sBlock);
		if ( strcmp(sFilter, sScope) == 0 ) {
			return TRUE;
		}
	}

	return FALSE;
}


static bool procTestFilterValid(const char* sFilter)
{
	if ( (sFilter == NULL) || (sFilter[0] == '\0') ) {
		return TRUE;
	}

	if ( procTestFilterEquals(sFilter, "smoke", "open_core") ) {
		return TRUE;
	}
	if ( procTestFilterEquals(sFilter, "unit", "index_path") ) {
		return TRUE;
	}
	if ( procTestFilterEquals(sFilter, "integration", "build_volume") ) {
		return TRUE;
	}
	if ( procTestFilterEquals(sFilter, "integration", "solid_readonly") ) {
		return TRUE;
	}
	if ( strcmp(sFilter, "all") == 0 ) {
		return TRUE;
	}

	return FALSE;
}


static bool procTestFilterIsHelp(const char* sFilter)
{
	if ( sFilter == NULL ) {
		return FALSE;
	}

	return (strcmp(sFilter, "-h") == 0)
		|| (strcmp(sFilter, "--help") == 0)
		|| (strcmp(sFilter, "help") == 0);
}


static void procTestPrintFilterHelp(void)
{
	printf("xpack_test usage:\n");
	printf("  xpack_test [filter]\n\n");
	printf("available filters:\n");
	printf("  all\n");
	printf("  smoke\n");
	printf("  smoke/open_core\n");
	printf("  unit\n");
	printf("  unit/index_path\n");
	printf("  integration\n");
	printf("  integration/build_volume\n");
	printf("  integration/solid_readonly\n");
	printf("\n");
	printf("note:\n");
	printf("  filters use staged cumulative execution.\n");
	printf("  unit runs smoke + unit.\n");
	printf("  integration/build_volume runs smoke + unit + build_volume.\n");
	printf("  integration/solid_readonly runs smoke + unit + build_volume + solid_readonly.\n");
}

int main(int argc, char** argv)
{
	const char* sPathPkgCore;
	const char* sPathPkgIndex;
	const char* sPathPkgLinux;
	const char* sPathPkgPath;
	const char* sPathPkgBuild;
	const char* sPathPkgBuildExistsTemp;
	const char* sPathPkgBuildExistsDir;
	const char* sPathPkgCodec;
	const char* sPathPkgVolume;
	const char* sPathPkgSolid;
	const char* sPathPkgSolidStore;
	const char* sPathPkgLayout;
	const char* sPathPkgFormat;
	const char* sPathFileExport;
	const char* sPathFileHuge;
	const char* sPathPkgStoreHuge;
	const char* sPathPkgBuildFailTemp;
	const char* sPathPkgSaveFail;
	const char* sPathPkgReplace;
	const char* sPathPkgReplaceNew;
	const char* sPathPkgReplaceTemp;
	const char* sPathPkgReplaceBackup;
	const char* sPathPkgRaw64;
	const char* sPathPkgReadonly;
	const char* sPathPkgTest;
	const char* sPathPkgVolumeTemp;
	const char* sPathPkgOpenDir;
	const char* sPathPkgOpenSparse;
	const char* sPathPkgOpenZeroSparse;
	const char sDataCore[] = "core-data-123";
	const char sDataIndex[] = "index-buffered-data";
	const char sDataIndexUpdate[] = "index-buffered-update";
	const char sDataIndexPending[] = "index-pending-remove";
	const char sDataPath[] = "path-buffered-data";
	const char sDataReadonly[] = "readonly-data";
	const char sMetaTest[] = "phase1-meta";
	const char sMetaSolid[] = "solid-meta";
	const char sMetaReadonly[] = "readonly-meta";
	const char sDataSolidUpdate[] = "solid-normal-update";
	const char sDataSolidStoreA[] = "A";
	const char sDataSolidStoreB[] = "B";
	uint8_t arrInfoExtCoreSet[16];
	uint8_t arrInfoExtCoreRead[16];
	char sDataBuildA[1024];
	char sDataBuildB[2048];
	char sDataSaveFailBase[65000];
	char sDataSaveFailNew[2048];
	char sDataReplaceNew[70000];
	char sPathReadonlyAlt[260];
	char sDataVolumeA[70000];
	char sDataVolumeB[80000];
	char sDataSolidA[8192];
	char sDataSolidB[8192];
	char sPathTooLong[300];
	str sPathFile;
	str sPathVolume;
	xfile hFile;
	xfile hFileTmp;
	xarray_struct arrTest;
	xlist_struct lstTest;
	xdict_struct tblTest;
	xmempool objPool;
	xpkObject objXpk;
	xpkObject objCheck;
	xpkEntry* pEntry;
	xpkEntry* pMapEntry;
	xpkOpenOptions objOpt;
	xpkBuildOptions objBuildOpt;
	xpkWriteOptions objWriteOpt;
	xpkPackType iPackType;
	xpkPackType iPackTypeSaved;
	const char* sTestFilter;
	bool bRunSmoke;
	bool bRunUnit;
	bool bRunBuildVolume;
	bool bRunSolidReadonly;
	xpkFileInfo objInfo;
	xpkFileInfoPath objInfoPath;
	xpkStat objStat;
	void* pMetaRead;
	void* pDataRead;
	void* pDataLarge;
	bool bNewItem;
	char* sPathKey;
	char* sPathSaved;
	int iEachCount;
	int iMatchCount;
	int iRetCall;
	int iCheckRet;
	uint32_t iHash32;
	uint32_t iMetaSize;
	uint32_t iPosRet;
	uint32_t iPosSaved;
	uint32_t iDataLargeSize;
	uint32_t iVolumeCount;
	uint32_t iVolumeSize;
	uint8_t iCorruptByte;
	uint64_t iBuildSizeBefore;
	uint64_t iBuildSizeAfter;
	uint64_t iDataSize;
	size_t iReadSize;
	int* pValArray;
	int* pValList;
	int* pValDict;
	CLzma2EncProps objLzmaProps;

	if ( LZ4_versionNumber() <= 0 ) {
		return 1;
	}
	if ( ZSTD_versionNumber() <= 0 ) {
		return 2;
	}

	Lzma2EncProps_Init(&objLzmaProps);

	iHash32 = xrtHash32((ptr)"xpk", 3);
	if ( iHash32 == 0 ) {
		return 3;
	}
	if ( xrtNow() == 0 ) {
		return 4;
	}

	sTestFilter = (argc > 1) ? argv[1] : getenv("XPACK_TEST_FILTER");
	if ( procTestFilterIsHelp(sTestFilter) ) {
		procTestPrintFilterHelp();
		return 0;
	}
	if ( !procTestFilterValid(sTestFilter) ) {
		fprintf(stderr, "unknown test filter: %s\n\n", sTestFilter);
		procTestPrintFilterHelp();
		return 2;
	}

	bRunSmoke =
		procTestFilterEquals(sTestFilter, "smoke", "open_core")
		|| procTestFilterEquals(sTestFilter, "unit", "index_path")
		|| procTestFilterEquals(sTestFilter, "integration", "build_volume")
		|| procTestFilterEquals(sTestFilter, "integration", "solid_readonly");

	bRunUnit =
		procTestFilterEquals(sTestFilter, "unit", "index_path")
		|| procTestFilterEquals(sTestFilter, "integration", "build_volume")
		|| procTestFilterEquals(sTestFilter, "integration", "solid_readonly");

	bRunBuildVolume =
		procTestFilterEquals(sTestFilter, "integration", "build_volume")
		|| procTestFilterEquals(sTestFilter, "integration", "solid_readonly");

	bRunSolidReadonly = procTestFilterEquals(sTestFilter, "integration", "solid_readonly");

	sPathPkgCore = "release/x64/xpack_phase3_core.xpk";
	sPathPkgIndex = "release/x64/xpack_phase3_index.xpk";
	sPathPkgLinux = "release/x64/xpack_phase3_linux.xpk";
	sPathPkgPath = "release/x64/xpack_phase3_path.xpk";
	sPathPkgBuild = "release/x64/xpack_phase3_build.xpk";
	sPathPkgBuildExistsTemp = "release/x64/xpack_phase3_build_exists_temp.xpk";
	sPathPkgBuildExistsDir = "release/x64/xpack_phase3_build_exists_temp_dir";
	sPathPkgCodec = "release/x64/xpack_phase3_codec.xpk";
	sPathPkgVolume = "release/x64/xpack_phase3_volume.xpk";
	sPathPkgSolid = "release/x64/xpack_phase3_solid.xpk";
	sPathPkgSolidStore = "release/x64/xpack_phase3_solid_store.xpk";
	sPathPkgLayout = "release/x64/xpack_phase3_layout.xpk";
	sPathPkgFormat = "release/x64/xpack_phase3_format_bad.xpk";
	sPathFileExport = "release/x64/xpack_phase3_export.bin";
	sPathFileHuge = "release/x64/xpack_phase3_huge_source.bin";
	sPathPkgStoreHuge = "release/x64/xpack_phase3_store_huge.xpk";
	sPathPkgBuildFailTemp = "release/x64/xpack_phase3_build_fail_cleanup.xpk.build.tmp";
	sPathPkgSaveFail = "release/x64/xpack_phase3_save_fail_queue.xpk";
	sPathPkgReplace = "release/x64/xpack_phase3_replace_safe.xpk";
	sPathPkgReplaceNew = "release/x64/xpack_phase3_replace_new.xpk";
	sPathPkgReplaceTemp = "release/x64/xpack_phase3_replace_safe.xpk.build.tmp";
	sPathPkgReplaceBackup = "release/x64/xpack_phase3_replace_safe.xpk.replace.bak";
	sPathPkgRaw64 = "release/x64/xpack_phase3_raw64.xpk";
	sPathPkgReadonly = "release/x64/xpack_phase3_readonly.xpk";
	sPathPkgTest = "release/x64/xpack_phase1_test.xpk";
	sPathPkgVolumeTemp = "release/x64/xpack_phase3_volume.xpk.build.tmp";
	sPathPkgOpenDir = "release/x64/xpack_phase3_open_dir";
	sPathPkgOpenSparse = "release/x64/xpack_phase3_open_sparse.xpk";
	sPathPkgOpenZeroSparse = "release/x64/xpack_phase3_open_zero_sparse.xpk";
	remove(sPathPkgCore);
	remove(sPathPkgIndex);
	remove(sPathPkgLinux);
	remove(sPathPkgPath);
	remove(sPathPkgBuild);
	remove(sPathPkgBuildExistsTemp);
	procTestDeletePathFamily(sPathPkgBuildExistsDir);
	remove(sPathPkgCodec);
	remove(sPathPkgSolid);
	remove(sPathPkgSolidStore);
	remove(sPathPkgFormat);
	remove(sPathFileExport);
	remove(sPathFileHuge);
	remove(sPathPkgStoreHuge);
	procTestDeletePathFamily(sPathPkgBuildFailTemp);
	procTestDeletePathFamily(sPathPkgBuildExistsTemp);
	procTestDeletePathFamily(sPathPkgSaveFail);
	remove(sPathPkgReadonly);
	remove(sPathPkgTest);
	procTestDeletePathFamily(sPathPkgReplace);
	procTestDeletePathFamily(sPathPkgReplaceNew);
	procTestDeletePathFamily(sPathPkgReplaceTemp);
	procTestDeletePathFamily(sPathPkgReplaceBackup);
	procTestDeletePathFamily(sPathPkgRaw64);
	procTestDeletePathFamily(sPathPkgVolume);
	procTestDeletePathFamily(sPathPkgVolumeTemp);
	procTestDeletePathFamily(sPathPkgLayout);
	procTestDeletePathFamily(sPathPkgOpenDir);
	procTestDeletePathFamily(sPathPkgOpenSparse);
	procTestDeletePathFamily(sPathPkgOpenZeroSparse);
	memset(sDataBuildA, 'A', sizeof(sDataBuildA));
	memset(sDataBuildB, 'B', sizeof(sDataBuildB));
	memset(sDataSaveFailBase, 'Q', sizeof(sDataSaveFailBase));
	memset(sDataSaveFailNew, 'N', sizeof(sDataSaveFailNew));
	memset(sDataReplaceNew, 'R', sizeof(sDataReplaceNew));
	memset(sDataVolumeA, 'V', sizeof(sDataVolumeA));
	memset(sDataVolumeB, 'W', sizeof(sDataVolumeB));
	memset(sDataSolidA, 0, sizeof(sDataSolidA));
	memset(sDataSolidB, 0, sizeof(sDataSolidB));
	memset(sPathTooLong, 'p', sizeof(sPathTooLong));
	memset(sPathReadonlyAlt, 0, sizeof(sPathReadonlyAlt));
	memset(arrInfoExtCoreSet, 0, sizeof(arrInfoExtCoreSet));
	memset(arrInfoExtCoreRead, 0, sizeof(arrInfoExtCoreRead));
	memcpy(sPathTooLong, "assets/", 7);
	sPathTooLong[sizeof(sPathTooLong) - 1] = '\0';
	if ( (strlen(sPathPkgReadonly) + 4) >= sizeof(sPathReadonlyAlt) ) {
		return 679;
	}
	strcpy(sPathReadonlyAlt, ".\\\\");
	strcat(sPathReadonlyAlt, sPathPkgReadonly);
#ifdef _WIN32
	for ( iEachCount = 0; sPathReadonlyAlt[iEachCount] != '\0'; iEachCount++ ) {
		if ( sPathReadonlyAlt[iEachCount] == '/' ) {
			sPathReadonlyAlt[iEachCount] = '\\';
		} else {
			sPathReadonlyAlt[iEachCount] = (char)toupper((unsigned char)sPathReadonlyAlt[iEachCount]);
		}
	}
#endif
	memcpy(arrInfoExtCoreSet, "core-ext-demo", 13);
	memcpy(sDataSolidA, "solid-alpha-", 12);
	memset(sDataSolidA + 12, 'A', sizeof(sDataSolidA) - 12);
	memcpy(sDataSolidB, "solid-alpha-", 12);
	memset(sDataSolidB + 12, 'B', sizeof(sDataSolidB) - 12);

	sPathFile = xrtPathJoin(2, ".", "xpack.c");
	if ( sPathFile == NULL ) {
		return 5;
	}
	if ( !xrtPathExists(sPathFile) ) {
		xrtFree(sPathFile);
		return 6;
	}

	hFile = xrtOpen(sPathFile, TRUE, XRT_CP_BINARY);
	xrtFree(sPathFile);
	if ( hFile == NULL ) {
		return 7;
	}
	if ( xrtGetEOF(hFile) == 0 ) {
		xrtClose(hFile);
		return 8;
	}
	xrtClose(hFile);

	xrtArrayInit(&arrTest, sizeof(int), XRT_OBJMODE_LOCAL);
	if ( xrtArrayAppend(&arrTest, 1) != 1 ) {
		xrtArrayUnit(&arrTest);
		return 9;
	}
	pValArray = (int*)xrtArrayGet(&arrTest, 1);
	if ( pValArray == NULL ) {
		xrtArrayUnit(&arrTest);
		return 10;
	}
	*pValArray = 42;
	xrtArrayUnit(&arrTest);

	xrtListInit(&lstTest, sizeof(int), XRT_OBJMODE_LOCAL);
	pValList = (int*)xrtListSet(&lstTest, 1001, &bNewItem);
	if ( (pValList == NULL) || !bNewItem ) {
		xrtListUnit(&lstTest);
		return 11;
	}
	*pValList = 24;
	if ( (int*)xrtListGet(&lstTest, 1001) == NULL ) {
		xrtListUnit(&lstTest);
		return 12;
	}
	xrtListUnit(&lstTest);

	xrtDictInit(&tblTest, sizeof(int), XRT_OBJMODE_LOCAL);
	pValDict = (int*)xrtDictSet(&tblTest, (ptr)"xpk", 3, &bNewItem);
	if ( (pValDict == NULL) || !bNewItem ) {
		xrtDictUnit(&tblTest);
		return 13;
	}
	*pValDict = 66;
	if ( (int*)xrtDictGet(&tblTest, (ptr)"xpk", 3) == NULL ) {
		xrtDictUnit(&tblTest);
		return 14;
	}
	xrtDictUnit(&tblTest);

	objPool = xrtMemPoolCreate(0, XRT_OBJMODE_LOCAL);
	if ( objPool == NULL ) {
		return 15;
	}
	xrtMemPoolDestroy(objPool);

	/* smoke: runtime / open / format / core */
	if ( bRunSmoke ) {
#include "smoke/selftest_open_core.inc.h"
	}

	/* unit: index / path / linux path mode */
	if ( bRunUnit ) {
#include "unit/selftest_index_path.inc.h"
	}

	/* integration: build / codec / volume / replace / save rollback */
	if ( bRunBuildVolume ) {
#include "integration/selftest_build_volume.inc.h"
	}

	/* integration: solid / layout / readonly / internal state guards */
	if ( bRunSolidReadonly ) {
#include "integration/selftest_solid_readonly.inc.h"
	}

	remove(sPathPkgCore);
	remove(sPathPkgIndex);
	remove(sPathPkgPath);
	remove(sPathPkgBuild);
	remove(sPathPkgBuildExistsTemp);
	remove(sPathPkgCodec);
	remove(sPathPkgSolid);
	remove(sPathPkgSolidStore);
	remove(sPathPkgFormat);
	remove(sPathFileExport);
	remove(sPathFileHuge);
	remove(sPathPkgStoreHuge);
	remove(sPathPkgReadonly);
	procTestDeletePathFamily(sPathPkgBuildExistsTemp);
	procTestDeletePathFamily(sPathPkgSaveFail);
	procTestDeletePathFamily(sPathPkgReplace);
	procTestDeletePathFamily(sPathPkgReplaceTemp);
	procTestDeletePathFamily(sPathPkgReplaceBackup);
	procTestDeletePathFamily(sPathPkgRaw64);
	procTestDeletePathFamily(sPathPkgVolume);
	procTestDeletePathFamily(sPathPkgVolumeTemp);
	procTestDeletePathFamily(sPathPkgLayout);
	procTestDeletePathFamily(sPathPkgOpenSparse);
	procTestDeletePathFamily(sPathPkgOpenZeroSparse);

	return 0;
}
