
#Include "Windows.bi"
#Include "lzma.bi"
#Include "xrtl.bi"



Dim pBuff As Any Ptr
Dim iSize As UInteger = xRtl.File.eRead(ExePath() & "\liblzma.a", @pBuff)

Print "读取文件长度 : ", iSize, "Byte"

Dim pOutBuff As Any Ptr = Allocate(iSize)
Dim iOutSize As UInteger = iSize
Dim pProps As Any Ptr = Allocate(5)
Dim iProps As UInteger = 5

If LzmaCompress(pOutBuff, @iOutSize, pBuff, iSize, pProps, @iProps) = SZ_OK Then
	Print "压缩后的长度 : ", iOutSize, "Byte"
	xRtl.File.Write(ExePath() & "\liblzma.a.压缩后的", pOutBuff, 0, iOutSize)
EndIf



Sleep
