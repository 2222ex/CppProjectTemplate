#include "launcher.h"

#include "utils/process_util.h"

Launcher::Launcher()
{
    log = Logger::Log();

    AppInfo_cs2.name = "cs2.exe";
    AppInfo_cs2.path = kGameLauncherPath;
    AppInfo_cs2.launch_param = kGameLaunchParam;
    AppInfo_cs2.pi = {};
    AppInfo_cs2.exit_code = 0;
    AppInfo_cs2.is_launch = false;
    AppInfo_cs2.success_call_back = {};
}

bool Launcher::launch_application(AppInfo &appInfo)
{
    SPDLOG_LOGGER_TRACE(log, "Launcher::launch_application");
    if (appInfo.is_launch.load())
    {
        Logger::Log()->info("application {} already launch", appInfo.name);
        return true;
    }

    std::string err_msg;
    DWORD pid = GetProcessPIDByName(appInfo.name, err_msg);

    Logger::Log()->info("get process {} pid: {},err_msg: {}", appInfo.name, pid, err_msg);
    if (pid != 0)
    {
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        appInfo.pi.hProcess = hProcess;

        Logger::Log()->info("steam already running");

        TerminateProcess(hProcess, 0);
    }
    else
    {
    }

    std::string commandLine;

    // CreateProcess
    STARTUPINFO si = {sizeof(STARTUPINFO)};

    commandLine = appInfo.path + " " + appInfo.launch_param;
    SPDLOG_LOGGER_TRACE(log, "commandLine: {}", commandLine);

    TCHAR szCommandLine[MAX_PATH];
    strncpy(szCommandLine, commandLine.c_str(), MAX_PATH);

    BOOL success = CreateProcess(
        NULL,          // Application name
        szCommandLine, // Command line
        NULL,          // Process security attributes
        NULL,          // Thread security attributes
        FALSE,         // Inherit handles
        0,             // Creation flags
        NULL,          // Environment variables
        NULL,          // Current directory
        &si,           // Startup information
        &appInfo.pi    // Process information
    );

    if (!success)
    {
        appInfo.is_launch = false;
        err_msg = "CreateProcess failed. Error: " + std::to_string(GetLastError());
        return false;
    }
    SPDLOG_LOGGER_INFO(log, "CreateProcess {} success", appInfo.name);

    appInfo.is_launch = true;
    appInfo.success_call_back();

    WaitForSingleObject(appInfo.pi.hProcess, INFINITE);

    if (appInfo.pi.hProcess && GetExitCodeProcess(appInfo.pi.hProcess, &appInfo.exit_code))
    {
        SPDLOG_LOGGER_INFO(log, "Process {} exit with code: {}", appInfo.name, appInfo.exit_code);
    }

    appInfo.is_launch = false;

    CloseHandle(appInfo.pi.hProcess);

    appInfo.pi.hProcess = NULL;
    appInfo.pi = {};

    // CloseHandle(appInfo.pi.hThread);
    // appInfo.pi.hThread = NULL;

    return true;
}

bool Launcher::terminate_application(AppInfo &appInfo, std::string &err_msg)
{
    SPDLOG_LOGGER_TRACE(log, "Launcher::terminate_application");
    if (!appInfo.is_launch.load())
    {
        err_msg = fmt::format("{} no launch", appInfo.name);
        return false;
    }

    if (appInfo.pi.hProcess && TerminateProcess(appInfo.pi.hProcess, 0))
    {
        SPDLOG_LOGGER_INFO(log, "TerminateProcess {} success", appInfo.name);
        appInfo.pi = {};
        appInfo.is_launch = false;
        return true;
    }
    else
    {
        SPDLOG_LOGGER_ERROR(log, "TerminateProcess {} failed", appInfo.name);
    }

    return false;
}
