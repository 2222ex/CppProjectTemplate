#include "auto_open_crate.h"

#include "client_module.h"

AutoOpenCrate::AutoOpenCrate(/* args */)
{
}

void AutoOpenCrate::GetInventory()
{
    Logger::Log()->info("AutoOpenCrate::GetInventory");

    inventory.clear();

    auto &client = ClientModuleSingleton::instance();
    auto inventory_count = client.GetItemVectorCount();

    Logger::Log()->info("inventory_count: {}", inventory_count);

    for (size_t i = 0; i < inventory_count; i++)
    {
        Item item = {};

        item.CEconItemView_item = client.MyGetItemVectorItem(i);
        Logger::Log()->info("CEconItemView_item: {:#x}", item.CEconItemView_item);

        item.item_id = client.GetCEconItemViewItemId(item.CEconItemView_item);
        Logger::Log()->info("item_id: {}", item.item_id);

        if (item.item_id == 17293822569102708641ULL || item.item_id == 17293822569110896676)
        {
            continue;
        }

        item.valve_def_name = client.GetCEconItemViewValveDefName(item.CEconItemView_item);
        Logger::Log()->info("valve_def_name: {}", item.valve_def_name);

        inventory.push_back(std::move(item));
    }
}

// 一次开一种箱子
void AutoOpenCrate::OpenCrate(OpenCrateRequest openCrateRequest, OpenCrateResult &openCrateResult)
{
    GetInventory();

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
            Logger::Log()->info("Crate found: {}", item.valve_def_name);
            break;
        }
    }

    if (crate_item.item_id == 0)
    {
        // 没有找到这个箱子

        openCrateResult.msg = fmt::format("Crate not found: {}", openCrateRequest.crate_name);
        openCrateResult.is_success = false;
        return;
    }
    if (openCrateRequest.is_need_tool)
    {

        // 找到能开这个箱子的钥匙的名字
        for (auto &item : inventory)
        {
            if (client.IsItemCanOpenCrate(item.CEconItemView_item, crate_item.CEconItemView_item, 4))
            {
                correct_key_name = item.valve_def_name;
                break;
            }
        }

        if (correct_key_name.empty())
        {
            Logger::Log()->info("correct_key_name empty");

            openCrateResult.msg = fmt::format("correct_key_name empty");
            openCrateResult.is_success = false;
            return;
        }
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
        else if (openCrateRequest.is_need_tool && item.valve_def_name == correct_key_name)
        {
            key_ids.push_back(item.item_id);
        }
    }

    if (crate_ids.size() < openCrateRequest.count)
    {
        openCrateResult.msg = fmt::format("no enough crates,current: {},need: {}", crate_ids.size(), openCrateRequest.count);
        openCrateResult.is_success = false;
        return;
    }
    else if (openCrateRequest.is_need_tool && key_ids.size() < openCrateRequest.count)
    {
        openCrateResult.msg = fmt::format("no enough keys,current: {},need: {}", key_ids.size(), openCrateRequest.count);
        openCrateResult.is_success = false;
        return;
    }

    for (size_t i = 0; i < openCrateRequest.count; i++)
    {
        std::string keyStr = std::to_string(key_ids[i]);
        std::string crateStr = std::to_string(crate_ids[i]);

        const char *szKeyId = keyStr.c_str();
        const char *szCrateId = crateStr.c_str();
        Logger::Log()->info("key_ids[i]: {}, crate_ids[i]: {}", key_ids[i], crate_ids[i]);
        Logger::Log()->info("szKeyId: {}, szCrateId: {}", szKeyId, szCrateId);
        client.MyUseTool((uintptr_t) client.useToolUnkParam1, (char *) szKeyId, (char *) szCrateId);
        std::this_thread::sleep_for(std::chrono::seconds(6));
    }

    openCrateResult.msg = fmt::format("success");
    openCrateResult.is_success = true;

    return;
}
