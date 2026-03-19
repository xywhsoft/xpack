/*
 * xPack Ver7 - 延迟写入测试
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

static void testDirtyFlags(void)
{
    TEST("DirtyFlags");
    
    cleanupTestFiles("test_dirty");
    
    xpkObject xpk = xpkOpen("test_dirty", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    // 启用缓冲模式
    xpkBufferedSet(xpk, 1);
    
    // 新包应该有脏标记（包头）
    xpkHead* head = xpkGetHead(xpk);
    printf("    After open: fileCount=%u, metaCompSize=%u\n", head->fileCount, head->metaCompSize);
    
    // 添加文件但不保存
    char data[256];
    memset(data, 0xAB, sizeof(data));
    uint32_t pos = xpkAppendData(xpk, data, sizeof(data), 7);
    if (pos == (uint32_t)-1) {
        xpkClose(xpk);
        FAIL("Failed to append data");
        return;
    }
    
    printf("    After append (no save): fileCount=%u, metaCompSize=%u\n", 
           xpkCount(xpk), head->metaCompSize);
    
    // 关闭但不保存（reopen 后应该看不到新文件）
    xpkClose(xpk);
    
    // 重新打开
    xpk = xpkOpen("test_dirty", 0, 1);
    if (!xpk) {
        FAIL("Failed to reopen package");
        return;
    }
    
    printf("    After reopen: fileCount=%u\n", xpkCount(xpk));
    
    // 因为没有保存，文件数量应该是 0
    if (xpkCount(xpk) != 0) {
        xpkClose(xpk);
        FAIL("Data should not be saved without xpkSave");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_dirty");
    PASS();
}

static void testSavePersists(void)
{
    TEST("SavePersists");
    
    cleanupTestFiles("test_persist");
    
    xpkObject xpk = xpkOpen("test_persist", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    // 启用缓冲模式
    xpkBufferedSet(xpk, 1);
    
    char data[256];
    memset(data, 0xCD, sizeof(data));
    uint32_t pos = xpkAppendData(xpk, data, sizeof(data), 7);
    if (pos == (uint32_t)-1) {
        xpkClose(xpk);
        FAIL("Failed to append data");
        return;
    }
    
    // 保存
    if (xpkSave(xpk) != 0) {
        xpkClose(xpk);
        FAIL("Failed to save");
        return;
    }
    
    xpkClose(xpk);
    
    // 重新打开
    xpk = xpkOpen("test_persist", 0, 1);
    if (!xpk) {
        FAIL("Failed to reopen package");
        return;
    }
    
    printf("    After reopen: fileCount=%u\n", xpkCount(xpk));
    
    if (xpkCount(xpk) != 1) {
        xpkClose(xpk);
        FAIL("Data should be saved after xpkSave");
        return;
    }
    
    // 验证数据
    uint64_t size = 0;
    void* readData = xpkExtractData(xpk, 0, &size);
    if (!readData) {
        xpkClose(xpk);
        FAIL("Failed to extract data");
        return;
    }
    
    if (size != sizeof(data) || memcmp(data, readData, sizeof(data)) != 0) {
        free(readData);
        xpkClose(xpk);
        FAIL("Data mismatch");
        return;
    }
    
    free(readData);
    xpkClose(xpk);
    cleanupTestFiles("test_persist");
    PASS();
}

static void testMetaDirty(void)
{
    TEST("MetaDirty");
    
    cleanupTestFiles("test_metadirty");
    
    xpkObject xpk = xpkOpen("test_metadirty", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    // 启用缓冲模式
    xpkBufferedSet(xpk, 1);
    
    // 设置 Meta 但不保存
    const char* testData = "Test Meta Data";
    if (xpkMetaSet(xpk, testData, strlen(testData) + 1, 7) != 0) {
        xpkClose(xpk);
        FAIL("Failed to set meta");
        return;
    }
    
    // 关闭但不保存
    xpkClose(xpk);
    
    // 重新打开
    xpk = xpkOpen("test_metadirty", 0, 1);
    if (!xpk) {
        FAIL("Failed to reopen package");
        return;
    }
    
    uint32_t size = 0;
    void* meta = xpkMetaGet(xpk, &size);
    
    // 因为没有保存，Meta 应该不存在
    if (meta != NULL || size != 0) {
        if (meta) free(meta);
        xpkClose(xpk);
        FAIL("Meta should not be saved without xpkSave");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_metadirty");
    PASS();
}

static void testMetaPersist(void)
{
    TEST("MetaPersist");
    
    cleanupTestFiles("test_metapersist");
    
    xpkObject xpk = xpkOpen("test_metapersist", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    // 启用缓冲模式
    xpkBufferedSet(xpk, 1);
    
    const char* testData = "Persistent Meta";
    if (xpkMetaSet(xpk, testData, strlen(testData) + 1, 7) != 0) {
        xpkClose(xpk);
        FAIL("Failed to set meta");
        return;
    }
    
    // 保存
    if (xpkSave(xpk) != 0) {
        xpkClose(xpk);
        FAIL("Failed to save");
        return;
    }
    
    xpkClose(xpk);
    
    // 重新打开
    xpk = xpkOpen("test_metapersist", 0, 1);
    if (!xpk) {
        FAIL("Failed to reopen package");
        return;
    }
    
    uint32_t size = 0;
    void* meta = xpkMetaGet(xpk, &size);
    
    if (!meta) {
        xpkClose(xpk);
        FAIL("Meta should be saved after xpkSave");
        return;
    }
    
    if (strcmp((const char*)meta, testData) != 0) {
        free(meta);
        xpkClose(xpk);
        FAIL("Meta data mismatch");
        return;
    }
    
    free(meta);
    xpkClose(xpk);
    cleanupTestFiles("test_metapersist");
    PASS();
}

static void testRemoveNotSaved(void)
{
    TEST("RemoveNotSaved");
    
    cleanupTestFiles("test_remove");
    
    xpkObject xpk = xpkOpen("test_remove", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    // 启用缓冲模式
    xpkBufferedSet(xpk, 1);
    
    // 添加并保存
    char data[256];
    memset(data, 0xEF, sizeof(data));
    xpkAppendData(xpk, data, sizeof(data), 7);
    xpkSave(xpk);
    
    printf("    After save: fileCount=%u\n", xpkCount(xpk));
    
    // 删除但不保存
    if (xpkRemove(xpk, 0) != 0) {
        xpkClose(xpk);
        FAIL("Failed to remove");
        return;
    }
    
    printf("    After remove (no save): deleted=%d\n", xpkInfoDeleted(xpk, 0));
    
    // 关闭但不保存
    xpkClose(xpk);
    
    // 重新打开
    xpk = xpkOpen("test_remove", 0, 1);
    if (!xpk) {
        FAIL("Failed to reopen package");
        return;
    }
    
    printf("    After reopen: fileCount=%u\n", xpkCount(xpk));
    
    // 因为删除没有保存，文件应该还在
    if (xpkCount(xpk) != 1) {
        xpkClose(xpk);
        FAIL("Remove should not be saved without xpkSave");
        return;
    }
    
    if (xpkInfoDeleted(xpk, 0)) {
        xpkClose(xpk);
        FAIL("File should not be marked as deleted after reopen");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_remove");
    PASS();
}

int main(void)
{
    xrtInit();
    
    printf("================================================\n");
    printf(" xPack Ver7 - Delayed Write Test\n");
    printf("================================================\n");
    
    testDirtyFlags();
    testSavePersists();
    testMetaDirty();
    testMetaPersist();
    testRemoveNotSaved();
    
    printf("\n================================================\n");
    printf(" Results: %d tests, %d passed, %d failed\n", 
           g_testsPassed + g_testsFailed, g_testsPassed, g_testsFailed);
    printf("================================================\n");
    
    return (g_testsFailed > 0) ? 1 : 0;
}
