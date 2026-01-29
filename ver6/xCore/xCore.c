


#define XXH_STATIC_LINKING_ONLY   /* access advanced declarations */
#define XXH_IMPLEMENTATION   /* access definitions */
#define XXH_NO_XXH3



#include "xcore.h"

#define MMU_USE_MBMU								// 自增长缓冲区
#include "../mmu.h"
#include "inc/base.h"
#include "inc/charset.h"
#include "inc/string.h"
#include "inc/path.h"
#include "inc/type.h"
#include "inc/time.h"



#pragma comment (lib, "user32")
#pragma comment (lib, "shell32")
#pragma comment (lib, "kernel32")
#pragma comment (lib, "shlwapi")



// 初始化 xCore
xCoreStruct xCore;
XXAPI void xCoreInit(void)
{
	
	// 初始化数据
	xCore.version = 0x00000100;
	xCore.nullstring = (str)"\0\0\0";
	xCore.objGC = 0;
	xCore.SysBits = xCore_SysBits();
	xCore.sRet = xCore.nullstring;
	xCore.iRetSize = 0;
	xCore.iRet = 0;
	xCore.iRet64 = 0;
	xCore.dRet = 0.0;
	xCore.LastErrorID = 0;
	xCore.LastError = xCore.nullstring;
	
	xCore.AppHandle;
	xCore.AppWindow;
	xCore.AppFile = xCore_AppFileA();
	xCore.AppPath = xCore_AppPathA();
	xCore.BinPath = xCore_ModPathA(NULL);
	
	// 初始化随机数序列
	srand(GetTickCount());
}

// 释放 xCore
XXAPI void xCoreUnit(void)
{
	
}



#ifdef DBUILD_DLL
	BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
	{
		if ( fdwReason == DLL_PROCESS_ATTACH ) {
			//当进程加载dll时调用dllMain
			xCoreInit();
		} else if ( fdwReason == DLL_PROCESS_DETACH ) {
			//当进程卸载dll时调用dllMain
			xCoreUnit();
		} else if ( fdwReason == DLL_THREAD_ATTACH ) {
			//当线程加载dll时调用dllMain
			
		} else if ( fdwReason == DLL_THREAD_DETACH ) {
			//当线程卸载dll时调用dllMain
			
		}
		return (TRUE);
	}
#endif


