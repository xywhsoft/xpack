


#include <stdlib.h>
#include <windows.h>
#include "../xCore/xcore.h"
#include "file.h"



/* ------------------------------------ 工具函数 ------------------------------------ */

// 是否 utf-8 [猜测]
int xBot_StringIsUtf8(char* sText, size_t iSize)
{
	size_t nBytes = 0;
	for (size_t i = 0; i < iSize; i++)
	{
		char b = sText[i];
		if (nBytes == 0) {
			if (b >= 0x80)
			{
				if (b >= 0xFC  && b <= 0xFD) { nBytes = 6; }	// 此范围内为6字节UTF-8字符
				else if (b >= 0xF8) { nBytes = 5; }				// 此范围内为5字节UTF-8字符
				else if (b >= 0xF0) { nBytes = 4; }				// 此范围内为4字节UTF-8字符    
				else if (b >= 0xE0) { nBytes = 3; }				// 此范围内为3字节UTF-8字符    
				else if (b >= 0xC0) { nBytes = 2; }				// 此范围内为2字节UTF-8字符    
				else { return FALSE; }
				nBytes--;
			}
		} else {
			if ((b & 0xC0) != 0x80)
			{
				// 值介于 0x80 与 0xC0 之间的为无效UTF-8字符
				return FALSE;
			}
			nBytes--;
		}
		
	}
	return TRUE;
}

// 是否 utf-7 [猜测]
BOOL xBot_StringIsUtf7(char* sText, size_t iSize)
{
	for (size_t i = 0; i < iSize; i++)
	{
		if (sText[i] & 0xC0 != 0)
		{
			return FALSE;
		}
	}
	return TRUE;
}

// 是否纯 ASCII [猜测]
BOOL xBot_StringIsAscii(char* sText, size_t iSize)
{
	for (size_t i = 0; i < iSize; i++)
	{
		if ((sText[i] & 0x80) != 0)
		{
			return FALSE;
		}
	}
	return TRUE;
}

// Unicode BE -> LE
void xBot_UCS2_BE2LE(char* sText, size_t iSize)
{
	char c = 0;
	for (size_t i = 0; i < iSize; i++)
	{
		if ((i & 1) == 1) {
			sText[i-1] = sText[i];
			sText[i]  = c;
		} else {
			c =  sText[i];
		}
	}
}

