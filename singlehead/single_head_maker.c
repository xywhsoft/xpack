#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct xpkFileEntry {
	const char* sPath;
} xpkFileEntry;


static const xpkFileEntry g_arrImplFile[] = {
	{ "src/base/memory.h" },
	{ "src/base/hash.h" },
	{ "src/base/time.h" },
	{ "src/service/core.h" },
	{ "src/codec/router.h" },
	{ "src/storage/fileio.h" },
	{ "src/format/layout.h" },
	{ "src/service/open.h" },
	{ "src/service/write.h" },
	{ "src/service/save.h" },
	{ "src/service/build.h" },
	{ "src/service/verify.h" },
	{ "src/service/each.h" },
	{ "src/api/package_api.h" },
	{ "src/api/default_api.h" },
	{ "src/api/index_api.h" },
	{ "src/api/path_api.h" },
	{ "src/api/admin_api.h" }
};


static int procWriteText(FILE* hDst, const char* sText)
{
	size_t iSize = strlen(sText);
	return fwrite(sText, 1, iSize, hDst) == iSize;
}


static int procCopyStream(FILE* hDst, FILE* hSrc)
{
	char arrBuf[8192];
	size_t iRead;

	for ( ;; ) {
		iRead = fread(arrBuf, 1, sizeof(arrBuf), hSrc);
		if ( iRead > 0 ) {
			if ( fwrite(arrBuf, 1, iRead, hDst) != iRead ) {
				return 0;
			}
		}
		if ( iRead < sizeof(arrBuf) ) {
			return ferror(hSrc) ? 0 : 1;
		}
	}
}


static int procWriteFileBlock(FILE* hDst, const char* sPath)
{
	FILE* hSrc = fopen(sPath, "rb");

	if ( hSrc == NULL ) {
		fprintf(stderr, "failed to open %s\n", sPath);
		return 0;
	}

	if ( !procWriteText(hDst, "\n/* ===== File: ") ||
		!procWriteText(hDst, sPath) ||
		!procWriteText(hDst, " ===== */\n\n") ||
		!procCopyStream(hDst, hSrc) ||
		!procWriteText(hDst, "\n") ) {
		fclose(hSrc);
		fprintf(stderr, "failed to copy %s\n", sPath);
		return 0;
	}

	fclose(hSrc);
	return 1;
}


static int procWriteFileList(FILE* hDst, const xpkFileEntry* arrFile, size_t iCount)
{
	size_t iIndex;

	for ( iIndex = 0; iIndex < iCount; iIndex++ ) {
		if ( !procWriteFileBlock(hDst, arrFile[iIndex].sPath) ) {
			return 0;
		}
	}

	return 1;
}


static int procWriteXrtInclude(FILE* hDst)
{
	return
		procWriteText(hDst, "#if defined(__has_include)\n") &&
		procWriteText(hDst, "#\tif __has_include(\"xrt.h\")\n") &&
		procWriteText(hDst, "#\t\tinclude \"xrt.h\"\n") &&
		procWriteText(hDst, "#\telif __has_include(\"../lib/xrt.h\")\n") &&
		procWriteText(hDst, "#\t\tinclude \"../lib/xrt.h\"\n") &&
		procWriteText(hDst, "#\telif __has_include(\"lib/xrt.h\")\n") &&
		procWriteText(hDst, "#\t\tinclude \"lib/xrt.h\"\n") &&
		procWriteText(hDst, "#\telse\n") &&
		procWriteText(hDst, "#\t\terror \"xpack singlehead requires external xrt.h in include path\"\n") &&
		procWriteText(hDst, "#\tendif\n") &&
		procWriteText(hDst, "#else\n") &&
		procWriteText(hDst, "#\tinclude \"xrt.h\"\n") &&
		procWriteText(hDst, "#endif\n");
}


