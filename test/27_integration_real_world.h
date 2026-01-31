/*
 * xPack Ver7 - Integration Real World (27)
 */

#include "test_framework.h"

static char* createTestData_27(int size, char pattern) {
    char* pData = (char*)malloc(size);
    for (int i = 0; i < size; i++) {
        pData[i] = (char)(pattern + (i % 26));
    }
    return pData;
}

TEST(config_file_backup_scenario) {
    const char* sFilename = "test_27_config_backup.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_LINUX, 0);
    ASSERT_NOT_NULL(xpk);

    char* pConfig1 = createTestData_27(1024, 'A');
    char* pConfig2 = createTestData_27(2048, 'B');
    char* pConfig3 = createTestData_27(512, 'C');

    void* pResult1 = xpkPathAppendData(xpk, "config/app.conf", pConfig1, 1024, 1);
    ASSERT_NOT_NULL(pResult1);
    void* pResult2 = xpkPathAppendData(xpk, "config/user.conf", pConfig2, 2048, 1);
    ASSERT_NOT_NULL(pResult2);
    void* pResult3 = xpkPathAppendData(xpk, "backup/app.conf.bak", pConfig3, 512, 1);
    ASSERT_NOT_NULL(pResult3);

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkExtractAll(xpk, "."), 0);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);

    free(pConfig1);
    free(pConfig2);
    free(pConfig3);
    xpkClose(xpk);
}

TEST(log_file_rotation_scenario) {
    const char* sFilename = "test_27_log_rotation.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 10; i++) {
        char* pLogData = createTestData_27(4096, 'A' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pLogData, 4096, i), UINT32_MAX);
        free(pLogData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkCount(xpk), 10);

    ASSERT_EQ(xpkRemove(xpk, 0), 0);
    ASSERT_EQ(xpkRemove(xpk, 0), 0);
    ASSERT_EQ(xpkRemove(xpk, 0), 0);

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkCount(xpk), 7);
    ASSERT_EQ(xpkRebuild(xpk), 0);

    xpkClose(xpk);
}

TEST(game_asset_package_scenario) {
    const char* sFilename = "test_27_game_assets.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_LINUX, 0);
    ASSERT_NOT_NULL(xpk);

    char* pTexture = createTestData_27(1024 * 1024, 'T');
    char* pModel = createTestData_27(512 * 1024, 'M');
    char* pSound = createTestData_27(256 * 1024, 'S');
    char* pScript = createTestData_27(64 * 1024, 'J');

    void* pResult1 = xpkPathAppendData(xpk, "textures/hero.png", pTexture, 1024 * 1024, 1);
    ASSERT_NOT_NULL(pResult1);
    void* pResult2 = xpkPathAppendData(xpk, "models/hero.obj", pModel, 512 * 1024, 1);
    ASSERT_NOT_NULL(pResult2);
    void* pResult3 = xpkPathAppendData(xpk, "sounds/hero_jump.wav", pSound, 256 * 1024, 1);
    ASSERT_NOT_NULL(pResult3);
    void* pResult4 = xpkPathAppendData(xpk, "scripts/hero.lua", pScript, 64 * 1024, 1);
    ASSERT_NOT_NULL(pResult4);

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkExtractAll(xpk, "."), 0);

    free(pTexture);
    free(pModel);
    free(pSound);
    free(pScript);
    xpkClose(xpk);
}

TEST(document_archive_scenario) {
    const char* sFilename = "test_27_documents.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_LINUX, 0);
    ASSERT_NOT_NULL(xpk);

    char* pDoc1 = createTestData_27(2048, 'D');
    char* pDoc2 = createTestData_27(4096, 'E');
    char* pDoc3 = createTestData_27(8192, 'F');

    void* pResult1 = xpkPathAppendData(xpk, "docs/report1.txt", pDoc1, 2048, 1);
    ASSERT_NOT_NULL(pResult1);
    void* pResult2 = xpkPathAppendData(xpk, "docs/report2.txt", pDoc2, 4096, 1);
    ASSERT_NOT_NULL(pResult2);
    void* pResult3 = xpkPathAppendData(xpk, "archive/old_report.txt", pDoc3, 8192, 1);
    ASSERT_NOT_NULL(pResult3);

    ASSERT_EQ(xpkSave(xpk), 0);

    uint32_t outSize = 0;
    void* pExtracted = xpkPathExtractData(xpk, "docs/report1.txt", &outSize);
    ASSERT_NOT_NULL(pExtracted);
    ASSERT_EQ(outSize, 2048);

    free(pDoc1);
    free(pDoc2);
    free(pDoc3);
    xpkClose(xpk);
}

