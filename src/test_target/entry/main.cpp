#include "base/logger.h"

int work(int work_count)
{
    SPDLOG_LOGGER_INFO(Logger::getLogger("work", true), "work_count: {}", work_count);
    return ++work_count;
}

int main(int argc, char **argv)
{
    bool running = true;
    std::thread test_thread = std::thread(
        [&running]()
        {
            int work_count = 0;
            while (running)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                work_count = work(work_count);
            }
        });
    while (!getchar())
    {
    }
    running = false;
    test_thread.join();
    return 0;
}