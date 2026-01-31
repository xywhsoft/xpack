/*
 * xPack Ver7 - Benchmark Test Runner
 */

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../src/xpack.h"
#include "benchmark_framework.h"

static int benchmark_run_count = 5;
static int benchmark_output_format = 0; // 0=console, 1=csv, 2=json
static const char* benchmark_output_file = "benchmark_report";

static void benchmark_compression_lz4(void) {
    printf("\n=== Benchmark: LZ4 Compression ===\n");

    BenchmarkStats stats;
    benchmark_stats_init(&stats);

    for (int i = 0; i < benchmark_run_count; i++) {
        BenchmarkResult result;
        benchmark_result_init(&result, "LZ4 Compression");

        size_t size = 100 * 1024 * 1024;
        void* data = malloc(size);
        if (!data) {
            printf("  [ERROR] Failed to allocate memory\n");
            return;
        }

        memset(data, 'A' + (i % 26), size);
        result.data_size = size;

        char filename[64];
        sprintf(filename, "bench_lz4_%d.xpk", i);

        uint64_t start_time = benchmark_get_time_ms();

        xpkObject xpk = xpkOpen(filename, 0, 0);
        if (xpk) {
            xpkAppendData(xpk, data, (uint32_t)size, 1);
            xpkSave(xpk);
            xpkClose(xpk);
        }

        uint64_t end_time = benchmark_get_time_ms();

        result.time_ms = end_time - start_time;

        if (result.time_ms > 0) {
            result.speed_mb_per_sec = (size * 1000) / result.time_ms;
        }

        free(data);

        benchmark_stats_add(&stats, &result);
    }

    for (int i = 0; i < stats.run_count; i++) {
        benchmark_result_print(&stats.runs[i]);
    }

    benchmark_stats_compute(&stats);
    benchmark_stats_print(&stats);

    if (benchmark_output_format == 1) {
        char filename[256];
        sprintf(filename, "%s_lz4.csv", benchmark_output_file);
        benchmark_stats_print_csv(&stats, filename);
    } else if (benchmark_output_format == 2) {
        char filename[256];
        sprintf(filename, "%s_lz4.json", benchmark_output_file);
        benchmark_stats_print_json(&stats, filename);
    }
}

static void benchmark_compression_zstd(void) {
    printf("\n=== Benchmark: ZSTD Compression ===\n");

    BenchmarkStats stats;
    benchmark_stats_init(&stats);

    for (int i = 0; i < benchmark_run_count; i++) {
        BenchmarkResult result;
        benchmark_result_init(&result, "ZSTD Compression");

        size_t size = 100 * 1024 * 1024;
        void* data = malloc(size);
        if (!data) {
            printf("  [ERROR] Failed to allocate memory\n");
            return;
        }

        memset(data, 'B' + (i % 26), size);
        result.data_size = size;

        char filename[64];
        sprintf(filename, "bench_zstd_%d.xpk", i);

        uint64_t start_time = benchmark_get_time_ms();

        xpkObject xpk = xpkOpen(filename, 0, 0);
        if (xpk) {
            xpkAppendData(xpk, data, (uint32_t)size, 6);
            xpkSave(xpk);
            xpkClose(xpk);
        }

        uint64_t end_time = benchmark_get_time_ms();

        result.time_ms = end_time - start_time;

        if (result.time_ms > 0) {
            result.speed_mb_per_sec = (size * 1000) / result.time_ms;
        }

        free(data);

        benchmark_stats_add(&stats, &result);
    }

    for (int i = 0; i < stats.run_count; i++) {
        benchmark_result_print(&stats.runs[i]);
    }

    benchmark_stats_compute(&stats);
    benchmark_stats_print(&stats);

    if (benchmark_output_format == 1) {
        char filename[256];
        sprintf(filename, "%s_zstd.csv", benchmark_output_file);
        benchmark_stats_print_csv(&stats, filename);
    } else if (benchmark_output_format == 2) {
        char filename[256];
        sprintf(filename, "%s_zstd.json", benchmark_output_file);
        benchmark_stats_print_json(&stats, filename);
    }
}

