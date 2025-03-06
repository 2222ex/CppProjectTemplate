#ifndef BASE_MODULE_H
#define BASE_MODULE_H

#include "../base/stdafx.h"
#include <Windows.h>

class BaseModule
{
private:

public:
    DWORD_PTR base, size, end;
    HMODULE hModule;
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
            return false;
        }

        hModule = GetModuleHandle(moduleName.c_str());
        if (hModule == NULL)
        {
            return false;
        }

        base = (DWORD_PTR) hModule;
        size = PIMAGE_NT_HEADERS(base + (DWORD_PTR) PIMAGE_DOS_HEADER(base)->e_lfanew)->OptionalHeader.SizeOfImage;
        end = base + size - 1;

        isInit = true;
        return true;
    }
};

#endif