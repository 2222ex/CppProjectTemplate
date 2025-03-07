#include "auto_open_crate.h"

#include "client_module.h"

AutoOpenCrate::AutoOpenCrate(/* args */)
{
}

void AutoOpenCrate::GetInventory()
{
    Logger::Log()->info("AutoOpenCrate::GetInventory");

    auto &client = ClientModuleSingleton::instance();
    auto inventory_count = client.MyGetItemVectorCount();

    for (size_t i = 0; i < inventory_count; i++)
    {
        Item item = {};

        item.CEconItemView_item = client.MyGetItemVectorItem(i);
        Logger::Log()->info("CEconItemView_item: {:#x}", item.CEconItemView_item);

        item.item_id = client.GetCEconItemViewItemId(item.CEconItemView_item);
        Logger::Log()->info("item_id: {}", item.item_id);

        item.valve_def_name = client.GetCEconItemViewValveDefName(item.CEconItemView_item);
        Logger::Log()->info("valve_def_name: {}", item.valve_def_name);

        inventory.push_back(std::move(item));
    }
}

// 一次开一种箱子
void AutoOpenCrate::OpenCrate(OpenCrateRequest openCrateRequest)
{
    Logger::Log()->info("AutoOpenCrate::OpenCrate");

    // std::map<std::string, std::unordered_set<std::string>> crate_key_map = {};

    auto &client = ClientModuleSingleton::instance();

    std::string correct_key_name;

    Item crate_item = {0, 0, ""};
    // 找到一个对应的箱子
    for (auto &item : inventory)
    {
        if (item.valve_def_name == openCrateRequest.crate_name)
        {
            crate_item = item;
        }
    }

    if (crate_item.item_id == 0)
    {
        // 没有找到这个箱子
        Logger::Log()->info("Crate not found: {}", openCrateRequest.crate_name);
        return;
    }

    // 找到能开这个箱子的钥匙的名字
    for (auto &item : inventory)
    {
        if (client.IsItemCanOpenCrate(item.CEconItemView_item, crate_item.CEconItemView_item, 4))
        {
            correct_key_name = item.valve_def_name;
            break;
        }
    }

    // TODO: 判断是否是纪念包 不需要钥匙的箱子
    if (correct_key_name.empty())
    {
        return;
    }

    Logger::Log()->info("Found correct_key_name: {}", correct_key_name);

    // 计算所需的钥匙和箱子在库存中共有多少个
    std::vector<uint64_t> key_ids;
    std::vector<uint64_t> crate_ids;
    for (auto &item : inventory)
    {
        if (item.valve_def_name == openCrateRequest.crate_name)
        {
            crate_ids.push_back(item.item_id);
        }
        else if (item.valve_def_name == correct_key_name)
        {
            key_ids.push_back(item.item_id);
        }
    }
    if (crate_ids.size() < openCrateRequest.count)
    {
        Logger::Log()->info("no enough crates,current: {},need: {}", crate_ids.size(), openCrateRequest.count);
        return;
    }
}
