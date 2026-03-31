#ifndef XPACK_H
#define XPACK_H

#include <stdint.h>

#ifndef XPK_XTIME_DEFINED
#define XPK_XTIME_DEFINED
typedef int64_t xtime;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN64)
	#if defined(XPK_BUILD_DLL)
		#define XPKAPI __declspec(dllexport)
	#elif defined(XPK_USE_DLL)
		#define XPKAPI __declspec(dllimport)
	#else
		#define XPKAPI
	#endif
#else
	#define XPKAPI
#endif

#define XPK_FILE_HEAD        0x116B7078u
#define XPK_HEAD_SIZE        64u
#define XPK_VOLUME_MIN       0x00010000u
#define XPK_VOLUME_MAX       0xFFFFFFFFu
#define XPK_PATH_BYTES       260u
#define XPK_PATH_ENTRY_SIZE  320u

#define XPK_TYPE_CORE        0u
#define XPK_TYPE_INDEX       1u
#define XPK_TYPE_LINUX       2u
#define XPK_TYPE_WIN32       3u

#define XPK_ALG_STORE        0u
#define XPK_ALG_LZ4          1u
#define XPK_ALG_LZ4HC        2u
#define XPK_ALG_ZSTD         3u
#define XPK_ALG_LZMA2        4u

#define XPK_FLAG_COMP_MASK     0x0000000Fu
#define XPK_FLAG_TYPE_MASK     0x000000F0u
#define XPK_FLAG_DELETED_MASK  0x00000100u

typedef struct xpkStruct* xpkObject;

typedef enum xpkPackType {
	XPK_PACK_CORE = 0,
	XPK_PACK_INDEX = 1,
	XPK_PACK_LINUX = 2,
	XPK_PACK_WIN32 = 3
} xpkPackType;

typedef enum xpkWritePolicy {
	XPK_WRITE_BUFFERED = 0,
	XPK_WRITE_IMMEDIATE = 1
} xpkWritePolicy;

typedef enum xpkErrorCode {
	XPK_OK = 0,
	XPK_ERR_PARAM = -1,
	XPK_ERR_STATE = -2,
	XPK_ERR_MEMORY = -3,
	XPK_ERR_IO = -4,
	XPK_ERR_FORMAT = -5,
	XPK_ERR_HASH = -6,
	XPK_ERR_NOT_FOUND = -7,
	XPK_ERR_EXISTS = -8,
	XPK_ERR_SOLID_DATA_WRITE = -9,
	XPK_ERR_READONLY = -10,
	XPK_ERR_UNSUPPORTED = -11
} xpkErrorCode;

typedef struct xpkOpenOptions {
	uint8_t readonly;
	uint8_t createIfMissing;
	uint8_t bufferedDefault;
	uint8_t reserved0;
} xpkOpenOptions;

typedef struct xpkWriteOptions {
	uint8_t compLevel;
	uint8_t writePolicy;
	uint8_t fileType;
	uint8_t reserved0;
} xpkWriteOptions;

typedef struct xpkBuildOptions {
	const char* tempPath;
	uint8_t replaceOriginal;
	uint8_t reserved0[7];
} xpkBuildOptions;

typedef struct xpkStat {
	uint32_t fileCount;
	uint64_t liveDataBytes;
	uint64_t holeBytes;
	uint64_t metaBytes;
	uint64_t entryTableBytes;
} xpkStat;

#pragma pack(push, 1)
typedef struct {
	uint32_t fileHead;
	uint32_t fileCount;

	uint32_t packType    : 2;
	uint32_t defComp     : 4;
	uint32_t metaComp    : 4;
	uint32_t infoComp    : 4;
	uint32_t infoExtSize : 18;

	uint32_t solidMode   : 1;
	uint32_t volumeMode  : 1;
	uint32_t reserved1   : 30;

	uint64_t dataOffset;
	uint32_t volumeSize;

	uint32_t metaRawSize;
	uint32_t metaCompSize;
	uint32_t metaHash;

	uint32_t infoCompSize;
	uint32_t infoHash;

	xtime createTime;
	xtime changeTime;
} xpkHead;

