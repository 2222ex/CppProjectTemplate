#include "base/logger.h"

void work()
{
    static int work_count = 0;
    SPDLOG_LOGGER_INFO(Logger::Log(), "work_count: {}", work_count);
}

int main(int argc, char **argv)
{
    bool running = true;
    std::thread test_thread = std::thread(
        [&running]()
        {
            while (running)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                work();
            }
        });
    while (!getchar())
    {
    }
    running = false;
    test_thread.join();
    return 0;
}