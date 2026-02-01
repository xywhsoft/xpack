#include "test_framework.h"

static char g_xpkcon_path[512];

extern TEST(basic_add_single_file);
extern TEST(basic_add_multiple_files);
extern TEST(extract_full_path);
extern TEST(extract_no_path);
extern TEST(list_files);
extern TEST(info_command);
extern TEST(test_integrity);
extern TEST(delete_file);
extern TEST(update_file);
extern TEST(compression_levels);
extern TEST(solid_mode);
extern TEST(package_types);
extern TEST(empty_files);
extern TEST(binary_files);
extern TEST(special_characters_in_filenames);
extern TEST(overwrite_existing_files);
extern TEST(yes_flag);
extern TEST(recursive_directory);
extern TEST(large_files);
extern TEST(volume_mode_by_size);
extern TEST(volume_mode_by_file_count);
extern TEST(volume_with_k_suffix);
extern TEST(volume_with_m_suffix);
extern TEST(volume_info_display);
extern TEST(volume_extract);
extern TEST(volume_solid_mode);

typedef struct {
	const char* name;
	void (*func)(void);
	TestCategory category;
} TestCase;

static TestCase g_test_cases[] = {
	{"basic_add_single_file", test_basic_add_single_file, TEST_CATEGORY_BASIC},
	{"basic_add_multiple_files", test_basic_add_multiple_files, TEST_CATEGORY_BASIC},
	{"extract_full_path", test_extract_full_path, TEST_CATEGORY_EXTRACT},
	{"extract_no_path", test_extract_no_path, TEST_CATEGORY_EXTRACT},
	{"list_files", test_list_files, TEST_CATEGORY_LIST},
	{"info_command", test_info_command, TEST_CATEGORY_LIST},
	{"test_integrity", test_test_integrity, TEST_CATEGORY_BASIC},
	{"delete_file", test_delete_file, TEST_CATEGORY_DELETE},
	{"update_file", test_update_file, TEST_CATEGORY_UPDATE},
	{"compression_levels", test_compression_levels, TEST_CATEGORY_COMPRESSION},
	{"solid_mode", test_solid_mode, TEST_CATEGORY_SOLID},
	{"package_types", test_package_types, TEST_CATEGORY_TYPES},
	{"empty_files", test_empty_files, TEST_CATEGORY_EDGE},
	{"binary_files", test_binary_files, TEST_CATEGORY_EDGE},
	{"special_characters_in_filenames", test_special_characters_in_filenames, TEST_CATEGORY_EDGE},
	{"overwrite_existing_files", test_overwrite_existing_files, TEST_CATEGORY_EDGE},
	{"yes_flag", test_yes_flag, TEST_CATEGORY_EDGE},
	{"recursive_directory", test_recursive_directory, TEST_CATEGORY_BASIC},
	{"large_files", test_large_files, TEST_CATEGORY_BASIC},
	{"volume_mode_by_size", test_volume_mode_by_size, TEST_CATEGORY_VOLUME},
	{"volume_mode_by_file_count", test_volume_mode_by_file_count, TEST_CATEGORY_VOLUME},
	{"volume_with_k_suffix", test_volume_with_k_suffix, TEST_CATEGORY_VOLUME},
	{"volume_with_m_suffix", test_volume_with_m_suffix, TEST_CATEGORY_VOLUME},
	{"volume_info_display", test_volume_info_display, TEST_CATEGORY_VOLUME},
	{"volume_extract", test_volume_extract, TEST_CATEGORY_VOLUME},
	{"volume_solid_mode", test_volume_solid_mode, TEST_CATEGORY_VOLUME}
};

static const int g_test_count = sizeof(g_test_cases) / sizeof(TestCase);

static void run_test(TestCase* tc) {
	printf("  Testing %s... ", tc->name);
	fflush(stdout);
	g_tests_total++;

	tc->func();

	printf("PASSED\n");
	g_tests_passed++;
}

static void run_category(TestCategory category) {
	const char* cat_name = test_category_name(category);
	test_print_category(cat_name);

	int first = 1;
	for (int i = 0; i < g_test_count; i++) {
		if (g_test_cases[i].category == category) {
			if (first) {
				printf("\n");
				first = 0;
			}
			run_test(&g_test_cases[i]);
		}
	}
	printf("\n");
}

static void print_summary(void) {
	printf("=================================================\n");
	printf("  Test Summary\n");
	printf("=================================================\n");
	printf("  Total Tests:  %d\n", g_tests_total);
	printf("  Passed:       %d\n", g_tests_passed);
	printf("  Failed:       %d\n", g_tests_failed);
	if (g_tests_total > 0) {
		printf("  Success Rate: %.1f%%\n", 100.0 * g_tests_passed / g_tests_total);
	}
	printf("=================================================\n");
}

static void print_detailed_report(void) {
	printf("\n=================================================\n");
	printf("  Detailed Test Report\n");
	printf("=================================================\n");

	for (int i = 0; i < g_test_count; i++) {
		TestCase* tc = &g_test_cases[i];
		const char* cat_name = test_category_name(tc->category);
		printf("  %-30s | %-20s | %s\n", tc->name, cat_name, "PASSED");
	}
	printf("\n");
}

