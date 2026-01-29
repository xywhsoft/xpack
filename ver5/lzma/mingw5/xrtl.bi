'==================================================================================
	'★ xywh Runtime Library 头文件
	'#-----------------------------------------------------------------------------
	'# 功能 : 
	'# 说明 : 
'==================================================================================



#Ifndef xywh_library_xrtl
	#Define xywh_library_xrtl
	#Ifndef XRTL_SOURCE_NOLIB
		#Define XRTL_SOURCE_NOLIB
		#Inclib "xrtl"
	#EndIf
	
	
	
	/' -------------------------- 基础头文件 -------------------------- '/
	#Include Once "Windows.bi"
	#Include Once "Win\commdlg.bi"
	#Include Once "Win\shlobj.bi"
	#Include Once "Crt\string.bi"
	
	
	
	' 临时内存最大保留数量
	#Define XRL_RUNTIME_MEMORY_COUNT		&H20
	
	
	' 查找文件标记 [查找所有文件]
	#Define AllFile FILE_ATTRIBUTE_ARCHIVE Or FILE_ATTRIBUTE_COMPRESSED Or FILE_ATTRIBUTE_DIRECTORY Or FILE_ATTRIBUTE_HIDDEN Or FILE_ATTRIBUTE_NORMAL Or FILE_ATTRIBUTE_READONLY Or FILE_ATTRIBUTE_SYSTEM Or FILE_ATTRIBUTE_TEMPORARY
	
	
	' 查找文件附加规则
	Enum Find_Rule
		NoAttribEx = 0		' 不限制
		FloderOnly = 1		' 只查找目录
		PointFloder = 2		' 去除根目录及父级目录
	End Enum
	
	
	
	Namespace xRtl
		Dim Shared xywh_library_xrtl_memory_point(1 To XRL_RUNTIME_MEMORY_COUNT) As Any Ptr		' 内存指针数组
		Dim Shared xywh_library_xrtl_memory_istep As Integer									' 轮转申请位置
		Dim Shared xywh_library_xrtl_memory_snull As ZString * 4 = !"\0\0\0"					' 空字符串
		
		Declare Function Shell(Path As ZString Ptr, WinShow As Integer = SW_SHOW) As HANDLE
		Declare Function Run(Path As ZString Ptr, WinShow As Integer = SW_SHOW) As HANDLE
		Declare Function Chain(Path As ZString Ptr, WinShow As Integer = SW_SHOW) As Integer
		Declare Sub DoEvents()
		Declare Function TempMemory(size As UInteger) As Any Ptr
		Declare Sub FreeMemory()
		
		Namespace Ini
			Declare Function GetStr(IniFile As ZString Ptr, IniSec As ZString Ptr, IniKey As ZString Ptr) As ZString Ptr
			Declare Function GetInt(IniFile As ZString Ptr, IniSec As ZString Ptr, IniKey As ZString Ptr) As Integer
			Declare Function SetStr(IniFile As ZString Ptr, IniSec As ZString Ptr, IniKey As ZString Ptr, kValue As ZString Ptr) As Integer
			Declare Function EnumSec(IniFile As ZString Ptr, OutArr As ZString Ptr Ptr Ptr) As Integer
			Declare Function EnumKey(IniFile As ZString Ptr, IniSec As ZString Ptr, OutArr As ZString Ptr Ptr Ptr) As Integer
			
		End Namespace
		
		Namespace Res
			Declare Function ToFile(ResID As Integer, FileName As ZString Ptr, hMod As HMODULE = NULL, rTpe As ZString Ptr = RT_RCDATA) As Integer
			Declare Function Load(ResID As Integer, OutMem As Any Ptr Ptr, hMod As HMODULE = NULL, rTpe As ZString Ptr = RT_RCDATA) As UInteger
			Declare Function LoadText(ResID As Integer, hMod As HMODULE = NULL) As ZString Ptr
			
		End Namespace
		
		Namespace File
			Dim Shared xywh_library_xrtl_file_ecode As Integer
			
			Declare Function Create(FilePath As ZString Ptr) As Integer
			Declare Function Open(FilePath As ZString Ptr, OnlyRead As Integer = 0) As HANDLE
			Declare Function Close(FileHdr As HANDLE) As Integer
			Declare Function Exists(FilePath As ZString Ptr) As Integer
			Declare Function hWrite(FileHdr As HANDLE, Buffer As Any Ptr, Addr As UInteger, Length As UInteger) As UInteger
			Declare Function Write(FilePath As ZString Ptr, Buffer As Any Ptr, Addr As UInteger, Length As UInteger) As UInteger
			Declare Function hRead(FileHdr As HANDLE, Buffer As Any Ptr, Addr As UInteger, Length As UInteger) As UInteger
			Declare Function Read(FilePath As ZString Ptr, Buffer As Any Ptr, Addr As UInteger, Length As UInteger) As UInteger
			Declare Function eRead(FilePath As ZString Ptr, Buffer As Any Ptr Ptr) As UInteger
			Declare Function hSize(FileHdr As HANDLE) As UInteger
			Declare Function Size(FilePath As ZString Ptr) As UInteger
			Declare Function hCut(FileHdr As HANDLE, FileSize As UInteger) As Integer
			Declare Function Cut(FilePath As ZString Ptr, FileSize As UInteger) As Integer
			Declare Function Scan(RootDir As ZString Ptr, Filter As ZString Ptr, Attrib As Integer, AttribEx As Find_Rule, Recursive As Integer, CallBack As Function(Path As ZString Ptr, FindData As WIN32_FIND_DATA Ptr, param As Integer = 0) As Integer, param As Integer) As Integer
			
		End Namespace
		
		Namespace Dlg
			Dim Shared xywh_library_xrtl_diadlg_fopen As ZString * 12 = !"打开文件\0\0\0"	' 打开文件对话框标题
			Dim Shared xywh_library_xrtl_diadlg_fsave As ZString * 12 = !"保存文件\0\0\0"	' 保存文件对话框标题
			Dim Shared xywh_library_xrtl_diadlg_dopen As ZString * 12 = !"选择目录\0\0\0"	' 打开文件夹对话框标题
			Dim Shared xywh_library_xrtl_diadlg_color(15) As UInteger						' 自定义颜色表
			
			Declare Function OpenFile(Parent As HWND, OutPath As ZString Ptr, iBufSize As UInteger = MAX_PATH, Filter As ZString Ptr, Title As ZString Ptr = @xRtl.Dlg.xywh_library_xrtl_diadlg_fopen, iFlag As Integer = 0) As Integer
			Declare Function SaveFile(Parent As HWND, OutPath As ZString Ptr, iBufSize As UInteger = MAX_PATH, Filter As ZString Ptr, Title As ZString Ptr = @xRtl.Dlg.xywh_library_xrtl_diadlg_fsave, iFlag As Integer = 0) As Integer
			Declare Function OpenFolder(Parent As HWND, OutPath As ZString Ptr, Title As ZString Ptr = @xRtl.Dlg.xywh_library_xrtl_diadlg_dopen) As Integer
			Declare Function SelFont(Parent As HWND, pFont As LOGFONT Ptr) As Integer
			Declare Function SelColor(Parent As HWND, pColor As UInteger Ptr) As Integer
			
		End Namespace
		
		Namespace Charset
			Declare Function A2W(ZStrPtr As ZString Ptr, ZStrLen As UInteger = 0) As Any Ptr
			Declare Function W2A(WStrPtr As WString Ptr, WStrLen As UInteger = 0) As Any Ptr
			Declare Function W2U(WStrPtr As WString Ptr, WStrLen As UInteger = 0) As Any Ptr
			Declare Function U2W(UTF8Ptr As ZString Ptr,UTF8Len As UInteger = 0) As Any Ptr
			Declare Function A2U(ZStr As ZString Ptr, ZLen As UInteger = 0) As ZString Ptr
			Declare Function U2A(UStr As ZString Ptr,ULen As UInteger = 0) As ZString Ptr
			Declare Function A2W_C(ZStrPtr As ZString Ptr, ZStrLen As UInteger = 0) As Any Ptr
			Declare Function W2A_C(WStrPtr As WString Ptr, WStrLen As UInteger = 0) As Any Ptr
			Declare Function W2U_C(WStrPtr As WString Ptr, WStrLen As UInteger = 0) As Any Ptr
			Declare Function U2W_C(UTF8Ptr As ZString Ptr,UTF8Len As UInteger = 0) As Any Ptr
			Declare Function A2U_C(ZStr As ZString Ptr, ZLen As UInteger = 0) As ZString Ptr
			Declare Function U2A_C(UStr As ZString Ptr,ULen As UInteger = 0) As ZString Ptr
			
		End Namespace
		
		Namespace Clip
			Declare Function GetText() As ZString Ptr
			Declare Function SetText(Text As ZString Ptr, Size As UInteger = 0) As Integer
			
		End Namespace
		
	End Namespace
#EndIf
