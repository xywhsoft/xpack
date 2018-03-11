


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
	
End Function

Function xPack.GetFileSize(idx As UInteger) As UInteger
	
End Function

Function xPack.GetDataSize(idx As UInteger) As UInteger
	
End Function

Function xPack.GetFileFlag(idx As UInteger) As Integer
	
End Function

Function xPack.GetFileHash(idx As UInteger) As Integer
	
End Function

Function xPack.GetFilePos(idx As UInteger) As UInteger
	
End Function

Function xPack.GetFileIdx(iPos As UInteger) As UInteger
	
End Function

Function xPack.ExistsIdx(idx As UInteger) As Integer
	
End Function



' 文件操作
Function xPack.AppendFile(sFile As ZString Ptr) As UInteger
	
End Function

Function xPack.AppendData(pInData As Any Ptr, iInSize As UInteger) As UInteger
	
End Function

Function xPack.UnpackFile(idx As UInteger, sFile As ZString Ptr) As UInteger
	
End Function

Function xPack.UnpackData(idx As UInteger, sOutData As Any Ptr) As UInteger
	
End Function

Function xPack.DeleteFile(idx As UInteger, bUsePos As Integer = 0) As Integer
	
End Function
