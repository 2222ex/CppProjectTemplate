#pragma once
#include "base/logger.h"
#include "base/stdafx.h"

#include <sw/redis++/redis++.h>

// 简单的 Redis 客户端封装
class RedisClient
{
public:
    static bool init();
    static inline std::unique_ptr<sw::redis::Redis> redis_;
};