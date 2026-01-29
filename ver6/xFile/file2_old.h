


// 文件编码
#define CHARSET_ERROR	   -10000			// 出错(用于获取编码时返回错误信息)
#define CHARSET_AUTO       -2				// 自动识别编码并转换为 UTF8
#define CHARSET_NULL       -1				// 不对数据做任何处理
#define CHARSET_OEM			0				// OEM代码页 [CP_ACP]
#define CHARSET_UNICODE     1200			// UNICODE
#define CHARSET_UNICODE_BE  1201			// UNICODE big-endian
#define CHARSET_ASCII       20127			// US-ASCII
#define CHARSET_UTF7        65000			// UTF7
#define CHARSET_UTF8        65001			// UTF8
#define CHARSET_UTF32		65005			// UTF32
#define CHARSET_UTF32_BE	65006			// UTF32 big-endian
#define CHARSER_DEFAULT     CHARSET_OEM		// 默认使用 OEM 编码

// 文件对象
typedef struct {
	HANDLE Handle;					// 文件句柄
	BOOL ReadOnly;					// 是否只读模式
	int Charset;					// 文件编码
	BOOL BOM;						// 是否有BOM
	unsigned int SeekOffset;		// 读写偏移 [用于跳过BOM]
} xBot_FileStruct, *xBot_FileObject;



// 是否 utf-8 [猜测]
int xBot_StringIsUtf8(char* sText, size_t iSize);

// 是否 utf-7 [猜测]
BOOL xBot_StringIsUtf7(char* sText, size_t iSize);

// 是否纯 ASCII [猜测]
BOOL xBot_StringIsAscii(char* sText, size_t iSize);

// Unicode BE -> LE
void xBot_UCS2_BE2LE(char* sText, size_t iSize);
#define xBot_UCS2_LE2BE xBot_UCS2_BE2LE

// utf32 BE -> LE [仅单个字符]
inline size_t xBot_UCS4_BE2LE(size_t iChar);
#define xBot_UCS4_LE2BE xBot_UCS2_BE2LE

// utf32 -> Unicode
size_t xBot_UCS4_To_UCS2(wchar_t* sText, size_t iSize, int iCharset);

// Unicode -> utf32
char* xBot_UCS2_To_UCS4(wchar_t* sText, size_t iSize, int iCharset);

// 打开文件
xBot_FileObject xBot_FileOpenW(const wstr sFile, BOOL bReadOnly, int iCharset);
xBot_FileObject xBot_FileOpenA(const astr sFile, BOOL bReadOnly, int iCharset);

// 关闭文件
BOOL xBot_FileClose(xBot_FileObject objFile);

// 获取文件编码
int xBot_FileCharset(xBot_FileObject objFile);

// 设置文件读写位置 [FILE_BEGIN、FILE_CURRENT、FILE_END]
ulong xBot_FileSeek(xBot_FileObject objFile, int iPos, DWORD dwMoveMethod);

// 获取文件读写位置
ulong xBot_FileCurpos(xBot_FileObject objFile);

// 获取一个已打开文件的长度
ulong xBot_FileSize(xBot_FileObject objFile);

// 是否已经读取到文件末尾
BOOL xBot_FileEOF(xBot_FileObject objFile);

// 设置文件在当前读写的位置结束
BOOL xBot_FileSetEOF(xBot_FileObject objFile);

// 向一个已打开的文件写入数据
ulong xBot_FileWriteW(xBot_FileObject objFile, const wstr pBuff, ulong iSize);
ulong xBot_FileWriteA(xBot_FileObject objFile, const astr pBuff, ulong iSize);

// 向一个已打开的文件写入二进制数据
int xBot_FilePut(xBot_FileObject objFile, ptr pBuffer, uint iAddr, uint iSize);

// 从一个已打开的文件读取数据 [需要释放内存]
wstr xBot_FileReadW(xBot_FileObject objFile, ulong iSize);
astr xBot_FileReadA(xBot_FileObject objFile, ulong iSize);

// 从一个已打开的文件读取二进制数据 [需要释放内存]
int xBot_FileGet(xBot_FileObject objFile, ptr pBuffer, uint iAddr, uint iSize);

// 判断文件是否存在
BOOL xBot_FileExistsW(const wstr sFile);
BOOL xBot_FileExistsA(const astr sFile);

// 判断路径是否存在
BOOL xBot_PathExistsW(const wstr sFile);
BOOL xBot_PathExistsA(const astr sFile);

// 判断目录是否存在
BOOL xBot_FolderExistsW(const wstr sFile);
BOOL xBot_FolderExistsA(const astr sFile);

// 获取文件长度
ulong xBot_FileGetSizeW(const wstr sFile);
ulong xBot_FileGetSizeA(const astr sFile);

// 设置文件长度
BOOL xBot_FileSetSizeW(const wstr sFile, ulong iSize);
BOOL xBot_FileSetSizeA(const astr sFile, ulong iSize);

// 向一个文件追加写入数据
ulong xBot_FileAppendW(const wstr sFile, const wstr pBuff, ulong iSize, int iCharset);
ulong xBot_FileAppendA(const astr sFile, const astr pBuff, ulong iSize, int iCharset);

// 将一个字符串写入文件，并覆盖原有的内容
ulong xBot_FileWriteAllW(const wstr sFile, const wstr pBuff, ulong iSize, int iCharset);
ulong xBot_FileWriteAllA(const astr sFile, const astr pBuff, ulong iSize, int iCharset);

// 从一个文件读出全部数据
wstr xBot_FileReadAllW(const wstr sFile, int iCharset);
astr xBot_FileReadAllA(const astr sFile, int iCharset);

// 将一段数据写入文件，并覆盖原有的内容
ulong xBot_FileWriteBinaryAllW(const wstr sFile, const ptr pBuff, ulong iSize);
ulong xBot_FileWriteBinaryAllA(const astr sFile, const ptr pBuff, ulong iSize);


