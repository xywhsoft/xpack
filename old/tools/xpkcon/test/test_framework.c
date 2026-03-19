#include "test_framework.h"

int g_tests_passed = 0;
int g_tests_failed = 0;
int g_tests_total = 0;
static int g_test_failed = 0;

static char g_xpkcon_path[512] = "xpkcon";

void test_set_failed(void) {
	g_test_failed = 1;
}

int test_get_failed(void) {
	return g_test_failed;
}

void test_clear_failed(void) {
	g_test_failed = 0;
}

const char* test_category_name(TestCategory category) {
	switch (category) {
		case TEST_CATEGORY_BASIC: return "Basic Operations";
		case TEST_CATEGORY_COMPRESSION: return "Compression Tests";
		case TEST_CATEGORY_EXTRACT: return "Extract Tests";
		case TEST_CATEGORY_LIST: return "List Tests";
		case TEST_CATEGORY_DELETE: return "Delete Tests";
		case TEST_CATEGORY_UPDATE: return "Update Tests";
		case TEST_CATEGORY_SOLID: return "Solid Compression";
		case TEST_CATEGORY_TYPES: return "Package Types";
		case TEST_CATEGORY_EDGE: return "Edge Cases";
		case TEST_CATEGORY_VOLUME: return "Volume Compression";
		default: return "Unknown Category";
	}
}

void test_print_header(const char* title) {
	printf("=================================================\n");
	printf("  xpkcon Test Suite - %s\n", title);
	printf("=================================================\n\n");
}

void test_print_footer(void) {
	printf("=================================================\n");
	printf("  Results: %d/%d tests passed (%d failed)\n", 
	       g_tests_passed, g_tests_total, g_tests_failed);
	printf("=================================================\n");
}

void test_print_separator(void) {
	printf("-------------------------------------------------\n");
}

void test_print_category(const char* name) {
	printf("[%s]\n", name);
}

int test_run_command(const char* cmd) {
#ifdef _WIN32
	STARTUPINFOA si = {0};
	PROCESS_INFORMATION pi = {0};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

	char cmdLine[4096];
	snprintf(cmdLine, sizeof(cmdLine), "cmd /c %s", cmd);

	if (!CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE, 
	                   CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
		return -1;
	}

	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD exitCode;
	GetExitCodeProcess(pi.hProcess, &exitCode);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return (int)exitCode;
#else
	return system(cmd);
#endif
}

int test_file_exists(const char* path) {
#ifdef _WIN32
	DWORD attrs = GetFileAttributesA(path);
	return (attrs != INVALID_FILE_ATTRIBUTES && 
	        !(attrs & FILE_ATTRIBUTE_DIRECTORY));
#else
	struct stat st;
	return (stat(path, &st) == 0 && S_ISREG(st.st_mode));
#endif
}

int test_file_size(const char* path, long* size) {
#ifdef _WIN32
	HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 
	                          NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return -1;
	LARGE_INTEGER li;
	if (!GetFileSizeEx(hFile, &li)) {
		CloseHandle(hFile);
		return -1;
	}
	*size = (long)li.QuadPart;
	CloseHandle(hFile);
#else
	struct stat st;
	if (stat(path, &st) != 0) return -1;
	*size = (long)st.st_size;
#endif
	return 0;
}

int test_file_compare(const char* file1, const char* file2) {
	long size1, size2;
	if (test_file_size(file1, &size1) != 0) return -1;
	if (test_file_size(file2, &size2) != 0) return -1;
	if (size1 != size2) return 1;

	void* buf1, *buf2;
	long readSize1, readSize2;
	if (test_file_read(file1, &buf1, &readSize1) != 0) return -1;
	if (test_file_read(file2, &buf2, &readSize2) != 0) {
		test_file_free(buf1);
		return -1;
	}

	int result = memcmp(buf1, buf2, (size_t)size1);
	test_file_free(buf1);
	test_file_free(buf2);
	return result;
}

int test_file_read(const char* path, void** buffer, long* size) {
	FILE* f = fopen(path, "rb");
	if (!f) return -1;

	fseek(f, 0, SEEK_END);
	long fileSize = ftell(f);
	fseek(f, 0, SEEK_SET);

	*buffer = malloc(fileSize);
	if (!*buffer) {
		fclose(f);
		return -1;
	}

	if (fread(*buffer, 1, fileSize, f) != (size_t)fileSize) {
		free(*buffer);
		*buffer = NULL;
		fclose(f);
		return -1;
	}

	fclose(f);
	*size = fileSize;
	return 0;
}

