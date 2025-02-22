#include "injector.h"
#include <tlhelp32.h>

int Injector::InjectQueueUserAPC(PCWSTR pszLibFile, DWORD dwProcessId)
{
    auto injector_log = Logger::get_instance().get_named_logger("injector");
    int cb = (lstrlenW(pszLibFile) + 1) * sizeof(wchar_t);

    HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, dwProcessId);
    if (hProcess == NULL)
    {
        SPDLOG_LOGGER_INFO(injector_log, "[-] Error: Could not open process for PID {}", dwProcessId);
        return 1;
    }

    LPVOID pszLibFileRemote = (PWSTR) VirtualAllocEx(hProcess, NULL, cb, MEM_COMMIT, PAGE_READWRITE);
    if (pszLibFileRemote == NULL)
    {
        SPDLOG_LOGGER_INFO(injector_log, "[-] Error: Could not allocate memory inside PID {}", dwProcessId);
        return 1;
    }

    LPVOID pfnThreadRtn = (LPVOID) GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
    if (pfnThreadRtn == NULL)
    {
        SPDLOG_LOGGER_INFO(injector_log, "[-] Error: Could not find LoadLibraryA function inside kernel32.dll library");
        return 1;
    }

    DWORD n = WriteProcessMemory(hProcess, pszLibFileRemote, (PVOID) pszLibFile, cb, NULL);
    if (n == 0)
    {
        SPDLOG_LOGGER_INFO(injector_log, "[-] Error: Could not write any bytes into the PID {} address space", dwProcessId);
        return 1;
    }

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        SPDLOG_LOGGER_INFO(injector_log, "[-] Error: Unable to get thread information");
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
                    SPDLOG_LOGGER_INFO(injector_log, "[-] Error: Can't open thread. Continuing to try other threads...");
                else
                {
                    DWORD dwResult = QueueUserAPC((PAPCFUNC) pfnThreadRtn, hThread, (ULONG_PTR) pszLibFileRemote);
                    if (!dwResult)
                        SPDLOG_LOGGER_INFO(injector_log, "[-] Error: Couldn't call QueueUserAPC on thread> Continuing to try othrt threads...");
                    // else
                    //     SPDLOG_LOGGER_INFO(injector_log, "[+] Success: DLL injected via CreateRemoteThread()");
                    CloseHandle(hThread);
                }
            }
        }
    }
    if (!threadId)
        SPDLOG_LOGGER_INFO(injector_log, "[-] Error: No threads found in thr target process");

    CloseHandle(hSnapshot);
    CloseHandle(hProcess);

    return 0;
}

Injector::Injector(/* args */)
{
}

Injector::~Injector()
{
}