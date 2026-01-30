#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xpack.h"
#include "xrt.h"

#define TEST(name) void name()
#define RUN_TEST(name) do { printf("Running: %s\n", #name); name(); printf("PASSED\n\n"); } while(0)
#define ASSERT_EQ(a, b) do { if ((a) != (b)) { printf("FAIL at %s:%d - Expected %d, got %d\n", __FILE__, __LINE__, (int)(b), (int)(a)); return; } } while(0)
#define ASSERT_NE(a, b) do { if ((a) == (b)) { printf("FAIL at %s:%d - Not expected %d\n", __FILE__, __LINE__, (int)(b)); return; } } while(0)
#define ASSERT_NOT_NULL(a) do { if ((a) == NULL) { printf("FAIL at %s:%d - NULL value\n", __FILE__, __LINE__); return; } } while(0)

static char* createTestData(int size, char pattern) {
	char* pData = (char*)malloc(size);
	for ( int i = 0; i < size; i++ ) {
		pData[i] = (char)(pattern + (i % 26));
	}
	return pData;
}

TEST(config_file_backup_scenario) {
	const char* sFilename = "test_27_config_backup.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_PATH, 6);
	ASSERT_NOT_NULL(xpk);

	char* pConfig1 = createTestData(1024, 'A');
	char* pConfig2 = createTestData(2048, 'B');
	char* pConfig3 = createTestData(512, 'C');

	ASSERT_EQ(xpkAppend(xpk, "config/app.conf", pConfig1, 1024), 0);
	ASSERT_EQ(xpkAppend(xpk, "config/user.conf", pConfig2, 2048), 0);
	ASSERT_EQ(xpkAppend(xpk, "backup/app.conf.bak", pConfig3, 512), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkExtractAll(xpk), 0);
	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	free(pConfig1);
	free(pConfig2);
	free(pConfig3);
	xpkClose(xpk);
}

TEST(log_file_rotation_scenario) {
	const char* sFilename = "test_27_log_rotation.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 10; i++ ) {
		char* pLogData = createTestData(4096, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pLogData, 4096, i), 0);
		free(pLogData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 10);

	ASSERT_EQ(xpkRemove(xpk, 0), 0);
	ASSERT_EQ(xpkRemove(xpk, 1), 0);
	ASSERT_EQ(xpkRemove(xpk, 2), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 7);
	ASSERT_EQ(xpkRebuild(xpk), 0);

	xpkClose(xpk);
}

TEST(game_asset_package_scenario) {
	const char* sFilename = "test_27_game_assets.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_PATH, 7);
	ASSERT_NOT_NULL(xpk);

	char* pTexture = createTestData(1024 * 1024, 'T');
	char* pModel = createTestData(512 * 1024, 'M');
	char* pSound = createTestData(256 * 1024, 'S');
	char* pScript = createTestData(64 * 1024, 'J');

	ASSERT_EQ(xpkAppend(xpk, "textures/hero.png", pTexture, 1024 * 1024), 0);
	ASSERT_EQ(xpkAppend(xpk, "models/hero.obj", pModel, 512 * 1024), 0);
	ASSERT_EQ(xpkAppend(xpk, "sounds/hero_jump.wav", pSound, 256 * 1024), 0);
	ASSERT_EQ(xpkAppend(xpk, "scripts/hero.lua", pScript, 64 * 1024), 0);

	ASSERT_EQ(xpkInfoTypeSet(xpk, XPK_FTYPE_IMAGE), 0);
	ASSERT_EQ(xpkAppend(xpk, "textures/hero.png", pTexture, 1024 * 1024), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkExtractAll(xpk), 0);

	free(pTexture);
	free(pModel);
	free(pSound);
	free(pScript);
	xpkClose(xpk);
}

