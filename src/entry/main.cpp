#include "../base/logger.h"
#include "../base/stdafx.h"

#include "../task_manager/http/http_server.h"
#include "../task_manager/steam_login.h"
#include "../task_manager/window/main_window.h"

BOOL EnableDebugPrivilege()
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
        return FALSE;

    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
        return FALSE;

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
        return FALSE;

    CloseHandle(hToken);
    return TRUE;
}

int main(int argc, char *argv[])
{
    if (EnableDebugPrivilege() == FALSE)
    {
        Logger::Log()->error("EnableDebugPrivilege failed");
    }

    std::thread(TaskManagerHttpServer::InitHttpServer).detach();
    std::thread(
        []()
        {
            while (true)
            {
                std::string err_msg;
                if (SteamLoginSingleton::instance().before_login(err_msg))
                {
                    // Logger::Log()->info("before_login succ");
                }
                else
                {
                    Logger::Log()->error("before_login failed, err_msg: {}", err_msg);
                }
            }
        })
        .detach();
    MainWindowSingleton::instance().Init();
    return 0;
}
