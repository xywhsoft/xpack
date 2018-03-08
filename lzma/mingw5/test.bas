
#Include "Windows.bi"
#Include "lzma.bi"
#Include "xrtl.bi"



Dim pBuff As Any Ptr
Dim iSize As UInteger = xRtl.File.eRead(ExePath() & "\liblzma.a", @pBuff)

Print "读取文件长度 : ", iSize, "Byte"

Dim pOutBuff As Any Ptr = Allocate(iSize * 2)
Dim iOutSize As UInteger
Dim pProps As Any Ptr = Allocate(6)
Dim iProps As UInteger = 5

Print LzmaCompress(pOutBuff, @iOutSize, pBuff, iSize, pProps, @iProps, 5, 1 Shl 24, 3, 0, 2, 32, 2)



Sleep
