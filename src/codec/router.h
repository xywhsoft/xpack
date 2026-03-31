static const xpkCompMap arrXpkCompTable[16] = {
	{ XPK_ALG_STORE, 0 },
	{ XPK_ALG_LZ4, 1 },
	{ XPK_ALG_LZ4, 4 },
	{ XPK_ALG_LZ4HC, 8 },
	{ XPK_ALG_LZ4HC, 12 },
	{ XPK_ALG_ZSTD, ZSTD_fast },
	{ XPK_ALG_ZSTD, ZSTD_dfast },
	{ XPK_ALG_ZSTD, ZSTD_greedy },
	{ XPK_ALG_ZSTD, ZSTD_lazy },
	{ XPK_ALG_ZSTD, ZSTD_lazy2 },
	{ XPK_ALG_ZSTD, ZSTD_btlazy2 },
	{ XPK_ALG_ZSTD, ZSTD_btopt },
	{ XPK_ALG_ZSTD, ZSTD_btultra },
	{ XPK_ALG_ZSTD, ZSTD_btultra2 },
	{ XPK_ALG_LZMA2, 6 },
	{ XPK_ALG_LZMA2, 9 }
};

static inline uint32_t procXpkCompLevelToAlg(uint8_t iLevel)
{
	return arrXpkCompTable[iLevel & 0x0Fu].iAlgorithm;
}

static inline int procXpkCompLevelToNative(uint8_t iLevel)
{
	return arrXpkCompTable[iLevel & 0x0Fu].iNativeLevel;
}

static inline int procXpkCodecBound(xpkObject objXpk, uint8_t iLevel, uint32_t iRawSize, uint32_t* pSizeRet)
{
	uint64_t iBound64;
	size_t iBound;

	if ( pSizeRet == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	switch ( procXpkCompLevelToAlg(iLevel) ) {
		case XPK_ALG_STORE:
			*pSizeRet = iRawSize;
			return XPK_OK;
		case XPK_ALG_LZ4:
		case XPK_ALG_LZ4HC:
			iBound = (size_t)LZ4_compressBound((int)iRawSize);
			break;
		case XPK_ALG_ZSTD:
			iBound = (size_t)ZSTD_compressBound((size_t)iRawSize);
			break;
		case XPK_ALG_LZMA2:
			iBound64 = (uint64_t)iRawSize + ((uint64_t)iRawSize / 100u) + 1025u;
			if ( iBound64 > UINT32_MAX ) {
				return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
			}
			*pSizeRet = (uint32_t)iBound64;
			return XPK_OK;
		default:
			return procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorCodecUnsupported);
	}

	if ( iBound > UINT32_MAX ) {
		return procXpkSetError(objXpk, XPK_ERR_STATE, sXpkErrorBadFormat);
	}

	*pSizeRet = (uint32_t)iBound;
	return XPK_OK;
}

