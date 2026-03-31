#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/xrt.h"
#include "xpack.h"

#define XPKCON_VERSION "2.0.0"

typedef enum CommandKind {
	CMD_NONE = 0,
	CMD_ADD,
	CMD_EXTRACT,
	CMD_EXTRACT_FLAT,
	CMD_LIST,
	CMD_TEST,
	CMD_DELETE,
	CMD_UPDATE,
	CMD_INFO,
	CMD_BUILD
} CommandKind;

typedef struct PatternList {
	const char** items;
	size_t count;
	size_t capacity;
} PatternList;

typedef struct InputItem {
	char* sourcePath;
	char* pathKey;
	uint32_t posKey;
	int64_t indexKey;
	int hasPathKey;
	int hasPosKey;
	int hasIndexKey;
} InputItem;

typedef struct InputList {
	InputItem* items;
	size_t count;
	size_t capacity;
} InputList;

typedef struct MatchItem {
	uint32_t pos;
	char* keyText;
} MatchItem;

typedef struct MatchList {
	MatchItem* items;
	size_t count;
	size_t capacity;
} MatchList;

typedef struct CliOptions {
	CommandKind command;
	const char* archivePath;
	const char* outputDir;
	const char* tempPath;
	xpkPackType packType;
	int packTypeSpecified;
	int compLevel;
	int solidMode;
	int solidModeSpecified;
	uint32_t volumeSize;
	int volumeSizeSpecified;
	int autoYes;
	int recursive;
	int verbose;
	PatternList patterns;
} CliOptions;

typedef struct EntryKeyView {
	const char* key;
	char buffer[64];
	int ignoreCase;
	uint64_t fileSize;
	uint64_t packedSize;
	uint32_t flag;
	int64_t indexValue;
} EntryKeyView;

typedef struct CollectMatchesContext {
	xpkPackType packType;
	const CliOptions* options;
	MatchList* matches;
} CollectMatchesContext;

typedef struct ListContext {
	xpkPackType packType;
	const CliOptions* options;
	uint64_t totalSize;
	uint64_t totalPacked;
	uint32_t matchedCount;
} ListContext;

typedef struct TestContext {
	xpkPackType packType;
	const CliOptions* options;
	uint32_t matchedCount;
	uint32_t passed;
	uint32_t failed;
} TestContext;

typedef struct ExtractContext {
	xpkPackType packType;
	const CliOptions* options;
	const char* baseDir;
	int flatMode;
	uint32_t matchedCount;
	uint32_t extracted;
} ExtractContext;

typedef struct IndexMaxContext {
	int found;
	int64_t maxIndex;
} IndexMaxContext;

typedef struct ExpandScanContext {
	xpkPackType packType;
	CommandKind command;
	const char* scanRoot;
	const char* pathPrefix;
	InputList* inputs;
	int64_t* nextIndex;
	int hadError;
} ExpandScanContext;

static void print_usage(void);
static void print_version(void);
static int parse_args(int argc, char** argv, CliOptions* options);
static int command_add_or_update(xpkObject pkg, const CliOptions* options, int updateMode);
static int command_extract(xpkObject pkg, const CliOptions* options, int flatMode);
static int command_list(xpkObject pkg, const CliOptions* options);
static int command_test(xpkObject pkg, const CliOptions* options);
static int command_delete(xpkObject pkg, const CliOptions* options);
static int command_info(xpkObject pkg, const CliOptions* options);
static int command_build(xpkObject pkg, const CliOptions* options);
static int apply_package_settings(xpkObject pkg, const CliOptions* options, int allowPackTypeChange, int applyLayout, int* layoutHintRet);
static int save_or_build_package(xpkObject pkg, const CliOptions* options);
static int is_build_required_message(const char* text);
static int find_pack_type(xpkObject pkg, xpkPackType* typeRet);
static const char* pack_type_name(xpkPackType type);
static const char* method_name_from_level(uint32_t level);
static int parse_pack_type_value(const char* text, xpkPackType* typeRet);
static int parse_uint32_value(const char* text, uint32_t* valueRet);
static int parse_int64_value(const char* text, int64_t* valueRet);
static int parse_volume_size(const char* text, uint32_t* valueRet);
static int pattern_list_push(PatternList* list, const char* value);
static void pattern_list_free(PatternList* list);
static int input_list_push(InputList* list, const InputItem* item);
static void input_item_clear(InputItem* item);
static void input_list_free(InputList* list);
static int match_list_push(MatchList* list, uint32_t pos, const char* keyText);
static void match_list_free(MatchList* list);
static int match_list_sort_desc(MatchList* list);
static int collect_matches_callback(xpkObject pkg, uint32_t pos, const void* info, void* userData);
static int list_callback(xpkObject pkg, uint32_t pos, const void* info, void* userData);
static int test_callback(xpkObject pkg, uint32_t pos, const void* info, void* userData);
static int extract_callback(xpkObject pkg, uint32_t pos, const void* info, void* userData);
static int index_max_callback(xpkObject pkg, uint32_t pos, const void* info, void* userData);
static int dir_expand_callback(void* pathText, size_t pathSize, int dirKind, void* fileData, void* userData);
static void fill_entry_key_view(xpkPackType packType, uint32_t pos, const void* info, EntryKeyView* view);
static int entry_matches_patterns(const CliOptions* options, const EntryKeyView* view);
static int expand_inputs(const CliOptions* options, xpkObject pkg, xpkPackType packType, int updateMode, InputList* inputs);
static int expand_one_input(const CliOptions* options, xpkPackType packType, int updateMode, const char* rawSpec, int64_t* nextIndex, InputList* inputs);
static int add_or_update_path_file(xpkObject pkg, const CliOptions* options, xpkPackType packType, int updateMode, const InputItem* item);
static int add_or_update_index_file(xpkObject pkg, const CliOptions* options, int updateMode, const InputItem* item);
static int add_or_update_core_file(xpkObject pkg, const CliOptions* options, int updateMode, const InputItem* item);
static int append_input_item_file(InputList* inputs, const char* sourcePath, const char* pathKey, int64_t indexKey, int hasIndexKey, uint32_t posKey, int hasPosKey);
static char* dup_string(const char* text);
static char* dup_string_n(const char* text, size_t size);
static char* dup_trim_trailing_seps(const char* path);
static char* dup_generated_extract_name(xpkPackType packType, uint32_t pos, const EntryKeyView* view);
static char* dup_output_full_path(const char* baseDir, const char* relativePath);
static char* dup_safe_relative_path(const char* storedPath);
static char* dup_stored_join(const char* left, const char* right);
static const char* path_basename_ptr(const char* path);
static int path_is_abs_fs(const char* path);
static int path_is_sep(char ch);
static int ensure_parent_dir(const char* path);
static int print_xpk_error(const char* action, const char* target, xpkObject pkg);
static int open_package_for_command(const CliOptions* options, xpkObject* pkgRet);

static int compare_uint32_desc(const void* a, const void* b)
{
	const MatchItem* left = (const MatchItem*)a;
	const MatchItem* right = (const MatchItem*)b;
	if (left->pos < right->pos) {
		return 1;
	}
	if (left->pos > right->pos) {
		return -1;
	}
	return 0;
}

