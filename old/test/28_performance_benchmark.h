/*
 * xPack Ver7 - performance_benchmark (28)
 */

#include "test_framework.h"
#include <stdlib.h>

TEST(benchmark_append_small_files) {
    const char* sFilename = "test_28_bench_append_small.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 1000; i++) {
        char* pData = (char*)malloc(512);
        memset(pData, 'A' + (i % 26), 512);
        ASSERT_EQ(xpkAppendData(xpk, pData, 512, i), 0);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);
}

TEST(benchmark_append_large_files) {
    const char* sFilename = "test_28_bench_append_large.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 10; i++) {
        char* pData = (char*)malloc(5 * 1024 * 1024);
        memset(pData, 'A' + (i % 26), 5 * 1024 * 1024);
        ASSERT_EQ(xpkAppendData(xpk, pData, 5 * 1024 * 1024, i), 0);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);
}

TEST(benchmark_extract_all) {
    const char* sFilename = "test_28_bench_extract_all.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 500; i++) {
        char* pData = (char*)malloc(4096);
        memset(pData, 'A' + (i % 26), 4096);
        xpkAppendData(xpk, pData, 4096, 6);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 1);
    ASSERT_NOT_NULL(xpk);

    for (uint32_t i = 0; i < xpkCount(xpk); i++) {
        uint32_t outSize = 0;
        void* extracted = xpkExtractData(xpk, i, &outSize);
        ASSERT_NOT_NULL(extracted);
        ASSERT_EQ(outSize, 4096);
        xpkFree(extracted);
    }

    xpkClose(xpk);
}

TEST(benchmark_verify_all) {
    const char* sFilename = "test_28_bench_verify_all.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 200; i++) {
        char* pData = (char*)malloc(2048);
        memset(pData, 'A' + (i % 26), 2048);
        xpkAppendData(xpk, pData, 2048, 6);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 1);
    ASSERT_NOT_NULL(xpk);

    for (uint32_t i = 0; i < xpkCount(xpk); i++) {
        ASSERT_EQ(xpkVerify(xpk, i), 0);
    }

    xpkClose(xpk);
}

TEST(benchmark_traverse_all) {
    const char* sFilename = "test_28_bench_traverse_all.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 300; i++) {
        char* pData = (char*)malloc(1024);
        memset(pData, 'A' + (i % 26), 1024);
        xpkAppendData(xpk, pData, 1024, 6);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 1);
    ASSERT_NOT_NULL(xpk);

    for (uint32_t i = 0; i < xpkCount(xpk); i++) {
        uint32_t size = xpkInfoSize(xpk, i);
        uint32_t packed = xpkInfoPacked(xpk, i);
        ASSERT_GT(size, 0);
        ASSERT_GT(packed, 0);
    }

    xpkClose(xpk);
}

TEST(benchmark_find_operations) {
    const char* sFilename = "test_28_bench_find.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 100; i++) {
        char* pData = (char*)malloc(512);
        memset(pData, 'A' + (i % 26), 512);
        xpkAppendData(xpk, pData, 512, 6);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 100);

    for (uint32_t i = 0; i < 100; i++) {
        uint32_t size = xpkInfoSize(xpk, i);
        ASSERT_GT(size, 0);
    }

    xpkClose(xpk);
}

TEST(benchmark_rebuild) {
    const char* sFilename = "test_28_bench_rebuild.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 50; i++) {
        char* pData = (char*)malloc(2048);
        memset(pData, 'A' + (i % 26), 2048);
        xpkAppendData(xpk, pData, 2048, 6);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 1);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkRebuild(xpk), 0);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 50);
    xpkClose(xpk);
}

TEST(benchmark_save_load_cycles) {
    const char* sFilename = "test_28_bench_save_load.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 20; i++) {
        char* pData = (char*)malloc(4096);
        memset(pData, 'A' + (i % 26), 4096);
        xpkAppendData(xpk, pData, 4096, 6);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    for (int i = 0; i < 10; i++) {
        xpk = xpkOpen(sFilename, 0, 1);
        ASSERT_NOT_NULL(xpk);
        ASSERT_EQ(xpkCount(xpk), 20);
        xpkClose(xpk);
    }
}

