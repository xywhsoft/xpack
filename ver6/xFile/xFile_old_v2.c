


#include <stdlib.h>
#include <windows.h>
#include "../xCore/xcore.h"
#include "xFile.h"



/* ------------------------------------ 工具函数 ------------------------------------ */

// 是否 utf-8 [猜测]
XXAPI int xCore_StringIsUtf8(char* sText, size_t iSize)
{
	size_t nBytes = 0;
	for ( size_t i = 0; i < iSize; i++ ) {
		char b = sText[i];
		if ( nBytes == 0 ) {
			if ( b >= 0x80 ) {
				if ( b >= 0xFC  && b <= 0xFD ) { nBytes = 6; }	// 此范围内为6字节UTF-8字符
				else if ( b >= 0xF8 ) { nBytes = 5; }				// 此范围内为5字节UTF-8字符
				else if ( b >= 0xF0 ) { nBytes = 4; }				// 此范围内为4字节UTF-8字符    
				else if ( b >= 0xE0 ) { nBytes = 3; }				// 此范围内为3字节UTF-8字符    
				else if ( b >= 0xC0 ) { nBytes = 2; }				// 此范围内为2字节UTF-8字符    
				else { return FALSE; }
				nBytes--;
			}
		} else {
			if ( (b & 0xC0) != 0x80 ) {
				// 值介于 0x80 与 0xC0 之间的为无效UTF-8字符
				return FALSE;
			}
			nBytes--;
		}
	}
	return TRUE;
}

// 是否 utf-7 [猜测]
XXAPI int xCore_StringIsUtf7(char* sText, size_t iSize)
{
	for ( size_t i = 0; i < iSize; i++ ) {
		if ( sText[i] & 0xC0 != 0 ) {
			return FALSE;
		}
	}
	return TRUE;
}

// 是否纯 ASCII [猜测]
XXAPI int xCore_StringIsAscii(char* sText, size_t iSize)
{
	for ( size_t i = 0; i < iSize; i++ ) {
		if ( (sText[i] & 0x80) != 0 ) {
			return FALSE;
		}
	}
	return TRUE;
}

// Unicode BE -> LE
XXAPI void xCore_UCS2_BE2LE(char* sText, size_t iSize)
{
	char c = 0;
	for ( size_t i = 0; i < iSize; i++ ) {
		if ( (i & 1) == 1 ) {
			sText[i-1] = sText[i];
			sText[i]  = c;
		} else {
			c =  sText[i];
		}
	}
}

// utf32 BE -> LE [仅单个字符]
XXAPI static inline size_t xCore_UCS4_BE2LE(size_t iChar)
{
	char b1 = ((char*)&iChar)[0];
	char b2 = ((char*)&iChar)[1];
	char b3 = ((char*)&iChar)[2];
	char b4 = ((char*)&iChar)[3];
	((char*)&iChar)[0] = b4;
	((char*)&iChar)[1] = b3;
	((char*)&iChar)[2] = b2;
	((char*)&iChar)[3] = b1;
	return iChar;
}

// utf32 -> Unicode
XXAPI size_t xCore_UCS4_To_UCS2(wchar_t* sText, size_t iSize, int iCharset)
{
	iSize = iSize  / 4;
	size_t iAddr = 0;
	size_t iChar;
	for ( size_t i = 0; i < iSize; i++ ) {
		iChar = ((size_t*)sText)[i];
		// BE -> LE
		if ( iCharset == CHARSET_UTF32_BE ) {
			iChar = xCore_UCS4_BE2LE(iChar);
		}
		// utf32 -> utf16
		if ( iChar <= 0xFFFF ) {
			sText[iAddr] = iChar;
			iAddr++;
		} else if ( iChar <= 0xEFFFF ) {
			sText[iAddr] = (0xD800 + (iChar >> 10) - 0x40);
			iAddr++;
			sText[iAddr] = (0xDC00 + (iChar & 0x03FF));
			iAddr++;
		}
	}
	sText[iAddr] = 0;
	return iAddr * 2;
}

// Unicode -> utf32
XXAPI char* xCore_UCS2_To_UCS4(wchar_t* sText, size_t iSize, int iCharset)
{
	iSize = iSize / 2;
	size_t iAddr = 0;
	size_t* sDest = malloc((iSize + 1) * 4);
	for ( size_t i = 0; i < iSize; i++ ) {
		wchar_t w1 = sText[i];
		if ( (w1 >= 0xD800) && (w1 <= 0xDFFF) ) {
			if ( w1 < 0xDC00 ) {
				wchar_t w2 = sText[i + 1];
				if ( (w2 >= 0xDC00) && (w2 <= 0xDFFF) ) {
					sDest[iAddr] = (w2 & 0x03FF) + (((w1 & 0x03FF) + 0x40) << 10);
					// BE -> LE
					if ( iCharset == CHARSET_UTF32_BE ) { sDest[iAddr] = xCore_UCS4_BE2LE(sDest[iAddr]); }
					iAddr++;
				}
			}
			i++;
		} else {
			sDest[iAddr] = w1;
			// BE -> LE
			if ( iCharset == CHARSET_UTF32_BE ) { sDest[iAddr] = xCore_UCS4_BE2LE(sDest[iAddr]); }
			iAddr++;
		}
	}
	sDest[iAddr] = 0;
	xCore.iRetSize = iAddr * 4;
	return (char*)sDest;
}



/* ------------------------------------ C 函数库 ------------------------------------ */

