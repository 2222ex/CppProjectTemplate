#include "task_contents.h"

std::atomic<uint64_t> TaskContent::next_task_id {0};

TaskContent::TaskContent() :
    recv_time_point(std::chrono::steady_clock::now()),
    task_id(get_next_task_id())
{
}

TaskContent::~TaskContent() {}

uint64_t TaskContent::get_next_task_id()
{
    return next_task_id.fetch_add(1, std::memory_order_relaxed);
}
