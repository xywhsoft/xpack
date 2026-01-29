


#define XXH_STATIC_LINKING_ONLY   /* access advanced declarations */
#define XXH_IMPLEMENTATION   /* access definitions */
#define XXH_NO_XXH3



#include "xcore.h"

#include "lib/xxhash.h"
#include "lib/avltree.h"

#include "inc/tgc.h"
#include "inc/xcore.h"
#include "inc/charset.h"
#include "inc/base.h"
#include "inc/string.h"
#include "inc/path.h"
#include "inc/ini.h"
#include "inc/xui.h"
#include "inc/dialog.h"
#include "inc/com.h"
#include "inc/table.h"
#include "inc/type.h"
#include "inc/res.h"
#include "inc/chipboard.h"
#include "inc/time.h"



#pragma comment (lib, "user32")
#pragma comment (lib, "shell32")
#pragma comment (lib, "kernel32")
#pragma comment (lib, "shlwapi")
#pragma comment (lib, "comdlg32")
#pragma comment (lib, "gdi32")
#pragma comment (lib, "ole32")
#pragma comment (lib, "imm32")



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
	
	xCore.hCurKL = NULL;
	xCore.hIMC = NULL;
	
	xCore.malloc = malloc;
	xCore.calloc = calloc;
	xCore.realloc = realloc;
	xCore.free = xCore_free;
	
	xCore.AppHandle;
	xCore.AppWindow;
	xCore.AppFile = xCore_AppFileA();
	xCore.AppPath = xCore_AppPathA();
	xCore.BinPath = xCore_ModPathA(NULL);
	
	xCore.GC.Start = xCore_GC_Start;
	xCore.GC.Stop = xCore_GC_Stop;
	xCore.GC.Pause = xCore_GC_Pause;
	xCore.GC.Resume = xCore_GC_Resume;
	xCore.GC.Run = xCore_GC_Run;
	xCore.GC.Add = xCore_GC_Add;
	xCore.GC.Remove = xCore_GC_Remove;
	
	xCore.A2W = xCore_A2W;
	xCore.W2A = xCore_W2A;
	xCore.U2W = xCore_U2W;
	xCore.W2U = xCore_W2U;
	xCore.A2U = xCore_A2U;
	xCore.U2A = xCore_U2A;
	
	// 初始化其他库
	if ( xuiBaseFont == NULL ) {
		xuiInit(0, 0);
	}
	
	// 初始化随机数序列
	srand(GetTickCount());
}

// 释放 xCore
XXAPI void xCoreUnit(void)
{
	xCore_GC_Stop();
	
	// 卸载其他库
	if ( xuiBaseFont != NULL ) {
		xuiUnit();
	}
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


