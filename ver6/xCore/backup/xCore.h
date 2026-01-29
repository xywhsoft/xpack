


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
#include <imm.h>



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
	
	
	
	#define AVL_RET_OK    TRUE			// (0)
	#define AVL_RET_ERROR FALSE			// (-1)
	
	typedef struct avl_node {
		struct avl_node* left;			/* 左侧子树的指针 */
		struct avl_node* right;			/* 右侧子树的指针 */
		int    height;					/* 此节点相对于根节点的高度 */
	} AVL_NODE, *AVL_TREE;
	
	typedef struct avl_node_extends {
		AVL_NODE avlNode;
		str Key;
		#if defined(__x86_64__) || defined(_WIN64)
			uint64 Hash;
		#else
			uint32 Hash;
		#endif
		ptr Val;
		/*
		int32 Type;
		union {
			int32 iVal32;
			int64 iVal64;
			uint32 uVal32;
			uint64 uVal64;
			double dVal;
			str sVal;
			ptr pVal;
		};
		*/
	} xTableNode, *pTableNode;
	typedef AVL_TREE* xTableObject;

	typedef int (*AVL_COMPARE)(AVL_NODE *pNode, void * pKey);
	/* avluint_treewalk 回调例程 */
	typedef int (*AVL_CALLBACK)(AVL_NODE *pNode, void * pArg);
	typedef int (*AVL_FREE)(AVL_NODE *pNode);
	typedef int (*TBL_PROC_SCAN)(pTableNode pNode);
	
	int avl_insert (AVL_TREE * pRoot, AVL_NODE * pNode, void * pKey, AVL_COMPARE cmpRtn);
	int avl_destroy(AVL_TREE root, AVL_FREE free_rtn);
	AVL_NODE* avl_delete (AVL_TREE * pRoot, void * pKey, AVL_COMPARE cmpRtn);
	AVL_NODE* avl_search (AVL_TREE root, void * pKey, AVL_COMPARE cmpRtn);
	AVL_NODE* avl_successor_get (AVL_TREE root, void * pKey, AVL_COMPARE cmpRtn);
	AVL_NODE* avl_predecessor_get (AVL_TREE root, void * pKey, AVL_COMPARE cmpRtn);
	AVL_NODE* avl_minimum_get (AVL_TREE root);
	AVL_NODE* avl_maximum_get (AVL_TREE root);
	int avl_tree_walk (AVL_TREE pRoot, AVL_CALLBACK preRtn, void * preArg,
						   AVL_CALLBACK inRtn, void * inArg, AVL_CALLBACK postRtn, void * postArg);
	int avl_insert_inform (AVL_TREE * pRoot, void * pNewNode, void * key,
							   void ** ppKeyHolder, AVL_COMPARE cmpRtn);
	void* avl_remove_insert (AVL_TREE * pRoot, void * pNewNode, void * key, AVL_COMPARE cmpRtn);
	
	
	
	// Hash
	XXAPI uint32 XXH32(const void* input, size_t length, uint32 seed);
	XXAPI uint64 XXH64(const void* input, size_t length, uint64 seed);
	
	
	
	// Table
	XXAPI xTableObject xTable_Create();
	XXAPI int xTable_Destroy(xTableObject tblRoot);
	XXAPI pTableNode xTable_InsertA(xTableObject tblRoot, astr sKey, ptr pVal);
	XXAPI pTableNode xTable_InsertW(xTableObject tblRoot, wstr sKey, ptr pVal);
	XXAPI pTableNode xTable_SearchA(xTableObject tblRoot, astr sKey);
	XXAPI pTableNode xTable_SearchW(xTableObject tblRoot, wstr sKey);
	XXAPI ptr xTable_GetValA(xTableObject tblRoot, astr sKey);
	XXAPI ptr xTable_GetValW(xTableObject tblRoot, wstr sKey);
	XXAPI pTableNode xTable_SetValA(xTableObject tblRoot, astr sKey, ptr pVal);
	XXAPI pTableNode xTable_SetValW(xTableObject tblRoot, wstr sKey, ptr pVal);
	XXAPI int xTable_RemoveA(xTableObject tblRoot, astr sKey);
	XXAPI int xTable_RemoveW(xTableObject tblRoot, wstr sKey);
	XXAPI int xTable_Scan(xTableObject tblRoot, TBL_PROC_SCAN procScan);
	
	
	
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
	int xCore_TimeLeap(int y);
	
	// 计算月份天数
	int xCore_DaysInMonth(int y, int m);
	
	// 计算某年的天数
	#define xCore_DaysInYear(y) 		(365 + xCore_TimeLeap(y))
	
	// 获取当前时间
	char* xCore_TimeS();
	char* xCore_DateS();
	
	// 获取时间戳
	double xCore_Timer();
	
	// 设置系统时间和日期
	int xCore_SetDateTime(int y, int m, int d, int h, int n, int s);
	int xCore_SetDate(int y, int m, int d);
	int xCore_SetTime(int h, int m, int s);
	int xCore_SetDateSA(char* sDate);
	int xCore_SetTimeSA(char* sTime);
	
	// 构造时间和日期
	double xCore_TimeSerial(int h, int m, int s);
	int xCore_DateSerial(int y, int m, int d);
	double xCore_Now();
	int xCore_Date();
	double xCore_Time();
	
	// 获取时间的一部分
	int xCore_Year(double serial);
	int xCore_Month(double serial);
	int xCore_Day(double serial);
	int xCore_Weekday(double serial, int first_day_of_week);
	int xCore_Hour(double serial);
	int xCore_Minute(double serial);
	int xCore_Second(double serial);
	void xCore_hDateDecodeSerial( double serial, int *pYear, int *pMonth, int *pDay );
	void xCore_hTimeDecodeSerial( double serial, int *pHour, int *pMinute, int *pSecond, int use_qb_hack );
	
	int    xCore_hTimeGetIntervalType(char* interval);
	double xCore_DateAdd( char* interval, double interval_value_arg, double serial );
	int64  xCore_DateDiff( char* interval, double serial1, double serial2, int first_day_of_week, int first_day_of_year );
	int    xCore_DatePart( char* interval, double serial, int first_day_of_week, int first_day_of_year );
	
	int    xCore_hDateParse( const char *text, size_t text_len, int *pDay, int *pMonth, int *pYear, size_t *pLength );
	int    xCore_hTimeParse( const char *text, size_t text_len, int *pHour, int *pMinute, int *pSecond, size_t *pLength );
	int    xCore_DateParse( char* s, int *pDay, int *pMonth, int *pYear );
	int    xCore_TimeParse( char* s, int *pHour, int *pMinute, int *pSecond );
	int    xCore_DateTimeParse( char* s, int *pDay, int *pMonth, int *pYear, int *pHour, int *pMinute, int *pSecond, int want_date, int want_time );
	
	void   xCore_hNormalizeDate   ( int *pDay, int *pMonth, int *pYear );
	int    xCore_hGetDayOfYearEx  ( int year, int month, int day );
	int    xCore_hGetDayOfYear    ( double serial );
	int    xCore_hGetWeekOfYear   ( int ref_year, double serial, int first_day_of_year, int first_day_of_week );
	int    xCore_hGetWeeksOfYear  ( int ref_year, int first_day_of_year, int first_day_of_week );
	
	const char*	xCore_IntlGetMonthName ( int month, int short_name );
	const char* xCore_IntlGetWeekdayName( int weekday, int short_names );
	
	
	
	enum {
	  TGC_MARK = 0x01,
	  TGC_ROOT = 0x02,
	  TGC_LEAF = 0x04
	};

	typedef struct {
	  void *ptr;
	  int flags;
	  size_t size, hash;
	  void (*dtor)(void*);
	} tgc_ptr_t;

	typedef struct {
	  void *bottom;
	  int paused;
	  uintptr_t minptr, maxptr;
	  tgc_ptr_t *items, *frees;
	  double loadfactor, sweepfactor;
	  size_t nitems, nslots, mitems, nfrees;
	} tgc_t;
	
	
	
	void tgc_start(tgc_t *gc, void *stk);
	void tgc_stop(tgc_t *gc);
	void tgc_pause(tgc_t *gc);
	void tgc_resume(tgc_t *gc);
	void tgc_run(tgc_t *gc);
	
	void *tgc_alloc(tgc_t *gc, size_t size);
	void *tgc_calloc(tgc_t *gc, size_t num, size_t size);
	void *tgc_realloc(tgc_t *gc, void *ptr, size_t size);
	void tgc_free(tgc_t *gc, void *ptr);
	
	void *tgc_alloc_opt(tgc_t *gc, size_t size, int flags, void(*dtor)(void*));
	void *tgc_calloc_opt(tgc_t *gc, size_t num, size_t size, int flags, void(*dtor)(void*));
	
	void tgc_set_dtor(tgc_t *gc, void *ptr, void(*dtor)(void*));
	void tgc_set_flags(tgc_t *gc, void *ptr, int flags);
	int tgc_get_flags(tgc_t *gc, void *ptr);
	void(*tgc_get_dtor(tgc_t *gc, void *ptr))(void*);
	size_t tgc_get_size(tgc_t *gc, void *ptr);
	
	
	
	// 获取程序文件路径（需要使用 xCore_free 释放内存）
	XXAPI wstr xCore_AppFileW();
	XXAPI astr xCore_AppFileA();
	XXAPI ustr xCore_AppFileU();
	
	// 获取程序所在文件夹的路径，总是以 \ 结尾（需要使用 xCore_free 释放内存）
	XXAPI wstr xCore_AppPathW();
	XXAPI astr xCore_AppPathA();
	XXAPI ustr xCore_AppPathU();
	
	// 获取模块的文件路径（需要使用 xCore_free 释放内存）
	XXAPI wstr xCore_ModFileW(HANDLE hInst);
	XXAPI astr xCore_ModFileA(HANDLE hInst);
	XXAPI ustr xCore_ModFileU(HANDLE hInst);
	
	// 获取模块所在文件夹的路径，总是以 \ 结尾（需要使用 xCore_free 释放内存）
	XXAPI wstr xCore_ModPathW(HANDLE hInst);
	XXAPI astr xCore_ModPathA(HANDLE hInst);
	XXAPI ustr xCore_ModPathU(HANDLE hInst);
	
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
	
	// 获取系统目录
	XXAPI wstr xCore_SystemPathW(int csidl);
	XXAPI astr xCore_SystemPathA(int csidl);
	XXAPI ustr xCore_SystemPathU(int csidl);
	
	// 随机数
	XXAPI int xCore_Rand(int min, int max);
	
	// 获取系统位数
	XXAPI int xCore_SysBits();
	
	// 禁用输入法
	XXAPI int IME_Disable(HWND hWindow);
	
	// 启用输入法
	XXAPI int IME_Enable(HWND hWindow);
	
	
	
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
	
	// 获取绝对路径
	XXAPI wstr Path_GetAbsW(wstr sPath, wstr sCurPath);
	XXAPI astr Path_GetAbsA(astr sPath, astr sCurPath);
	
	// 判断是否为绝对路径（Linux 系统以 / 开头为绝对路径，Windows系统含 : 为绝对路径）
	XXAPI int Path_IsAbsA(astr sPath);
	XXAPI int Path_IsAbsW(wstr sPath);
	
	// 判断是否为相对路径
	XXAPI int Path_IsRelA(astr sPath);
	XXAPI int Path_IsRelW(wstr sPath);
	
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
	
	
	
	// GC 申请内存
	XXAPI ptr xCore_GC_malloc(size_t size);
	
	// GC 申请类内存
	XXAPI ptr xCore_GC_calloc(size_t count, size_t size);
	
	// 重新申请 GC 内存
	XXAPI ptr xCore_GC_realloc(ptr pmem, size_t size);
	
	// 释放 GC 内存
	XXAPI void xCore_GC_free(ptr pmem);
	
	// 暂停 GC 自动执行
	XXAPI void xCore_GC_Pause(void);
	
	// 恢复 GC 自动执行
	XXAPI void xCore_GC_Resume(void);
	
	// 执行一次 GC
	XXAPI void xCore_GC_Run(void);
	
	// 添加一块内存到 GC
	XXAPI ptr xCore_GC_Add(ptr pmem, ulong size, int flags, void(*dtor)(ptr));
	
	// 从 GC 中移除一块内存
	XXAPI void xCore_GC_Remove(ptr pmem);
	
	// 获取 GC 栈顶
	XXAPI ptr xCore_GC_GetStackTop();
	
	// 设置栈顶
	XXAPI void xCore_GC_SetStackTop(ptr pStack);
	
	// 释放内存（ 释放 malloc 申请的内存，会先判断是否为 nullstring ）
	XXAPI void xCore_free(ptr pmem);
	
	// 开启GC
	XXAPI ptr xCore_GC_Start(ptr pStack);
	
	// 关闭GC
	XXAPI void xCore_GC_Stop();
	
	
	
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
	
	
	
	// 读文本
	XXAPI astr Ini_ReadA(const astr sFile, const astr sSec, const astr sKey);
	XXAPI wstr Ini_ReadW(const wstr sFile, const wstr sSec, const wstr sKey);
	XXAPI ustr Ini_ReadU(const ustr sFile, const ustr sSec, const ustr sKey);
	
	// 读数字
	XXAPI uint32 Ini_ReadIntA(const astr sFile, const astr sSec, const astr sKey);
	XXAPI uint32 Ini_ReadIntW(const wstr sFile, const wstr sSec, const wstr sKey);
	XXAPI uint32 Ini_ReadIntU(const ustr sFile, const ustr sSec, const ustr sKey);
	
	// 写文本
	XXAPI int Ini_WriteA(const astr sFile, const astr sSec, const astr sKey, const astr sVal);
	XXAPI int Ini_WriteW(const wstr sFile, const wstr sSec, const wstr sKey, const wstr sVal);
	XXAPI int Ini_WriteU(const ustr sFile, const ustr sSec, const ustr sKey, const ustr sVal);
	
	
	
	// 读取资源到内存
	ptr Res_ReadA(HINSTANCE hInst, int iResID, astr sTpe);
	
	// 读取资源文件中的字符串
	char* Res_ReadTextA(HINSTANCE hInst, int iResID);
	
	
	
	// 设置剪贴板内容
	int xCore_ClipSetTextA(char* sText, int iSize);
	
	// 获取剪贴板内容
	char* xCore_ClipGetTextA();
	
	
	
	// 输入对话框（需使用 xCore.free 释放）
	XXAPI wstr xInputBoxW(HWND hParent, wstr sLabel, wstr sTitle, wstr sText, int x, int y, int iExitStyle);
	XXAPI astr xInputBoxA(HWND hParent, astr sLabel, astr sTitle, astr sText, int x, int y, int iExitStyle);
	
	// 选择文件（需使用 xCore.free 释放）
	XXAPI wstr xOpenFileDialogW(HWND hParent, wstr sDefPath, wstr sFilter, wstr sTitle, int iFlag);
	XXAPI astr xOpenFileDialogA(HWND hParent, astr sDefPath, astr sFilter, astr sTitle, int iFlag);
	
	// 多选文件（需使用 xCore.free 释放）
	XXAPI wstr* xOpenFilesDialogW(HWND hParent, wstr sDefPath, wstr sFilter, wstr sTitle, int iFlag);
	XXAPI astr* xOpenFilesDialogA(HWND hParent, astr sDefPath, astr sFilter, astr sTitle, int iFlag);
	
	// 选择文件夹（需使用 xCore.free 释放）
	XXAPI wstr xSelectFolderDialogW(HWND hParent, wstr sDefPath, wstr sTitle);
	XXAPI astr xSelectFolderDialogA(HWND hParent, astr sDefPath, astr sTitle);
	
	// 保存文件（需使用 xCore.free 释放）
	XXAPI wstr xSaveFileDialogW(HWND hParent, wstr sDefPath, wstr sFilter, wstr sTitle, int iFlag);
	XXAPI astr xSaveFileDialogA(HWND hParent, astr sDefPath, astr sFilter, astr sTitle, int iFlag);
	
	// 选择字体
	XXAPI LOGFONTW* xSelectFontDialogW(HWND hParent);
	XXAPI LOGFONTA* xSelectFontDialogA(HWND hParent);
	
	// 选择颜色
	XXAPI int xSelectColorDialog(HWND hParent, int iColor);



	// 从 DLL 获取 COM 对象
	typedef HRESULT (WINAPI *PFNDllGetClassObject)(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
	int GetDllObjectW(HMODULE hDll, wchar_t* sClsID, wchar_t* sIID, void** objRet);
	int GetDllObjectA(HMODULE hDll, char* sClsID, char* sIID, void** objRet);
	
	
	
	// 初始化 xCore
	XXAPI void xCoreInit(void);
	
	// 释放 xCore
	XXAPI void xCoreUnit(void);
	
	
	
	typedef struct {
		
		// 系统 API 指针
		int (WINAPI *ZwSuspendProcess)(HANDLE Process);
		int (WINAPI *ZwResumeProcess)(HANDLE Process);
		
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
		
		// 库数据
		HKL hCurKL;
		HIMC hIMC;
		
		// 应用信息
		HMODULE AppHandle;
		HWND AppWindow;
		str AppFile;
		str AppPath;
		str BinPath;
		
		// 内存命令 (支持GC)
		ptr (*malloc)(size_t size);
		ptr (*calloc)(size_t count, size_t size);
		ptr (*realloc)(ptr pmem, size_t size);
		void (*free)(ptr pmem);
		
		// GC命令
		struct {
			ptr (*Start)(ptr pStack);
			void (*Stop)(void);
			void (*Pause)(void);
			void (*Resume)(void);
			void (*Run)(void);
			ptr (*Add)(ptr pmem, ulong size, int flags, void(*dtor)(ptr));
			void (*Remove)(ptr pmem);
		} GC;
		
		// 字符集转换
		wstr (*A2W)(const astr pStr, ulong iSize);
		astr (*W2A)(const wstr pStr, ulong iSize);
		wstr (*U2W)(const ustr pStr, ulong iSize);
		ustr (*W2U)(const wstr pStr, ulong iSize);
		ustr (*A2U)(const astr pStr, ulong iSize);
		astr (*U2A)(const ustr pStr, ulong iSize);
		
	} xCoreStruct, *xCoreObject;
	
	
	#ifdef XXRTL_CORE_IMPORTDLL
		__declspec(dllimport) extern xCoreStruct xCore;
	#else
		XXAPI extern xCoreStruct xCore;
	#endif
	
	
	
#endif


