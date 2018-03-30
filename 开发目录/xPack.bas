#Include Once "Windows.bi"
#LibPath "Lib"



#Include "Inc\Lib\File.bi"
#Include "Inc\Lib\xBsmm.bi"
#Include "Inc\Lib\lzma.bi"
#Include "Inc\Lib\lz4.bi"
#Include "Inc\Lib\CityHash.bi"



#Include "Inc\xPack.bi"





#If __FB_OUT_EXE__
	
	Sub OnError(iErrCode As Integer, sErrText As ZString Ptr)
		Print iErrCode, *sErrText
	End Sub
	
	Dim xpk As xPack
	
	xpk.OnError = @OnError
	xpk.Open(ExePath() & "\1.xpk")
	
	Dim s As ZString * 128
	If xpk.FileCount > 0 Then
		For i As Integer = 1 To xpk.FileCount
			xpk.UnpackData(i, @s)
			Print s
			'xpk.UnpackFile(i, ExePath() & "\" & i & ".txt")
		Next
	EndIf
	
	xpk.AppendData(@"1111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000", 100, 0)
	xpk.AppendData(@"9999999999888888888877777777776666666666555555555544444444443333333333222222222211111111110000000000", 100, 1)
	xpk.AppendData(@"1111111111111111111111111111114444444444444444444444444444447777777777777777777777777777770000000000", 100, 2)
	xpk.AppendData(@"9999999999999999999999999999996666666666666666666666666666663333333333333333333333333333330000000000", 100, 3)
	xpk.Save(0)
	xpk.Close()
	
	Sleep
	
#ElseIf __FB_OUT_DLL__
	
	/'
	Function DllMain Alias "DllMain" (hinstDLL As HINSTANCE, fdwReason As DWORD, lpvReserved As LPVOID) As BOOL
		Select Case fdwReason
			Case DLL_PROCESS_ATTACH
				xyLib_Init(hinstDLL)
			Case DLL_PROCESS_DETACH
				xyLib_Unit()
			Case DLL_THREAD_ATTACH
				
			Case DLL_THREAD_DETACH
				
		End Select
		Return 1
	End Function
	'/
	
#EndIf
