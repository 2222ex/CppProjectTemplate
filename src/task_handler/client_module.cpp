#include "client_module.h"

#include "MinHook.h"

ClientModule::ClientModule(/* args */)
{
}

// a1: CEconTool_CrateKey* this
// a2: C_EconItemView* key
// a3: C_EconItemView* crate
void ClientModule::MyUseTool(__int64 a1, char *key, char *crate)
{
    Logger::Log()->info("ClientModule::MyUseTool");

    Logger::Log()->info("a1: {:#x}, key: {}, crate: {}", a1, key, crate);

    ClientModuleSingleton::instance().oUseTool(a1, key, crate);
}

PVOID ClientModule::MyGetLocalCSInventory()
{
    // search string: CCSGO_HudRosettaSelector and look down
    return (PVOID) * reinterpret_cast<DWORD_PTR *>(reinterpret_cast<DWORD_PTR>(GetCSInventoryManager()) + 0x3D1A0);
}

__int64 ClientModule::MyGetItemVectorItem(int i)
{
    // [[[rcx+0x28] + rbp*8]]
    return *(uint64_t *) (*(uint64_t *) ((uint64_t) MyGetLocalCSInventory() + 40) + 8 * i);
    // return (*(uint64_t *) ((uint64_t) MyGetLocalCSInventory() + 40) + 8 * i);
}

int ClientModule::MyGetItemVectorCount()
{
    return *((unsigned int *) ((uintptr_t) MyGetLocalCSInventory() + 0x20));
}

void ClientModule::MyGetItemVectorInfo()
{
    Logger::Log()->info("MyGetItemVectorInfo");

    int count = MyGetItemVectorCount();
    Logger::Log()->info("count: {}", count);

    for (size_t i = 0; i < count; i++)
    {
        __int64 itemAddress = MyGetItemVectorItem(i);
        Logger::Log()->info("itemAddress: {:#x}", itemAddress);

        uint64_t itemId = (*(__int64(__fastcall **)(__int64))(*(uint64_t *) MyGetItemVectorItem(i) + 120i64))(MyGetItemVectorItem(i));
        Logger::Log()->info("itemId: {}", itemId);
    }
}

bool ClientModule::InitClient()
{
    GetCSInventoryManager = reinterpret_cast<pCSInventoryManager>(base + 0x5189B0);

    //
    DumpInventoryToConsole = reinterpret_cast<pDumpInventoryToConsole>(base + 0x5197b0);

    // DumpInventoryToConsole = reinterpret_cast<pDumpInventoryToConsole>(base + 0xc2d660);

    Logger::Log()->info("GetCSInventoryManager(): {}", fmt::ptr(GetCSInventoryManager()));

    // DumpInventoryToConsole(*reinterpret_cast<CSInventoryManager *>(GetCSInventoryManager()), false);

    // search string: UseTool
    hookInfoMap["UseTool"] = {(LPVOID) (base + 0xB4DB60), &ClientModule::MyUseTool, reinterpret_cast<LPVOID *>(&oUseTool)};

    return false;
}

bool ClientModule::Detach()
{
    for (auto &pair : hookInfoMap)
    {
        auto hookInfo = pair.second;

        if (MH_DisableHook(hookInfo.pTarget) != MH_OK)
        {
            Logger::Log()->error("MH_DisableHook {} failed", pair.first);
            continue;
        }
    }
    return true;
}
