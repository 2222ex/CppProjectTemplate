#ifndef AUTO_OPEN_CRATE_H
#define AUTO_OPEN_CRATE_H

#include "../base/logger.h"
#include "../base/singleton.h"

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
    };

    void GetInventory();
    void OpenCrate(OpenCrateRequest openCrateRequest);
};

class AutoOpenCrateSingleton : public Singleton<AutoOpenCrate, true>
{
};

#endif