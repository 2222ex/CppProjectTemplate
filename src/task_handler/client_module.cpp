#include "client_module.h"

#include "utils/data_transform_util.h"
#include "utils/pattern_util.h"

#include "MinHook.h"

ClientModule::ClientModule(/* args */)
{
    useToolUnkParam1 = (char *) malloc(sizeof(1024));
}

// a1: ???
// a2: char *key
// a3: char *crate
// search string: UseTool
void ClientModule::MyUseTool(uintptr_t a1, char *key, char *crate)
{
    Logger::Log()->trace("ClientModule::MyUseTool");

    Logger::Log()->trace("a1: {:#x}, key: {}, crate: {}", a1, key, crate);

    ClientModuleSingleton::instance().oUseTool(a1, key, crate);
}

PVOID ClientModule::MyGetLocalCSInventory()
{
    // Logger::Log()->trace("ClientModule::MyGetLocalCSInventory");

    // return (PVOID) * reinterpret_cast<uintptr_t *>(reinterpret_cast<uintptr_t>(GetCSInventoryManager()) + 0x3D1A0);
    return (PVOID) localCSInventory;
}

// in function: DumpInventoryToConsole
uint64_t ClientModule::MyGetItemVectorItem(int i)
{
    // [[[rcx+0x28] + rbp*8]]
    return *(uintptr_t *) (*(uintptr_t *) ((uintptr_t) MyGetLocalCSInventory() + 40) + 8 * i);
}

// in function: DumpInventoryToConsole
int ClientModule::GetItemVectorCount()
{
    return *((unsigned int *) ((uintptr_t) MyGetLocalCSInventory() + 0x20));
}

void ClientModule::GetItemVectorInfo()
{
    Logger::Log()->trace("GetItemVectorInfo");

    int count = GetItemVectorCount();
    Logger::Log()->info("count: {}", count);

    for (size_t i = 0; i < count; i++)
    {
        uintptr_t itemAddress = MyGetItemVectorItem(i);
        Logger::Log()->info("itemAddress: {:#x}", itemAddress);

        // (*(__int64(__fastcall **)(__int64))(*(uint64_t *) MyGetItemVectorItem(i) + 120i64))(MyGetItemVectorItem(i));
        uint64_t itemId = GetCEconItemViewItemId(itemAddress);
        // uint64_t itemId = (*(__int64(__fastcall **)(__int64))(*(uint64_t *) MyGetItemVectorItem(i) + 120i64))(MyGetItemVectorItem(i));
        Logger::Log()->info("itemId: {}", itemId);

        Logger::Log()->info("valve_def_name: {}", GetCEconItemViewValveDefName(itemAddress));
    }
}

// in function: DumpInventoryToConsole
uint64_t ClientModule::GetCEconItemViewItemId(uintptr_t CEconItemView_item)
{
    return (*(__int64(__fastcall **)(__int64))(*(uint64_t *) CEconItemView_item + 120i64))(CEconItemView_item);
}

// in function: DumpInventoryToConsole
char *ClientModule::GetCEconItemViewValveDefName(uintptr_t CEconItemView_item)
{
    // typedef uintptr_t(__fastcall * pGetNamePointer)(uintptr_t item);
    // pGetNamePointer GetNamePointer = reinterpret_cast<pGetNamePointer>(base + 0xc81e40);
    uintptr_t temp = GetNamePointer(CEconItemView_item);
    return *(char **) (temp + 496);
}

