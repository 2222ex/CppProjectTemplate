#ifndef MAIN_MODULE_H
#define MAIN_MODULE_H

#include "base/logger.h"
#include "base_module.h"

struct PatternInfo
{
    std::string pattern; // Hex pattern to search for ,e.g. "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 ??"
    int offset;          // Offset from the found pattern address, e.g. 0x10
    std::function<bool(uint64_t)> func;
};

class MainModule : public BaseModule
{
private:

public:
    MainModule(/* args */);
    ~MainModule() = default;
    std::map<std::string, PatternInfo> pattern_map;

    bool InitClient(std::string &err_msg);
};

class MainModuleSingleton : public Singleton<MainModule, true>
{
};

#endif