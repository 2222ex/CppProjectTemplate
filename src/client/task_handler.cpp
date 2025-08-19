#include "task_handler.h"

#include "client_ipc_manager.h"
#include "ipc/task.h"

TaskHandler::TaskHandler(/* args */)
{
}

TaskHandler::~TaskHandler()
{
}

bool TaskHandler::Init()
{
    MsgInTaskSingleton::instance().msg_callback_map[CMsgContent::MsgTpyeCase::kPong] = [](const Packet &packet)
    {
        auto content = packet.content().ping();
        SPDLOG_LOGGER_INFO(Logger::Log(), "client recv pong: {}", packet.DebugString());
    };
    return true;
}