TEST(source_code_package_scenario) {
    const char* sFilename = "test_27_source.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_LINUX, 0);
    ASSERT_NOT_NULL(xpk);

    char* pHeader = createTestData_27(1024, 'H');
    char* pSource = createTestData_27(4096, 'S');
    char* pMakefile = createTestData_27(512, 'M');

    void* pResult1 = xpkPathAppendData(xpk, "include/xpack.h", pHeader, 1024, 1);
    ASSERT_NOT_NULL(pResult1);
    void* pResult2 = xpkPathAppendData(xpk, "src/xpack.c", pSource, 4096, 1);
    ASSERT_NOT_NULL(pResult2);
    void* pResult3 = xpkPathAppendData(xpk, "Makefile", pMakefile, 512, 1);
    ASSERT_NOT_NULL(pResult3);

    ASSERT_EQ(xpkSave(xpk), 0);

    xpkStat stat;
    ASSERT_EQ(xpkStatGet(xpk, &stat), 0);
    ASSERT_EQ(stat.totalSize, 1024 + 4096 + 512);
    ASSERT_NE(stat.packedSize, 0);

    free(pHeader);
    free(pSource);
    free(pMakefile);
    xpkClose(xpk);
}

TEST(multimedia_collection_scenario) {
    const char* sFilename = "test_27_multimedia.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_LINUX, 0);
    ASSERT_NOT_NULL(xpk);

    char* pImage = createTestData_27(512 * 1024, 'I');
    char* pAudio = createTestData_27(2048 * 1024, 'A');
    char* pVideo = createTestData_27(10240 * 1024, 'V');

    void* pResult1 = xpkPathAppendData(xpk, "images/photo.jpg", pImage, 512 * 1024, 1);
    ASSERT_NOT_NULL(pResult1);
    void* pResult2 = xpkPathAppendData(xpk, "music/song.mp3", pAudio, 2048 * 1024, 1);
    ASSERT_NOT_NULL(pResult2);
    void* pResult3 = xpkPathAppendData(xpk, "videos/movie.mp4", pVideo, 10240 * 1024, 1);
    ASSERT_NOT_NULL(pResult3);

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);

    free(pImage);
    free(pAudio);
    free(pVideo);
    xpkClose(xpk);
}

TEST(database_snapshot_scenario) {
    const char* sFilename = "test_27_database.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 24; i++) {
        char* pSnapshot = createTestData_27(8192, 'A' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pSnapshot, 8192, i), UINT32_MAX);
        free(pSnapshot);
    }

    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_EQ(xpkRemove(xpk, 0), 0);
    ASSERT_EQ(xpkRemove(xpk, 0), 0);
    ASSERT_EQ(xpkRemove(xpk, 0), 0);

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkRebuild(xpk), 0);

    ASSERT_EQ(xpkVerifyAll(xpk), 0);
    ASSERT_EQ(xpkCount(xpk), 21);

    xpkClose(xpk);
}

TEST(software_update_package_scenario) {
    const char* sFilename = "test_27_update.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_LINUX, 0);
    ASSERT_NOT_NULL(xpk);

    char* pExe = createTestData_27(1024 * 1024, 'X');
    char* pDll = createTestData_27(512 * 1024, 'D');
    char* pConfig = createTestData_27(4096, 'C');

    void* pResult1 = xpkPathAppendData(xpk, "bin/app.exe", pExe, 1024 * 1024, 1);
    ASSERT_NOT_NULL(pResult1);
    void* pResult2 = xpkPathAppendData(xpk, "bin/library.dll", pDll, 512 * 1024, 1);
    ASSERT_NOT_NULL(pResult2);
    void* pResult3 = xpkPathAppendData(xpk, "config/settings.ini", pConfig, 4096, 1);
    ASSERT_NOT_NULL(pResult3);

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkExtractAll(xpk, "."), 0);

    free(pExe);
    free(pDll);
    free(pConfig);
    xpkClose(xpk);
}

