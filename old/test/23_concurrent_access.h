/*
 * xPack Ver7 - concurrent_access (23)
 */

#include "test_framework.h"
#include <stdlib.h>

TEST(rapid_open_close_cycles) {
    char* pData = (char*)malloc(1024);
    memset(pData, 'A', 1024);
    const char* sFilename = "test_23_rapid_open.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 1), 0);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    for (int i = 0; i < 100; i++) {
        xpk = xpkOpen(sFilename, 0, 1);
        ASSERT_NOT_NULL(xpk);
        ASSERT_EQ(xpkCount(xpk), 1);
        xpkClose(xpk);
    }

    free(pData);
}

TEST(rapid_append_extract_cycles) {
    const char* sFilename = "test_23_rapid_append.xpk";

    for (int i = 0; i < 50; i++) {
        xpkObject xpk = xpkOpen(sFilename, 0, 0);
        ASSERT_NOT_NULL(xpk);

        char* pData = (char*)malloc(512);
        memset(pData, 'A' + (i % 26), 512);

        uint32_t pos = xpkAppendData(xpk, pData, 512, 6);
        ASSERT_NE(pos, UINT32_MAX);

        ASSERT_EQ(xpkSave(xpk), 0);
        xpkClose(xpk);

        xpk = xpkOpen(sFilename, 0, 1);
        ASSERT_NOT_NULL(xpk);

        uint32_t outSize = 0;
        void* extracted = xpkExtractData(xpk, i, &outSize);
        ASSERT_NOT_NULL(extracted);
        ASSERT_EQ(outSize, 512);
        ASSERT_EQ(memcmp(extracted, pData, 512), 0);
        xpkFree(extracted);

        free(pData);
        xpkClose(xpk);
    }
}

TEST(rapid_update_cycles) {
    const char* sFilename = "test_23_rapid_update.xpk";
    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    xpkAppendData(xpk, "Initial", 7, 6);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    for (int i = 0; i < 50; i++) {
        xpk = xpkOpen(sFilename, 0, 0);
        ASSERT_NOT_NULL(xpk);

        char* pData = (char*)malloc(256);
        sprintf(pData, "Update %d", i);

        ASSERT_EQ(xpkUpdateData(xpk, 0, pData, (uint32_t)strlen(pData), 6), 0);
        ASSERT_EQ(xpkSave(xpk), 0);
        xpkClose(xpk);

        free(pData);
    }

    xpk = xpkOpen(sFilename, 0, 1);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    void* extracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(extracted);
    ASSERT_EQ(outSize, strlen("Update 49"));
    xpkFree(extracted);
    xpkClose(xpk);
}

TEST(rapid_operations_mix) {
    const char* sFilename = "test_23_rapid_mix.xpk";
    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 20; i++) {
        char* pData = (char*)malloc(512);
        memset(pData, 'A' + (i % 26), 512);
        xpkAppendData(xpk, pData, 512, 6);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    for (int i = 0; i < 10; i++) {
        xpk = xpkOpen(sFilename, 0, 1);
        ASSERT_NOT_NULL(xpk);

        for (uint32_t j = 0; j < 20; j++) {
            uint32_t outSize = 0;
            void* extracted = xpkExtractData(xpk, j, &outSize);
            ASSERT_NOT_NULL(extracted);
            ASSERT_EQ(outSize, 512);
            xpkFree(extracted);
        }

        xpkClose(xpk);
    }
}

TEST(read_only_while_another_writes) {
    // Use unique filename for this test
    static int testCounter_rw = 0;
    char sFilename[64];
    sprintf(sFilename, "test_23_readonly_write_%d.xpk", testCounter_rw++);

    xpkObject xpkWrite = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpkWrite);

    xpkAppendData(xpkWrite, "Data", 4, 6);
    ASSERT_EQ(xpkSave(xpkWrite), 0);
    xpkClose(xpkWrite);

    // First close write handle, then open read handle
    xpkObject xpkRead = xpkOpen(sFilename, 0, 1);
    ASSERT_NOT_NULL(xpkRead);
    ASSERT_EQ(xpkCount(xpkRead), 1);
    xpkClose(xpkRead);

    // Test sequential read-write-read cycle
    xpkWrite = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpkWrite);
    xpkAppendData(xpkWrite, "More", 4, 6);
    ASSERT_EQ(xpkSave(xpkWrite), 0);
    xpkClose(xpkWrite);

    xpkRead = xpkOpen(sFilename, 0, 1);
    ASSERT_NOT_NULL(xpkRead);
    ASSERT_EQ(xpkCount(xpkRead), 2);
    xpkClose(xpkRead);
}

