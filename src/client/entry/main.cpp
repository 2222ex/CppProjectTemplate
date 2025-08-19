#include "base/logger.h"
#include "base/stdafx.h"

#include "client_ipc_manager.h"
#include "ipc/task.h"
#include "task_handler.h"

int main(int argc, char *argv[])
{
    ipc_manager_ptr = std::make_shared<ClientIPCManager>("test_channel");

    TaskHandlerSingleton::instance().Init();

    std::thread t_msg_handler(
        []()
        {
            MsgInTaskSingleton::instance().start();
        });

    std::thread t_ipc_server(
        []()
        {
            ipc_manager_ptr->start_recv();
        });

    std::thread(
        []()
        {
            while (true)
            {
                Packet packet;
                auto *msg = packet.mutable_content();
                auto *ping = msg->mutable_ping();
                ping->set_msg("Ping?");

                ipc_manager_ptr->send(packet.SerializeAsString());
                SPDLOG_LOGGER_INFO(Logger::Log(), "Send msg: {}", packet.DebugString());
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        })
        .join();

    if (t_msg_handler.joinable())
    {
        t_msg_handler.join();
    }

    return 0;
}
