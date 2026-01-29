


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <direct.h>
#include <windows.h>
#ifdef __TINYC__
	#include <winapi/winuser.h>
	#include <winapi/winnetwk.h>
	#include <winapi/shlobj.h>
	#include <winapi/shlwapi.h>
	#include <winapi/shellapi.h>
	#include <winapi/commdlg.h>
#else
	#include <winuser.h>
	#include <winnetwk.h>
	#include <shlobj.h>
	#include <shlwapi.h>
	#include <shellapi.h>
	#include <commdlg.h>
#endif



#ifndef XXRTL_CORE
	#define XXRTL_CORE
	
	
	
	// charset define
	#define XCORE_CHARSET_ANSI		0
	#define XCORE_CHARSET_UNICODE	1
	#define XCORE_CHARSET_UTF8		2
	
	
	
	// basic type defint
	typedef char* astr;
	typedef wchar_t* wstr;
	typedef char* ustr;
	#ifdef SETUP_CHARSET_USEANSI
		#define SETUP_CHARSET_DEFINE
		typedef astr str;
	#endif
	#ifdef SETUP_CHARSET_USEUNICODE
		#define SETUP_CHARSET_DEFINE
		typedef wstr str;
	#endif
	#ifdef SETUP_CHARSET_USEUTF8
		#define SETUP_CHARSET_DEFINE
		typedef ustr str;
	#endif
	#ifndef SETUP_CHARSET_DEFINE
		typedef astr str;
	#endif
	
	typedef char int8;
	typedef unsigned char uint8;
	typedef short int16;
	typedef unsigned short uint16;
	typedef unsigned int uint;
	typedef int int32;
	typedef unsigned int uint32;
	typedef long long int64;
	typedef unsigned long long uint64;
	// long = auto 32 / 64 bit integer
	typedef unsigned long ulong;
	
	typedef void* ptr;
	typedef LONG_PTR intptr;
	
	/*
	#ifndef bool
		typedef int bool;
	#endif
	#ifndef true
		#define true -1
	#endif
	#ifndef false
		#define false 0
	#endif
	*/
	#ifndef TRUE
		#define TRUE 1
	#endif
	#ifndef FALSE
		#define FALSE 0
	#endif
	#ifndef null
		#define null 0
	#endif

	#ifdef BUILD_DLL
		#define XXAPI	__declspec(dllexport)
	#else
		#define XXAPI
	#endif
	
	
	
	#define XCORE_WEEK_FIRST_SYSTEM            0
	#define XCORE_WEEK_FIRST_JAN_1             1
	#define XCORE_WEEK_FIRST_FOUR_DAYS         2
	#define XCORE_WEEK_FIRST_FULL_WEEK         3
	#define XCORE_WEEK_FIRST_DEFAULT           XCORE_WEEK_FIRST_JAN_1

	#define XCORE_WEEK_DAY_SYSTEM              0
	#define XCORE_WEEK_DAY_SUNDAY              1
	#define XCORE_WEEK_DAY_MONDAY              2
	#define XCORE_WEEK_DAY_TUESDAY             3
	#define XCORE_WEEK_DAY_WEDNESDAY           4
	#define XCORE_WEEK_DAY_THURSDAY            5
	#define XCORE_WEEK_DAY_FRIDAY              6
	#define XCORE_WEEK_DAY_SATURDAY            7
	#define XCORE_WEEK_DAY_DEFAULT             XCORE_WEEK_DAY_SUNDAY

	#define XCORE_TIME_INTERVAL_INVALID        0
	#define XCORE_TIME_INTERVAL_YEAR           1
	#define XCORE_TIME_INTERVAL_QUARTER        2
	#define XCORE_TIME_INTERVAL_MONTH          3
	#define XCORE_TIME_INTERVAL_DAY_OF_YEAR    4
	#define XCORE_TIME_INTERVAL_DAY            5
	#define XCORE_TIME_INTERVAL_WEEKDAY        6
	#define XCORE_TIME_INTERVAL_WEEK_OF_YEAR   7
	#define XCORE_TIME_INTERVAL_HOUR           8
	#define XCORE_TIME_INTERVAL_MINUTE         9
	#define XCORE_TIME_INTERVAL_SECOND         10
	
	// 计算是否为闰年 (闰年返回1，否则返回0)
	XXAPI int xCore_TimeLeap(int y);
	
	// 计算月份天数
	XXAPI int xCore_DaysInMonth(int y, int m);
	
	// 计算某年的天数
	#define xCore_DaysInYear(y) 		(365 + xCore_TimeLeap(y))
	
	// 获取当前时间
	XXAPI char* xCore_TimeS();
	XXAPI char* xCore_DateS();
	
	// 获取时间戳
	XXAPI double xCore_Timer();
	
	// 设置系统时间和日期
	XXAPI int xCore_SetDateTime(int y, int m, int d, int h, int n, int s);
	XXAPI int xCore_SetDate(int y, int m, int d);
	XXAPI int xCore_SetTime(int h, int m, int s);
	XXAPI int xCore_SetDateSA(char* sDate);
	XXAPI int xCore_SetTimeSA(char* sTime);
	
	// 构造时间和日期
	XXAPI double xCore_TimeSerial(int h, int m, int s);
	XXAPI int xCore_DateSerial(int y, int m, int d);
	XXAPI double xCore_Now();
	XXAPI int xCore_Date();
	XXAPI double xCore_Time();
	
	// 获取时间的一部分
	XXAPI int xCore_Year(double serial);
	XXAPI int xCore_Month(double serial);
	XXAPI int xCore_Day(double serial);
	XXAPI int xCore_Weekday(double serial, int first_day_of_week);
	XXAPI int xCore_Hour(double serial);
	XXAPI int xCore_Minute(double serial);
	XXAPI int xCore_Second(double serial);
	XXAPI void xCore_hDateDecodeSerial( double serial, int *pYear, int *pMonth, int *pDay );
	XXAPI void xCore_hTimeDecodeSerial( double serial, int *pHour, int *pMinute, int *pSecond, int use_qb_hack );
	
	XXAPI int    xCore_hTimeGetIntervalType(char* interval);
	XXAPI double xCore_DateAdd( char* interval, double interval_value_arg, double serial );
	XXAPI int64  xCore_DateDiff( char* interval, double serial1, double serial2, int first_day_of_week, int first_day_of_year );
	XXAPI int    xCore_DatePart( char* interval, double serial, int first_day_of_week, int first_day_of_year );
	
	XXAPI int    xCore_hDateParse( const char *text, size_t text_len, int *pDay, int *pMonth, int *pYear, size_t *pLength );
	XXAPI int    xCore_hTimeParse( const char *text, size_t text_len, int *pHour, int *pMinute, int *pSecond, size_t *pLength );
	XXAPI int    xCore_DateParse( char* s, int *pDay, int *pMonth, int *pYear );
	XXAPI int    xCore_TimeParse( char* s, int *pHour, int *pMinute, int *pSecond );
	XXAPI int    xCore_DateTimeParse( char* s, int *pDay, int *pMonth, int *pYear, int *pHour, int *pMinute, int *pSecond, int want_date, int want_time );
	
	XXAPI void   xCore_hNormalizeDate   ( int *pDay, int *pMonth, int *pYear );
	XXAPI int    xCore_hGetDayOfYearEx  ( int year, int month, int day );
	XXAPI int    xCore_hGetDayOfYear    ( double serial );
	XXAPI int    xCore_hGetWeekOfYear   ( int ref_year, double serial, int first_day_of_year, int first_day_of_week );
	XXAPI int    xCore_hGetWeeksOfYear  ( int ref_year, int first_day_of_year, int first_day_of_week );
	
	XXAPI const char*	xCore_IntlGetMonthName ( int month, int short_name );
	XXAPI const char* xCore_IntlGetWeekdayName( int weekday, int short_names );
	
	
	
	// 释放内存（ 释放 malloc 申请的内存，会先判断是否为 nullstring ）
	XXAPI void xCore_free(ptr pmem);
	
	// 获取模块的文件路径（需要使用 xCore_free 释放内存）
	XXAPI wstr xCore_ModFileW(HANDLE hInst);
	XXAPI astr xCore_ModFileA(HANDLE hInst);
	XXAPI ustr xCore_ModFileU(HANDLE hInst);
	
	// 获取程序文件路径（需要使用 xCore_free 释放内存）
	XXAPI wstr xCore_AppFileW();
	XXAPI astr xCore_AppFileA();
	XXAPI ustr xCore_AppFileU();
	
	// 获取模块所在文件夹的路径，总是以 \ 结尾（需要使用 xCore_free 释放内存）
	XXAPI wstr xCore_ModPathW(HANDLE hInst);
	XXAPI astr xCore_ModPathA(HANDLE hInst);
	XXAPI ustr xCore_ModPathU(HANDLE hInst);
	
	// 获取程序所在文件夹的路径，总是以 \ 结尾（需要使用 xCore_free 释放内存）
	XXAPI wstr xCore_AppPathW();
	XXAPI astr xCore_AppPathA();
	XXAPI ustr xCore_AppPathU();
	
	// 运行程序
	XXAPI HANDLE xCore_RunW(wstr sPath, int iShow);
	XXAPI HANDLE xCore_RunA(astr sPath, int iShow);
	XXAPI HANDLE xCore_RunU(ustr sPath, int iShow);
	
	// 打开文件
	XXAPI HANDLE xCore_StartW(wstr sPath, int iShow);
	XXAPI HANDLE xCore_StartA(astr sPath, int iShow);
	XXAPI HANDLE xCore_StartU(ustr sPath, int iShow);
	
	// 运行程序并等待程序运行结束
	XXAPI int xCore_ChainW(wstr sPath, int iShow);
	XXAPI int xCore_ChainA(astr sPath, int iShow);
	XXAPI int xCore_ChainU(ustr sPath, int iShow);
	
	// 运行程序，返回控制台输出
	XXAPI astr xShellA(astr sCMD);
	XXAPI ustr xShellU(ustr sCMD);
	XXAPI wstr xShellW(wstr sCMD);
	
	// 获取系统目录
	XXAPI wstr xCore_SystemPathW(int csidl);
	XXAPI astr xCore_SystemPathA(int csidl);
	XXAPI ustr xCore_SystemPathU(int csidl);
	
	// 随机数
	XXAPI int xCore_Rand(int min, int max);
	
	// 获取系统位数
	XXAPI int xCore_SysBits();
	
	
	
	// 通过路径获取文件名 + 扩展名
	XXAPI wstr Path_FileNameExtW(wstr sPath);
	XXAPI astr Path_FileNameExtA(astr sPath);
	
	// 通过路径获取文件名
	XXAPI wstr Path_FileNameW(wstr sPath);
	XXAPI astr Path_FileNameA(astr sPath);
	
	// 通过路径获取扩展名
	XXAPI wstr Path_FileExtW(wstr sPath);
	XXAPI astr Path_FileExtA(astr sPath);
	
	// 通过路径获取文件夹(以 \ 结尾)
	XXAPI wstr Path_FilePathW(wstr sPath);
	XXAPI astr Path_FilePathA(astr sPath);
	
	// 判断是否为绝对路径（Linux 系统以 / 开头为绝对路径，Windows系统含 : 为绝对路径）
	XXAPI int Path_IsAbsA(astr sPath);
	XXAPI int Path_IsAbsW(wstr sPath);
	
	// 判断是否为相对路径
	XXAPI int Path_IsRelA(astr sPath);
	XXAPI int Path_IsRelW(wstr sPath);
	
	// 获取绝对路径
	XXAPI wstr Path_GetAbsW(wstr sPath, wstr sCurPath);
	XXAPI astr Path_GetAbsA(astr sPath, astr sCurPath);
	
	// 获取相对路径
	XXAPI wstr Path_GetRelW(wstr sPath, wstr sCurPath);
	XXAPI astr Path_GetRelA(astr sPath, astr sCurPath);
	
	// 获取随机不存在的路径
	XXAPI wstr Path_RandomW(wstr sHead, wstr sFoot, int iRand);
	XXAPI astr Path_RandomA(astr sHead, astr sFoot, int iRand);
	
	// 文件路径合法性检查（返回 TRUE 代表路径有问题）
	XXAPI int Path_SafeCheckW(wstr sPath, int bRepair);
	XXAPI int Path_SafeCheckA(astr sPath, int bRepair);
	
	// 根据 文件夹 + 文件 合并完整的路径，自动补充文件夹末尾的斜杠（需要使用 xCore_free 释放内存）
	XXAPI wstr Path_JoinW(wstr sPath, wstr sFile);
	XXAPI astr Path_JoinA(astr sPath, astr sFile);
	
	// 判断两个路径是否可以形成相对路径（不区分大小写，自动适配 / 和 \ 符号）
	XXAPI int Path_RelLikeW(wstr sPath, wstr sFile);
	XXAPI int Path_RelLikeA(astr sPath, astr sFile);
	
	
	
	// 多字节 转 Unicode
	XXAPI wstr xCore_M2W(const str pStr, uint32 iCodePage, ulong iSize);
	
	// Unicode 转 多字节
	XXAPI str xCore_W2M(const wstr pStr, uint32 iCodePage, ulong iSize);
	
	// ANSI 转 Unicode
	XXAPI wstr xCore_A2W(const astr pZStr, ulong iSize);
	
	// utf-8 转 Unicode
	XXAPI wstr xCore_U2W(const ustr pUStr, ulong iSize);
	
	// Unicode 转 ANSI
	XXAPI astr xCore_W2A(const wstr pWStr, ulong iSize);
	
	// Unicode 转 utf-8
	XXAPI ustr xCore_W2U(const wstr pWStr, ulong iSize);
	
	// ANSI 转 utf-8
	XXAPI ustr xCore_A2U(const astr pZStr, ulong iSize);
	
	// utf-8 转 ANSI
	XXAPI astr xCore_U2A(const ustr pUStr, ulong iSize);
	
	
	
	// 字符串格式化（需使用 xCore.free 释放）
	XXAPI astr xCore_FormatA(astr sFormat, ...);
	XXAPI wstr xCore_FormatW(wstr sFormat, ...);
	
	// 字符串分割（需使用 xCore.free 释放）
	XXAPI astr* xCore_SplitCharA(astr sText, char iChar, int bSrcRevise);
	XXAPI astr* xCore_SplitTextA(astr sText, astr sSep, int bSrcRevise);
	XXAPI wstr* xCore_SplitCharW(wstr sText, wchar_t iChar, int bSrcRevise);
	XXAPI wstr* xCore_SplitTextW(wstr sText, wstr sSep, int bSrcRevise);
	XXAPI astr* xCore_SplitA(astr sText, astr sSep, int bSrcRevise);
	XXAPI wstr* xCore_SplitW(wstr sText, wstr sSep, int bSrcRevise);
	
	// 字符串替换（需使用 xCore.free 释放）
	XXAPI astr xCore_ReplaceA(astr original, astr pattern, astr replacement);
	XXAPI wstr xCore_ReplaceW(wstr original, wstr pattern, wstr replacement);
	
	// 创建字符串副本（需使用 xCore.free 释放）
	XXAPI wstr xCore_CopyStringW(wstr sText, int iSize);
	XXAPI astr xCore_CopyStringA(astr sText, int iSize);
	
	// 搜索字符串（没找到字符串的情况下会返回 NULL）
	XXAPI wstr xCore_FindStrW(wstr sText, wstr sSubText, int bCase);
	XXAPI uint xCore_InStrW(wstr sText, wstr sSubText, int bCase);
	XXAPI astr xCore_FindStrA(astr sText, astr sSubText, int bCase);
	XXAPI uint xCore_InStrA(astr sText, astr sSubText, int bCase);
	
	// 字符串转为小写（bSrcRevise 为 false 时，需使用 xCore.free 释放内存）
	XXAPI wstr xCore_LCaseW(wstr sText, int iSize, int bSrcRevise);
	XXAPI astr xCore_LCaseA(astr sText, int iSize, int bSrcRevise);
	
	// 字符串转为大写（bSrcRevise 为 false 时，需使用 xCore.free 释放内存）
	XXAPI wstr xCore_UCaseW(wstr sText, int iSize, int bSrcRevise);
	XXAPI astr xCore_UCaseA(astr sText, int iSize, int bSrcRevise);
	
	// 裁剪字符串（bSrcRevise 为 false 时，需使用 xCore.free 释放内存）
	XXAPI wstr xxLTrimW(wstr sText, wstr sSub, int bSrcRevise);
	XXAPI wstr xxRTrimW(wstr sText, wstr sSub, int bSrcRevise);
	XXAPI wstr xxTrimW(wstr sText, wstr sSub, int bSrcRevise);
	// 因为编码转换的缘故，bSrcRevise 参数不起作用，仅为了保证与 UNICODE 函数的兼容
	XXAPI astr xxLTrimA(astr sText, astr sSub, int bSrcRevise);
	XXAPI astr xxRTrimA(astr sText, astr sSub, int bSrcRevise);
	XXAPI astr xxTrimA(astr sText, astr sSub, int bSrcRevise);
	XXAPI ustr xxLTrimU(ustr sText, ustr sSub, int bSrcRevise);
	XXAPI ustr xxRTrimU(ustr sText, ustr sSub, int bSrcRevise);
	XXAPI ustr xxTrimU(ustr sText, ustr sSub, int bSrcRevise);
	
	// 过滤字符串（bSrcRevise 为 false 时，需使用 xCore.free 释放内存）
	XXAPI wstr xxStringFilterW(wstr sText, wstr sFilter, int bSrcRevise);
	// 因为编码转换的缘故，bSrcRevise 参数不起作用，仅为了保证与 UNICODE 函数的兼容
	XXAPI astr xxStringFilterA(astr sText, astr sFilter, int bSrcRevise);
	XXAPI ustr xxStringFilterU(ustr sText, ustr sFilter, int bSrcRevise);
	
	// HEX 编码
	XXAPI char* HexEncode(char* sMem, uint32 iSize);
	
	// HEX 解码
	XXAPI char* HexDecode(char* sMem, uint32 iSize);
	
	// URI 编码
	XXAPI char* UriEncode(char* sURL);
	
	// URI 解码
	XXAPI char* UriDecode(char* sURL);
	
	
	
	// 逻辑转字符串
	astr BoolToString(int bVal, int iTpe);
	
	// 字符串转逻辑
	int StringToBool(astr sVal);
	
	
	
	// 初始化 xCore
	XXAPI void xCoreInit(void);
	
	// 释放 xCore
	XXAPI void xCoreUnit(void);
	
	
	
	typedef struct {
		
		// 全局数据 (不可改变)
		uint32 version;
		str nullstring;
		ptr objGC;
		int SysBits;
		
		// 临时性全局数据 (可以改变，用于多个返回值的情况做临时存储)
		str sRet;
		ulong iRetSize;
		int iRet;
		int64 iRet64;
		double dRet;
		int LastErrorID;
		str LastError;
		
		// 应用信息
		HMODULE AppHandle;
		HWND AppWindow;
		str AppFile;
		str AppPath;
		str BinPath;
		
	} xCoreStruct, *xCoreObject;
	
	
	#ifdef XXRTL_CORE_IMPORTDLL
		__declspec(dllimport) extern xCoreStruct xCore;
	#else
		XXAPI extern xCoreStruct xCore;
	#endif
	
	
	
#endif


