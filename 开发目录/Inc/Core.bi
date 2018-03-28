


' 包操作
Function xPack.Open(sFile As ZString Ptr) As Integer
	' 已经打开过文件则先关闭
	If IsOpen Then
		Close()
	EndIf
	IsChange = 0
	' 文件不存在则创建文件
	If FileExists(sFile) = 0 Then
		Return Create(sFile)
	EndIf
	' 打开文件
	FileHandle = Open_File(sFile)
	If FileHandle = 0 Then
		OnErr(1, XPACK_ERROR_1)
	EndIf
	' 读取信息段，验证文件头和版本
	Get_File(FileHandle, @PackHead, 0, SizeOf(xPack_FileHead))
	If PackHead.FileHead <> "xpk" Then
		CloseHandle(FileHandle)
		FileHandle = NULL
		ZeroMemory(@PackHead, SizeOf(xPack_FileHead))
		OnErr(2, XPACK_ERROR_2)
	EndIf
	If PackHead.Ver_Cpt <> VerCpt Then
		CloseHandle(FileHandle)
		FileHandle = NULL
		ZeroMemory(@PackHead, SizeOf(xPack_FileHead))
		OnErr(3, XPACK_ERROR_3)
	EndIf
	' 读取、解压文件列表(LDB段)
	LDB = New xBsmm(SizeOf(xPack_FileInfo) + PackHead.InfoSize, 32, PackHead.FileCount)
	If PackHead.FileCount Then
		Dim LDB_Data As Any Ptr = malloc(PackHead.LDB_Size)
		Get_File(FileHandle, LDB_Data, PackHead.LDB_Addr, PackHead.LDB_Size)
		Dim LDB_DeCompSize As UInteger = (SizeOf(xPack_FileInfo) + PackHead.InfoSize) * PackHead.FileCount
		Dim DeCompSize As UInteger = Lzma_Uncompress(LDB_Data, LDB->StructMemory, PackHead.LDB_Size, LDB_DeCompSize)
		free(LDB_Data)
		' 校验文件列表数据
		If DeCompSize <> LDB_DeCompSize Then
			CloseHandle(FileHandle)
			FileHandle = NULL
			ZeroMemory(@PackHead, SizeOf(xPack_FileHead))
			Delete LDB
			OnErr(4, XPACK_ERROR_4)
		EndIf
		If CityHash32(LDB->StructMemory, LDB_DeCompSize) <> PackHead.LDB_Hash Then
			CloseHandle(FileHandle)
			FileHandle = NULL
			ZeroMemory(@PackHead, SizeOf(xPack_FileHead))
			Delete LDB
			OnErr(5, XPACK_ERROR_5)
		EndIf
		' 文件打开成功
		LDB->StructCount = PackHead.FileCount
		IsOpen = -1
	EndIf
End Function

Function xPack.Create(sFile As ZString Ptr, iInfoSize As Integer = 0) As Integer
	If FileExists(sFile) = 0 Then
		If iInfoSize < 0 Then iInfoSize = 0
		If iInfoSize > 65535 Then iInfoSize = 65535
		Dim NewPackHead As xPack_FileHead
		NewPackHead.FileHead = "xpk"
		NewPackHead.Ver_Cpt = VerCpt
		NewPackHead.Ver_Sub = VerSub
		NewPackHead.InfoSize = iInfoSize
		NewPackHead.FileCount = 0
		NewPackHead.LDB_Addr = SizeOf(xPack_FileHead)
		NewPackHead.LDB_Size = 0
		NewPackHead.LDB_Hash = 0
		NewPackHead.Ext_Addr = 0
		NewPackHead.Ext_Comp = 0
		NewPackHead.Ext_Size = 0
		NewPackHead.Ext_Hash = 0
		PutFile(sFile, @NewPackHead, 0, SizeOf(xPack_FileHead))
	EndIf
	Return Open(sFile)
End Function