// utf32 BE -> LE [仅单个字符]
inline size_t xBot_UCS4_BE2LE(size_t iChar)
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
size_t xBot_UCS4_To_UCS2(wchar_t* sText, size_t iSize, int iCharset)
{
	iSize = iSize  / 4;
	size_t iAddr = 0;
	size_t iChar;
	for (size_t i = 0; i < iSize; i++)
	{
		iChar = ((size_t*)sText)[i];
		// BE -> LE
		if (iCharset == CHARSET_UTF32_BE)
		{
			iChar = xBot_UCS4_BE2LE(iChar);
		}
		// utf32 -> utf16
		if (iChar <= 0xFFFF) {
			sText[iAddr] = iChar;
			iAddr++;
		} else if (iChar <= 0xEFFFF) {
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
char* xBot_UCS2_To_UCS4(wchar_t* sText, size_t iSize, int iCharset)
{
	iSize = iSize / 2;
	size_t iAddr = 0;
	size_t* sDest = malloc((iSize + 1) * 4);
	for (size_t i = 0; i < iSize; i++)
	{
		wchar_t w1 = sText[i];
		if ((w1 >= 0xD800) && (w1 <= 0xDFFF)) {
			if (w1 < 0xDC00)
			{
				wchar_t w2 = sText[i + 1];
				if ((w2 >= 0xDC00) && (w2 <= 0xDFFF))
				{
					sDest[iAddr] = (w2 & 0x03FF) + (((w1 & 0x03FF) + 0x40) << 10);
					// BE -> LE
					if (iCharset == CHARSET_UTF32_BE) { sDest[iAddr] = xBot_UCS4_BE2LE(sDest[iAddr]); }
					iAddr++;
				}
			}
			i++;
		} else {
			sDest[iAddr] = w1;
			// BE -> LE
			if (iCharset == CHARSET_UTF32_BE) { sDest[iAddr] = xBot_UCS4_BE2LE(sDest[iAddr]); }
			iAddr++;
		}
	}
	sDest[iAddr] = 0;
	xCore.iRetSize = iAddr * 4;
	return (char*)sDest;
}



/* ------------------------------------ C 函数库 ------------------------------------ */

// 打开文件
xBot_FileObject xBot_FileOpenW(const wstr sFile, BOOL bReadOnly, int iCharset)
{
	HANDLE hFile = CreateFileW(sFile, bReadOnly ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, bReadOnly ? OPEN_EXISTING : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		xBot_FileObject objFile = malloc(sizeof(xBot_FileStruct));
		if (objFile)
		{
			objFile->Handle = hFile;
			objFile->ReadOnly = bReadOnly;
			objFile->Charset = iCharset;
			objFile->BOM = FALSE;
			objFile->SeekOffset = 0;
			size_t iSize = GetFileSize(hFile, NULL);
			// 自动识别文件编码
			if (iCharset == CHARSET_AUTO)
			{
				if (iSize == 0) {
					// 空文件不设置编码
					objFile->Charset = CHARSET_NULL;
				} else {
					char* BOM = xBot_FileReadA(objFile, 4);
					if ((BOM[0] == 0xFF) && (BOM[1] == 0xFE) && (BOM[2] == 0x00) && (BOM[3] == 0x00)) {
						// UTF-32 LE (little-endian)
						objFile->Charset = CHARSET_UTF32;
						objFile->BOM = TRUE;
						objFile->SeekOffset = 4;
					} else if ((BOM[0] == 0x00) && (BOM[1] == 0x00) && (BOM[2] == 0xFE) && (BOM[3] == 0xFF)) {
						// UTF-32 BE (big-endian)
						objFile->Charset = CHARSET_UTF32_BE;
						objFile->BOM = TRUE;
						objFile->SeekOffset = 4;
					} else if ((BOM[0] == 0xEF) && (BOM[1] == 0xBB) && (BOM[2] == 0xBF)) {
						// UTF-8
						objFile->Charset = CHARSET_UTF8;
						objFile->BOM = TRUE;
						objFile->SeekOffset = 3;
					} else if ((BOM[0] == 0xFF) && (BOM[1] == 0xFE)) {
						// UTF-16 LE (little-endian)
						objFile->Charset = CHARSET_UNICODE;
						objFile->BOM = TRUE;
						objFile->SeekOffset = 2;
					} else if ((BOM[0] == 0xFE) && (BOM[1] == 0xFF)) {
						// UTF-16 BE (big-endian)
						objFile->Charset = CHARSET_UNICODE_BE;
						objFile->BOM = TRUE;
						objFile->SeekOffset = 2;
					} else {
						// 无 BOM 编码猜测 [选取64KB文本猜测]
						size_t iReadSize = iSize > 65536 ? 65536 : iSize;
						SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
						char* sText = xBot_FileReadA(objFile, iReadSize);
						if (xBot_StringIsUtf7(sText, iReadSize)) {
							// UTF7
							objFile->Charset = CHARSET_UTF7;
							objFile->BOM = FALSE;
							objFile->SeekOffset = 0;
						} else if (xBot_StringIsAscii(sText, iReadSize)) {
							// US-ASCII
							objFile->Charset = CHARSET_ASCII;
							objFile->BOM = FALSE;
							objFile->SeekOffset = 0;
						} else if (xBot_StringIsUtf8(sText, iReadSize)) {
							// UTF8 [无BOM]
							objFile->Charset = CHARSET_UTF8;
							objFile->BOM = FALSE;
							objFile->SeekOffset = 0;
						} else {
							// OEM [获取本机代码页]
							objFile->Charset = CHARSET_OEM;
							objFile->BOM = FALSE;
							objFile->SeekOffset = 0;
						}
						free(sText);
					}
					free(BOM);
				}
			} else if (bReadOnly == FALSE) {
				// 为带有BOM的编码补充文件头
				if (objFile->Charset == CHARSET_UNICODE)
				{
					SetFilePointer(hFile, objFile->SeekOffset, NULL, FILE_BEGIN);
					unsigned short fh = 0xFFFE;
					WriteFile(objFile->Handle, (void*)&fh, 2, (LPDWORD)&iSize, NULL);
					objFile->BOM = TRUE;
					objFile->SeekOffset = 2;
				}
				if (objFile->Charset == CHARSET_UNICODE_BE)
				{
					SetFilePointer(hFile, objFile->SeekOffset, NULL, FILE_BEGIN);
					unsigned short fh = 0xFEFF;
					WriteFile(objFile->Handle, (void*)&fh, 2, (LPDWORD)&iSize, NULL);
					objFile->BOM = TRUE;
					objFile->SeekOffset = 2;
				}
				if (objFile->Charset == CHARSET_UTF32)
				{
					SetFilePointer(hFile, objFile->SeekOffset, NULL, FILE_BEGIN);
					unsigned int fh = 0xFFFE0000;
					WriteFile(objFile->Handle, (void*)&fh, 4, (LPDWORD)&iSize, NULL);
					objFile->BOM = TRUE;
					objFile->SeekOffset = 4;
				}
				if (objFile->Charset == CHARSET_UTF32_BE)
				{
					SetFilePointer(hFile, objFile->SeekOffset, NULL, FILE_BEGIN);
					unsigned int fh = 0x0000FEFF;
					WriteFile(objFile->Handle, (void*)&fh, 4, (LPDWORD)&iSize, NULL);
					objFile->BOM = TRUE;
					objFile->SeekOffset = 4;
				}
			}
			SetFilePointer(hFile, objFile->SeekOffset, NULL, FILE_BEGIN);
			return objFile;
		}
	}
	return NULL;
}
xBot_FileObject xBot_FileOpenA(const astr sFile, BOOL bReadOnly, int iCharset)
{
	wchar_t* sFileW = xBot_M2W(sFile, CP_ACP, 0, FALSE);
	xBot_FileObject objFile = xBot_FileOpenW(sFileW, bReadOnly, iCharset);
	free(sFileW);
	return objFile;
}

// 关闭文件
BOOL xBot_FileClose(xBot_FileObject objFile)
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
int xBot_FileCharset(xBot_FileObject objFile)
{
	if (objFile && (objFile->Handle != INVALID_HANDLE_VALUE))
	{
		return objFile->Charset;
	}
	return CHARSET_ERROR;
}

// 设置文件读写位置 [FILE_BEGIN、FILE_CURRENT、FILE_END]
ulong xBot_FileSeek(xBot_FileObject objFile, int iPos, DWORD dwMoveMethod)
{
	if (objFile && (objFile->Handle != INVALID_HANDLE_VALUE))
	{
		if (dwMoveMethod == FILE_END){
			return SetFilePointer(objFile->Handle, iPos, NULL, dwMoveMethod);
		} else {
			return SetFilePointer(objFile->Handle, iPos + objFile->SeekOffset, NULL, dwMoveMethod);
		}
	}
	return 0;
}

// 获取文件读写位置
ulong xBot_FileCurpos(xBot_FileObject objFile)
{
	if (objFile && (objFile->Handle != INVALID_HANDLE_VALUE))
	{
		return SetFilePointer(objFile->Handle, 0, NULL, FILE_CURRENT);
	}
	return 0;
}

// 获取一个已打开文件的长度
ulong xBot_FileSize(xBot_FileObject objFile)
{
	if (objFile && (objFile->Handle != INVALID_HANDLE_VALUE))
	{
		return GetFileSize(objFile->Handle, NULL);
	}
	return 0;
}

// 是否已经读取到文件末尾
BOOL xBot_FileEOF(xBot_FileObject objFile)
{
	if (objFile && (objFile->Handle != INVALID_HANDLE_VALUE))
	{
		size_t pAddr = SetFilePointer(objFile->Handle, 0, NULL, FILE_CURRENT);
		size_t iSize = GetFileSize(objFile->Handle, NULL);
		return (pAddr >= iSize);
	}
	return TRUE;
}

// 设置文件在当前读写的位置结束
BOOL xBot_FileSetEOF(xBot_FileObject objFile)
{
	if (objFile && (objFile->Handle != INVALID_HANDLE_VALUE))
	{
		SetEndOfFile(objFile->Handle);
		return TRUE;
	}
	return FALSE;
}

// 向一个已打开的文件写入数据
ulong xBot_FileWriteW(xBot_FileObject objFile, const wstr pBuff, ulong iSize)
{
	if (objFile && (objFile->Handle != INVALID_HANDLE_VALUE))
	{
		if (iSize == 0)
		{
			iSize = wcslen(pBuff);
		}
		// 转换编码 [将数据转换为目标编码写入]
		char* sRet = (char*)pBuff;
		char* sAutoFree = NULL;
		if ((objFile->Charset >= 0) && (objFile->Charset != CHARSET_UNICODE))
		{
			if ((objFile->Charset == CHARSET_UTF32) || (objFile->Charset == CHARSET_UTF32_BE)) {
				// 将 UTF16 转换为 UTF32
				sRet = xBot_UCS2_To_UCS4(pBuff, iSize, objFile->Charset);
				iSize = xCore.iRetSize;
				sAutoFree = sRet;
			} else if (objFile->Charset == CHARSET_UNICODE_BE) {
				// 将 Unicode LE 转换为 Unicode BE
				xBot_UCS2_LE2BE((char*)pBuff, iSize);
			} else {
				// 其他编码转换后保存
				sRet = xBot_W2M(pBuff, objFile->Charset, iSize, FALSE);
				iSize = xCore.iRetSize;
				sAutoFree = sRet;
			}
		}
		// 写入转换后的数据
		size_t iWriteBytes;
		BOOL bRet = WriteFile(objFile->Handle, sRet, iSize, (LPDWORD)&iWriteBytes, NULL);
		if (sAutoFree) { free(sAutoFree); }
		if (bRet)
		{
			return iWriteBytes;
		}
	}
	return 0;
}
ulong xBot_FileWriteA(xBot_FileObject objFile, const astr pBuff, ulong iSize)
{
	if (objFile && (objFile->Handle != INVALID_HANDLE_VALUE))
	{
		if (iSize == 0)
		{
			iSize = strlen(pBuff);
		}
		// 转换编码 [将数据转换为目标编码写入]
		char* sRet = (char*)pBuff;
		char* sAutoFree = NULL;
		if ( (objFile->Charset >= 0) && (objFile->Charset != CHARSET_ASCII) )
		{
			// 先将编码转换为 Unicode
			sAutoFree = (char*)xBot_M2W(pBuff, CHARSET_OEM, iSize, FALSE);
			iSize = xCore.iRetSize;
			if ((objFile->Charset == CHARSET_UTF32) || (objFile->Charset == CHARSET_UTF32_BE)) {
				// 将 UTF16 转换为 UTF32
				sRet = xBot_UCS2_To_UCS4((wchar_t*)sAutoFree, iSize, objFile->Charset);
				iSize = xCore.iRetSize;
				free(sAutoFree);
				sAutoFree = sRet;
			} else if (objFile->Charset == CHARSET_UNICODE_BE) {
				// 将 Unicode LE 转换为 Unicode BE
				xBot_UCS2_LE2BE(sAutoFree, iSize);
				sRet = sAutoFree;
			} else if (objFile->Charset == CHARSET_UNICODE) {
				// Unicode 不做额外处理
				sRet = sAutoFree;
			} else {
				// 其他编码转换后保存
				sRet = xBot_W2M((wchar_t*)sAutoFree, objFile->Charset, iSize, FALSE);
				iSize = xCore.iRetSize;
				free(sAutoFree);
				sAutoFree = sRet;
			}
		}
		// 写入转换后的数据
		size_t iWriteBytes;
		BOOL bRet = WriteFile(objFile->Handle, sRet, iSize, (LPDWORD)&iWriteBytes, NULL);
		if (sAutoFree) { free(sAutoFree); }
		if (bRet)
		{
			return iWriteBytes;
		}
	}
	return 0;
}

// 向一个已打开的文件写入二进制数据
int xBot_FilePut(xBot_FileObject objFile, ptr pBuffer, uint iAddr, uint iSize)
{
	if ( objFile && (objFile->Handle != INVALID_HANDLE_VALUE) ) {
		int iFileAddr = SetFilePointer(objFile->Handle, iAddr, 0, FILE_BEGIN);
		if ( iFileAddr != HFILE_ERROR ) {
			//OVERLAPPED TOL;
			//memset(&TOL, 0, sizeof(OVERLAPPED));
			//TOL.Offset = iFileAddr;
			if ( WriteFile(objFile->Handle, pBuffer, iSize, &xCore.iRetSize, NULL) ) {
				return xCore.iRetSize;
			}
		}
	}
	xCore.iRetSize = 0;
	return 0;
}

// 从一个已打开的文件读取数据 [需要释放内存]
wstr xBot_FileReadW(xBot_FileObject objFile, ulong iSize)
{
	if (objFile && (objFile->Handle != INVALID_HANDLE_VALUE))
	{
		char* pBuff = malloc(iSize + 4);
		if (ReadFile(objFile->Handle, pBuff, iSize, &xCore.iRetSize, NULL))
		{
			// 读取到的文件可能是 utf-32 编码，所以留4个空字节做结尾
			pBuff[xCore.iRetSize] = 0;
			pBuff[xCore.iRetSize+1] = 0;
			pBuff[xCore.iRetSize+2] = 0;
			pBuff[xCore.iRetSize+3] = 0;
			// 转换编码 [将读取到的数据转换为 unicode 编码]
			wchar_t* sRet = (wchar_t*)pBuff;
			if ((objFile->Charset >= 0) && (objFile->Charset != CHARSET_UNICODE))
			{
				wchar_t* sTemp = sRet;
				size_t iTemp = xCore.iRetSize;
				// 将其他编码转换为 Unicode
				if ((objFile->Charset == CHARSET_UTF32) || (objFile->Charset == CHARSET_UTF32_BE)) {
					iTemp = xBot_UCS4_To_UCS2(sRet, iTemp, objFile->Charset);
				} else if ((objFile->Charset != CHARSET_UNICODE) && (objFile->Charset != CHARSET_UNICODE_BE)) {
					sRet = xBot_M2W((char*)sTemp, objFile->Charset, iTemp, FALSE);
					free(sTemp);
					sTemp = sRet;
					iTemp = xCore.iRetSize;
				}
				// 将 Unicode BE 转换为 Unicode LE
				if (objFile->Charset == CHARSET_UNICODE_BE)
				{
					xBot_UCS2_BE2LE((char*)sRet, iTemp);
				}
				free(sTemp);
				xCore.iRetSize = iTemp;
			}
			return sRet;
		} else {
			free(pBuff);
		}
	}
	xCore.iRetSize = 0;
	return (wchar_t*)xCore.nullstring;
}
astr xBot_FileReadA(xBot_FileObject objFile, ulong iSize)
{
	if (objFile && (objFile->Handle != INVALID_HANDLE_VALUE))
	{
		char* pBuff = malloc(iSize + 4);
		if (ReadFile(objFile->Handle, pBuff, iSize, &xCore.iRetSize, NULL))
		{
			// 读取到的文件可能是 utf-32 编码，所以留4个空字节做结尾
			pBuff[xCore.iRetSize] = 0;
			pBuff[xCore.iRetSize+1] = 0;
			pBuff[xCore.iRetSize+2] = 0;
			pBuff[xCore.iRetSize+3] = 0;
			// 转换编码 [将读取到的数据转换为 unicode 编码]
			char* sRet = pBuff;
			if ( (objFile->Charset >= 0) && (objFile->Charset != CHARSET_ASCII) )
			{
				char* sTemp = sRet;
				size_t iTemp = xCore.iRetSize;
				// 将其他编码转换为 Unicode
				if ((objFile->Charset == CHARSET_UTF32) || (objFile->Charset == CHARSET_UTF32_BE)) {
					iTemp = xBot_UCS4_To_UCS2((wchar_t*)sRet, iTemp, objFile->Charset);
				} else if ((objFile->Charset != CHARSET_UNICODE) && (objFile->Charset != CHARSET_UNICODE_BE)) {
					sRet = (char*)xBot_M2W(sTemp, objFile->Charset, iTemp, FALSE);
					free(sTemp);
					sTemp = sRet;
					iTemp = xCore.iRetSize;
				}
				// 将 Unicode BE 转换为 Unicode LE
				if (objFile->Charset == CHARSET_UNICODE_BE)
				{
					xBot_UCS2_BE2LE(sRet, iTemp);
				}
				// 最后将 Unicode 编码转换为 utf-8
				sRet = xBot_W2M((wchar_t*)sRet, CHARSET_OEM, iTemp, FALSE);
				free(sTemp);
				xCore.iRetSize = iTemp;
			}
			return sRet;
		} else {
			free(pBuff);
		}
	}
	xCore.iRetSize = 0;
	return xCore.nullstring;
}

// 从一个已打开的文件读取二进制数据 [需要释放内存]
int xBot_FileGet(xBot_FileObject objFile, ptr pBuffer, uint iAddr, uint iSize)
{
	if ( objFile && (objFile->Handle != INVALID_HANDLE_VALUE) ) {
		int iFileAddr = SetFilePointer(objFile->Handle, iAddr, 0, FILE_BEGIN);
		printf("SetFilePointer: %d\n", iFileAddr);
		if ( iFileAddr != HFILE_ERROR ) {
			//OVERLAPPED TOL;
			//memset(&TOL, 0, sizeof(OVERLAPPED));
			//TOL.Offset = iFileAddr;
			if ( ReadFile(objFile->Handle, pBuffer, iSize, &xCore.iRetSize, NULL) ) {
				return xCore.iRetSize;
			}
		}
	}
	xCore.iRetSize = 0;
	return 0;
}

// 判断文件是否存在
BOOL xBot_FileExistsW(const wstr sFile)
{
	HANDLE hFile = CreateFileW(sFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
		return TRUE;
	} else {
		if (GetLastError() == 32) { return TRUE; }
	}
	return FALSE;
}
BOOL xBot_FileExistsA(const astr sFile)
{
	HANDLE hFile = CreateFileA(sFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
		return TRUE;
	} else {
		if (GetLastError() == 32) { return TRUE; }
	}
	return FALSE;
}

// 判断路径是否存在
BOOL xBot_PathExistsW(const wstr sFile)
{
	return (GetFileAttributesW(sFile) != INVALID_FILE_ATTRIBUTES);
}
BOOL xBot_PathExistsA(const astr sFile)
{
	return (GetFileAttributesA(sFile) != INVALID_FILE_ATTRIBUTES);
}

// 判断目录是否存在
BOOL xBot_FolderExistsW(const wstr sFile)
{
	DWORD iRet = GetFileAttributesW(sFile);
	if ( iRet == INVALID_FILE_ATTRIBUTES ) { return FALSE; }
	if ( iRet & FILE_ATTRIBUTE_DIRECTORY ) {
		return TRUE;
	} else {
		return FALSE;
	}
}
BOOL xBot_FolderExistsA(const astr sFile)
{
	DWORD iRet = GetFileAttributesA(sFile);
	if ( iRet == INVALID_FILE_ATTRIBUTES ) { return FALSE; }
	if ( iRet & FILE_ATTRIBUTE_DIRECTORY ) {
		return TRUE;
	} else {
		return FALSE;
	}
}

// 获取文件长度
ulong xBot_FileGetSizeW(const wstr sFile)
{
	HANDLE hFile = CreateFileW(sFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile && hFile != INVALID_HANDLE_VALUE)
	{
		size_t iRet = GetFileSize(hFile, NULL);
		CloseHandle(hFile);
		return iRet;
	}
	return 0;
}
ulong xBot_FileGetSizeA(const astr sFile)
{
	HANDLE hFile = CreateFileA(sFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile && hFile != INVALID_HANDLE_VALUE)
	{
		size_t iRet = GetFileSize(hFile, NULL);
		CloseHandle(hFile);
		return iRet;
	}
	return 0;
}

// 设置文件长度
BOOL xBot_FileSetSizeW(const wstr sFile, ulong iSize)
{
	HANDLE hFile = CreateFileW(sFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile && hFile != INVALID_HANDLE_VALUE)
	{
		SetFilePointer(hFile, iSize, 0, FILE_BEGIN);
		SetEndOfFile(hFile);
		CloseHandle(hFile);
		return TRUE;
	}
	return FALSE;
}
BOOL xBot_FileSetSizeA(const astr sFile, ulong iSize)
{
	HANDLE hFile = CreateFileA(sFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile && hFile != INVALID_HANDLE_VALUE)
	{
		SetFilePointer(hFile, iSize, 0, FILE_BEGIN);
		SetEndOfFile(hFile);
		CloseHandle(hFile);
		return TRUE;
	}
	return FALSE;
}

// 向一个文件追加写入数据
ulong xBot_FileAppendW(const wstr sFile, const wstr pBuff, ulong iSize, int iCharset)
{
	xBot_FileObject objFile = xBot_FileOpenW(sFile, FALSE, iCharset);
	if (objFile)
	{
		if (iSize == 0) {
			iSize = wcslen(pBuff);
		}
		if ( iSize ) {
			xBot_FileSeek(objFile, 0, FILE_END);
			size_t iRet = xBot_FileWriteW(objFile, pBuff, iSize);
			xBot_FileClose(objFile);
			return iRet;
		}
	}
	return 0;
}
ulong xBot_FileAppendA(const astr sFile, const astr pBuff, ulong iSize, int iCharset)
{
	xBot_FileObject objFile = xBot_FileOpenA(sFile, FALSE, iCharset);
	if (objFile)
	{
		if (iSize == 0) {
			iSize = strlen(pBuff);
		}
		if ( iSize ) {
			xBot_FileSeek(objFile, 0, FILE_END);
			size_t iRet = xBot_FileWriteA(objFile, pBuff, iSize);
			xBot_FileClose(objFile);
			return iRet;
		}
	}
	return 0;
}

// 将一个字符串写入文件，并覆盖原有的内容
ulong xBot_FileWriteAllW(const wstr sFile, const wstr pBuff, ulong iSize, int iCharset)
{
	xBot_FileObject objFile = xBot_FileOpenW(sFile, FALSE, iCharset);
	if (objFile) {
		if (iSize == 0) {
			iSize = wcslen(pBuff);
		}
		if ( iSize ) {
			size_t iRet = xBot_FileWriteW(objFile, pBuff, iSize);
			//ffbot_FileSeek(hFile, iRet, FILE_BEGIN);
			xBot_FileSetEOF(objFile);
			xBot_FileClose(objFile);
			return iRet;
		}
	}
	return 0;
}
ulong xBot_FileWriteAllA(const astr sFile, const astr pBuff, ulong iSize, int iCharset)
{
	xBot_FileObject objFile = xBot_FileOpenA(sFile, FALSE, iCharset);
	if (objFile) {
		if (iSize == 0) {
			iSize = strlen(pBuff);
		}
		if ( iSize ) {
			size_t iRet = xBot_FileWriteA(objFile, pBuff, iSize);
			//ffbot_FileSeek(hFile, iRet, FILE_BEGIN);
			xBot_FileSetEOF(objFile);
			xBot_FileClose(objFile);
			return iRet;
		}
	}
	return 0;
}

// 从一个文件读出全部数据
wstr xBot_FileReadAllW(const wstr sFile, int iCharset)
{
	xBot_FileObject objFile = xBot_FileOpenW(sFile, TRUE, iCharset);
	if (objFile) {
		size_t iSize = xBot_FileSize(objFile) - objFile->SeekOffset;
		wchar_t* sRet = (wchar_t*)xCore.nullstring;
		if (iSize > 0) {
			sRet = xBot_FileReadW(objFile, iSize);
		}
		xBot_FileClose(objFile);
		return sRet;
	}
	xCore.iRetSize = 0;
	return (wchar_t*)xCore.nullstring;
}
astr xBot_FileReadAllA(const astr sFile, int iCharset)
{
	xBot_FileObject objFile = xBot_FileOpenA(sFile, TRUE, iCharset);
	if (objFile) {
		size_t iSize = xBot_FileSize(objFile) - objFile->SeekOffset;
		char* sRet = xCore.nullstring;
		if (iSize > 0) {
			sRet = xBot_FileReadA(objFile, iSize);
		}
		xBot_FileClose(objFile);
		return sRet;
	}
	xCore.iRetSize = 0;
	return xCore.nullstring;
}

// 将一段数据写入文件，并覆盖原有的内容
ulong xBot_FileWriteBinaryAllW(const wstr sFile, const ptr pBuff, ulong iSize)
{
	xBot_FileObject objFile = xBot_FileOpenW(sFile, FALSE, CHARSET_NULL);
	if (objFile) {
		if (iSize == 0) {
			iSize = wcslen(pBuff);
		}
		if ( iSize ) {
			size_t iRet = xBot_FilePut(objFile, pBuff, 0, iSize);
			//ffbot_FileSeek(hFile, iRet, FILE_BEGIN);
			xBot_FileSetEOF(objFile);
			xBot_FileClose(objFile);
			return iRet;
		} else {
			xBot_FileSeek(objFile, 0, FILE_BEGIN);
			xBot_FileSetEOF(objFile);
			xBot_FileClose(objFile);
			return 0;
		}
	}
	return 0;
}
ulong xBot_FileWriteBinaryAllA(const astr sFile, const ptr pBuff, ulong iSize)
{
	xBot_FileObject objFile = xBot_FileOpenA(sFile, FALSE, CHARSET_NULL);
	if (objFile) {
		if (iSize == 0) {
			iSize = strlen(pBuff);
		}
		if ( iSize ) {
			size_t iRet = xBot_FilePut(objFile, pBuff, 0, iSize);
			//ffbot_FileSeek(hFile, iRet, FILE_BEGIN);
			xBot_FileSetEOF(objFile);
			xBot_FileClose(objFile);
			return iRet;
		} else {
			xBot_FileSeek(objFile, 0, FILE_BEGIN);
			xBot_FileSetEOF(objFile);
			xBot_FileClose(objFile);
			return 0;
		}
	}
	return 0;
}