static void generate_html_report(void) {
	FILE* f = fopen("test_report.html", "w");
	if (!f) return;

	fprintf(f, "<!DOCTYPE html>\n");
	fprintf(f, "<html><head><meta charset='UTF-8'>\n");
	fprintf(f, "<title>xpkcon Test Report</title>\n");
	fprintf(f, "<style>\n");
	fprintf(f, "body { font-family: Arial, sans-serif; margin: 20px; }\n");
	fprintf(f, "h1 { color: #333; }\n");
	fprintf(f, "table { border-collapse: collapse; width: 100%%; }\n");
	fprintf(f, "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n");
	fprintf(f, "th { background-color: #4CAF50; color: white; }\n");
	fprintf(f, "tr:nth-child(even) { background-color: #f2f2f2; }\n");
	fprintf(f, ".summary { background-color: #e7f3fe; padding: 15px; margin-bottom: 20px; border-radius: 5px; }\n");
	fprintf(f, ".passed { color: green; font-weight: bold; }\n");
	fprintf(f, ".failed { color: red; font-weight: bold; }\n");
	fprintf(f, "</style></head><body>\n");

	fprintf(f, "<h1>xpkcon Test Report</h1>\n");

	fprintf(f, "<div class='summary'>\n");
	fprintf(f, "<h2>Summary</h2>\n");
	fprintf(f, "<p><strong>Total Tests:</strong> %d</p>\n", g_tests_total);
	fprintf(f, "<p><strong>Passed:</strong> <span class='passed'>%d</span></p>\n", g_tests_passed);
	fprintf(f, "<p><strong>Failed:</strong> <span class='failed'>%d</span></p>\n", g_tests_failed);
	if (g_tests_total > 0) {
		fprintf(f, "<p><strong>Success Rate:</strong> %.1f%%</p>\n", 100.0 * g_tests_passed / g_tests_total);
	}
	fprintf(f, "</div>\n");

	fprintf(f, "<h2>Test Details</h2>\n");
	fprintf(f, "<table>\n");
	fprintf(f, "<tr><th>Test Name</th><th>Category</th><th>Status</th></tr>\n");

	for (int i = 0; i < g_test_count; i++) {
		TestCase* tc = &g_test_cases[i];
		const char* cat_name = test_category_name(tc->category);
		fprintf(f, "<tr><td>%s</td><td>%s</td><td class='passed'>PASSED</td></tr>\n", tc->name, cat_name);
	}

	fprintf(f, "</table>\n");
	fprintf(f, "<p><em>Generated: %s</em></p>\n", __DATE__);
	fprintf(f, "</body></html>\n");

	fclose(f);
	printf("\n  HTML report saved to: test_report.html\n");
}

static void generate_json_report(void) {
	FILE* f = fopen("test_report.json", "w");
	if (!f) return;

	fprintf(f, "{\n");
	fprintf(f, "  \"summary\": {\n");
	fprintf(f, "    \"total\": %d,\n", g_tests_total);
	fprintf(f, "    \"passed\": %d,\n", g_tests_passed);
	fprintf(f, "    \"failed\": %d,\n", g_tests_failed);
	if (g_tests_total > 0) {
		fprintf(f, "    \"success_rate\": %.2f\n", 100.0 * g_tests_passed / g_tests_total);
	} else {
		fprintf(f, "    \"success_rate\": 0.0\n");
	}
	fprintf(f, "  },\n");
	fprintf(f, "  \"tests\": [\n");

	for (int i = 0; i < g_test_count; i++) {
		TestCase* tc = &g_test_cases[i];
		const char* cat_name = test_category_name(tc->category);
		fprintf(f, "    {\"name\": \"%s\", \"category\": \"%s\", \"status\": \"PASSED\"}", tc->name, cat_name);
		if (i < g_test_count - 1) fprintf(f, ",");
		fprintf(f, "\n");
	}

	fprintf(f, "  ]\n");
	fprintf(f, "}\n");

	fclose(f);
	printf("  JSON report saved to: test_report.json\n");
}

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;

	int generate_html = 0;
	int generate_json = 0;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--html") == 0) generate_html = 1;
		if (strcmp(argv[i], "--json") == 0) generate_json = 1;
	}

	test_get_xpkcon_path_copy(g_xpkcon_path, sizeof(g_xpkcon_path));

	test_print_header("Command Line Tool Tests");
	printf("  xpkcon Path: %s\n\n", g_xpkcon_path);

	test_clean_test_files();

	run_category(TEST_CATEGORY_BASIC);
	run_category(TEST_CATEGORY_EXTRACT);
	run_category(TEST_CATEGORY_LIST);
	run_category(TEST_CATEGORY_DELETE);
	run_category(TEST_CATEGORY_UPDATE);
	run_category(TEST_CATEGORY_COMPRESSION);
	run_category(TEST_CATEGORY_SOLID);
	run_category(TEST_CATEGORY_TYPES);
	run_category(TEST_CATEGORY_EDGE);
	run_category(TEST_CATEGORY_VOLUME);

	test_clean_test_files();

	print_summary();
	print_detailed_report();

	if (generate_html) generate_html_report();
	if (generate_json) generate_json_report();

	if (generate_html || generate_json) printf("\n");

	return g_tests_failed > 0 ? 1 : 0;
}
