#include <stdio.h>
#include <windows.h>

int main() {
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    SECURITY_ATTRIBUTES sa = {0};
    HANDLE hOutputRead, hOutputWrite;

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    CreatePipe(&hOutputRead, &hOutputWrite, &sa, 0);
    SetHandleInformation(hOutputRead, HANDLE_FLAG_INHERIT, 0);

    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = hOutputWrite;
    si.hStdError = hOutputWrite;

    char cmdLine[4096];
    snprintf(cmdLine, sizeof(cmdLine), "\"../../../release/x64/xpkcon.exe\" a test_basic.xpk test_input1.txt");

    printf("Running command: %s\n", cmdLine);

    if (!CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE,
                       0, NULL, NULL, &si, &pi)) {
        printf("CreateProcess failed: %d\n", GetLastError());
        return 1;
    }

    CloseHandle(hOutputWrite);

    char buffer[4096];
    DWORD bytesRead;
    while (ReadFile(hOutputRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        printf("%s", buffer);
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hOutputRead);

    printf("\nExit code: %d\n", exitCode);
    return 0;
}
