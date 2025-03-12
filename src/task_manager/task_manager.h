#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include "logger.h"
#include "singleton.h"
#include "stdafx.h"

class TaskManager
{
private:
    /* data */

public:
    TaskManager(/* args */);
    ~TaskManager() = default;

    void get_task();
};

class TaskManagerSingleton : public Singleton<TaskManager, true>
{
};

#endif