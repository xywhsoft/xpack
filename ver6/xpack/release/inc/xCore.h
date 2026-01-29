


#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdint.h>
#include <windows.h>



#ifndef XXRTL_CORE
	#define XXRTL_CORE
	
	
	
	// charset define
	#define XXRTL_CHARSET_ANSI		0
	#define XXRTL_CHARSET_UNICODE	1
	#define XXRTL_CHARSET_UTF8		2
	
	
	
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
	typedef long intptr;
	
	typedef int bool;
	#define true -1
	#define false 0
	#define null 0

	#ifdef BUILD_DLL
		#define XXAPI	__declspec(dllexport)
	#else
		#define XXAPI
	#endif
	
	
	
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
	
	
	
	// GC 申请内存
	ptr xCore_GC_malloc(size_t size);
	
	// GC 申请类内存
	ptr xCore_GC_calloc(size_t count, size_t size);
	
	// 重新申请 GC 内存
	ptr xCore_GC_realloc(ptr pmem, size_t size);
	
	// 释放 GC 内存
	void xCore_GC_free(ptr pmem);
	
	// 暂停 GC 自动执行
	void xCore_GC_Pause(void);
	
	// 恢复 GC 自动执行
	void xCore_GC_Resume(void);
	
	// 执行一次 GC
	void xCore_GC_Run(void);
	
	// 添加一块内存到 GC
	ptr xCore_GC_Add(ptr pmem, ulong size, int flags, void(*dtor)(ptr));
	
	// 从 GC 中移除一块内存
	void xCore_GC_Remove(ptr pmem);
	
	// 获取 GC 栈顶
	ptr xCore_GC_GetStackTop();
	
	// 设置栈顶
	void xCore_GC_SetStackTop(ptr pStack);
	
	// 释放内存（ 释放 malloc 申请的内存，会先判断是否为 nullstring ）
	void xCore_free(ptr pmem);
	
	// 开启GC
	ptr xCore_GC_Start(ptr pStack);
	
	// 关闭GC
	void xCore_GC_Stop();
	
	
	
	// 多字节 转 Unicode
	wstr xCore_M2W(const str pStr, uint32 iCodePage, ulong iSize);
	
	// Unicode 转 多字节
	str xCore_W2M(const wstr pStr, uint32 iCodePage, ulong iSize);
	
	// ANSI 转 Unicode
	wstr xCore_A2W(const astr pZStr, ulong iSize);
	
	// utf-8 转 Unicode
	wstr xCore_U2W(const ustr pUStr, ulong iSize);
	
	// Unicode 转 ANSI
	astr xCore_W2A(const wstr pWStr, ulong iSize);
	
	// Unicode 转 utf-8
	ustr xCore_W2U(const wstr pWStr, ulong iSize);
	
	// ANSI 转 utf-8
	ustr xCore_A2U(const astr pZStr, ulong iSize);
	
	// utf-8 转 ANSI
	astr xCore_U2A(const ustr pUStr, ulong iSize);
	
	
	
	// 字符串格式化（需使用 xCore.free 释放）
	astr xCore_FormatA(astr sFormat, ...);
	wstr xCore_FormatW(wstr sFormat, ...);
	
	// 字符串分割（需使用 xCore.free 释放）
	astr* xCore_SplitCharA(astr sText, char iChar, int bSrcRevise);
	astr* xCore_SplitTextA(astr sText, astr sSep, int bSrcRevise);
	wstr* xCore_SplitCharW(wstr sText, wchar_t iChar, int bSrcRevise);
	wstr* xCore_SplitTextW(wstr sText, wstr sSep, int bSrcRevise);
	astr* xCore_SplitA(astr sText, astr sSep, int bSrcRevise);
	wstr* xCore_SplitW(wstr sText, wstr sSep, int bSrcRevise);
	
	// 字符串替换（需使用 xCore.free 释放）
	astr xCore_ReplaceA(astr original, astr pattern, astr replacement);
	wstr xCore_ReplaceW(wstr original, wstr pattern, wstr replacement);
	
	// 创建字符串副本（需使用 xCore.free 释放）
	wstr xCore_CopyStringW(wstr sText, int iSize);
	astr xCore_CopyStringA(astr sText, int iSize);
	
	// 字符串转为小写（bSrcRevise 为 false 时，需使用 xCore.free 释放内存）
	wstr xCore_LCaseW(wstr sText, int iSize, int bSrcRevise);
	astr xCore_LCaseA(astr sText, int iSize, int bSrcRevise);
	
	// 字符串转为大写（bSrcRevise 为 false 时，需使用 xCore.free 释放内存）
	wstr xCore_UCaseW(wstr sText, int iSize, int bSrcRevise);
	astr xCore_UCaseA(astr sText, int iSize, int bSrcRevise);
	
	
	
	// 初始化 xCore
	void xCoreInit(void);
	
	// 释放 xCore
	void xCoreUnit(void);
	
	
	
	typedef struct {
		
		// 临时性全局数据 (不可改变)
		uint32 version;
		str nullstring;
		ptr objGC;
		
		// 临时性全局数据 (可以改变)
		str sRet;
		ulong iRetSize;
		int iRet;
		int64 iRet64;
		double dRet;
		int LastErrorID;
		str LastError;
		
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
	
	
	
	__declspec(dllimport) extern xCoreStruct xCore;
	
	
	
#endif


