#include "client_module.h"

#include "utils/data_transform_util.h"
#include "utils/pattern_util.h"

#include "MinHook.h"

ClientModule::ClientModule(/* args */)
{
}

bool ClientModule::InitClient(std::string &err_msg)
{
    auto f1 = [this](uint64_t addr)
    {
        return true;
    };

    pattern_map = {
        {"CSInventoryManager", {"", 0, f1}}

    };

    for (auto pair : pattern_map)
    {
        Logger::Log()->info("init_{}", pair.first);
        uintptr_t pattern_addr = search_pattern_in_module(miModule, hexstring2shorts(pair.second.pattern));
        if (pattern_addr == 0)
        {
            err_msg = fmt::format("{} pattern not found!", pair.first);
            return false;
        }
        int offset = pair.second.offset;

        Logger::Log()->info("pattern_addr + offset: {:#x}", pattern_addr + offset);

        if (pair.second.func(pattern_addr + offset) == false)
        {
            err_msg = fmt::format("init {} failed", pair.first);
            return false;
        }
    }

    return false;
}
