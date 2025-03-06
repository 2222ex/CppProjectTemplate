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

    typedef void(__fastcall *pUseTool)(__int64 a1, char*  a2, char*  a3);
    pUseTool oUseTool;
    static void MyUseTool(__int64 a1, char* a2, char* a3);

    typedef PVOID (*pCSInventoryManager)();
    pCSInventoryManager GetCSInventoryManager;

    PVOID MyGetLocalCSInventory();
    __int64 MyGetItemVectorItem(int i);
    int MyGetItemVectorCount();
    void MyGetItemVectorInfo();
    

    typedef void(__fastcall *pDumpInventoryToConsole)(PVOID a1, bool bRoot);
    pDumpInventoryToConsole DumpInventoryToConsole;

    bool InitClient();
    bool Detach();
};

class ClientModuleSingleton : public Singleton<ClientModule, true>
{
};

#endif