typedef struct {
	uint32_t flag;
	uint32_t fileHash;
	uint64_t dataOffset;
	uint64_t dataSize;
	uint64_t fileSize;
} xpkFileInfo;

typedef struct {
	uint32_t flag;
	uint32_t fileHash;
	uint64_t dataOffset;
	uint64_t dataSize;
	uint64_t fileSize;
	int64_t fileIndex;
} xpkFileInfoIndex;

typedef struct {
	uint32_t flag;
	uint32_t fileHash;
	uint64_t dataOffset;
	uint64_t dataSize;
	uint64_t fileSize;
	char pathBytes[XPK_PATH_BYTES];
	uint32_t platformAttr;
	uint64_t createTime;
	uint64_t modifyTime;
	uint64_t accessTime;
} xpkFileInfoPath;
#pragma pack(pop)

typedef char xpkStaticAssertHeadSize[(sizeof(xpkHead) == XPK_HEAD_SIZE) ? 1 : -1];
typedef char xpkStaticAssertFileInfoSize[(sizeof(xpkFileInfo) == 32u) ? 1 : -1];
typedef char xpkStaticAssertFileInfoIndexSize[(sizeof(xpkFileInfoIndex) == 40u) ? 1 : -1];
typedef char xpkStaticAssertFileInfoPathSize[(sizeof(xpkFileInfoPath) == XPK_PATH_ENTRY_SIZE) ? 1 : -1];

typedef int (*xpkEachProc)(xpkObject xpk, uint32_t pos, const void* info, void* userData);

XPKAPI xpkObject xpkOpen(const char* packagePath, const xpkOpenOptions* options);
XPKAPI int xpkClose(xpkObject xpk);
XPKAPI int xpkSave(xpkObject xpk);
XPKAPI int xpkBuild(xpkObject xpk, const xpkBuildOptions* options);

XPKAPI int xpkGetPackType(xpkObject xpk, xpkPackType* outType);
XPKAPI int xpkSetPackType(xpkObject xpk, xpkPackType type);
XPKAPI int xpkGetDefaultComp(xpkObject xpk, uint8_t* outLevel);
XPKAPI int xpkSetDefaultComp(xpkObject xpk, uint8_t level);
XPKAPI int xpkGetMetaComp(xpkObject xpk, uint8_t* outLevel);
XPKAPI int xpkSetMetaComp(xpkObject xpk, uint8_t level);
XPKAPI int xpkGetInfoComp(xpkObject xpk, uint8_t* outLevel);
XPKAPI int xpkSetInfoComp(xpkObject xpk, uint8_t level);
XPKAPI int xpkGetInfoExtSize(xpkObject xpk, uint32_t* outSize);
XPKAPI int xpkSetInfoExtSize(xpkObject xpk, uint32_t size);
XPKAPI int xpkGetVolumeSize(xpkObject xpk, uint32_t* outSize);
XPKAPI int xpkSetVolumeSize(xpkObject xpk, uint32_t size);
XPKAPI int xpkGetSolidMode(xpkObject xpk, int* outEnabled);
XPKAPI int xpkSetSolidMode(xpkObject xpk, int enabled);

XPKAPI void* xpkMetaGet(xpkObject xpk, uint32_t* outSize);
XPKAPI int xpkMetaSet(xpkObject xpk, const void* data, uint32_t size, uint8_t compLevel);
XPKAPI int xpkMetaClear(xpkObject xpk);

XPKAPI uint32_t xpkCount(xpkObject xpk);
XPKAPI int xpkGetInfo(xpkObject xpk, uint32_t pos, xpkFileInfo* outInfo);
XPKAPI int xpkGetInfoExt(xpkObject xpk, uint32_t pos, void* outData, uint32_t size);
XPKAPI int xpkSetInfoExt(xpkObject xpk, uint32_t pos, const void* data, uint32_t size);
XPKAPI int xpkAddFile(xpkObject xpk, const char* srcPath, const xpkWriteOptions* options, uint32_t* outPos);
XPKAPI int xpkAddData(xpkObject xpk, const void* data, uint64_t size, const xpkWriteOptions* options, uint32_t* outPos);
XPKAPI int xpkReadToFile(xpkObject xpk, uint32_t pos, const char* dstPath);
XPKAPI void* xpkReadToMemory(xpkObject xpk, uint32_t pos, uint64_t* outSize);
XPKAPI int xpkUpdateFile(xpkObject xpk, uint32_t pos, const char* srcPath, const xpkWriteOptions* options);
XPKAPI int xpkUpdateData(xpkObject xpk, uint32_t pos, const void* data, uint64_t size, const xpkWriteOptions* options);
XPKAPI int xpkRemove(xpkObject xpk, uint32_t pos);
XPKAPI int xpkSetFlag(xpkObject xpk, uint32_t pos, uint32_t mask, uint32_t value);