// 打开文件
XXAPI xFileObject xFile_OpenW(wstr sFile, int bReadOnly, int iCharset)
{
	HANDLE hFile = CreateFileW(sFile, bReadOnly ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, bReadOnly ? OPEN_EXISTING : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hFile == INVALID_HANDLE_VALUE ) { return NULL; }
	xFileObject objFile = malloc(sizeof(xFileStruct));
	if ( objFile == NULL ) { CloseHandle(hFile); return NULL; }
	objFile->Handle = hFile;
	objFile->Charset = iCharset;
	objFile->BOM = 0;
	// 自动识别文件编码
	char BOM[4];
	size_t iSize = GetFileSize(hFile, NULL);
	if ( iCharset == CHARSET_AUTO ) {
		if ( iSize == 0 ) {
			// 空文件设置为默认编码
			objFile->Charset = CHARSER_DEFAULT;
		} else {
			SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
			ReadFile(hFile, BOM, 4, &xCore.iRetSize, NULL);
			if ( (BOM[0] == 0xFF) && (BOM[1] == 0xFE) && (BOM[2] == 0x00) && (BOM[3] == 0x00) ) {
				// UTF-32 LE (little-endian)
				objFile->Charset = CHARSET_UTF32;
				objFile->BOM = 4;
			} else if ( (BOM[0] == 0x00) && (BOM[1] == 0x00) && (BOM[2] == 0xFE) && (BOM[3] == 0xFF) ) {
				// UTF-32 BE (big-endian)
				objFile->Charset = CHARSET_UTF32_BE;
				objFile->BOM = 4;
			} else if ( (BOM[0] == 0x84) && (BOM[1] == 0x31) && (BOM[2] == 0x95) && (BOM[3] == 0x33) ) {
				// GB 18030
				objFile->Charset = CHARSET_GB18030;
				objFile->BOM = 4;
			} else if ( (BOM[0] == 0xEF) && (BOM[1] == 0xBB) && (BOM[2] == 0xBF) ) {
				// UTF-8
				objFile->Charset = CHARSET_UTF8;
				objFile->BOM = 3;
			} else if ( (BOM[0] == 0xFF) && (BOM[1] == 0xFE) ) {
				// UTF-16 LE (little-endian)
				objFile->Charset = CHARSET_UNICODE;
				objFile->BOM = 2;
			} else if ( (BOM[0] == 0xFE) && (BOM[1] == 0xFF) ) {
				// UTF-16 BE (big-endian)
				objFile->Charset = CHARSET_UNICODE_BE;
				objFile->BOM = 2;
			} else {
				// 无 BOM 编码猜测 [选取最长64KB文本猜测]
				size_t iReadSize = iSize > 65536 ? 65536 : iSize;
				char* sText = malloc(iReadSize);
				SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
				ReadFile(hFile, sText, iReadSize, &xCore.iRetSize, NULL);
				if ( xCore_StringIsUtf7(sText, iReadSize) ) {
					// UTF7
					objFile->Charset = CHARSET_UTF7;
				} else if ( xCore_StringIsAscii(sText, iReadSize) ) {
					// US-ASCII
					objFile->Charset = CHARSET_ASCII;
				} else if ( xCore_StringIsUtf8(sText, iReadSize) ) {
					// UTF8 [无BOM]
					objFile->Charset = CHARSET_UTF8;
				} else {
					// 猜测不出来就设置为默认编码
					objFile->Charset = CHARSER_DEFAULT;
				}
				free(sText);
			}
		}
	} else {
		if ( iSize == 0 ) {
			// 0 字节文件自动添加文件头
			if ( bReadOnly == 0 ) {
				if ( iCharset == CHARSET_UNICODE ) {
					unsigned short fh = 0xFFFE;
					WriteFile(hFile, (void*)&fh, 2, &xCore.iRetSize, NULL);
					objFile->BOM = 2;
				} else if ( iCharset == CHARSET_UNICODE_BE ) {
					unsigned short fh = 0xFEFF;
					WriteFile(hFile, (void*)&fh, 2, &xCore.iRetSize, NULL);
					objFile->BOM = 2;
				} else if ( iCharset == CHARSET_UTF32 ) {
					unsigned int fh = 0xFFFE0000;
					WriteFile(hFile, (void*)&fh, 4, &xCore.iRetSize, NULL);
					objFile->BOM = 4;
				} else if ( iCharset == CHARSET_UTF32_BE ) {
					unsigned int fh = 0x0000FEFF;
					WriteFile(hFile, (void*)&fh, 4, &xCore.iRetSize, NULL);
					objFile->BOM = 4;
				} else if ( iCharset == CHARSET_GB18030 ) {
					unsigned int fh = 0x84319533;
					WriteFile(hFile, (void*)&fh, 4, &xCore.iRetSize, NULL);
					objFile->BOM = 4;
				}
			}
		} else {
			// 固定编码的文件检测并跳过文件头
			SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
			ReadFile(hFile, BOM, 4, &xCore.iRetSize, NULL);
			if ( iCharset == CHARSET_UNICODE ) {
				if ( (BOM[0] == 0xFF) && (BOM[1] == 0xFE) ) {
					objFile->BOM = 2;
				}
			} else if ( iCharset == CHARSET_UNICODE_BE ) {
				if ( (BOM[0] == 0xFE) && (BOM[1] == 0xFF) ) {
					objFile->BOM = 2;
				}
			} else if ( iCharset == CHARSET_UTF8 ) {
				if ( (BOM[0] == 0xEF) && (BOM[1] == 0xBB) && (BOM[2] == 0xBF) ) {
					objFile->BOM = 3;
				}
			} else if ( iCharset == CHARSET_UTF32 ) {
				if ( (BOM[0] == 0xFF) && (BOM[1] == 0xFE) && (BOM[2] == 0x00) && (BOM[3] == 0x00) ) {
					objFile->BOM = 4;
				}
			} else if ( iCharset == CHARSET_UTF32_BE ) {
				if ( (BOM[0] == 0x00) && (BOM[1] == 0x00) && (BOM[2] == 0xFE) && (BOM[3] == 0xFF) ) {
					objFile->BOM = 4;
				}
			}
		}
	}
	// 跳过文件 BOM
	SetFilePointer(hFile, objFile->BOM, NULL, FILE_BEGIN);
	return objFile;
}
XXAPI xFileObject xFile_OpenA(astr sFile, int bReadOnly, int iCharset)
{
	wstr sFileW = xCore_M2W(sFile, CP_ACP, 0);
	xFileObject objFile = xFile_OpenW(sFileW, bReadOnly, iCharset);
	free(sFileW);
	return objFile;
}

// 关闭文件
XXAPI int xFile_Close(xFileObject objFile)
{
	if (objFile && (objFile->Handle != INVALID_HANDLE_VALUE))
	{
		CloseHandle(objFile->Handle);
		free(objFile);
		return TRUE;
	}
	return FALSE;
}

// 获取文件编码
XXAPI int xFile_Charset(xFileObject objFile)
{
	if ( objFile && (objFile->Handle != INVALID_HANDLE_VALUE) ) {
		return objFile->Charset;
	}
	return CHARSET_ERROR;
}

// 设置文件读写位置 [FILE_BEGIN、FILE_CURRENT、FILE_END]
XXAPI int xFile_Seek(xFileObject objFile, int64 iPos, DWORD dwMoveMethod)
{
	if ( objFile && (objFile->Handle != INVALID_HANDLE_VALUE) ) {
		LARGE_INTEGER iPos_stu;
		iPos_stu.QuadPart = iPos;
		return SetFilePointerEx(objFile->Handle, iPos_stu, NULL, dwMoveMethod);
	}
	return 0;
}

// 获取文件读写位置
XXAPI uint64 xFile_CurPos(xFileObject objFile)
{
	if ( objFile && (objFile->Handle != INVALID_HANDLE_VALUE) ) {
		LARGE_INTEGER iPos_stu;
		iPos_stu.QuadPart = 0;
		SetFilePointerEx(objFile->Handle, iPos_stu, &iPos_stu, FILE_CURRENT);
		return iPos_stu.QuadPart;
	}
	return 0;
}

// 获取一个已打开文件的长度
XXAPI uint64 xFile_Size(xFileObject objFile)
{
	if ( objFile && (objFile->Handle != INVALID_HANDLE_VALUE) ) {
		LARGE_INTEGER iSize;
		GetFileSizeEx(objFile->Handle, &iSize);
		return iSize.QuadPart;
	}
	return 0;
}

// 是否已经读取到文件末尾
XXAPI int xFile_EOF(xFileObject objFile)
{
	if ( objFile && (objFile->Handle != INVALID_HANDLE_VALUE) ) {
		uint64 pAddr = xFile_CurPos(objFile);
		uint64 iSize = xFile_Size(objFile);
		return (pAddr >= iSize);
	}
	return TRUE;
}

