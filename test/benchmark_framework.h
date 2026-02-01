/*
 * xPack Ver7 - Benchmark Framework
 */

#ifndef BENCHMARK_FRAMEWORK_H
#define BENCHMARK_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#ifndef UINT64_MAX
#define UINT64_MAX ((uint64_t)-1)
#endif

#define BENCHMARK_MAX_NAME_LEN 128
#define BENCHMARK_MAX_RUNS 100

typedef struct {
    char name[BENCHMARK_MAX_NAME_LEN];
    uint64_t data_size;
    uint64_t time_ms;
    uint64_t speed_mb_per_sec;
    uint64_t peak_memory_kb;
    char notes[256];
} BenchmarkResult;

typedef struct {
    BenchmarkResult runs[BENCHMARK_MAX_RUNS];
    int run_count;
    uint64_t total_time_ms;
    uint64_t min_time_ms;
    uint64_t max_time_ms;
    uint64_t avg_time_ms;
    uint64_t median_time_ms;
} BenchmarkStats;

static uint64_t benchmark_get_time_ms(void) {
    return (uint64_t)clock() * 1000 / CLOCKS_PER_SEC;
}

static void benchmark_result_init(BenchmarkResult* result, const char* name) {
    strncpy(result->name, name, BENCHMARK_MAX_NAME_LEN - 1);
    result->name[BENCHMARK_MAX_NAME_LEN - 1] = '\0';
    result->data_size = 0;
    result->time_ms = 0;
    result->speed_mb_per_sec = 0;
    result->peak_memory_kb = 0;
    result->notes[0] = '\0';
}

static void benchmark_stats_init(BenchmarkStats* stats) {
    memset(stats, 0, sizeof(BenchmarkStats));
}

static void benchmark_stats_add(BenchmarkStats* stats, const BenchmarkResult* result) {
    if (stats->run_count >= BENCHMARK_MAX_RUNS) {
        return;
    }

    memcpy(&stats->runs[stats->run_count], result, sizeof(BenchmarkResult));
    stats->total_time_ms += result->time_ms;

    if (stats->run_count == 0) {
        stats->min_time_ms = result->time_ms;
        stats->max_time_ms = result->time_ms;
    } else {
        if (result->time_ms < stats->min_time_ms) {
            stats->min_time_ms = result->time_ms;
        }
        if (result->time_ms > stats->max_time_ms) {
            stats->max_time_ms = result->time_ms;
        }
    }

    stats->run_count++;
    stats->avg_time_ms = stats->total_time_ms / stats->run_count;
}

static void benchmark_stats_compute(BenchmarkStats* stats) {
    if (stats->run_count == 0) {
        return;
    }

    // 重置统计值（因为 benchmark_stats_add 已经计算过，这里重新计算以确保正确性）
    stats->total_time_ms = 0;
    stats->min_time_ms = UINT64_MAX;
    stats->max_time_ms = 0;

    for (int i = 0; i < stats->run_count; i++) {
        stats->total_time_ms += stats->runs[i].time_ms;
        if (stats->runs[i].time_ms < stats->min_time_ms) {
            stats->min_time_ms = stats->runs[i].time_ms;
        }
        if (stats->runs[i].time_ms > stats->max_time_ms) {
            stats->max_time_ms = stats->runs[i].time_ms;
        }
    }

    stats->avg_time_ms = stats->total_time_ms / stats->run_count;

    if (stats->run_count > 1) {
        uint64_t sorted[BENCHMARK_MAX_RUNS];
        for (int i = 0; i < stats->run_count; i++) {
            sorted[i] = stats->runs[i].time_ms;
        }

        for (int i = 0; i < stats->run_count - 1; i++) {
            for (int j = i + 1; j < stats->run_count; j++) {
                if (sorted[i] > sorted[j]) {
                    uint64_t temp = sorted[i];
                    sorted[i] = sorted[j];
                    sorted[j] = temp;
                }
            }
        }

        stats->median_time_ms = sorted[stats->run_count / 2];
    }
}

