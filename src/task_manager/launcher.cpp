#include "launcher.h"

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