// 设置文件在当前读写的位置结束
XXAPI int xFile_SetEOF(xFileObject objFile)
{
	if ( objFile && (objFile->Handle != INVALID_HANDLE_VALUE) ) {
		SetEndOfFile(objFile->Handle);
		return TRUE;
	}
	return FALSE;
}



// 向一个已打开的文件写入数据（pBuff为写入的字节数，而不是字符数！）
XXAPI ulong xFile_WriteW(xFileObject objFile, wstr pBuff, ulong iSize)
{
	if ( (pBuff == NULL) || (objFile == NULL) || (objFile->Handle == INVALID_HANDLE_VALUE) ) { return 0; }
	if ( iSize == 0 ) { iSize = wcslen(pBuff) * sizeof(wchar_t); }
	if ( iSize == 0 ) { return 0; }
	// 转换编码 [将数据转换为目标编码写入]
	ptr pOut = pBuff;
	char* sAutoFree = NULL;
	if ( (objFile->Charset < 0) || (objFile->Charset == CHARSET_UNICODE) ) {
		// 以二进制格式直接写入
	} else if ( objFile->Charset == CHARSET_UNICODE_BE ) {
		// 将 Unicode LE 转换为 Unicode BE
		xCore_UCS2_LE2BE(pOut, iSize);
	} else if ( (objFile->Charset == CHARSET_UTF32) || (objFile->Charset == CHARSET_UTF32_BE) ) {
		// 将 UTF16 转换为 UTF32
		pOut = xCore_UCS2_To_UCS4(pOut, iSize, objFile->Charset);
		iSize = xCore.iRetSize;
		sAutoFree = pOut;
	} else {
		// 其他编码转换一下
		pOut = xCore_W2M(pBuff, objFile->Charset, iSize);
		iSize = xCore.iRetSize;
		sAutoFree = pOut;
	}
	// 写入转换后的数据
	ulong iWriteBytes;
	int bRet = WriteFile(objFile->Handle, pOut, iSize, &iWriteBytes, NULL);
	if (sAutoFree) { free(sAutoFree); }
	if ( bRet ) { return iWriteBytes; } else { return 0; }
}
XXAPI ulong xFile_WriteA(xFileObject objFile, astr pBuff, ulong iSize)
{
	if ( (pBuff == NULL) || (objFile == NULL) || (objFile->Handle == INVALID_HANDLE_VALUE) ) { return 0; }
	if ( iSize == 0 ) { iSize = strlen(pBuff); }
	if ( iSize == 0 ) { return 0; }
	// 转换编码 [将数据转换为目标编码写入]
	ptr pOut = pBuff;
	char* sAutoFree = NULL;
	if ( (objFile->Charset < 0) || (objFile->Charset == CHARSET_ASCII) || (objFile->Charset == CHARSET_OEMCP) || (objFile->Charset == CHARSET_ACP) ) {
		// 以二进制格式直接写入
	} else {
		// 先将编码转换为 Unicode
		sAutoFree = (char*)xCore_M2W(pBuff, CHARSET_OEMCP, iSize);
		iSize = xCore.iRetSize;
		if ( objFile->Charset == CHARSET_UNICODE_BE ) {
			// 将 Unicode LE 转换为 Unicode BE
			xCore_UCS2_LE2BE(sAutoFree, iSize);
			pOut = sAutoFree;
		} else if ( objFile->Charset == CHARSET_UNICODE ) {
			// Unicode 不做额外处理
			pOut = sAutoFree;
		} else if ( (objFile->Charset == CHARSET_UTF32) || (objFile->Charset == CHARSET_UTF32_BE) ) {
			// 将 UTF16 转换为 UTF32
			pOut = xCore_UCS2_To_UCS4((wstr)sAutoFree, iSize, objFile->Charset);
			iSize = xCore.iRetSize;
			free(sAutoFree);
			sAutoFree = pOut;
		} else {
			// 其他编码转换后保存
			pOut = xCore_W2M((wstr)sAutoFree, objFile->Charset, iSize);
			iSize = xCore.iRetSize;
			free(sAutoFree);
			sAutoFree = pOut;
		}
	}
	// 写入转换后的数据
	ulong iWriteBytes;
	int bRet = WriteFile(objFile->Handle, pOut, iSize, &iWriteBytes, NULL);
	if (sAutoFree) { free(sAutoFree); }
	if ( bRet ) { return iWriteBytes; } else { return 0; }
}
XXAPI ulong xFile_WriteU(xFileObject objFile, ustr pBuff, ulong iSize)
{
	if ( (pBuff == NULL) || (objFile == NULL) || (objFile->Handle == INVALID_HANDLE_VALUE) ) { return 0; }
	if ( iSize == 0 ) { iSize = strlen(pBuff); }
	if ( iSize == 0 ) { return 0; }
	// 转换编码 [将数据转换为目标编码写入]
	ptr pOut = pBuff;
	char* sAutoFree = NULL;
	if ( (objFile->Charset < 0) || (objFile->Charset == CHARSET_UTF8) ) {
		// 以二进制格式直接写入
	} else {
		// 先将编码转换为 Unicode
		sAutoFree = (char*)xCore_M2W(pBuff, CHARSET_UTF8, iSize);
		iSize = xCore.iRetSize;
		if ( objFile->Charset == CHARSET_UNICODE_BE ) {
			// 将 Unicode LE 转换为 Unicode BE
			xCore_UCS2_LE2BE(sAutoFree, iSize);
			pOut = sAutoFree;
		} else if ( objFile->Charset == CHARSET_UNICODE ) {
			// Unicode 不做额外处理
			pOut = sAutoFree;
		} else if ( (objFile->Charset == CHARSET_UTF32) || (objFile->Charset == CHARSET_UTF32_BE) ) {
			// 将 UTF16 转换为 UTF32
			pOut = xCore_UCS2_To_UCS4((wstr)sAutoFree, iSize, objFile->Charset);
			iSize = xCore.iRetSize;
			free(sAutoFree);
			sAutoFree = pOut;
		} else {
			// 其他编码转换后保存
			pOut = xCore_W2M((wstr)sAutoFree, objFile->Charset, iSize);
			iSize = xCore.iRetSize;
			free(sAutoFree);
			sAutoFree = pOut;
		}
	}
	// 写入转换后的数据
	ulong iWriteBytes;
	int bRet = WriteFile(objFile->Handle, pOut, iSize, &iWriteBytes, NULL);
	if ( sAutoFree ) { free(sAutoFree); }
	if ( bRet ) { return iWriteBytes; } else { return 0; }
}

