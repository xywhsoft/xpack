#include <stdio.h>
#include <windows.h>

int main() {
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    char cmdLine[4096];
    snprintf(cmdLine, sizeof(cmdLine), "\"../../../release/x64/xpkcon.exe\" a test_basic.xpk test_input1.txt");

    printf("Running command: %s\n", cmdLine);

    if (!CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE,
                       0, NULL, NULL, &si, &pi)) {
        printf("CreateProcess failed: %d\n", GetLastError());
        return 1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    printf("Exit code: %d\n", exitCode);
    return 0;
}
