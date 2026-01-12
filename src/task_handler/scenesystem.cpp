#include "scenesystem.h"

#include "parse_render_command.h"

ScenesystemModule::ScenesystemModule()
{
}

ScenesystemModule::~ScenesystemModule()
{
}

void ScenesystemModule::Init()
{
    // SPDLOG_LOGGER_INFO(logger, "ScenesystemModule::Init - Hooking AddCommandBufferToQueue");
    // hook2.InstallHook("AddCommandBufferToQueue", (LPVOID) (this->base + 0xB8510), FuncType::fastcall);
    // hook2.AddHook(
    //     "AddCommandBufferToQueue_Log",
    //     [this](auto &next, uint64_t pRenderDevice, uint64_t a2)
    //     {
    //         uint64_t pCmdBuffer = *reinterpret_cast<uint64_t *>(a2 + 0x20);
    //         if (pCmdBuffer)
    //         {
    //             SPDLOG_LOGGER_INFO(logger, "======== AddCommandBufferToQueue ========");
    //             ParseAllRenderCmdBlocks(reinterpret_cast<void *>(pCmdBuffer));
    //         }

    //         next(pRenderDevice, a2);
    //     });

    hook3.InstallHook("CSceneSystem_RenderLayerDrawList", (LPVOID) (this->base + 0x108300), FuncType::fastcall);
    hook3.AddHook(
        "CSceneSystem_RenderLayerDrawList_Log",
        [this](auto &next, uint64_t pSceneSystem, uint64_t pViewDrawListRef, uint64_t pLayer, uint64_t a4, unsigned int nBatchFlags, uint64_t pDrawListArray)
        {
            next(pSceneSystem, pViewDrawListRef, pLayer, a4, nBatchFlags, pDrawListArray);
            std::string pLayerName;
            if (pLayer)
            {
                pLayerName = reinterpret_cast<char *>(pLayer + 1208);
            }
            if (pLayerName != "Dynamic Opaque Forward")
            {
                return;
            }

            SPDLOG_LOGGER_INFO(logger, "======== CSceneSystem_RenderLayerDrawList: pLayerName='{}' ========", pLayerName);
            uint32_t nBatchCount = *(uint32_t *) (pLayer + 3064);

            char *pBatch = **reinterpret_cast<char ***>(pLayer + 3072);
            void *cmd_buffer = *reinterpret_cast<char **>(pBatch + 0x28) + 0x20;
            ParseAllRenderCmdBlocks(*reinterpret_cast<void **>(cmd_buffer));
        });
}