// 从一个已打开的文件读取数据（数据需使用xCore_free释放）
XXAPI wstr xFile_ReadW(xFileObject objFile, ulong iSize)
{
	if ( (objFile == NULL) || (objFile->Handle == INVALID_HANDLE_VALUE) ) { xCore.iRetSize = 0; return (wstr)xCore.nullstring; }
	if ( iSize == 0 ) { xCore.iRetSize = 0; return (wstr)xCore.nullstring; }
	astr pBuff = malloc(iSize + 4);
	if ( pBuff == 0 ) { xCore.iRetSize = 0; return (wstr)xCore.nullstring; }
	if ( ReadFile(objFile->Handle, pBuff, iSize, &xCore.iRetSize, NULL) ) {
		// 读取到的文件可能是 utf-32 编码，所以留4个空字节做结尾
		pBuff[xCore.iRetSize] = 0;
		pBuff[xCore.iRetSize+1] = 0;
		pBuff[xCore.iRetSize+2] = 0;
		pBuff[xCore.iRetSize+3] = 0;
		// 转换编码 [将读取到的数据转换为 unicode 编码]
		if ( (objFile->Charset >= 0) && (objFile->Charset != CHARSET_UNICODE) ) {
			if ( (objFile->Charset == CHARSET_UTF32) || (objFile->Charset == CHARSET_UTF32_BE) ) {
				// 将 UTF32 转换为 UTF16
				iSize = xCore_UCS4_To_UCS2((wstr)pBuff, xCore.iRetSize, objFile->Charset);
			} else if ( objFile->Charset == CHARSET_UNICODE_BE ) {
				// 将 Unicode BE 转换为 Unicode LE
				xCore_UCS2_BE2LE(pBuff, xCore.iRetSize);
			} else if ( (objFile->Charset != CHARSET_UNICODE) && (objFile->Charset != CHARSET_UNICODE_BE) ) {
				// 其他编码转换为 UTF16
				wstr sRet = xCore_M2W(pBuff, objFile->Charset, xCore.iRetSize);
				free(pBuff);
				pBuff = (astr)sRet;
				iSize = xCore.iRetSize;
			}
		}
		xCore.iRetSize = iSize;
		return (wstr)pBuff;
	}
	free(pBuff);
	xCore.iRetSize = 0;
	return (wstr)xCore.nullstring;
}
XXAPI astr xFile_ReadA(xFileObject objFile, ulong iSize)
{
	if ( (objFile == NULL) || (objFile->Handle == INVALID_HANDLE_VALUE) ) { xCore.iRetSize = 0; return xCore.nullstring; }
	if ( iSize == 0 ) { xCore.iRetSize = 0; return xCore.nullstring; }
	astr pBuff = malloc(iSize + 4);
	if ( pBuff == 0 ) { xCore.iRetSize = 0; return xCore.nullstring; }
	if ( ReadFile(objFile->Handle, pBuff, iSize, &xCore.iRetSize, NULL) ) {
		// 读取到的文件可能是 utf-32 编码，所以留4个空字节做结尾
		pBuff[xCore.iRetSize] = 0;
		pBuff[xCore.iRetSize+1] = 0;
		pBuff[xCore.iRetSize+2] = 0;
		pBuff[xCore.iRetSize+3] = 0;
		// 转换编码 [将读取到的数据转换为 OEM 编码]
		if ( (objFile->Charset >= 0) && (objFile->Charset != CHARSET_ASCII) && (objFile->Charset != CHARSET_OEMCP) && (objFile->Charset != CHARSET_ACP) ) {
			if ( (objFile->Charset == CHARSET_UTF32) || (objFile->Charset == CHARSET_UTF32_BE) ) {
				// 将 UTF32 转换为 UTF16
				iSize = xCore_UCS4_To_UCS2((wstr)pBuff, xCore.iRetSize, objFile->Charset);
			} else if ( objFile->Charset == CHARSET_UNICODE_BE ) {
				// 将 Unicode BE 转换为 Unicode LE
				xCore_UCS2_BE2LE(pBuff, xCore.iRetSize);
			} else if ( (objFile->Charset != CHARSET_UNICODE) && (objFile->Charset != CHARSET_UNICODE_BE) ) {
				// 其他编码转换为 UTF16
				wstr sRet = xCore_M2W(pBuff, objFile->Charset, xCore.iRetSize);
				free(pBuff);
				pBuff = (astr)sRet;
				iSize = xCore.iRetSize;
			}
			// 最后将 UTF16 转换为 OEM 编码
			astr sRet = xCore_W2M((wstr)pBuff, CHARSET_OEMCP, xCore.iRetSize);
			free(pBuff);
			pBuff = sRet;
			iSize = xCore.iRetSize;
		}
		xCore.iRetSize = iSize;
		return pBuff;
	}
	free(pBuff);
	xCore.iRetSize = 0;
	return xCore.nullstring;
}
XXAPI ustr xFile_ReadU(xFileObject objFile, ulong iSize)
{
	if ( (objFile == NULL) || (objFile->Handle == INVALID_HANDLE_VALUE) ) { xCore.iRetSize = 0; return xCore.nullstring; }
	if ( iSize == 0 ) { xCore.iRetSize = 0; return xCore.nullstring; }
	astr pBuff = malloc(iSize + 4);
	if ( pBuff == 0 ) { xCore.iRetSize = 0; return xCore.nullstring; }
	if ( ReadFile(objFile->Handle, pBuff, iSize, &xCore.iRetSize, NULL) ) {
		// 读取到的文件可能是 utf-32 编码，所以留4个空字节做结尾
		pBuff[xCore.iRetSize] = 0;
		pBuff[xCore.iRetSize+1] = 0;
		pBuff[xCore.iRetSize+2] = 0;
		pBuff[xCore.iRetSize+3] = 0;
		// 转换编码 [将读取到的数据转换为 OEM 编码]
		if ( (objFile->Charset >= 0) && (objFile->Charset != CHARSET_UTF8) ) {
			if ( (objFile->Charset == CHARSET_UTF32) || (objFile->Charset == CHARSET_UTF32_BE) ) {
				// 将 UTF32 转换为 UTF16
				iSize = xCore_UCS4_To_UCS2((wstr)pBuff, xCore.iRetSize, objFile->Charset);
			} else if ( objFile->Charset == CHARSET_UNICODE_BE ) {
				// 将 Unicode BE 转换为 Unicode LE
				xCore_UCS2_BE2LE(pBuff, xCore.iRetSize);
			} else if ( (objFile->Charset != CHARSET_UNICODE) && (objFile->Charset != CHARSET_UNICODE_BE) ) {
				// 其他编码转换为 UTF16
				wstr sRet = xCore_M2W(pBuff, objFile->Charset, xCore.iRetSize);
				free(pBuff);
				pBuff = (astr)sRet;
				iSize = xCore.iRetSize;
			}
			// 最后将 UTF16 转换为 UTF8 编码
			ustr sRet = xCore_W2M((wstr)pBuff, CHARSET_UTF8, xCore.iRetSize);
			free(pBuff);
			pBuff = sRet;
			iSize = xCore.iRetSize;
		}
		xCore.iRetSize = iSize;
		return pBuff;
	}
	free(pBuff);
	xCore.iRetSize = 0;
	return xCore.nullstring;
}

// 向一个已打开的文件写入二进制数据
XXAPI ulong xFile_Put(xFileObject objFile, ptr pBuffer, ulong iSize)
{
	if ( objFile && (objFile->Handle != INVALID_HANDLE_VALUE) ) {
		if ( WriteFile(objFile->Handle, pBuffer, iSize, &xCore.iRetSize, NULL) ) {
			return xCore.iRetSize;
		}
	}
	xCore.iRetSize = 0;
	return 0;
}

// 从一个已打开的文件读取二进制数据
XXAPI ulong xFile_Get(xFileObject objFile, ptr pBuffer, ulong iSize)
{
	if ( objFile && (objFile->Handle != INVALID_HANDLE_VALUE) ) {
		if ( ReadFile(objFile->Handle, pBuffer, iSize, &xCore.iRetSize, NULL) ) {
			return xCore.iRetSize;
		}
	}
	xCore.iRetSize = 0;
	return 0;
}



