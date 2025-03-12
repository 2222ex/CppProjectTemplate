#include "logger.h"

Logger::Logger()
{
    auto max_size = 1048576 * 10;
    auto max_files = 200;
    rootLogger = spdlog::rotating_logger_mt("Topo", "task_handler.txt", max_size, max_files);
    rootLogger->set_level(spdlog::level::trace);
    rootLogger->set_pattern("[%H:%M:%S %z][%l]: %v"); // (%@)
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