int main(int argc, char** argv)
{
	CliOptions options;
	xpkObject pkg;
	int ret;

	memset(&options, 0, sizeof(options));
	options.compLevel = 7;
	options.packType = XPK_PACK_WIN32;
	options.outputDir = ".";

	ret = parse_args(argc, argv, &options);
	if (ret != 0) {
		pattern_list_free(&options.patterns);
		return (ret > 0) ? ret : 1;
	}

	pkg = NULL;
	ret = open_package_for_command(&options, &pkg);
	if (ret != 0) {
		pattern_list_free(&options.patterns);
		return ret;
	}

	switch (options.command) {
		case CMD_ADD:
			ret = command_add_or_update(pkg, &options, 0);
			break;
		case CMD_UPDATE:
			ret = command_add_or_update(pkg, &options, 1);
			break;
		case CMD_EXTRACT:
			ret = command_extract(pkg, &options, 0);
			break;
		case CMD_EXTRACT_FLAT:
			ret = command_extract(pkg, &options, 1);
			break;
		case CMD_LIST:
			ret = command_list(pkg, &options);
			break;
		case CMD_TEST:
			ret = command_test(pkg, &options);
			break;
		case CMD_DELETE:
			ret = command_delete(pkg, &options);
			break;
		case CMD_INFO:
			ret = command_info(pkg, &options);
			break;
		case CMD_BUILD:
			ret = command_build(pkg, &options);
			break;
		default:
			ret = 1;
			break;
	}

	if (pkg != NULL) {
		(void)xpkClose(pkg);
	}
	pattern_list_free(&options.patterns);
	return ret;
}

static void print_usage(void)
{
	printf("xpkcon %s\n", XPKCON_VERSION);
	printf("Usage:\n");
	printf("  xpkcon <command> <archive> [options] [patterns/files...]\n");
	printf("\n");
	printf("Commands:\n");
	printf("  a, add       Add files to an archive\n");
	printf("  u, update    Update files, add if missing when key can be inferred\n");
	printf("  x, extract   Extract with stored paths\n");
	printf("  e            Extract to a flat output directory\n");
	printf("  l, list      List archive contents\n");
	printf("  t, test      Verify archive data\n");
	printf("  d, delete    Delete entries by path/index/position pattern\n");
	printf("  i, info      Show archive information\n");
	printf("  b, build     Rebuild/archive compaction and apply layout changes\n");
	printf("\n");
	printf("Options:\n");
	printf("  -t<type>     Pack type: core | index | linux | win32\n");
	printf("  -l<level>    Compression level: 0..15 (default: 7)\n");
	printf("  -s<0|1>      Solid mode target: 0=off, 1=on\n");
	printf("  -V<size>     Volume size, e.g. 64M, 700M, 1G, 0 to disable\n");
	printf("  -o<dir>      Output directory for extract commands\n");
	printf("  -w<path>     Temporary build path used by xpkBuild\n");
	printf("  -r           Recurse into directories for add/update\n");
	printf("  -v           Verbose output\n");
	printf("  -y           Assume yes for delete confirmation\n");
	printf("  -h           Show help\n");
	printf("  --version    Show version\n");
	printf("\n");
	printf("Notes:\n");
	printf("  Path packs accept optional mapping syntax: pack/path::local/file\n");
	printf("  Index packs accept optional mapping syntax: 100::local/file\n");
	printf("  Core update accepts mapping syntax: 5::local/file\n");
	printf("  List/test/delete/extract patterns match stored path, index or position.\n");
}

static void print_version(void)
{
	printf("xpkcon %s\n", XPKCON_VERSION);
}

static CommandKind parse_command_name(const char* text)
{
	if (text == NULL) {
		return CMD_NONE;
	}
	if (strcmp(text, "a") == 0 || strcmp(text, "add") == 0) {
		return CMD_ADD;
	}
	if (strcmp(text, "u") == 0 || strcmp(text, "update") == 0) {
		return CMD_UPDATE;
	}
	if (strcmp(text, "x") == 0 || strcmp(text, "extract") == 0) {
		return CMD_EXTRACT;
	}
	if (strcmp(text, "e") == 0) {
		return CMD_EXTRACT_FLAT;
	}
	if (strcmp(text, "l") == 0 || strcmp(text, "list") == 0) {
		return CMD_LIST;
	}
	if (strcmp(text, "t") == 0 || strcmp(text, "test") == 0) {
		return CMD_TEST;
	}
	if (strcmp(text, "d") == 0 || strcmp(text, "delete") == 0) {
		return CMD_DELETE;
	}
	if (strcmp(text, "i") == 0 || strcmp(text, "info") == 0) {
		return CMD_INFO;
	}
	if (strcmp(text, "b") == 0 || strcmp(text, "build") == 0 || strcmp(text, "rebuild") == 0) {
		return CMD_BUILD;
	}
	return CMD_NONE;
}

static int take_option_value(int argc, char** argv, int* index, const char* arg, const char** valueRet)
{
	if (arg[2] != '\0') {
		*valueRet = arg + 2;
		return 1;
	}
	if ((*index + 1) >= argc) {
		return 0;
	}
	(*index)++;
	*valueRet = argv[*index];
	return 1;
}

static int parse_args(int argc, char** argv, CliOptions* options)
{
	int i;

	if (argc < 2) {
		print_usage();
		return 1;
	}
	if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
		print_usage();
		return 1;
	}
	if (strcmp(argv[1], "--version") == 0) {
		print_version();
		return 1;
	}

	options->command = parse_command_name(argv[1]);
	if (options->command == CMD_NONE) {
		fprintf(stderr, "Unknown command '%s'.\n", argv[1]);
		print_usage();
		return -1;
	}

	for (i = 2; i < argc; ++i) {
		const char* arg = argv[i];
		if (arg[0] == '-' && arg[1] != '\0') {
			const char* valueText;
			if (strcmp(arg, "--") == 0) {
				for (++i; i < argc; ++i) {
					if (options->archivePath == NULL) {
						options->archivePath = argv[i];
					} else if (!pattern_list_push(&options->patterns, argv[i])) {
						fprintf(stderr, "Out of memory.\n");
						return -1;
					}
				}
				break;
			}
			if (strcmp(arg, "--help") == 0) {
				print_usage();
				return 1;
			}
			if (strcmp(arg, "--version") == 0) {
				print_version();
				return 1;
			}
			if (strcmp(arg, "--temp") == 0) {
				if ((i + 1) >= argc) {
					fprintf(stderr, "--temp requires a path.\n");
					return -1;
				}
				options->tempPath = argv[++i];
				continue;
			}
			if (strcmp(arg, "--split-mode") == 0) {
				if ((i + 1) < argc && argv[i + 1][0] != '-') {
					++i;
				}
				fprintf(stderr, "Warning: --split-mode is not supported by the current xpack API and will be ignored.\n");
				continue;
			}

			switch (arg[1]) {
				case 't':
					if (!take_option_value(argc, argv, &i, arg, &valueText)) {
						fprintf(stderr, "-t requires a pack type.\n");
						return -1;
					}
					if (!parse_pack_type_value(valueText, &options->packType)) {
						fprintf(stderr, "Invalid pack type '%s'.\n", valueText);
						return -1;
					}
					options->packTypeSpecified = 1;
					break;

				case 'l':
					if (!take_option_value(argc, argv, &i, arg, &valueText)) {
						fprintf(stderr, "-l requires a compression level.\n");
						return -1;
					}
					if (!parse_uint32_value(valueText, (uint32_t*)&options->compLevel) || options->compLevel < 0 || options->compLevel > 15) {
						fprintf(stderr, "Compression level must be between 0 and 15.\n");
						return -1;
					}
					break;

				case 's':
					if (!take_option_value(argc, argv, &i, arg, &valueText)) {
						fprintf(stderr, "-s requires 0 or 1.\n");
						return -1;
					}
					if (strcmp(valueText, "0") == 0) {
						options->solidMode = 0;
					} else if (strcmp(valueText, "1") == 0) {
						options->solidMode = 1;
					} else {
						fprintf(stderr, "Solid mode must be 0 or 1.\n");
						return -1;
					}
					options->solidModeSpecified = 1;
					break;

				case 'V':
					if (!take_option_value(argc, argv, &i, arg, &valueText)) {
						fprintf(stderr, "-V requires a size.\n");
						return -1;
					}
					if (!parse_volume_size(valueText, &options->volumeSize)) {
						fprintf(stderr, "Invalid volume size '%s'.\n", valueText);
						return -1;
					}
					options->volumeSizeSpecified = 1;
					break;

				case 'o':
					if (!take_option_value(argc, argv, &i, arg, &valueText)) {
						fprintf(stderr, "-o requires a directory path.\n");
						return -1;
					}
					options->outputDir = valueText;
					break;

				case 'w':
					if (!take_option_value(argc, argv, &i, arg, &valueText)) {
						fprintf(stderr, "-w requires a temporary path.\n");
						return -1;
					}
					options->tempPath = valueText;
					break;

				case 'r':
					if (arg[2] != '\0') {
						fprintf(stderr, "Unknown option '%s'.\n", arg);
						return -1;
					}
					options->recursive = 1;
					break;

				case 'v':
					if (arg[2] != '\0') {
						fprintf(stderr, "Unknown option '%s'.\n", arg);
						return -1;
					}
					options->verbose = 1;
					break;

				case 'y':
					if (arg[2] != '\0') {
						fprintf(stderr, "Unknown option '%s'.\n", arg);
						return -1;
					}
					options->autoYes = 1;
					break;

				case 'h':
					print_usage();
					return 1;

				default:
					fprintf(stderr, "Unknown option '%s'.\n", arg);
					return -1;
			}
		} else {
			if (options->archivePath == NULL) {
				options->archivePath = arg;
			} else if (!pattern_list_push(&options->patterns, arg)) {
				fprintf(stderr, "Out of memory.\n");
				return -1;
			}
		}
	}

	if (options->archivePath == NULL) {
		fprintf(stderr, "Archive path is required.\n");
		return -1;
	}
	return 0;
}

