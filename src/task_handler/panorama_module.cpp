#include "panorama_module.h"

#include "../base/logger.h"
#include <MinHook.h>

PanoramaModule::PanoramaModule(/* args */)
{
}

// a1: UiEnginePointer
// a2: CUIPanel* contextPanel
// a3: scriptSouce
// a4: originFile
// a5: line
void PanoramaModule::MyRunVtsScript(PVOID UiEnginePointer, PVOID contextPanel, const char *lpszVtsContent, const char *lpszVtsFilePath, PVOID line)
{
    // Logger::Log()->info("PanoramaModule::MyRunVtsScript");

    // Logger::Log()->info("UiEnginePointer: {:#x}", (uintptr_t) UiEnginePointer);
    // Logger::Log()->info("contextPanel: {:#x}", (uintptr_t) contextPanel);
    // Logger::Log()->info("lpszVtsFilePath: {}", lpszVtsFilePath);

    PanoramaModuleSingleton::instance().oRunVtsScript(UiEnginePointer, contextPanel, lpszVtsContent, lpszVtsFilePath, line);
}

bool PanoramaModule::InitPanorama()
{
    // hookInfoMap["RunScript"] = {(LPVOID) (base + 0xA7300), &PanoramaModule::MyRunVtsScript, reinterpret_cast<LPVOID *>(&oRunVtsScript)};

    return true;
}
