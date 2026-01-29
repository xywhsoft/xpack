


#include <stdlib.h>
#include <stdio.h>



#include "../xCore/xcore.h"
#include "../xFile/xFile.h"
#include "../smm/smm.h"



#include "xPack.h"



void OnError(int iErrCode, str sErrText)
{
	printf("%d\t%s\n", iErrCode, sErrText);
}

int main(int argc, char** argv)
{
	printf("开始执行\n\n\n");
	xCoreInit();
	
	
	
	/* Core 模式测试
	// 添加文件
	printf("Core模式添加文件测试：\n");
	xFile_DeleteA("1.xpk");
	xPackObject xpk = xPack_Open("1.xpk", 0, FALSE);
	printf("XPK对象：%d\n", xpk);
	printf("错误号：%d\n", xCore.LastErrorID);
	printf("错误描述：%s\n", xCore.LastError);
	xpk->OnError = OnError;
	printf("添加文件：%d\n", xPack_Core_AppendFile(xpk, "core_in/1.txt", XPK_COMP_NO));
	printf("添加文件：%d\n", xPack_Core_AppendFile(xpk, "core_in/2.txt", XPK_COMP_FAST));
	printf("添加文件：%d\n", xPack_Core_AppendFile(xpk, "core_in/1.txt", XPK_COMP_NO));
	printf("添加文件：%d\n", xPack_Core_AppendFile(xpk, "core_in/3.txt", XPK_COMP_HIGH));
	printf("添加文件：%d\n", xPack_Core_AppendFile(xpk, "core_in/1.txt", XPK_COMP_NO));
	printf("添加文件：%d\n", xPack_Core_AppendFile(xpk, "core_in/4.txt", XPK_COMP_FAST));
	printf("添加文件：%d\n", xPack_Core_AppendFile(xpk, "core_in/1.txt", XPK_COMP_NO));
	printf("添加文件：%d\n", xPack_Core_AppendFile(xpk, "core_in/5.txt", XPK_COMP_HIGH));
	printf("添加文件：%d\n", xPack_Core_AppendFile(xpk, "core_in/1.txt", XPK_COMP_NO));
	xPack_Close(xpk);
	printf("\n");
	
	// 解包文件
	printf("Core模式解包文件测试：\n");
	xpk = xPack_OpenA("1.xpk", 0, TRUE);
	printf("XPK对象：%d\n", xpk);
	printf("错误号：%d\n", xCore.LastErrorID);
	printf("错误描述：%s\n", xCore.LastError);
	xpk->OnError = OnError;
	uint iCount = xPack_FileCount(xpk);
	for ( int i = 1; i <= iCount; i++ ) {
		printf("解压文件%d：%s\n", i, xCore_FormatA("core_out/%d.txt", i));
		xPack_Core_UnpackFile(xpk, i, xCore_FormatA("core_out/%d.txt", i));
	}
	xPack_Close(xpk);
	printf("\n\n");
	//*/
	
	
	
	/* Index 模式测试
	// 添加文件
	printf("Index模式添加文件测试：\n");
	xFile_DeleteA("2.xpk");
	xPackObject xpk2 = xPack_Open("2.xpk", 0, FALSE);
	printf("XPK对象：%d\n", xpk2);
	printf("错误号：%d\n", xCore.LastErrorID);
	printf("错误描述：%s\n", xCore.LastError);
	xpk2->OnError = OnError;
	xPack_SetPackType(xpk2, XPK_CLASS_Index);
	printf("添加文件：%d\n", xPack_Index_AppendFile(xpk2, 11, "core_in/1.txt", XPK_COMP_NO)->FileIndex);
	printf("添加文件：%d\n", xPack_Index_AppendFile(xpk2, 12, "core_in/2.txt", XPK_COMP_FAST)->FileIndex);
	printf("添加文件：%d\n", xPack_Index_AppendFile(xpk2, 13, "core_in/1.txt", XPK_COMP_NO)->FileIndex);
	printf("添加文件：%d\n", xPack_Index_AppendFile(xpk2, 14, "core_in/3.txt", XPK_COMP_HIGH)->FileIndex);
	printf("添加文件：%d\n", xPack_Index_AppendFile(xpk2, 15, "core_in/1.txt", XPK_COMP_NO)->FileIndex);
	printf("添加文件：%d\n", xPack_Index_AppendFile(xpk2, 16, "core_in/4.txt", XPK_COMP_FAST)->FileIndex);
	printf("添加文件：%d\n", xPack_Index_AppendFile(xpk2, 17, "core_in/1.txt", XPK_COMP_NO)->FileIndex);
	printf("添加文件：%d\n", xPack_Index_AppendFile(xpk2, 18, "core_in/5.txt", XPK_COMP_HIGH)->FileIndex);
	printf("添加文件：%d\n", xPack_Index_AppendFile(xpk2, 19, "core_in/1.txt", XPK_COMP_NO)->FileIndex);
	xPack_Close(xpk2);
	printf("\n");
	
	// 解包文件
	printf("Index模式解包文件测试：\n");
	xpk2 = xPack_OpenA("2.xpk", 0, TRUE);
	printf("XPK对象：%d\n", xpk2);
	printf("错误号：%d\n", xCore.LastErrorID);
	printf("错误描述：%s\n", xCore.LastError);
	xpk2->OnError = OnError;
	for ( int i = 11; i <= 19; i++ ) {
		printf("解压文件%d：%s\n", i, xCore_FormatA("core_out/%d.txt", i));
		xPack_Index_UnpackFile(xpk2, i, xCore_FormatA("core_out/%d.txt", i));
	}
	xPack_Close(xpk2);
	printf("\n\n");
	//*/
	
	
	
	/* Win32 模式测试
	// 添加文件
	printf("Win32模式添加文件测试：\n");
	xFile_DeleteA("3.xpk");
	xPackObject xpk3 = xPack_Open("3.xpk", 0, FALSE);
	printf("XPK对象：%d\n", xpk3);
	printf("错误号：%d\n", xCore.LastErrorID);
	printf("错误描述：%s\n", xCore.LastError);
	xpk3->OnError = OnError;
	xPack_SetPackType(xpk3, XPK_CLASS_Win32);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "111.txt", "core_in/1.txt", XPK_COMP_NO)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "102.txt", "core_in/2.txt", XPK_COMP_FAST)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "121.txt", "core_in/1.txt", XPK_COMP_NO)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "103.txt", "core_in/3.txt", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "131.txt", "core_in/1.txt", XPK_COMP_NO)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "104.txt", "core_in/4.txt", XPK_COMP_FAST)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "141.txt", "core_in/1.txt", XPK_COMP_NO)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "105.txt", "core_in/5.txt", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "151.txt", "core_in/1.txt", XPK_COMP_NO)->FilePath);
	xPack_Close(xpk3);
	printf("\n");
	
	// 解包文件
	printf("Win32模式解包文件测试：\n");
	xpk3 = xPack_OpenA("3.xpk", 0, TRUE);
	printf("XPK对象：%d\n", xpk3);
	printf("错误号：%d\n", xCore.LastErrorID);
	printf("错误描述：%s\n", xCore.LastError);
	xpk3->OnError = OnError;
	for ( int i = 1; i <= 9; i++ ) {
		printf("解压文件%d：%s\n", i, xCore_FormatA("core_out/%d.txt", i));
		xPack_FileInfo_Win32* pInfo = xPack_GetFileInfo(xpk3, i);
		xPack_Win32_UnpackFile(xpk3, pInfo->FilePath, xCore_FormatA("core_out/%s", pInfo->FilePath));
	}
	xPack_Close(xpk3);
	printf("\n\n");
	//*/
	
	
	
	//* Win32 模式测试
	// 添加文件
	
	printf("Win32模式添加文件测试：\n");
	xFile_DeleteA("10000.xpk");
	xPackObject xpk3 = xPack_Open("10000.xpk", 0, FALSE);
	printf("XPK对象：%d\n", xpk3);
	printf("错误号：%d\n", xCore.LastErrorID);
	printf("错误描述：%s\n", xCore.LastError);
	xpk3->OnError = OnError;
	xPack_SetPackType(xpk3, XPK_CLASS_Win32);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "GeekBot.exe", "更新包/GeekBot.exe", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "Launcher_upd.exe", "更新包/Launcher_upd.exe", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "upd.exe", "更新包/upd.exe", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "Updatelog.txt", "更新包/Updatelog.txt", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "xPack_upd.dll", "更新包/xPack_upd.dll", XPK_COMP_HIGH)->FilePath);
	
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\build\\ThinBasic_Bundle_UI.exe", "更新包/bin/build/ThinBasic_Bundle_UI.exe", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Compilers\\thinBasic_Compilers.ini", "更新包/bin/Compilers/thinBasic_Compilers.ini", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\Oxygen.dll", "更新包/bin/Lib/Oxygen.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_ADO.dll", "更新包/bin/Lib/thinBasic_ADO.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\ThinBasic_ADODB.dll", "更新包/bin/Lib/ThinBasic_ADODB.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_AppConfig.dll", "更新包/bin/Lib/thinBasic_AppConfig.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_Biff.dll", "更新包/bin/Lib/thinBasic_Biff.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_CGI.dll", "更新包/bin/Lib/thinBasic_CGI.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_CJSon.dll", "更新包/bin/Lib/thinBasic_CJSon.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_COMM.dll", "更新包/bin/Lib/thinBasic_COMM.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_Console.dll", "更新包/bin/Lib/thinBasic_Console.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_Crypto.dll", "更新包/bin/Lib/thinBasic_Crypto.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_Dictionary.dll", "更新包/bin/Lib/thinBasic_Dictionary.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_DT.dll", "更新包/bin/Lib/thinBasic_DT.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_Eval.dll", "更新包/bin/Lib/thinBasic_Eval.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_Excel.dll", "更新包/bin/Lib/thinBasic_Excel.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_Exe.dll", "更新包/bin/Lib/thinBasic_Exe.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_File.dll", "更新包/bin/Lib/thinBasic_File.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_FileLine.dll", "更新包/bin/Lib/thinBasic_FileLine.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_FTP.dll", "更新包/bin/Lib/thinBasic_FTP.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_GDIp.dll", "更新包/bin/Lib/thinBasic_GDIp.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_I18N.dll", "更新包/bin/Lib/thinBasic_I18N.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_iComplex.dll", "更新包/bin/Lib/thinBasic_iComplex.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_Inet.dll", "更新包/bin/Lib/thinBasic_Inet.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_INI.dll", "更新包/bin/Lib/thinBasic_INI.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_LAN.dll", "更新包/bin/Lib/thinBasic_LAN.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_LL.dll", "更新包/bin/Lib/thinBasic_LL.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\ThinBasic_LzRtl.dll", "更新包/bin/Lib/ThinBasic_LzRtl.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_Math.dll", "更新包/bin/Lib/thinBasic_Math.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_MMF.dll", "更新包/bin/Lib/thinBasic_MMF.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_MSXML2.dll", "更新包/bin/Lib/thinBasic_MSXML2.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_OS.dll", "更新包/bin/Lib/thinBasic_OS.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_Oxygen.dll", "更新包/bin/Lib/thinBasic_Oxygen.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_PC.dll", "更新包/bin/Lib/thinBasic_PC.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_RAS.dll", "更新包/bin/Lib/thinBasic_RAS.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_Registry.dll", "更新包/bin/Lib/thinBasic_Registry.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_SAPI.dll", "更新包/bin/Lib/thinBasic_SAPI.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_SMTP.dll", "更新包/bin/Lib/thinBasic_SMTP.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_SQLite.dll", "更新包/bin/Lib/thinBasic_SQLite.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_STAT.dll", "更新包/bin/Lib/thinBasic_STAT.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_StringBuilder.dll", "更新包/bin/Lib/thinBasic_StringBuilder.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_TBASS.dll", "更新包/bin/Lib/thinBasic_TBASS.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_TBGL.dll", "更新包/bin/Lib/thinBasic_TBGL.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_TcpUdp.dll", "更新包/bin/Lib/thinBasic_TcpUdp.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_TImage.dll", "更新包/bin/Lib/thinBasic_TImage.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_Tokenizer.dll", "更新包/bin/Lib/thinBasic_Tokenizer.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_Trace.dll", "更新包/bin/Lib/thinBasic_Trace.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_UI.dll", "更新包/bin/Lib/thinBasic_UI.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_UIAdv.dll", "更新包/bin/Lib/thinBasic_UIAdv.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_VBRegExp.dll", "更新包/bin/Lib/thinBasic_VBRegExp.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_WinMM.dll", "更新包/bin/Lib/thinBasic_WinMM.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_WMI.dll", "更新包/bin/Lib/thinBasic_WMI.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_XPRINT.dll", "更新包/bin/Lib/thinBasic_XPRINT.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Lib\\thinBasic_ZLIB.dll", "更新包/bin/Lib/thinBasic_ZLIB.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\Build.exe", "更新包/bin/Build.exe", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\FileAssociation.exe", "更新包/bin/FileAssociation.exe", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\LogServer.exe", "更新包/bin/LogServer.exe", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\thinBasic.exe", "更新包/bin/thinBasic.exe", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\thinBasicc.exe", "更新包/bin/thinBasicc.exe", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\thinCore.dll", "更新包/bin/thinCore.dll", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "bin\\xLib.dll", "更新包/bin/xLib.dll", XPK_COMP_HIGH)->FilePath);
	
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "Help\\thinBasic.chm", "更新包/Help/thinBasic.chm", XPK_COMP_HIGH)->FilePath);
	
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "Modules\\Redis\\boot.tbs", "更新包/Modules/Redis/boot.tbs", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "Modules\\Redis\\command.json", "更新包/Modules/Redis/command.json", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "Modules\\Redis\\hiredis.dll", "更新包/Modules/Redis/hiredis.dll", XPK_COMP_HIGH)->FilePath);
	
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "setup\\Language\\en.json", "更新包/setup/Language/en.json", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "setup\\app_gui.ico", "更新包/setup/app_gui.ico", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "setup\\boot.tbs", "更新包/setup/boot.tbs", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "setup\\bootD.tbs", "更新包/setup/bootD.tbs", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "setup\\command.json", "更新包/setup/command.json", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "setup\\command_en.json", "更新包/setup/command_en.json", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "setup\\setup.json", "更新包/setup/setup.json", XPK_COMP_HIGH)->FilePath);
	
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "template\\app_gui.ico", "更新包/template/app_gui.ico", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "template\\JSON读取.tbs", "更新包/template/JSON读取.tbs", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "template\\Option.json", "更新包/template/Option.json", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "template\\ProjectMake.exe", "更新包/template/ProjectMake.exe", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "template\\Redis范例.tbs", "更新包/template/Redis范例.tbs", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "template\\template.lzp", "更新包/template/template.lzp", XPK_COMP_HIGH)->FilePath);
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "template\\大漠插件.tbs", "更新包/template/大漠插件.tbs", XPK_COMP_HIGH)->FilePath);
	
	printf("添加文件：%s\n", xPack_Win32_AppendFile(xpk3, "Tool\\VisualDesigner\\thinVisualDesigner.exe", "更新包/Tool/VisualDesigner/thinVisualDesigner.exe", XPK_COMP_HIGH)->FilePath);
	
	xPack_Close(xpk3);
	printf("\n");
	
	/*
	// 解包文件
	printf("Win32模式解包文件测试：\n");
	xPackObject xpk3 = xPack_Open("10000.xpk", 0, TRUE);
	printf("XPK对象：%d\n", xpk3);
	printf("错误号：%d\n", xCore.LastErrorID);
	printf("错误描述：%s\n", xCore.LastError);
	xpk3->OnError = OnError;
	for ( int i = 1; i <= 5; i++ ) {
		printf("解压文件%d：%s\n", i, xCore_FormatA("core_out/%d.txt", i));
		xPack_FileInfo_Win32* pInfo = xPack_GetFileInfo(xpk3, i);
		xPack_Win32_UnpackFile(xpk3, pInfo->FilePath, xCore_FormatA("core_out/%s", pInfo->FilePath));
	}
	xPack_Close(xpk3);
	printf("\n\n");
	//*/
	
	
	
	printf("运行结束\n");
}


