/*
 * xPack Ver7 - Cross Platform (25)
 */

#include "test_framework.h"

static char* createTestData_25(int size, char pattern) {
    char* pData = (char*)malloc(size);
    for (int i = 0; i < size; i++) {
        pData[i] = (char)(pattern + (i % 26));
    }
    return pData;
}

TEST(win32_path_case_insensitive) {
    char* pData = createTestData_25(1024, 'A');
    const char* sFilename = "test_25_win32.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_WIN32, 0);
    ASSERT_NOT_NULL(xpk);

    void* pResult = xpkPathAppendData(xpk, "test/file.txt", pData, 1024, 1);
    ASSERT_NOT_NULL(pResult);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_NE(xpkPathFind(xpk, "Test/File.TXT"), UINT32_MAX);
    ASSERT_NE(xpkPathFind(xpk, "TEST/FILE.TXT"), UINT32_MAX);

    free(pData);
    xpkClose(xpk);
}

TEST(linux_path_case_sensitive) {
    char* pData = createTestData_25(1024, 'B');
    const char* sFilename = "test_25_linux.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_LINUX, 0);
    ASSERT_NOT_NULL(xpk);

    void* pResult = xpkPathAppendData(xpk, "test/file.txt", pData, 1024, 1);
    ASSERT_NOT_NULL(pResult);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_NE(xpkPathFind(xpk, "test/file.txt"), UINT32_MAX);
    ASSERT_EQ(xpkPathFind(xpk, "Test/File.TXT"), UINT32_MAX);
    ASSERT_EQ(xpkPathFind(xpk, "TEST/FILE.TXT"), UINT32_MAX);

    free(pData);
    xpkClose(xpk);
}

TEST(win32_path_separator_handling) {
    char* pData = createTestData_25(1024, 'C');
    const char* sFilename = "test_25_separator.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_WIN32, 0);
    ASSERT_NOT_NULL(xpk);

    void* pResult = xpkPathAppendData(xpk, "folder\\subfolder\\file.txt", pData, 1024, 1);
    ASSERT_NOT_NULL(pResult);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_NE(xpkPathFind(xpk, "folder/subfolder/file.txt"), UINT32_MAX);

    free(pData);
    xpkClose(xpk);
}

TEST(linux_path_separator_handling) {
    char* pData = createTestData_25(1024, 'D');
    const char* sFilename = "test_25_linux_sep.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_LINUX, 0);
    ASSERT_NOT_NULL(xpk);

    void* pResult = xpkPathAppendData(xpk, "folder/subfolder/file.txt", pData, 1024, 1);
    ASSERT_NOT_NULL(pResult);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_NE(xpkPathFind(xpk, "folder/subfolder/file.txt"), UINT32_MAX);

    free(pData);
    xpkClose(xpk);
}

TEST(win32_and_linux_comparison) {
    char* pData1 = createTestData_25(1024, 'E');
    char* pData2 = createTestData_25(1024, 'F');
    const char* sWin32File = "test_25_win32_comp.xpk";
    const char* sLinuxFile = "test_25_linux_comp.xpk";

    xpkObject xpkWin32 = xpkOpen(sWin32File, XPK_TYPE_WIN32, 0);
    ASSERT_NOT_NULL(xpkWin32);

    void* pResult1 = xpkPathAppendData(xpkWin32, "Folder/File.TXT", pData1, 1024, 1);
    ASSERT_NOT_NULL(pResult1);
    ASSERT_EQ(xpkSave(xpkWin32), 0);

    xpkObject xpkLinux = xpkOpen(sLinuxFile, XPK_TYPE_LINUX, 0);
    ASSERT_NOT_NULL(xpkLinux);

    void* pResult2 = xpkPathAppendData(xpkLinux, "folder/file.txt", pData2, 1024, 1);
    ASSERT_NOT_NULL(pResult2);
    ASSERT_EQ(xpkSave(xpkLinux), 0);

    ASSERT_NE(xpkPathFind(xpkWin32, "folder/file.txt"), UINT32_MAX);
    ASSERT_NE(xpkPathFind(xpkLinux, "folder/file.txt"), UINT32_MAX);

    free(pData1);
    free(pData2);
    xpkClose(xpkWin32);
    xpkClose(xpkLinux);
}

