#include "base/logger.h"

Logger::Logger()
{
}

std::shared_ptr<spdlog::logger> Logger::Log()
{
    return getLogger("main");
}

std::shared_ptr<spdlog::logger> Logger::getLogger(const std::string &logger_name, bool to_console)
{
    std::shared_ptr<spdlog::logger> logger = spdlog::get(logger_name);
    if (logger)
        return logger;

    std::lock_guard<std::mutex> lock_instance(logger_mutex);

    logger = spdlog::get(logger_name);
    if (logger)
        return logger;

    auto max_size = 1048576 * 10;
    auto max_files = 20;

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

    logger->set_pattern("[%Y-%m-%d %H:%M:%S %z][%t][%^%l%$][%s:%#:%!]: %v"); // (%@)
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::trace);

    return logger;
}
