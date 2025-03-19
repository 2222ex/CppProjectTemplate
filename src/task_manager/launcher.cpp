#include "launcher.h"

#include <Psapi.h>
#include <Windows.h>

Launcher::Launcher()
{
    log = Logger::Log();

    AppInfo_steam = {
        .name = "steam",
        .path = kSteamLauncherPath,
        .launch_param = kSteamLauncherParam,
        .game_pi = {},
        .exit_code = 0,
        .is_launch = false,
        .success_call_back = {}

    };

    AppInfo_cs2 = {
        .name = "cs2",
        .path = kGameLauncherPath,
        .launch_param = kGameLaunchParam,
        .game_pi = {},
        .exit_code = 0,
        .is_launch = false,
        .success_call_back = {}

    };
}

bool Launcher::launch_application(AppInfo &appInfo)
{
    SPDLOG_LOGGER_TRACE(log, "Launcher::launch_application");
    if (appInfo.is_launch)
    {
        return false;
    }

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

        HMODULE hMod;
        DWORD cbNeeded;
        char buff[255];
        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
        {
            GetModuleBaseNameA(hProcess, hMod, (LPSTR) &buff, DWORD(sizeof(buff) / sizeof(char)));
        }

        if (std::string(buff) == "steamwebhelper.exe")
        {
            if (TerminateProcess(hProcess, 0) != 0)
            {
                err_msg = "steamwebhelper.exe already running but failed to terminate";
                return false;
            }

            CloseHandle(hProcess);
            break;
        }
        if (i == cProcesses - 1)
        {
        }
        CloseHandle(hProcess);
    }

    std::string commandLine;

    // CreateProcess
    STARTUPINFO si = {sizeof(STARTUPINFO)};

    commandLine = appInfo.path + " " + appInfo.launch_param;
    SPDLOG_LOGGER_TRACE(log, "commandLine: {}", commandLine);

    TCHAR szCommandLine[MAX_PATH];
    strncpy(szCommandLine, commandLine.c_str(), MAX_PATH);

    BOOL success = CreateProcess(
        NULL,            // Application name
        szCommandLine,   // Command line
        NULL,            // Process security attributes
        NULL,            // Thread security attributes
        FALSE,           // Inherit handles
        0,               // Creation flags
        NULL,            // Environment variables
        NULL,            // Current directory
        &si,             // Startup information
        &appInfo.game_pi // Process information
    );

    if (!success)
    {
        appInfo.is_launch = false;
        err_msg = "CreateProcess failed. Error: " + std::to_string(GetLastError());
        return false;
    }
    SPDLOG_LOGGER_TRACE(log, "CreateProcess success");
    appInfo.is_launch = true;

    WaitForSingleObject(appInfo.game_pi.hProcess, INFINITE);

    if (GetExitCodeProcess(appInfo.game_pi.hProcess, &appInfo.exit_code))
    {
        SPDLOG_LOGGER_INFO(log, "Process exit with code: {}", appInfo.exit_code);
    }

    appInfo.is_launch = false;

    CloseHandle(appInfo.game_pi.hProcess);
    CloseHandle(appInfo.game_pi.hThread);
    appInfo.game_pi.hProcess = NULL;
    appInfo.game_pi.hThread = NULL;

    return true;
}

bool Launcher::terminate_application(AppInfo &appInfo)
{
    SPDLOG_LOGGER_TRACE(log, "Launcher::terminate_application");
    if (!appInfo.is_launch)
    {
        return false;
    }

    if (appInfo.game_pi.hProcess && TerminateProcess(appInfo.game_pi.hProcess, 0))
    {
        SPDLOG_LOGGER_TRACE(log, "TerminateProcess success");
        return true;
    }

    return false;
}
