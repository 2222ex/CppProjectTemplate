#ifndef INJECTOR_H
#define INJECTOR_H

#include "../base/singleton.h"
#include "../base/stdafx.h"

#include "../base/logger.h"

class Injector
{
private:
    /* data */

public:
    int InjectQueueUserAPC(PCWSTR pszLibFile, DWORD dwProcessId);

    Injector(/* args */);
    ~Injector();
};

class InjectorSingleton : Singleton<Injector, true>
{
};

#endif