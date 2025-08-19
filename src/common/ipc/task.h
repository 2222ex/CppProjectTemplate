#pragma once

#include "base/task.h"
#include "ipc/task_contents.h"
#include "packet.pb.h"

class MsgInTask : public ITask
{
private:
    /* data */

public:
    ITask::RET process(std::shared_ptr<TaskContent> content) override;
    std::map<CMsgContent::MsgTpyeCase, std::function<void(const Packet &)>> msg_callback_map;
};

class MsgInTaskSingleton : public Singleton<MsgInTask>
{
};