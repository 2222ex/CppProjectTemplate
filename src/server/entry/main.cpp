#pragma once
#include "base/logger.h"
#include "base/stdafx.h"

#include "server_ipc_manager.h"
#include "task.h"

int main(int argc, char *argv[])
{
    ipc_manager_ptr = std::make_shared<ServerIPCManager>("test_channel");

    std::thread t_ipc_server(
        []()
        {
            ipc_manager_ptr->start_recv();
        });

    std::thread t_msg_handler(
        []()
        {
            MsgInTaskSingleton::instance().start();
        });

    if (t_msg_handler.joinable())
    {
        t_msg_handler.join();
    }
    if (t_ipc_server.joinable())
    {
        t_ipc_server.join();
    }

    return 0;
}