bool ClientModule::InitClient()
{
    auto f_init_GetCSInventoryManager = [this](uint64_t addr)
    {
        // 负数 补码
        int32_t rel32 = 0x100000000 - *reinterpret_cast<uint32_t *>(addr);
        Logger::Log()->trace("rel32: {:#x}", rel32);
        Logger::Log()->trace("addr - rel32 + 4: {:#x}", addr - rel32 + 4);

        GetCSInventoryManager = reinterpret_cast<pCSInventoryManager>(addr - rel32 + 4);
        return true;
    };

    auto f_init_localCSInventory = [this](uint64_t addr)
    {
        uint32_t val = *reinterpret_cast<uint32_t *>(addr);
        Logger::Log()->trace("val: {:#x}", val);

        localCSInventory = *reinterpret_cast<uintptr_t *>((reinterpret_cast<uintptr_t>(GetCSInventoryManager()) + val));
        if (localCSInventory == NULL)
        {
            return false;
        }
        Logger::Log()->trace("localCSInventory: {:#x}", localCSInventory);

        return true;
    };

    auto f_init_IsItemCanOpenCrate = [this](uint64_t addr)
    {
        IsItemCanOpenCrate = (pIsItemCanOpenCrate) (addr);

        return true;
    };

    auto f_init_UseTool = [this](uint64_t addr)
    {
        hookInfoMap["UseTool"] = {(LPVOID) (addr), &ClientModule::MyUseTool, reinterpret_cast<LPVOID *>(&oUseTool)};
        return true;
    };

    auto f_init_GetNamePointer = [this](uint64_t addr)
    {
        GetNamePointer = reinterpret_cast<pGetNamePointer>(addr);
        if (GetNamePointer == NULL)
        {
            return false;
        }

        return true;
    };

    pattern_map = {

        {"CSInventoryManager", {"f2 0f 11 4c 24 ?? e8 ?? ?? ?? ?? 48 8b 88 ?? ?? ?? ??", 7, f_init_GetCSInventoryManager}},

        // search string: CCSGO_HudRosettaSelector and look down
        {"localCSInventory", {"f2 0f 11 4c 24 ?? e8 ?? ?? ?? ?? 48 8b 88 ?? ?? ?? ??", 14, f_init_localCSInventory}},

        // search string: GetChosenActionItemsCount ,找到这个函数的返回值，跟踪这个返回值
        {"IsItemCanOpenCrate", {"FF 50 ?? 48 8B E8 48 85 C0 0F 84 ?? ?? ?? ?? 65 48 8B 0C 25 ?? ?? ?? ??", -0x36, f_init_IsItemCanOpenCrate}},

        // search string: UseTool
        {"UseTool", {"49 8b d8 4c 8b f9  48 85 db 0f 84 ?? ?? ?? ?? 48 89 6c 24 ??", -0x10, f_init_UseTool}},

        // in function: DumpInventoryToConsole
        {"GetNamePointer", {"48 8b 4b ?? 48 8b 5c 24 ?? 48 85 c9 75 08 33 c0", -0x78, f_init_GetNamePointer}}

    };

    for (auto pair : pattern_map)
    {
        Logger::Log()->info("init_{}", pair.first);
        uintptr_t pattern_addr = search_pattern_in_module(miModule, hexstring2shorts(pair.second.pattern));
        if (pattern_addr == 0)
        {
            Logger::Log()->error("{} pattern not found!", pair.first);
            return false;
        }
        int offset = pair.second.offset;

        Logger::Log()->info("pattern_addr + offset: {:#x}", pattern_addr + offset);

        if (pair.second.func(pattern_addr + offset) == false)
        {
            Logger::Log()->error("init {} failed", pair.first);
            return false;
        }
    }

    // search string: "      %s (ID %llu) at backpack slot %d\n" or "(CLIENT) Inventory:\n"
    DumpInventoryToConsole = reinterpret_cast<pDumpInventoryToConsole>(base + 0x5197b0);

    // DumpInventoryToConsole = reinterpret_cast<pDumpInventoryToConsole>(base + 0xc2d660);

    Logger::Log()->info("GetCSInventoryManager(): {}", fmt::ptr(GetCSInventoryManager()));

    return false;
}

bool ClientModule::Detach()
{
    free(useToolUnkParam1);
    return true;
}

// bool ClientModule::init_GetCSInventoryManager(uint64_t addr)
// {
//     GetCSInventoryManager = reinterpret_cast<pCSInventoryManager>(base + 0x5189B0);
//     return true;
// }

// bool ClientModule::init_localCSInventory(uint64_t addr)
// {
//     Logger::Log()->info("ClientModule::init_localCSInventory");

//     Logger::Log()->info("addr: {:#x}", addr);

//     uint32_t val = *reinterpret_cast<uint32_t *>(addr);
//     Logger::Log()->info("val: {:#x}", val);

//     localCSInventory = *reinterpret_cast<uintptr_t *>((reinterpret_cast<uintptr_t>(GetCSInventoryManager()) + val));
//     Logger::Log()->info("localCSInventory: {:#x}", localCSInventory);

//     return true;
// }

// bool ClientModule::init_IsItemCanOpenCrate(uint64_t addr)
// {
//     Logger::Log()->info("addr: {:#x}", addr);

//     IsItemCanOpenCrate = (pIsItemCanOpenCrate) (addr);

//     return true;
// }