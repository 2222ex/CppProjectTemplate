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
    static std::shared_ptr<spdlog::logger> Log();
    static std::shared_ptr<spdlog::logger> getLogger(const std::string &logger_name, bool to_console = false);

private:
    Logger();
    Logger(Logger const &) = delete;
    Logger &operator=(Logger const &) = delete;

    inline static std::mutex logger_mutex;
};

#endif // LOGGER_H