static void benchmark_compression_lzma2(void) {
    printf("\n=== Benchmark: LZMA2 Compression ===\n");

    BenchmarkStats stats;
    benchmark_stats_init(&stats);

    for (int i = 0; i < benchmark_run_count; i++) {
        BenchmarkResult result;
        benchmark_result_init(&result, "LZMA2 Compression");

        size_t size = 50 * 1024 * 1024;
        void* data = malloc(size);
        if (!data) {
            printf("  [ERROR] Failed to allocate memory\n");
            return;
        }

        memset(data, 'C' + (i % 26), size);
        result.data_size = size;

        char filename[64];
        sprintf(filename, "bench_lzma2_%d.xpk", i);

        uint64_t start_time = benchmark_get_time_ms();

        xpkObject xpk = xpkOpen(filename, 0, 0);
        if (xpk) {
            xpkAppendData(xpk, data, (uint32_t)size, 11);
            xpkSave(xpk);
            xpkClose(xpk);
        }

        uint64_t end_time = benchmark_get_time_ms();

        result.time_ms = end_time - start_time;

        if (result.time_ms > 0) {
            result.speed_mb_per_sec = (size * 1000) / result.time_ms;
        }

        free(data);

        benchmark_stats_add(&stats, &result);
    }

    for (int i = 0; i < stats.run_count; i++) {
        benchmark_result_print(&stats.runs[i]);
    }

    benchmark_stats_compute(&stats);
    benchmark_stats_print(&stats);

    if (benchmark_output_format == 1) {
        char filename[256];
        sprintf(filename, "%s_lzma2.csv", benchmark_output_file);
        benchmark_stats_print_csv(&stats, filename);
    } else if (benchmark_output_format == 2) {
        char filename[256];
        sprintf(filename, "%s_lzma2.json", benchmark_output_file);
        benchmark_stats_print_json(&stats, filename);
    }
}

static void benchmark_decompression(void) {
    printf("\n=== Benchmark: Decompression ===\n");

    BenchmarkStats stats;
    benchmark_stats_init(&stats);

    char filename[64];
    sprintf(filename, "bench_decomp.xpk");

    xpkObject xpk = xpkOpen(filename, 0, 0);
    if (xpk) {
        size_t size = 100 * 1024 * 1024;
        void* data = malloc(size);
        if (data) {
            memset(data, 'D', size);
            xpkAppendData(xpk, data, (uint32_t)size, 6);
            xpkSave(xpk);
            free(data);
        }
        xpkClose(xpk);
    }

    xpk = xpkOpen(filename, 0, 1);
    if (xpk) {
        for (int i = 0; i < benchmark_run_count; i++) {
            BenchmarkResult result;
            benchmark_result_init(&result, "Decompression");

            uint32_t size = xpkInfoSize(xpk, 0);
            result.data_size = size;

            uint64_t start_time = benchmark_get_time_ms();

            uint32_t outSize = 0;
            void* extracted = xpkExtractData(xpk, 0, &outSize);

            uint64_t end_time = benchmark_get_time_ms();

            result.time_ms = end_time - start_time;

            if (result.time_ms > 0) {
                result.speed_mb_per_sec = (size * 1000) / result.time_ms;
            }

            if (extracted) {
                xpkFree(extracted);
            }

            benchmark_stats_add(&stats, &result);
        }
        xpkClose(xpk);
    }

    for (int i = 0; i < stats.run_count; i++) {
        benchmark_result_print(&stats.runs[i]);
    }

    benchmark_stats_compute(&stats);
    benchmark_stats_print(&stats);

    if (benchmark_output_format == 1) {
        char filename[256];
        sprintf(filename, "%s_decompression.csv", benchmark_output_file);
        benchmark_stats_print_csv(&stats, filename);
    } else if (benchmark_output_format == 2) {
        char filename[256];
        sprintf(filename, "%s_decompression.json", benchmark_output_file);
        benchmark_stats_print_json(&stats, filename);
    }
}

