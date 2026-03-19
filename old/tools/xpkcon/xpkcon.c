#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <windows.h>
#include "xrt/xrt.h"
#include "xpack.h"

#define MAX_FILES 4096
#define MAX_LINE 1024
#define MAX_PATH_LEN 1024

typedef struct {
	char command[16];
	char archive[MAX_PATH_LEN];
	int packType;
	int compLevel;
	int solidMode;
	int volumeMode;
	uint32_t volumeSize;
	int volumeSplitMode;
	char outputDir[MAX_PATH_LEN];
	int autoYes;
	int recursive;
	int verbose;
	char* files[MAX_FILES];
	int fileCount;
} CommandLineArgs;

typedef struct {
	uint64_t totalSize;
	uint64_t processedSize;
	uint32_t fileCount;
	time_t startTime;
} ProgressInfo;
static const char* VERSION = "1.2.0";
static ProgressInfo g_progress = {0};

void print_usage(void);
void print_version(void);
int parse_args(int argc, char* argv[], CommandLineArgs* args);
int cmd_add(xpkObject xpk, CommandLineArgs* args);
int cmd_extract(xpkObject xpk, CommandLineArgs* args);
int cmd_extract_simple(xpkObject xpk, CommandLineArgs* args);
int cmd_list(xpkObject xpk, CommandLineArgs* args);
int cmd_test(xpkObject xpk, CommandLineArgs* args);
int cmd_delete(xpkObject xpk, CommandLineArgs* args);
int cmd_update(xpkObject xpk, CommandLineArgs* args);
int cmd_info(xpkObject xpk, CommandLineArgs* args);
int is_directory(const char* path);
int is_file_exists(const char* path);
void format_size(uint64_t size, char* buf, int bufSize);
void format_time(time_t t, char* buf, int bufSize);
int wildcard_match(const char* pattern, const char* text);
int process_directory(const char* dir, const char* pattern, char** files, int* count, int maxCount, int recursive);
void init_progress(uint64_t totalSize, uint32_t fileCount);
void update_progress(uint64_t processed);
void finish_progress(void);

int main(int argc, char* argv[])
{
	CommandLineArgs args = {0};
	xpkObject xpk = NULL;
	int ret = 0;

	if (argc < 2) {
		print_usage();
		return 1;
	}

	if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
		print_usage();
		return 0;
	}

	if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
		print_version();
		return 0;
	}

	if (parse_args(argc, argv, &args) != 0) {
		return 1;
	}

	xpk = xpkOpen(args.archive, 0, 0);
	if (xpk == NULL) {
		fprintf(stderr, "Error: Cannot open archive '%s'\n", args.archive);
		return 1;
	}

	if (args.command[0] == '\0') {
		fprintf(stderr, "Error: No command specified\n");
		print_usage();
		xpkClose(xpk);
		return 1;
	}

	if (strcmp(args.command, "a") == 0) {
		ret = cmd_add(xpk, &args);
	} else if (strcmp(args.command, "x") == 0) {
		ret = cmd_extract(xpk, &args);
	} else if (strcmp(args.command, "e") == 0) {
		ret = cmd_extract_simple(xpk, &args);
	} else if (strcmp(args.command, "l") == 0) {
		ret = cmd_list(xpk, &args);
	} else if (strcmp(args.command, "t") == 0) {
		ret = cmd_test(xpk, &args);
	} else if (strcmp(args.command, "d") == 0) {
		ret = cmd_delete(xpk, &args);
	} else if (strcmp(args.command, "u") == 0) {
		ret = cmd_update(xpk, &args);
	} else if (strcmp(args.command, "i") == 0) {
		ret = cmd_info(xpk, &args);
	} else {
		fprintf(stderr, "Error: Unknown command '%s'\n", args.command);
		ret = 1;
	}

	if (ret == 0) {
		if (xpkSave(xpk) != 0) {
			fprintf(stderr, "Warning: Failed to save archive\n");
		}
	}

	xpkClose(xpk);
	return ret;
}