XPKAPI int xpkIndexFind(xpkObject xpk, int64_t fileIndex, uint32_t* outPos);
XPKAPI int xpkIndexGetInfo(xpkObject xpk, int64_t fileIndex, xpkFileInfoIndex* outInfo);
XPKAPI int xpkIndexAddFile(xpkObject xpk, int64_t fileIndex, const char* srcPath, const xpkWriteOptions* options);
XPKAPI int xpkIndexAddData(xpkObject xpk, int64_t fileIndex, const void* data, uint64_t size, const xpkWriteOptions* options);
XPKAPI int xpkIndexReadToFile(xpkObject xpk, int64_t fileIndex, const char* dstPath);
XPKAPI void* xpkIndexReadToMemory(xpkObject xpk, int64_t fileIndex, uint64_t* outSize);
XPKAPI int xpkIndexUpdateFile(xpkObject xpk, int64_t fileIndex, const char* srcPath, const xpkWriteOptions* options);
XPKAPI int xpkIndexUpdateData(xpkObject xpk, int64_t fileIndex, const void* data, uint64_t size, const xpkWriteOptions* options);
XPKAPI int xpkIndexRemove(xpkObject xpk, int64_t fileIndex);
XPKAPI int xpkIndexSetFlag(xpkObject xpk, int64_t fileIndex, uint32_t mask, uint32_t value);

XPKAPI int xpkPathExists(xpkObject xpk, const char* packagePath);
XPKAPI int xpkPathGetInfo(xpkObject xpk, const char* packagePath, xpkFileInfoPath* outInfo);
XPKAPI int xpkPathAddFile(xpkObject xpk, const char* packagePath, const char* srcPath, const xpkWriteOptions* options);
XPKAPI int xpkPathAddData(xpkObject xpk, const char* packagePath, const void* data, uint64_t size, const xpkWriteOptions* options);
XPKAPI int xpkPathReadToFile(xpkObject xpk, const char* packagePath, const char* dstPath);
XPKAPI void* xpkPathReadToMemory(xpkObject xpk, const char* packagePath, uint64_t* outSize);
XPKAPI int xpkPathUpdateFile(xpkObject xpk, const char* packagePath, const char* srcPath, const xpkWriteOptions* options);
XPKAPI int xpkPathUpdateData(xpkObject xpk, const char* packagePath, const void* data, uint64_t size, const xpkWriteOptions* options);
XPKAPI int xpkPathRename(xpkObject xpk, const char* oldPath, const char* newPath);
XPKAPI int xpkPathRemove(xpkObject xpk, const char* packagePath);
XPKAPI int xpkPathSetAttr(xpkObject xpk, const char* packagePath, uint32_t platformAttr);

XPKAPI int xpkEach(xpkObject xpk, xpkEachProc proc, void* userData);
XPKAPI int xpkEachMatch(xpkObject xpk, const char* pattern, xpkEachProc proc, void* userData);

XPKAPI int xpkVerify(xpkObject xpk, uint32_t pos);
XPKAPI int xpkVerifyAll(xpkObject xpk);
XPKAPI int xpkStatGet(xpkObject xpk, xpkStat* outStat);
XPKAPI void xpkFree(void* ptr);
XPKAPI uint32_t xpkHash32(const void* data, uint64_t size);

XPKAPI xpkErrorCode xpkLastError(xpkObject xpk);
XPKAPI const char* xpkLastErrorMessage(xpkObject xpk);

#ifdef __cplusplus
}
#endif

#if defined(XPACK_BUILD_CORE)

/* xPack core declarations require xrt + codec headers to be included first. */

#define XPK_ERROR_TEXT_CAP 256