TEST(benchmark_update_operations) {
    const char* sFilename = "test_28_bench_update.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 30; i++) {
        char* pData = (char*)malloc(2048);
        memset(pData, 'A' + (i % 26), 2048);
        xpkAppendData(xpk, pData, 2048, 6);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (uint32_t i = 0; i < 30; i++) {
        char* pData = (char*)malloc(2048);
        memset(pData, 'X', 2048);
        ASSERT_EQ(xpkUpdateData(xpk, i, pData, 2048, 6), 0);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);
}

TEST(benchmark_remove_operations) {
    const char* sFilename = "test_28_bench_remove.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 50; i++) {
        char* pData = (char*)malloc(1024);
        memset(pData, 'A' + (i % 26), 1024);
        xpkAppendData(xpk, pData, 1024, 6);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 25; i++) {
        ASSERT_EQ(xpkRemove(xpk, 0), 0);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 25);
    xpkClose(xpk);
}

TEST(benchmark_statistics) {
    const char* sFilename = "test_28_bench_stats.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 100; i++) {
        char* pData = (char*)malloc(2048);
        memset(pData, 'A' + (i % 26), 2048);
        xpkAppendData(xpk, pData, 2048, 6);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 1);
    ASSERT_NOT_NULL(xpk);

    xpkStat stat;
    ASSERT_EQ(xpkStatGet(xpk, &stat), 0);
    ASSERT_EQ(stat.fileCount, 100);
    ASSERT_GT(stat.totalSize, 0);
    ASSERT_GT(stat.packedSize, 0);
    ASSERT_GT(stat.ratio, 0.0);

    xpkClose(xpk);
}

TEST(benchmark_compression_levels) {
    for (int level = 0; level <= 12; level++) {
        char filename[64];
        sprintf(filename, "test_28_bench_level_%d.xpk", level);

        xpkObject xpk = xpkOpen(filename, 0, 0);
        ASSERT_NOT_NULL(xpk);

        for (int i = 0; i < 10; i++) {
            char* pData = (char*)malloc(8192);
            memset(pData, 'A' + (i % 26), 8192);
            xpkAppendData(xpk, pData, 8192, level);
            free(pData);
        }

        ASSERT_EQ(xpkSave(xpk), 0);
        xpkClose(xpk);
    }
}

void register_28_performance_benchmark_tests(void) {
    TEST_REGISTER(benchmark_append_small_files, CAT_PERFORMANCE, "Benchmark Append Small Files");
    TEST_REGISTER(benchmark_append_large_files, CAT_PERFORMANCE, "Benchmark Append Large Files");
    TEST_REGISTER(benchmark_extract_all, CAT_PERFORMANCE, "Benchmark Extract All");
    TEST_REGISTER(benchmark_verify_all, CAT_PERFORMANCE, "Benchmark Verify All");
    TEST_REGISTER(benchmark_traverse_all, CAT_PERFORMANCE, "Benchmark Traverse All");
    TEST_REGISTER(benchmark_find_operations, CAT_PERFORMANCE, "Benchmark Find Operations");
    TEST_REGISTER(benchmark_rebuild, CAT_PERFORMANCE, "Benchmark Rebuild");
    TEST_REGISTER(benchmark_save_load_cycles, CAT_PERFORMANCE, "Benchmark Save Load Cycles");
    TEST_REGISTER(benchmark_update_operations, CAT_PERFORMANCE, "Benchmark Update Operations");
    TEST_REGISTER(benchmark_remove_operations, CAT_PERFORMANCE, "Benchmark Remove Operations");
    TEST_REGISTER(benchmark_statistics, CAT_PERFORMANCE, "Benchmark Statistics");
    TEST_REGISTER(benchmark_compression_levels, CAT_PERFORMANCE, "Benchmark Compression Levels");
}