static inline int procXpkCodecStoreCopy(xpkObject objXpk, const void* pData, uint32_t iRawSize, void** pBufRet, uint32_t* pSizeRet, uint8_t* pLevelRet)
{
	void* pBuf;

	*pBufRet = NULL;
	*pSizeRet = 0;
	*pLevelRet = 0;

	if ( iRawSize == 0 ) {
		return XPK_OK;
	}

	pBuf = xpkAllocInternal(iRawSize);
	if ( pBuf == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	memcpy(pBuf, pData, iRawSize);
	*pBufRet = pBuf;
	*pSizeRet = iRawSize;
	return XPK_OK;
}

static inline int procXpkCodecEncode(xpkObject objXpk, uint8_t iLevel, const void* pData, uint32_t iRawSize, void** pBufRet, uint32_t* pSizeRet, uint8_t* pLevelRet)
{
	uint32_t iBound;
	void* pBuf;
	int iAlg;
	int iRet;
	size_t iZstdRet;
	ZSTD_CCtx* pCctx;
	CLzma2EncHandle hLzma2;
	CLzma2EncProps objLzma2Props;
	Byte iPropByte;
	size_t iLzma2Size;
	SRes iLzmaRes;

	if ( (pBufRet == NULL) || (pSizeRet == NULL) || (pLevelRet == NULL) ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	*pBufRet = NULL;
	*pSizeRet = 0;
	*pLevelRet = iLevel;
	if ( iRawSize == 0 ) {
		return XPK_OK;
	}
	if ( pData == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}
	if ( iLevel == 0 ) {
		return procXpkCodecStoreCopy(objXpk, pData, iRawSize, pBufRet, pSizeRet, pLevelRet);
	}

	iRet = procXpkCodecBound(objXpk, iLevel, iRawSize, &iBound);
	if ( iRet != XPK_OK ) {
		return iRet;
	}

	pBuf = xpkAllocInternal(iBound);
	if ( pBuf == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	iAlg = procXpkCompLevelToAlg(iLevel);
	if ( iAlg == XPK_ALG_LZ4 ) {
		iRet = LZ4_compress_fast((const char*)pData, (char*)pBuf, (int)iRawSize, (int)iBound, procXpkCompLevelToNative(iLevel));
		if ( iRet <= 0 ) {
			xpkFreeInternal(pBuf);
			return procXpkSetError(objXpk, XPK_ERR_IO, "lz4 compress failed");
		}
		*pSizeRet = (uint32_t)iRet;
	} else if ( iAlg == XPK_ALG_LZ4HC ) {
		iRet = LZ4_compress_HC((const char*)pData, (char*)pBuf, (int)iRawSize, (int)iBound, procXpkCompLevelToNative(iLevel));
		if ( iRet <= 0 ) {
			xpkFreeInternal(pBuf);
			return procXpkSetError(objXpk, XPK_ERR_IO, "lz4hc compress failed");
		}
		*pSizeRet = (uint32_t)iRet;
	} else if ( iAlg == XPK_ALG_ZSTD ) {
		pCctx = ZSTD_createCCtx();
		if ( pCctx == NULL ) {
			xpkFreeInternal(pBuf);
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}
		ZSTD_CCtx_setParameter(pCctx, ZSTD_c_checksumFlag, 0);
		ZSTD_CCtx_setParameter(pCctx, ZSTD_c_strategy, (ZSTD_strategy)procXpkCompLevelToNative(iLevel));
		iZstdRet = ZSTD_compress2(pCctx, pBuf, (size_t)iBound, pData, (size_t)iRawSize);
		ZSTD_freeCCtx(pCctx);
		if ( ZSTD_isError(iZstdRet) ) {
			xpkFreeInternal(pBuf);
			return procXpkSetError(objXpk, XPK_ERR_IO, "zstd compress failed");
		}
		*pSizeRet = (uint32_t)iZstdRet;
	} else if ( iAlg == XPK_ALG_LZMA2 ) {
		hLzma2 = Lzma2Enc_Create(&g_Alloc, &g_BigAlloc);
		if ( hLzma2 == NULL ) {
			xpkFreeInternal(pBuf);
			return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
		}

		Lzma2EncProps_Init(&objLzma2Props);
		objLzma2Props.lzmaProps.level = procXpkCompLevelToNative(iLevel);
		iLzmaRes = Lzma2Enc_SetProps(hLzma2, &objLzma2Props);
		if ( iLzmaRes != SZ_OK ) {
			Lzma2Enc_Destroy(hLzma2);
			xpkFreeInternal(pBuf);
			return procXpkSetError(objXpk, XPK_ERR_IO, "lzma2 set props failed");
		}

		Lzma2Enc_SetDataSize(hLzma2, (UInt64)iRawSize);
		iPropByte = Lzma2Enc_WriteProperties(hLzma2);
		((Byte*)pBuf)[0] = iPropByte;
		iLzma2Size = (size_t)iBound - 1;
		iLzmaRes = Lzma2Enc_Encode2(hLzma2, NULL, (Byte*)pBuf + 1, &iLzma2Size, NULL, (const Byte*)pData, (size_t)iRawSize, NULL);
		Lzma2Enc_Destroy(hLzma2);
		if ( iLzmaRes != SZ_OK ) {
			xpkFreeInternal(pBuf);
			return procXpkSetError(objXpk, XPK_ERR_IO, "lzma2 compress failed");
		}
		*pSizeRet = (uint32_t)(iLzma2Size + 1);
	} else {
		xpkFreeInternal(pBuf);
		return procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorCodecUnsupported);
	}

	if ( *pSizeRet >= iRawSize ) {
		xpkFreeInternal(pBuf);
		return procXpkCodecStoreCopy(objXpk, pData, iRawSize, pBufRet, pSizeRet, pLevelRet);
	}

	*pBufRet = pBuf;
	return XPK_OK;
}

static inline int procXpkCodecDecode(xpkObject objXpk, uint8_t iLevel, const void* pData, uint32_t iCompSize, uint32_t iRawSize, void** pBufRet)
{
	void* pBuf;
	int iAlg;
	int iRet;
	size_t iZstdRet;
	Byte iPropByte;
	SizeT iDstLen;
	SizeT iSrcLen;
	ELzmaStatus iLzmaStatus;
	SRes iLzmaRes;

	if ( pBufRet == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_PARAM, sXpkErrorInvalidParam);
	}

	*pBufRet = NULL;
	if ( iRawSize == 0 ) {
		return XPK_OK;
	}
	if ( (pData == NULL) || (iCompSize == 0) ) {
		return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
	}

	pBuf = xpkAllocInternal(iRawSize);
	if ( pBuf == NULL ) {
		return procXpkSetError(objXpk, XPK_ERR_MEMORY, sXpkErrorOutOfMemory);
	}

	iAlg = procXpkCompLevelToAlg(iLevel);
	if ( iAlg == XPK_ALG_STORE ) {
		if ( iCompSize != iRawSize ) {
			xpkFreeInternal(pBuf);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}
		memcpy(pBuf, pData, iRawSize);
	} else if ( (iAlg == XPK_ALG_LZ4) || (iAlg == XPK_ALG_LZ4HC) ) {
		iRet = LZ4_decompress_safe((const char*)pData, (char*)pBuf, (int)iCompSize, (int)iRawSize);
		if ( iRet != (int)iRawSize ) {
			xpkFreeInternal(pBuf);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, "lz4 decompress failed");
		}
	} else if ( iAlg == XPK_ALG_ZSTD ) {
		iZstdRet = ZSTD_decompress(pBuf, (size_t)iRawSize, pData, (size_t)iCompSize);
		if ( ZSTD_isError(iZstdRet) || (iZstdRet != iRawSize) ) {
			xpkFreeInternal(pBuf);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, "zstd decompress failed");
		}
	} else if ( iAlg == XPK_ALG_LZMA2 ) {
		if ( iCompSize < 1 ) {
			xpkFreeInternal(pBuf);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, sXpkErrorBadFormat);
		}

		iPropByte = ((const Byte*)pData)[0];
		iDstLen = (SizeT)iRawSize;
		iSrcLen = (SizeT)(iCompSize - 1);
		iLzmaRes = Lzma2Decode((Byte*)pBuf, &iDstLen, (const Byte*)pData + 1, &iSrcLen, iPropByte, LZMA_FINISH_END, &iLzmaStatus, &g_Alloc);
		if ( (iLzmaRes != SZ_OK) || (iDstLen != iRawSize) ) {
			xpkFreeInternal(pBuf);
			return procXpkSetError(objXpk, XPK_ERR_FORMAT, "lzma2 decompress failed");
		}
	} else {
		xpkFreeInternal(pBuf);
		return procXpkSetError(objXpk, XPK_ERR_UNSUPPORTED, sXpkErrorCodecUnsupported);
	}

	*pBufRet = pBuf;
	return XPK_OK;
}
