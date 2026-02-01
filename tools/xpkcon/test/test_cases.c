#include "test_framework.h"

TEST(basic_add_single_file) {
	ASSERT_EQ(test_create_test_file("test_input1.txt", "Hello, World!"), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_basic.xpk test_input1.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_basic.xpk"));
	ASSERT(test_file_exists("test_input1.txt"));

	test_run_command("del test_input1.txt 2>nul");
	test_run_command("del test_basic.xpk 2>nul");
}

TEST(basic_add_multiple_files) {
	ASSERT_EQ(test_create_test_file("test_file1.txt", "Content 1"), 0);
	ASSERT_EQ(test_create_test_file("test_file2.txt", "Content 2"), 0);
	ASSERT_EQ(test_create_test_file("test_file3.txt", "Content 3"), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_multi.xpk test_file1.txt test_file2.txt test_file3.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_multi.xpk"));

	test_run_command("del test_*.txt 2>nul");
	test_run_command("del test_multi.xpk 2>nul");
}

TEST(extract_full_path) {
	ASSERT_EQ(test_create_test_file("orig_file1.txt", "Data 1"), 0);
	ASSERT_EQ(test_create_test_file("orig_file2.txt", "Data 2"), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_extract.xpk orig_file1.txt orig_file2.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT_EQ(test_dir_create("test_output"), 0);

	snprintf(cmd, sizeof(cmd), "\"%s\" x test_extract.xpk -o test_output", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_output/orig_file1.txt"));
	ASSERT(test_file_exists("test_output/orig_file2.txt"));

	ASSERT_EQ(test_file_compare("orig_file1.txt", "test_output/orig_file1.txt"), 0);
	ASSERT_EQ(test_file_compare("orig_file2.txt", "test_output/orig_file2.txt"), 0);

	test_run_command("del orig_*.txt 2>nul");
	test_run_command("del test_extract.xpk 2>nul");
	test_dir_remove("test_output");
}

TEST(extract_no_path) {
	ASSERT_EQ(test_create_test_file("flat_file1.txt", "Flat 1"), 0);
	ASSERT_EQ(test_create_test_file("flat_file2.txt", "Flat 2"), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_flat.xpk flat_file1.txt flat_file2.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT_EQ(test_dir_create("test_flat_out"), 0);

	snprintf(cmd, sizeof(cmd), "\"%s\" e test_flat.xpk -o test_flat_out", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_flat_out/flat_file1.txt"));
	ASSERT(test_file_exists("test_flat_out/flat_file2.txt"));

	ASSERT_EQ(test_file_compare("flat_file1.txt", "test_flat_out/flat_file1.txt"), 0);
	ASSERT_EQ(test_file_compare("flat_file2.txt", "test_flat_out/flat_file2.txt"), 0);

	test_run_command("del flat_*.txt 2>nul");
	test_run_command("del test_flat.xpk 2>nul");
	test_dir_remove("test_flat_out");
}

TEST(list_files) {
	ASSERT_EQ(test_create_test_file("list_test1.txt", "List 1"), 0);
	ASSERT_EQ(test_create_test_file("list_test2.txt", "List 2"), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_list.xpk list_test1.txt list_test2.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	snprintf(cmd, sizeof(cmd), "\"%s\" l test_list.xpk > list_output.txt 2>&1", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("list_output.txt"));

	test_run_command("del list_test*.txt 2>nul");
	test_run_command("del test_list.xpk 2>nul");
	test_run_command("del list_output.txt 2>nul");
}

TEST(test_integrity) {
	ASSERT_EQ(test_create_test_file("test_verify1.txt", "Verify content 1"), 0);
	ASSERT_EQ(test_create_test_file("test_verify2.txt", "Verify content 2"), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_test.xpk test_verify1.txt test_verify2.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	snprintf(cmd, sizeof(cmd), "\"%s\" t test_test.xpk", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	test_run_command("del test_verify*.txt 2>nul");
	test_run_command("del test_test.xpk 2>nul");
}

TEST(delete_file) {
	ASSERT_EQ(test_create_test_file("del_file1.txt", "Delete 1"), 0);
	ASSERT_EQ(test_create_test_file("del_file2.txt", "Delete 2"), 0);
	ASSERT_EQ(test_create_test_file("del_file3.txt", "Delete 3"), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_del.xpk del_file1.txt del_file2.txt del_file3.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	snprintf(cmd, sizeof(cmd), "\"%s\" d test_del.xpk -y del_file2.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_del.xpk"));

	ASSERT_EQ(test_dir_create("test_del_out"), 0);
	snprintf(cmd, sizeof(cmd), "\"%s\" x test_del.xpk -o test_del_out", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_del_out/del_file1.txt"));
	ASSERT(!test_file_exists("test_del_out/del_file2.txt"));
	ASSERT(test_file_exists("test_del_out/del_file3.txt"));

	test_run_command("del del_*.txt 2>nul");
	test_run_command("del test_del.xpk 2>nul");
	test_dir_remove("test_del_out");
}

TEST(update_file) {
	ASSERT_EQ(test_create_test_file("upd_orig1.txt", "Original 1"), 0);
	ASSERT_EQ(test_create_test_file("upd_orig2.txt", "Original 2"), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_update.xpk upd_orig1.txt upd_orig2.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT_EQ(test_create_test_file("upd_orig1.txt", "Updated 1"), 0);

	snprintf(cmd, sizeof(cmd), "\"%s\" u test_update.xpk -l7 upd_orig1.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT_EQ(test_dir_create("test_upd_out"), 0);
	snprintf(cmd, sizeof(cmd), "\"%s\" x test_update.xpk -o test_upd_out", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_upd_out/upd_orig1.txt"));

	ASSERT_NE(test_file_compare("upd_orig1.txt", "test_upd_out/upd_orig1.txt"), 0);

	test_run_command("del upd_*.txt 2>nul");
	test_run_command("del test_update.xpk 2>nul");
	test_dir_remove("test_upd_out");
}

TEST(info_command) {
	ASSERT_EQ(test_create_test_file("info_test1.txt", "Info 1"), 0);
	ASSERT_EQ(test_create_test_file("info_test2.txt", "Info 2"), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_info.xpk info_test1.txt info_test2.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	snprintf(cmd, sizeof(cmd), "\"%s\" i test_info.xpk > info_output.txt 2>&1", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("info_output.txt"));

	test_run_command("del info_test*.txt 2>nul");
	test_run_command("del test_info.xpk 2>nul");
	test_run_command("del info_output.txt 2>nul");
}

TEST(compression_levels) {
	uint8_t testData[2048];
	for (int i = 0; i < 2048; i++) testData[i] = (uint8_t)(i % 256);
	ASSERT_EQ(test_create_test_binary_file("comp_test.bin", testData, 2048), 0);

	char cmd[1024];
	for (int level = 0; level <= 15; level++) {
		snprintf(cmd, sizeof(cmd), "\"%s\" a test_comp_%d.xpk -l%d comp_test.bin", test_get_xpkcon_path(), level, level);
		ASSERT_EQ(test_run_command(cmd), 0);
		ASSERT(test_file_exists("test_comp_0.xpk"));

		snprintf(cmd, sizeof(cmd), "\"%s\" t test_comp_%d.xpk", test_get_xpkcon_path(), level);
		ASSERT_EQ(test_run_command(cmd), 0);

		ASSERT_EQ(test_dir_create("test_comp_out"), 0);
		snprintf(cmd, sizeof(cmd), "\"%s\" x test_comp_%d.xpk -o test_comp_out", test_get_xpkcon_path(), level);
		ASSERT_EQ(test_run_command(cmd), 0);

		ASSERT_EQ(test_file_compare("comp_test.bin", "test_comp_out/comp_test.bin"), 0);

		snprintf(cmd, sizeof(cmd), "\"%s\" del test_comp_%d.xpk 2>nul", test_get_xpkcon_path(), level);
		test_run_command(cmd);
		test_dir_remove("test_comp_out");
	}

	test_run_command("del comp_test.bin 2>nul");
}

TEST(solid_mode) {
	ASSERT_EQ(test_create_test_file("solid_file1.txt", "Repeated data pattern AAA"), 0);
	ASSERT_EQ(test_create_test_file("solid_file2.txt", "Repeated data pattern AAA"), 0);
	ASSERT_EQ(test_create_test_file("solid_file3.txt", "Repeated data pattern AAA"), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_solid.xpk -s1 solid_file1.txt solid_file2.txt solid_file3.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_solid.xpk"));

	ASSERT_EQ(test_dir_create("test_solid_out"), 0);
	snprintf(cmd, sizeof(cmd), "\"%s\" x test_solid.xpk -o test_solid_out", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_solid_out/solid_file1.txt"));
	ASSERT(test_file_exists("test_solid_out/solid_file2.txt"));
	ASSERT(test_file_exists("test_solid_out/solid_file3.txt"));

	ASSERT_EQ(test_file_compare("solid_file1.txt", "test_solid_out/solid_file1.txt"), 0);
	ASSERT_EQ(test_file_compare("solid_file2.txt", "test_solid_out/solid_file2.txt"), 0);
	ASSERT_EQ(test_file_compare("solid_file3.txt", "test_solid_out/solid_file3.txt"), 0);

	test_run_command("del solid_*.txt 2>nul");
	test_run_command("del test_solid.xpk 2>nul");
	test_dir_remove("test_solid_out");
}

TEST(package_types) {
	ASSERT_EQ(test_create_test_file("type_test.txt", "Type test content"), 0);

	const char* types[] = {"core", "index", "linux", "win32"};
	char cmd[1024];

	for (int i = 0; i < 4; i++) {
		snprintf(cmd, sizeof(cmd), "\"%s\" a test_type_%s.xpk -t%s type_test.txt", test_get_xpkcon_path(), types[i], types[i]);
		ASSERT_EQ(test_run_command(cmd), 0);
		ASSERT(test_file_exists("test_type_core.xpk"));

		snprintf(cmd, sizeof(cmd), "\"%s\" t test_type_%s.xpk", test_get_xpkcon_path(), types[i]);
		ASSERT_EQ(test_run_command(cmd), 0);

		snprintf(cmd, sizeof(cmd), "\"%s\" del test_type_%s.xpk 2>nul", test_get_xpkcon_path(), types[i]);
		test_run_command(cmd);
	}

	test_run_command("del type_test.txt 2>nul");
}

TEST(empty_files) {
	ASSERT_EQ(test_create_test_file("empty1.txt", ""), 0);
	ASSERT_EQ(test_create_test_file("empty2.txt", ""), 0);
	ASSERT_EQ(test_create_test_file("nonempty.txt", "Some content"), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_empty.xpk empty1.txt empty2.txt nonempty.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_empty.xpk"));

	ASSERT_EQ(test_dir_create("test_empty_out"), 0);
	snprintf(cmd, sizeof(cmd), "\"%s\" x test_empty.xpk -o test_empty_out", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_empty_out/empty1.txt"));
	ASSERT(test_file_exists("test_empty_out/empty2.txt"));
	ASSERT(test_file_exists("test_empty_out/nonempty.txt"));

	long size1, size2;
	ASSERT_EQ(test_file_size("test_empty_out/empty1.txt", &size1), 0);
	ASSERT_EQ(size1, 0);
	ASSERT_EQ(test_file_size("test_empty_out/empty2.txt", &size2), 0);
	ASSERT_EQ(size2, 0);

	ASSERT_EQ(test_file_compare("nonempty.txt", "test_empty_out/nonempty.txt"), 0);

	test_run_command("del empty*.txt 2>nul");
	test_run_command("del nonempty.txt 2>nul");
	test_run_command("del test_empty.xpk 2>nul");
	test_dir_remove("test_empty_out");
}

TEST(binary_files) {
	uint8_t binaryData[512];
	for (int i = 0; i < 512; i++) binaryData[i] = (uint8_t)i;
	ASSERT_EQ(test_create_test_binary_file("binary_test.dat", binaryData, 512), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_binary.xpk binary_test.dat", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT_EQ(test_dir_create("test_binary_out"), 0);
	snprintf(cmd, sizeof(cmd), "\"%s\" x test_binary.xpk -o test_binary_out", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_binary_out/binary_test.dat"));
	ASSERT_EQ(test_file_compare("binary_test.dat", "test_binary_out/binary_test.dat"), 0);

	test_run_command("del binary_test.dat 2>nul");
	test_run_command("del test_binary.xpk 2>nul");
	test_dir_remove("test_binary_out");
}

TEST(recursive_directory) {
	ASSERT_EQ(test_dir_create("test_recur_src"), 0);
	ASSERT_EQ(test_dir_create("test_recur_src/subdir1"), 0);
	ASSERT_EQ(test_dir_create("test_recur_src/subdir1/deep"), 0);
	ASSERT_EQ(test_dir_create("test_recur_src/subdir2"), 0);

	ASSERT_EQ(test_create_test_file("test_recur_src/file1.txt", "Root file 1"), 0);
	ASSERT_EQ(test_create_test_file("test_recur_src/file2.txt", "Root file 2"), 0);
	ASSERT_EQ(test_create_test_file("test_recur_src/subdir1/sub1.txt", "Subdir 1 file"), 0);
	ASSERT_EQ(test_create_test_file("test_recur_src/subdir1/deep/deep.txt", "Deep file"), 0);
	ASSERT_EQ(test_create_test_file("test_recur_src/subdir2/sub2.txt", "Subdir 2 file"), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_recur.xpk -r test_recur_src", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_recur.xpk"));

	ASSERT_EQ(test_dir_create("test_recur_out"), 0);
	snprintf(cmd, sizeof(cmd), "\"%s\" x test_recur.xpk -o test_recur_out", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_recur_out/test_recur_src/file1.txt"));
	ASSERT(test_file_exists("test_recur_out/test_recur_src/subdir1/sub1.txt"));
	ASSERT(test_file_exists("test_recur_out/test_recur_src/subdir1/deep/deep.txt"));

	ASSERT_EQ(test_file_compare("test_recur_src/file1.txt", "test_recur_out/test_recur_src/file1.txt"), 0);
	ASSERT_EQ(test_file_compare("test_recur_src/subdir1/deep/deep.txt", "test_recur_out/test_recur_src/subdir1/deep/deep.txt"), 0);

	test_dir_remove("test_recur_src");
	test_run_command("del test_recur.xpk 2>nul");
	test_dir_remove("test_recur_out");
}

TEST(large_files) {
	const size_t fileSize = 1024 * 1024;
	uint8_t* largeData = (uint8_t*)malloc(fileSize);
	for (size_t i = 0; i < fileSize; i++) largeData[i] = (uint8_t)(i % 256);

	ASSERT_EQ(test_create_test_binary_file("large_test.dat", largeData, fileSize), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_large.xpk large_test.dat", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT_EQ(test_dir_create("test_large_out"), 0);
	snprintf(cmd, sizeof(cmd), "\"%s\" x test_large.xpk -o test_large_out", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_large_out/large_test.dat"));
	ASSERT_EQ(test_file_compare("large_test.dat", "test_large_out/large_test.dat"), 0);

	free(largeData);
	test_run_command("del large_test.dat 2>nul");
	test_run_command("del test_large.xpk 2>nul");
	test_dir_remove("test_large_out");
}

TEST(special_characters_in_filenames) {
	ASSERT_EQ(test_create_test_file("file with spaces.txt", "Spaces test"), 0);
	ASSERT_EQ(test_create_test_file("file-with-dashes.txt", "Dashes test"), 0);
	ASSERT_EQ(test_create_test_file("file_with_underscores.txt", "Underscores test"), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_special.xpk \"file with spaces.txt\" file-with-dashes.txt file_with_underscores.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT_EQ(test_dir_create("test_special_out"), 0);
	snprintf(cmd, sizeof(cmd), "\"%s\" x test_special.xpk -o test_special_out", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_special_out/file with spaces.txt"));
	ASSERT(test_file_exists("test_special_out/file-with-dashes.txt"));
	ASSERT(test_file_exists("test_special_out/file_with_underscores.txt"));

	ASSERT_EQ(test_file_compare("file with spaces.txt", "test_special_out/file with spaces.txt"), 0);

	test_run_command("del \"file with spaces.txt\" 2>nul");
	test_run_command("del \"file-with-dashes.txt\" 2>nul");
	test_run_command("del \"file_with_underscores.txt\" 2>nul");
	test_run_command("del test_special.xpk 2>nul");
	test_dir_remove("test_special_out");
}

TEST(overwrite_existing_files) {
	ASSERT_EQ(test_create_test_file("overwrite_test.txt", "Original content"), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_overwrite.xpk overwrite_test.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT_EQ(test_create_test_file("overwrite_test.txt", "Modified content"), 0);

	ASSERT_EQ(test_dir_create("test_overwrite_out"), 0);
	snprintf(cmd, sizeof(cmd), "\"%s\" e test_overwrite.xpk -o test_overwrite_out -y", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_overwrite_out/overwrite_test.txt"));
	ASSERT_EQ(test_file_compare("test_overwrite_out/overwrite_test.txt", "overwrite_test.txt"), 0);

	test_run_command("del overwrite_test.txt 2>nul");
	test_run_command("del test_overwrite.xpk 2>nul");
	test_dir_remove("test_overwrite_out");
}

TEST(yes_flag) {
	ASSERT_EQ(test_create_test_file("yes_test.txt", "Yes flag test"), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_yes.xpk yes_test.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	snprintf(cmd, sizeof(cmd), "\"%s\" d test_yes.xpk -y yes_test.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT_EQ(test_dir_create("test_yes_out"), 0);
	snprintf(cmd, sizeof(cmd), "\"%s\" x test_yes.xpk -o test_yes_out", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(!test_file_exists("test_yes_out/yes_test.txt"));

	test_run_command("del yes_test.txt 2>nul");
	test_run_command("del test_yes.xpk 2>nul");
	test_dir_remove("test_yes_out");
}

TEST(volume_mode_by_size) {
	ASSERT_EQ(test_create_test_file("vol_file1.txt", "Volume test file 1 content"), 0);
	ASSERT_EQ(test_create_test_file("vol_file2.txt", "Volume test file 2 content"), 0);
	ASSERT_EQ(test_create_test_file("vol_file3.txt", "Volume test file 3 content"), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_vol_size.xpk -V10K vol_file1.txt vol_file2.txt vol_file3.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_vol_size.xpk"));

	snprintf(cmd, sizeof(cmd), "\"%s\" i test_vol_size.xpk > vol_info.txt 2>&1", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);
	ASSERT(test_file_exists("vol_info.txt"));

	ASSERT_EQ(test_dir_create("test_vol_out"), 0);
	snprintf(cmd, sizeof(cmd), "\"%s\" x test_vol_size.xpk -o test_vol_out", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_vol_out/vol_file1.txt"));
	ASSERT(test_file_exists("test_vol_out/vol_file2.txt"));
	ASSERT(test_file_exists("test_vol_out/vol_file3.txt"));

	ASSERT_EQ(test_file_compare("vol_file1.txt", "test_vol_out/vol_file1.txt"), 0);
	ASSERT_EQ(test_file_compare("vol_file2.txt", "test_vol_out/vol_file2.txt"), 0);
	ASSERT_EQ(test_file_compare("vol_file3.txt", "test_vol_out/vol_file3.txt"), 0);

	test_run_command("del vol_*.txt 2>nul");
	test_run_command("del test_vol_size.xpk 2>nul");
	test_run_command("del test_vol_size.* 2>nul");
	test_dir_remove("test_vol_out");
}

TEST(volume_mode_by_file_count) {
	ASSERT_EQ(test_create_test_file("vfc_file1.txt", "File count test 1"), 0);
	ASSERT_EQ(test_create_test_file("vfc_file2.txt", "File count test 2"), 0);
	ASSERT_EQ(test_create_test_file("vfc_file3.txt", "File count test 3"), 0);
	ASSERT_EQ(test_create_test_file("vfc_file4.txt", "File count test 4"), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_vfc.xpk -V2 --split-mode=file vfc_file1.txt vfc_file2.txt vfc_file3.txt vfc_file4.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_vfc.xpk"));

	ASSERT_EQ(test_dir_create("test_vfc_out"), 0);
	snprintf(cmd, sizeof(cmd), "\"%s\" x test_vfc.xpk -o test_vfc_out", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_vfc_out/vfc_file1.txt"));
	ASSERT(test_file_exists("test_vfc_out/vfc_file2.txt"));
	ASSERT(test_file_exists("test_vfc_out/vfc_file3.txt"));
	ASSERT(test_file_exists("test_vfc_out/vfc_file4.txt"));

	ASSERT_EQ(test_file_compare("vfc_file1.txt", "test_vfc_out/vfc_file1.txt"), 0);
	ASSERT_EQ(test_file_compare("vfc_file2.txt", "test_vfc_out/vfc_file2.txt"), 0);
	ASSERT_EQ(test_file_compare("vfc_file3.txt", "test_vfc_out/vfc_file3.txt"), 0);
	ASSERT_EQ(test_file_compare("vfc_file4.txt", "test_vfc_out/vfc_file4.txt"), 0);

	test_run_command("del vfc_*.txt 2>nul");
	test_run_command("del test_vfc.xpk 2>nul");
	test_run_command("del test_vfc.* 2>nul");
	test_dir_remove("test_vfc_out");
}

TEST(volume_with_k_suffix) {
	ASSERT_EQ(test_create_test_file("k_test_file.txt", "K suffix test content here"), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_vol_k.xpk -V5K k_test_file.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_vol_k.xpk"));

	ASSERT_EQ(test_dir_create("test_vol_k_out"), 0);
	snprintf(cmd, sizeof(cmd), "\"%s\" x test_vol_k.xpk -o test_vol_k_out", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_vol_k_out/k_test_file.txt"));
	ASSERT_EQ(test_file_compare("k_test_file.txt", "test_vol_k_out/k_test_file.txt"), 0);

	test_run_command("del k_test_file.txt 2>nul");
	test_run_command("del test_vol_k.xpk 2>nul");
	test_run_command("del test_vol_k.* 2>nul");
	test_dir_remove("test_vol_k_out");
}

TEST(volume_with_m_suffix) {
	ASSERT_EQ(test_create_test_file("m_test_file.txt", "M suffix test content"), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_vol_m.xpk -V1M m_test_file.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_vol_m.xpk"));

	ASSERT_EQ(test_dir_create("test_vol_m_out"), 0);
	snprintf(cmd, sizeof(cmd), "\"%s\" x test_vol_m.xpk -o test_vol_m_out", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_vol_m_out/m_test_file.txt"));
	ASSERT_EQ(test_file_compare("m_test_file.txt", "test_vol_m_out/m_test_file.txt"), 0);

	test_run_command("del m_test_file.txt 2>nul");
	test_run_command("del test_vol_m.xpk 2>nul");
	test_run_command("del test_vol_m.* 2>nul");
	test_dir_remove("test_vol_m_out");
}

TEST(volume_info_display) {
	ASSERT_EQ(test_create_test_file("vinfo_file1.txt", "Info test 1"), 0);
	ASSERT_EQ(test_create_test_file("vinfo_file2.txt", "Info test 2"), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_vinfo.xpk -V10K vinfo_file1.txt vinfo_file2.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_vinfo.xpk"));

	snprintf(cmd, sizeof(cmd), "\"%s\" i test_vinfo.xpk > vinfo_output.txt 2>&1", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);
	ASSERT(test_file_exists("vinfo_output.txt"));

	ASSERT_EQ(test_dir_create("test_vinfo_out"), 0);
	snprintf(cmd, sizeof(cmd), "\"%s\" x test_vinfo.xpk -o test_vinfo_out", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_vinfo_out/vinfo_file1.txt"));
	ASSERT(test_file_exists("test_vinfo_out/vinfo_file2.txt"));

	ASSERT_EQ(test_file_compare("vinfo_file1.txt", "test_vinfo_out/vinfo_file1.txt"), 0);
	ASSERT_EQ(test_file_compare("vinfo_file2.txt", "test_vinfo_out/vinfo_file2.txt"), 0);

	test_run_command("del vinfo_*.txt 2>nul");
	test_run_command("del test_vinfo.xpk 2>nul");
	test_run_command("del test_vinfo.* 2>nul");
	test_run_command("del vinfo_output.txt 2>nul");
	test_dir_remove("test_vinfo_out");
}

TEST(volume_extract) {
	ASSERT_EQ(test_create_test_file("vext_file1.txt", "Extract volume test 1"), 0);
	ASSERT_EQ(test_create_test_file("vext_file2.txt", "Extract volume test 2"), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_vext.xpk -V5K vext_file1.txt vext_file2.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_vext.xpk"));

	ASSERT_EQ(test_dir_create("test_vext_out"), 0);
	snprintf(cmd, sizeof(cmd), "\"%s\" x test_vext.xpk -o test_vext_out", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_vext_out/vext_file1.txt"));
	ASSERT(test_file_exists("test_vext_out/vext_file2.txt"));

	ASSERT_EQ(test_file_compare("vext_file1.txt", "test_vext_out/vext_file1.txt"), 0);
	ASSERT_EQ(test_file_compare("vext_file2.txt", "test_vext_out/vext_file2.txt"), 0);

	test_run_command("del vext_*.txt 2>nul");
	test_run_command("del test_vext.xpk 2>nul");
	test_run_command("del test_vext.* 2>nul");
	test_dir_remove("test_vext_out");
}

TEST(volume_solid_mode) {
	ASSERT_EQ(test_create_test_file("vsol_file1.txt", "Solid volume test 1 - repeated data"), 0);
	ASSERT_EQ(test_create_test_file("vsol_file2.txt", "Solid volume test 2 - repeated data"), 0);
	ASSERT_EQ(test_create_test_file("vsol_file3.txt", "Solid volume test 3 - repeated data"), 0);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "\"%s\" a test_vsol.xpk -s1 -V10K vsol_file1.txt vsol_file2.txt vsol_file3.txt", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_vsol.xpk"));

	ASSERT_EQ(test_dir_create("test_vsol_out"), 0);
	snprintf(cmd, sizeof(cmd), "\"%s\" x test_vsol.xpk -o test_vsol_out", test_get_xpkcon_path());
	ASSERT_EQ(test_run_command(cmd), 0);

	ASSERT(test_file_exists("test_vsol_out/vsol_file1.txt"));
	ASSERT(test_file_exists("test_vsol_out/vsol_file2.txt"));
	ASSERT(test_file_exists("test_vsol_out/vsol_file3.txt"));

	ASSERT_EQ(test_file_compare("vsol_file1.txt", "test_vsol_out/vsol_file1.txt"), 0);
	ASSERT_EQ(test_file_compare("vsol_file2.txt", "test_vsol_out/vsol_file2.txt"), 0);
	ASSERT_EQ(test_file_compare("vsol_file3.txt", "test_vsol_out/vsol_file3.txt"), 0);

	test_run_command("del vsol_*.txt 2>nul");
	test_run_command("del test_vsol.xpk 2>nul");
	test_run_command("del test_vsol.* 2>nul");
	test_dir_remove("test_vsol_out");
}
