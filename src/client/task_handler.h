#pragma once
#include "base/logger.h"
#include "base/stdafx.h"
class TaskHandler
{
private:
    /* data */

public:
    TaskHandler(/* args */);
    bool Init();
    ~TaskHandler();
};

class TaskHandlerSingleton : public Singleton<TaskHandler>
{
};