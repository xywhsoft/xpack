


// 文件编码
#define CHARSET_ERROR		-0x10000		// 出错(用于获取编码时返回错误信息)
#define CHARSET_AUTO		-2				// 自动识别编码
#define CHARSET_BINARY		-1				// 二进制文件
#define CHARSET_ACP			CP_ACP			// OEM代码页 [CP_ACP]	[val:0]
#define CHARSET_OEMCP		CP_OEMCP		// OEM代码页 [CP_OEMCP]	[val:1]
#define CHARSET_UNICODE     1200			// UNICODE
#define CHARSET_UNICODE_BE  1201			// UNICODE big-endian
#define CHARSET_ASCII       20127			// US-ASCII
#define CHARSET_GB18030     54936			// GB 18030
#define CHARSET_UTF7        65000			// UTF7
#define CHARSET_UTF8        65001			// UTF8
#define CHARSET_UTF32		65005			// UTF32
#define CHARSET_UTF32_BE	65006			// UTF32 big-endian
#define CHARSER_DEFAULT     CHARSET_BINARY	// 默认使用二进制方式读文件

// 查找文件时的过滤规则
#define FILE_SCAN_NOFILE		1			// 不包含文件
#define FILE_SCAN_NOFOLDER		2			// 不包含文件夹
#define FILE_SCAN_POINTFOLDER	4			// 包含 . 和 .. 的文件夹

// 文件对象
typedef struct {
	HANDLE Handle;					// 文件句柄
	int Charset;					// 文件编码
	int BOM;						// BOM大小
} xFileStruct, *xFileObject;



// 是否 utf-8 [猜测]
XXAPI int xCore_StringIsUtf8(char* sText, size_t iSize);

// 是否 utf-7 [猜测]
XXAPI int xCore_StringIsUtf7(char* sText, size_t iSize);

// 是否纯 ASCII [猜测]
XXAPI int xCore_StringIsAscii(char* sText, size_t iSize);

// Unicode BE -> LE
XXAPI void xCore_UCS2_BE2LE(char* sText, size_t iSize);
#define xCore_UCS2_LE2BE xCore_UCS2_BE2LE

// utf32 BE -> LE [仅单个字符]
XXAPI static inline size_t xCore_UCS4_BE2LE(size_t iChar);
#define xCore_UCS4_LE2BE xCore_UCS2_BE2LE

// utf32 -> Unicode
XXAPI size_t xCore_UCS4_To_UCS2(wchar_t* sText, size_t iSize, int iCharset);

// Unicode -> utf32
XXAPI char* xCore_UCS2_To_UCS4(wchar_t* sText, size_t iSize, int iCharset);



// 打开文件
XXAPI xFileObject xFile_OpenW(wstr sFile, int bReadOnly, int iCharset);
XXAPI xFileObject xFile_OpenA(astr sFile, int bReadOnly, int iCharset);

// 关闭文件
XXAPI int xFile_Close(xFileObject objFile);

// 获取文件编码
XXAPI int xFile_Charset(xFileObject objFile);

// 设置文件读写位置 [FILE_BEGIN、FILE_CURRENT、FILE_END]
XXAPI int xFile_Seek(xFileObject objFile, int64 iPos, DWORD dwMoveMethod);

// 获取文件读写位置
XXAPI uint64 xFile_CurPos(xFileObject objFile);

// 获取一个已打开文件的长度
XXAPI uint64 xFile_Size(xFileObject objFile);

// 是否已经读取到文件末尾
XXAPI int xFile_EOF(xFileObject objFile);

// 设置文件在当前读写的位置结束
XXAPI int xFile_SetEOF(xFileObject objFile);



// 向一个已打开的文件写入数据
XXAPI ulong xFile_WriteW(xFileObject objFile, wstr pBuff, ulong iSize);
XXAPI ulong xFile_WriteA(xFileObject objFile, astr pBuff, ulong iSize);
XXAPI ulong xFile_WriteU(xFileObject objFile, ustr pBuff, ulong iSize);

// 从一个已打开的文件读取数据（数据需使用xCore_free释放）
XXAPI wstr xFile_ReadW(xFileObject objFile, ulong iSize);
XXAPI astr xFile_ReadA(xFileObject objFile, ulong iSize);
XXAPI ustr xFile_ReadU(xFileObject objFile, ulong iSize);

// 向一个已打开的文件写入二进制数据
XXAPI ulong xFile_Put(xFileObject objFile, ptr pBuffer, ulong iSize);

// 从一个已打开的文件读取二进制数据
XXAPI ulong xFile_Get(xFileObject objFile, ptr pBuffer, ulong iSize);



// 判断文件是否存在
XXAPI int xFile_ExistsW(const wstr sFile);
XXAPI int xFile_ExistsA(const astr sFile);

// 判断路径是否存在
XXAPI int xFile_PathExistsW(const wstr sFile);
XXAPI int xFile_PathExistsA(const astr sFile);

