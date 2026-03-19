/*
 * xPack Ver7 - Index 模式测试
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

static void testIndexCreate(void)
{
    TEST("IndexCreate");
    
    cleanupTestFiles("test_index");
    
    xpkObject xpk = xpkOpen("test_index", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    if (xpkTypeSet(xpk, XPK_TYPE_INDEX) != 0) {
        xpkClose(xpk);
        FAIL("Failed to set Index mode");
        return;
    }
    
    xpkBufferedSet(xpk, 1);
    
    xpkFileInfoIndex* info = xpkIndexAppendData(xpk, 100, "data100", 8, 7);
    if (!info) {
        xpkClose(xpk);
        FAIL("Failed to append data with index 100");
        return;
    }
    
    if (info->fileIndex != 100) {
        xpkClose(xpk);
        FAIL("Index mismatch");
        return;
    }
    
    if (xpkCount(xpk) != 1) {
        xpkClose(xpk);
        FAIL("Count should be 1");
        return;
    }
    
    xpkSave(xpk);
    xpkClose(xpk);
    cleanupTestFiles("test_index");
    PASS();
}

static void testIndexFind(void)
{
    TEST("IndexFind");
    
    cleanupTestFiles("test_index_find");
    
    xpkObject xpk = xpkOpen("test_index_find", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    xpkTypeSet(xpk, XPK_TYPE_INDEX);
    xpkBufferedSet(xpk, 1);
    
    xpkIndexAppendData(xpk, 100, "data100", 8, 7);
    xpkIndexAppendData(xpk, 200, "data200", 8, 7);
    xpkIndexAppendData(xpk, 300, "data300", 8, 7);
    
    uint32_t pos = xpkIndexFind(xpk, 200);
    if (pos == (uint32_t)-1) {
        xpkClose(xpk);
        FAIL("Failed to find index 200");
        return;
    }
    
    if (pos != 1) {
        xpkClose(xpk);
        FAIL("Position should be 1");
        return;
    }
    
    pos = xpkIndexFind(xpk, 999);
    if (pos != (uint32_t)-1) {
        xpkClose(xpk);
        FAIL("Should not find index 999");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_index_find");
    PASS();
}

static void testIndexExtract(void)
{
    TEST("IndexExtract");
    
    cleanupTestFiles("test_index_extract");
    
    xpkObject xpk = xpkOpen("test_index_extract", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    xpkTypeSet(xpk, XPK_TYPE_INDEX);
    xpkBufferedSet(xpk, 1);
    
    const char* data = "Hello, Index Mode!";
    xpkIndexAppendData(xpk, 42, data, strlen(data) + 1, 7);
    
    xpkSave(xpk);
    xpkClose(xpk);
    
    xpk = xpkOpen("test_index_extract", 0, 1);
    if (!xpk) {
        FAIL("Failed to reopen package");
        return;
    }
    
    uint64_t size = 0;
    void* extracted = xpkIndexExtractData(xpk, 42, &size);
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
    cleanupTestFiles("test_index_extract");
    PASS();
}

static void testIndexRemove(void)
{
    TEST("IndexRemove");
    
    cleanupTestFiles("test_index_remove");
    
    xpkObject xpk = xpkOpen("test_index_remove", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    xpkTypeSet(xpk, XPK_TYPE_INDEX);
    xpkBufferedSet(xpk, 1);
    
    xpkIndexAppendData(xpk, 100, "data100", 8, 7);
    xpkIndexAppendData(xpk, 200, "data200", 8, 7);
    
    if (xpkIndexRemove(xpk, 100) != 0) {
        xpkClose(xpk);
        FAIL("Failed to remove index 100");
        return;
    }
    
    uint32_t pos = xpkIndexFind(xpk, 100);
    if (pos != (uint32_t)-1) {
        xpkClose(xpk);
        FAIL("Index 100 should not be found after remove");
        return;
    }
    
    pos = xpkIndexFind(xpk, 200);
    if (pos == (uint32_t)-1) {
        xpkClose(xpk);
        FAIL("Index 200 should still be found");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_index_remove");
    PASS();
}

static void testIndexUpdate(void)
{
    TEST("IndexUpdate");
    
    cleanupTestFiles("test_index_update");
    
    xpkObject xpk = xpkOpen("test_index_update", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    xpkTypeSet(xpk, XPK_TYPE_INDEX);
    xpkBufferedSet(xpk, 1);
    
    xpkIndexAppendData(xpk, 100, "old data", 9, 7);
    
    const char* newData = "new data for update test";
    if (xpkIndexUpdateData(xpk, 100, newData, strlen(newData) + 1, 7) != 0) {
        xpkClose(xpk);
        FAIL("Failed to update index 100");
        return;
    }
    
    xpkSave(xpk);
    xpkClose(xpk);
    
    xpk = xpkOpen("test_index_update", 0, 1);
    if (!xpk) {
        FAIL("Failed to reopen package");
        return;
    }
    
    uint64_t size = 0;
    void* extracted = xpkIndexExtractData(xpk, 100, &size);
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
    cleanupTestFiles("test_index_update");
    PASS();
}

static void testIndexDuplicate(void)
{
    TEST("IndexDuplicate");
    
    cleanupTestFiles("test_index_dup");
    
    xpkObject xpk = xpkOpen("test_index_dup", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    xpkTypeSet(xpk, XPK_TYPE_INDEX);
    xpkBufferedSet(xpk, 1);
    
    xpkIndexAppendData(xpk, 100, "data1", 6, 7);
    
    xpkFileInfoIndex* info = xpkIndexAppendData(xpk, 100, "data2", 6, 7);
    if (info != NULL) {
        xpkClose(xpk);
        FAIL("Should fail to add duplicate index");
        return;
    }
    
    if (xpkLastError() != 12) {  // XPK_ERR_EXISTS = 12
        xpkClose(xpk);
        FAIL("Error should be XPK_ERR_EXISTS");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_index_dup");
    PASS();
}

int main(void)
{
    xrtInit();
    
    printf("================================================\n");
    printf(" xPack Ver7 - Index Mode Test\n");
    printf("================================================\n");
    
    testIndexCreate();
    testIndexFind();
    testIndexExtract();
    testIndexRemove();
    testIndexUpdate();
    testIndexDuplicate();
    
    printf("\n================================================\n");
    printf(" Results: %d tests, %d passed, %d failed\n", 
           g_testsPassed + g_testsFailed, g_testsPassed, g_testsFailed);
    printf("================================================\n");
    
    return (g_testsFailed > 0) ? 1 : 0;
}
