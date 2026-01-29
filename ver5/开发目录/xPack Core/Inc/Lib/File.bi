'==================================================================================
	'★ xywh File System Object 文件库
	'#-------------------------------------------------------------------------------
	'# 功能 : 
	'# 说明 : 
'==================================================================================



#Ifndef xywh_library_file
	#Define xywh_library_file
	
	
	
	Function PutFile(ByVal FileName As ZString Ptr,ByVal Buffer As Any Ptr,ByVal Addr As Integer,ByVal Lenght As Integer) As Integer
		Dim FileHdr As HANDLE = CreateFile(FileName,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL)
		If FileHdr <> INVALID_HANDLE_VALUE Then
			Dim FileAddr As Integer = SetFilePointer(FileHdr,Addr,0,FILE_BEGIN)
			If FileAddr <> HFILE_ERROR Then
				Dim TOL As OVERLAPPED
				Dim WriteBytes As Integer
				TOL.Offset = FileAddr
				If WriteFile(FileHdr,Buffer,Lenght,@WriteBytes,@TOL) Then
					CloseHandle(FileHdr)
					Return WriteBytes
				EndIf
			EndIf
		EndIf
	End Function
	
	Function GetFile(ByVal FileName As ZString Ptr,ByVal Buffer As Any Ptr,ByVal Addr As Integer,ByVal Lenght As Integer) As Integer
		Dim FileHdr As HANDLE = CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL)
		If FileHdr <> INVALID_HANDLE_VALUE Then
			Dim FileAddr As Integer = SetFilePointer(FileHdr,Addr,0,FILE_BEGIN)
			If FileAddr <> HFILE_ERROR Then
				Dim TOL As OVERLAPPED
				Dim ReadBytes As Integer
				TOL.Offset = FileAddr
				If ReadFile(FileHdr,Buffer,Lenght,@ReadBytes,@TOL) Then
					CloseHandle(FileHdr)
					Return ReadBytes
				EndIf
			EndIf
		EndIf
	End Function
	
	Function NewFile(ByVal FileName As ZString Ptr) As Integer
		Dim FileHdr As HANDLE = CreateFile(FileName,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL)
		If FileHdr <> INVALID_HANDLE_VALUE Then
			CloseHandle(FileHdr)
			Return -1
		EndIf
	End Function
	
	Function FileExists(ByVal FileName As ZString Ptr) As Integer
		Dim FileHdr As HANDLE = CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL)
		Dim ErrorID As Integer = GetLastError()
		CloseHandle(FileHdr)
		If FileHdr = INVALID_HANDLE_VALUE Then
			If ErrorID = 2 Then
				Return 0
			Else
				Return -1
			EndIf
		Else
			Return -1
		EndIf
	End Function
	
	Function FileLen(ByVal FileName As ZString Ptr) As UInteger
		Dim FileHdr As HANDLE = CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL)
		If FileHdr <> INVALID_HANDLE_VALUE Then
			FileLen = GetFileSize(FileHdr,NULL)
			CloseHandle(FileHdr)
		EndIf
	End Function
	
	Function SetFileSize(ByVal FileName As ZString Ptr,ByVal FileSize As Integer) As Integer
		Dim FileHdr As HANDLE = CreateFile(FileName,GENERIC_READ Or GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL)
		If FileHdr <> INVALID_HANDLE_VALUE Then
			SetFilePointer(FileHdr,FileSize,0,FILE_BEGIN)
			SetEndOfFile(FileHdr)
			CloseHandle(FileHdr)
			Return -1
		EndIf
	End Function
	
	Function Open_File(ByVal FileName As ZString Ptr,OnlyRead As Integer = 0) As HANDLE
		Dim FileHdr As HANDLE = CreateFile(FileName,GENERIC_READ Or IIf(OnlyRead,0,GENERIC_WRITE),FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL)
		If FileHdr <> INVALID_HANDLE_VALUE Then
			Return FileHdr
		EndIf
	End Function
	
	Function Put_File(ByVal FileHdr As HANDLE,ByVal Buffer As Any Ptr,ByVal Addr As Integer,ByVal Lenght As Integer) As Integer
		Dim FileAddr As Integer = SetFilePointer(FileHdr,Addr,0,FILE_BEGIN)
		If FileAddr <> HFILE_ERROR Then
			Dim TOL As OVERLAPPED
			Dim WriteBytes As Integer
			TOL.Offset = FileAddr
			If WriteFile(FileHdr,Buffer,Lenght,@WriteBytes,@TOL) Then
				Return WriteBytes
			EndIf
		EndIf
	End Function
	
	Function Get_File(ByVal FileHdr As HANDLE,ByVal Buffer As Any Ptr,ByVal Addr As Integer,ByVal Lenght As Integer) As Integer
		Dim FileAddr As Integer = SetFilePointer(FileHdr,Addr,0,FILE_BEGIN)
		If FileAddr <> HFILE_ERROR Then
			Dim TOL As OVERLAPPED
			Dim ReadBytes As Integer
			TOL.Offset = FileAddr
			If ReadFile(FileHdr,Buffer,Lenght,@ReadBytes,@TOL) Then
				Return ReadBytes
			EndIf
		EndIf
	End Function
	
	Function File_Len(ByVal FileHdr As HANDLE) As UInteger
		Return GetFileSize(FileHdr,NULL)
	End Function
#EndIf
