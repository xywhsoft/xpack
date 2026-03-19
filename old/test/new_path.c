/*
 * xPack Ver7 - Path 模式测试
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xpack.h"

static int g_testsPassed = 0;
static int g_testsFailed = 0;

#define TEST(name) printf("\n[TEST] %s\n", name)
#define PASS() do { g_testsPassed++; printf("  PASS\n"); } while(0)
#define FAIL(msg) do { g_testsFailed++; printf("  FAIL: %s\n", msg); } while(0)

static void cleanupTestFiles(const char* basePath)
{
    char path[512];
    snprintf(path, sizeof(path), "%s.xpk", basePath);
    xrtFileDelete(path);
    for (int i = 1; i <= 10; i++) {
        snprintf(path, sizeof(path), "%s.%03d.xpk", basePath, i);
        xrtFileDelete(path);
    }
}

static void testPathCreate(void)
{
    TEST("PathCreate");
    
    cleanupTestFiles("test_path");
    
    xpkObject xpk = xpkOpen("test_path", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    if (xpkTypeSet(xpk, XPK_TYPE_LINUX) != 0) {
        xpkClose(xpk);
        FAIL("Failed to set Linux mode");
        return;
    }
    
    xpkBufferedSet(xpk, 1);
    
    uint32_t pos = xpkPathAppendData(xpk, "/home/user/file.txt", "test data", 10, 7);
    if (pos == (uint32_t)-1) {
        xpkClose(xpk);
        FAIL("Failed to append data with path");
        return;
    }
    
    if (xpkCount(xpk) != 1) {
        xpkClose(xpk);
        FAIL("Count should be 1");
        return;
    }
    
    xpkSave(xpk);
    xpkClose(xpk);
    cleanupTestFiles("test_path");
    PASS();
}

static void testPathFind(void)
{
    TEST("PathFind");
    
    cleanupTestFiles("test_path_find");
    
    xpkObject xpk = xpkOpen("test_path_find", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    xpkTypeSet(xpk, XPK_TYPE_LINUX);
    xpkBufferedSet(xpk, 1);
    
    xpkPathAppendData(xpk, "/dir/file1.txt", "data1", 6, 7);
    xpkPathAppendData(xpk, "/dir/file2.txt", "data2", 6, 7);
    xpkPathAppendData(xpk, "/dir/subdir/file3.txt", "data3", 6, 7);
    
    uint32_t pos = xpkPathFind(xpk, "/dir/file2.txt");
    if (pos == (uint32_t)-1) {
        xpkClose(xpk);
        FAIL("Failed to find /dir/file2.txt");
        return;
    }
    
    if (pos != 1) {
        xpkClose(xpk);
        FAIL("Position should be 1");
        return;
    }
    
    pos = xpkPathFind(xpk, "/nonexistent.txt");
    if (pos != (uint32_t)-1) {
        xpkClose(xpk);
        FAIL("Should not find nonexistent path");
        return;
    }
    
    if (!xpkPathExists(xpk, "/dir/file1.txt")) {
        xpkClose(xpk);
        FAIL("PathExists should return true");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_path_find");
    PASS();
}

static void testPathExtract(void)
{
    TEST("PathExtract");
    
    cleanupTestFiles("test_path_extract");
    
    xpkObject xpk = xpkOpen("test_path_extract", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    xpkTypeSet(xpk, XPK_TYPE_LINUX);
    xpkBufferedSet(xpk, 1);
    
    const char* data = "Hello, Path Mode!";
    xpkPathAppendData(xpk, "/data/hello.txt", data, strlen(data) + 1, 7);
    
    xpkSave(xpk);
    xpkClose(xpk);
    
    xpk = xpkOpen("test_path_extract", 0, 1);
    if (!xpk) {
        FAIL("Failed to reopen package");
        return;
    }
    
    uint64_t size = 0;
    void* extracted = xpkPathExtractData(xpk, "/data/hello.txt", &size);
    if (!extracted) {
        xpkClose(xpk);
        FAIL("Failed to extract data");
        return;
    }
    
    if (strcmp((const char*)extracted, data) != 0) {
        free(extracted);
        xpkClose(xpk);
        FAIL("Data mismatch");
        return;
    }
    
    free(extracted);
    xpkClose(xpk);
    cleanupTestFiles("test_path_extract");
    PASS();
}

static void testPathRemove(void)
{
    TEST("PathRemove");
    
    cleanupTestFiles("test_path_remove");
    
    xpkObject xpk = xpkOpen("test_path_remove", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    xpkTypeSet(xpk, XPK_TYPE_LINUX);
    xpkBufferedSet(xpk, 1);
    
    xpkPathAppendData(xpk, "/file1.txt", "data1", 6, 7);
    xpkPathAppendData(xpk, "/file2.txt", "data2", 6, 7);
    
    if (xpkPathRemove(xpk, "/file1.txt") != 0) {
        xpkClose(xpk);
        FAIL("Failed to remove /file1.txt");
        return;
    }
    
    uint32_t pos = xpkPathFind(xpk, "/file1.txt");
    if (pos != (uint32_t)-1) {
        xpkClose(xpk);
        FAIL("/file1.txt should not be found after remove");
        return;
    }
    
    pos = xpkPathFind(xpk, "/file2.txt");
    if (pos == (uint32_t)-1) {
        xpkClose(xpk);
        FAIL("/file2.txt should still be found");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_path_remove");
    PASS();
}

static void testPathUpdate(void)
{
    TEST("PathUpdate");
    
    cleanupTestFiles("test_path_update");
    
    xpkObject xpk = xpkOpen("test_path_update", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    xpkTypeSet(xpk, XPK_TYPE_LINUX);
    xpkBufferedSet(xpk, 1);
    
    xpkPathAppendData(xpk, "/config.txt", "old config", 10, 7);
    
    const char* newData = "new config data for update test";
    if (xpkPathUpdateData(xpk, "/config.txt", newData, strlen(newData) + 1, 7) != 0) {
        xpkClose(xpk);
        FAIL("Failed to update /config.txt");
        return;
    }
    
    xpkSave(xpk);
    xpkClose(xpk);
    
    xpk = xpkOpen("test_path_update", 0, 1);
    if (!xpk) {
        FAIL("Failed to reopen package");
        return;
    }
    
    uint64_t size = 0;
    void* extracted = xpkPathExtractData(xpk, "/config.txt", &size);
    if (!extracted) {
        xpkClose(xpk);
        FAIL("Failed to extract data after update");
        return;
    }
    
    if (strcmp((const char*)extracted, newData) != 0) {
        free(extracted);
        xpkClose(xpk);
        FAIL("Data mismatch after update");
        return;
    }
    
    free(extracted);
    xpkClose(xpk);
    cleanupTestFiles("test_path_update");
    PASS();
}

static void testPathGet(void)
{
    TEST("PathGet");
    
    cleanupTestFiles("test_path_get");
    
    xpkObject xpk = xpkOpen("test_path_get", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    xpkTypeSet(xpk, XPK_TYPE_LINUX);
    xpkBufferedSet(xpk, 1);
    
    xpkPathAppendData(xpk, "/path/to/file.txt", "data", 5, 7);
    
    const char* path = xpkPathGet(xpk, 0);
    if (!path) {
        xpkClose(xpk);
        FAIL("Failed to get path");
        return;
    }
    
    if (strcmp(path, "/path/to/file.txt") != 0) {
        xpkClose(xpk);
        FAIL("Path mismatch");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_path_get");
    PASS();
}

static void testWin32CaseInsensitive(void)
{
    TEST("Win32CaseInsensitive");
    
    cleanupTestFiles("test_win32_case");
    
    xpkObject xpk = xpkOpen("test_win32_case", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    xpkTypeSet(xpk, XPK_TYPE_WIN32);
    xpkBufferedSet(xpk, 1);
    
    xpkPathAppendData(xpk, "C:\\Users\\Test\\File.txt", "data", 5, 7);
    
    // Win32 模式应该不区分大小写
    uint32_t pos = xpkPathFind(xpk, "c:\\users\\test\\file.txt");
    if (pos == (uint32_t)-1) {
        xpkClose(xpk);
        FAIL("Win32 mode should be case insensitive");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_win32_case");
    PASS();
}

static void testPathDuplicate(void)
{
    TEST("PathDuplicate");
    
    cleanupTestFiles("test_path_dup");
    
    xpkObject xpk = xpkOpen("test_path_dup", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    xpkTypeSet(xpk, XPK_TYPE_LINUX);
    xpkBufferedSet(xpk, 1);
    
    xpkPathAppendData(xpk, "/same/path.txt", "data1", 6, 7);
    
    uint32_t pos = xpkPathAppendData(xpk, "/same/path.txt", "data2", 6, 7);
    if (pos != (uint32_t)-1) {
        xpkClose(xpk);
        FAIL("Should fail to add duplicate path");
        return;
    }
    
    if (xpkLastError() != 12) {  // XPK_ERR_EXISTS = 12
        xpkClose(xpk);
        FAIL("Error should be XPK_ERR_EXISTS");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_path_dup");
    PASS();
}

static int g_eachCount = 0;
static int eachCallback(void* xpk, uint32_t pos, void* info, void* userData)
{
    (void)xpk; (void)pos; (void)info; (void)userData;
    g_eachCount++;
    return 0;
}

static void testEach(void)
{
    TEST("Each");
    
    cleanupTestFiles("test_each");
    
    xpkObject xpk = xpkOpen("test_each", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    xpkTypeSet(xpk, XPK_TYPE_LINUX);
    xpkBufferedSet(xpk, 1);
    
    xpkPathAppendData(xpk, "/file1.txt", "data1", 6, 7);
    xpkPathAppendData(xpk, "/file2.txt", "data2", 6, 7);
    xpkPathAppendData(xpk, "/file3.txt", "data3", 6, 7);
    
    g_eachCount = 0;
    if (xpkEach(xpk, eachCallback, NULL) != 0) {
        xpkClose(xpk);
        FAIL("xpkEach failed");
        return;
    }
    
    if (g_eachCount != 3) {
        xpkClose(xpk);
        FAIL("xpkEach should iterate 3 files");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_each");
    PASS();
}

static int g_matchCount = 0;
static int matchCallback(void* xpk, uint32_t pos, void* info, void* userData)
{
    (void)xpk; (void)pos; (void)info; (void)userData;
    g_matchCount++;
    return 0;
}

static void testEachMatch(void)
{
    TEST("EachMatch");
    
    cleanupTestFiles("test_each_match");
    
    xpkObject xpk = xpkOpen("test_each_match", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    xpkTypeSet(xpk, XPK_TYPE_LINUX);
    xpkBufferedSet(xpk, 1);
    
    xpkPathAppendData(xpk, "/dir/file1.txt", "data1", 6, 7);
    xpkPathAppendData(xpk, "/dir/file2.log", "data2", 6, 7);
    xpkPathAppendData(xpk, "/dir/file3.txt", "data3", 6, 7);
    xpkPathAppendData(xpk, "/other/doc.txt", "data4", 6, 7);
    
    g_matchCount = 0;
    if (xpkEachMatch(xpk, "*.txt", matchCallback, NULL) != 0) {
        xpkClose(xpk);
        FAIL("xpkEachMatch failed");
        return;
    }
    
    if (g_matchCount != 3) {
        xpkClose(xpk);
        FAIL("xpkEachMatch should match 3 .txt files");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_each_match");
    PASS();
}

static void testExtractAll(void)
{
    TEST("ExtractAll");
    
    cleanupTestFiles("test_extract_all");
    xrtDirDelete((str)"test_extract_dir");
    
    xpkObject xpk = xpkOpen("test_extract_all", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    xpkTypeSet(xpk, XPK_TYPE_LINUX);
    xpkBufferedSet(xpk, 1);
    
    xpkPathAppendData(xpk, "/file1.txt", "content1", 9, 7);
    xpkPathAppendData(xpk, "/subdir/file2.txt", "content2", 9, 7);
    
    xpkSave(xpk);
    xpkClose(xpk);
    
    xpk = xpkOpen("test_extract_all", 0, 1);
    if (!xpk) {
        FAIL("Failed to reopen package");
        return;
    }
    
    int count = xpkExtractAll(xpk, "test_extract_dir");
    if (count != 2) {
        xpkClose(xpk);
        FAIL("xpkExtractAll should extract 2 files");
        return;
    }
    
    xpkClose(xpk);
    
    size_t size = 0;
    char* data = (char*)xrtFileGetAll((str)"test_extract_dir/file1.txt", &size);
    if (!data || strcmp(data, "content1") != 0) {
        FAIL("Extracted file1.txt content mismatch");
    }
    if (data) free(data);
    
    data = (char*)xrtFileGetAll((str)"test_extract_dir/subdir/file2.txt", &size);
    if (!data || strcmp(data, "content2") != 0) {
        FAIL("Extracted file2.txt content mismatch");
    }
    if (data) free(data);
    
    xrtFileDelete((str)"test_extract_dir/file1.txt");
    xrtFileDelete((str)"test_extract_dir/subdir/file2.txt");
    xrtDirDelete((str)"test_extract_dir/subdir");
    xrtDirDelete((str)"test_extract_dir");
    
    cleanupTestFiles("test_extract_all");
    PASS();
}

static void testVolumeStatGet(void)
{
    TEST("VolumeStatGet");
    
    cleanupTestFiles("test_vol_stat");
    
    xpkObject xpk = xpkOpen("test_vol_stat", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    xpkTypeSet(xpk, XPK_TYPE_LINUX);
    xpkBufferedSet(xpk, 1);
    xpkVolumeSizeSet(xpk, 1024 * 1024);
    
    xpkPathAppendData(xpk, "/file.txt", "test data", 10, 7);
    
    xpkSave(xpk);
    
    xpkVolumeStat stat;
    if (xpkVolumeStatGet(xpk, &stat) != 0) {
        xpkClose(xpk);
        FAIL("xpkVolumeStatGet failed");
        return;
    }
    
    if (stat.volumeCount != 1) {
        xpkClose(xpk);
        FAIL("volumeCount should be 1");
        return;
    }
    
    if (stat.volumeSize != 1024 * 1024) {
        xpkClose(xpk);
        FAIL("volumeSize mismatch");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_vol_stat");
    PASS();
}

int main(void)
{
    xrtInit();
    
    printf("================================================\n");
    printf(" xPack Ver7 - Path Mode Test\n");
    printf("================================================\n");
    fflush(stdout);
    
    testPathCreate(); fflush(stdout);
    testPathFind(); fflush(stdout);
    testPathExtract(); fflush(stdout);
    testPathRemove(); fflush(stdout);
    testPathUpdate(); fflush(stdout);
    testPathGet(); fflush(stdout);
    testWin32CaseInsensitive(); fflush(stdout);
    testPathDuplicate(); fflush(stdout);
    testEach(); fflush(stdout);
    testEachMatch(); fflush(stdout);
    testExtractAll(); fflush(stdout);
    testVolumeStatGet(); fflush(stdout);
    
    printf("\n================================================\n");
    printf(" Results: %d tests, %d passed, %d failed\n", 
           g_testsPassed + g_testsFailed, g_testsPassed, g_testsFailed);
    printf("================================================\n");
    
    return (g_testsFailed > 0) ? 1 : 0;
}
