#include "../base/logger.h"
#include "../base/stdafx.h"

#include "../task_manager/http_server.h"
#include "../task_manager/steam_login.h"
#include "../task_manager/window/main_window.h"

int main(int argc, char *argv[])
{

    std::thread(InitHttpServer).detach();
    std::thread(
        []()
        {
            while (true)
            {
                SteamLoginSingleton::instance().before_login();
            }
        })
        .detach();
    MainWindowSingleton::instance().Init();
    return 0;
}