// 判断文件是否存在
XXAPI int xFile_ExistsW(const wstr sFile)
{
	HANDLE hFile = CreateFileW(sFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hFile != INVALID_HANDLE_VALUE ) {
		CloseHandle(hFile);
		return TRUE;
	} else {
		if ( GetLastError() == 32 ) { return TRUE; }
	}
	return FALSE;
}
XXAPI int xFile_ExistsA(const astr sFile)
{
	HANDLE hFile = CreateFileA(sFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hFile != INVALID_HANDLE_VALUE ) {
		CloseHandle(hFile);
		return TRUE;
	} else {
		if ( GetLastError() == 32 ) { return TRUE; }
	}
	return FALSE;
}

// 判断路径是否存在
XXAPI int xFile_PathExistsW(const wstr sFile)
{
	return ( GetFileAttributesW(sFile) != INVALID_FILE_ATTRIBUTES );
}
XXAPI int xFile_PathExistsA(const astr sFile)
{
	return ( GetFileAttributesA(sFile) != INVALID_FILE_ATTRIBUTES );
}

// 判断目录是否存在
XXAPI int xFile_FolderExistsW(const wstr sFile)
{
	WIN32_FILE_ATTRIBUTE_DATA wfad = { 0 };
	DWORD iRet = GetFileAttributesExW(sFile, GetFileExInfoStandard, &wfad);
	if ( iRet == 0 ) { return FALSE; }
	if ( wfad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
		return TRUE;
	} else {
		return FALSE;
	}
}
XXAPI int xFile_FolderExistsA(const astr sFile)
{
	WIN32_FILE_ATTRIBUTE_DATA wfad = { 0 };
	DWORD iRet = GetFileAttributesExA(sFile, GetFileExInfoStandard, &wfad);
	if ( iRet == 0 ) { return FALSE; }
	if ( wfad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
		return TRUE;
	} else {
		return FALSE;
	}
}