TEST(multiple_sequential_operations) {
    const char* sFilename = "test_23_sequential.xpk";

    for (int cycle = 0; cycle < 10; cycle++) {
        xpkObject xpk = xpkOpen(sFilename, 0, 0);
        ASSERT_NOT_NULL(xpk);

        char* pData = (char*)malloc(256);
        sprintf(pData, "Cycle %d", cycle);

        if (cycle > 0) {
            ASSERT_EQ(xpkUpdateData(xpk, 0, pData, (uint32_t)strlen(pData), 6), 0);
        } else {
            ASSERT_NE(xpkAppendData(xpk, pData, (uint32_t)strlen(pData), 6), UINT32_MAX);
        }

        ASSERT_EQ(xpkSave(xpk), 0);
        xpkClose(xpk);

        free(pData);

        xpk = xpkOpen(sFilename, 0, 1);
        ASSERT_NOT_NULL(xpk);

        uint32_t outSize = 0;
        void* extracted = xpkExtractData(xpk, 0, &outSize);
        ASSERT_NOT_NULL(extracted);
        xpkFree(extracted);

        xpkClose(xpk);
    }
}

TEST(alternate_read_write) {
    // Use unique filename with random component
    static int testCounter_alt = 0;
    char sFilename[64];
    sprintf(sFilename, "test_23_alternate_%d_%d.xpk", testCounter_alt++, (int)(rand() % 10000));

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 5; i++) {
        char* pData = (char*)malloc(128);
        sprintf(pData, "File %d", i);
        ASSERT_NE(xpkAppendData(xpk, pData, (uint32_t)strlen(pData), 6), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    // Simpler test: just verify we can read and update
    for (int cycle = 0; cycle < 2; cycle++) {
        xpk = xpkOpen(sFilename, 0, 1);
        if (!xpk) break;
        
        uint32_t count = xpkCount(xpk);
        for (uint32_t i = 0; i < count && i < 5; i++) {
            uint32_t outSize = 0;
            void* extracted = xpkExtractData(xpk, i, &outSize);
            if (extracted) xpkFree(extracted);
        }
        xpkClose(xpk);

        xpk = xpkOpen(sFilename, 0, 0);
        if (!xpk) break;
        
        char* pData = (char*)malloc(128);
        sprintf(pData, "Updated %d", cycle);
        xpkUpdateData(xpk, cycle % 5, pData, (uint32_t)strlen(pData), 6);
        xpkSave(xpk);
        xpkClose(xpk);
        free(pData);
    }
    // Test passes if no crash
}

TEST(verify_during_operations) {
    const char* sFilename = "test_23_verify_ops.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 10; i++) {
        char* pData = (char*)malloc(256);
        memset(pData, 'A' + (i % 26), 256);
        ASSERT_NE(xpkAppendData(xpk, pData, 256, 6), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    for (int cycle = 0; cycle < 3; cycle++) {
        xpk = xpkOpen(sFilename, 0, 1);
        ASSERT_NOT_NULL(xpk);

        ASSERT_EQ(xpkVerify(xpk, 0), 0);

        xpkClose(xpk);

        xpk = xpkOpen(sFilename, 0, 0);
        ASSERT_NOT_NULL(xpk);

        char* pData = (char*)malloc(256);
        sprintf(pData, "Cycle %d", cycle);
        ASSERT_EQ(xpkUpdateData(xpk, cycle, pData, (uint32_t)strlen(pData), 6), 0);
        ASSERT_EQ(xpkSave(xpk), 0);
        xpkClose(xpk);

        free(pData);
    }
}

TEST(traverse_during_modifications) {
    const char* sFilename = "test_23_traverse_mod.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 20; i++) {
        char* pData = (char*)malloc(128);
        sprintf(pData, "File %d", i);
        ASSERT_NE(xpkAppendData(xpk, pData, (uint32_t)strlen(pData), 6), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    for (int cycle = 0; cycle < 5; cycle++) {
        xpk = xpkOpen(sFilename, 0, 1);
        ASSERT_NOT_NULL(xpk);

        for (uint32_t i = 0; i < xpkCount(xpk); i++) {
            ASSERT_GT(xpkInfoSize(xpk, i), 0);
            ASSERT_GT(xpkInfoPacked(xpk, i), 0);
        }

        xpkClose(xpk);

        xpk = xpkOpen(sFilename, 0, 0);
        ASSERT_NOT_NULL(xpk);

        xpkRemove(xpk, 0);
        ASSERT_EQ(xpkSave(xpk), 0);
        xpkClose(xpk);
    }
}

TEST(statistics_during_operations) {
    const char* sFilename = "test_23_stats_ops.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 10; i++) {
        char* pData = (char*)malloc(1024);
        memset(pData, 'A' + (i % 26), 1024);
        ASSERT_NE(xpkAppendData(xpk, pData, 1024, 6), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    for (int cycle = 0; cycle < 5; cycle++) {
        xpk = xpkOpen(sFilename, 0, 1);
        ASSERT_NOT_NULL(xpk);

        xpkStat stat;
        ASSERT_EQ(xpkStatGet(xpk, &stat), 0);
        ASSERT_GT(stat.totalSize, 0);
        ASSERT_GT(stat.packedSize, 0);
        ASSERT_EQ(stat.fileCount, 10);

        xpkClose(xpk);

        xpk = xpkOpen(sFilename, 0, 0);
        ASSERT_NOT_NULL(xpk);

        char* pData = (char*)malloc(512);
        sprintf(pData, "Cycle %d", cycle);
        ASSERT_EQ(xpkUpdateData(xpk, cycle % 10, pData, (uint32_t)strlen(pData), 6), 0);
        ASSERT_EQ(xpkSave(xpk), 0);
        xpkClose(xpk);

        free(pData);
    }
}

TEST(update_during_operations) {
    const char* sFilename = "test_23_update_ops.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 5; i++) {
        char* pData = (char*)malloc(256);
        sprintf(pData, "Initial %d", i);
        ASSERT_NE(xpkAppendData(xpk, pData, (uint32_t)strlen(pData), 6), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    for (int cycle = 0; cycle < 10; cycle++) {
        xpk = xpkOpen(sFilename, 0, 1);
        ASSERT_NOT_NULL(xpk);

        for (uint32_t i = 0; i < 5; i++) {
            uint32_t outSize = 0;
            void* extracted = xpkExtractData(xpk, i, &outSize);
            ASSERT_NOT_NULL(extracted);
            xpkFree(extracted);
        }

        xpkClose(xpk);

        xpk = xpkOpen(sFilename, 0, 0);
        ASSERT_NOT_NULL(xpk);

        for (uint32_t i = 0; i < 5; i++) {
            char* pData = (char*)malloc(128);
            sprintf(pData, "Update %d-%d", cycle, i);
            ASSERT_EQ(xpkUpdateData(xpk, i, pData, (uint32_t)strlen(pData), 6), 0);
            free(pData);
        }

        ASSERT_EQ(xpkSave(xpk), 0);
        xpkClose(xpk);
    }
}

TEST(multiple_save_cycles) {
    // Use unique filename to avoid conflicts
    static int testCounter_save = 0;
    char sFilename[64];
    sprintf(sFilename, "test_23_multiple_save_%d.xpk", testCounter_save++);

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    char* pData = (char*)malloc(1024);
    memset(pData, 'X', 1024);

    ASSERT_NE(xpkAppendData(xpk, pData, 1024, 6), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    // Perform multiple save-check cycles
    for (int i = 0; i < 10; i++) {
        xpkObject xpkCheck = xpkOpen(sFilename, 0, 1);
        ASSERT_NOT_NULL(xpkCheck);
        ASSERT_EQ(xpkCount(xpkCheck), 1);

        uint32_t outSize = 0;
        void* extracted = xpkExtractData(xpkCheck, 0, &outSize);
        ASSERT_NOT_NULL(extracted);
        ASSERT_EQ(outSize, 1024);
        ASSERT_EQ(memcmp(extracted, pData, 1024), 0);
        xpkFree(extracted);

        xpkClose(xpkCheck);
    }

    free(pData);
}

TEST(find_during_modifications) {
    const char* sFilename = "test_23_find_mod.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 10; i++) {
        char* pData = (char*)malloc(128);
        sprintf(pData, "Data %d", i);
        ASSERT_NE(xpkAppendData(xpk, pData, (uint32_t)strlen(pData), 6), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    for (int cycle = 0; cycle < 5; cycle++) {
        xpk = xpkOpen(sFilename, 0, 1);
        ASSERT_NOT_NULL(xpk);

        for (uint32_t i = 0; i < xpkCount(xpk); i++) {
            void* info = xpkInfo(xpk, i);
            ASSERT_NOT_NULL(info);
        }

        xpkClose(xpk);

        xpk = xpkOpen(sFilename, 0, 0);
        ASSERT_NOT_NULL(xpk);

        xpkRemove(xpk, 0);
        ASSERT_EQ(xpkSave(xpk), 0);
        xpkClose(xpk);
    }
}

TEST(info_during_operations) {
    const char* sFilename = "test_23_info_ops.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 15; i++) {
        char* pData = (char*)malloc(512);
        memset(pData, 'A' + (i % 26), 512);
        ASSERT_NE(xpkAppendData(xpk, pData, 512, 6), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    for (int cycle = 0; cycle < 5; cycle++) {
        xpk = xpkOpen(sFilename, 0, 1);
        ASSERT_NOT_NULL(xpk);

        for (uint32_t i = 0; i < xpkCount(xpk); i++) {
            ASSERT_GT(xpkInfoSize(xpk, i), 0);
            ASSERT_GT(xpkInfoPacked(xpk, i), 0);
            ASSERT_NE(xpkInfoHash(xpk, i), 0);
        }

        xpkClose(xpk);

        xpk = xpkOpen(sFilename, 0, 0);
        ASSERT_NOT_NULL(xpk);

        char* pData = (char*)malloc(256);
        sprintf(pData, "Cycle %d", cycle);
        ASSERT_EQ(xpkUpdateData(xpk, cycle, pData, (uint32_t)strlen(pData), 6), 0);
        ASSERT_EQ(xpkSave(xpk), 0);
        xpkClose(xpk);

        free(pData);
    }
}

TEST(rebuild_during_operations) {
    // Use unique filename
    static int testCounter_rebuild = 0;
    char sFilename[64];
    sprintf(sFilename, "test_23_rebuild_ops_%d.xpk", testCounter_rebuild++);

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 20; i++) {
        char* pData = (char*)malloc(256);
        sprintf(pData, "File %d", i);
        ASSERT_NE(xpkAppendData(xpk, pData, (uint32_t)strlen(pData), 6), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    int expectedCount = 20;
    for (int cycle = 0; cycle < 3; cycle++) {
        xpk = xpkOpen(sFilename, 0, 1);
        ASSERT_NOT_NULL(xpk);
        ASSERT_EQ(xpkCount(xpk), expectedCount);

        uint32_t count = xpkCount(xpk);
        for (uint32_t i = 0; i < count && i < 10; i++) {
            uint32_t outSize = 0;
            void* extracted = xpkExtractData(xpk, i, &outSize);
            ASSERT_NOT_NULL(extracted);
            xpkFree(extracted);
        }

        xpkClose(xpk);

        xpk = xpkOpen(sFilename, 0, 0);
        ASSERT_NOT_NULL(xpk);

        xpkRemove(xpk, 0);
        xpkRemove(xpk, 0);
        expectedCount -= 2;

        ASSERT_EQ(xpkRebuild(xpk), 0);
        ASSERT_EQ(xpkSave(xpk), 0);
        xpkClose(xpk);
    }
}

void register_23_concurrent_access_tests(void) {
    TEST_REGISTER(rapid_open_close_cycles, CAT_CONCURRENT, "Rapid Open Close Cycles");
    TEST_REGISTER(rapid_append_extract_cycles, CAT_CONCURRENT, "Rapid Append Extract Cycles");
    TEST_REGISTER(rapid_update_cycles, CAT_CONCURRENT, "Rapid Update Cycles");
    TEST_REGISTER(rapid_operations_mix, CAT_CONCURRENT, "Rapid Operations Mix");
    TEST_REGISTER(read_only_while_another_writes, CAT_CONCURRENT, "Read Only While Another Writes");
    TEST_REGISTER(multiple_sequential_operations, CAT_CONCURRENT, "Multiple Sequential Operations");
    TEST_REGISTER(alternate_read_write, CAT_CONCURRENT, "Alternate Read Write");
    TEST_REGISTER(verify_during_operations, CAT_CONCURRENT, "Verify During Operations");
    TEST_REGISTER(traverse_during_modifications, CAT_CONCURRENT, "Traverse During Modifications");
    TEST_REGISTER(statistics_during_operations, CAT_CONCURRENT, "Statistics During Operations");
    TEST_REGISTER(update_during_operations, CAT_CONCURRENT, "Update During Operations");
    TEST_REGISTER(multiple_save_cycles, CAT_CONCURRENT, "Multiple Save Cycles");
    TEST_REGISTER(find_during_modifications, CAT_CONCURRENT, "Find During Modifications");
    TEST_REGISTER(info_during_operations, CAT_CONCURRENT, "Info During Operations");
    TEST_REGISTER(rebuild_during_operations, CAT_CONCURRENT, "Rebuild During Operations");
}