Function xPack.Save(bIsRebuild As Integer) As Integer
	' 压缩文件列表
	Dim iSize As UInteger = (SizeOf(xPack_FileInfo) + PackHead.InfoSize) * LDB->StructCount
	Dim pData As Any Ptr = malloc(iSize)
	PackHead.LDB_Size = Lzma_Compress(LDB->StructMemory, pData, iSize, iSize)
	If PackHead.LDB_Size = 0 Then
		free(pData)
		OnErr(9, XPACK_ERROR_9)
	EndIf
	' 写入文件列表
	Dim iRet As Integer = Put_File(FileHandle, pData, PackHead.LDB_Addr, PackHead.LDB_Size)
	free(pData)
	If iRet = 0 Then
		OnErr(10, XPACK_ERROR_10)
	EndIf
	' 更新、写入文件头数据
	PackHead.FileCount = LDB->StructCount
	PackHead.LDB_Hash = CityHash32(LDB->StructMemory, iSize)
	PackHead.Ver_Cpt = VerCpt
	PackHead.Ver_Sub = VerSub
	If Put_File(FileHandle, @PackHead, 0, SizeOf(xPack_FileHead)) = 0 Then
		OnErr(11, XPACK_ERROR_11)
	EndIf
	IsChange = 0
	Return -1
End Function

Sub xPack.Close()
	' 保存文件 [存在修改时自动存储]
	If IsChange Then
		Save(0)
	EndIf
	' 重置数据
	IsOpen = 0
	IsChange = 0
	If FileHandle Then
		CloseHandle(FileHandle)
		FileHandle = NULL
	EndIf
	If LDB Then
		free(LDB)
		LDB = NULL
	EndIf
	ZeroMemory(@PackHead, SizeOf(xPack_FileHead))
End Sub



' 包信息操作
Function xPack.GetExtData(pOutData As Any Ptr) As Integer
	
End Function

Function xPack.SetExtData(pInData As Any Ptr, iInSize As UInteger) As Integer
	
End Function



' 文件信息操作
Function xPack.GetFileInfo(idx As UInteger, bUsePos As Integer = 0) As xPack_FileInfo Ptr
	If bUsePos = 0 Then
		idx = GetFilePos(idx)
	EndIf
	Return LDB->GetPtrStruct(idx)
End Function

Function xPack.GetFileSize(idx As UInteger, bUsePos As Integer = 0) As UInteger
	Dim pInfo As xPack_FileInfo Ptr = GetFileInfo(idx, bUsePos)
	If pInfo Then
		Return pInfo->FileSize
	EndIf
End Function

Function xPack.GetDataSize(idx As UInteger, bUsePos As Integer = 0) As UInteger
	Dim pInfo As xPack_FileInfo Ptr = GetFileInfo(idx, bUsePos)
	If pInfo Then
		Return pInfo->DataSize
	EndIf
End Function

Function xPack.GetFileFlag(idx As UInteger, bUsePos As Integer = 0) As Integer
	Dim pInfo As xPack_FileInfo Ptr = GetFileInfo(idx, bUsePos)
	If pInfo Then
		Return pInfo->FileFlag
	EndIf
End Function

Function xPack.GetFileHash(idx As UInteger, bUsePos As Integer = 0) As Integer
	Dim pInfo As xPack_FileInfo Ptr = GetFileInfo(idx, bUsePos)
	If pInfo Then
		Return pInfo->FileHash
	EndIf
End Function

Function xPack.GetFilePos(idx As UInteger) As UInteger
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
	Dim pInfo As xPack_FileInfo Ptr = LDB->GetPtrStruct(iPos)
	If pInfo Then
		Return pInfo->FileIndex
	EndIf
End Function



' 文件操作
Function xPack.AppendFile(idx As UInteger, sFile As ZString Ptr, iCompLevel As Integer = -1) As UInteger
	Dim pFile As HANDLE = Open_File(sFile)
	If pFile = 0 Then
		OnErr(2, XPACK_ERROR_2)
	EndIf
	' 读取文件到内存
	Dim iSize As UInteger = File_Len(pFile)
	Dim pData As Any Ptr = malloc(iSize)
	iSize = Get_File(pFile, pData, 0, iSize)
	CloseHandle(pFile)
	' 添加文件数据
	If iSize Then
		Function = AppendData(idx, pData, iSize, iCompLevel)
		free(pData)
	Else
		free(pData)
		OnErr(6, XPACK_ERROR_6)
	EndIf
End Function

