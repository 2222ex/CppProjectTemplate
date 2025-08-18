#include "task.h"

#include "logger.h"

ITask::ITask(size_t parallel_task_num) :
    wait_num(0),
    succ_num(0),
    fail_num(0),
    doing_num(0),
    readd_num(0),
    succ_total_time(0),
    wait_total_time(0),
    lastest_succ_time_point(std::chrono::system_clock::now()),
    lastest_fail_time_point(std::chrono::system_clock::now()),
    parallel_task_num(parallel_task_num),
    status(STATUS::STOP)
{
    executor = std::make_shared<tf::Executor>(parallel_task_num);
}

void ITask::start(bool block, int32_t sleep_ms)
{
    if (status == STATUS::RUN)
    {
        return;
    }
    status = STATUS::RUN;
    auto func = [this, sleep_ms]()
    {
        while (status == STATUS::RUN)
        {
            if (sleep_ms > 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
            }
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait_for(
                lock, std::chrono::milliseconds(1000),
                [this]
                {
                    return tasks.size_approx() || status == STATUS::STOP;
                });
            if (status == STATUS::STOP)
                break;

            size_t task_count = 0;
            while (true)
            {
                std::shared_ptr<TaskContent> task_content;
                auto ret = tasks.try_dequeue(task_content);
                if (!ret)
                    break;

                taskflow.emplace(
                    [task_content = std::move(task_content), this]() mutable
                    {
                        process_once(std::move(task_content));
                    });

                ++task_count;
                if (task_count >= parallel_task_num)
                    break;
            }
            executor->run(taskflow).wait();
            taskflow.clear();
        }
    };
    if (block)
        func();
    else
    {
        std::thread t(func);
        t.detach();
    }
}

void ITask::stop()
{
    status = STATUS::STOP;
}

void ITask::add(std::shared_ptr<TaskContent> contents)
{
    ++wait_num;
    tasks.enqueue(std::move(contents));
    cv.notify_one();
}

void ITask::process_once(std::shared_ptr<TaskContent> task_content)
{
    // SPDLOG_LOGGER_INFO(LoggerSingleton::instance().get_main_logger(), "ITask process_once");

    --wait_num;
    ++doing_num;

    task_content->start_time_point = std::chrono::steady_clock::now();
    wait_total_time += static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(task_content->start_time_point - task_content->recv_time_point).count());

    auto ret = process(task_content);
    switch (ret)
    {
    case RET::SUCCESS:
        ++succ_num;
        task_content->done_time_point = std::chrono::steady_clock::now();
        succ_total_time += static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(task_content->done_time_point - task_content->start_time_point).count());
        break;
    case RET::FAIL:
        ++fail_num;
        SPDLOG_LOGGER_INFO(Logger::Log(), "task fail: {}", task_content->task_id.load());
        break;
    case RET::READD:
        ++readd_num;
        ++fail_num;
        add(task_content);
        break;
    default:
        break;
    }
    --doing_num;
}