void print_usage(void)
{
	printf("xpkcon - xPack Command Line Tool v%s\n", VERSION);
	printf("\n");
	printf("Usage: xpkcon <command> <archive> [options] [files...]\n");
	printf("\n");
	printf("Commands:\n");
	printf("  a <archive> [files...]    Add files to archive\n");
	printf("  x <archive> [files...]    Extract files with full paths\n");
	printf("  e <archive> [files...]    Extract files to current directory\n");
	printf("  l <archive>               List archive contents\n");
	printf("  t <archive>               Test archive integrity\n");
	printf("  d <archive> [files...]    Delete files from archive\n");
	printf("  u <archive> [files...]    Update files in archive\n");
	printf("  i <archive>               Display archive information\n");
	printf("\n");
	printf("Options:\n");
	printf("  -t<type>     Package type: core/index/linux/win32\n");
	printf("  -l<level>    Compression level: 0-15 (default: 7)\n");
	printf("  -s<0|1>      Solid mode: 0=separate, 1=solid\n");
	printf("  -V<size>     Volume size in bytes (e.g., 10M, 100M, 1G)\n");
	printf("  --split-mode  Split mode: 0=bytes, 1=files (default: 0)\n");
	printf("  -o<path>     Output directory\n");
	printf("  -y           Assume yes on all prompts\n");
	printf("  -r           Recursive directory processing\n");
	printf("  -v           Verbose output\n");
	printf("  -h, --help   Show this help message\n");
	printf("  --version    Show version information\n");
	printf("\n");
	printf("Volume Size Format:\n");
	printf("  Examples: 10M (10 MB), 100M (100 MB), 1G (1 GB)\n");
	printf("  Supported suffixes: K, M, G (case-insensitive)\n");
	printf("\n");
	printf("Compression Levels:\n");
	printf("  0  = No compression\n");
	printf("  1-4= LZ4/LZ4-HC\n");
	printf("  5-13= ZSTD (fast to ultra)\n");
	printf("  14-15= LZMA2 (high compression)\n");
}

void print_version(void)
{
	printf("xpkcon version %s\n", VERSION);
	printf("xPack library version 7.0\n");
	printf("Built with TCC compiler\n");
}

uint32_t parse_volume_size(const char* str)
{
	uint64_t size = 0;
	char unit = 0;
	
	if (sscanf(str, "%llu%c", &size, &unit) != 2) {
		size = atoll(str);
		return (uint32_t)size;
	}

	switch (unit) {
		case 'K':
		case 'k':
			size *= 1024;
			break;
		case 'M':
		case 'm':
			size *= 1024 * 1024;
			break;
		case 'G':
		case 'g':
			size *= 1024 * 1024 * 1024;
			break;
		default:
			break;
	}

	return (uint32_t)size;
}

