#Include Once "Windows.bi"
#LibPath "Lib"



#Include "Inc\Lib\File.bi"
#Include "Inc\Lib\xBsmm.bi"
#Include "Inc\Lib\lzma.bi"
#Include "Inc\Lib\lz4.bi"
#Include "Inc\Lib\CityHash.bi"



#Include "Inc\xPack.bi"
#Include "Inc\Core.bi"



Function OnError(iErrCode As Integer, sErrText As ZString Ptr) As Integer
	Print iErrCode, *sErrText
End Function

Dim xpk As xPack

xpk.OnError = @OnError

xpk.Open(ExePath() & "\1.xpk")
xpk.AppendData(1, @"1234567890", 10, 0)
xpk.AppendData(2, @"1234567890", 10, 0)
xpk.AppendData(3, @"1234567890", 10, 0)
xpk.AppendData(4, @"1234567890", 10, 0)
xpk.Save(0)
xpk.Close()

Sleep