static int open_package_for_command(const CliOptions* options, xpkObject* pkgRet)
{
	xpkOpenOptions openOptions;
	int createIfMissing;

	memset(&openOptions, 0, sizeof(openOptions));
	createIfMissing = (options->command == CMD_ADD || options->command == CMD_UPDATE || options->command == CMD_BUILD);
	openOptions.createIfMissing = (uint8_t)(createIfMissing ? 1 : 0);
	openOptions.readonly = (uint8_t)((options->command == CMD_LIST || options->command == CMD_TEST ||
		options->command == CMD_EXTRACT || options->command == CMD_EXTRACT_FLAT || options->command == CMD_INFO) ? 1 : 0);
	openOptions.bufferedDefault = 0;

	*pkgRet = xpkOpen(options->archivePath, &openOptions);
	if (*pkgRet == NULL) {
		fprintf(stderr, "Failed to open '%s': %s\n", options->archivePath, xpkLastErrorMessage(NULL));
		return 1;
	}
	return 0;
}

static int parse_pack_type_value(const char* text, xpkPackType* typeRet)
{
	if (strcmp(text, "core") == 0) {
		*typeRet = XPK_PACK_CORE;
		return 1;
	}
	if (strcmp(text, "index") == 0) {
		*typeRet = XPK_PACK_INDEX;
		return 1;
	}
	if (strcmp(text, "linux") == 0) {
		*typeRet = XPK_PACK_LINUX;
		return 1;
	}
	if (strcmp(text, "win32") == 0) {
		*typeRet = XPK_PACK_WIN32;
		return 1;
	}
	return 0;
}

static int parse_uint32_value(const char* text, uint32_t* valueRet)
{
	char* endText;
	unsigned long long value;

	if (text == NULL || text[0] == '\0') {
		return 0;
	}
	errno = 0;
	value = strtoull(text, &endText, 10);
	if (errno != 0 || endText == text || *endText != '\0' || value > 0xFFFFFFFFull) {
		return 0;
	}
	*valueRet = (uint32_t)value;
	return 1;
}

static int parse_int64_value(const char* text, int64_t* valueRet)
{
	char* endText;
	long long value;

	if (text == NULL || text[0] == '\0') {
		return 0;
	}
	errno = 0;
	value = strtoll(text, &endText, 10);
	if (errno != 0 || endText == text || *endText != '\0') {
		return 0;
	}
	*valueRet = (int64_t)value;
	return 1;
}

static int parse_volume_size(const char* text, uint32_t* valueRet)
{
	char* endText;
	unsigned long long value;
	unsigned long long factor;
	char unit;

	if (text == NULL || text[0] == '\0') {
		return 0;
	}
	errno = 0;
	value = strtoull(text, &endText, 10);
	if (errno != 0 || endText == text) {
		return 0;
	}
	factor = 1;
	if (*endText != '\0') {
		unit = (char)toupper((unsigned char)*endText++);
		if (*endText != '\0') {
			return 0;
		}
		switch (unit) {
			case 'K': factor = 1024ull; break;
			case 'M': factor = 1024ull * 1024ull; break;
			case 'G': factor = 1024ull * 1024ull * 1024ull; break;
			default: return 0;
		}
	}
	value *= factor;
	if (value > 0xFFFFFFFFull) {
		return 0;
	}
	if (value != 0 && value < XPK_VOLUME_MIN) {
		return 0;
	}
	*valueRet = (uint32_t)value;
	return 1;
}

static int command_add_or_update(xpkObject pkg, const CliOptions* options, int updateMode)
{
	InputList inputs;
	xpkPackType packType;

	memset(&inputs, 0, sizeof(inputs));
	if (!find_pack_type(pkg, &packType)) {
		return print_xpk_error("read pack type", options->archivePath, pkg);
	}
	if (!apply_package_settings(pkg, options, xpkCount(pkg) == 0, 0, NULL)) {
		input_list_free(&inputs);
		return print_xpk_error("apply package settings", options->archivePath, pkg);
	}
	if (!find_pack_type(pkg, &packType)) {
		input_list_free(&inputs);
		return print_xpk_error("read pack type", options->archivePath, pkg);
	}
	if (!expand_inputs(options, pkg, packType, updateMode, &inputs)) {
		input_list_free(&inputs);
		return 1;
	}
	if (inputs.count == 0) {
		fprintf(stderr, "No input files to process.\n");
		input_list_free(&inputs);
		return 1;
	}

	{
		size_t i;
		for (i = 0; i < inputs.count; ++i) {
			int ok;
			if (packType == XPK_PACK_WIN32 || packType == XPK_PACK_LINUX) {
				ok = add_or_update_path_file(pkg, options, packType, updateMode, &inputs.items[i]);
			} else if (packType == XPK_PACK_INDEX) {
				ok = add_or_update_index_file(pkg, options, updateMode, &inputs.items[i]);
			} else {
				ok = add_or_update_core_file(pkg, options, updateMode, &inputs.items[i]);
			}
			if (!ok) {
				input_list_free(&inputs);
				return 1;
			}
		}
	}

	if (!apply_package_settings(pkg, options, 0, 1, NULL)) {
		input_list_free(&inputs);
		return print_xpk_error("apply package settings", options->archivePath, pkg);
	}
	input_list_free(&inputs);
	if (!save_or_build_package(pkg, options)) {
		return 1;
	}
	return 0;
}

static int command_extract(xpkObject pkg, const CliOptions* options, int flatMode)
{
	ExtractContext context;
	xpkPackType packType;

	if (!find_pack_type(pkg, &packType)) {
		return print_xpk_error("read pack type", options->archivePath, pkg);
	}
	if (!xrtDirExists((str)options->outputDir) && !xrtDirCreateAll((str)options->outputDir)) {
		fprintf(stderr, "Failed to create output directory '%s'.\n", options->outputDir);
		return 1;
	}

	memset(&context, 0, sizeof(context));
	context.packType = packType;
	context.options = options;
	context.baseDir = options->outputDir;
	context.flatMode = flatMode;

	if (xpkEach(pkg, extract_callback, &context) != XPK_OK) {
		return print_xpk_error("extract entries", options->archivePath, pkg);
	}
	if (context.matchedCount == 0) {
		fprintf(stderr, "No entries matched.\n");
		return 1;
	}

	printf("Extracted %u/%u entr%s.\n",
		context.extracted,
		context.matchedCount,
		(context.matchedCount == 1) ? "y" : "ies");
	return (context.extracted == context.matchedCount) ? 0 : 1;
}

