#include "task_handler.h"

#include "ipc/task.h"
#include "server_ipc_manager.h"

TaskHandler::TaskHandler(/* args */)
{
}

TaskHandler::~TaskHandler()
{
}

bool TaskHandler::Init()
{
    MsgInTaskSingleton::instance().msg_callback_map[CMsgContent::MsgTpyeCase::kPing] = [](const Packet &packet)
    {
        auto content = packet.content().ping();
        SPDLOG_LOGGER_INFO(Logger::Log(), "server recv ping: {}", packet.DebugString());

        Packet resp_packet;
        auto resp_pong = resp_packet.mutable_content()->mutable_pong();
        resp_pong->set_msg("pong!");

        ipc_manager_ptr->send(resp_packet.SerializeAsString());
        SPDLOG_LOGGER_INFO(Logger::Log(), "server send pong: {}", resp_packet.DebugString());
    };
    return true;
}