#include "../base/logger.h"
#include "../base/stdafx.h"

#include "../task_manager/main_window.h"

int main(int argc, char *argv[])
{
    std::cout << "Hello, World!" << std::endl;
    MainWindowSingleton::instance().Init();
    return 0;
}
