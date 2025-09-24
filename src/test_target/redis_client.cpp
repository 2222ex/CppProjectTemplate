#include "redis_client.h"

bool RedisClient::init()
{
    auto redis_log = Logger::getLogger("redis", true);
    try
    {
        sw::redis::ConnectionOptions connection_options;
        connection_options.host = "r-jianshitu-server.redis.rds.aliyuncs.com";
        connection_options.port = 6379;
        connection_options.password = "EfYaQ5lY%$hHfe&P";
        connection_options.db = 0;
        connection_options.connect_timeout = std::chrono::milliseconds(1000);
        connection_options.socket_timeout = std::chrono::milliseconds(1000);

        redis_ = std::make_unique<sw::redis::Redis>(connection_options);
        return true;
    }
    catch (const std::exception &e)
    {
        SPDLOG_LOGGER_INFO(redis_log, "[redis] Redis error: {}", e.what());
    }
    return false;
}
