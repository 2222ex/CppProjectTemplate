#pragma once

#include "base/task.h"
#include "ipc/task_contents.h"

class MsgInTask : public ITask
{
private:
    /* data */

public:
    ITask::RET process(std::shared_ptr<TaskContent> content) override;
};

class MsgInTaskSingleton : public Singleton<MsgInTask>
{
};