static int command_list(xpkObject pkg, const CliOptions* options)
{
	ListContext context;
	xpkPackType packType;

	if (!find_pack_type(pkg, &packType)) {
		return print_xpk_error("read pack type", options->archivePath, pkg);
	}

	memset(&context, 0, sizeof(context));
	context.packType = packType;
	context.options = options;

	if (packType == XPK_PACK_INDEX) {
		printf("%-6s %-14s %-14s %-14s %-8s %-10s %s\n", "Pos", "Index", "Size", "Packed", "Ratio", "Method", "Name");
	} else {
		printf("%-6s %-14s %-14s %-8s %-10s %s\n", "Pos", "Size", "Packed", "Ratio", "Method", (packType == XPK_PACK_CORE) ? "Name" : "Path");
	}
	if (xpkEach(pkg, list_callback, &context) != XPK_OK) {
		return print_xpk_error("list entries", options->archivePath, pkg);
	}
	if (context.matchedCount == 0) {
		fprintf(stderr, "No entries matched.\n");
		return 1;
	}

	printf("\nEntries: %u\n", context.matchedCount);
	printf("Total size: %" PRIu64 "\n", context.totalSize);
	printf("Total packed: %" PRIu64 "\n", context.totalPacked);
	return 0;
}

static int command_test(xpkObject pkg, const CliOptions* options)
{
	TestContext context;
	xpkPackType packType;

	if (!find_pack_type(pkg, &packType)) {
		return print_xpk_error("read pack type", options->archivePath, pkg);
	}

	memset(&context, 0, sizeof(context));
	context.packType = packType;
	context.options = options;

	if (xpkEach(pkg, test_callback, &context) != XPK_OK) {
		return print_xpk_error("test entries", options->archivePath, pkg);
	}
	if (context.matchedCount == 0) {
		fprintf(stderr, "No entries matched.\n");
		return 1;
	}

	printf("Tested %u entr%s: %u passed, %u failed.\n",
		context.matchedCount,
		(context.matchedCount == 1) ? "y" : "ies",
		context.passed,
		context.failed);
	return (context.failed == 0) ? 0 : 1;
}

static int command_delete(xpkObject pkg, const CliOptions* options)
{
	CollectMatchesContext context;
	MatchList matches;
	xpkPackType packType;

	if (!find_pack_type(pkg, &packType)) {
		return print_xpk_error("read pack type", options->archivePath, pkg);
	}
	if (options->patterns.count == 0) {
		fprintf(stderr, "Delete requires at least one pattern.\n");
		return 1;
	}

	memset(&matches, 0, sizeof(matches));
	memset(&context, 0, sizeof(context));
	context.packType = packType;
	context.options = options;
	context.matches = &matches;

	if (xpkEach(pkg, collect_matches_callback, &context) != XPK_OK) {
		match_list_free(&matches);
		return print_xpk_error("collect delete targets", options->archivePath, pkg);
	}
	if (matches.count == 0) {
		fprintf(stderr, "No entries matched.\n");
		match_list_free(&matches);
		return 1;
	}
	if (!options->autoYes) {
		char line[16];
		printf("Delete %u entr%s from '%s'? [y/N] ",
			(unsigned)matches.count,
			(matches.count == 1) ? "y" : "ies",
			options->archivePath);
		fflush(stdout);
		if (fgets(line, sizeof(line), stdin) == NULL || (line[0] != 'y' && line[0] != 'Y')) {
			printf("Cancelled.\n");
			match_list_free(&matches);
			return 1;
		}
	}

	match_list_sort_desc(&matches);
	{
		size_t i;
		for (i = 0; i < matches.count; ++i) {
			if (xpkRemove(pkg, matches.items[i].pos) != XPK_OK) {
				match_list_free(&matches);
				return print_xpk_error("delete entry", options->archivePath, pkg);
			}
			if (options->verbose) {
				printf("Deleted: %s\n", matches.items[i].keyText);
			}
		}
	}

	match_list_free(&matches);
	if (!save_or_build_package(pkg, options)) {
		return 1;
	}
	return 0;
}

static int command_info(xpkObject pkg, const CliOptions* options)
{
	xpkPackType packType;
	xpkStat statInfo;
	uint8_t level;
	int solidMode;
	uint32_t volumeSize;
	uint64_t physicalSize;

	if (!find_pack_type(pkg, &packType)) {
		return print_xpk_error("read pack type", options->archivePath, pkg);
	}
	if (xpkStatGet(pkg, &statInfo) != XPK_OK) {
		return print_xpk_error("read archive stats", options->archivePath, pkg);
	}

	printf("Archive: %s\n", options->archivePath);
	printf("Pack type: %s\n", pack_type_name(packType));
	printf("Entries: %u\n", statInfo.fileCount);
	if (xpkGetDefaultComp(pkg, &level) == XPK_OK) {
		printf("Default comp: %u (%s)\n", level, method_name_from_level(level));
	}
	if (xpkGetMetaComp(pkg, &level) == XPK_OK) {
		printf("Meta comp: %u (%s)\n", level, method_name_from_level(level));
	}
	if (xpkGetInfoComp(pkg, &level) == XPK_OK) {
		printf("Info comp: %u (%s)\n", level, method_name_from_level(level));
	}
	if (xpkGetSolidMode(pkg, &solidMode) == XPK_OK) {
		printf("Solid mode: %s\n", solidMode ? "on" : "off");
	}
	if (xpkGetVolumeSize(pkg, &volumeSize) == XPK_OK) {
		printf("Volume size: %s", (volumeSize == 0) ? "off" : "");
		if (volumeSize != 0) {
			printf("%u", volumeSize);
		}
		printf("\n");
	}
	printf("Live data: %" PRIu64 "\n", statInfo.liveDataBytes);
	printf("Holes: %" PRIu64 "\n", statInfo.holeBytes);
	printf("Metadata bytes: %" PRIu64 "\n", statInfo.metaBytes);
	printf("Entry table bytes: %" PRIu64 "\n", statInfo.entryTableBytes);
	physicalSize = (uint64_t)xrtFileGetSize((str)options->archivePath);
	if (physicalSize > 0) {
		printf("Physical file size: %" PRIu64 "\n", physicalSize);
	}
	return 0;
}

static int command_build(xpkObject pkg, const CliOptions* options)
{
	xpkBuildOptions buildOptions;

	if (!apply_package_settings(pkg, options, xpkCount(pkg) == 0, 1, NULL)) {
		return print_xpk_error("apply package settings", options->archivePath, pkg);
	}
	memset(&buildOptions, 0, sizeof(buildOptions));
	buildOptions.replaceOriginal = 1;
	buildOptions.tempPath = options->tempPath;
	if (xpkBuild(pkg, &buildOptions) != XPK_OK) {
		return print_xpk_error("build archive", options->archivePath, pkg);
	}
	if (options->verbose) {
		printf("Rebuilt: %s\n", options->archivePath);
	}
	return 0;
}

static int apply_package_settings(xpkObject pkg, const CliOptions* options, int allowPackTypeChange, int applyLayout, int* layoutHintRet)
{
	xpkPackType currentType;
	int solidBefore;
	uint32_t volumeBefore;

	if (layoutHintRet != NULL) {
		*layoutHintRet = 0;
	}
	if (!find_pack_type(pkg, &currentType)) {
		return 0;
	}

	if (xpkCount(pkg) == 0 || options->packTypeSpecified) {
		xpkPackType targetType = options->packType;
		if (xpkCount(pkg) == 0 || allowPackTypeChange) {
			if (xpkSetPackType(pkg, targetType) != XPK_OK) {
				return 0;
			}
			currentType = targetType;
		} else if (currentType != targetType) {
			fprintf(stderr, "Warning: archive already uses pack type '%s'; ignoring requested '%s'.\n",
				pack_type_name(currentType),
				pack_type_name(targetType));
		}
	}

	if (!applyLayout) {
		return 1;
	}

	if (options->solidModeSpecified) {
		if (xpkGetSolidMode(pkg, &solidBefore) != XPK_OK) {
			return 0;
		}
		if (solidBefore != options->solidMode) {
			if (xpkSetSolidMode(pkg, options->solidMode) != XPK_OK) {
				return 0;
			}
			if (layoutHintRet != NULL) {
				*layoutHintRet = 1;
			}
		}
	}

	if (options->volumeSizeSpecified) {
		if (xpkGetVolumeSize(pkg, &volumeBefore) != XPK_OK) {
			return 0;
		}
		if (volumeBefore != options->volumeSize) {
			if (xpkSetVolumeSize(pkg, options->volumeSize) != XPK_OK) {
				return 0;
			}
			if (layoutHintRet != NULL) {
				*layoutHintRet = 1;
			}
		}
	}

	return 1;
}

