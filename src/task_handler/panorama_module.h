#ifndef PANORAMA_MODULE_H
#define PANORAMA_MODULE_H

#include "../base/singleton.h"
#include "base_module.h"

class PanoramaModule : public BaseModule
{
private:
    /* data */

public:
    PanoramaModule(/* args */);
    ~PanoramaModule() = default;

    bool InitPanorama();

    typedef void(__fastcall *pRunVtsScript)(PVOID p1, PVOID p2, const char *lpszVtsContent, const char *lpszVtsFilePath, PVOID p5);
    pRunVtsScript oRunVtsScript;
    static void MyRunVtsScript(PVOID p1, PVOID p2, const char *lpszVtsContent, const char *lpszVtsFilePath, PVOID p5);
};

class PanoramaModuleSingleton : public Singleton<PanoramaModule, true>
{
};

#endif