int parse_args(int argc, char* argv[], CommandLineArgs* args)
{
	args->packType = XPK_TYPE_WIN32;
	args->compLevel = 7;
	args->solidMode = 0;
	args->volumeMode = 0;
	args->volumeSize = 0;
	args->volumeSplitMode = 0;
	args->autoYes = 0;
	args->recursive = 0;
	args->verbose = 0;
	args->fileCount = 0;
	args->command[0] = '\0';

	int i = 1;
	while (i < argc) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == '-') {
				if (strcmp(argv[i], "--help") == 0) {
					print_usage();
					return 1;
				}
				if (strcmp(argv[i], "--version") == 0) {
					print_version();
					return 1;
				}
				if (strcmp(argv[i], "--split-mode") == 0) {
					if (i + 1 < argc) {
						i++;
						args->volumeSplitMode = atoi(argv[i]);
					}
				}
			} else {
				switch (argv[i][1]) {
					case 't':
						if (argv[i][2] != '\0') {
							if (strcmp(&argv[i][2], "core") == 0) args->packType = XPK_TYPE_CORE;
							else if (strcmp(&argv[i][2], "index") == 0) args->packType = XPK_TYPE_INDEX;
							else if (strcmp(&argv[i][2], "linux") == 0) args->packType = XPK_TYPE_LINUX;
							else if (strcmp(&argv[i][2], "win32") == 0) args->packType = XPK_TYPE_WIN32;
							else fprintf(stderr, "Warning: Unknown type '%s'\n", &argv[i][2]);
						} else if (i + 1 < argc) {
							i++;
							if (strcmp(argv[i], "core") == 0) args->packType = XPK_TYPE_CORE;
							else if (strcmp(argv[i], "index") == 0) args->packType = XPK_TYPE_INDEX;
							else if (strcmp(argv[i], "linux") == 0) args->packType = XPK_TYPE_LINUX;
							else if (strcmp(argv[i], "win32") == 0) args->packType = XPK_TYPE_WIN32;
						}
						break;
					case 'l':
						if (argv[i][2] != '\0') {
							args->compLevel = atoi(&argv[i][2]);
						} else if (i + 1 < argc) {
							i++;
							args->compLevel = atoi(argv[i]);
						}
						if (args->compLevel < 0 || args->compLevel > 15) {
							fprintf(stderr, "Error: Compression level must be 0-15\n");
							return 1;
						}
						break;
					case 's':
						if (argv[i][2] != '\0') {
							args->solidMode = atoi(&argv[i][2]);
						} else if (i + 1 < argc) {
							i++;
							args->solidMode = atoi(argv[i]);
						}
						break;
					case 'V':
						if (argv[i][2] != '\0') {
							args->volumeSize = parse_volume_size(&argv[i][2]);
						} else if (i + 1 < argc) {
							i++;
							args->volumeSize = parse_volume_size(argv[i]);
						}
						if (args->volumeSize > 0) {
							args->volumeMode = 1;
						}
						break;
					case 'o':
						if (argv[i][2] != '\0') {
							strncpy(args->outputDir, &argv[i][2], MAX_PATH_LEN - 1);
						} else if (i + 1 < argc) {
							i++;
							strncpy(args->outputDir, argv[i], MAX_PATH_LEN - 1);
						}
						break;
					case 'y':
						args->autoYes = 1;
						break;
					case 'r':
						args->recursive = 1;
						break;
					case 'v':
						args->verbose = 1;
						break;
					case 'h':
						print_usage();
						return 1;
					default:
						fprintf(stderr, "Warning: Unknown option '%s'\n", argv[i]);
						break;
				}
			}
		} else {
			if (args->command[0] == '\0') {
				strncpy(args->command, argv[i], sizeof(args->command) - 1);
			} else if (args->archive[0] == '\0') {
				strncpy(args->archive, argv[i], MAX_PATH_LEN - 1);
			} else {
				if (args->fileCount < MAX_FILES) {
					args->files[args->fileCount++] = argv[i];
				}
			}
		}
		i++;
	}

	if (args->command[0] == '\0') {
		fprintf(stderr, "Error: No command specified\n");
		return 1;
	}

	if (args->archive[0] == '\0') {
		fprintf(stderr, "Error: No archive specified\n");
		return 1;
	}

	return 0;
}

