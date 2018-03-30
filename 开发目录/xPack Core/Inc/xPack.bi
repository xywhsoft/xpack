


#Define XPACK_EXPORT Export

#Define XPACK_VERSION			5			' 包结构版本

#Define XPACK_CLASS_CORE		&H0			' 使用 iPos 访问的压缩包 [二次开发核心]
#Define XPACK_CLASS_INDEX		&H10		' 使用 Index 访问的压缩包
#Define XPACK_CLASS_PATH		&H20		' 使用路径结构访问的压缩包
#Define XPACK_CLASS_SPATH		&H30		' 使用单层路径访问的压缩包

#Define XPACK_FILETYPE_OTHER	0			' 任意类型
#Define XPACK_FILETYPE_XGI		1			' xgi 图像格式
#Define XPACK_FILETYPE_STB		2			' stb 支持的图像格式
#Define XPACK_FILETYPE_SOUND	101			' 声音格式
#Define XPACK_FILETYPE_TEXT		201			' 文本格式
#Define XPACK_FILETYPE_WTEXT	202			' UNICODE 文本格式
#Define XPACK_FILETYPE_UTEXT	203			' UTF-8 文本格式
#Define XPACK_FILETYPE_DIR		255			' 目录

#Define XPACK_COMP_NOUSED		0			' 存储
#Define XPACK_COMP_LEVEL1		1			' 极速压缩
#Define XPACK_COMP_LEVEL2		2			' 标准压缩
#Define XPACK_COMP_LEVEL3		3			' 极限压缩
#Define XPACK_COMP_BITS			3			' 压缩位掩码



#Define XPACK_DEFAULT_LDBCOMP	XPACK_COMP_LEVEL2		' 默认 LDB 压缩级别
#Define XPACK_DEFAULT_FILECOMP	XPACK_COMP_LEVEL2		' 默认文件压缩级别



#Define OnErr(a, b) If OnError Then : /'LastError = a'/ : OnError(a, b) : Return 0 : EndIf



#Define XPACK_ERROR_1	"文件无法访问"
#Define XPACK_ERROR_2	"文件格式不正确"
#Define XPACK_ERROR_3	"内存不足"
#Define XPACK_ERROR_4	"文件列表读取失败"
#Define XPACK_ERROR_5	"操作前必须先打开文件包"
#Define XPACK_ERROR_6	"文件列表数据添加失败"
#Define XPACK_ERROR_7	"不允许添加 0 字节数据"
#Define XPACK_ERROR_8	"文件数据写入失败"
#Define XPACK_ERROR_9	"已经添加过文件的包无法修改文件信息段数据长度"
#Define XPACK_ERROR_10	"无效的文件位置"
#Define XPACK_ERROR_11	"文件数据校验失败"



' 包信息头		[ 24 Byte ]
Type xPack_FileHead Field = 1
	FileHead As ZString * 4			' 文件标识头 [xpk]
	PackVers As UByte				' 包文件版本
	PackFlag As UByte				' 包标记 [XX:LDB压缩算法、00XX:文件默认压缩算法、0000XXXX:压缩包类型（0:普通、&H10:Idx包、&H20:Path包、&H30:SPath包）]
	InfoSize As UShort				' 文件信息头 附加数据长度
	FileCount As UInteger			' 文件数量
	LDB_Addr As UInteger			' 文件表位置 [文件表默认使用 Level2 压缩]
	LDB_Size As UInteger			' 文件表大小
	LDB_Hash As UInteger			' 文件表哈希值
End Type



' 文件信息头	[ 20 Byte ]
Type xPack_FileInfo Field = 1
	DataAddr As UInteger			' 数据位置
	DataSize As UInteger			' 数据大小
	FileSize As UInteger			' 文件大小 [解压后]
	FileHash As UInteger			' 文件哈希值 [解压后]
	CompLevel As UByte				' 压缩级别 [0:不压缩、1:快速压缩、2:均衡压缩、3:最大压缩]
	FileType As UByte				' 文件类型
	Reserve As UShort				' 未使用数据
