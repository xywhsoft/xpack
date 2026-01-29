


/* ------------------------------------ C 函数库 ------------------------------------ */



// 释放内存（ 释放 malloc 申请的内存，会先判断是否为 nullstring ）
XXAPI void xCore_free(ptr pmem)
{
	if ( pmem && (pmem != xCore.nullstring) ) { free(pmem); }
}



// 获取模块的文件路径（需要使用 xCore_free 释放内存）
XXAPI wstr xCore_ModFileW(HANDLE hInst)
{
	wstr sFile = malloc(MAX_PATH * 2);
	xCore.iRetSize = GetModuleFileNameW(hInst, sFile, MAX_PATH);
	return sFile;
}
XXAPI astr xCore_ModFileA(HANDLE hInst)
{
	astr sFile = malloc(MAX_PATH);
	xCore.iRetSize = GetModuleFileNameA(hInst, sFile, MAX_PATH);
	return sFile;
}
XXAPI ustr xCore_ModFileU(HANDLE hInst)
{
	wstr sFile = xCore_AppFileW(hInst);
	ustr sRet = xCore_W2U(sFile, xCore.iRetSize);
	free(sRet);
	return sRet;
}

// 获取程序文件路径（需要使用 xCore_free 释放内存）
XXAPI wstr xCore_AppFileW() { return xCore_ModFileW(NULL); }
XXAPI astr xCore_AppFileA() { return xCore_ModFileA(NULL); }
XXAPI ustr xCore_AppFileU() { return xCore_ModFileU(NULL); }



// 获取模块所在文件夹的路径，总是以 \ 结尾（需要使用 xCore_free 释放内存）
XXAPI wstr xCore_ModPathW(HANDLE hInst)
{
	wstr sFile = malloc(MAX_PATH * 2);
	GetModuleFileNameW(hInst, sFile, MAX_PATH);
	xCore.iRetSize = wcsrchr(sFile, L'\\') - sFile + 1;
	sFile[xCore.iRetSize] = 0;
	return sFile;
}
XXAPI astr xCore_ModPathA(HANDLE hInst)
{
	astr sFile = malloc(MAX_PATH);
	GetModuleFileNameA(hInst, sFile, MAX_PATH);
	xCore.iRetSize = strrchr(sFile, L'\\') - sFile + 1;
	sFile[xCore.iRetSize] = 0;
	return sFile;
}
XXAPI ustr xCore_ModPathU(HANDLE hInst)
{
	wstr sFile = xCore_ModPathW(hInst);
	ustr sRet = xCore_W2U(sFile, xCore.iRetSize * 2);
	free(sFile);
	return sRet;
}

// 获取程序所在文件夹的路径，总是以 \ 结尾（需要使用 xCore_free 释放内存）
XXAPI wstr xCore_AppPathW() { return xCore_ModPathW(NULL); }
XXAPI astr xCore_AppPathA() { return xCore_ModPathA(NULL); }
XXAPI ustr xCore_AppPathU() { return xCore_ModPathU(NULL); }