#if defined(__GNUC__) || defined(__clang__)
	#define XPK_UNUSED __attribute__((unused))
#else
	#define XPK_UNUSED
#endif

enum {
	XPK_INTERNAL_ERROR_TEXT_CAP = XPK_ERROR_TEXT_CAP,
	XPK_ENTRY_BASE_SIZE = 32,
	XPK_INFO_EXT_INDEX = 8,
	XPK_INFO_EXT_PATH = 288
};

typedef struct xpkErrorState xpkErrorState;
typedef struct xpkEntry xpkEntry;
typedef struct xpkStorage xpkStorage;
typedef struct xpkWriteQueue xpkWriteQueue;

struct xpkErrorState {
	int iCode;
	char sText[XPK_ERROR_TEXT_CAP];
};

struct xpkEntry {
	uint32_t iPos;
	uint32_t iFlag;
	uint32_t iFileHash;
	uint64_t iDataOffset;
	uint64_t iDataSize;
	uint64_t iFileSize;
	uint8_t bStored;

	int64_t iFileIndex;

	void* pInfoExt;
	char* sPath;
	uint32_t iPlatformAttr;
	uint64_t tCreateTime;
	uint64_t tModifyTime;
	uint64_t tAccessTime;
};

struct xpkStruct {
	xpkHead objHead;

	uint8_t bReadonly;
	uint8_t bBufferedDefault;
	uint8_t bDirtyHead;
	uint8_t bDirtyPackageMeta;
	uint8_t bDirtyEntryTable;
	uint8_t bDirtyData;
	uint8_t bSolidApplied;
	uint8_t bVolumeApplied;
	uint16_t reserved0;
	uint32_t iVolumeSizeApplied;

	uint64_t iAppendPos;
	uint64_t iFileSize;

	char* sPathPackage;
	void* pPackageMeta;
	uint32_t iPackageMetaSize;

	xarray_struct arrEntry;
	xlist_struct lstEntry;
	xdict_struct tblEntry;
	uint32_t iEntryCount;

	xpkWriteQueue* pWriteQueue;
	xpkStorage* pStorage;
	xpkErrorState err;
};

typedef enum xpkDirtyBits {
	XPK_DIRTY_NONE = 0,
	XPK_DIRTY_HEAD = 1 << 0,
	XPK_DIRTY_PACKAGE_META = 1 << 1,
	XPK_DIRTY_ENTRY_TABLE = 1 << 2,
	XPK_DIRTY_DATA = 1 << 3
} xpkDirtyBits;

typedef struct xpkCompMap {
	uint32_t iAlgorithm;
	int iNativeLevel;
} xpkCompMap;

typedef struct xpkMappedFile {
	const void* pView;
	uint64_t iSize;
#if defined(_WIN32) || defined(_WIN64)
	HANDLE hMap;
#else
	void* pMap;
#endif
} xpkMappedFile;

typedef struct xpkLooseVolumeScan {
	xpkObject objXpk;
	const char* sNameBase;
	size_t iNameSize;
	uint32_t iStartVolume;
	uint32_t iCount;
	int bDelete;
	int iError;
} xpkLooseVolumeScan;

typedef struct xpkWriteNode {
	uint32_t iPos;
	uint8_t iLevel;
	uint8_t reserved0[3];
	uint32_t iCompSize;
	uint64_t iRawSize;
	void* pCompData;
} xpkWriteNode;

struct xpkWriteQueue {
	xarray_struct arrNode;
};

#define XPK_WRITE_FILE_CHUNK_SIZE (8u * 1024u * 1024u)
#define XPK_CODEC_STREAM_CHUNK_SIZE (256u * 1024u)

typedef struct xpkFlushSnapshot {
	uint32_t iPos;
	uint32_t iFlag;
	uint64_t iDataOffset;
	uint64_t iDataSize;
	uint64_t iFileSize;
} xpkFlushSnapshot;

typedef struct xpkSaveRollback {
	int bFileExisted;
	int bHeadValid;
	int bTailInFile;
	int bKeepTailFile;
	uint64_t iLogicalSize;
	uint64_t iDataOffset;
	uint64_t iTailSize;
	uint8_t sHeadBuf[XPK_HEAD_SIZE];
	void* pTailData;
	char* sTailPath;
} xpkSaveRollback;