End Type





Type xPack
	
	' 回调函数
	OnError As Sub(iErrCode As Integer, sErrText As ZString Ptr)			' [错误号码, 错误描述(中文)]
	
	' 包操作
	Declare Function Open(sFile As ZString Ptr, iOffset As UInteger = 0) As Integer
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
	Declare Function GetFileInfo(iPos As UInteger) As xPack_FileInfo Ptr
	Declare Function GetFileSize(iPos As UInteger) As UInteger
	Declare Function GetFileDataSize(iPos As UInteger) As UInteger
	Declare Function GetFileHash(iPos As UInteger) As Integer
	Declare Function GetFileCompLevel(iPos As UInteger) As UInteger
	Declare Function GetFileType(iPos As UInteger) As UByte
	Declare Function SetFileType(iPos As UInteger, iNewVal As UByte) As Integer
	
	' 文件操作
	Declare Function AppendFile(sFile As ZString Ptr, iCompLevel As Integer = -1, iFileType As UByte = XPACK_FILETYPE_OTHER) As UInteger
	Declare Function AppendData(pInData As Any Ptr, iInSize As UInteger, iCompLevel As Integer = -1, iFileType As UByte = XPACK_FILETYPE_OTHER) As UInteger
	Declare Function UnpackFile(iPos As UInteger, sFile As ZString Ptr) As UInteger
	Declare Function UnpackData(iPos As UInteger, sOutData As Any Ptr, bAlloc As Integer = 0) As UInteger
	Declare Function DeleteFile(iPos As UInteger) As Integer
	
	' 数据
	IsChange As Integer					' 是否存在修改 [添加删除文件、修改设置]
	Protected:
	FileHandle As HANDLE				' 文件句柄 [打开文件后用于读写操作]
	FileOffset As UInteger				' 文件偏移
	PackHead As xPack_FileHead			' 文件头
	LDB As xBsmm Ptr					' 文件信息段数据
	
End Type





' 压缩入口（压缩失败自动转为无压缩）
Function xPack_Compress(tpe As UByte Ptr, pSrc As Any Ptr, iSrcSize As UInteger, pDst As Any Ptr Ptr, iDstSize As UInteger) As UInteger
	Dim RetInt As UInteger
	Select Case (*tpe And XPACK_COMP_BITS)
		Case XPACK_COMP_LEVEL1
			' 快速压缩，使用 LZ4 压缩算法
			Dim pOut As Any Ptr = malloc(iDstSize)
			RetInt = LZ4_compress_default(pSrc, pOut, iSrcSize, iDstSize)
			If RetInt Then
				*pDst = pOut
				Return RetInt
			Else
				free(pOut)
			EndIf
		Case XPACK_COMP_LEVEL2
			' 均衡压缩，使用 LZMA 快速算法
			Dim pOut As Any Ptr = malloc(iDstSize)
			RetInt = Lzma_Compress(pSrc, pOut, iSrcSize, iDstSize, 1)
			If RetInt Then
				*pDst = pOut
				Return RetInt
			Else
				free(pOut)
			EndIf
		Case XPACK_COMP_LEVEL3
			' 最高压缩，使用 LZMA 常规算法
			Dim pOut As Any Ptr = malloc(iDstSize)
			RetInt = Lzma_Compress(pSrc, pOut, iSrcSize, iDstSize, 6)
			If RetInt Then
				*pDst = pOut
				Return RetInt
			Else
				free(pOut)
			EndIf
	End Select
	' 不压缩则直接返回输入数据
	*tpe And= Not(XPACK_COMP_BITS)
	*pDst = pSrc
	Return iSrcSize
End Function

