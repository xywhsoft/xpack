


' 包操作
Function xPack.Open(sFile As ZString Ptr) As Integer
	
End Function

Function xPack.Create(sFile As ZString Ptr, iInfoSize As Integer) As Integer
	
End Function

Function xPack.Save(bIsRebuild As Integer) As Integer
	
End Function

Sub xPack.Close()
	
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
