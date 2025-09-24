#pragma once

#include "base/logger.h"
#include "base/stdafx.h"

#include <sw/redis++/redis++.h>

// 简单的集中续约服务：一条线程轮询续期
class RedisLeaseService
{
public:
    struct Entry
    {
        int id;
        sw::redis::Redis *redis;
        std::string key;
        std::string value; // 持有者 token
        int ttl_ms;
        std::chrono::steady_clock::time_point next_tick;
        std::atomic<bool> active {true};
    };

    static RedisLeaseService &instance()
    {
        static RedisLeaseService inst;
        return inst;
    }

    int add(sw::redis::Redis *redis, std::string key, std::string value, int ttl_ms);
    void remove(int id);

private:
    RedisLeaseService();
    ~RedisLeaseService();

    void run_();

    std::mutex mtx_;
    std::condition_variable cv_;
    std::list<Entry> entries_;
    std::atomic<bool> running_;
    std::thread worker_;
    int seq_ = 0;
    std::shared_ptr<spdlog::logger> logger_;
};