TEST(cache_package_scenario) {
    const char* sFilename = "test_27_cache.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 100; i++) {
        char* pCacheData = createTestData_27(4096, 'C' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pCacheData, 4096, i), UINT32_MAX);
        free(pCacheData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_EQ(xpkRemove(xpk, 0), 0);
    ASSERT_EQ(xpkRemove(xpk, 0), 0);

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkRebuild(xpk), 0);

    ASSERT_EQ(xpkCount(xpk), 98);

    xpkClose(xpk);
}

TEST(mixed_content_archive_scenario) {
    const char* sFilename = "test_27_mixed.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_LINUX, 0);
    ASSERT_NOT_NULL(xpk);

    char* pText = createTestData_27(2048, 'T');
    char* pBinary = createTestData_27(4096, 'B');
    char* pImage = createTestData_27(1024 * 1024, 'I');
    char* pAudio = createTestData_27(512 * 1024, 'A');

    void* pResult1 = xpkPathAppendData(xpk, "text/readme.txt", pText, 2048, 1);
    ASSERT_NOT_NULL(pResult1);
    void* pResult2 = xpkPathAppendData(xpk, "binary/data.dat", pBinary, 4096, 1);
    ASSERT_NOT_NULL(pResult2);
    void* pResult3 = xpkPathAppendData(xpk, "images/logo.png", pImage, 1024 * 1024, 1);
    ASSERT_NOT_NULL(pResult3);
    void* pResult4 = xpkPathAppendData(xpk, "sounds/bell.wav", pAudio, 512 * 1024, 1);
    ASSERT_NOT_NULL(pResult4);

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);

    ASSERT_EQ(xpkExtractAll(xpk, "."), 0);

    free(pText);
    free(pBinary);
    free(pImage);
    free(pAudio);
    xpkClose(xpk);
}

TEST(incremental_backup_scenario) {
    const char* sFilename = "test_27_incremental.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    char* pFullBackup = createTestData_27(8192, 'F');
    ASSERT_NE(xpkAppendData(xpk, pFullBackup, 8192, 1), UINT32_MAX);

    ASSERT_EQ(xpkSave(xpk), 0);

    for (int i = 0; i < 10; i++) {
        char* pIncremental = createTestData_27(1024, 'I' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pIncremental, 1024, i + 2), UINT32_MAX);
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

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 20; i++) {
        char* pVersion = createTestData_27(4096, 'V' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pVersion, 4096, i), UINT32_MAX);
        free(pVersion);
    }

    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_EQ(xpkRemove(xpk, 0), 0);
    ASSERT_EQ(xpkRemove(xpk, 0), 0);
    ASSERT_EQ(xpkRemove(xpk, 0), 0);

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkRebuild(xpk), 0);

    ASSERT_EQ(xpkCount(xpk), 17);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);

    xpkClose(xpk);
}

void register_27_integration_real_world_tests(void) {
    TEST_REGISTER(config_file_backup_scenario, CAT_CORE, "Config file backup scenario");
    TEST_REGISTER(log_file_rotation_scenario, CAT_CORE, "Log file rotation scenario");
    TEST_REGISTER(game_asset_package_scenario, CAT_CORE, "Game asset package scenario");
    TEST_REGISTER(document_archive_scenario, CAT_CORE, "Document archive scenario");
    TEST_REGISTER(source_code_package_scenario, CAT_CORE, "Source code package scenario");
    TEST_REGISTER(multimedia_collection_scenario, CAT_CORE, "Multimedia collection scenario");
    TEST_REGISTER(database_snapshot_scenario, CAT_CORE, "Database snapshot scenario");
    TEST_REGISTER(software_update_package_scenario, CAT_CORE, "Software update package scenario");
    TEST_REGISTER(cache_package_scenario, CAT_CORE, "Cache package scenario");
    TEST_REGISTER(mixed_content_archive_scenario, CAT_CORE, "Mixed content archive scenario");
    TEST_REGISTER(incremental_backup_scenario, CAT_CORE, "Incremental backup scenario");
    TEST_REGISTER(version_control_scenario, CAT_CORE, "Version control scenario");
}