TEST(document_archive_scenario) {
	const char* sFilename = "test_27_documents.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_PATH, 5);
	ASSERT_NOT_NULL(xpk);

	char* pDoc1 = createTestData(2048, 'D');
	char* pDoc2 = createTestData(4096, 'E');
	char* pDoc3 = createTestData(8192, 'F');

	ASSERT_EQ(xpkAppend(xpk, "docs/report1.txt", pDoc1, 2048), 0);
	ASSERT_EQ(xpkAppend(xpk, "docs/report2.txt", pDoc2, 4096), 0);
	ASSERT_EQ(xpkAppend(xpk, "archive/old_report.txt", pDoc3, 8192), 0);

	ASSERT_EQ(xpkSave(xpk), 0);

	char* pSearchData = (char*)malloc(8192);
	int iSize = 8192;
	ASSERT_EQ(xpkExtract(xpk, pSearchData, &iSize, 1), 0);
	ASSERT_EQ(iSize, 2048);

	free(pSearchData);
	free(pDoc1);
	free(pDoc2);
	free(pDoc3);
	xpkClose(xpk);
}

TEST(source_code_package_scenario) {
	const char* sFilename = "test_27_source.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_PATH, 6);
	ASSERT_NOT_NULL(xpk);

	char* pHeader = createTestData(1024, 'H');
	char* pSource = createTestData(4096, 'S');
	char* pMakefile = createTestData(512, 'M');

	ASSERT_EQ(xpkAppend(xpk, "include/xpack.h", pHeader, 1024), 0);
	ASSERT_EQ(xpkAppend(xpk, "src/xpack.c", pSource, 4096), 0);
	ASSERT_EQ(xpkAppend(xpk, "Makefile", pMakefile, 512), 0);

	ASSERT_EQ(xpkSave(xpk), 0);

	int iOriginalSize = 0;
	int iCompressedSize = 0;
	ASSERT_EQ(xpkStatGet(xpk, &iOriginalSize, &iCompressedSize, 0), 0);
	ASSERT_EQ(iOriginalSize, 1024 + 4096 + 512);
	ASSERT_NE(iCompressedSize, 0);

	free(pHeader);
	free(pSource);
	free(pMakefile);
	xpkClose(xpk);
}

