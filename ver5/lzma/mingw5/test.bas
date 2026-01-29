
#Include "Windows.bi"
#include "string.bi"
#Include "lzma.bi"
#Include "xrtl.bi"





Dim pProps As Any Ptr = Allocate(5)
Dim iProps As UInteger = 5

For i As Integer = 1 To 7 Step 5
	Dim sFile As ZString * 260
	
	sFile = "C:\test\1.bmp"
	
	Dim pBuff As Any Ptr
	Dim iSize As UInteger = xRtl.File.eRead(sFile, @pBuff)
	
	Print sFile & "  文件原始长度 : ", iSize, "Byte"
	
	Dim pOutBuff As Any Ptr = Allocate(iSize)
	Dim iOutSize As UInteger = iSize
	
	Dim ST As Double = Timer()
	If LzmaCompress(pOutBuff, @iOutSize, pBuff, iSize, pProps, @iProps, i) = SZ_OK Then
		Print "压缩后的长度(" & i & ") : ", iOutSize, "Byte"
		Print "压缩比(" & i & ") : ", iOutSize / iSize * 100, "%"
	EndIf
	Print "压缩耗时 : ", Format(Timer() - ST, "0.0000")
	
	DeAllocate(pBuff)
	Print
	
Next

For i As Integer = 1 To 7 Step 5
	Dim sFile As ZString * 260
	
	sFile = "C:\test\1.jpg"
	
	Dim pBuff As Any Ptr
	Dim iSize As UInteger = xRtl.File.eRead(sFile, @pBuff)
	
	Print sFile & "  文件原始长度 : ", iSize, "Byte"
	
	Dim pOutBuff As Any Ptr = Allocate(iSize)
	Dim iOutSize As UInteger = iSize
	
	Dim ST As Double = Timer()
	If LzmaCompress(pOutBuff, @iOutSize, pBuff, iSize, pProps, @iProps, i) = SZ_OK Then
		Print "压缩后的长度(" & i & ") : ", iOutSize, "Byte"
		Print "压缩比(" & i & ") : ", iOutSize / iSize * 100, "%"
	EndIf
	Print "压缩耗时 : ", Format(Timer() - ST, "0.0000")
	
	DeAllocate(pBuff)
	Print
	
Next

For i As Integer = 1 To 7 Step 5
	Dim sFile As ZString * 260
	
	sFile = "C:\test\xge.dll"
	
	Dim pBuff As Any Ptr
	Dim iSize As UInteger = xRtl.File.eRead(sFile, @pBuff)
	
	Print sFile & "  文件原始长度 : ", iSize, "Byte"
	
	Dim pOutBuff As Any Ptr = Allocate(iSize)
	Dim iOutSize As UInteger = iSize
	
	Dim ST As Double = Timer()
	If LzmaCompress(pOutBuff, @iOutSize, pBuff, iSize, pProps, @iProps, i) = SZ_OK Then
		Print "压缩后的长度(" & i & ") : ", iOutSize, "Byte"
		Print "压缩比(" & i & ") : ", iOutSize / iSize * 100, "%"
	EndIf
	Print "压缩耗时 : ", Format(Timer() - ST, "0.0000")
	
	DeAllocate(pBuff)
	Print
	
Next

For i As Integer = 1 To 7 Step 5
	Dim sFile As ZString * 260
	
	sFile = "C:\test\万历十五年.txt"
	
	Dim pBuff As Any Ptr
	Dim iSize As UInteger = xRtl.File.eRead(sFile, @pBuff)
	
	Print sFile & "  文件原始长度 : ", iSize, "Byte"
	
	Dim pOutBuff As Any Ptr = Allocate(iSize)
	Dim iOutSize As UInteger = iSize
	
	Dim ST As Double = Timer()
	If LzmaCompress(pOutBuff, @iOutSize, pBuff, iSize, pProps, @iProps, i) = SZ_OK Then
		Print "压缩后的长度(" & i & ") : ", iOutSize, "Byte"
		Print "压缩比(" & i & ") : ", iOutSize / iSize * 100, "%"
	EndIf
	Print "压缩耗时 : ", Format(Timer() - ST, "0.0000")
	
	DeAllocate(pBuff)
	Print
	
Next






Sleep