static int save_or_build_package(xpkObject pkg, const CliOptions* options)
{
	xpkBuildOptions buildOptions;
	int ret;

	ret = xpkSave(pkg);
	if (ret == XPK_OK) {
		return 1;
	}
	if (ret != XPK_ERR_STATE || !is_build_required_message(xpkLastErrorMessage(pkg))) {
		print_xpk_error("save archive", options->archivePath, pkg);
		return 0;
	}

	memset(&buildOptions, 0, sizeof(buildOptions));
	buildOptions.replaceOriginal = 1;
	buildOptions.tempPath = options->tempPath;
	if (options->verbose) {
		printf("Layout change requires rebuild, running xpkBuild...\n");
	}
	if (xpkBuild(pkg, &buildOptions) != XPK_OK) {
		print_xpk_error("build archive", options->archivePath, pkg);
		return 0;
	}
	return 1;
}

static int is_build_required_message(const char* text)
{
	if (text == NULL) {
		return 0;
	}
	return (strstr(text, "requires xpkBuild") != NULL) ? 1 : 0;
}

static int find_pack_type(xpkObject pkg, xpkPackType* typeRet)
{
	return (xpkGetPackType(pkg, typeRet) == XPK_OK) ? 1 : 0;
}

static const char* pack_type_name(xpkPackType type)
{
	switch (type) {
		case XPK_PACK_CORE: return "core";
		case XPK_PACK_INDEX: return "index";
		case XPK_PACK_LINUX: return "linux";
		case XPK_PACK_WIN32: return "win32";
		default: return "unknown";
	}
}

static const char* method_name_from_level(uint32_t level)
{
	if (level == 0) {
		return "STORE";
	}
	if (level <= 2) {
		return "LZ4";
	}
	if (level <= 4) {
		return "LZ4HC";
	}
	if (level <= 13) {
		return "ZSTD";
	}
	return "LZMA2";
}

static int collect_matches_callback(xpkObject pkg, uint32_t pos, const void* info, void* userData)
{
	CollectMatchesContext* context;
	EntryKeyView view;
	(void)pkg;

	context = (CollectMatchesContext*)userData;
	fill_entry_key_view(context->packType, pos, info, &view);
	if (!entry_matches_patterns(context->options, &view)) {
		return XPK_OK;
	}
	if (!match_list_push(context->matches, pos, view.key)) {
		return XPK_ERR_MEMORY;
	}
	return XPK_OK;
}

static int list_callback(xpkObject pkg, uint32_t pos, const void* info, void* userData)
{
	ListContext* context;
	EntryKeyView view;
	double ratio;
	(void)pkg;

	context = (ListContext*)userData;
	fill_entry_key_view(context->packType, pos, info, &view);
	if (!entry_matches_patterns(context->options, &view)) {
		return XPK_OK;
	}

	ratio = (view.fileSize == 0) ? 0.0 : ((double)view.packedSize * 100.0 / (double)view.fileSize);
	if (context->packType == XPK_PACK_INDEX) {
		printf("%-6u %-14" PRId64 " %-14" PRIu64 " %-14" PRIu64 " %7.1f%% %-10s %s\n",
			pos,
			view.indexValue,
			view.fileSize,
			view.packedSize,
			ratio,
			method_name_from_level(view.flag & XPK_FLAG_COMP_MASK),
			view.key);
	} else {
		printf("%-6u %-14" PRIu64 " %-14" PRIu64 " %7.1f%% %-10s %s\n",
			pos,
			view.fileSize,
			view.packedSize,
			ratio,
			method_name_from_level(view.flag & XPK_FLAG_COMP_MASK),
			view.key);
	}
	context->matchedCount++;
	context->totalSize += view.fileSize;
	context->totalPacked += view.packedSize;
	return XPK_OK;
}

static int test_callback(xpkObject pkg, uint32_t pos, const void* info, void* userData)
{
	TestContext* context;
	EntryKeyView view;
	int ret;

	context = (TestContext*)userData;
	fill_entry_key_view(context->packType, pos, info, &view);
	if (!entry_matches_patterns(context->options, &view)) {
		return XPK_OK;
	}

	context->matchedCount++;
	ret = xpkVerify(pkg, pos);
	if (ret == XPK_OK) {
		context->passed++;
		if (context->options->verbose) {
			printf("OK: %s\n", view.key);
		}
	} else {
		context->failed++;
		fprintf(stderr, "FAILED: %s: %s\n", view.key, xpkLastErrorMessage(pkg));
	}
	return XPK_OK;
}

static int extract_callback(xpkObject pkg, uint32_t pos, const void* info, void* userData)
{
	ExtractContext* context;
	EntryKeyView view;
	char* relativePath;
	char* outputPath;

	context = (ExtractContext*)userData;
	fill_entry_key_view(context->packType, pos, info, &view);
	if (!entry_matches_patterns(context->options, &view)) {
		return XPK_OK;
	}

	context->matchedCount++;
	if (context->flatMode) {
		relativePath = dup_generated_extract_name(context->packType, pos, &view);
		if ((context->packType == XPK_PACK_WIN32 || context->packType == XPK_PACK_LINUX) && view.key != NULL) {
			free(relativePath);
			relativePath = dup_string(path_basename_ptr(view.key));
		}
	} else if (context->packType == XPK_PACK_WIN32 || context->packType == XPK_PACK_LINUX) {
		relativePath = dup_safe_relative_path(view.key);
	} else {
		relativePath = dup_generated_extract_name(context->packType, pos, &view);
	}
	if (relativePath == NULL || relativePath[0] == '\0') {
		free(relativePath);
		fprintf(stderr, "Skipped unsafe entry path: %s\n", view.key ? view.key : "<null>");
		return XPK_OK;
	}

	outputPath = dup_output_full_path(context->baseDir, relativePath);
	free(relativePath);
	if (outputPath == NULL) {
		return XPK_ERR_MEMORY;
	}
	if (!ensure_parent_dir(outputPath)) {
		fprintf(stderr, "Failed to create parent directory for '%s'.\n", outputPath);
		free(outputPath);
		return XPK_OK;
	}
	if (xpkReadToFile(pkg, pos, outputPath) != XPK_OK) {
		fprintf(stderr, "Failed to extract '%s': %s\n", view.key, xpkLastErrorMessage(pkg));
		free(outputPath);
		return XPK_OK;
	}
	context->extracted++;
	if (context->options->verbose) {
		printf("Extracted: %s\n", outputPath);
	}
	free(outputPath);
	return XPK_OK;
}

static int index_max_callback(xpkObject pkg, uint32_t pos, const void* info, void* userData)
{
	const xpkFileInfoIndex* indexInfo;
	IndexMaxContext* context;
	(void)pkg;
	(void)pos;

	indexInfo = (const xpkFileInfoIndex*)info;
	context = (IndexMaxContext*)userData;
	if (!context->found || indexInfo->fileIndex > context->maxIndex) {
		context->found = 1;
		context->maxIndex = indexInfo->fileIndex;
	}
	return XPK_OK;
}

