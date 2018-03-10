


' 包信息头		[ 40 Byte ]
Type xPack_FileHead Field = 1
	FileHead As ZString * 4			' 文件标识头 [xpk]
	Ver_Cpt As UByte				' 兼容版本
	Ver_Sub As UByte				' 子版本
	InfoSize As UShort				' 文件信息头 附加数据长度
	FileCount As UInteger			' 文件数量
	LDB_Addr As UInteger			' 文件表位置 [文件表使用 LZ4 压缩]
	LDB_Size As UInteger			' 文件表大小
	LDB_Hash As UInteger			' 文件表哈希值
	Ext_Addr As UInteger			' 附加数据位置 [附加数据使用 LZ4 压缩]
	Ext_Comp As UInteger			' 附加数据大小 [压缩后]
	Ext_Size As UInteger			' 附加数据大小 [压缩前]
	Ext_Hash As UInteger			' 附加数据哈希值
End Type

' 文件信息头	[ 24 Byte ]
Type xPack_FileInfo Field = 1
	DataAddr As UInteger			' 数据位置
	DataSize As UInteger			' 数据大小
	FileHash As Integer				' 文件哈希值 [解压后]
	FileSize As UInteger			' 文件大小 [解压后]
	FileFlag As UShort				' 文件标记
	FileRefs As UShort				' 文件引用计数
	FileIndex As UInteger			' 文件 Index [访问ID]
End Type



Type xPack
	
	' 回调函数
	OnError As Function(iErrCode As Integer, sErrText As ZString Ptr) As Integer		' [错误号码, 错误描述(中文)]
	OnCompress As Function(pInData As Any Ptr, iInSize As UInteger, pOutData As Any Ptr, iOutSize As UInteger, iLevel As Integer) As UInteger		' [输入数据, 输入数据长度, 输出数据(内存已经分配好), 输出缓冲区长度, 压缩级别]
	OnDeCompress As Function(pInData As Any Ptr, iInSize As UInteger, pOutData As Any Ptr, iOutSize As UInteger) As UInteger						' [输入数据, 输入数据长度, 输出数据(内存已经分配好), 输出缓冲区长度]
	
	' 包操作
	Declare Function Open(sFile As ZString Ptr) As Integer
	Declare Function Create(sFile As ZString Ptr, iInfoSize As Integer = 0) As Integer
	Declare Function Save(bIsRebuild As Integer) As Integer
	Declare Sub Close()
	
	' 包信息操作
	Declare Function GetExtData(pOutData As Any Ptr) As Integer
	Declare Function SetExtData(pInData As Any Ptr, iInSize As UInteger) As Integer
	
	' 文件信息操作
	Declare Function GetFileInfo(idx As UInteger, bUsePos As Integer = 0) As xPack_FileInfo Ptr
	Declare Function GetFileSize(idx As UInteger) As UInteger
	Declare Function GetDataSize(idx As UInteger) As UInteger
	Declare Function GetFileFlag(idx As UInteger) As Integer
	Declare Function GetFileHash(idx As UInteger) As Integer
	Declare Function GetFilePos(idx As UInteger) As UInteger
	Declare Function GetFileIdx(iPos As UInteger) As UInteger
	Declare Function ExistsIdx(idx As UInteger) As Integer
	
	' 文件操作
	Declare Function AppendFile(sFile As ZString Ptr) As UInteger
	Declare Function AppendData(pInData As Any Ptr, iInSize As UInteger) As UInteger
	Declare Function UnpackFile(idx As UInteger, sFile As ZString Ptr) As UInteger
	Declare Function UnpackData(idx As UInteger, sOutData As Any Ptr) As UInteger
	Declare Function DeleteFile(idx As UInteger, bUsePos As Integer = 0) As Integer
	
	' 数据
	IsOpen As Integer				' 是否有打开的文件
	IsChange As Integer				' 是否存在修改 [添加删除文件、修改Ext数据]
	FileHandle As HANDLE			' 文件句柄 [打开文件后用于读写操作]
	PackHead As xPack_FileHead		' 文件头
	LDB As xBsmm Ptr				' 文件信息段数据
	
End Type
