#include "base/logger.h"

std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> Logger::loggers;

Logger::Logger()
{
}

std::shared_ptr<spdlog::logger> Logger::Log()
{
    return getLogger("main");
}

std::shared_ptr<spdlog::logger> Logger::getLogger(const std::string &logger_name, bool to_console)
{
    if (loggers.count(logger_name))
        return loggers[logger_name];

    auto max_size = 1048576 * 10;
    auto max_files = 20;
    std::shared_ptr<spdlog::logger> logger;
    if (to_console)
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("log/" + logger_name + ".log", max_size, max_files);
        logger = std::make_shared<spdlog::logger>(logger_name, spdlog::sinks_init_list {file_sink, console_sink});
    }
    else
    {
        logger = spdlog::rotating_logger_mt(logger_name, "log/" + logger_name + ".log", max_size, max_files);
    }

    logger->set_pattern("[%H:%M:%S %z][" + logger_name + "][%l]: %v"); // (%@)
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::trace);
    loggers[logger_name] = logger;

    return logger;
}
