#include "injector.h"
#include <tlhelp32.h>

int Injector::InjectQueueUserAPC(PCWSTR pszLibFile, DWORD dwProcessId, std::string &errMsg)
{

    int cb = (lstrlenW(pszLibFile) + 1) * sizeof(wchar_t);

    HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, dwProcessId);
    if (hProcess == NULL)
    {
        errMsg = fmt::format("[-] Error: Could not open process for PID {}", dwProcessId);
        return 1;
    }

    LPVOID pszLibFileRemote = (PWSTR) VirtualAllocEx(hProcess, NULL, cb, MEM_COMMIT, PAGE_READWRITE);
    if (pszLibFileRemote == NULL)
    {
        errMsg = fmt::format("[-] Error: Could not allocate memory inside PID {}", dwProcessId);
        return 1;
    }

    LPVOID pfnThreadRtn = (LPVOID) GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
    if (pfnThreadRtn == NULL)
    {
        errMsg = fmt::format("[-] Error: Could not find LoadLibraryA function inside kernel32.dll library");
        return 1;
    }

    DWORD n = WriteProcessMemory(hProcess, pszLibFileRemote, (PVOID) pszLibFile, cb, NULL);
    if (n == 0)
    {
        errMsg = fmt::format("[-] Error: Could not write any bytes into the PID {} address space", dwProcessId);
        return 1;
    }

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        errMsg = fmt::format("[-] Error: Unable to get thread information");
        return 1;
    }

    DWORD threadId = 0;
    THREADENTRY32 threadEntry;
    threadEntry.dwSize = sizeof(THREADENTRY32);

    BOOL bResult = Thread32First(hSnapshot, &threadEntry);
    while (bResult)
    {
        bResult = Thread32Next(hSnapshot, &threadEntry);
        if (bResult)
        {
            if (threadEntry.th32OwnerProcessID == dwProcessId)
            {
                threadId = threadEntry.th32ThreadID;

                // SPDLOG_LOGGER_INFO(injector_log, "[+] Using thread: {}", threadId);
                HANDLE hThread = OpenThread(THREAD_SET_CONTEXT, FALSE, threadId);
                if (hThread == NULL)
                {
                    errMsg = fmt::format("[-] Error: Can't open thread. Continuing to try other threads...");
                }
                else
                {
                    DWORD dwResult = QueueUserAPC((PAPCFUNC) pfnThreadRtn, hThread, (ULONG_PTR) pszLibFileRemote);
                    CloseHandle(hThread);

                    if (!dwResult)
                    {
                        errMsg = fmt::format("[-] Error: Couldn't call QueueUserAPC on thread> Continuing to try othrt threads...");
                    }
                    else
                    {
                        errMsg = "[+] Success: DLL injected via CreateRemoteThread()";
                        break;
                    }
                }
            }
        }
    }
    if (!threadId)
    {
        errMsg = "[-] Error: No threads found in thr target process";
    }

    CloseHandle(hSnapshot);
    CloseHandle(hProcess);

    return 0;
}

int Injector::InjectUseCreateRemoteThread(LPCTSTR szDllPath, DWORD dwPID, std::string &errMsg)
{
    HANDLE hProcess = NULL, hThread = NULL;
    HMODULE hMod = NULL;
    LPVOID pRemoteBuf = NULL;

    DWORD dwBufSize = (DWORD) (_tcslen(szDllPath) + 1) * sizeof(TCHAR);
    LPTHREAD_START_ROUTINE pThreadProc;

    if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID)))
    {
        errMsg = fmt::format("OpenProcess({}) failed!!! [{}]\n", dwPID, GetLastError());
        return 1;
    }

    pRemoteBuf = VirtualAllocEx(hProcess, NULL, dwBufSize, MEM_COMMIT, PAGE_READWRITE);

    WriteProcessMemory(hProcess, pRemoteBuf, (LPVOID) szDllPath, dwBufSize, NULL);

    hMod = GetModuleHandle("kernel32.dll");
    pThreadProc = (LPTHREAD_START_ROUTINE) GetProcAddress(hMod, "LoadLibraryA");

    hThread = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, pRemoteBuf, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);

    CloseHandle(hThread);
    CloseHandle(hProcess);

    errMsg = "Used CreateRemoteThread inject success ";

    return TRUE;
}

int Injector::UnLoadLibrary(LPCTSTR szDllName, DWORD dwPID, std::string &errMsg)
{
    BOOL bMore = FALSE, bFound = FALSE;
    HANDLE hSnapshot, hProcess, hThread;
    HMODULE hModule = NULL;
    MODULEENTRY32 me = {sizeof(me)};
    LPTHREAD_START_ROUTINE pThreadProc;

    // 使用TH32CS_SNAPMODULE参数，获取加载到notepad进程的DLL名称
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);

    bMore = Module32First(hSnapshot, &me);
    for (; bMore; bMore = Module32Next(hSnapshot, &me))
    {
        if (!_tcsicmp((LPCTSTR) me.szModule, szDllName) ||
            !_tcsicmp((LPCTSTR) me.szExePath, szDllName))
        {
            bFound = TRUE;
            break;
        }
    }

    if (!bFound)
    {
        errMsg = "dll not found";
        CloseHandle(hSnapshot);
        return 1;
    }

    if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID)))
    {
        errMsg = fmt::format("OpenProcess({}) failed!!! [{}]\n", dwPID, GetLastError());
        return 1;
    }

    hModule = GetModuleHandle("kernel32.dll");
    pThreadProc = (LPTHREAD_START_ROUTINE) GetProcAddress(hModule, "FreeLibrary");
    hThread = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, me.modBaseAddr, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);

    CloseHandle(hThread);
    CloseHandle(hProcess);
    CloseHandle(hSnapshot);

    errMsg = fmt::format("Unload Success");

    return 0;
}

DWORD Injector::FindProcessId(LPCTSTR szProcessName)
{
    DWORD dwPID = 0xFFFFFFFF;
    HANDLE hSnapShot = INVALID_HANDLE_VALUE;
    PROCESSENTRY32 pe;

    pe.dwSize = sizeof(PROCESSENTRY32);
    hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);

    Process32First(hSnapShot, &pe);
    do
    {
        if (!_tcsicmp(szProcessName, (LPCTSTR) pe.szExeFile))
        {
            dwPID = pe.th32ProcessID;
            break;
        }
    } while (Process32Next(hSnapShot, &pe));

    CloseHandle(hSnapShot);

    return dwPID;
}

Injector::Injector(/* args */)
{
}