' 解压入口
Function xPack_DeCompress(tpe As UByte, pSrc As Any Ptr, iSrcSize As UInteger, pDst As Any Ptr, iDstSize As UInteger) As UInteger
	Select Case (tpe And XPACK_COMP_BITS)
		Case XPACK_COMP_LEVEL1
			' LZ4 解压
			Return LZ4_decompress_safe(pSrc, pDst, iSrcSize, iDstSize)
		Case XPACK_COMP_LEVEL2, XPACK_COMP_LEVEL3
			' LZMA 解压
			Return Lzma_Uncompress(pSrc, pDst, iSrcSize, iDstSize)
		Case Else
			' 不压缩则直接复制数据（以较短的数据为准）
			Dim CopySize As UInteger = IIf(iDstSize > iSrcSize, iSrcSize, iDstSize)
			CopyMemory(pDst, pSrc, CopySize)
			Return CopySize
	End Select
End Function





' 包操作
Function xPack.Open(sFile As ZString Ptr, iOffset As UInteger = 0) As Integer XPACK_EXPORT
	' 已经打开过文件则先关闭
	If FileHandle Then
		Close()
	EndIf
	IsChange = 0
	' 文件不存在则创建文件
	If FileExists(sFile) = 0 Then
		Dim NewPackHead As xPack_FileHead
		NewPackHead.FileHead = "xpk"
		NewPackHead.PackVers = XPACK_VERSION
		NewPackHead.PackFlag = XPACK_DEFAULT_LDBCOMP Or (XPACK_DEFAULT_FILECOMP Shl 2)
		NewPackHead.InfoSize = 0
		NewPackHead.FileCount = 0
		NewPackHead.LDB_Addr = SizeOf(xPack_FileHead)
		NewPackHead.LDB_Size = 0
		NewPackHead.LDB_Hash = 0
		PutFile(sFile, @NewPackHead, iOffset, SizeOf(xPack_FileHead))
	EndIf
	' 打开文件
	FileOffset = iOffset
	FileHandle = Open_File(sFile)
	If FileHandle = 0 Then
		OnErr(1, XPACK_ERROR_1)
	EndIf
	' 读取信息段，验证文件头和版本
	Get_File(FileHandle, @PackHead, iOffset, SizeOf(xPack_FileHead))
	If (PackHead.FileHead <> "xpk") OrElse (PackHead.PackVers <> XPACK_VERSION) Then
		CloseHandle(FileHandle)
		FileHandle = NULL
		OnErr(2, XPACK_ERROR_2)
	EndIf
	' 读取文件列表(LDB段)
	LDB = New xBsmm(SizeOf(xPack_FileInfo) + PackHead.InfoSize, 32, PackHead.FileCount)
	If LDB = NULL Then
		CloseHandle(FileHandle)
		FileHandle = NULL
		OnErr(3, XPACK_ERROR_3)
	EndIf
	' 读取文件列表（文件列表没压缩时直接读入xBsmm）
	If PackHead.FileCount Then
		Dim LDB_Size As UInteger = (SizeOf(xPack_FileInfo) + PackHead.InfoSize) * PackHead.FileCount
		If (PackHead.PackFlag And XPACK_COMP_BITS) = 0 Then
			Get_File(FileHandle, LDB->StructMemory, iOffset + PackHead.LDB_Addr, PackHead.LDB_Size)
		Else
			' 解压文件列表(LDB段)
			Dim LDB_Data As Any Ptr = malloc(PackHead.LDB_Size)
			Get_File(FileHandle, LDB_Data, iOffset + PackHead.LDB_Addr, PackHead.LDB_Size)
			xPack_DeCompress(PackHead.PackFlag, LDB_Data, PackHead.LDB_Size, LDB->StructMemory, LDB_Size)
			free(LDB_Data)
		EndIf
		' 校验文件列表数据
		If CityHash32(LDB->StructMemory, LDB_Size) <> PackHead.LDB_Hash Then
			CloseHandle(FileHandle)
			FileHandle = NULL
			Delete LDB
			OnErr(4, XPACK_ERROR_4)
		EndIf
		' 文件打开成功
		LDB->StructCount = PackHead.FileCount
	EndIf
	Return -1
