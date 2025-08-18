#include "task.h"

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
        SPDLOG_LOGGER_ERROR(Logger::Log(), "parse string failed. data: {}", in->task_content.message.data());
        return ITask::RET::FAIL;
    }
    SPDLOG_LOGGER_INFO(Logger::Log(), "client recv msg: {}", packet.DebugString());
    return ITask::RET::SUCCESS;
}