void test_file_free(void* buffer) {
	if (buffer) free(buffer);
}

int test_dir_create(const char* path) {
#ifdef _WIN32
	return _mkdir(path);
#else
	return mkdir(path, 0755);
#endif
}

int test_dir_exists(const char* path) {
#ifdef _WIN32
	DWORD attrs = GetFileAttributesA(path);
	return (attrs != INVALID_FILE_ATTRIBUTES && 
	        (attrs & FILE_ATTRIBUTE_DIRECTORY));
#else
	struct stat st;
	return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
#endif
}

int test_dir_remove(const char* path) {
#ifdef _WIN32
	char cmd[512];
	snprintf(cmd, sizeof(cmd), "rmdir /s /q \"%s\" 2>nul", path);
	return system(cmd);
#else
	char cmd[512];
	snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", path);
	return system(cmd);
#endif
}

int test_create_test_file(const char* path, const char* content) {
	FILE* f = fopen(path, "wb");
	if (!f) return -1;
	size_t len = strlen(content);
	if (fwrite(content, 1, len, f) != len) {
		fclose(f);
		return -1;
	}
	fclose(f);
	return 0;
}

int test_create_test_binary_file(const char* path, const void* data, size_t size) {
	FILE* f = fopen(path, "wb");
	if (!f) return -1;
	if (fwrite(data, 1, size, f) != size) {
		fclose(f);
		return -1;
	}
	fclose(f);
	return 0;
}

int test_get_xpkcon_path_copy(char* buffer, size_t size) {
#ifdef _WIN32
	const char* releaseDir = "../../../release/x64/xpkcon.exe";
#else
	const char* releaseDir = "../../../release/linux/xpkcon";
#endif
	if (test_file_exists(releaseDir)) {
		strncpy(buffer, releaseDir, size - 1);
		buffer[size - 1] = '\0';
		return 0;
	}
#ifdef _WIN32
	const char* altDir = "../../../release/x86/xpkcon.exe";
	if (test_file_exists(altDir)) {
		strncpy(buffer, altDir, size - 1);
		buffer[size - 1] = '\0';
		return 0;
	}
#endif
	strncpy(buffer, "xpkcon", size - 1);
	buffer[size - 1] = '\0';
	return 0;
}

void test_set_xpkcon_path(const char* path) {
	strncpy(g_xpkcon_path, path, sizeof(g_xpkcon_path) - 1);
	g_xpkcon_path[sizeof(g_xpkcon_path) - 1] = '\0';
}

const char* test_get_xpkcon_path(void) {
	return g_xpkcon_path;
}

int test_clean_test_files(void) {
#ifdef _WIN32
	test_run_command("del /q test*.xpk 2>nul");
	test_run_command("del /q test*.txt 2>nul");
	test_run_command("del /q vol*.txt 2>nul");
	test_run_command("del /q vfc*.txt 2>nul");
	test_run_command("del /q k_test*.txt 2>nul");
	test_run_command("del /q m_test*.txt 2>nul");
	test_run_command("del /q vinfo*.txt 2>nul");
	test_run_command("del /q vext*.txt 2>nul");
	test_run_command("del /q vsol*.txt 2>nul");
	test_run_command("rmdir /s /q test_output 2>nul");
	test_run_command("rmdir /s /q test_*_out 2>nul");
#else
	test_run_command("rm -f test*.xpk 2>/dev/null");
	test_run_command("rm -f test*.txt 2>/dev/null");
	test_run_command("rm -f vol*.txt 2>/dev/null");
	test_run_command("rm -f vfc*.txt 2>/dev/null");
	test_run_command("rm -f k_test*.txt 2>/dev/null");
	test_run_command("rm -f m_test*.txt 2>/dev/null");
	test_run_command("rm -f vinfo*.txt 2>/dev/null");
	test_run_command("rm -f vext*.txt 2>/dev/null");
	test_run_command("rm -f vsol*.txt 2>/dev/null");
	test_run_command("rm -rf test_output 2>/dev/null");
	test_run_command("rm -rf test_*_out 2>/dev/null");
#endif
	return 0;
}