static int dir_expand_callback(void* pathText, size_t pathSize, int dirKind, void* fileData, void* userData)
{
	ExpandScanContext* context;
	const char* path;
	const char* suffix;
	char* pathKey;
	int64_t indexValue;
	(void)pathSize;
	(void)fileData;

	context = (ExpandScanContext*)userData;
	if (dirKind != 0) {
		return 0;
	}
	path = (const char*)pathText;

	if (context->packType == XPK_PACK_WIN32 || context->packType == XPK_PACK_LINUX) {
		suffix = path + strlen(context->scanRoot);
		while (*suffix == '/' || *suffix == '\\') {
			++suffix;
		}
		if (context->pathPrefix != NULL && context->pathPrefix[0] != '\0') {
			pathKey = dup_stored_join(context->pathPrefix, suffix);
		} else {
			pathKey = dup_string(path);
		}
		if (pathKey == NULL || !append_input_item_file(context->inputs, path, pathKey, 0, 0, 0, 0)) {
			free(pathKey);
			context->hadError = 1;
			return 1;
		}
		free(pathKey);
		return 0;
	}

	if (context->command == CMD_UPDATE && context->packType == XPK_PACK_CORE) {
		fprintf(stderr, "Core update requires explicit position mapping, directory input is not supported.\n");
		context->hadError = 1;
		return 1;
	}

	if (context->packType == XPK_PACK_INDEX) {
		indexValue = (*context->nextIndex)++;
		if (!append_input_item_file(context->inputs, path, NULL, indexValue, 1, 0, 0)) {
			context->hadError = 1;
			return 1;
		}
		return 0;
	}

	if (!append_input_item_file(context->inputs, path, NULL, 0, 0, 0, 0)) {
		context->hadError = 1;
		return 1;
	}
	return 0;
}

static void fill_entry_key_view(xpkPackType packType, uint32_t pos, const void* info, EntryKeyView* view)
{
	memset(view, 0, sizeof(*view));
	view->ignoreCase = (packType == XPK_PACK_WIN32) ? 1 : 0;

	if (packType == XPK_PACK_INDEX) {
		const xpkFileInfoIndex* indexInfo = (const xpkFileInfoIndex*)info;
		snprintf(view->buffer, sizeof(view->buffer), "%" PRId64, indexInfo->fileIndex);
		view->key = view->buffer;
		view->fileSize = indexInfo->fileSize;
		view->packedSize = indexInfo->dataSize;
		view->flag = indexInfo->flag;
		view->indexValue = indexInfo->fileIndex;
		return;
	}
	if (packType == XPK_PACK_WIN32 || packType == XPK_PACK_LINUX) {
		const xpkFileInfoPath* pathInfo = (const xpkFileInfoPath*)info;
		view->key = pathInfo->pathBytes;
		view->fileSize = pathInfo->fileSize;
		view->packedSize = pathInfo->dataSize;
		view->flag = pathInfo->flag;
		return;
	}

	{
		const xpkFileInfo* fileInfo = (const xpkFileInfo*)info;
		snprintf(view->buffer, sizeof(view->buffer), "%u", pos);
		view->key = view->buffer;
		view->fileSize = fileInfo->fileSize;
		view->packedSize = fileInfo->dataSize;
		view->flag = fileInfo->flag;
	}
}

static int entry_matches_patterns(const CliOptions* options, const EntryKeyView* view)
{
	size_t i;

	if (options->patterns.count == 0) {
		return 1;
	}
	for (i = 0; i < options->patterns.count; ++i) {
		if (xrtStrLike((str)view->key, 0, (str)options->patterns.items[i], 0, view->ignoreCase) != 0) {
			return 1;
		}
	}
	return 0;
}

static int expand_inputs(const CliOptions* options, xpkObject pkg, xpkPackType packType, int updateMode, InputList* inputs)
{
	int64_t nextIndex;
	IndexMaxContext indexContext;
	size_t i;

	nextIndex = 1;
	if (packType == XPK_PACK_INDEX) {
		memset(&indexContext, 0, sizeof(indexContext));
		if (xpkEach(pkg, index_max_callback, &indexContext) != XPK_OK) {
			print_xpk_error("scan existing indices", options->archivePath, pkg);
			return 0;
		}
		if (indexContext.found) {
			nextIndex = indexContext.maxIndex + 1;
		}
	}

	for (i = 0; i < options->patterns.count; ++i) {
		if (!expand_one_input(options, packType, updateMode, options->patterns.items[i], &nextIndex, inputs)) {
			return 0;
		}
	}
	return 1;
}

static int expand_one_input(const CliOptions* options, xpkPackType packType, int updateMode, const char* rawSpec, int64_t* nextIndex, InputList* inputs)
{
	const char* delimiter;
	char* leftText;
	char* rightText;
	char* scanRoot;
	char* pathKey;
	const char* sourcePath;
	const char* basename;
	ExpandScanContext scanContext;

	delimiter = strstr(rawSpec, "::");
	leftText = NULL;
	rightText = NULL;
	scanRoot = NULL;
	pathKey = NULL;
	memset(&scanContext, 0, sizeof(scanContext));

	if (delimiter != NULL) {
		leftText = dup_string_n(rawSpec, (size_t)(delimiter - rawSpec));
		rightText = dup_string(delimiter + 2);
		if (leftText == NULL || rightText == NULL || leftText[0] == '\0' || rightText[0] == '\0') {
			fprintf(stderr, "Invalid mapping '%s'.\n", rawSpec);
			free(leftText);
			free(rightText);
			return 0;
		}
		sourcePath = rightText;
	} else {
		sourcePath = rawSpec;
	}

	if (xrtFileExists((str)sourcePath)) {
		if (packType == XPK_PACK_WIN32 || packType == XPK_PACK_LINUX) {
			if (leftText != NULL) {
				pathKey = dup_string(leftText);
			} else if (path_is_abs_fs(sourcePath)) {
				basename = path_basename_ptr(sourcePath);
				pathKey = dup_string((basename != NULL && basename[0] != '\0') ? basename : "file");
			} else {
				pathKey = dup_string(sourcePath);
			}
			if (pathKey == NULL || !append_input_item_file(inputs, sourcePath, pathKey, 0, 0, 0, 0)) {
				free(pathKey);
				free(leftText);
				free(rightText);
				return 0;
			}
			free(pathKey);
		} else if (packType == XPK_PACK_INDEX) {
			int64_t indexValue;
			if (leftText != NULL) {
				if (!parse_int64_value(leftText, &indexValue)) {
					fprintf(stderr, "Invalid index mapping '%s'.\n", rawSpec);
					free(leftText);
					free(rightText);
					return 0;
				}
			} else {
				indexValue = (*nextIndex)++;
			}
			if (!append_input_item_file(inputs, sourcePath, NULL, indexValue, 1, 0, 0)) {
				free(leftText);
				free(rightText);
				return 0;
			}
		} else {
			uint32_t posValue;
			if (leftText != NULL) {
				if (!parse_uint32_value(leftText, &posValue) || posValue == 0) {
					fprintf(stderr, "Invalid core position mapping '%s'.\n", rawSpec);
					free(leftText);
					free(rightText);
					return 0;
				}
				if (!append_input_item_file(inputs, sourcePath, NULL, 0, 0, posValue, 1)) {
					free(leftText);
					free(rightText);
					return 0;
				}
			} else if (!updateMode) {
				if (!append_input_item_file(inputs, sourcePath, NULL, 0, 0, 0, 0)) {
					free(leftText);
					free(rightText);
					return 0;
				}
			} else {
				fprintf(stderr, "Core update requires explicit position mapping: <pos>::<file>\n");
				free(leftText);
				free(rightText);
				return 0;
			}
		}

		free(leftText);
		free(rightText);
		return 1;
	}

	if (!xrtDirExists((str)sourcePath)) {
		fprintf(stderr, "Input not found: %s\n", sourcePath);
		free(leftText);
		free(rightText);
		return 0;
	}
	if (!options->recursive) {
		fprintf(stderr, "Skipping directory '%s' (use -r to recurse).\n", sourcePath);
		free(leftText);
		free(rightText);
		return 1;
	}
	if ((leftText != NULL) && !(packType == XPK_PACK_WIN32 || packType == XPK_PACK_LINUX)) {
		fprintf(stderr, "Directory mapping is only supported for path packs.\n");
		free(leftText);
		free(rightText);
		return 0;
	}

	scanRoot = dup_trim_trailing_seps(sourcePath);
	if (scanRoot == NULL) {
		free(leftText);
		free(rightText);
		return 0;
	}
	if (packType == XPK_PACK_WIN32 || packType == XPK_PACK_LINUX) {
		if (leftText != NULL) {
			scanContext.pathPrefix = leftText;
		} else if (path_is_abs_fs(sourcePath)) {
			basename = path_basename_ptr(scanRoot);
			scanContext.pathPrefix = (basename != NULL && basename[0] != '\0') ? basename : NULL;
		} else {
			scanContext.pathPrefix = NULL;
		}
	}

	scanContext.packType = packType;
	scanContext.command = updateMode ? CMD_UPDATE : CMD_ADD;
	scanContext.scanRoot = scanRoot;
	scanContext.inputs = inputs;
	scanContext.nextIndex = nextIndex;
	scanContext.hadError = 0;

	(void)xrtDirScan((str)scanRoot, TRUE, dir_expand_callback, &scanContext);
	free(scanRoot);
	free(leftText);
	free(rightText);
	return scanContext.hadError ? 0 : 1;
}

