#pragma once

#include "base/task_contents.h"

struct InOutMessage
{
    enum class Type : int8_t
    {
        IPC = 3,
    };
    Type type;
    std::variant<std::string, uint32_t> id;
    std::string message;
};

class MsgInContent : public TaskContent
{
private:

public:
    MsgInContent(const InOutMessage &&_task_content);
    InOutMessage task_content;
};
