#include "base/logger.h"

bool running = true;

// 导出函数,使其可以被 DLL 通过函数名找到
extern "C" __declspec(dllexport) int work(int work_count)
{
    SPDLOG_LOGGER_INFO(Logger::getLogger("work", true), "work_count: {}", work_count);
    return ++work_count;
}

class Parent
{
private:
    inline static std::shared_ptr<spdlog::logger> log = Logger::getLogger("Parent", true);

public:
    virtual int v1(int a)
    {
        SPDLOG_LOGGER_INFO(log, "v1_count: {}", a);
        return a + 1;
    }

    virtual int v2(int a)
    {
        SPDLOG_LOGGER_INFO(log, "v2_count: {}", a);
        return a + 1;
    }
};

extern "C" __declspec(dllexport) Parent *CreateParent()
{
    return new Parent();
}

void parent_worker()
{
    Parent *obj = CreateParent();
    int v1_count = 0;
    int v2_count = 0;
    while (running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        v1_count = obj->v1(v1_count);

        v2_count = obj->v2(v2_count);
    }
}

int main(int argc, char **argv)
{

    std::thread t1 = std::thread(
        []()
        {
            int work_count = 0;
            while (running)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                work_count = work(work_count);
            }
        });
    std::thread t2(parent_worker);
    while (!getchar())
    {
    }
    running = false;
    t1.join();
    t2.join();
    return 0;
}