TEST(absolute_path_handling) {
    char* pData = createTestData_25(1024, 'G');
    const char* sFilename = "test_25_abs_path.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_WIN32, 0);
    ASSERT_NOT_NULL(xpk);

    void* pResult = xpkPathAppendData(xpk, "C:\\Windows\\System32\\file.txt", pData, 1024, 1);
    ASSERT_NOT_NULL(pResult);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_NE(xpkPathFind(xpk, "C:\\Windows\\System32\\file.txt"), UINT32_MAX);

    free(pData);
    xpkClose(xpk);
}

TEST(relative_path_handling) {
    char* pData = createTestData_25(1024, 'H');
    const char* sFilename = "test_25_rel_path.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_LINUX, 0);
    ASSERT_NOT_NULL(xpk);

    void* pResult = xpkPathAppendData(xpk, "../data/file.txt", pData, 1024, 1);
    ASSERT_NOT_NULL(pResult);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_NE(xpkPathFind(xpk, "../data/file.txt"), UINT32_MAX);

    free(pData);
    xpkClose(xpk);
}

TEST(path_with_spaces) {
    char* pData = createTestData_25(1024, 'I');
    const char* sFilename = "test_25_spaces.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_LINUX, 0);
    ASSERT_NOT_NULL(xpk);

    void* pResult = xpkPathAppendData(xpk, "folder with spaces/file name.txt", pData, 1024, 1);
    ASSERT_NOT_NULL(pResult);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_NE(xpkPathFind(xpk, "folder with spaces/file name.txt"), UINT32_MAX);

    free(pData);
    xpkClose(xpk);
}

TEST(unicode_path_handling) {
    char* pData = createTestData_25(1024, 'J');
    const char* sFilename = "test_25_unicode.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_LINUX, 0);
    ASSERT_NOT_NULL(xpk);

    void* pResult = xpkPathAppendData(xpk, "文件/test.txt", pData, 1024, 1);
    ASSERT_NOT_NULL(pResult);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_NE(xpkPathFind(xpk, "文件/test.txt"), UINT32_MAX);

    free(pData);
    xpkClose(xpk);
}

TEST(nested_directory_structure) {
    char* pData = createTestData_25(512, 'K');
    const char* sFilename = "test_25_nested.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_LINUX, 0);
    ASSERT_NOT_NULL(xpk);

    void* pResult1 = xpkPathAppendData(xpk, "a/b/c/d/e/file.txt", pData, 512, 1);
    ASSERT_NOT_NULL(pResult1);
    void* pResult2 = xpkPathAppendData(xpk, "x/y/z/file2.txt", pData, 512, 1);
    ASSERT_NOT_NULL(pResult2);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_NE(xpkPathFind(xpk, "a/b/c/d/e/file.txt"), UINT32_MAX);
    ASSERT_NE(xpkPathFind(xpk, "x/y/z/file2.txt"), UINT32_MAX);

    free(pData);
    xpkClose(xpk);
}

TEST(file_extension_variations) {
    char* pData = createTestData_25(1024, 'L');
    const char* sFilename = "test_25_extensions.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_LINUX, 0);
    ASSERT_NOT_NULL(xpk);

    void* pResult1 = xpkPathAppendData(xpk, "file.txt", pData, 1024, 1);
    ASSERT_NOT_NULL(pResult1);
    void* pResult2 = xpkPathAppendData(xpk, "file.TXT", pData, 1024, 1);
    ASSERT_NOT_NULL(pResult2);
    void* pResult3 = xpkPathAppendData(xpk, "file.dat", pData, 1024, 1);
    ASSERT_NOT_NULL(pResult3);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_NE(xpkPathFind(xpk, "file.txt"), UINT32_MAX);
    ASSERT_NE(xpkPathFind(xpk, "file.TXT"), UINT32_MAX);
    ASSERT_NE(xpkPathFind(xpk, "file.dat"), UINT32_MAX);

    free(pData);
    xpkClose(xpk);
}

TEST(path_length_limits) {
    char* pData = createTestData_25(512, 'M');
    const char* sFilename = "test_25_path_len.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_LINUX, 0);
    ASSERT_NOT_NULL(xpk);

    char sLongPath[512];
    memset(sLongPath, 'a', 255);
    sLongPath[255] = '/';
    memset(sLongPath + 256, 'b', 255);
    sLongPath[511] = '\0';

    void* pResult = xpkPathAppendData(xpk, sLongPath, pData, 512, 1);
    ASSERT_NOT_NULL(pResult);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_NE(xpkPathFind(xpk, sLongPath), UINT32_MAX);

    free(pData);
    xpkClose(xpk);
}

