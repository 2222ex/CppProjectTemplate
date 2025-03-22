#ifndef BASE_MODULE_H
#define BASE_MODULE_H

#include "../base/logger.h"
#include "../base/stdafx.h"

#include <Psapi.h>

class BaseModule
{
private:

public:
    DWORD_PTR base, size, end;
    HMODULE hModule;
    MODULEINFO miModule;
    bool isInit;

    BaseModule(/* args */)
    {
        isInit = false;
    }

    ~BaseModule()
    {
    }

    struct HookInfo
    {
        LPVOID pTarget;
        LPVOID pDetour;
        LPVOID *ppOriginal;
    };
    std::map<std::string, HookInfo> hookInfoMap;

    bool InitModuleInfo(std::string moduleName)
    {
        if (moduleName.empty())
        {
            hModule = GetModuleHandleA(NULL);
        }
        else
        {
            hModule = GetModuleHandle(moduleName.c_str());
            if (hModule == NULL)
            {
                Logger::Log()->error("{} GetModuleHandle err", moduleName);
                return false;
            }
        }

        base = (DWORD_PTR) hModule;
        size = PIMAGE_NT_HEADERS(base + (DWORD_PTR) PIMAGE_DOS_HEADER(base)->e_lfanew)->OptionalHeader.SizeOfImage;
        end = base + size - 1;

        if (GetModuleInformation(GetCurrentProcess(), hModule, &miModule, sizeof(MODULEINFO)) == false)
        {
            Logger::Log()->error("{} GetModuleInformation err", moduleName);
            return false;
        }

        isInit = true;
        return true;
    }
};

#endif