#define XPK_SAVE_ROLLBACK_MEM_LIMIT   (8u * 1024u * 1024u)
#define XPK_SAVE_ROLLBACK_CHUNK_SIZE  (8u * 1024u * 1024u)

typedef struct xpkBuildLzmaSeqIn {
	ISeqInStream vt;
	xfile hFile;
	uint64_t iRemain;
} xpkBuildLzmaSeqIn;

typedef struct xpkBuildLzmaSeqOut {
	ISeqOutStream vt;
	xfile hFile;
	uint64_t iSize;
} xpkBuildLzmaSeqOut;

typedef struct xpkWriteLzmaSeqIn {
	ISeqInStream vt;
	xfile hFile;
	uint64_t iRemain;
} xpkWriteLzmaSeqIn;

typedef struct xpkWriteLzmaSeqOut {
	ISeqOutStream vt;
	xfile hFile;
	uint64_t iSize;
} xpkWriteLzmaSeqOut;

typedef struct xpkWriteLzmaMemIn {
	ISeqInStream vt;
	const uint8_t* pData;
	uint64_t iSize;
	uint64_t iPos;
} xpkWriteLzmaMemIn;

static inline int procXpkWriteQueueCount(xpkObject objXpk);
static inline void procXpkUnitWriteQueue(xpkObject objXpk);
static inline void procXpkMarkClean(xpkObject objXpk);
static inline void procXpkInitHead(xpkObject objXpk);
static inline void procXpkMarkAppliedLayout(xpkObject objXpk);
static inline int procXpkAddDataEntry(xpkObject objXpk, xpkEntry* pEntrySeed, const void* pData, uint64_t iSize, const xpkWriteOptions* pOpt, uint32_t* pPosRet);
static inline int procXpkValidateEntryCountState(xpkObject objXpk);
static inline int procXpkValidateLiveEntryLookup(xpkObject objXpk, const xpkEntry* pEntry);
static inline int procXpkBuildAppendEntryOnly(xpkObject objDst, const xpkEntry* pEntrySrc, uint64_t iDataOffset, uint64_t iDataSize, uint64_t iFileSize);

static inline uint8_t procXpkSolidStoredCompLevel(xpkObject objXpk);
static inline int procXpkCalcSolidRawSize(xpkObject objXpk, uint64_t* pSizeRet);
static inline int procXpkHashMappedFileRange(xpkObject objXpk, xfile hFile, uint64_t iOffset, uint64_t iSize, uint32_t* pHashRet);
static inline int procXpkCopyDecodedLz4BlockToFile(xpkObject objXpk, uint8_t iLevel, const void* pCompData, uint32_t iCompSize, uint64_t iRawSize, const char* sPathFile);
static inline int procXpkWriteImmediateStoreFile(xpkObject objXpk, xpkEntry* pEntry, const char* sSrcPath);
static inline int procXpkWriteImmediateLz4Data(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint32_t iSize, uint8_t iLevel);
static inline int procXpkWriteImmediateZstdData(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint32_t iSize, uint8_t iLevel);
static inline int procXpkWriteImmediateLzma2Data(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint32_t iSize, uint8_t iLevel);
static inline int procXpkWriteBufferedLz4Data(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint32_t iSize, uint8_t iLevel);
static inline int procXpkWriteBufferedZstdData(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint32_t iSize, uint8_t iLevel);
static inline int procXpkWriteBufferedLzma2Data(xpkObject objXpk, xpkEntry* pEntry, const void* pData, uint32_t iSize, uint8_t iLevel);

static inline int procXpkVerifyStoredEntryWithFile(xpkObject objXpk, xpkEntry* pEntry, xfile hFile);
static inline int procXpkVerifyStoredEntryMapped(xpkObject objXpk, xpkEntry* pEntry, const xpkMappedFile* pMap);
static inline int procXpkVerifyDecodedEntryWithFile(xpkObject objXpk, xpkEntry* pEntry, xfile hFile);
static inline int procXpkVerifyDecodedEntryMapped(xpkObject objXpk, xpkEntry* pEntry, const xpkMappedFile* pMap);

#endif

#endif