int cmd_add(xpkObject xpk, CommandLineArgs* args)
{
	int type = xpkType(xpk);
	uint32_t count = xpkCount(xpk);

	if (count == 0) {
		if (xpkTypeSet(xpk, args->packType) != 0) {
			fprintf(stderr, "Error: Failed to set package type\n");
			return 1;
		}
		if (args->solidMode && xpkSolidModeSet(xpk, 1) != 0) {
			fprintf(stderr, "Error: Failed to set solid mode\n");
			return 1;
		}
		if (args->volumeMode && xpkVolumeModeSet(xpk, 1) != 0) {
			fprintf(stderr, "Error: Failed to set volume mode\n");
			return 1;
		}
		if (args->volumeSize > 0 && xpkVolumeSizeSet(xpk, args->volumeSize) != 0) {
			fprintf(stderr, "Error: Failed to set volume size\n");
			return 1;
		}
		if (args->volumeSplitMode >= 0 && xpkVolumeSplitModeSet(xpk, args->volumeSplitMode) != 0) {
			fprintf(stderr, "Error: Failed to set volume split mode\n");
			return 1;
		}
	} else {
		if (type != args->packType) {
			fprintf(stderr, "Warning: Archive type mismatch, using existing type\n");
		}
		if (args->solidMode != xpkSolidMode(xpk)) {
			fprintf(stderr, "Warning: Solid mode cannot be changed on existing archive\n");
		}
		if (args->volumeMode && !xpkVolumeMode(xpk)) {
			fprintf(stderr, "Error: Volume mode cannot be enabled on existing archive\n");
			return 1;
		}
	}

	if (args->fileCount == 0) {
		fprintf(stderr, "Error: No files specified\n");
		return 1;
	}

	if (args->verbose) {
		if (args->volumeMode) {
			char sizeBuf[32];
			format_size(args->volumeSize, sizeBuf, sizeof(sizeBuf));
			printf("Volume mode: enabled (size: %s)\n", sizeBuf);
		}
		if (args->solidMode) {
			printf("Solid mode: enabled\n");
		}
	}

	for (int i = 0; i < args->fileCount; i++) {
		const char* path = args->files[i];
		
		if (is_directory(path)) {
			if (args->recursive) {
				char* subFiles[MAX_FILES];
				int subCount = 0;
				process_directory(path, "*", subFiles, &subCount, MAX_FILES, 1);
				
				for (int j = 0; j < subCount; j++) {
					uint32_t pos;
					if (args->packType == XPK_TYPE_WIN32 || args->packType == XPK_TYPE_LINUX) {
						pos = (uint32_t)(uintptr_t)xpkPathAppendFile(xpk, subFiles[j], subFiles[j], args->compLevel);
					} else {
						pos = xpkAppendFile(xpk, subFiles[j], args->compLevel);
					}
					if (pos == UINT32_MAX) {
						fprintf(stderr, "Error: Failed to add '%s'\n", subFiles[j]);
						free(subFiles[j]);
						return 1;
					}
					if (args->verbose) {
						printf("  Added: %s\n", subFiles[j]);
					}
					free(subFiles[j]);
				}
			} else {
				fprintf(stderr, "Warning: Skipping directory '%s' (use -r for recursive)\n", path);
			}
		} else if (is_file_exists(path)) {
			uint32_t pos;
			if (args->packType == XPK_TYPE_WIN32 || args->packType == XPK_TYPE_LINUX) {
				pos = (uint32_t)(uintptr_t)xpkPathAppendFile(xpk, path, path, args->compLevel);
			} else {
				pos = xpkAppendFile(xpk, path, args->compLevel);
			}
			if (pos == UINT32_MAX) {
				fprintf(stderr, "Error: Failed to add '%s'\n", path);
				return 1;
			}
			if (args->verbose) {
				printf("  Added: %s\n", path);
			}
		} else {
			fprintf(stderr, "Error: File not found '%s'\n", path);
			return 1;
		}
	}

	if (args->volumeMode) {
		int volCount = xpkVolumeCount(xpk);
		if (volCount > 1) {
			printf("Created %d volumes\n", volCount);
			for (int i = 0; i < volCount; i++) {
				const char* volPath = xpkVolumePath(xpk, i);
				if (volPath) {
					printf("  Volume %d: %s\n", i + 1, volPath);
				}
			}
		}
	}

	return 0;
}

int cmd_extract(xpkObject xpk, CommandLineArgs* args)
{
	uint32_t count = xpkCount(xpk);
	if (count == 0) {
		printf("Archive is empty\n");
		return 0;
	}

	char baseDir[MAX_PATH_LEN];
	if (args->outputDir[0] != '\0') {
		strncpy(baseDir, args->outputDir, MAX_PATH_LEN - 1);
	} else {
		strncpy(baseDir, ".", MAX_PATH_LEN - 1);
	}

	int packType = xpkType(xpk);
	int extracted = 0;
	for (uint32_t i = 0; i < count; i++) {
		int shouldExtract = 1;
		if (args->fileCount > 0) {
			shouldExtract = 0;
			for (int j = 0; j < args->fileCount; j++) {
				if (packType == XPK_TYPE_WIN32 || packType == XPK_TYPE_LINUX) {
					const char* filePath = xpkPathGet(xpk, i);
					if (filePath && wildcard_match(args->files[j], filePath)) {
						shouldExtract = 1;
						break;
					}
				} else {
					char posStr[32];
					snprintf(posStr, sizeof(posStr), "%u", i);
					if (wildcard_match(args->files[j], posStr)) {
						shouldExtract = 1;
						break;
					}
				}
			}
		}

		if (shouldExtract) {
			char outPath[MAX_PATH_LEN];
			if (packType == XPK_TYPE_WIN32 || packType == XPK_TYPE_LINUX) {
				const char* filePath = xpkPathGet(xpk, i);
				if (!filePath) {
					fprintf(stderr, "Error: Failed to get file path for position %u\n", i);
					continue;
				}
				snprintf(outPath, MAX_PATH_LEN, "%s/%s", baseDir, filePath);
			} else {
				snprintf(outPath, MAX_PATH_LEN, "%s/file_%u", baseDir, i);
			}
			
			int result = xpkExtractFile(xpk, i, outPath);
			if (result == 0) {
				if (args->verbose) {
					printf("  Extracted: %s\n", outPath);
				}
				extracted++;
			} else {
				if (args->verbose) {
					fprintf(stderr, "  Failed to extract: %s (error code: %d)\n", outPath, result);
				}
			}
		}
	}

	printf("Extracted %d file(s)\n", extracted);
	return 0;
}

