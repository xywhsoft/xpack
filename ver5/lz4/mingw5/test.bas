


#Include "Windows.bi"
#include "string.bi"
#Include "xrtl.bi"
#Include "lz4.bi"



#Define FilePath	ExePath & "\lz4.bi"
#Define OutFile		FilePath & ".lz4"



/'
Dim slen As UInteger
Dim smem As Any Ptr
Dim dlen As Integer
Dim dmem As Any Ptr
Dim ST As Double
Dim size As Integer
'/



/' 压缩文件
slen = FileLen(FilePath)
smem = Allocate(slen)
GetFile(FilePath, smem, 0, slen)

dlen = LZ4_compressBound(slen)
dmem = Allocate(dlen)

dlen = LZ4_compress_default(smem, dmem, slen, dlen)

PutFile(OutFile, dmem, 0, dlen)
'/



Do
	Dim sFile As ZString * 260
	
	sFile = "C:\test\1.bmp"
	
	Dim pBuff As Any Ptr
	Dim iSize As UInteger = xRtl.File.eRead(sFile, @pBuff)
	
	Print sFile & "  文件原始长度 : ", iSize, "Byte"
	
	Dim pOutBuff As Any Ptr = Allocate(iSize)
	Dim iOutSize As UInteger = iSize
	
	Dim ST As Double = Timer()
	
	iOutSize = LZ4_compress_default(pBuff, pOutBuff, iSize, iOutSize)
	
	If pOutBuff Then
		Print "压缩后的长度 : ", iOutSize, "Byte"
		Print "压缩比 : ", iOutSize / iSize * 100, "%"
	EndIf
	Print "压缩耗时 : ", Format(Timer() - ST, "0.0000")
	
	DeAllocate(pBuff)
	Print
Loop While FALSE

Do
	Dim sFile As ZString * 260
	
	sFile = "C:\test\1.jpg"
	
	Dim pBuff As Any Ptr
	Dim iSize As UInteger = xRtl.File.eRead(sFile, @pBuff)
	
	Print sFile & "  文件原始长度 : ", iSize, "Byte"
	
	Dim pOutBuff As Any Ptr = Allocate(iSize)
	Dim iOutSize As UInteger = iSize
	
	Dim ST As Double = Timer()
	
	iOutSize = LZ4_compress_default(pBuff, pOutBuff, iSize, iOutSize)
	
	If pOutBuff Then
		Print "压缩后的长度 : ", iOutSize, "Byte"
		Print "压缩比 : ", iOutSize / iSize * 100, "%"
	EndIf
	Print "压缩耗时 : ", Format(Timer() - ST, "0.0000")
	
	DeAllocate(pBuff)
	Print
Loop While FALSE

Do
	Dim sFile As ZString * 260
	
	sFile = "C:\test\xge.dll"
	
	Dim pBuff As Any Ptr
	Dim iSize As UInteger = xRtl.File.eRead(sFile, @pBuff)
	
	Print sFile & "  文件原始长度 : ", iSize, "Byte"
	
	Dim pOutBuff As Any Ptr = Allocate(iSize)
	Dim iOutSize As UInteger = iSize
	
	Dim ST As Double = Timer()
	
	iOutSize = LZ4_compress_default(pBuff, pOutBuff, iSize, iOutSize)
	
	If pOutBuff Then
		Print "压缩后的长度 : ", iOutSize, "Byte"
		Print "压缩比 : ", iOutSize / iSize * 100, "%"
	EndIf
	Print "压缩耗时 : ", Format(Timer() - ST, "0.0000")
	
	DeAllocate(pBuff)
	Print
Loop While FALSE

Do
	Dim sFile As ZString * 260
	
	sFile = "C:\test\万历十五年.txt"
	
	Dim pBuff As Any Ptr
	Dim iSize As UInteger = xRtl.File.eRead(sFile, @pBuff)
	
	Print sFile & "  文件原始长度 : ", iSize, "Byte"
	
	Dim pOutBuff As Any Ptr = Allocate(iSize)
	Dim iOutSize As UInteger = iSize
	
	Dim ST As Double = Timer()
	
	iOutSize = LZ4_compress_default(pBuff, pOutBuff, iSize, iOutSize)
	
	If pOutBuff Then
		Print "压缩后的长度 : ", iOutSize, "Byte"
		Print "压缩比 : ", iOutSize / iSize * 100, "%"
	EndIf
	Print "压缩耗时 : ", Format(Timer() - ST, "0.0000")
	
	DeAllocate(pBuff)
	Print
Loop While FALSE



Sleep

'/

/' 解压文件
slen = FileLen(OutFile)
smem = Allocate(slen)
GetFile(OutFile, smem, 0, slen)

dlen = 1228844
dmem = Allocate(dlen)

dlen = LZ4_decompress_safe(smem, dmem, slen, dlen)
PutFile(FilePath & "dlz", dmem, 0, dlen)
'/
