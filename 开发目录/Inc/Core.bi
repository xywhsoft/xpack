


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
	If PackHead.FileCount Then
		LDB = New xBsmm(SizeOf(xPack_FileInfo) + PackHead.InfoSize, 32, PackHead.FileCount)
		Dim LDB_Data As Any Ptr = malloc(PackHead.LDB_Size)
		Get_File(FileHandle, LDB_Data, PackHead.LDB_Addr, PackHead.LDB_Size)
		Dim LDB_DeCompSize As UInteger = (SizeOf(xPack_FileInfo) + PackHead.InfoSize) * PackHead.FileCount
		Dim DeCompSize As UInteger = LZ4_decompress_safe(LDB_Data, LDB->StructMemory, PackHead.LDB_Size, LDB_DeCompSize)
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
Function xPack.AppendFile(sFile As ZString Ptr, iCompLevel As Integer = -1) As UInteger
	Dim pFile As HANDLE = Open_File(sFile)
	If pFile Then
		Dim iSize As UInteger = File_Len(pFile)
		Dim pData As Any Ptr = malloc(iSize)
		iSize = Get_File(pFile, pData, 0, iSize)
		CloseHandle(pFile)
		If iSize Then
			Function = AppendData(pData, iSize, iCompLevel)
		EndIf
		free(pData)
	EndIf
End Function

Function xPack.AppendData(pInData As Any Ptr, iInSize As UInteger, iCompLevel As Integer = -1) As UInteger
	If iCompLevel < 0 Then iCompLevel = Default_CompLevel
	If iCompLevel > 3 Then iCompLevel = 3
	Select Case iCompLevel
		Case 0
			' 不压缩
		Case 1
			' LZ4压缩
		Case 2
			' LZMA快速压缩
		Case 3
			' LZMA普通压缩
	End Select
	
	'Put_File(FileHandle, )
End Function

Function xPack.UnpackFile(idx As UInteger, sFile As ZString Ptr) As UInteger
	
End Function

Function xPack.UnpackData(idx As UInteger, sOutData As Any Ptr) As UInteger
	
End Function

Function xPack.DeleteFile(idx As UInteger, bUsePos As Integer = 0) As Integer
	
End Function
