#include "test_framework.h"

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

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;

	char xpkcon_path[512];
	test_get_xpkcon_path_copy(xpkcon_path, sizeof(xpkcon_path));
	test_set_xpkcon_path(xpkcon_path);

	test_print_header("Command Line Tool Tests");

	test_clean_test_files();

	test_print_category("Basic Operations");
	RUN_TEST(basic_add_single_file);
	RUN_TEST(basic_add_multiple_files);

	test_print_category("Extract Tests");
	RUN_TEST(extract_full_path);
	RUN_TEST(extract_no_path);

	test_print_category("List and Info Tests");
	RUN_TEST(list_files);
	RUN_TEST(info_command);

	test_print_category("Test Integrity");
	RUN_TEST(test_integrity);

	test_print_category("Delete and Update Tests");
	RUN_TEST(delete_file);
	RUN_TEST(update_file);

	test_print_category("Compression Tests");
	RUN_TEST(compression_levels);

	test_print_category("Solid Mode Tests");
	RUN_TEST(solid_mode);

	test_print_category("Package Type Tests");
	RUN_TEST(package_types);

	test_print_category("Edge Cases");
	RUN_TEST(empty_files);
	RUN_TEST(binary_files);
	RUN_TEST(special_characters_in_filenames);
	RUN_TEST(overwrite_existing_files);
	RUN_TEST(yes_flag);

	test_print_category("Advanced Tests");
	RUN_TEST(recursive_directory);
	RUN_TEST(large_files);

	test_print_category("Volume Compression Tests");
	RUN_TEST(volume_mode_by_size);
	RUN_TEST(volume_mode_by_file_count);
	RUN_TEST(volume_with_k_suffix);
	RUN_TEST(volume_with_m_suffix);
	RUN_TEST(volume_info_display);
	RUN_TEST(volume_extract);
	RUN_TEST(volume_solid_mode);

	test_clean_test_files();

	test_print_footer();

	return g_tests_failed > 0 ? 1 : 0;
}
