#ifndef AUTO_OPEN_CRATE_H
#define AUTO_OPEN_CRATE_H

#include "../base/logger.h"
#include "../base/singleton.h"
#include <nlohmann/json.hpp>

class AutoOpenCrate
{
private:
    /* data */

public:
    AutoOpenCrate(/* args */);
    ~AutoOpenCrate() = default;

    struct Item
    {
        uintptr_t CEconItemView_item;
        uint64_t item_id;
        std::string valve_def_name;
    };

    std::vector<Item> inventory;

    struct OpenCrateRequest
    {
        std::string crate_name;
        int count;
        bool is_need_tool;
    };
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(OpenCrateRequest, crate_name, count, is_need_tool)

    struct OpenCrateResult
    {
        std::string msg;
        bool is_success;
    };
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(OpenCrateResult, msg, is_success)

    void GetInventory();
    void OpenCrate(OpenCrateRequest openCrateRequest, OpenCrateResult &openCrateResult);

    struct CrateItemInfo
    {
        std::string name;
        std::string valve_def_name;
        std::string correct_key_name;
        bool is_need_tool;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(CrateItemInfo, name, valve_def_name, correct_key_name, is_need_tool)
    };
    void DumpCrateInfo();
};

class AutoOpenCrateSingleton : public Singleton<AutoOpenCrate, true>
{
};

#endif