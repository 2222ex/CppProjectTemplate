#include "process_util.h"

#include "logger.h"

DWORD GetProcessPIDByName(std::string process_name, std::string &err_msg)
{
    DWORD aProcesses[4096], cbNeeded;
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
    {
        err_msg = fmt::format("EnumProcesses failed, err: {}", GetLastError());
        return -1;
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

        HMODULE hMod;
        DWORD cbNeeded;
        char buff[255] = {0};
        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
        {
            GetModuleBaseNameA(hProcess, hMod, (LPSTR) &buff, DWORD(sizeof(buff) / sizeof(char)));
        }
        else
        {

            err_msg = fmt::format("EnumProcessModules failed, err: {}", GetLastError());
            continue;
        }
        // Logger::Log()->info("process_name: {},buff: {}", process_name, buff);

        CloseHandle(hProcess);

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