static int add_or_update_path_file(xpkObject pkg, const CliOptions* options, xpkPackType packType, int updateMode, const InputItem* item)
{
	xpkWriteOptions writeOptions;
	int exists;
	int ret;
	const char* keyText;

	memset(&writeOptions, 0, sizeof(writeOptions));
	writeOptions.compLevel = (uint8_t)options->compLevel;
	writeOptions.writePolicy = XPK_WRITE_IMMEDIATE;
	writeOptions.fileType = (packType == XPK_PACK_LINUX) ? XPK_TYPE_LINUX : XPK_TYPE_WIN32;

	keyText = (item->pathKey != NULL) ? item->pathKey : item->sourcePath;
	exists = xpkPathExists(pkg, keyText);
	ret = xpkLastError(pkg);
	if (!exists && ret != XPK_OK) {
		print_xpk_error("query entry", keyText, pkg);
		return 0;
	}

	if (updateMode && exists) {
		if (xpkPathUpdateFile(pkg, keyText, item->sourcePath, &writeOptions) != XPK_OK) {
			print_xpk_error("update entry", keyText, pkg);
			return 0;
		}
		if (options->verbose) {
			printf("Updated: %s <- %s\n", keyText, item->sourcePath);
		}
		return 1;
	}

	if (exists) {
		fprintf(stderr, "Entry already exists: %s\n", keyText);
		return 0;
	}
	if (xpkPathAddFile(pkg, keyText, item->sourcePath, &writeOptions) != XPK_OK) {
		print_xpk_error("add entry", keyText, pkg);
		return 0;
	}
	if (options->verbose) {
		printf("Added: %s <- %s\n", keyText, item->sourcePath);
	}
	return 1;
}

static int add_or_update_index_file(xpkObject pkg, const CliOptions* options, int updateMode, const InputItem* item)
{
	xpkWriteOptions writeOptions;
	int ret;

	memset(&writeOptions, 0, sizeof(writeOptions));
	writeOptions.compLevel = (uint8_t)options->compLevel;
	writeOptions.writePolicy = XPK_WRITE_IMMEDIATE;
	writeOptions.fileType = XPK_TYPE_INDEX;

	if (updateMode) {
		ret = xpkIndexFind(pkg, item->indexKey, NULL);
		if (ret == XPK_OK) {
			if (xpkIndexUpdateFile(pkg, item->indexKey, item->sourcePath, &writeOptions) != XPK_OK) {
				print_xpk_error("update index entry", item->sourcePath, pkg);
				return 0;
			}
			if (options->verbose) {
				printf("Updated: %" PRId64 " <- %s\n", item->indexKey, item->sourcePath);
			}
			return 1;
		}
		if (ret != XPK_ERR_NOT_FOUND) {
			print_xpk_error("query index entry", item->sourcePath, pkg);
			return 0;
		}
	}

	if (xpkIndexAddFile(pkg, item->indexKey, item->sourcePath, &writeOptions) != XPK_OK) {
		print_xpk_error("add index entry", item->sourcePath, pkg);
		return 0;
	}
	if (options->verbose) {
		printf("Added: %" PRId64 " <- %s\n", item->indexKey, item->sourcePath);
	}
	return 1;
}

static int add_or_update_core_file(xpkObject pkg, const CliOptions* options, int updateMode, const InputItem* item)
{
	xpkWriteOptions writeOptions;
	uint32_t outPos;

	memset(&writeOptions, 0, sizeof(writeOptions));
	writeOptions.compLevel = (uint8_t)options->compLevel;
	writeOptions.writePolicy = XPK_WRITE_IMMEDIATE;
	writeOptions.fileType = XPK_TYPE_CORE;

	if (updateMode) {
		if (!item->hasPosKey) {
			fprintf(stderr, "Core update requires explicit position mapping.\n");
			return 0;
		}
		if (xpkUpdateFile(pkg, item->posKey, item->sourcePath, &writeOptions) != XPK_OK) {
			print_xpk_error("update core entry", item->sourcePath, pkg);
			return 0;
		}
		if (options->verbose) {
			printf("Updated: %u <- %s\n", item->posKey, item->sourcePath);
		}
		return 1;
	}

	outPos = 0;
	if (xpkAddFile(pkg, item->sourcePath, &writeOptions, &outPos) != XPK_OK) {
		print_xpk_error("add core entry", item->sourcePath, pkg);
		return 0;
	}
	if (options->verbose) {
		printf("Added: %u <- %s\n", outPos, item->sourcePath);
	}
	return 1;
}

static int pattern_list_push(PatternList* list, const char* value)
{
	const char** newItems;
	size_t newCapacity;

	if (list->count == list->capacity) {
		newCapacity = (list->capacity == 0) ? 8 : (list->capacity * 2);
		newItems = (const char**)realloc(list->items, newCapacity * sizeof(newItems[0]));
		if (newItems == NULL) {
			return 0;
		}
		list->items = newItems;
		list->capacity = newCapacity;
	}
	list->items[list->count++] = value;
	return 1;
}

static void pattern_list_free(PatternList* list)
{
	free(list->items);
	list->items = NULL;
	list->count = 0;
	list->capacity = 0;
}

static int input_list_push(InputList* list, const InputItem* item)
{
	InputItem* newItems;
	size_t newCapacity;

	if (list->count == list->capacity) {
		newCapacity = (list->capacity == 0) ? 16 : (list->capacity * 2);
		newItems = (InputItem*)realloc(list->items, newCapacity * sizeof(newItems[0]));
		if (newItems == NULL) {
			return 0;
		}
		list->items = newItems;
		list->capacity = newCapacity;
	}
	list->items[list->count++] = *item;
	return 1;
}

static void input_item_clear(InputItem* item)
{
	if (item == NULL) {
		return;
	}
	free(item->sourcePath);
	free(item->pathKey);
	memset(item, 0, sizeof(*item));
}

static void input_list_free(InputList* list)
{
	size_t i;
	for (i = 0; i < list->count; ++i) {
		input_item_clear(&list->items[i]);
	}
	free(list->items);
	list->items = NULL;
	list->count = 0;
	list->capacity = 0;
}

static int match_list_push(MatchList* list, uint32_t pos, const char* keyText)
{
	MatchItem* newItems;
	size_t newCapacity;
	char* keyCopy;

	keyCopy = dup_string(keyText);
	if (keyCopy == NULL) {
		return 0;
	}
	if (list->count == list->capacity) {
		newCapacity = (list->capacity == 0) ? 16 : (list->capacity * 2);
		newItems = (MatchItem*)realloc(list->items, newCapacity * sizeof(newItems[0]));
		if (newItems == NULL) {
			free(keyCopy);
			return 0;
		}
		list->items = newItems;
		list->capacity = newCapacity;
	}
	list->items[list->count].pos = pos;
	list->items[list->count].keyText = keyCopy;
	list->count++;
	return 1;
}