// 获取文件长度
XXAPI uint64 xFile_GetSizeW(const wstr sFile)
{
	HANDLE hFile = CreateFileW(sFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hFile && hFile != INVALID_HANDLE_VALUE ) {
		LARGE_INTEGER iSize;
		GetFileSizeEx(hFile, &iSize);
		return iSize.QuadPart;
	}
	return 0;
}
XXAPI uint64 xFile_GetSizeA(const astr sFile)
{
	HANDLE hFile = CreateFileA(sFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hFile && hFile != INVALID_HANDLE_VALUE ) {
		LARGE_INTEGER iSize;
		GetFileSizeEx(hFile, &iSize);
		return iSize.QuadPart;
	}
	return 0;
}
XXAPI uint64 xFile_GetSizeU(const ustr sFile)
{
	wstr sFileW = xCore_U2W(sFile, 0);
	HANDLE hFile = CreateFileW(sFileW, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	xCore_free(sFileW);
	if ( hFile && hFile != INVALID_HANDLE_VALUE ) {
		LARGE_INTEGER iSize;
		GetFileSizeEx(hFile, &iSize);
		return iSize.QuadPart;
	}
	return 0;
}

// 设置文件长度
XXAPI int xFile_SetSizeW(const wstr sFile, uint64 iSize)
{
	HANDLE hFile = CreateFileW(sFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hFile && hFile != INVALID_HANDLE_VALUE ) {
		LARGE_INTEGER iPos_stu;
		iPos_stu.QuadPart = iSize;
		SetFilePointerEx(hFile, iPos_stu, NULL, FILE_BEGIN);
		SetEndOfFile(hFile);
		CloseHandle(hFile);
		return TRUE;
	}
	return FALSE;
}
XXAPI int xFile_SetSizeA(const astr sFile, uint64 iSize)
{
	HANDLE hFile = CreateFileA(sFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hFile && hFile != INVALID_HANDLE_VALUE ) {
		LARGE_INTEGER iPos_stu;
		iPos_stu.QuadPart = iSize;
		SetFilePointerEx(hFile, iPos_stu, NULL, FILE_BEGIN);
		SetEndOfFile(hFile);
		CloseHandle(hFile);
		return TRUE;
	}
	return FALSE;
}
XXAPI int xFile_SetSizeU(const ustr sFile, uint64 iSize)
{
	wstr sFileW = xCore_U2W(sFile, 0);
	HANDLE hFile = CreateFileW(sFileW, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	xCore_free(sFileW);
	if ( hFile && hFile != INVALID_HANDLE_VALUE ) {
		LARGE_INTEGER iPos_stu;
		iPos_stu.QuadPart = iSize;
		SetFilePointerEx(hFile, iPos_stu, NULL, FILE_BEGIN);
		SetEndOfFile(hFile);
		CloseHandle(hFile);
		return TRUE;
	}
	return FALSE;
}

// 向一个文件追加写入数据
XXAPI ulong xFile_AppendW(wstr sFile, wstr pBuff, ulong iSize, int iCharset)
{
	if ( pBuff == NULL ) { return 0; }
	xFileObject objFile = xFile_OpenW(sFile, FALSE, iCharset);
	if ( objFile ) {
		if ( iSize == 0 ) { iSize = wcslen(pBuff); }
		if ( iSize == 0 ) { return 0; }
		xFile_Seek(objFile, 0, FILE_END);
		ulong iRet = xFile_WriteW(objFile, pBuff, iSize);
		xFile_Close(objFile);
		return iRet;
	}
	return 0;
}
XXAPI ulong xFile_AppendA(astr sFile, astr pBuff, ulong iSize, int iCharset)
{
	if ( pBuff == NULL ) { return 0; }
	xFileObject objFile = xFile_OpenA(sFile, FALSE, iCharset);
	if ( objFile ) {
		if ( iSize == 0 ) { iSize = strlen(pBuff); }
		if ( iSize == 0 ) { return 0; }
		xFile_Seek(objFile, 0, FILE_END);
		ulong iRet = xFile_WriteA(objFile, pBuff, iSize);
		xFile_Close(objFile);
		return iRet;
	}
	return 0;
}
XXAPI ulong xFile_AppendU(ustr sFile, ustr pBuff, ulong iSize, int iCharset)
{
	if ( pBuff == NULL ) { return 0; }
	wstr sFileW = xCore_U2W(sFile, 0);
	xFileObject objFile = xFile_OpenW(sFileW, FALSE, iCharset);
	xCore_free(sFileW);
	if ( objFile ) {
		if ( iSize == 0 ) { iSize = strlen(pBuff); }
		if ( iSize == 0 ) { return 0; }
		xFile_Seek(objFile, 0, FILE_END);
		ulong iRet = xFile_WriteU(objFile, pBuff, iSize);
		xFile_Close(objFile);
		return iRet;
	}
	return 0;
}

// 将一个字符串写入文件，并覆盖原有的内容
XXAPI ulong xFile_WriteAllW(wstr sFile, wstr pBuff, ulong iSize, int iCharset)
{
	if ( pBuff == NULL ) { return 0; }
	xFileObject objFile = xFile_OpenW(sFile, FALSE, iCharset);
	if ( objFile ) {
		if ( iSize == 0 ) { iSize = wcslen(pBuff); }
		if ( iSize == 0 ) { return 0; }
		ulong iRet = xFile_WriteW(objFile, pBuff, iSize);
		xFile_SetEOF(objFile);
		xFile_Close(objFile);
		return iRet;
	}
	return 0;
}
XXAPI ulong xFile_WriteAllA(astr sFile, astr pBuff, ulong iSize, int iCharset)
{
	if ( pBuff == NULL ) { return 0; }
	xFileObject objFile = xFile_OpenA(sFile, FALSE, iCharset);
	if ( objFile ) {
		if ( iSize == 0 ) { iSize = strlen(pBuff); }
		if ( iSize == 0 ) { return 0; }
		ulong iRet = xFile_WriteA(objFile, pBuff, iSize);
		xFile_SetEOF(objFile);
		xFile_Close(objFile);
		return iRet;
	}
	return 0;
}
XXAPI ulong xFile_WriteAllU(ustr sFile, ustr pBuff, ulong iSize, int iCharset)
{
	if ( pBuff == NULL ) { return 0; }
	wstr sFileW = xCore_U2W(sFile, 0);
	xFileObject objFile = xFile_OpenW(sFileW, FALSE, iCharset);
	xCore_free(sFileW);
	if ( objFile ) {
		if ( iSize == 0 ) { iSize = strlen(pBuff); }
		if ( iSize == 0 ) { return 0; }
		ulong iRet = xFile_WriteU(objFile, pBuff, iSize);
		xFile_SetEOF(objFile);
		xFile_Close(objFile);
		return iRet;
	}
	return 0;
}

// 从一个文件读出全部数据（数据需使用xCore.free释放）
XXAPI wstr xFile_ReadAllW(wstr sFile, int iCharset)
{
	xFileObject objFile = xFile_OpenW(sFile, TRUE, iCharset);
	if ( objFile ) {
		uint64 iSize = xFile_Size(objFile) - objFile->BOM;
		wstr sRet = (wstr)xCore.nullstring;
		if ( iSize > 0 ) { sRet = xFile_ReadW(objFile, iSize - objFile->BOM); }
		xFile_Close(objFile);
		return sRet;
	}
	xCore.iRetSize = 0;
	return (wchar_t*)xCore.nullstring;
}
XXAPI astr xFile_ReadAllA(astr sFile, int iCharset)
{
	xFileObject objFile = xFile_OpenA(sFile, TRUE, iCharset);
	if ( objFile ) {
		uint64 iSize = xFile_Size(objFile) - objFile->BOM;
		astr sRet = xCore.nullstring;
		if ( iSize > 0 ) { sRet = xFile_ReadA(objFile, iSize - objFile->BOM); }
		xFile_Close(objFile);
		return sRet;
	}
	xCore.iRetSize = 0;
	return xCore.nullstring;
}
XXAPI astr xFile_ReadAllU(ustr sFile, int iCharset)
{
	wstr sFileW = xCore_U2W(sFile, 0);
	xFileObject objFile = xFile_OpenW(sFileW, TRUE, iCharset);
	xCore_free(sFileW);
	if ( objFile ) {
		uint64 iSize = xFile_Size(objFile) - objFile->BOM;
		astr sRet = xCore.nullstring;
		if ( iSize > 0 ) { sRet = xFile_ReadU(objFile, iSize - objFile->BOM); }
		xFile_Close(objFile);
		return sRet;
	}
	xCore.iRetSize = 0;
	return xCore.nullstring;
}

// 将一段数据写入文件，并覆盖原有的内容
XXAPI ulong xFile_PutAllW(wstr sFile, ptr pBuff, ulong iSize)
{
	HANDLE hFile = CreateFileW(sFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hFile == INVALID_HANDLE_VALUE ) { return 0; }
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	xCore.iRetSize = 0;
	WriteFile(hFile, pBuff, iSize, &xCore.iRetSize, NULL);
	SetEndOfFile(hFile);
	CloseHandle(hFile);
	return xCore.iRetSize;
}
XXAPI ulong xFile_PutAllA(astr sFile, ptr pBuff, ulong iSize)
{
	HANDLE hFile = CreateFileA(sFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hFile == INVALID_HANDLE_VALUE ) { return 0; }
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	xCore.iRetSize = 0;
	WriteFile(hFile, pBuff, iSize, &xCore.iRetSize, NULL);
	SetEndOfFile(hFile);
	CloseHandle(hFile);
	return xCore.iRetSize;
}
XXAPI ulong xFile_PutAllU(ustr sFile, ptr pBuff, ulong iSize)
{
	wstr sFileW = xCore_U2W(sFile, 0);
	HANDLE hFile = CreateFileW(sFileW, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	xCore_free(sFileW);
	if ( hFile == INVALID_HANDLE_VALUE ) { return 0; }
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	xCore.iRetSize = 0;
	WriteFile(hFile, pBuff, iSize, &xCore.iRetSize, NULL);
	SetEndOfFile(hFile);
	CloseHandle(hFile);
	return xCore.iRetSize;
}

// 将一段数据写入文件，并覆盖原有的内容（内存需使用xCore.Free释放）
XXAPI ptr xFile_GetAllW(wstr sFile)
{
	HANDLE hFile = CreateFileW(sFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hFile == INVALID_HANDLE_VALUE ) { return 0; }
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	xCore.iRetSize = GetFileSize(hFile, NULL);
	ptr pBuff = malloc(xCore.iRetSize);
	ReadFile(hFile, pBuff, xCore.iRetSize, &xCore.iRetSize, NULL);
	CloseHandle(hFile);
	return pBuff;
}
XXAPI ptr xFile_GetAllA(astr sFile)
{
	HANDLE hFile = CreateFileA(sFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hFile == INVALID_HANDLE_VALUE ) { return 0; }
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	xCore.iRetSize = GetFileSize(hFile, NULL);
	ptr pBuff = malloc(xCore.iRetSize);
	ReadFile(hFile, pBuff, xCore.iRetSize, &xCore.iRetSize, NULL);
	CloseHandle(hFile);
	return pBuff;
}
XXAPI ptr xFile_GetAllU(ustr sFile)
{
	wstr sFileW = xCore_U2W(sFile, 0);
	HANDLE hFile = CreateFileW(sFileW, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	xCore_free(sFileW);
	if ( hFile == INVALID_HANDLE_VALUE ) { return 0; }
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	xCore.iRetSize = GetFileSize(hFile, NULL);
	ptr pBuff = malloc(xCore.iRetSize);
	ReadFile(hFile, pBuff, xCore.iRetSize, &xCore.iRetSize, NULL);
	CloseHandle(hFile);
	return pBuff;
}



// 查找文件
XXAPI int xFile_ScanA(astr sRoot, astr sFilter, int iAttrib, int iAttribEx, int bRecu, ptr pProc, ptr iParam)
{
	WIN32_FIND_DATA objFindData;
	int iFileCount = 0;
	int iSize = strlen(sRoot);
	int (*pCallBack)(char* sRootPath, WIN32_FIND_DATA* pFindData, ptr iParam) = pProc;
	if ( strlen(sFilter) == 0 ) {
		return 0;
	}
	// 遍历文件
	if ( (sRoot[iSize-1] != 92) && (sRoot[iSize-1] != 47) ) {
		sRoot = xCore_FormatA("%s\\", sRoot);
	}
	char* sPath = xCore_FormatA("%s%s", sRoot, sFilter);
	HANDLE hFind = FindFirstFile(sPath, &objFindData);
	xCore.iRetSize = 0;
	if ( hFind != INVALID_HANDLE_VALUE ) {
		int bOver = TRUE;
		while ( bOver ) {
			if ( ( objFindData.dwFileAttributes & ( iAttrib | FILE_ATTRIBUTE_DIRECTORY ) ) == 0 ) {
				// 查找到文件
				if ( (iAttribEx & FILE_SCAN_NOFILE) == 0 ) {
					if ( pProc ) {
						xCore.iRetSize = pCallBack(sRoot, &objFindData, iParam);
						if ( xCore.iRetSize ) {
							break;
						}
					}
					iFileCount++;
				}
			}
			bOver = FindNextFile(hFind, &objFindData);
		}
	}
	FindClose(hFind);
	if ( xCore.iRetSize ) {
		return iFileCount;
	}
	// 递归子目录
	sPath = xCore_FormatA("%s*", sRoot);
	HANDLE hFindFolder = FindFirstFile(sPath, &objFindData);
	if ( hFindFolder != INVALID_HANDLE_VALUE ) {
		int bOver = TRUE;
		while ( bOver ) {
			if ( objFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
				// 过滤当前文件夹和父文件夹
				if ( ( strcmp(objFindData.cFileName, ".") == 0 ) || ( strcmp(objFindData.cFileName, "..") == 0 ) ) {
					if ( iAttribEx & FILE_SCAN_POINTFOLDER ) {
						if ( pProc ) {
							xCore.iRetSize = pCallBack(sRoot, &objFindData, iParam);
							if ( xCore.iRetSize ) {
								break;
							}
						}
						iFileCount++;
					}
				} else {
					// 查找到文件夹
					if ( (iAttribEx & FILE_SCAN_NOFOLDER) == 0 ) {
						if ( pProc ) {
							xCore.iRetSize = pCallBack(sRoot, &objFindData, iParam);
							if ( xCore.iRetSize ) {
								break;
							}
						}
					}
					iFileCount++;
					// 递归
					if ( bRecu ) {
						sPath = xCore_FormatA("%s%s\\", sRoot, objFindData.cFileName);
						iFileCount += xFile_ScanA(sPath, sFilter, iAttrib, iAttribEx, bRecu, pProc, iParam);
						if ( xCore.iRetSize ) {
							break;
						}
					}
				}
			}
			bOver = FindNextFile(hFindFolder, &objFindData);
		}
	}
	FindClose(hFindFolder);
	return iFileCount;
}
XXAPI int xFile_ScanW(wstr sRoot, wstr sFilter, int iAttrib, int iAttribEx, int bRecu, ptr pProc, ptr iParam)
{
	WIN32_FIND_DATAW objFindData;
	int iFileCount = 0;
	int iSize = wcslen(sRoot);
	int (*pCallBack)(wchar_t* sRootPath, WIN32_FIND_DATAW* pFindData, ptr iParam) = pProc;
	if ( wcslen(sFilter) == 0 ) {
		return 0;
	}
	// 遍历文件
	if ( (sRoot[iSize-1] != 92) && (sRoot[iSize-1] != 47) ) {
		sRoot = xCore_FormatW(L"%s\\", sRoot);
	}
	wchar_t* sPath = xCore_FormatW(L"%s%s", sRoot, sFilter);
	HANDLE hFind = FindFirstFileW(sPath, &objFindData);
	xCore.iRetSize = 0;
	if ( hFind != INVALID_HANDLE_VALUE ) {
		int bOver = TRUE;
		while ( bOver ) {
			if ( ( objFindData.dwFileAttributes & ( iAttrib | FILE_ATTRIBUTE_DIRECTORY ) ) == 0 ) {
				// 查找到文件
				if ( (iAttribEx & FILE_SCAN_NOFILE) == 0 ) {
					if ( pProc ) {
						xCore.iRetSize = pCallBack(sRoot, &objFindData, iParam);
						if ( xCore.iRetSize ) {
							break;
						}
					}
					iFileCount++;
				}
			}
			bOver = FindNextFileW(hFind, &objFindData);
		}
	}
	FindClose(hFind);
	if ( xCore.iRetSize ) {
		return iFileCount;
	}
	// 递归子目录
	sPath = xCore_FormatW(L"%s*", sRoot);
	HANDLE hFindFolder = FindFirstFileW(sPath, &objFindData);
	xCore.iRetSize = 0;
	if ( hFindFolder != INVALID_HANDLE_VALUE ) {
		int bOver = TRUE;
		while ( bOver ) {
			if ( objFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
				// 过滤当前文件夹和父文件夹
				if ( ( wcscmp(objFindData.cFileName, L".") == 0 ) || ( wcscmp(objFindData.cFileName, L"..") == 0 ) ) {
					if ( iAttribEx & FILE_SCAN_POINTFOLDER ) {
						if ( pProc ) {
							xCore.iRetSize = pCallBack(sRoot, &objFindData, iParam);
							if ( xCore.iRetSize ) {
								break;
							}
						}
						iFileCount++;
					}
				} else {
					// 查找到文件夹
					if ( (iAttribEx & FILE_SCAN_NOFOLDER) == 0 ) {
						if ( pProc ) {
							xCore.iRetSize = pCallBack(sRoot, &objFindData, iParam);
							if ( xCore.iRetSize ) {
								break;
							}
						}
					}
					iFileCount++;
					// 递归
					if ( bRecu ) {
						sPath = xCore_FormatW(L"%s%s\\", sRoot, objFindData.cFileName);
						iFileCount += xFile_ScanW(sPath, sFilter, iAttrib, iAttribEx, bRecu, pProc, iParam);
						if ( xCore.iRetSize ) {
							break;
						}
					}
				}
			}
			bOver = FindNextFileW(hFindFolder, &objFindData);
		}
	}
	FindClose(hFindFolder);
	return iFileCount;
}



// 判断是否为网络路径
XXAPI int xFile_IsNetworkPathW(wstr sPath)
{
	if ( ( sPath[0] != 0 ) && ( sPath[0] == 92 ) ) {
		if ( sPath[1] == 92 ) {
			return TRUE;
		}
	}
	return FALSE;
}
XXAPI int xFile_IsNetworkPathA(astr sPath)
{
	if ( ( sPath[0] != 0 ) && ( sPath[0] == 92 ) ) {
		if ( sPath[1] == 92 ) {
			return TRUE;
		}
	}
	return FALSE;
}



// 获取文件或目录的属性
XXAPI int xFile_GetPathAttrW(wstr sPath)
{
	return GetFileAttributesW(sPath);
}
XXAPI int xFile_GetPathAttrA(astr sPath)
{
	return GetFileAttributesA(sPath);
}



// 设置文件或目录的属性
XXAPI int xFile_SetPathAttrW(wstr sPath, int iAttr)
{
	return SetFileAttributesW(sPath, iAttr);
}
XXAPI int xFile_SetPathAttrA(astr sPath, int iAttr)
{
	return SetFileAttributesA(sPath, iAttr);
}



// 复制文件
XXAPI int xFile_CopyW(wstr sSrcPath, wstr sDstPath, int bReWrite)
{
	return CopyFileW(sSrcPath, sDstPath, bReWrite ? FALSE : TRUE);
}
XXAPI int xFile_CopyA(astr sSrcPath, astr sDstPath, int bReWrite)
{
	return CopyFileA(sSrcPath, sDstPath, bReWrite ? FALSE : TRUE);
}



// 移动文件
XXAPI int xFile_MoveW(wstr sSrcPath, wstr sDstPath, int bReWrite)
{
	if ( CopyFileW(sSrcPath, sDstPath, bReWrite ? FALSE : TRUE) ) {
		return DeleteFileW(sSrcPath);
	}
	return FALSE;
}
XXAPI int xFile_MoveA(astr sSrcPath, astr sDstPath, int bReWrite)
{
	if ( CopyFileA(sSrcPath, sDstPath, bReWrite ? FALSE : TRUE) ) {
		return DeleteFileA(sSrcPath);
	}
	return FALSE;
}



// 删除文件
XXAPI int xFile_DeleteW(wstr sPath)
{
	return DeleteFileW(sPath);
}
XXAPI int xFile_DeleteA(astr sPath)
{
	return DeleteFileA(sPath);
}



// 创建文件夹
XXAPI void xFile_CreateDirW(wstr sPath)
{
	wstr sRet = xCore_ReplaceW(sPath, L"/", L"\\");
	wstr* arrPath = xCore_SplitCharW(sRet, '\\', FALSE);
	int iSize = xCore.iRetSize;
	wstr sCurPath = malloc(4096);
	int i;
	if ( xFile_IsNetworkPathW(sPath) ) {
		// 网络路径 ( \\ 开头的 )
		wcscpy(sCurPath, L"\\\\");
		wcscat(sCurPath, arrPath[2]);
		i = 3;
	} else {
		// 普通路径
		wcscpy(sCurPath, arrPath[0]);
		i = 1;
	}
	for ( ; i < iSize; i++ ) {
		if ( arrPath[i][0] != 0 ) {
			wcscat(sCurPath, L"\\");
			wcscat(sCurPath, arrPath[i]);
			CreateDirectoryW(sCurPath, NULL);
		}
	}
	free(sCurPath);
}
XXAPI void xFile_CreateDirA(astr sPath)
{
	wstr sPathW = xCore_A2W(sPath, 0);
	xFile_CreateDirW(sPathW);
	free(sPathW);
}



// 复制文件夹
typedef struct {
	wstr DstPath;
	int PathPos;
	int bReWrite;
	int bMoveMode;			// 移动模式
} xFile_CopyFolder_Info;
int xFile_Folder_CopyMove_Proc(wstr sRootPath, WIN32_FIND_DATAW* pFindData, xFile_CopyFolder_Info* pInfo)
{
	wstr sSrcPath = xCore_FormatW(L"%s%s", sRootPath, pFindData->cFileName);
	wstr sDstPath = xCore_FormatW(L"%s%s", pInfo->DstPath, &sSrcPath[pInfo->PathPos]);
	if ( pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
		CreateDirectoryW(sDstPath, NULL);
	} else {
		if ( pInfo->bMoveMode ) {
			xFile_MoveW(sSrcPath, sDstPath, pInfo->bReWrite);
		} else {
			xFile_CopyW(sSrcPath, sDstPath, pInfo->bReWrite);
		}
	}
	xCore.iRetSize = 0;
	return FALSE;
}
XXAPI int xFile_CopyDirW(wstr sSrcPath, wstr sDstPath, wstr sFilter, int bReWrite)
{
	xFile_CreateDirW(sDstPath);
	xFile_CopyFolder_Info* pInfo = malloc(sizeof(xFile_CopyFolder_Info));
	pInfo->DstPath = sDstPath;
	pInfo->PathPos = wcslen(sSrcPath);
	pInfo->bReWrite = bReWrite;
	pInfo->bMoveMode = FALSE;
	return xFile_ScanW(sSrcPath, sFilter, 0, 0, TRUE, xFile_Folder_CopyMove_Proc, pInfo);
}
XXAPI int xFile_CopyDirA(astr sSrcPath, astr sDstPath, astr sFilter, int bReWrite)
{
	wstr sSrcPathW = xCore_A2W(sSrcPath, 0);
	wstr sDstPathW = xCore_A2W(sDstPath, 0);
	wstr sFilterW = xCore_A2W(sFilter, 0);
	int iRet = xFile_CopyDirW(sSrcPathW, sDstPathW, sFilterW, bReWrite);
	free(sSrcPathW);
	free(sDstPathW);
	free(sFilterW);
	return iRet;
}



// 移动文件夹
XXAPI int xFile_MoveDirW(wstr sSrcPath, wstr sDstPath, wstr sFilter, int bReWrite)
{
	xFile_CreateDirW(sDstPath);
	xFile_CopyFolder_Info* pInfo = malloc(sizeof(xFile_CopyFolder_Info));
	pInfo->DstPath = sDstPath;
	pInfo->PathPos = wcslen(sSrcPath);
	pInfo->bReWrite = bReWrite;
	pInfo->bMoveMode = TRUE;
	return xFile_ScanW(sSrcPath, sFilter, 0, 0, TRUE, xFile_Folder_CopyMove_Proc, pInfo);
}
XXAPI int xFile_MoveDirA(astr sSrcPath, astr sDstPath, astr sFilter, int bReWrite)
{
	wstr sSrcPathW = xCore_A2W(sSrcPath, 0);
	wstr sDstPathW = xCore_A2W(sDstPath, 0);
	wstr sFilterW = xCore_A2W(sFilter, 0);
	int iRet = xFile_MoveDirW(sSrcPathW, sDstPathW, sFilterW, bReWrite);
	free(sSrcPathW);
	free(sDstPathW);
	free(sFilterW);
	return iRet;
}



// 删除文件夹
int xFile_Folder_Delete_Proc(wstr sRootPath, WIN32_FIND_DATAW* pFindData, ptr pParam)
{
	wstr sPath = xCore_FormatW(L"%s%s", sRootPath, pFindData->cFileName);
	if ( pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
		RemoveDirectoryW(sPath);
	} else {
		xFile_DeleteW(sPath);
	}
	xCore.iRetSize = 0;
	return FALSE;
}
XXAPI int xFile_DeleteDirW(wstr sPath, wstr sFilter)
{
	int iRet = xFile_ScanW(sPath, sFilter, 0, FILE_SCAN_NOFOLDER, TRUE, xFile_Folder_Delete_Proc, NULL);
	xFile_ScanW(sPath, sFilter, 0, FILE_SCAN_NOFILE, TRUE, xFile_Folder_Delete_Proc, NULL);
	return iRet;
}
XXAPI int xFile_DeleteDirA(astr sPath, astr sFilter)
{
	wstr sPathW = xCore_A2W(sPath, 0);
	wstr sFilterW = xCore_A2W(sFilter, 0);
	int iRet = xFile_DeleteDirW(sPathW, sFilterW);
	free(sPathW);
	free(sFilterW);
	return iRet;
}