int cmd_extract_simple(xpkObject xpk, CommandLineArgs* args)
{
	uint32_t count = xpkCount(xpk);
	if (count == 0) {
		printf("Archive is empty\n");
		return 0;
	}

	char baseDir[MAX_PATH_LEN];
	if (args->outputDir[0] != '\0') {
		strncpy(baseDir, args->outputDir, MAX_PATH_LEN - 1);
	} else {
		strncpy(baseDir, ".", MAX_PATH_LEN - 1);
	}

	int packType = xpkType(xpk);
	int extracted = 0;
	for (uint32_t i = 0; i < count; i++) {
		int shouldExtract = 1;
		if (args->fileCount > 0) {
			shouldExtract = 0;
			for (int j = 0; j < args->fileCount; j++) {
				if (packType == XPK_TYPE_WIN32 || packType == XPK_TYPE_LINUX) {
					const char* filePath = xpkPathGet(xpk, i);
					if (filePath && wildcard_match(args->files[j], filePath)) {
						shouldExtract = 1;
						break;
					}
				} else {
					char posStr[32];
					snprintf(posStr, sizeof(posStr), "%u", i);
					if (wildcard_match(args->files[j], posStr)) {
						shouldExtract = 1;
						break;
					}
				}
			}
		}

		if (shouldExtract) {
			char outPath[MAX_PATH_LEN];
			if (packType == XPK_TYPE_WIN32 || packType == XPK_TYPE_LINUX) {
				const char* filePath = xpkPathGet(xpk, i);
				if (!filePath) {
					fprintf(stderr, "Error: Failed to get file path for position %u\n", i);
					continue;
				}
				char* lastSlash = strrchr(filePath, '/');
				char* lastBackslash = strrchr(filePath, '\\');
				char* fileName = filePath;
				if (lastSlash) fileName = lastSlash + 1;
				if (lastBackslash && lastBackslash > fileName) fileName = lastBackslash + 1;
				snprintf(outPath, MAX_PATH_LEN, "%s/%s", baseDir, fileName);
			} else {
				snprintf(outPath, MAX_PATH_LEN, "%s/file_%u", baseDir, i);
			}
			
			if (xpkExtractFile(xpk, i, outPath) == 0) {
				if (args->verbose) {
					printf("  Extracted: %s\n", outPath);
				}
				extracted++;
			}
		}
	}

	printf("Extracted %d file(s)\n", extracted);
	return 0;
}

int cmd_list(xpkObject xpk, CommandLineArgs* args)
{
	uint32_t count = xpkCount(xpk);
	if (count == 0) {
		printf("Archive is empty\n");
		return 0;
	}

	int packType = xpkType(xpk);
	
	if (packType == XPK_TYPE_WIN32 || packType == XPK_TYPE_LINUX) {
		printf("  %-40s %12s %12s %8s %6s\n", "Filename", "Size", "Packed", "Ratio", "Level");
		printf("  %-40s %12s %12s %8s %6s\n", "--------", "----", "------", "-----", "-----");
	} else {
		printf("  %-10s %12s %12s %8s %6s\n", "Position", "Size", "Packed", "Ratio", "Level");
		printf("  %-10s %12s %12s %8s %6s\n", "--------", "----", "------", "-----", "-----");
	}

	uint64_t totalSize = 0;
	uint64_t totalPacked = 0;

	for (uint32_t i = 0; i < count; i++) {
		uint32_t size = xpkInfoSize(xpk, i);
		uint32_t packed = xpkInfoPacked(xpk, i);
		int level = xpkInfoLevel(xpk, i);

		totalSize += size;
		totalPacked += packed;

		char sizeBuf[32], packedBuf[32];
		format_size(size, sizeBuf, sizeof(sizeBuf));
		format_size(packed, packedBuf, sizeof(packedBuf));

		float ratio = size > 0 ? (100.0f * packed / size) : 0.0f;
		
		if (packType == XPK_TYPE_WIN32 || packType == XPK_TYPE_LINUX) {
			const char* filePath = xpkPathGet(xpk, i);
			if (filePath) {
				char fileName[41];
				strncpy(fileName, filePath, 40);
				fileName[40] = '\0';
				printf("  %-40s %12s %12s %7.1f%% %6d\n", fileName, sizeBuf, packedBuf, ratio, level);
			} else {
				printf("  %-40s %12s %12s %7.1f%% %6d\n", "<unknown>", sizeBuf, packedBuf, ratio, level);
			}
		} else {
			printf("  %-10u %12s %12s %7.1f%% %6d\n", i, sizeBuf, packedBuf, ratio, level);
		}
	}

	char totalSizeBuf[32], totalPackedBuf[32];
	format_size(totalSize, totalSizeBuf, sizeof(totalSizeBuf));
	format_size(totalPacked, totalPackedBuf, sizeof(totalPackedBuf));
	float totalRatio = totalSize > 0 ? (100.0f * totalPacked / totalSize) : 0.0f;

	printf("\nTotal: %u file(s)\n", count);
	printf("  Size:   %s\n", totalSizeBuf);
	printf("  Packed: %s (%.1f%%)\n", totalPackedBuf, totalRatio);

	return 0;
}

