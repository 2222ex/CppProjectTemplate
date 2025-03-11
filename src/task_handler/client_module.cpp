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
    Logger::Log()->info("ClientModule::MyUseTool");

    Logger::Log()->info("a1: {:#x}, key: {}, crate: {}", a1, key, crate);

    ClientModuleSingleton::instance().oUseTool(a1, key, crate);
}

PVOID ClientModule::MyGetLocalCSInventory()
{
    Logger::Log()->trace("ClientModule::MyGetLocalCSInventory");

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
    Logger::Log()->info("GetItemVectorInfo");

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
    typedef uintptr_t(__fastcall * pGetNamePointer)(uintptr_t item);
    pGetNamePointer GetNamePointer = reinterpret_cast<pGetNamePointer>(base + 0xc81e40);
    uintptr_t temp = GetNamePointer(CEconItemView_item);
    return *(char **) (temp + 496);
}

bool ClientModule::init_localCSInventory()
{
    Logger::Log()->info("ClientModule::init_localCSInventory");

    // search string: CCSGO_HudRosettaSelector and look down
    uintptr_t pattern_addr = search_pattern_in_module(miModule, hexstring2shorts("f2 0f 11 4c 24 ?? e8 ?? ?? ?? ?? 48 8b 88 ?? ?? ?? ??"));
    if (pattern_addr == 0)
    {
        return false;
    }

    int offset = 14;
    Logger::Log()->info("pattern_addr + offset: {:#x}", pattern_addr + offset);

    uint32_t val = *reinterpret_cast<uint32_t *>(pattern_addr + offset);
    Logger::Log()->info("val: {:#x}", val);

    localCSInventory = *reinterpret_cast<uintptr_t *>((reinterpret_cast<uintptr_t>(GetCSInventoryManager()) + val));
    Logger::Log()->info("localCSInventory: {:#x}", localCSInventory);

    return true;
}

bool ClientModule::InitClient()
{
    GetCSInventoryManager = reinterpret_cast<pCSInventoryManager>(base + 0x5189B0);

    init_localCSInventory();

    // search string: "      %s (ID %llu) at backpack slot %d\n" or "(CLIENT) Inventory:\n"
    DumpInventoryToConsole = reinterpret_cast<pDumpInventoryToConsole>(base + 0x5197b0);

    // search string: GetChosenActionItemsCount ,找到这个函数的返回值，跟踪这个返回值
    // IsItemCanOpenCrate = reinterpret_cast<pIsItemCanOpenCrate>(base + 0xc8ce30);
    IsItemCanOpenCrate = (pIsItemCanOpenCrate) (base + 0xc8ce30);
    if (IsItemCanOpenCrate == nullptr)
    {
        Logger::Log()->error("IsItemCanOpenCrate: null");
        return false;
    }
    else
    {
        Logger::Log()->info("IsItemCanOpenCrate: {}", fmt::ptr(IsItemCanOpenCrate));
    }

    // DumpInventoryToConsole = reinterpret_cast<pDumpInventoryToConsole>(base + 0xc2d660);

    Logger::Log()->info("GetCSInventoryManager(): {}", fmt::ptr(GetCSInventoryManager()));

    // DumpInventoryToConsole(*reinterpret_cast<CSInventoryManager *>(GetCSInventoryManager()), false);

    // search string: UseTool
    hookInfoMap["UseTool"] = {(LPVOID) (base + 0xB4DB60), &ClientModule::MyUseTool, reinterpret_cast<LPVOID *>(&oUseTool)};

    return false;
}

bool ClientModule::Detach()
{
    free(useToolUnkParam1);
    return true;
}