// 判断目录是否存在
XXAPI int xFile_FolderExistsW(const wstr sFile);
XXAPI int xFile_FolderExistsA(const astr sFile);



// 获取文件长度
XXAPI uint64 xFile_GetSizeW(const wstr sFile);
XXAPI uint64 xFile_GetSizeA(const astr sFile);
XXAPI uint64 xFile_GetSizeU(const ustr sFile);

// 设置文件长度
XXAPI int xFile_SetSizeW(const wstr sFile, uint64 iSize);
XXAPI int xFile_SetSizeA(const astr sFile, uint64 iSize);
XXAPI int xFile_SetSizeU(const ustr sFile, uint64 iSize);

// 向一个文件追加写入数据
XXAPI ulong xFile_AppendW(wstr sFile, wstr pBuff, ulong iSize, int iCharset);
XXAPI ulong xFile_AppendA(astr sFile, astr pBuff, ulong iSize, int iCharset);
XXAPI ulong xFile_AppendU(ustr sFile, ustr pBuff, ulong iSize, int iCharset);

// 将一个字符串写入文件，并覆盖原有的内容
XXAPI ulong xFile_WriteAllW(wstr sFile, wstr pBuff, ulong iSize, int iCharset);
XXAPI ulong xFile_WriteAllA(astr sFile, astr pBuff, ulong iSize, int iCharset);
XXAPI ulong xFile_WriteAllU(ustr sFile, ustr pBuff, ulong iSize, int iCharset);

// 从一个文件读出全部数据（数据需使用xCore_free释放）
XXAPI wstr xFile_ReadAllW(wstr sFile, int iCharset);
XXAPI astr xFile_ReadAllA(astr sFile, int iCharset);
XXAPI astr xFile_ReadAllU(ustr sFile, int iCharset);

// 将一段数据写入文件，并覆盖原有的内容
XXAPI ulong xFile_PutAllW(wstr sFile, ptr pBuff, ulong iSize);
XXAPI ulong xFile_PutAllA(astr sFile, ptr pBuff, ulong iSize);
XXAPI ulong xFile_PutAllU(ustr sFile, ptr pBuff, ulong iSize);

// 将一段数据写入文件，并覆盖原有的内容（内存需使用xCore_Free释放）
XXAPI ptr xFile_GetAllW(wstr sFile);
XXAPI ptr xFile_GetAllA(astr sFile);
XXAPI ptr xFile_GetAllU(ustr sFile);



// 查找文件
XXAPI int xFile_ScanA(astr sRoot, astr sFilter, int iAttrib, int iAttribEx, int bRecu, ptr pProc, ptr iParam);
XXAPI int xFile_ScanW(wstr sRoot, wstr sFilter, int iAttrib, int iAttribEx, int bRecu, ptr pProc, ptr iParam);



// 判断是否为网络路径
XXAPI int xFile_IsNetworkPathW(wstr sPath);
XXAPI int xFile_IsNetworkPathA(astr sPath);

// 获取文件或目录的属性
XXAPI int xFile_GetPathAttrW(wstr sPath);
XXAPI int xFile_GetPathAttrA(astr sPath);

// 设置文件或目录的属性
XXAPI int xFile_SetPathAttrW(wstr sPath, int iAttr);
XXAPI int xFile_SetPathAttrA(astr sPath, int iAttr);

// 复制文件
XXAPI int xFile_CopyW(wstr sSrcPath, wstr sDstPath, int bReWrite);
XXAPI int xFile_CopyA(astr sSrcPath, astr sDstPath, int bReWrite);

// 移动文件
XXAPI int xFile_MoveW(wstr sSrcPath, wstr sDstPath, int bReWrite);
XXAPI int xFile_MoveA(astr sSrcPath, astr sDstPath, int bReWrite);

// 删除文件
XXAPI int xFile_DeleteW(wstr sPath);
XXAPI int xFile_DeleteA(astr sPath);

// 创建文件夹
XXAPI void xFile_CreateDirW(wstr sPath);
XXAPI void xFile_CreateDirA(astr sPath);

// 复制文件夹
XXAPI int xFile_CopyDirW(wstr sSrcPath, wstr sDstPath, wstr sFilter, int bReWrite);
XXAPI int xFile_CopyDirA(astr sSrcPath, astr sDstPath, astr sFilter, int bReWrite);

// 移动文件夹
XXAPI int xFile_MoveDirW(wstr sSrcPath, wstr sDstPath, wstr sFilter, int bReWrite);
XXAPI int xFile_MoveDirA(astr sSrcPath, astr sDstPath, astr sFilter, int bReWrite);

// 删除文件夹
XXAPI int xFile_DeleteDirW(wstr sPath, wstr sFilter);
XXAPI int xFile_DeleteDirA(astr sPath, astr sFilter);