int cmd_test(xpkObject xpk, CommandLineArgs* args)
{
	uint32_t count = xpkCount(xpk);
	if (count == 0) {
		printf("Archive is empty\n");
		return 0;
	}

	int passed = 0;
	int failed = 0;

	for (uint32_t i = 0; i < count; i++) {
		if (xpkVerify(xpk, i) == 0) {
			if (args->verbose) {
				printf("  OK: %u\n", i);
			}
			passed++;
		} else {
			if (args->verbose) {
				printf("  FAILED: %u\n", i);
			}
			failed++;
		}
	}

	printf("Test results: %d passed, %d failed\n", passed, failed);
	return failed > 0 ? 1 : 0;
}

int cmd_delete(xpkObject xpk, CommandLineArgs* args)
{
	if (args->fileCount == 0) {
		fprintf(stderr, "Error: No files specified\n");
		return 1;
	}

	if (!args->autoYes) {
		printf("Delete %d file(s)? [y/N] ", args->fileCount);
		char response[10];
		if (fgets(response, sizeof(response), stdin) == NULL || 
		    (response[0] != 'y' && response[0] != 'Y')) {
			printf("Cancelled\n");
			return 1;
		}
	}

	for (int i = 0; i < args->fileCount; i++) {
		uint32_t pos = xpkPathFind(xpk, args->files[i]);
		if (pos != UINT32_MAX) {
			if (xpkRemove(xpk, pos) == 0) {
				if (args->verbose) {
					printf("  Deleted: %s\n", args->files[i]);
				}
			}
		}
	}

	return 0;
}

int cmd_update(xpkObject xpk, CommandLineArgs* args)
{
	if (args->fileCount == 0) {
		fprintf(stderr, "Error: No files specified\n");
		return 1;
	}

	for (int i = 0; i < args->fileCount; i++) {
		const char* path = args->files[i];
		
		if (is_file_exists(path)) {
			uint32_t pos = xpkPathFind(xpk, path);
			if (pos != UINT32_MAX) {
				if (xpkUpdateFile(xpk, pos, path, args->compLevel) == 0) {
					if (args->verbose) {
						printf("  Updated: %s\n", path);
					}
				} else {
					fprintf(stderr, "Error: Failed to update '%s'\n", path);
					return 1;
				}
			}
		}
	}

	return 0;
}