// 运行程序
XXAPI HANDLE xCore_RunW(wstr sPath, int iShow)
{
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	GetStartupInfoW(&si);
	si.wShowWindow = iShow;
	CreateProcessW(NULL, sPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	return pi.hProcess;
}
XXAPI HANDLE xCore_RunA(astr sPath, int iShow)
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	GetStartupInfoA(&si);
	si.wShowWindow = iShow;
	CreateProcessA(NULL, sPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	return pi.hProcess;
}
XXAPI HANDLE xCore_RunU(ustr sPath, int iShow)
{
	wstr sPathW = xCore_U2W(sPath, 0);
	HANDLE hRet = xCore_RunW(sPathW, iShow);
	free(sPathW);
	return hRet;
}



// 打开文件
XXAPI HANDLE xCore_StartW(wstr sPath, int iShow)
{
	HANDLE hRet = ShellExecuteW(0, NULL, sPath, NULL, NULL, iShow);
	return hRet;
}
XXAPI HANDLE xCore_StartA(astr sPath, int iShow)
{
	HANDLE hRet = ShellExecuteA(0, NULL, sPath, NULL, NULL, iShow);
	return hRet;
}
XXAPI HANDLE xCore_StartU(ustr sPath, int iShow)
{
	wstr sPathW = xCore_U2W(sPath, 0);
	HANDLE hRet = xCore_StartW(sPathW, iShow);
	free(sPathW);
	return hRet;
}



// 运行程序并等待程序运行结束
XXAPI int xCore_ChainW(wstr sPath, int iShow)
{
	DWORD iRet = 0;
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	GetStartupInfoW(&si);
	si.wShowWindow = iShow;
	CreateProcessW(NULL, sPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	WaitForSingleObject(pi.hProcess, INFINITE);
	GetExitCodeProcess(pi.hProcess, &iRet);
	return iRet;
}
XXAPI int xCore_ChainA(astr sPath, int iShow)
{
	DWORD iRet = 0;
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	GetStartupInfoA(&si);
	si.wShowWindow = iShow;
	CreateProcessA(NULL, sPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	WaitForSingleObject(pi.hProcess, INFINITE);
	GetExitCodeProcess(pi.hProcess, &iRet);
	return iRet;
}
XXAPI int xCore_ChainU(ustr sPath, int iShow)
{
	wstr sPathW = xCore_U2W(sPath, 0);
	int iRet = xCore_ChainW(sPathW, iShow);
	free(sPathW);
	return iRet;
}



// 运行程序，返回控制台输出
XXAPI astr xShellA(astr sCMD)
{
	char buffer[1024];
	FILE *fp = popen(sCMD, "r");
	if ( fp == NULL ) {
		return xCore.nullstring;
	}
	MBMU_Object objBuf = MBMU_Create(16384, 16384);
	while ( fgets(buffer, 1024, fp) != NULL ) {
		MBMU_Append(objBuf, buffer, 0, MBMU_UTF8);
	}
	pclose(fp);
	astr sRet = malloc(objBuf->Length + 1);
	memcpy(sRet, objBuf->Buffer, objBuf->Length);
	sRet[objBuf->Length] = 0;
	MBMU_Destroy(objBuf);
	return sRet;
}
XXAPI ustr xShellU(ustr sCMD)
{
	char buffer[1024];
	FILE *fp = popen(sCMD, "r");
	if ( fp == NULL ) {
		return xCore.nullstring;
	}
	MBMU_Object objBuf = MBMU_Create(16384, 16384);
	while ( fgets(buffer, 1024, fp) != NULL ) {
		MBMU_Append(objBuf, buffer, 0, MBMU_UTF8);
	}
	pclose(fp);
	ustr sRet = xCore_A2U(objBuf->Buffer, objBuf->Length);
	MBMU_Destroy(objBuf);
	return sRet;
}
XXAPI wstr xShellW(wstr sCMD)
{
	char buffer[1024];
	FILE *fp = wpopen(sCMD, L"r");
	if ( fp == NULL ) {
		return (wstr)xCore.nullstring;
	}
	MBMU_Object objBuf = MBMU_Create(16384, 16384);
	while ( fgets(buffer, 1024, fp) != NULL ) {
		MBMU_Append(objBuf, buffer, 0, MBMU_UTF8);
	}
	pclose(fp);
	wstr sRet = xCore_A2W(objBuf->Buffer, objBuf->Length);
	MBMU_Destroy(objBuf);
	return sRet;
}



// 获取系统目录
XXAPI wstr xCore_SystemPathW(int csidl)
{
	LPITEMIDLIST pidl;
	wstr sRet = malloc(MAX_PATH * 2);
	SHGetSpecialFolderLocation(0, csidl, &pidl);
	SHGetPathFromIDListW(pidl, sRet);
	// 如果目录不以 \ 结尾则补充
	int iSize = wcslen(sRet);
	if (sRet[iSize - 1] != 92) {
		sRet[iSize] = 92;
		sRet[iSize + 1] = 0;
	}
	xCore.iRetSize = iSize;
	return sRet;
}
XXAPI astr xCore_SystemPathA(int csidl)
{
	LPITEMIDLIST pidl;
	astr sRet = malloc(MAX_PATH);
	SHGetSpecialFolderLocation(0, csidl, &pidl);
	SHGetPathFromIDListA(pidl, sRet);
	// 如果目录不以 \ 结尾则补充
	int iSize = strlen(sRet);
	if (sRet[iSize - 1] != 92) {
		sRet[iSize] = 92;
		sRet[iSize + 1] = 0;
	}
	xCore.iRetSize = iSize;
	return sRet;
}
XXAPI ustr xCore_SystemPathU(int csidl)
{
	wstr sPath = xCore_SystemPathW(csidl);
	ustr sPathU = xCore_W2U(sPath, 0);
	xCore_free(sPath);
	return sPathU;
}



// 随机数
XXAPI int xCore_Rand(int min, int max)
{
	if ( min == max ) {
		return min;
	} else if ( min > max ) {
		return max + (rand() % (min - max));
	} else {
		return min + (rand() % (max - min));
	}
}



// 获取系统位数
XXAPI int xCore_SysBits()
{
	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);
	if ( (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) || (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64) ) {
		return 64;
	} else {
		return 32;
	}
}