End Function

Function xPack.Save(bIsRebuild As Integer) As Integer XPACK_EXPORT
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 更新文件头数据
	PackHead.FileHead = "xpk"
	PackHead.PackVers = XPACK_VERSION
	PackHead.FileCount = LDB->StructCount
	If PackHead.FileCount > 0 Then
		' 压缩文件列表
		Dim iSize As UInteger = (SizeOf(xPack_FileInfo) + PackHead.InfoSize) * PackHead.FileCount
		PackHead.LDB_Hash = CityHash32(LDB->StructMemory, iSize)
		Dim pData As Any Ptr
		PackHead.LDB_Size = xPack_Compress(@PackHead.PackFlag, LDB->StructMemory, iSize, @pData, iSize)
		' 写入文件列表
		Dim iRet As Integer = Put_File(FileHandle, pData, FileOffset + PackHead.LDB_Addr, PackHead.LDB_Size)
		If PackHead.PackFlag And XPACK_COMP_BITS <> 0 Then
			free(pData)
		EndIf
		If iRet = 0 Then
			OnErr(8, XPACK_ERROR_8)
		EndIf
	Else
		PackHead.LDB_Hash = 0
	EndIf
	' 写入文件头数据
	If Put_File(FileHandle, @PackHead, FileOffset, SizeOf(xPack_FileHead)) = 0 Then
		OnErr(8, XPACK_ERROR_8)
	EndIf
	IsChange = 0
	Return -1
End Function

Function xPack.Close() As Integer XPACK_EXPORT
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 保存文件 [存在修改时自动存储]
	If IsChange Then
		Save(0)
	EndIf
	' 重置数据
	CloseHandle(FileHandle)
	FileHandle = NULL
	If LDB Then
		Delete(LDB)
		LDB = NULL
	EndIf
	Return -1
End Function

Function xPack.IsOpen() As Integer XPACK_EXPORT
	Return IIf(FileHandle, -1, 0)
End Function



' 包信息操作
Function xPack.FileCount() As UInteger XPACK_EXPORT
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 读取数据
	Return LDB->StructCount
End Function

Function xPack.SetFileInfoExtSize(iNewVal As UShort) As Integer XPACK_EXPORT
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 已经添加过文件就不能调整了
	If LDB->StructCount > 0 Then
		OnErr(9, XPACK_ERROR_9)
	EndIf
	' 重新申请LDB
	Dim NewLDB As xBsmm Ptr = New xBsmm(SizeOf(xPack_FileInfo) + iNewVal, 32, 0)
	If NewLDB = NULL Then
		OnErr(3, XPACK_ERROR_3)
	EndIf
	Delete(LDB)
	LDB = NewLDB
	' 写入数据
	PackHead.InfoSize = iNewVal
	IsChange = -1
	Return -1
End Function

Function xPack.GetFileInfoExtSize() As UShort XPACK_EXPORT
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 读取数据
	Return PackHead.InfoSize
End Function

Function xPack.SetDefaultCompressLevel(iNewVal As UInteger) As Integer XPACK_EXPORT
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 写入数据
	If iNewVal > 3 Then iNewVal = 3
	PackHead.PackFlag And= Not(XPACK_COMP_BITS Shl 2)
	PackHead.PackFlag Or= (iNewVal Shl 2)
	IsChange = -1
	Return -1
End Function

Function xPack.GetDefaultCompressLevel() As UInteger XPACK_EXPORT
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 读取数据
	Return (PackHead.PackFlag Shr 2) And XPACK_COMP_BITS
End Function



' 文件信息操作
Function xPack.GetFileInfo(iPos As UInteger) As xPack_FileInfo Ptr XPACK_EXPORT
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 从LDB读取数据
	Dim pInfo As xPack_FileInfo Ptr = LDB->GetPtrStruct(iPos)
	If pInfo Then
		Return pInfo
	Else
		OnErr(10, XPACK_ERROR_10)
	EndIf
