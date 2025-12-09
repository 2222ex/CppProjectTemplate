#include "main_module.h"

#include "utils/data_transform_util.h"
#include "utils/pattern_util.h"

MainModule::MainModule(/* args */)
{
    log = Logger::getLogger("main_module", true);
}

bool MainModule::InitClient(std::string &err_msg)
{
    auto f1 = [this](uint64_t addr)
    {
        hook_Work.InstallHook("work", (LPVOID) addr, FuncType::cdeclcall);
        hook_Work.AddHook(
            "work1",
            [this](auto &next, int work_count)
            {
                SPDLOG_LOGGER_INFO(log, "hook work1 work_count: {}", work_count);
                return next(work_count + 1);
            });

        return true;
    };

    // f1((uint64_t) GetProcAddress(hModule, "work"));

    pattern_map = {
        // {"Work", {"53 8B DC 83 EC ?? 83 E4 ?? 83 C4 ?? 55 8B 6B ?? 89 6C 24 ?? 8B EC 6A ?? 68 ?? ?? ?? ?? 64 A1 ?? ?? ?? ?? 50 53 81 EC ?? ?? ?? ?? A1 ?? ?? ?? ?? 33 C5 89 45 ?? 50 8D 45 ?? 64 A3 ?? ?? ?? ?? 68", 0, f1}}

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
