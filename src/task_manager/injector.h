#ifndef INJECTOR_H
#define INJECTOR_H

#include "../base/singleton.h"
#include "../base/stdafx.h"

#include "../base/logger.h"

#include "tchar.h"

class Injector
{
private:
    /* data */

public:
    int InjectQueueUserAPC(PCWSTR pszLibFile, DWORD dwProcessId, std::string &errMsg);
    int InjectUseCreateRemoteThread(LPCTSTR pszLibFile, DWORD dwPID, std::string &errMsg);
    int UnLoadLibrary(LPCTSTR szDllName, DWORD dwPID, std::string &errMsg);
    DWORD FindProcessId(LPCTSTR szProcessName);

    Injector();
    ~Injector() = default;
};

class InjectorSingleton : public Singleton<Injector, true>
{
};

#endif