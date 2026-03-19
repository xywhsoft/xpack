/*
 * xPack Ver7 - Coverage Framework
 */

#ifndef COVERAGE_FRAMEWORK_H
#define COVERAGE_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define COVERAGE_MAX_FUNCTIONS 500
#define COVERAGE_MAX_NAME_LEN 128

typedef struct {
    char name[COVERAGE_MAX_NAME_LEN];
    int call_count;
    int branch_count;
    int branches_taken;
} CoverageFunction;

typedef struct {
    char filename[COVERAGE_MAX_NAME_LEN];
    int total_lines;
    int covered_lines;
    int total_functions;
    int covered_functions;
    int total_branches;
    int covered_branches;
    CoverageFunction functions[COVERAGE_MAX_FUNCTIONS];
    int function_count;
} CoverageFile;

typedef struct {
    CoverageFile files[50];
    int file_count;
    int total_lines;
    int covered_lines;
    int total_functions;
    int covered_functions;
    int total_branches;
    int covered_branches;
} CoverageStats;

static CoverageStats g_coverage = {0};

static void coverage_init(void) {
    memset(&g_coverage, 0, sizeof(CoverageStats));
}

static void coverage_file_init(const char* filename) {
    if (g_coverage.file_count >= 50) {
        return;
    }

    CoverageFile* file = &g_coverage.files[g_coverage.file_count++];
    strncpy(file->filename, filename, COVERAGE_MAX_NAME_LEN - 1);
    file->filename[COVERAGE_MAX_NAME_LEN - 1] = '\0';
    file->total_lines = 0;
    file->covered_lines = 0;
    file->total_functions = 0;
    file->covered_functions = 0;
    file->total_branches = 0;
    file->covered_branches = 0;
    file->function_count = 0;
}

static void coverage_function_add(const char* filename, const char* funcname) {
    CoverageFile* file = NULL;

    for (int i = 0; i < g_coverage.file_count; i++) {
        if (strcmp(g_coverage.files[i].filename, filename) == 0) {
            file = &g_coverage.files[i];
            break;
        }
    }

    if (!file) {
        return;
    }

    if (file->function_count >= COVERAGE_MAX_FUNCTIONS) {
        return;
    }

    CoverageFunction* func = &file->functions[file->function_count++];
    strncpy(func->name, funcname, COVERAGE_MAX_NAME_LEN - 1);
    func->name[COVERAGE_MAX_NAME_LEN - 1] = '\0';
    func->call_count = 0;
    func->branch_count = 0;
    func->branches_taken = 0;

    file->total_functions++;
    g_coverage.total_functions++;
}

static void coverage_function_call(const char* filename, const char* funcname) {
    for (int i = 0; i < g_coverage.file_count; i++) {
        if (strcmp(g_coverage.files[i].filename, filename) == 0) {
            CoverageFile* file = &g_coverage.files[i];
            for (int j = 0; j < file->function_count; j++) {
                if (strcmp(file->functions[j].name, funcname) == 0) {
                    file->functions[j].call_count++;
                    if (file->functions[j].call_count == 1) {
                        file->covered_functions++;
                        g_coverage.covered_functions++;
                    }
                    return;
                }
            }
            break;
        }
    }
}

static void coverage_branch_add(const char* filename, const char* funcname) {
    for (int i = 0; i < g_coverage.file_count; i++) {
        if (strcmp(g_coverage.files[i].filename, filename) == 0) {
            CoverageFile* file = &g_coverage.files[i];
            for (int j = 0; j < file->function_count; j++) {
                if (strcmp(file->functions[j].name, funcname) == 0) {
                    file->functions[j].branch_count++;
                    file->total_branches++;
                    g_coverage.total_branches++;
                    return;
                }
            }
            break;
        }
    }
}

