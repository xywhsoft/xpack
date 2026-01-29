// Plugin 功能包装
ptr ffrtl_Plugin_LoadA(astr sName, ptr pParam)
{
	int iSize = strlen(sName);
	ffbot_pluginobject plug = ffrtl.HashTable.GetValue(ffrtl.tblPlugins, sName, iSize);
	if ( plug ) {
		// 已经加载过的插件，直接返回指针
		plug->iRef++;
		void (*PlugLoad)(astr sName, ptr pParam, ptr pRtl) = (ptr)GetProcAddress(plug->hDLL, "ffbot_plugin_load");
		if ( PlugLoad ) {
			PlugLoad(sName, pParam, &ffrtl);
		}
		return plug->pObj;
	} else {
		// 判断 sName 是否合法（31个字符内、英文开头、英文数字下划线点号组成）
		if ( iSize > 31 ) { ffrtl.retlong = 1; return NULL; }
		if ( (sName[0] < 65) || (sName[0] > 123) || ((sName[0] > 90) && (sName[0] < 97)) ) { ffrtl.retlong = 1; return NULL; }
		for ( int i = 1; i < iSize; i++ ) {
			if ( ((sName[i] > 64) && (sName[i] < 91)) || ((sName[i] > 96) && (sName[i] < 123)) || ((sName[i] > 47) && (sName[i] < 58)) || (sName[i] == 46) || (sName[i] == 95) ) {
			} else {
				ffrtl.retlong = 1;
				return NULL;
			}
		}
		// 判断路径是否存在
		astr sPath = ffrtl.Format("%slib\\%s%d.ffl", ffrtl.AppPathA, sName, FFRTL_BITS);
		if ( ffrtl.FileExistsA(sPath) == FALSE ) {
			ffrtl.retlong = 2;
			return NULL;
		}
		HANDLE hDll = LoadLibraryA(sPath);
		if ( hDll == NULL ) {
			ffrtl.retlong = 3;
			return NULL;
		}
		ptr (*PlugInit)(astr sName, ptr pParam, ptr pRtl) = (ptr)GetProcAddress(hDll, "ffbot_plugin_init");
		if ( PlugInit == NULL ) {
			FreeLibrary(hDll);
			ffrtl.retlong = 4;
			return NULL;
		}
		ptr pObj = PlugInit(sName, pParam, &ffrtl);
		if ( pObj == NULL ) {
			FreeLibrary(hDll);
			ffrtl.retlong = 5;
			return NULL;
		}
		void (*PlugLoad)(astr sName, ptr pParam, ptr pRtl) = (ptr)GetProcAddress(hDll, "ffbot_plugin_load");
		if ( PlugLoad ) {
			PlugLoad(sName, pParam, &ffrtl);
		}
		plug = malloc(sizeof(ffbot_pluginstruct));
		plug->hDLL = hDll;
		plug->iRef = 1;
		plug->pObj = pObj;
		ffrtl.HashTable.SetValue(ffrtl.tblPlugins, sName, iSize, plug, sizeof(ffbot_pluginstruct));
		free(plug);
		return pObj;
	}
}
ptr ffrtl_Plugin_LoadW(wstr sName, ptr pParam)
{
	return ffrtl_Plugin_LoadA(ffrtl.W2A(sName, 0), pParam);
}

ptr ffrtl_Plugin_GetA(astr sName)
{
	int iSize = strlen(sName);
	ffbot_pluginobject plug = ffrtl.HashTable.GetValue(ffrtl.tblPlugins, sName, iSize);
	if ( plug ) {
		return plug->pObj;
	} else {
		return NULL;
	}
}
ptr ffrtl_Plugin_GetW(wstr sName)
{
	return ffrtl_Plugin_GetA(ffrtl.W2A(sName, 0));
}

int ffrtl_Plugin_FreeA(astr sName, ptr pParam)
{
	int iSize = strlen(sName);
	ffbot_pluginobject plug = ffrtl.HashTable.GetValue(ffrtl.tblPlugins, sName, iSize);
	if ( plug ) {
		// 释放插件
		plug->iRef--;
		void (*PlugFree)(astr sName, ptr pParam, ptr pRtl) = (ptr)GetProcAddress(plug->hDLL, "ffbot_plugin_free");
		if ( PlugFree ) {
			PlugFree(sName, pParam, &ffrtl);
		}
		// 释放DLL
		if ( plug->iRef <= 0 ) {
			void (*PlugUnit)(astr sName, ptr pParam, ptr pRtl) = (ptr)GetProcAddress(plug->hDLL, "ffbot_plugin_unit");
			if ( PlugUnit ) {
				PlugUnit(sName, pParam, &ffrtl);
			}
			FreeLibrary(plug->hDLL);
			ffrtl.HashTable.Remove(ffrtl.tblPlugins, sName, iSize);
		}
		return TRUE;
	} else {
		return FALSE;
	}
}
int ffrtl_Plugin_FreeW(wstr sName, ptr pParam)
{
	return ffrtl_Plugin_FreeA(ffrtl.W2A(sName, 0), pParam);
}