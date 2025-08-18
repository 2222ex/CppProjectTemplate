#ifndef BASE_TASK_H
#define BASE_TASK_H

#include "stdafx.h"

#include "logger.h"
#include "packet.pb.h"
#include "task_contents.h"

class ITask
{
public:
    enum class STATUS
    {
        STOP = 0,
        RUN = 1,
    };
    enum class RET
    {
        SUCCESS = 1,
        FAIL = 0,
        READD = -1,
    };
    ITask(size_t parallel_task_num = std::thread::hardware_concurrency());
    virtual ~ITask() = default;

    void start(bool block = false, int32_t sleep_ms = 0);
    void stop();
    virtual void add(std::shared_ptr<TaskContent> contents);
    virtual void process_once(std::shared_ptr<TaskContent> task_content);
    virtual RET process(std::shared_ptr<TaskContent> task_content) = 0;

    std::atomic<uint32_t> wait_num;
    std::atomic<uint32_t> succ_num;
    std::atomic<uint32_t> fail_num;
    std::atomic<uint32_t> doing_num;
    std::atomic<uint32_t> readd_num;
    std::atomic<uint64_t> succ_total_time;
    std::atomic<uint64_t> wait_total_time;

    std::atomic<std::chrono::system_clock::time_point> lastest_succ_time_point;
    std::atomic<std::chrono::system_clock::time_point> lastest_fail_time_point;

private:
    size_t parallel_task_num;
    STATUS status;

    moodycamel::ConcurrentQueue<std::shared_ptr<TaskContent>> tasks;
    std::condition_variable cv;
    std::mutex mtx;

    std::shared_ptr<tf::Executor> executor;
    tf::Taskflow taskflow;
};

#endif // BASE_TASK_H