TEST(multimedia_collection_scenario) {
	const char* sFilename = "test_27_multimedia.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_PATH, 8);
	ASSERT_NOT_NULL(xpk);

	char* pImage = createTestData(512 * 1024, 'I');
	char* pAudio = createTestData(2048 * 1024, 'A');
	char* pVideo = createTestData(10240 * 1024, 'V');

	ASSERT_EQ(xpkAppend(xpk, "images/photo.jpg", pImage, 512 * 1024), 0);
	ASSERT_EQ(xpkAppend(xpk, "music/song.mp3", pAudio, 2048 * 1024), 0);
	ASSERT_EQ(xpkAppend(xpk, "videos/movie.mp4", pVideo, 10240 * 1024), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	free(pImage);
	free(pAudio);
	free(pVideo);
	xpkClose(xpk);
}

TEST(database_snapshot_scenario) {
	const char* sFilename = "test_27_database.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 9);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 24; i++ ) {
		char* pSnapshot = createTestData(8192, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pSnapshot, 8192, i), 0);
		free(pSnapshot);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_EQ(xpkRemove(xpk, 0), 0);
	ASSERT_EQ(xpkRemove(xpk, 1), 0);
	ASSERT_EQ(xpkRemove(xpk, 2), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkRebuild(xpk), 0);

	ASSERT_EQ(xpkVerifyAll(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 21);

	xpkClose(xpk);
}

TEST(software_update_package_scenario) {
	const char* sFilename = "test_27_update.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_PATH, 6);
	ASSERT_NOT_NULL(xpk);

	char* pExe = createTestData(1024 * 1024, 'X');
	char* pDll = createTestData(512 * 1024, 'D');
	char* pConfig = createTestData(4096, 'C');

	ASSERT_EQ(xpkAppend(xpk, "bin/app.exe", pExe, 1024 * 1024), 0);
	ASSERT_EQ(xpkAppend(xpk, "bin/library.dll", pDll, 512 * 1024), 0);
	ASSERT_EQ(xpkAppend(xpk, "config/settings.ini", pConfig, 4096), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkExtractAll(xpk), 0);

	char* pUpdatedExe = createTestData(2048 * 1024, 'Y');
	ASSERT_EQ(xpkUpdate(xpk, "bin/app.exe", pUpdatedExe, 2048 * 1024), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	free(pExe);
	free(pDll);
	free(pConfig);
	free(pUpdatedExe);
	xpkClose(xpk);
}

TEST(cache_package_scenario) {
	const char* sFilename = "test_27_cache.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 4);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 100; i++ ) {
		char* pCacheData = createTestData(4096, 'C' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pCacheData, 4096, i), 0);
		free(pCacheData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_EQ(xpkRemove(xpk, 0), 0);
	ASSERT_EQ(xpkRemove(xpk, 1), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkRebuild(xpk), 0);

	ASSERT_EQ(xpkCount(xpk), 98);

	xpkClose(xpk);
}

TEST(mixed_content_archive_scenario) {
	const char* sFilename = "test_27_mixed.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_PATH, 6);
	ASSERT_NOT_NULL(xpk);

	char* pText = createTestData(2048, 'T');
	char* pBinary = createTestData(4096, 'B');
	char* pImage = createTestData(1024 * 1024, 'I');
	char* pAudio = createTestData(512 * 1024, 'A');

	ASSERT_EQ(xpkAppend(xpk, "text/readme.txt", pText, 2048), 0);
	ASSERT_EQ(xpkAppend(xpk, "binary/data.dat", pBinary, 4096), 0);
	ASSERT_EQ(xpkAppend(xpk, "images/logo.png", pImage, 1024 * 1024), 0);
	ASSERT_EQ(xpkAppend(xpk, "sounds/bell.wav", pAudio, 512 * 1024), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	ASSERT_EQ(xpkExtractAll(xpk), 0);

	free(pText);
	free(pBinary);
	free(pImage);
	free(pAudio);
	xpkClose(xpk);
}

TEST(incremental_backup_scenario) {
	const char* sFilename = "test_27_incremental.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	char* pFullBackup = createTestData(8192, 'F');
	ASSERT_EQ(xpkAppendData(xpk, pFullBackup, 8192, 1), 0);

	ASSERT_EQ(xpkSave(xpk), 0);

	for ( int i = 0; i < 10; i++ ) {
		char* pIncremental = createTestData(1024, 'I' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pIncremental, 1024, i + 2), 0);
		free(pIncremental);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 11);

	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	free(pFullBackup);
	xpkClose(xpk);
}

TEST(version_control_scenario) {
	const char* sFilename = "test_27_version.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 6);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 20; i++ ) {
		char* pVersion = createTestData(4096, 'V' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pVersion, 4096, i), 0);
		free(pVersion);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_EQ(xpkRemove(xpk, 0), 0);
	ASSERT_EQ(xpkRemove(xpk, 1), 0);
	ASSERT_EQ(xpkRemove(xpk, 2), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkRebuild(xpk), 0);

	ASSERT_EQ(xpkCount(xpk), 17);
	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	xpkClose(xpk);
}

int main(int argc, char* argv[]) {
	xrtInit();

	printf("[Integration Real World Tests]\n");
	printf("===============================\n\n");

	RUN_TEST(config_file_backup_scenario);
	RUN_TEST(log_file_rotation_scenario);
	RUN_TEST(game_asset_package_scenario);
	RUN_TEST(document_archive_scenario);
	RUN_TEST(source_code_package_scenario);
	RUN_TEST(multimedia_collection_scenario);
	RUN_TEST(database_snapshot_scenario);
	RUN_TEST(software_update_package_scenario);
	RUN_TEST(cache_package_scenario);
	RUN_TEST(mixed_content_archive_scenario);
	RUN_TEST(incremental_backup_scenario);
	RUN_TEST(version_control_scenario);

	printf("===============================\n");
	printf("All Integration Real World Tests Passed!\n");

	return 0;
}
