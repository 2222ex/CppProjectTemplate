#include "main_module.h"

#include "utils/data_transform_util.h"
#include "utils/pattern_util.h"

#include "MinHook.h"

MainModule::MainModule(/* args */)
{
    log = Logger::getLogger("main_module", true);
}

bool MainModule::InitClient(std::string &err_msg)
{
    auto f1 = [this](uint64_t addr)
    {
        hook_Work.InstallHook("work", (LPVOID) addr);
        hook_Work.AddHook(
            "work",
            [this](auto &next, int work_count)
            {
                SPDLOG_LOGGER_INFO(log, "hook work1 work_count: {}", work_count);
                return next(work_count + 1);
            });
        hook_Work.AddHook(
            "work2",
            [this](auto &next, int work_count)
            {
                SPDLOG_LOGGER_INFO(log, "hook work2 work_count: {}", work_count);
                return next(work_count + 1);
            });
        hook_Work.AddHook(
            "work3",
            [this](auto &next, int work_count)
            {
                SPDLOG_LOGGER_INFO(log, "hook work3 work_count: {}", work_count);
                return next(work_count + 1);
            });
        return true;
    };

    pattern_map = {
        {"Work", {"89 4C 24 ?? 56 57 48 81 EC ?? ?? ?? ?? 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 84 24 ?? ?? ?? ?? 48 8D 15", 0, f1}}

    };

    for (auto pair : pattern_map)
    {
        SPDLOG_LOGGER_INFO(log, "init_{}", pair.first);
        uintptr_t pattern_addr = search_pattern_in_module(miModule, hexstring2shorts(pair.second.pattern));
        if (pattern_addr == 0)
        {
            err_msg = fmt::format("{} pattern not found!", pair.first);
            return false;
        }
        int offset = pair.second.offset;

        SPDLOG_LOGGER_INFO(log, "pattern_addr + offset: {:#x}", pattern_addr + offset);

        if (pair.second.func(pattern_addr + offset) == false)
        {
            err_msg = fmt::format("init {} failed", pair.first);
            return false;
        }
    }

    return true;
}
