#include "task.h"
#include "server_ipc_manager.h"

ITask::RET MsgInTask::process(std::shared_ptr<TaskContent> content)
{
    auto in = std::dynamic_pointer_cast<MsgInContent>(content);
    if (!in)
    {
        return ITask::RET::FAIL;
    }

    Packet packet;
    if (!packet.ParseFromArray(in->task_content.message.data(), static_cast<int32_t>(in->task_content.message.size())))
    {
        SPDLOG_LOGGER_ERROR(Logger::Log(), "parse string failed.");
        return ITask::RET::FAIL;
    }
    SPDLOG_LOGGER_INFO(Logger::Log(), "server recv msg: {}", packet.DebugString());

    Packet pong_pkg;
    auto *pong = pong_pkg.mutable_content()->mutable_pong();
    pong->set_msg("Server: pong.");
    ipc_manager_ptr->send(pong_pkg.SerializeAsString());

    return ITask::RET::SUCCESS;
}
