#Include "file.bas"



#Inclib "lz4"



Extern "Windows-MS"
	Declare Function LZ4_compress_default(source As ZString Ptr, dest As ZString Ptr, sourceSize As Integer, maxDestSize As Integer) As Integer
	Declare Function LZ4_decompress_safe(source As ZString Ptr, dest As ZString Ptr, compressedSize As Integer, maxDecompressedSize As Integer) As Integer
	Declare Function LZ4_compressBound(inputSize As Integer) As Integer
End Extern



#Define FilePath	ExePath & "\111.xgi"
#Define OutFile		FilePath & ".lz4"



Dim slen As UInteger
Dim smem As Any Ptr
Dim dlen As Integer
Dim dmem As Any Ptr
Dim ST As Double
Dim size As Integer



'/' 压缩文件
slen = FileLen(FilePath)
smem = Allocate(slen)
GetFile(FilePath, smem, 0, slen)

dlen = LZ4_compressBound(slen)
dmem = Allocate(dlen)

dlen = LZ4_compress_default(smem, dmem, slen, dlen)

PutFile(OutFile, dmem, 0, dlen)
'/



/' 高压缩比压缩文件
slen = FileLen(FilePath)
smem = Allocate(slen)
GetFile(FilePath, smem, 0, slen)

dlen = LZ4_compressBound(slen)
dmem = Allocate(dlen)

ST = Timer
size = LZ4_compress_HC(smem, dmem, slen, dlen, 1)
Print 1, size, Timer - ST

ST = Timer
size = LZ4_compress_HC(smem, dmem, slen, dlen, 2)
Print 2, size, Timer - ST

ST = Timer
size = LZ4_compress_HC(smem, dmem, slen, dlen, 3)
Print 3, size, Timer - ST

ST = Timer
size = LZ4_compress_HC(smem, dmem, slen, dlen, 4)
Print 4, size, Timer - ST

ST = Timer
size = LZ4_compress_HC(smem, dmem, slen, dlen, 5)
Print 5, size, Timer - ST

ST = Timer
size = LZ4_compress_HC(smem, dmem, slen, dlen, 6)
Print 6, size, Timer - ST

ST = Timer
size = LZ4_compress_HC(smem, dmem, slen, dlen, 7)
Print 7, size, Timer - ST

ST = Timer
size = LZ4_compress_HC(smem, dmem, slen, dlen, 8)
Print 8, size, Timer - ST

ST = Timer
size = LZ4_compress_HC(smem, dmem, slen, dlen, 9)
Print 9, size, Timer - ST

ST = Timer
size = LZ4_compress_HC(smem, dmem, slen, dlen, 10)
Print 10, size, Timer - ST

ST = Timer
size = LZ4_compress_HC(smem, dmem, slen, dlen, 11)
Print 11, size, Timer - ST

ST = Timer
size = LZ4_compress_HC(smem, dmem, slen, dlen, 12)
Print 12, size, Timer - ST

ST = Timer
size = LZ4_compress_HC(smem, dmem, slen, dlen, 13)
Print 13, size, Timer - ST

ST = Timer
size = LZ4_compress_HC(smem, dmem, slen, dlen, 14)
Print 14, size, Timer - ST

ST = Timer
size = LZ4_compress_HC(smem, dmem, slen, dlen, 15)
Print 15, size, Timer - ST

ST = Timer
size = LZ4_compress_HC(smem, dmem, slen, dlen, 16)
Print 16, size, Timer - ST
'/



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
