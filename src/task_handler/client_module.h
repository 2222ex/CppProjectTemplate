#ifndef CLIENT_MODULE_H
#define CLIENT_MODULE_H

#include "../base/logger.h"
#include "../base/singleton.h"
#include "base_module.h"

class ClientModule : public BaseModule
{
private:

public:
    ClientModule(/* args */);
    ~ClientModule() = default;

    typedef void(__fastcall *pUseTool)(uintptr_t a1, char *a2, char *a3);
    pUseTool oUseTool;
    static void MyUseTool(uintptr_t a1, char *a2, char *a3);

    typedef PVOID (*pCSInventoryManager)();
    pCSInventoryManager GetCSInventoryManager;

    PVOID MyGetLocalCSInventory();
    uintptr_t MyGetItemVectorItem(int i);
    int GetItemVectorCount();
    void GetItemVectorInfo();

    uint64_t GetCEconItemViewItemId(uintptr_t item);
    char *GetCEconItemViewValveDefName(uintptr_t CEconItemView_item);

    //
    // a3: 4
    typedef bool(__fastcall *pIsItemCanOpenCrate)(uintptr_t CEconItemView_item, uintptr_t CEconItemView_crate, unsigned int a3);
    pIsItemCanOpenCrate IsItemCanOpenCrate;

    typedef void(__fastcall *pDumpInventoryToConsole)(PVOID a1, bool bRoot);
    pDumpInventoryToConsole DumpInventoryToConsole;

    bool init_localCSInventory();
    uintptr_t localCSInventory;

    bool InitClient();
    bool Detach();

    char *useToolUnkParam1;
};

class ClientModuleSingleton : public Singleton<ClientModule, true>
{
};

#endif