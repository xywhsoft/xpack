


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
Function xPack.Open(sFile As ZString Ptr) As Integer
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
		NewPackHead.PackFlag = XPACK_COMP_LEVEL2 Or (XPACK_COMP_LEVEL2 Shl 2)
		NewPackHead.InfoSize = 0
		NewPackHead.FileCount = 0
		NewPackHead.LDB_Addr = SizeOf(xPack_FileHead)
		NewPackHead.LDB_Size = 0
		NewPackHead.LDB_Hash = 0
		PutFile(sFile, @NewPackHead, 0, SizeOf(xPack_FileHead))
	EndIf
	' 打开文件
	FileHandle = Open_File(sFile)
	If FileHandle = 0 Then
		OnErr(1, XPACK_ERROR_1)
	EndIf
	' 读取信息段，验证文件头和版本
	Get_File(FileHandle, @PackHead, 0, SizeOf(xPack_FileHead))
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
	' 解压文件列表(LDB段)
	If PackHead.FileCount Then
		Dim LDB_Data As Any Ptr = malloc(PackHead.LDB_Size)
		Get_File(FileHandle, LDB_Data, PackHead.LDB_Addr, PackHead.LDB_Size)
		Dim LDB_DeCompSize As UInteger = (SizeOf(xPack_FileInfo) + PackHead.InfoSize) * PackHead.FileCount
		Dim DeCompSize As UInteger = xPack_DeCompress(PackHead.PackFlag, LDB_Data, PackHead.LDB_Size, LDB->StructMemory, LDB_DeCompSize)
		free(LDB_Data)
		' 校验文件列表数据
		If (DeCompSize <> LDB_DeCompSize) OrElse (CityHash32(LDB->StructMemory, LDB_DeCompSize) <> PackHead.LDB_Hash) Then
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

Function xPack.Save(bIsRebuild As Integer) As Integer
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
		Dim iRet As Integer = Put_File(FileHandle, pData, PackHead.LDB_Addr, PackHead.LDB_Size)
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
	If Put_File(FileHandle, @PackHead, 0, SizeOf(xPack_FileHead)) = 0 Then
		OnErr(8, XPACK_ERROR_8)
	EndIf
	IsChange = 0
	Return -1
End Function

Function xPack.Close() As Integer
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
		free(LDB)
		LDB = NULL
	EndIf
	Return -1
End Function

Function xPack.IsOpen() As Integer
	Return IIf(FileHandle, -1, 0)
End Function



' 文件信息操作
Function xPack.GetFileInfo(idx As UInteger, bUsePos As Integer = 0) As xPack_FileInfo Ptr
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 从LDB读取数据
	If bUsePos = 0 Then
		idx = GetFilePos(idx)
	EndIf
	Return LDB->GetPtrStruct(idx)
End Function

Function xPack.GetFileSize(idx As UInteger, bUsePos As Integer = 0) As UInteger
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 从LDB读取数据
	Dim pInfo As xPack_FileInfo Ptr = GetFileInfo(idx, bUsePos)
	If pInfo Then
		Return pInfo->FileSize
	EndIf
End Function

Function xPack.GetDataSize(idx As UInteger, bUsePos As Integer = 0) As UInteger
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 从LDB读取数据
	Dim pInfo As xPack_FileInfo Ptr = GetFileInfo(idx, bUsePos)
	If pInfo Then
		Return pInfo->DataSize
	EndIf
End Function

Function xPack.GetFileHash(idx As UInteger, bUsePos As Integer = 0) As Integer
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 从LDB读取数据
	Dim pInfo As xPack_FileInfo Ptr = GetFileInfo(idx, bUsePos)
	If pInfo Then
		Return pInfo->FileHash
	EndIf
End Function



' idx 和 pos 转换
Function xPack.GetFilePos(idx As UInteger) As UInteger
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	' 遍历 LDB 计算位置
	If LDB->StructCount Then
		Dim pInfo As xPack_FileInfo Ptr
		For i As Integer = 1 To LDB->StructCount
			pInfo = LDB->GetPtrStruct(i)
			If pInfo Then
				If pInfo->FileIndex = idx Then
					Return i
				EndIf
			EndIf
		Next
	EndIf
End Function

Function xPack.GetFileIdx(iPos As UInteger) As UInteger
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	Dim pInfo As xPack_FileInfo Ptr = LDB->GetPtrStruct(iPos)
	If pInfo Then
		Return pInfo->FileIndex
	EndIf
End Function



' 文件操作
Function xPack.AppendFile(idx As UInteger, sFile As ZString Ptr, iCompLevel As Integer = -1) As UInteger
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
	Function = AppendData(idx, pData, iSize, iCompLevel)
	free(pData)
End Function

Function xPack.AppendData(idx As UInteger, pInData As Any Ptr, iInSize As UInteger, iCompLevel As Integer = -1) As UInteger
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
	pInfo->FileRefs = 0
	pInfo->FileIndex = idx
	If iCompLevel < 0 Then iCompLevel = (PackHead.PackFlag Shr 2) And XPACK_COMP_BITS
	pInfo->FileFlag = IIf(iCompLevel > 3, 3, iCompLevel)
	' 写入文件数据
	Dim pData As Any Ptr
	Dim iSize As UInteger = xPack_Compress(@pInfo->FileFlag, pInData, iInSize, @pData, iInSize)
	Dim iPutSize As UInteger = Put_File(FileHandle, pData, PackHead.LDB_Addr, iSize)
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

Function xPack.UnpackFile(idx As UInteger, sFile As ZString Ptr) As UInteger
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	
End Function

Function xPack.UnpackData(idx As UInteger, sOutData As Any Ptr) As UInteger
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	
End Function

Function xPack.DeleteFile(idx As UInteger, bUsePos As Integer = 0) As Integer
	' 必须先打开压缩包
	If FileHandle = NULL Then
		OnErr(5, XPACK_ERROR_5)
	EndIf
	
End Function
