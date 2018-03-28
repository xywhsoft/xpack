


#Define XPACK_VERSION			5



#Define XPACK_COMP_NOUSED		0
#Define XPACK_COMP_LEVEL1		1
#Define XPACK_COMP_LEVEL2		2
#Define XPACK_COMP_LEVEL3		3
#Define XPACK_COMP_BITS			3



#Define XPACK_DEFAULT_LDBCOMP	XPACK_COMP_NOUSED
#Define XPACK_DEFAULT_FILECOMP	XPACK_COMP_LEVEL2



#Define OnErr(a, b) If OnError Then : LastError = a : OnError(a, b) : Return 0 : EndIf



#Define XPACK_ERROR_1	"文件无法访问"
#Define XPACK_ERROR_2	"文件格式不正确"
#Define XPACK_ERROR_3	"内存不足"
#Define XPACK_ERROR_4	"文件列表读取失败"
#Define XPACK_ERROR_5	"操作前必须先打开文件包"
#Define XPACK_ERROR_6	"文件列表数据添加失败"
#Define XPACK_ERROR_7	"不允许添加 0 字节数据"
#Define XPACK_ERROR_8	"文件数据写入长度异常"
#Define XPACK_ERROR_9	"已经添加过文件的包无法修改文件信息段数据长度"



' 包信息头		[ 24 Byte ]
Type xPack_FileHead Field = 1
	FileHead As ZString * 4			' 文件标识头 [xpk]
	PackVers As UByte				' 包文件版本
	PackFlag As UByte				' 包标记 [XX:LDB压缩算法、00XX:文件默认压缩算法、0000XXXX:暂未使用]
	InfoSize As UShort				' 文件信息头 附加数据长度
	FileCount As UInteger			' 文件数量
	LDB_Addr As UInteger			' 文件表位置 [文件表默认使用 Level2 压缩]
	LDB_Size As UInteger			' 文件表大小
	LDB_Hash As UInteger			' 文件表哈希值
End Type



' 文件信息头	[ 24 Byte ]
Type xPack_FileInfo Field = 1
	DataAddr As UInteger			' 数据位置
	DataSize As UInteger			' 数据大小
	FileHash As Integer				' 文件哈希值 [解压后]
	FileSize As UInteger			' 文件大小 [解压后]
	FileFlag As UByte				' 文件标记 [XX:压缩算法]
	FileVers As UByte				' 文件版本
	FileRefs As UShort				' 文件引用计数
	FileIndex As Integer			' 文件 Index [访问ID]
End Type





Type xPack
	
	' 回调函数
	OnError As Sub(iErrCode As Integer, sErrText As ZString Ptr)			' [错误号码, 错误描述(中文)]
	
	' 包操作
	Declare Function Open(sFile As ZString Ptr) As Integer
	Declare Function Save(bIsRebuild As Integer) As Integer
	Declare Function Close() As Integer
	Declare Function IsOpen() As Integer
	
	' 包信息操作
	Declare Function FileCount() As UInteger
	Declare Function SetFileInfoExtSize(iNewVal As UShort) As Integer
	Declare Function GetFileInfoExtSize() As UShort
	Declare Function SetDefaultCompressLevel(iNewVal As UInteger) As Integer
	Declare Function GetDefaultCompressLevel() As UInteger
	
	' 文件信息操作
	Declare Function GetFileInfo(idx As UInteger, bUsePos As Integer = 0) As xPack_FileInfo Ptr
	Declare Function GetFileSize(idx As UInteger, bUsePos As Integer = 0) As UInteger
	Declare Function GetFileDataSize(idx As UInteger, bUsePos As Integer = 0) As UInteger
	Declare Function GetFileHash(idx As UInteger, bUsePos As Integer = 0) As Integer
	Declare Function GetFileCompLevel(idx As UInteger, bUsePos As Integer = 0) As UInteger
	
	' idx 和 pos 转换
	Declare Function GetFilePos(idx As UInteger) As UInteger
	Declare Function GetFileIdx(iPos As UInteger) As UInteger
	
	' 文件操作
	Declare Function AppendFile(idx As UInteger, sFile As ZString Ptr, iCompLevel As Integer = -1) As UInteger
	Declare Function AppendData(idx As UInteger, pInData As Any Ptr, iInSize As UInteger, iCompLevel As Integer = -1) As UInteger
	Declare Function UnpackFile(idx As UInteger, sFile As ZString Ptr) As UInteger
	Declare Function UnpackData(idx As UInteger, sOutData As Any Ptr) As UInteger
	Declare Function DeleteFile(idx As UInteger, bUsePos As Integer = 0) As Integer
	
	' 数据
	LastError As Integer				' 最后一次记录的错误
	IsChange As Integer					' 是否存在修改 [添加删除文件、修改设置]
	Protected:
	FileHandle As HANDLE				' 文件句柄 [打开文件后用于读写操作]
	PackHead As xPack_FileHead			' 文件头
	LDB As xBsmm Ptr					' 文件信息段数据
	
End Type