static void match_list_free(MatchList* list)
{
	size_t i;
	for (i = 0; i < list->count; ++i) {
		free(list->items[i].keyText);
	}
	free(list->items);
	list->items = NULL;
	list->count = 0;
	list->capacity = 0;
}

static int match_list_sort_desc(MatchList* list)
{
	qsort(list->items, list->count, sizeof(list->items[0]), compare_uint32_desc);
	return 1;
}

static int append_input_item_file(InputList* inputs, const char* sourcePath, const char* pathKey, int64_t indexKey, int hasIndexKey, uint32_t posKey, int hasPosKey)
{
	InputItem item;

	memset(&item, 0, sizeof(item));
	item.sourcePath = dup_string(sourcePath);
	item.pathKey = (pathKey != NULL) ? dup_string(pathKey) : NULL;
	item.indexKey = indexKey;
	item.posKey = posKey;
	item.hasPathKey = (pathKey != NULL) ? 1 : 0;
	item.hasIndexKey = hasIndexKey;
	item.hasPosKey = hasPosKey;
	if (item.sourcePath == NULL || (pathKey != NULL && item.pathKey == NULL)) {
		input_item_clear(&item);
		return 0;
	}
	if (!input_list_push(inputs, &item)) {
		input_item_clear(&item);
		return 0;
	}
	return 1;
}

static char* dup_string(const char* text)
{
	size_t size;
	char* copy;

	if (text == NULL) {
		return NULL;
	}
	size = strlen(text) + 1;
	copy = (char*)malloc(size);
	if (copy == NULL) {
		return NULL;
	}
	memcpy(copy, text, size);
	return copy;
}

static char* dup_string_n(const char* text, size_t size)
{
	char* copy;

	copy = (char*)malloc(size + 1);
	if (copy == NULL) {
		return NULL;
	}
	memcpy(copy, text, size);
	copy[size] = '\0';
	return copy;
}

static char* dup_trim_trailing_seps(const char* path)
{
	size_t size;
	char* copy;

	if (path == NULL) {
		return NULL;
	}
	copy = dup_string(path);
	if (copy == NULL) {
		return NULL;
	}
	size = strlen(copy);
	while (size > 1 && path_is_sep(copy[size - 1])) {
		if (size == 3 && isalpha((unsigned char)copy[0]) && copy[1] == ':' && path_is_sep(copy[2])) {
			break;
		}
		copy[--size] = '\0';
	}
	return copy;
}

static char* dup_generated_extract_name(xpkPackType packType, uint32_t pos, const EntryKeyView* view)
{
	char buffer[128];

	if (packType == XPK_PACK_INDEX) {
		snprintf(buffer, sizeof(buffer), "index_%" PRId64 ".bin", view->indexValue);
	} else {
		snprintf(buffer, sizeof(buffer), "entry_%u.bin", pos);
	}
	return dup_string(buffer);
}

static char* dup_output_full_path(const char* baseDir, const char* relativePath)
{
	size_t baseSize;
	size_t relSize;
	char* path;
	int needSep;

	baseSize = strlen(baseDir);
	relSize = strlen(relativePath);
	needSep = (baseSize != 0 && !path_is_sep(baseDir[baseSize - 1])) ? 1 : 0;
	path = (char*)malloc(baseSize + needSep + relSize + 1);
	if (path == NULL) {
		return NULL;
	}
	memcpy(path, baseDir, baseSize);
	if (needSep) {
		path[baseSize++] = '/';
	}
	memcpy(path + baseSize, relativePath, relSize);
	path[baseSize + relSize] = '\0';
	return path;
}

static char* dup_safe_relative_path(const char* storedPath)
{
	size_t size;
	size_t i;
	size_t outSize;
	char* out;

	if (storedPath == NULL || storedPath[0] == '\0') {
		return NULL;
	}
	if (path_is_abs_fs(storedPath) || storedPath[0] == '/' || storedPath[0] == '\\') {
		return NULL;
	}

	size = strlen(storedPath);
	out = (char*)malloc(size + 1);
	if (out == NULL) {
		return NULL;
	}

	i = 0;
	outSize = 0;
	while (i < size) {
		size_t start;
		size_t len;
		while (i < size && path_is_sep(storedPath[i])) {
			++i;
		}
		start = i;
		while (i < size && !path_is_sep(storedPath[i])) {
			++i;
		}
		len = i - start;
		if (len == 0) {
			continue;
		}
		if (len == 1 && storedPath[start] == '.') {
			continue;
		}
		if (len == 2 && storedPath[start] == '.' && storedPath[start + 1] == '.') {
			free(out);
			return NULL;
		}
		if (outSize != 0) {
			out[outSize++] = '/';
		}
		memcpy(out + outSize, storedPath + start, len);
		outSize += len;
	}

	if (outSize == 0) {
		free(out);
		return NULL;
	}
	out[outSize] = '\0';
	return out;
}

static char* dup_stored_join(const char* left, const char* right)
{
	size_t leftSize;
	char* out;
	size_t outSize;

	if (left == NULL || left[0] == '\0') {
		return dup_string(right);
	}
	if (right == NULL || right[0] == '\0') {
		return dup_string(left);
	}

	leftSize = strlen(left);
	out = (char*)malloc(leftSize + strlen(right) + 2);
	if (out == NULL) {
		return NULL;
	}

	outSize = 0;
	memcpy(out + outSize, left, leftSize);
	outSize += leftSize;
	while (outSize > 0 && path_is_sep(out[outSize - 1])) {
		--outSize;
	}
	out[outSize++] = '/';
	while (*right != '\0' && path_is_sep(*right)) {
		++right;
	}
	memcpy(out + outSize, right, strlen(right) + 1);
	return out;
}

static const char* path_basename_ptr(const char* path)
{
	const char* slash;
	const char* backslash;
	const char* result;

	if (path == NULL) {
		return "";
	}
	slash = strrchr(path, '/');
	backslash = strrchr(path, '\\');
	result = path;
	if (slash != NULL && slash[1] != '\0') {
		result = slash + 1;
	}
	if (backslash != NULL && backslash[1] != '\0' && backslash + 1 > result) {
		result = backslash + 1;
	}
	return result;
}

static int path_is_abs_fs(const char* path)
{
	if (path == NULL || path[0] == '\0') {
		return 0;
	}
#if defined(_WIN32) || defined(_WIN64)
	if ((isalpha((unsigned char)path[0]) && path[1] == ':') ||
		(path[0] == '\\' && path[1] == '\\') ||
		(path[0] == '/' && path[1] == '/')) {
		return 1;
	}
	return 0;
#else
	return (path[0] == '/') ? 1 : 0;
#endif
}

static int path_is_sep(char ch)
{
	return (ch == '/' || ch == '\\') ? 1 : 0;
}

static int ensure_parent_dir(const char* path)
{
	const char* slash;
	const char* backslash;
	const char* cut;
	char* dirPath;
	size_t size;

	slash = strrchr(path, '/');
	backslash = strrchr(path, '\\');
	cut = slash;
	if (backslash != NULL && (cut == NULL || backslash > cut)) {
		cut = backslash;
	}
	if (cut == NULL) {
		return 1;
	}
	size = (size_t)(cut - path);
	if (size == 0) {
		return 1;
	}
	dirPath = dup_string_n(path, size);
	if (dirPath == NULL) {
		return 0;
	}
	if (!xrtDirExists((str)dirPath) && !xrtDirCreateAll((str)dirPath)) {
		free(dirPath);
		return 0;
	}
	free(dirPath);
	return 1;
}

static int print_xpk_error(const char* action, const char* target, xpkObject pkg)
{
	fprintf(stderr, "Failed to %s '%s': %s\n", action, target, xpkLastErrorMessage(pkg));
	return 1;
}
