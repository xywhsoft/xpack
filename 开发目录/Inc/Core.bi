


#Define XPACK_TYPE_OTHER		0
#Define XPACK_TYPE_XGI			1
#Define XPACK_TYPE_STB			2
#Define XPACK_TYPE_BASS			101
#Define XPACK_TYPE_TEXT			201


/'
' idx 和 pos 转换
Function xPack.GetFilePos(idx As Integer) As UInteger
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
	OnErr(10, XPACK_ERROR_10)
End Function
'/
