#ifndef BASE_TASK_CONTENTS_H
#define BASE_TASK_CONTENTS_H

#include "stdafx.h"

class TaskContent
{
public:
    TaskContent();
    virtual ~TaskContent();

    static uint64_t get_next_task_id();

    std::chrono::steady_clock::time_point recv_time_point;
    std::chrono::steady_clock::time_point start_time_point;
    std::chrono::steady_clock::time_point done_time_point;

    std::atomic<uint64_t> task_id;
    static std::atomic<uint64_t> next_task_id;
};

#endif // BASE_TASK_CONTENTS_H