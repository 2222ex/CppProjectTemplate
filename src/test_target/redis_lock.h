#pragma once

#include "base/logger.h"
#include "base/stdafx.h"
#include "redis_lease_service.h"

#define REDIS_LOCK_DEBUG

class RedisLock
{
private:
    std::shared_ptr<spdlog::logger> logger_;
    sw::redis::Redis *redis;
    std::string key;
    std::string value;
    int ttl_ms;
    int retry_interval_ms;
    int max_retries;

    // 租约由集中服务管理，此处仅保存注册 id
    int lease_id = 0;

    bool acquired_ = false;

    struct HeldInfo
    {
        std::string value;
        int count = 0;
        int lease_id = 0;
        int ttl_ms = 0;
    };

    inline static thread_local std::unordered_map<std::string, HeldInfo> held_;

    // 生成唯一锁值（UUIDv4 风格，32位hex）
    static std::string generate_lock_token()
    {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<unsigned> dist(0, 255);

        std::array<unsigned char, 16> bytes {};
        for (auto &b : bytes)
            b = static_cast<unsigned char>(dist(gen));
        // UUID v4 / variant 位
        bytes[6] = (bytes[6] & 0x0F) | 0x40;
        bytes[8] = (bytes[8] & 0x3F) | 0x80;

        std::ostringstream oss;
        for (auto b : bytes)
        {
            oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
        }
        return oss.str();
    }

    bool try_lock_once()
    {
        try
        {
            return redis->set(key, value, std::chrono::milliseconds(ttl_ms), sw::redis::UpdateType::NOT_EXIST);
        }
        catch (const std::exception &e)
        {
            SPDLOG_LOGGER_ERROR(Logger::getLogger("redis"), "RedisLock::try_lock_once error: {}", e.what());
            return false;
        }
    }

public:
    RedisLock(sw::redis::Redis *redis, const std::string &key, int ttl_ms = 10000, int retry_interval_ms = 1000, int max_retries = 5) :
        redis(redis),
        key("lock_" + key),
        value(generate_lock_token()),
        ttl_ms(ttl_ms),
        retry_interval_ms(retry_interval_ms),
        max_retries(max_retries)

    {
        logger_ = Logger::getLogger("redis", true);
    }

    ~RedisLock()
    {
        unlock();
    }

    bool try_lock()
    {

        auto it = held_.find(key);
        if (it != held_.end())
        {

            // 递归重入
            it->second.count += 1;
            value = it->second.value;
            lease_id = it->second.lease_id;
            acquired_ = true;
#ifdef REDIS_LOCK_DEBUG
            SPDLOG_LOGGER_INFO(logger_, "re-entering, depth={}", it->second.count);
#endif
            return true;
        }

        for (int i = 0; i < max_retries; ++i)
        {
            if (try_lock_once())
            {
                // 注册到集中续约服务
                lease_id = RedisLeaseService::instance().add(redis, key, value, ttl_ms);
                HeldInfo info;
                info.value = value;
                info.count = 1;
                info.lease_id = lease_id;
                info.ttl_ms = ttl_ms;
                held_[key] = info;
                acquired_ = true;
#ifdef REDIS_LOCK_DEBUG
                SPDLOG_LOGGER_INFO(logger_, "acquired lock");
#endif
                return true;
            }
#ifdef REDIS_LOCK_DEBUG
            SPDLOG_LOGGER_INFO(logger_, "retrying {} / {}", i + 1, max_retries);
#endif

            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms));
        }

        return false;
    }

    void unlock()
    {
        if (!acquired_)
        {
            return;
        }

        auto it = held_.find(key);
        if (it == held_.end())
        {
            acquired_ = false;
            return; // 未持有锁
        }

        // 递归释放
        if (--(it->second.count) > 0)
        {
            acquired_ = false;
#ifdef REDIS_LOCK_DEBUG
            SPDLOG_LOGGER_INFO(logger_, "lock decrementing depth to {}", it->second.count);
#endif
            return;
        }

        // 先停止续约，避免解锁过程中又被续期
        if (lease_id > 0)
        {
            RedisLeaseService::instance().remove(lease_id);
            lease_id = 0;
        }

        const std::string lua_script = R"(
            if redis.call('get', KEYS[1]) == ARGV[1] then
                return redis.call('del', KEYS[1])
            else
                return 0
            end
        )";
        try
        {
            redis->eval<long long>(
                lua_script,
                {key},
                {value});
        }
        catch (const std::exception &e)
        {
            SPDLOG_LOGGER_ERROR(logger_, "RedisLock::unlock error: {}", e.what());
        }
#ifdef REDIS_LOCK_DEBUG
        SPDLOG_LOGGER_INFO(logger_, "released lock");
#endif
    }
};