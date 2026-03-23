#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct
{
	const char* sPath;
} xpkHeaderFile;


static const xpkHeaderFile g_arrDeclFile[] = {
	{ "src/api/public_decl.h" }
};


static const xpkHeaderFile g_arrImplFile[] = {
	{ "src/base/config.h" },
	{ "src/base/const.h" },
	{ "src/base/types.h" },
	{ "src/base/error.h" },
	{ "src/base/memory.h" },
	{ "src/base/hash.h" },
	{ "src/base/time.h" },
	{ "src/internal/forward.h" },
	{ "src/model/entry.h" },
	{ "src/model/object.h" },
	{ "src/model/dirty.h" },
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


static int procCopyStream(FILE* hDst, FILE* hSrc)
{
	char arrBuf[8192];
	size_t iRead;

	while ( 1 ) {
		iRead = fread(arrBuf, 1, sizeof(arrBuf), hSrc);
		if ( iRead > 0 ) {
			if ( fwrite(arrBuf, 1, iRead, hDst) != iRead ) {
				return 0;
			}
		}
		if ( iRead < sizeof(arrBuf) ) {
			if ( ferror(hSrc) ) {
				return 0;
			}
			break;
		}
	}

	return 1;
}


static int procWriteText(FILE* hDst, const char* sText)
{
	size_t iSize;

	iSize = strlen(sText);
	return (fwrite(sText, 1, iSize, hDst) == iSize) ? 1 : 0;
}


static int procWriteFileBlock(FILE* hDst, const char* sPath)
{
	FILE* hSrc;

	hSrc = fopen(sPath, "rb");
	if ( hSrc == NULL ) {
		fprintf(stderr, "failed to open %s\n", sPath);
		return 0;
	}

	if ( !procWriteText(hDst, "\n/* ===== File: ") ) {
		fclose(hSrc);
		return 0;
	}
	if ( !procWriteText(hDst, sPath) ) {
		fclose(hSrc);
		return 0;
	}
	if ( !procWriteText(hDst, " ===== */\n\n") ) {
		fclose(hSrc);
		return 0;
	}

	if ( !procCopyStream(hDst, hSrc) ) {
		fclose(hSrc);
		fprintf(stderr, "failed to copy %s\n", sPath);
		return 0;
	}

	if ( !procWriteText(hDst, "\n") ) {
		fclose(hSrc);
		return 0;
	}

	fclose(hSrc);
	return 1;
}


static int procWriteFileList(FILE* hDst, const xpkHeaderFile* arrFile, size_t iCount)
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
		procWriteText(hDst, "/* xPack requires xrt and keeps xrt as an external dependency. */\n") &&
		procWriteText(hDst, "#if defined(__has_include)\n") &&
		procWriteText(hDst, "#\tif __has_include(\"xrt.h\")\n") &&
		procWriteText(hDst, "#\t\tinclude \"xrt.h\"\n") &&
		procWriteText(hDst, "#\telif __has_include(\"lib/xrt.h\")\n") &&
		procWriteText(hDst, "#\t\tinclude \"lib/xrt.h\"\n") &&
		procWriteText(hDst, "#\telse\n") &&
		procWriteText(hDst, "#\t\terror \"xpack requires xrt.h to be available in the include path\"\n") &&
		procWriteText(hDst, "#\tendif\n") &&
		procWriteText(hDst, "#else\n") &&
		procWriteText(hDst, "#\tinclude \"lib/xrt.h\"\n") &&
		procWriteText(hDst, "#endif\n\n");
}


static int procWriteCodecInclude(FILE* hDst, const char* sDirectPath, const char* sFallbackPath, int bFirst)
{
	if ( bFirst ) {
		if ( !procWriteText(hDst, "#if defined(__has_include)\n") ) {
			return 0;
		}
	} else {
		if ( !procWriteText(hDst, "\n") ) {
			return 0;
		}
	}

	return
		procWriteText(hDst, "#\tif __has_include(\"") &&
		procWriteText(hDst, sDirectPath) &&
		procWriteText(hDst, "\")\n") &&
		procWriteText(hDst, "#\t\tinclude \"") &&
		procWriteText(hDst, sDirectPath) &&
		procWriteText(hDst, "\"\n") &&
		procWriteText(hDst, "#\telif __has_include(\"") &&
		procWriteText(hDst, sFallbackPath) &&
		procWriteText(hDst, "\")\n") &&
		procWriteText(hDst, "#\t\tinclude \"") &&
		procWriteText(hDst, sFallbackPath) &&
		procWriteText(hDst, "\"\n") &&
		procWriteText(hDst, "#\telse\n") &&
		procWriteText(hDst, "#\t\terror \"missing third-party header: ") &&
		procWriteText(hDst, sDirectPath) &&
		procWriteText(hDst, "\"\n") &&
		procWriteText(hDst, "#\tendif\n");
}