Function xPack.AppendData(idx As UInteger, pInData As Any Ptr, iInSize As UInteger, iCompLevel As Integer = -1) As UInteger
	If iCompLevel < 0 Then iCompLevel = Default_CompLevel
	If iCompLevel > 3 Then iCompLevel = 3
	' 创建文件列表项
	Dim iFilePos As UInteger = LDB->AppendStruct()
	Dim pInfo As xPack_FileInfo Ptr = LDB->GetPtrStruct(iFilePos)
	If pInfo = NULL Then
		OnErr(7, XPACK_ERROR_7)
	EndIf
	' 计算文件Hash值、写入固定属性
	Dim iDataHash As UInteger = CityHash32(pInData, iInSize)
	pInfo->FileHash = iDataHash
	pInfo->DataAddr = PackHead.LDB_Addr
	pInfo->FileSize = iInSize
	pInfo->FileRefs = 0
	pInfo->FileIndex = idx
	' 写入文件
	Select Case iCompLevel
		Case 1
			' LZ4压缩
			Dim pData As Any Ptr = malloc(iInSize)
			Dim iCompSize As UInteger = LZ4_compress_default(pInData, pData, iInSize, iInSize)
			If iCompSize Then
				If Put_File(FileHandle, pData, PackHead.LDB_Addr, iCompSize) Then
					pInfo->DataSize = iCompSize
					pInfo->FileFlag = XPACK_COMP_Level1
					PackHead.LDB_Addr += iCompSize
					free(pData)
				Else
					free(pData)
					LDB->DeleteStruct(iFilePos)
					OnErr(8, XPACK_ERROR_8)
				EndIf
			Else
				free(pData)
				iCompLevel = 0
			EndIf
		Case 2
			' LZMA快速压缩
			Dim pData As Any Ptr = malloc(iInSize)
			Dim iCompSize As UInteger
			Dim arrProps(4) As UByte
			Dim iPropsSize As UInteger = 5
			If LzmaCompress(pData, @iCompSize, pInData, iInSize, @arrProps(0), @iPropsSize, 1) = SZ_OK Then
				If Put_File(FileHandle, pData, PackHead.LDB_Addr, iCompSize) Then
					pInfo->DataSize = iCompSize
					pInfo->FileFlag = XPACK_COMP_Level1
					PackHead.LDB_Addr += iCompSize
					free(pData)
				Else
					free(pData)
					LDB->DeleteStruct(iFilePos)
					OnErr(8, XPACK_ERROR_8)
				EndIf
			Else
				free(pData)
				iCompLevel = 0
			EndIf
		Case 3
			' LZMA普通压缩
			Dim pData As Any Ptr = malloc(iInSize)
			Dim iCompSize As UInteger
			Dim arrProps(4) As UByte
			Dim iPropsSize As UInteger = 5
			If LzmaCompress(pData, @iCompSize, pInData, iInSize, @arrProps(0), @iPropsSize, 6) = SZ_OK Then
				If Put_File(FileHandle, pData, PackHead.LDB_Addr, iCompSize) Then
					pInfo->DataSize = iCompSize
					pInfo->FileFlag = XPACK_COMP_Level1
					PackHead.LDB_Addr += iCompSize
					free(pData)
				Else
					free(pData)
					LDB->DeleteStruct(iFilePos)
					OnErr(8, XPACK_ERROR_8)
				EndIf
			Else
				free(pData)
				iCompLevel = 0
			EndIf
	End Select
	If iCompLevel = 0 Then
		' 不压缩
		If Put_File(FileHandle, pInData, PackHead.LDB_Addr, iInSize) Then
			pInfo->DataSize = iInSize
			pInfo->FileFlag = XPACK_COMP_NOUSED
			PackHead.LDB_Addr += iInSize
		Else
			LDB->DeleteStruct(iFilePos)
			OnErr(8, XPACK_ERROR_8)
		EndIf
	EndIf
	IsChange = -1
	Return iFilePos
End Function

Function xPack.UnpackFile(idx As UInteger, sFile As ZString Ptr) As UInteger
	
End Function

Function xPack.UnpackData(idx As UInteger, sOutData As Any Ptr) As UInteger
	
End Function

Function xPack.DeleteFile(idx As UInteger, bUsePos As Integer = 0) As Integer
	
End Function
