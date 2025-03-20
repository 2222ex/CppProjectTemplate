#include "logger.h"

Logger::Logger()
{
    auto max_size = 1048576 * 10;
    auto max_files = 200;
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("log.txt", max_size, max_files);
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    rootLogger = std::make_shared<spdlog::logger>("ex", spdlog::sinks_init_list {file_sink, console_sink});
    rootLogger->set_level(spdlog::level::trace);
    rootLogger->set_pattern("[%H:%M:%S %z][%t][%^%l%$]: %v"); // (%@)
    rootLogger->flush_on(spdlog::level::trace);
}

Logger &Logger::getInstance()
{
    static Logger instance;
    return instance;
}

std::shared_ptr<spdlog::logger> Logger::Log()
{
    return getInstance().rootLogger;
}

std::shared_ptr<spdlog::logger> Logger::getThreadLogger(std::string thread_name)
{
    if (loggers.count(thread_name))
        return loggers[thread_name];

    auto max_size = 1048576 * 10;
    auto max_files = 200;
    auto logger = spdlog::rotating_logger_mt(thread_name, "log/" + thread_name + ".txt", max_size, max_files);
    logger->set_pattern("[%H:%M:%S %z][" + thread_name + "][%l]: %v"); // (%@)
    logger->flush_on(spdlog::level::trace);

    loggers[thread_name] = logger;

    return logger;
}

std::shared_ptr<spdlog::logger> Logger::getNamedLogger(std::string &&logger_name)
{
    return getInstance().getThreadLogger(logger_name);
}