End Function

Function xPack.GetFileSize(iPos As UInteger) As UInteger XPACK_EXPORT
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 从LDB读取数据
	Dim pInfo As xPack_FileInfo Ptr = LDB->GetPtrStruct(iPos)
	If pInfo Then
		Return pInfo->FileSize
	Else
		OnErr(10, XPACK_ERROR_10)
	EndIf
End Function

Function xPack.GetFileDataSize(iPos As UInteger) As UInteger XPACK_EXPORT
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 从LDB读取数据
	Dim pInfo As xPack_FileInfo Ptr = LDB->GetPtrStruct(iPos)
	If pInfo Then
		Return pInfo->DataSize
	Else
		OnErr(10, XPACK_ERROR_10)
	EndIf
End Function

Function xPack.GetFileHash(iPos As UInteger) As Integer XPACK_EXPORT
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 从LDB读取数据
	Dim pInfo As xPack_FileInfo Ptr = LDB->GetPtrStruct(iPos)
	If pInfo Then
		Return pInfo->FileHash
	Else
		OnErr(10, XPACK_ERROR_10)
	EndIf
End Function

Function xPack.GetFileCompLevel(iPos As UInteger) As UInteger XPACK_EXPORT
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 从LDB读取数据
	Dim pInfo As xPack_FileInfo Ptr = LDB->GetPtrStruct(iPos)
	If pInfo Then
		Return pInfo->CompLevel
	Else
		OnErr(10, XPACK_ERROR_10)
	EndIf
End Function

Function xPack.GetFileType(iPos As UInteger) As UByte XPACK_EXPORT
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 从LDB读取数据
	Dim pInfo As xPack_FileInfo Ptr = LDB->GetPtrStruct(iPos)
	If pInfo Then
		Return pInfo->FileType
	Else
		OnErr(10, XPACK_ERROR_10)
	EndIf
End Function

Function xPack.SetFileType(iPos As UInteger, iNewVal As UByte) As Integer XPACK_EXPORT
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 修改数据
	Dim pInfo As xPack_FileInfo Ptr = LDB->GetPtrStruct(iPos)
	If pInfo Then
		pInfo->FileType = iNewVal
		Return -1
	Else
		OnErr(10, XPACK_ERROR_10)
	EndIf
End Function



' 文件操作
Function xPack.AppendFile(sFile As ZString Ptr, iCompLevel As Integer = -1, iFileType As UByte = XPACK_FILETYPE_OTHER) As UInteger XPACK_EXPORT
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 打开文件
	Dim pFile As HANDLE = Open_File(sFile)
	If pFile = 0 Then
		OnErr(1, XPACK_ERROR_1)
	EndIf
	' 读取文件到内存
	Dim iSize As UInteger = File_Len(pFile)
	Dim pData As Any Ptr = malloc(iSize)
	iSize = Get_File(pFile, pData, 0, iSize)
	CloseHandle(pFile)
	' 添加文件数据
	Function = AppendData(pData, iSize, iCompLevel, iFileType)
	free(pData)
End Function

