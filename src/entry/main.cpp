#include "../base/logger.h"
#include "../base/stdafx.h"

#include "../task_manager/window/main_window.h"

int main(int argc, char *argv[])
{
    for (size_t i = 0; i < argc; i++)
    {
        Logger::Log()->info("arg {} : {}", i, argv[i]);
    }

    MainWindowSingleton::instance().Init();
    return 0;
}
