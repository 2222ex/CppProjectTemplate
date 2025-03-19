#ifndef LOGGER_H
#define LOGGER_H

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include <memory>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

class Logger
{
public:
    static Logger &getInstance();

    static std::shared_ptr<spdlog::logger> Log();

    static std::shared_ptr<spdlog::logger> getNamedLogger(std::string &&logger_name);

private:
    Logger();
    Logger(Logger const &) = delete;
    Logger &operator=(Logger const &) = delete;
    std::shared_ptr<spdlog::logger> getThreadLogger(std::string thread_name);
    std::shared_ptr<spdlog::logger> rootLogger;
    std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> loggers;
};

#endif // LOGGER_H
