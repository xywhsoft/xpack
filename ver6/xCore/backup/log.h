// 日志功能包装
void ffrtl_SetLogFileA(astr sFile)
{
	ffrtl.Log.private_LogFileW = ffrtl.A2W(sFile, 0);
	ffrtl.Log.private_LogFileA = sFile;
}
void ffrtl_SetLogFileW(wstr sFile)
{
	ffrtl.Log.private_LogFileW = sFile;
	ffrtl.Log.private_LogFileA = ffrtl.W2A(sFile, 0);
}
void ffrtl_SetLogFileU(ustr sFile)
{
	ffrtl.Log.private_LogFileW = ffrtl.U2W(sFile, 0);
	ffrtl.Log.private_LogFileA = ffrtl.U2A(sFile, 0);
}
void ffrtl_OutLogA(astr sFormat, ...)
{
	va_list ip;
	va_start(ip, sFormat);
	int iSize = vsnprintf(NULL, 0, sFormat, ip);
	va_end(ip);
	if ( iSize > 0 ) {
		astr sRet = malloc(iSize + 1);
		va_start(ip, sFormat);
		iSize = vsnprintf(sRet, iSize + 1, sFormat, ip);
		va_end(ip);
		sRet[iSize] = 0;
		time_t rawtime = time(NULL);
		struct tm* pstm = localtime(&rawtime);
		astr sRetOver = ffbot_FormatA("[%04d-%02d-%02d %02d:%02d:%02d] %s\n", 1900 + pstm->tm_year, pstm->tm_mon + 1, pstm->tm_mday, pstm->tm_hour, pstm->tm_min, pstm->tm_sec, sRet);
		ffbot_FileAppendA(ffrtl.Log.private_LogFileA, sRetOver, ffrtl.retsize, CHARSET_UTF8);
		// 写入日志到UI列表
		/*
		SendMessage(hConList, LB_ADDSTRING, 0, (long)sRet);
		int iCount = SendMessage(hConList, LB_GETCOUNT, 0, 0);
		if ( iCount > 160 ) {
			SendMessage(hConList, LB_DELETESTRING, 0, 0);
		}
		SendMessage(hConList, WM_VSCROLL, SB_BOTTOM, 0);
		*/
		// 回调函数
		if ( ffrtl.Log.OutputProcA ) {
			ffrtl.Log.OutputProcA(sRet, sRetOver);
		}
		ffrtl.free(sRetOver);
		free(sRet);
	}
}
void ffrtl_OutLogW(wstr sFormat, ...)
{
	va_list ip;
	va_start(ip, sFormat);
	int iSize = vsnwprintf(NULL, 0, sFormat, ip);
	va_end(ip);
	if ( iSize > 0 ) {
		wstr sRet = malloc( (iSize + 1) * 2 );
		va_start(ip, sFormat);
		iSize = vsnwprintf(sRet, iSize + 1, sFormat, ip);
		va_end(ip);
		sRet[iSize] = 0;
		time_t rawtime = time(NULL);
		struct tm* pstm = localtime(&rawtime);
		wstr sRetOver = ffbot_FormatW(L"[%04d-%02d-%02d %02d:%02d:%02d] %s\n", 1900 + pstm->tm_year, pstm->tm_mon + 1, pstm->tm_mday, pstm->tm_hour, pstm->tm_min, pstm->tm_sec, sRet);
		ffbot_FileAppendW(ffrtl.Log.private_LogFileW, sRetOver, ffrtl.retsize, CHARSET_UTF8);
		// 写入日志到UI列表
		/*
		SendMessage(hConList, LB_ADDSTRING, 0, (long)sRet);
		int iCount = SendMessage(hConList, LB_GETCOUNT, 0, 0);
		if ( iCount > 160 ) {
			SendMessage(hConList, LB_DELETESTRING, 0, 0);
		}
		SendMessage(hConList, WM_VSCROLL, SB_BOTTOM, 0);
		*/
		// 回调函数
		if ( ffrtl.Log.OutputProcW ) {
			ffrtl.Log.OutputProcW(sRet, sRetOver);
		}
		ffrtl.free(sRetOver);
		free(sRet);
	}
}