TEST(dot_directory_handling) {
    char* pData = createTestData_25(1024, 'N');
    const char* sFilename = "test_25_dots.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_LINUX, 0);
    ASSERT_NOT_NULL(xpk);

    void* pResult1 = xpkPathAppendData(xpk, "./file.txt", pData, 1024, 1);
    ASSERT_NOT_NULL(pResult1);
    void* pResult2 = xpkPathAppendData(xpk, "../file.txt", pData, 1024, 1);
    ASSERT_NOT_NULL(pResult2);
    void* pResult3 = xpkPathAppendData(xpk, "folder/./file.txt", pData, 1024, 1);
    ASSERT_NOT_NULL(pResult3);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_NE(xpkPathFind(xpk, "./file.txt"), UINT32_MAX);
    ASSERT_NE(xpkPathFind(xpk, "../file.txt"), UINT32_MAX);
    ASSERT_NE(xpkPathFind(xpk, "folder/./file.txt"), UINT32_MAX);

    free(pData);
    xpkClose(xpk);
}

TEST(drive_letter_in_path) {
    char* pData = createTestData_25(1024, 'O');
    const char* sFilename = "test_25_drive.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_WIN32, 0);
    ASSERT_NOT_NULL(xpk);

    void* pResult1 = xpkPathAppendData(xpk, "D:\\data\\file.txt", pData, 1024, 1);
    ASSERT_NOT_NULL(pResult1);
    void* pResult2 = xpkPathAppendData(xpk, "E:\\backup\\file.txt", pData, 1024, 1);
    ASSERT_NOT_NULL(pResult2);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_NE(xpkPathFind(xpk, "D:\\data\\file.txt"), UINT32_MAX);
    ASSERT_NE(xpkPathFind(xpk, "E:\\backup\\file.txt"), UINT32_MAX);

    free(pData);
    xpkClose(xpk);
}

TEST(special_characters_in_path) {
    char* pData = createTestData_25(1024, 'P');
    const char* sFilename = "test_25_special.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_LINUX, 0);
    ASSERT_NOT_NULL(xpk);

    void* pResult1 = xpkPathAppendData(xpk, "folder/file_name-test.txt", pData, 1024, 1);
    ASSERT_NOT_NULL(pResult1);
    void* pResult2 = xpkPathAppendData(xpk, "folder/file.name.txt", pData, 1024, 1);
    ASSERT_NOT_NULL(pResult2);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_NE(xpkPathFind(xpk, "folder/file_name-test.txt"), UINT32_MAX);
    ASSERT_NE(xpkPathFind(xpk, "folder/file.name.txt"), UINT32_MAX);

    free(pData);
    xpkClose(xpk);
}

void register_25_cross_platform_tests(void) {
    TEST_REGISTER(win32_path_case_insensitive, CAT_PATH, "Win32 path case insensitive");
    TEST_REGISTER(linux_path_case_sensitive, CAT_PATH, "Linux path case sensitive");
    TEST_REGISTER(win32_path_separator_handling, CAT_PATH, "Win32 path separator handling");
    TEST_REGISTER(linux_path_separator_handling, CAT_PATH, "Linux path separator handling");
    TEST_REGISTER(win32_and_linux_comparison, CAT_PATH, "Win32 and Linux comparison");
    TEST_REGISTER(absolute_path_handling, CAT_PATH, "Absolute path handling");
    TEST_REGISTER(relative_path_handling, CAT_PATH, "Relative path handling");
    TEST_REGISTER(path_with_spaces, CAT_PATH, "Path with spaces");
    TEST_REGISTER(unicode_path_handling, CAT_PATH, "Unicode path handling");
    TEST_REGISTER(nested_directory_structure, CAT_PATH, "Nested directory structure");
    TEST_REGISTER(file_extension_variations, CAT_PATH, "File extension variations");
    TEST_REGISTER(path_length_limits, CAT_PATH, "Path length limits");
    TEST_REGISTER(dot_directory_handling, CAT_PATH, "Dot directory handling");
    TEST_REGISTER(drive_letter_in_path, CAT_PATH, "Drive letter in path");
    TEST_REGISTER(special_characters_in_path, CAT_PATH, "Special characters in path");
}