static void coverage_branch_take(const char* filename, const char* funcname) {
    for (int i = 0; i < g_coverage.file_count; i++) {
        if (strcmp(g_coverage.files[i].filename, filename) == 0) {
            CoverageFile* file = &g_coverage.files[i];
            for (int j = 0; j < file->function_count; j++) {
                if (strcmp(file->functions[j].name, funcname) == 0) {
                    if (file->functions[j].branches_taken < file->functions[j].branch_count) {
                        file->functions[j].branches_taken++;
                        file->covered_branches++;
                        g_coverage.covered_branches++;
                    }
                    return;
                }
            }
            break;
        }
    }
}

static void coverage_line_add(const char* filename, int count) {
    for (int i = 0; i < g_coverage.file_count; i++) {
        if (strcmp(g_coverage.files[i].filename, filename) == 0) {
            g_coverage.files[i].total_lines += count;
            g_coverage.total_lines += count;
            break;
        }
    }
}

static void coverage_line_cover(const char* filename, int count) {
    for (int i = 0; i < g_coverage.file_count; i++) {
        if (strcmp(g_coverage.files[i].filename, filename) == 0) {
            int new_covered = g_coverage.files[i].covered_lines + count;
            if (new_covered <= g_coverage.files[i].total_lines) {
                g_coverage.files[i].covered_lines = new_covered;
                g_coverage.covered_lines = g_coverage.files[i].covered_lines;
            }
            break;
        }
    }
}

static void coverage_print_file(const CoverageFile* file) {
    double line_pct = file->total_lines > 0 ? (file->covered_lines * 100.0) / file->total_lines : 100.0;
    double func_pct = file->total_functions > 0 ? (file->covered_functions * 100.0) / file->total_functions : 100.0;
    double branch_pct = file->total_branches > 0 ? (file->covered_branches * 100.0) / file->total_branches : 100.0;

    const char* status = line_pct >= 80 ? "[OK]" : (line_pct >= 60 ? "[WARN]" : "[FAIL]");

    printf("  %s %-30s  Lines: %5.1f%%  Funcs: %5.1f%%  Branches: %5.1f%%\n",
           status,
           file->filename,
           line_pct,
           func_pct,
           branch_pct);
}

static void coverage_print_summary(void) {
    printf("\n");
    printf("Coverage Report\n");
    printf("===============\n");
    printf("\n");

    for (int i = 0; i < g_coverage.file_count; i++) {
        coverage_print_file(&g_coverage.files[i]);
    }

    printf("\n");
    printf("Overall Coverage\n");
    printf("---------------\n");

    double line_pct = g_coverage.total_lines > 0 ? (g_coverage.covered_lines * 100.0) / g_coverage.total_lines : 0.0;
    double func_pct = g_coverage.total_functions > 0 ? (g_coverage.covered_functions * 100.0) / g_coverage.total_functions : 0.0;
    double branch_pct = g_coverage.total_branches > 0 ? (g_coverage.covered_branches * 100.0) / g_coverage.total_branches : 0.0;

    const char* status = line_pct >= 80 ? "[OK]" : (line_pct >= 60 ? "[WARN]" : "[FAIL]");

    printf("  %s Lines:    %5.1f%% (%d/%d)\n", status, line_pct, g_coverage.covered_lines, g_coverage.total_lines);
    printf("     Functions: %5.1f%% (%d/%d)\n", func_pct, g_coverage.covered_functions, g_coverage.total_functions);
    printf("     Branches:  %5.1f%% (%d/%d)\n", branch_pct, g_coverage.covered_branches, g_coverage.total_branches);
    printf("\n");

    printf("Legend:\n");
    printf("  [OK]   Coverage >= 80%%\n");
    printf("  [WARN] Coverage 60-79%%\n");
    printf("  [FAIL] Coverage < 60%%\n");
    printf("\n");
}