Function xPack.AppendData(pInData As Any Ptr, iInSize As UInteger, iCompLevel As Integer = -1, iFileType As UByte = XPACK_FILETYPE_OTHER) As UInteger XPACK_EXPORT
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 添加数据长度为 0
	If iInSize = 0 Then
		OnErr(7, XPACK_ERROR_7)
	EndIf
	' 创建文件列表项
	Dim iFilePos As UInteger = LDB->AppendStruct()
	Dim pInfo As xPack_FileInfo Ptr = LDB->GetPtrStruct(iFilePos)
	If pInfo = NULL Then
		OnErr(6, XPACK_ERROR_6)
	EndIf
	' 计算文件Hash值、写入固定属性
	pInfo->FileHash = CityHash32(pInData, iInSize)
	pInfo->DataAddr = PackHead.LDB_Addr
	pInfo->FileSize = iInSize
	pInfo->FileType = iFileType
	pInfo->Reserve = 0
	If iCompLevel < 0 Then iCompLevel = (PackHead.PackFlag Shr 2) And XPACK_COMP_BITS
	pInfo->CompLevel = IIf(iCompLevel > 3, 3, iCompLevel)
	' 写入文件数据
	Dim pData As Any Ptr
	Dim iSize As UInteger = xPack_Compress(@pInfo->CompLevel, pInData, iInSize, @pData, iInSize)
	Dim iPutSize As UInteger = Put_File(FileHandle, pData, FileOffset + PackHead.LDB_Addr, iSize)
	free(pData)
	If iPutSize = iSize Then
		pInfo->DataSize = iSize
		PackHead.LDB_Addr += iSize
	Else
		LDB->DeleteStruct(iFilePos)
		OnErr(8, XPACK_ERROR_8)
	EndIf
	' 设置修改标记，返回文件 pos
	IsChange = -1
	Return iFilePos
End Function

Function xPack.UnpackFile(iPos As UInteger, sFile As ZString Ptr) As UInteger XPACK_EXPORT
	Dim pData As Any Ptr
	Dim iSize As UInteger = UnpackData(iPos, @pData, -1)
	If iSize Then
		Dim iPutSize As UInteger = PutFile(sFile, pData, 0, iSize)
		free(pData)
		If iPutSize = iSize Then
			Return -1
		Else
			OnErr(8, XPACK_ERROR_8)
		EndIf
	EndIf
	' 释放内存
	If pData Then free(pData)
End Function

Function xPack.UnpackData(iPos As UInteger, sOutData As Any Ptr, bAlloc As Integer = 0) As UInteger XPACK_EXPORT
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 读取文件信息
	Dim pInfo As xPack_FileInfo Ptr = LDB->GetPtrStruct(iPos)
	If pInfo Then
		Dim pOut As Any Ptr = sOutData
		' 如果要求程序申请内存 [需要使用 free 释放] [会自动增加字符串截断]
		If bAlloc Then
			Dim pAlloc As ZString Ptr = malloc(pInfo->FileSize + 2)
			If pAlloc Then
				*Cast(Any Ptr Ptr, sOutData) = pAlloc
				pOut = pAlloc
				pAlloc[pInfo->FileSize] = 0
				pAlloc[pInfo->FileSize + 1] = 0
			Else
				OnErr(3, XPACK_ERROR_3)
			EndIf
		EndIf
		' 读取文件数据（不需要解压时直接读到目标缓冲不中转）
		If (pInfo->CompLevel And XPACK_COMP_BITS) = 0 Then
			Get_File(FileHandle, pOut, FileOffset + pInfo->DataAddr, pInfo->DataSize)
		Else
			' 解压文件数据
			Dim pData As Any Ptr = malloc(pInfo->DataSize)
			Get_File(FileHandle, pData, FileOffset + pInfo->DataAddr, pInfo->DataSize)
			xPack_DeCompress(pInfo->CompLevel, pData, pInfo->DataSize, pOut, pInfo->FileSize)
			free(pData)
		EndIf
		' hash 校验
		If pInfo->FileHash = CityHash32(pOut, pInfo->FileSize) Then
			Return pInfo->FileSize
		Else
			OnErr(11, XPACK_ERROR_11)
		EndIf
	Else
		OnErr(10, XPACK_ERROR_10)
	EndIf
End Function

Function xPack.DeleteFile(iPos As UInteger) As Integer XPACK_EXPORT
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 删除文件信息
	If LDB->DeleteStruct(iPos) Then
		IsChange = -1
		Return -1
	Else
		OnErr(10, XPACK_ERROR_10)
	EndIf
End Function
