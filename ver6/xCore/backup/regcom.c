


int RegCOM(astr sFile)
{
	HANDLE hDll = LoadLibraryA(sFile);
	if ( hDll == NULL ) {
		return FALSE;
	}
	HRESULT (WINAPI *COM_DLLRegisterServer)(void) = (ptr)GetProcAddress(hDll, "DLLRegisterServer");
	if ( COM_DLLRegisterServer ) {
		return FALSE;
	}
	if ( COM_DLLRegisterServer() == S_OK ) {
		return TRUE;
	} else {
		return FALSE;
	}
}