int cmd_info(xpkObject xpk, CommandLineArgs* args)
{
	uint32_t count = xpkCount(xpk);
	int type = xpkType(xpk);
	int solid = xpkSolidMode(xpk);
	int volume = xpkVolumeMode(xpk);

	const char* typeStr = "Unknown";
	switch (type) {
		case XPK_TYPE_CORE: typeStr = "Core"; break;
		case XPK_TYPE_INDEX: typeStr = "Index"; break;
		case XPK_TYPE_LINUX: typeStr = "Linux"; break;
		case XPK_TYPE_WIN32: typeStr = "Win32"; break;
	}

	uint64_t totalSize = 0;
	uint64_t totalPacked = 0;
	for (uint32_t i = 0; i < count; i++) {
		totalSize += xpkInfoSize(xpk, i);
		totalPacked += xpkInfoPacked(xpk, i);
	}

	printf("Archive Information:\n");
	printf("  Path:       %s\n", args->archive);
	printf("  Type:       %s\n", typeStr);
	printf("  Files:      %u\n", count);
	printf("  Size:       %llu bytes\n", (unsigned long long)totalSize);
	printf("  Packed:     %llu bytes\n", (unsigned long long)totalPacked);
	if (totalSize > 0) {
		printf("  Ratio:      %.1f%%\n", 100.0 * totalPacked / totalSize);
	}
	printf("  Solid Mode: %s\n", solid ? "Yes" : "No");
	printf("  Volume Mode: %s\n", volume ? "Yes" : "No");

	if (volume) {
		int volCount = xpkVolumeCount(xpk);
		printf("  Volumes:    %d\n", volCount);

		xpkVolumeStat volStat;
		if (xpkVolumeStatGet(xpk, &volStat) == 0) {
			printf("  Volume Info:\n");
			for (int i = 0; i < volCount; i++) {
				const char* volPath = xpkVolumePath(xpk, i);
				if (volPath) {
					char sizeBuf[32];
					format_size(volStat.volumeSizes[i], sizeBuf, sizeof(sizeBuf));
					printf("    Volume %d: %s (%s)\n", i + 1, volPath, sizeBuf);
				}
			}
		}
	}

	return 0;
}

int is_directory(const char* path)
{
	DWORD attrs = GetFileAttributesA(path);
	return (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY));
}

int is_file_exists(const char* path)
{
	DWORD attrs = GetFileAttributesA(path);
	return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
}

void format_size(uint64_t size, char* buf, int bufSize)
{
	const char* units[] = {"B", "KB", "MB", "GB", "TB"};
	int unitIndex = 0;
	double dsize = (double)size;

	while (dsize >= 1024.0 && unitIndex < 4) {
		dsize /= 1024.0;
		unitIndex++;
	}

	snprintf(buf, bufSize, "%.2f %s", dsize, units[unitIndex]);
}

void format_time(time_t t, char* buf, int bufSize)
{
	struct tm* tm = localtime(&t);
	strftime(buf, bufSize, "%Y-%m-%d %H:%M:%S", tm);
}

int wildcard_match(const char* pattern, const char* text)
{
	if (!pattern || !text) return 0;
	
	while (*pattern && *text) {
		if (*pattern == '*') {
			pattern++;
			if (*pattern == '\0') return 1;
			
			while (*text) {
				if (wildcard_match(pattern, text)) {
					return 1;
				}
				text++;
			}
			return 0;
		} else if (*pattern == '?') {
			pattern++;
			text++;
		} else if (*pattern == *text) {
			pattern++;
			text++;
		} else {
			return 0;
		}
	}
	
	while (*pattern == '*') pattern++;
	
	return *pattern == '\0' && *text == '\0';
}

int process_directory(const char* dir, const char* pattern, char** files, int* count, int maxCount, int recursive)
{
	WIN32_FIND_DATAA fd;
	char searchPath[MAX_PATH_LEN];
	snprintf(searchPath, MAX_PATH_LEN, "%s\\*", dir);

	HANDLE hFind = FindFirstFileA(searchPath, &fd);
	if (hFind == INVALID_HANDLE_VALUE) {
		return 0;
	}

	do {
		if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) {
			continue;
		}

		char fullPath[MAX_PATH_LEN];
		snprintf(fullPath, MAX_PATH_LEN, "%s\\%s", dir, fd.cFileName);

		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (recursive && *count < maxCount) {
				process_directory(fullPath, pattern, files, count, maxCount, 1);
			}
		} else {
			if (*count < maxCount) {
				char* fileCopy = strdup(fullPath);
				if (fileCopy) {
					files[(*count)++] = fileCopy;
				}
			}
		}
	} while (FindNextFileA(hFind, &fd));

	FindClose(hFind);
	return 0;
}

void init_progress(uint64_t totalSize, uint32_t fileCount)
{
	g_progress.totalSize = totalSize;
	g_progress.processedSize = 0;
	g_progress.fileCount = fileCount;
	g_progress.startTime = time(NULL);
}

void update_progress(uint64_t processed)
{
	g_progress.processedSize = processed;
}

void finish_progress(void)
{
	time_t elapsed = time(NULL) - g_progress.startTime;
	printf("Completed in %ld seconds\n", elapsed);
}
