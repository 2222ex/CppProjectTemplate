#include "process_util.h"

DWORD GetProcessPIDByName(std::string process_name, std::string &err_msg)
{
    DWORD aProcesses[1024], cbNeeded;
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
    {
        err_msg = "EnumProcesses failed";
        return false;
    }
    unsigned int cProcesses = cbNeeded / sizeof(DWORD);
    for (unsigned int i = 0; i < cProcesses; ++i)
    {
        DWORD curr_pid = aProcesses[i];
        if (curr_pid == 0)
            continue;
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, curr_pid);
        if (hProcess == NULL)
            continue;

        CloseHandle(hProcess);

        HMODULE hMod;
        DWORD cbNeeded;
        char buff[255];
        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
        {
            GetModuleBaseNameA(hProcess, hMod, (LPSTR) &buff, DWORD(sizeof(buff) / sizeof(char)));
        }

        if (std::string(buff) == process_name)
        {
            return curr_pid;
        }
        if (i == cProcesses - 1)
        {
            return 0;
        }
    }

    return 0;
}