#ifndef XPK_MODEL_ENTRY_H
#define XPK_MODEL_ENTRY_H

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

#endif
