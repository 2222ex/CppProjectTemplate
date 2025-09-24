#include "redis_lease_service.h"

RedisLeaseService::RedisLeaseService() :
    running_(true), worker_([this]
                            {
                                run_();
                            })
{
    logger_ = Logger::getLogger("redis", true); // 初始化 logger
}

RedisLeaseService::~RedisLeaseService()
{
    running_ = false;
    cv_.notify_one();
    if (worker_.joinable())
        worker_.join();
}

int RedisLeaseService::add(sw::redis::Redis *redis, std::string key, std::string value, int ttl_ms)
{
    std::lock_guard<std::mutex> lk(mtx_);
    int id = ++seq_;
    auto now = std::chrono::steady_clock::now();

    auto &e = entries_.emplace_back();
    e.id = id;
    e.redis = redis;
    e.key = std::move(key);
    e.value = std::move(value);
    e.ttl_ms = ttl_ms;
    e.next_tick = now + std::chrono::milliseconds(ttl_ms / 2); // 下一次续期时间
    e.active.store(true, std::memory_order_relaxed);

    cv_.notify_one();
    return id;
}

void RedisLeaseService::remove(int id)
{
    if (id <= 0)
        return;
    std::lock_guard<std::mutex> lk(mtx_);
    for (auto &e : entries_)
    {
        if (e.id == id)
        {
            e.active.store(false, std::memory_order_relaxed);
            break;
        }
    }
    // 让工作线程尽快清理
    cv_.notify_one();
}

void RedisLeaseService::run_()
{
    using namespace std::chrono;
    const auto min_interval = milliseconds(200); // 轮询最小间隔
    while (running_)
    {
        std::unique_lock<std::mutex> lk(mtx_);
        if (entries_.empty())
        {
            cv_.wait_for(lk, seconds(1));
        }
        else
        {
            auto now = steady_clock::now();

            // 复制需要续期的条目，避免持锁执行 Redis
            std::list<Entry> due;
            for (auto it = entries_.begin(); it != entries_.end();)
            {
                if (!it->active.load(std::memory_order_relaxed))
                { // 不再续期，移除
                    it = entries_.erase(it);
                    continue;
                }

                if (now >= it->next_tick) // 到期需要续期
                {
                    SPDLOG_LOGGER_TRACE(logger_, "LeaseService: renewing key {}", it->key);
                    // 使用 emplace_back() 构造副本
                    auto &due_entry = due.emplace_back();
                    due_entry.id = it->id;
                    due_entry.redis = it->redis;
                    due_entry.key = it->key;
                    due_entry.value = it->value;
                    due_entry.ttl_ms = it->ttl_ms;
                    due_entry.next_tick = it->next_tick;
                    due_entry.active.store(it->active.load(std::memory_order_relaxed), std::memory_order_relaxed);

                    // 下次续期时间
                    it->next_tick = now + milliseconds(std::max(100, it->ttl_ms / 2));
                }
                ++it;
            }

            // 计算下一次醒来的时间
            auto next_tp = now + seconds(1);
            for (auto &e : entries_)
                next_tp = std::min(next_tp, e.next_tick);

            auto wait_dur = std::max(min_interval, duration_cast<milliseconds>(next_tp - now));
            lk.unlock();

            // 执行续期
            for (auto &e : due)
            {
                if (!e.active.load(std::memory_order_relaxed))
                    continue;
                static const std::string lua = R"(
                        if redis.call('get', KEYS[1]) == ARGV[1] then
                            return redis.call('pexpire', KEYS[1], ARGV[2])
                        else
                            return 0
                        end
                    )";

                try
                {
                    e.redis->eval<long long>(lua, {e.key}, {e.value, std::to_string(e.ttl_ms)});
                }
                catch (const std::exception &ex)
                {
                    if (logger_)
                        SPDLOG_LOGGER_ERROR(logger_, "LeaseService extend error for key {}: {}", e.key, ex.what());
                }
            }

            // 进入下一轮
            std::unique_lock<std::mutex> lk2(mtx_);
            cv_.wait_for(lk2, wait_dur);
        }
    }
}