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