static void benchmark_file_operations(void) {
    printf("\n=== Benchmark: File Operations ===\n");

    BenchmarkStats stats_append;
    BenchmarkStats stats_remove;
    BenchmarkStats stats_update;

    benchmark_stats_init(&stats_append);
    benchmark_stats_init(&stats_remove);
    benchmark_stats_init(&stats_update);

    for (int i = 0; i < benchmark_run_count; i++) {
        char filename[64];
        sprintf(filename, "bench_file_ops_%d.xpk", i);

        xpkObject xpk = xpkOpen(filename, 0, 0);
        if (xpk) {
            BenchmarkResult result_append;
            benchmark_result_init(&result_append, "Append 1000 files");

            uint64_t start_time = benchmark_get_time_ms();

            for (int j = 0; j < 1000; j++) {
                char data[1024];
                sprintf(data, "File %d", j);
                xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
            }

            uint64_t end_time = benchmark_get_time_ms();

            result_append.time_ms = end_time - start_time;
            benchmark_stats_add(&stats_append, &result_append);

            xpkSave(xpk);
            xpkClose(xpk);
        }

        xpk = xpkOpen(filename, 0, 0);
        if (xpk) {
            BenchmarkResult result_remove;
            benchmark_result_init(&result_remove, "Remove 500 files");

            uint64_t start_time = benchmark_get_time_ms();

            for (int j = 0; j < 500; j++) {
                xpkRemove(xpk, 0);
            }

            uint64_t end_time = benchmark_get_time_ms();

            result_remove.time_ms = end_time - start_time;
            benchmark_stats_add(&stats_remove, &result_remove);

            xpkSave(xpk);
            xpkClose(xpk);
        }

        xpk = xpkOpen(filename, 0, 0);
        if (xpk) {
            BenchmarkResult result_update;
            benchmark_result_init(&result_update, "Update 500 files");

            uint64_t start_time = benchmark_get_time_ms();

            for (int j = 0; j < 500; j++) {
                char data[1024];
                sprintf(data, "Updated file %d", j);
                xpkUpdateData(xpk, j, data, (uint32_t)strlen(data), 6);
            }

            uint64_t end_time = benchmark_get_time_ms();

            result_update.time_ms = end_time - start_time;
            benchmark_stats_add(&stats_update, &result_update);

            xpkSave(xpk);
            xpkClose(xpk);
        }
    }

    printf("\n  Append operations:\n");
    for (int i = 0; i < stats_append.run_count; i++) {
        benchmark_result_print(&stats_append.runs[i]);
    }
    benchmark_stats_compute(&stats_append);
    benchmark_stats_print(&stats_append);

    printf("\n  Remove operations:\n");
    for (int i = 0; i < stats_remove.run_count; i++) {
        benchmark_result_print(&stats_remove.runs[i]);
    }
    benchmark_stats_compute(&stats_remove);
    benchmark_stats_print(&stats_remove);

    printf("\n  Update operations:\n");
    for (int i = 0; i < stats_update.run_count; i++) {
        benchmark_result_print(&stats_update.runs[i]);
    }
    benchmark_stats_compute(&stats_update);
    benchmark_stats_print(&stats_update);

    if (benchmark_output_format == 1) {
        char filename[256];
        sprintf(filename, "%s_file_ops.csv", benchmark_output_file);
        benchmark_stats_print_csv(&stats_append, filename);
    } else if (benchmark_output_format == 2) {
        char filename[256];
        sprintf(filename, "%s_file_ops.json", benchmark_output_file);
        benchmark_stats_print_json(&stats_append, filename);
    }
}

static void print_usage(void) {
    printf("Usage: xpack_benchmark_runner [options]\n");
    printf("Options:\n");
    printf("  -r <count>     Number of runs per benchmark (default: 5)\n");
    printf("  -o <format>    Output format: 0=console, 1=csv, 2=json (default: 0)\n");
    printf("  -f <filename>   Output file prefix (default: benchmark_report)\n");
    printf("  -h             Show this help\n");
}

int main(int argc, char* argv[]) {
    printf("xPack Ver7 - Benchmark Test Runner\n");
    printf("===================================\n");

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            benchmark_run_count = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            benchmark_output_format = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            benchmark_output_file = argv[++i];
        } else if (strcmp(argv[i], "-h") == 0) {
            print_usage();
            return 0;
        }
    }

    printf("\nConfiguration:\n");
    printf("  Runs per benchmark: %d\n", benchmark_run_count);
    printf("  Output format: %s\n", benchmark_output_format == 0 ? "console" : (benchmark_output_format == 1 ? "CSV" : "JSON"));
    printf("  Output file prefix: %s\n", benchmark_output_file);

    benchmark_compression_lz4();
    benchmark_compression_zstd();
    benchmark_compression_lzma2();
    benchmark_decompression();
    benchmark_file_operations();

    printf("\n=== All Benchmarks Complete ===\n");
    printf("Results saved to %s.*\n", benchmark_output_file);

    return 0;
}