static int procWriteCodecIncludes(FILE* hDst)
{
	return
		procWriteText(hDst, "/* Third-party compression libraries stay external dependencies. */\n") &&
		procWriteCodecInclude(hDst, "lz4.h", "lib/lz4/lz4.h", 1) &&
		procWriteCodecInclude(hDst, "lz4hc.h", "lib/lz4/lz4hc.h", 0) &&
		procWriteCodecInclude(hDst, "zstd.h", "lib/zstd/zstd.h", 0) &&
		procWriteCodecInclude(hDst, "Alloc.h", "lib/lzma/Alloc.h", 0) &&
		procWriteCodecInclude(hDst, "Lzma2Dec.h", "lib/lzma/Lzma2Dec.h", 0) &&
		procWriteCodecInclude(hDst, "Lzma2Enc.h", "lib/lzma/Lzma2Enc.h", 0) &&
		procWriteText(hDst, "#else\n") &&
		procWriteText(hDst, "#\tinclude \"lib/lz4/lz4.h\"\n") &&
		procWriteText(hDst, "#\tinclude \"lib/lz4/lz4hc.h\"\n") &&
		procWriteText(hDst, "#\tinclude \"lib/zstd/zstd.h\"\n") &&
		procWriteText(hDst, "#\tinclude \"lib/lzma/Alloc.h\"\n") &&
		procWriteText(hDst, "#\tinclude \"lib/lzma/Lzma2Dec.h\"\n") &&
		procWriteText(hDst, "#\tinclude \"lib/lzma/Lzma2Enc.h\"\n") &&
		procWriteText(hDst, "#endif\n\n");
}


int main(void)
{
	FILE* hDst;

	hDst = fopen("xpack.h", "wb");
	if ( hDst == NULL ) {
		fprintf(stderr, "failed to create xpack.h\n");
		return 1;
	}

	if ( !procWriteText(hDst, "/* Auto-generated single header from xPack source tree. */\n") ||
		!procWriteText(hDst, "#ifndef XPACK_H\n") ||
		!procWriteText(hDst, "#define XPACK_H\n\n") ||
		!procWriteXrtInclude(hDst) ||
		!procWriteText(hDst, "#ifdef __cplusplus\n") ||
		!procWriteText(hDst, "extern \"C\" {\n") ||
		!procWriteText(hDst, "#endif\n") ||
		!procWriteText(hDst, "\n") ||
		!procWriteFileList(hDst, g_arrDeclFile, sizeof(g_arrDeclFile) / sizeof(g_arrDeclFile[0])) ||
		!procWriteText(hDst, "\n#ifdef __cplusplus\n") ||
		!procWriteText(hDst, "}\n") ||
		!procWriteText(hDst, "#endif\n\n") ||
		!procWriteText(hDst, "#if defined(XPACK_IMPLEMENTATION)\n\n") ||
		!procWriteText(hDst, "/* Usage:\n") ||
		!procWriteText(hDst, "\t#define XRT_IMPLEMENTATION\n") ||
		!procWriteText(hDst, "\t#define XPACK_IMPLEMENTATION\n") ||
		!procWriteText(hDst, "\t#include \"xpack.h\"\n") ||
		!procWriteText(hDst, "\n\tCompile and link the external third-party sources for lz4 / zstd / lzma.\n") ||
		!procWriteText(hDst, "*/\n\n") ||
		!procWriteCodecIncludes(hDst) ||
		!procWriteFileList(hDst, g_arrImplFile, sizeof(g_arrImplFile) / sizeof(g_arrImplFile[0])) ||
		!procWriteText(hDst, "\n#endif\n\n") ||
		!procWriteText(hDst, "#endif\n") ) {
		fclose(hDst);
		fprintf(stderr, "failed to write xpack.h\n");
		return 2;
	}

	fclose(hDst);
	return 0;
}
