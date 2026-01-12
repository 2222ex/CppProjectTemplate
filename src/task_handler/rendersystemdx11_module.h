#pragma once

#include "base/logger.h"
#include "base_module.h"
#include "hook/base_hook_template.h"

class Rendersystemdx11Module : public BaseModule
{
private:

public:
    Rendersystemdx11Module(/* args */);
    ~Rendersystemdx11Module();

    void Init();
    HookTemplate<Hook_RenderCommandDispatcher, void, uint64_t, uint64_t, uint8_t> hook;
    HookTemplate<Hook_AddCommandBufferToQueue, void, uint64_t, uint64_t> hook2;

    std::shared_ptr<spdlog::logger> logger = Logger::Log();
};

class Rendersystemdx11ModuleSingleton : public Singleton<Rendersystemdx11Module>
{
};