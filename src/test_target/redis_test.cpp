#include "redis_test.h"

#include "redis_client.h"
#include "redis_lock.h"

const std::string kRedisCounterKey = "test_counter";
std::shared_ptr<spdlog::logger> redis_log;
bool stop_flag = false;

void work_thread(int thread_id)
{
    while (!stop_flag)
    {
        try
        {
            RedisLock lock(RedisClient::redis_.get(), kRedisCounterKey, 5000, 500, 10);
            if (!lock.try_lock())
            {
                SPDLOG_LOGGER_WARN(redis_log, "work_thread {}: failed to acquire lock", thread_id);
                continue;
            }
            // 成功获得锁，安全地操作计数器
            auto val = RedisClient::redis_->get(kRedisCounterKey);
            int counter = 0;
            if (val)
            {
                counter = std::stoi(*val);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10000)); // 模拟一些工作
            ++counter;
            RedisClient::redis_->set(kRedisCounterKey, std::to_string(counter));
            SPDLOG_LOGGER_INFO(redis_log, "work_thread {}: counter = {}", thread_id, counter);
        }
        catch (const std::exception &e)
        {
            SPDLOG_LOGGER_ERROR(redis_log, "work_thread {}: Redis error: {}", thread_id, e.what());
        }
    }
}

void test_redis()
{
    redis_log = Logger::getLogger("redis", true);
    RedisClient::init();

    RedisClient::redis_->set(kRedisCounterKey, "0");

    const int thread_count = 2;
    std::vector<std::thread> threads;
    for (int i = 0; i < thread_count; ++i)
    {
        threads.emplace_back(work_thread, i);
    }
    std::this_thread::sleep_for(std::chrono::seconds(60));
    stop_flag = true;
    for (auto &t : threads)
    {
        if (t.joinable())
            t.join();
    }
}