static void benchmark_result_print(const BenchmarkResult* result) {
    printf("  %-40s ", result->name);

    if (result->data_size > 0) {
        printf("Size: %8.2f MB  ", result->data_size / (1024.0 * 1024.0));
    }

    printf("Time: %6llu ms  ", (unsigned long long)result->time_ms);

    if (result->speed_mb_per_sec > 0) {
        printf("Speed: %8.2f MB/s  ", (double)result->speed_mb_per_sec);
    }

    if (result->peak_memory_kb > 0) {
        printf("Mem: %8llu KB", (unsigned long long)result->peak_memory_kb);
    }

    if (result->notes[0] != '\0') {
        printf("  [%s]", result->notes);
    }

    printf("\n");
}

static void benchmark_stats_print(const BenchmarkStats* stats) {
    printf("\n");
    printf("  Statistics for %d runs:\n", stats->run_count);
    printf("    Total time:   %llu ms\n", (unsigned long long)stats->total_time_ms);
    printf("    Average time: %llu ms\n", (unsigned long long)stats->avg_time_ms);
    printf("    Min time:     %llu ms\n", (unsigned long long)stats->min_time_ms);
    printf("    Max time:     %llu ms\n", (unsigned long long)stats->max_time_ms);
    printf("    Median time:  %llu ms\n", (unsigned long long)stats->median_time_ms);
    printf("\n");
}

static void benchmark_stats_print_csv(const BenchmarkStats* stats, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        return;
    }

    fprintf(fp, "Run,Name,DataSizeMB,TimeMS,SpeedMBSec,MemoryKB,Notes\n");
    for (int i = 0; i < stats->run_count; i++) {
        const BenchmarkResult* r = &stats->runs[i];
        fprintf(fp, "%d,%s,%.2f,%llu,%.2f,%llu,%s\n",
                i + 1,
                r->name,
                r->data_size / (1024.0 * 1024.0),
                (unsigned long long)r->time_ms,
                (double)r->speed_mb_per_sec / (1024.0 * 1024.0),
                (unsigned long long)r->peak_memory_kb,
                r->notes);
    }

    fclose(fp);
}

static void benchmark_stats_print_json(const BenchmarkStats* stats, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        return;
    }

    fprintf(fp, "{\n");
    fprintf(fp, "  \"run_count\": %d,\n", stats->run_count);
    fprintf(fp, "  \"total_time_ms\": %llu,\n", (unsigned long long)stats->total_time_ms);
    fprintf(fp, "  \"avg_time_ms\": %llu,\n", (unsigned long long)stats->avg_time_ms);
    fprintf(fp, "  \"min_time_ms\": %llu,\n", (unsigned long long)stats->min_time_ms);
    fprintf(fp, "  \"max_time_ms\": %llu,\n", (unsigned long long)stats->max_time_ms);
    fprintf(fp, "  \"median_time_ms\": %llu,\n", (unsigned long long)stats->median_time_ms);
    fprintf(fp, "  \"runs\": [\n");

    for (int i = 0; i < stats->run_count; i++) {
        const BenchmarkResult* r = &stats->runs[i];
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"name\": \"%s\",\n", r->name);
        fprintf(fp, "      \"data_size\": %llu,\n", (unsigned long long)r->data_size);
        fprintf(fp, "      \"time_ms\": %llu,\n", (unsigned long long)r->time_ms);
        fprintf(fp, "      \"speed_mb_per_sec\": %.2f,\n", (double)r->speed_mb_per_sec / (1024.0 * 1024.0));
        fprintf(fp, "      \"peak_memory_kb\": %llu,\n", (unsigned long long)r->peak_memory_kb);
        fprintf(fp, "      \"notes\": \"%s\"\n", r->notes);
        fprintf(fp, "    }%s\n", i < stats->run_count - 1 ? "," : "");
    }

    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");

    fclose(fp);
}

#endif // BENCHMARK_FRAMEWORK_H
