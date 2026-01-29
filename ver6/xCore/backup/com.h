


 int GetDllObjectW(HMODULE hDll, wchar_t* sClsID, wchar_t* sIID, void** objRet)
{
	PFNDllGetClassObject Proc_DllGetClassObject = (PFNDllGetClassObject)GetProcAddress(hDll, "DllGetClassObject");
	if ( Proc_DllGetClassObject ) {
		CLSID ClassID;
		IID InterfaceID;
		struct IClassFactory* pICF;
		CLSIDFromString((LPOLESTR)sClsID, &ClassID);
		IIDFromString((LPOLESTR)sIID, &InterfaceID);
		HRESULT iRet = Proc_DllGetClassObject(&ClassID, &IID_IClassFactory, (LPVOID)&pICF);
		if ( iRet == S_OK ) {
			iRet = pICF->lpVtbl->CreateInstance(pICF, NULL, &InterfaceID, objRet);
			pICF->lpVtbl->Release(pICF);
			return -1;
		}
	}
	return 0;
}
int GetDllObjectA(HMODULE hDll, char* sClsID, char* sIID, void** objRet)
{
	PFNDllGetClassObject Proc_DllGetClassObject = (PFNDllGetClassObject)GetProcAddress(hDll, "DllGetClassObject");
	if ( Proc_DllGetClassObject ) {
		CLSID ClassID;
		IID InterfaceID;
		struct IClassFactory* pICF;
		wchar_t* sClsIDW = xCore_A2W(sClsID, 0);
		wchar_t* sIIDW = xCore_A2W(sIID, 0);
		CLSIDFromString((LPOLESTR)sClsIDW, &ClassID);
		IIDFromString((LPOLESTR)sIIDW, &InterfaceID);
		xCore_free(sClsIDW);
		xCore_free(sIIDW);
		HRESULT iRet = Proc_DllGetClassObject(&ClassID, &IID_IClassFactory, (LPVOID)&pICF);
		if ( iRet == S_OK ) {
			iRet = pICF->lpVtbl->CreateInstance(pICF, NULL, &InterfaceID, objRet);
			pICF->lpVtbl->Release(pICF);
			return -1;
		}
	}
	return 0;
}