static int procWriteCodecIncludes(FILE* hDst)
{
	return
		procWriteText(hDst, "#if defined(__has_include)\n") &&
		procWriteText(hDst, "#\tif __has_include(\"lz4.h\")\n") &&
		procWriteText(hDst, "#\t\tinclude \"lz4.h\"\n") &&
		procWriteText(hDst, "#\telif __has_include(\"../lib/lz4/lz4.h\")\n") &&
		procWriteText(hDst, "#\t\tinclude \"../lib/lz4/lz4.h\"\n") &&
		procWriteText(hDst, "#\telse\n") &&
		procWriteText(hDst, "#\t\terror \"missing third-party header: lz4.h\"\n") &&
		procWriteText(hDst, "#\tendif\n") &&
		procWriteText(hDst, "#\tif __has_include(\"lz4hc.h\")\n") &&
		procWriteText(hDst, "#\t\tinclude \"lz4hc.h\"\n") &&
		procWriteText(hDst, "#\telif __has_include(\"../lib/lz4/lz4hc.h\")\n") &&
		procWriteText(hDst, "#\t\tinclude \"../lib/lz4/lz4hc.h\"\n") &&
		procWriteText(hDst, "#\telse\n") &&
		procWriteText(hDst, "#\t\terror \"missing third-party header: lz4hc.h\"\n") &&
		procWriteText(hDst, "#\tendif\n") &&
		procWriteText(hDst, "#\tif __has_include(\"zstd.h\")\n") &&
		procWriteText(hDst, "#\t\tinclude \"zstd.h\"\n") &&
		procWriteText(hDst, "#\telif __has_include(\"../lib/zstd/zstd.h\")\n") &&
		procWriteText(hDst, "#\t\tinclude \"../lib/zstd/zstd.h\"\n") &&
		procWriteText(hDst, "#\telse\n") &&
		procWriteText(hDst, "#\t\terror \"missing third-party header: zstd.h\"\n") &&
		procWriteText(hDst, "#\tendif\n") &&
		procWriteText(hDst, "#\tif __has_include(\"Alloc.h\")\n") &&
		procWriteText(hDst, "#\t\tinclude \"Alloc.h\"\n") &&
		procWriteText(hDst, "#\telif __has_include(\"../lib/lzma/Alloc.h\")\n") &&
		procWriteText(hDst, "#\t\tinclude \"../lib/lzma/Alloc.h\"\n") &&
		procWriteText(hDst, "#\telse\n") &&
		procWriteText(hDst, "#\t\terror \"missing third-party header: Alloc.h\"\n") &&
		procWriteText(hDst, "#\tendif\n") &&
		procWriteText(hDst, "#\tif __has_include(\"Lzma2Dec.h\")\n") &&
		procWriteText(hDst, "#\t\tinclude \"Lzma2Dec.h\"\n") &&
		procWriteText(hDst, "#\telif __has_include(\"../lib/lzma/Lzma2Dec.h\")\n") &&
		procWriteText(hDst, "#\t\tinclude \"../lib/lzma/Lzma2Dec.h\"\n") &&
		procWriteText(hDst, "#\telse\n") &&
		procWriteText(hDst, "#\t\terror \"missing third-party header: Lzma2Dec.h\"\n") &&
		procWriteText(hDst, "#\tendif\n") &&
		procWriteText(hDst, "#\tif __has_include(\"Lzma2Enc.h\")\n") &&
		procWriteText(hDst, "#\t\tinclude \"Lzma2Enc.h\"\n") &&
		procWriteText(hDst, "#\telif __has_include(\"../lib/lzma/Lzma2Enc.h\")\n") &&
		procWriteText(hDst, "#\t\tinclude \"../lib/lzma/Lzma2Enc.h\"\n") &&
		procWriteText(hDst, "#\telse\n") &&
		procWriteText(hDst, "#\t\terror \"missing third-party header: Lzma2Enc.h\"\n") &&
		procWriteText(hDst, "#\tendif\n") &&
		procWriteText(hDst, "#else\n") &&
		procWriteText(hDst, "#\tinclude \"../lib/lz4/lz4.h\"\n") &&
		procWriteText(hDst, "#\tinclude \"../lib/lz4/lz4hc.h\"\n") &&
		procWriteText(hDst, "#\tinclude \"../lib/zstd/zstd.h\"\n") &&
		procWriteText(hDst, "#\tinclude \"../lib/lzma/Alloc.h\"\n") &&
		procWriteText(hDst, "#\tinclude \"../lib/lzma/Lzma2Dec.h\"\n") &&
		procWriteText(hDst, "#\tinclude \"../lib/lzma/Lzma2Enc.h\"\n") &&
		procWriteText(hDst, "#endif\n");
}


int main(void)
{
	FILE* hDst = fopen("singlehead/xpack.h", "wb");

	if ( hDst == NULL ) {
		fprintf(stderr, "failed to create singlehead/xpack.h\n");
		return 1;
	}

	if ( !procWriteText(hDst, "/* Auto-generated single header from xPack source tree. */\n") ||
		!procWriteText(hDst, "#ifndef XPACK_SINGLE_HEADER\n") ||
		!procWriteText(hDst, "#define XPACK_SINGLE_HEADER\n\n") ||
		!procWriteText(hDst, "/* Usage:\n") ||
		!procWriteText(hDst, "\t#include \"xrt.h\"    // external dependency, provide declarations or implementation yourself\n") ||
		!procWriteText(hDst, "\t#define XPACK_IMPLEMENTATION\n") ||
		!procWriteText(hDst, "\t#include \"xpack.h\"\n") ||
		!procWriteText(hDst, "\n\tCompile and link external dependencies yourself:\n") ||
		!procWriteText(hDst, "\t- xrt\n") ||
		!procWriteText(hDst, "\t- lz4 / zstd / lzma headers and libraries\n") ||
		!procWriteText(hDst, "*/\n\n") ||
		!procWriteText(hDst, "#if defined(XPACK_IMPLEMENTATION)\n") ||
		!procWriteXrtInclude(hDst) ||
		!procWriteText(hDst, "#\tdefine XPACK_BUILD_CORE\n") ||
		!procWriteCodecIncludes(hDst) ||
		!procWriteText(hDst, "#endif\n") ||
		!procWriteFileBlock(hDst, "xpack.h") ||
		!procWriteText(hDst, "\n#if defined(XPACK_IMPLEMENTATION)\n") ||
		!procWriteFileList(hDst, g_arrImplFile, sizeof(g_arrImplFile) / sizeof(g_arrImplFile[0])) ||
		!procWriteText(hDst, "\n#undef XPACK_BUILD_CORE\n") ||
		!procWriteText(hDst, "#endif\n\n") ||
		!procWriteText(hDst, "#endif\n") ) {
		fclose(hDst);
		fprintf(stderr, "failed to write singlehead/xpack.h\n");
		return 2;
	}

	fclose(hDst);
	return 0;
}
