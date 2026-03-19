#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xpack.h"

static int g_testsPassed = 0;
static int g_testsFailed = 0;

#define TEST(name) printf("\n[TEST] %s\n", name); fflush(stdout)
#define PASS() do { g_testsPassed++; printf("  PASS\n"); fflush(stdout); } while(0)
#define FAIL(msg) do { g_testsFailed++; printf("  FAIL: %s\n", msg); fflush(stdout); } while(0)

static void cleanupTestFiles(const char* basePath)
{
    char path[512];
    snprintf(path, sizeof(path), "%s.xpk", basePath);
    xrtFileDelete((str)path);
    for (int i = 1; i <= 10; i++) {
        snprintf(path, sizeof(path), "%s.%03d.xpk", basePath, i);
        xrtFileDelete((str)path);
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

int main(void)
{
    xrtInit();
    
    printf("================================================\n");
    printf(" xPack Ver7 - Minimal Path Test\n");
    printf("================================================\n");
    fflush(stdout);
    
    testPathCreate();
    
    printf("\n================================================\n");
    printf(" Results: %d tests, %d passed, %d failed\n", 
           g_testsPassed + g_testsFailed, g_testsPassed, g_testsFailed);
    printf("================================================\n");
    
    return (g_testsFailed > 0) ? 1 : 0;
}
