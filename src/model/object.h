#ifndef XPK_MODEL_OBJECT_H
#define XPK_MODEL_OBJECT_H

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

#endif