static void coverage_print_csv(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        return;
    }

    fprintf(fp, "Filename,Lines,LinesCovered,LinePct,Functions,FunctionsCovered,FuncPct,Branches,BranchesCovered,BranchPct\n");

    for (int i = 0; i < g_coverage.file_count; i++) {
        const CoverageFile* file = &g_coverage.files[i];
        double line_pct = file->total_lines > 0 ? (file->covered_lines * 100.0) / file->total_lines : 0.0;
        double func_pct = file->total_functions > 0 ? (file->covered_functions * 100.0) / file->total_functions : 0.0;
        double branch_pct = file->total_branches > 0 ? (file->covered_branches * 100.0) / file->total_branches : 0.0;

        fprintf(fp, "%s,%d,%d,%.2f,%d,%d,%.2f,%d,%d,%.2f\n",
                file->filename,
                file->total_lines,
                file->covered_lines,
                line_pct,
                file->total_functions,
                file->covered_functions,
                func_pct,
                file->total_branches,
                file->covered_branches,
                branch_pct);
    }

    fclose(fp);
}

static void coverage_print_json(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        return;
    }

    double line_pct = g_coverage.total_lines > 0 ? (g_coverage.covered_lines * 100.0) / g_coverage.total_lines : 0.0;
    double func_pct = g_coverage.total_functions > 0 ? (g_coverage.covered_functions * 100.0) / g_coverage.total_functions : 0.0;
    double branch_pct = g_coverage.total_branches > 0 ? (g_coverage.covered_branches * 100.0) / g_coverage.total_branches : 0.0;

    fprintf(fp, "{\n");
    fprintf(fp, "  \"total_lines\": %d,\n", g_coverage.total_lines);
    fprintf(fp, "  \"covered_lines\": %d,\n", g_coverage.covered_lines);
    fprintf(fp, "  \"line_percentage\": %.2f,\n", line_pct);
    fprintf(fp, "  \"total_functions\": %d,\n", g_coverage.total_functions);
    fprintf(fp, "  \"covered_functions\": %d,\n", g_coverage.covered_functions);
    fprintf(fp, "  \"function_percentage\": %.2f,\n", func_pct);
    fprintf(fp, "  \"total_branches\": %d,\n", g_coverage.total_branches);
    fprintf(fp, "  \"covered_branches\": %d,\n", g_coverage.covered_branches);
    fprintf(fp, "  \"branch_percentage\": %.2f,\n", branch_pct);
    fprintf(fp, "  \"files\": [\n");

    for (int i = 0; i < g_coverage.file_count; i++) {
        const CoverageFile* file = &g_coverage.files[i];
        double file_line_pct = file->total_lines > 0 ? (file->covered_lines * 100.0) / file->total_lines : 0.0;
        double file_func_pct = file->total_functions > 0 ? (file->covered_functions * 100.0) / file->total_functions : 0.0;
        double file_branch_pct = file->total_branches > 0 ? (file->covered_branches * 100.0) / file->total_branches : 0.0;

        fprintf(fp, "    {\n");
        fprintf(fp, "      \"filename\": \"%s\",\n", file->filename);
        fprintf(fp, "      \"total_lines\": %d,\n", file->total_lines);
        fprintf(fp, "      \"covered_lines\": %d,\n", file->covered_lines);
        fprintf(fp, "      \"line_percentage\": %.2f,\n", file_line_pct);
        fprintf(fp, "      \"total_functions\": %d,\n", file->total_functions);
        fprintf(fp, "      \"covered_functions\": %d,\n", file->covered_functions);
        fprintf(fp, "      \"function_percentage\": %.2f,\n", file_func_pct);
        fprintf(fp, "      \"total_branches\": %d,\n", file->total_branches);
        fprintf(fp, "      \"covered_branches\": %d,\n", file->covered_branches);
        fprintf(fp, "      \"branch_percentage\": %.2f\n", file_branch_pct);
        fprintf(fp, "    }%s\n", i < g_coverage.file_count - 1 ? "," : "");
    }

    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");

    fclose(fp);
}

#endif // COVERAGE_FRAMEWORK_H
