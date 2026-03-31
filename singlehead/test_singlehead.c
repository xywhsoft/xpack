#define XRT_IMPLEMENTATION
#include "../lib/xrt.h"

#define XPACK_IMPLEMENTATION
#include "xpack.h"


int main(void)
{
	xpkObject objXpk;
	xpkOpenOptions tOpen;
	xpkWriteOptions tWrite;
	uint32_t iPos;
	uint64_t iSizeData;
	void* pData;
	char sPathPkg[] = "release/x64/xpack_singlehead_smoke.xpk";
	char sPathExport[] = "release/x64/xpack_singlehead_export.bin";
	char sText[] = "xpack-single-header";
	size_t iSizeText;

	iSizeText = strlen(sText);
	pData = NULL;
	objXpk = NULL;
	iPos = 0;

	xrtFileDelete(sPathPkg);
	xrtFileDelete(sPathExport);

	memset(&tOpen, 0, sizeof(tOpen));
	tOpen.createIfMissing = TRUE;

	memset(&tWrite, 0, sizeof(tWrite));
	tWrite.compLevel = 6;
	tWrite.writePolicy = XPK_WRITE_BUFFERED;

	objXpk = xpkOpen(sPathPkg, &tOpen);
	if ( objXpk == NULL ) {
		return 1;
	}

	if ( xpkAddData(objXpk, sText, (uint64_t)iSizeText, &tWrite, &iPos) != XPK_OK ) {
		xpkClose(objXpk);
		return 2;
	}

	if ( xpkSave(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 3;
	}

	if ( xpkClose(objXpk) != XPK_OK ) {
		return 4;
	}
	objXpk = NULL;

	objXpk = xpkOpen(sPathPkg, NULL);
	if ( objXpk == NULL ) {
		return 5;
	}

	if ( xpkVerifyAll(objXpk) != XPK_OK ) {
		xpkClose(objXpk);
		return 6;
	}

	pData = xpkReadToMemory(objXpk, iPos, &iSizeData);
	if ( pData == NULL ) {
		xpkClose(objXpk);
		return 7;
	}

	if ( (iSizeData != (uint64_t)iSizeText) || (memcmp(pData, sText, iSizeText) != 0) ) {
		xpkFree(pData);
		xpkClose(objXpk);
		return 8;
	}
	xpkFree(pData);
	pData = NULL;

	if ( xpkReadToFile(objXpk, iPos, sPathExport) != XPK_OK ) {
		xpkClose(objXpk);
		return 9;
	}

	if ( !xrtFileExists(sPathExport) ) {
		xpkClose(objXpk);
		return 10;
	}

	if ( xpkClose(objXpk) != XPK_OK ) {
		return 11;
	}

	xrtFileDelete(sPathPkg);
	xrtFileDelete(sPathExport);
	return 0;
}
