#pragma once

#include "base/logger.h"
#include "base_module.h"
#include "hook/base_hook_template.h"

class ScenesystemModule : public BaseModule
{
private:

public:
    ScenesystemModule(/* args */);
    ~ScenesystemModule();

    void Init();
    HookTemplate<Hook_AddCommandBufferToQueue, void, uint64_t, uint64_t> hook2;
    HookTemplate<Hook_CSceneSystem_RenderLayerDrawList, void, uint64_t, uint64_t, uint64_t, uint64_t, unsigned int, uint64_t> hook3;

    std::shared_ptr<spdlog::logger> logger = Logger::Log();
};

class ScenesystemModuleSingleton : public Singleton<ScenesystemModule>
{
};