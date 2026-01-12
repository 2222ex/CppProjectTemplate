#include "rendersystemdx11_module.h"

#include "parse_render_command.h"

Rendersystemdx11Module::Rendersystemdx11Module(/* args */)
{
}

Rendersystemdx11Module::~Rendersystemdx11Module()
{
}

void Rendersystemdx11Module::Init()
{
    // 方式1: Hook RenderCommandDispatcher (单个命令缓冲区处理)
    // hook.InstallHook("RenderCommandDispatcher", (LPVOID) (this->base + 0x5CE20), FuncType::fastcall);
    // hook.AddHook(
    //     "RenderCommandDispatcher_Log",
    //     [this](auto &next, uint64_t a1, uint64_t a2, uint8_t a3)
    //     {
    //         ParseAllRenderCmdBlocks(*reinterpret_cast<void **>(a2 + 0x20));
    //         return next(a1, a2, a3);
    //     });

    // 方式2: Hook CRenderThread_EnqueueCommandBuffers (批量命令缓冲区入队)
    // 签名: void __fastcall CRenderThread_EnqueueCommandBuffers(
    //           const __m128i* pRenderThread,  // RCX
    //           __int64** ppCmdBuffers,         // RDX - 指针数组，每项8字节
    //           int nBufferCount)               // R8
    /*
    hook.InstallHook("CRenderThread_EnqueueCommandBuffers", (LPVOID) (this->base + 0x5AF80), FuncType::fastcall);
    hook.AddHook(
        "CRenderThread_EnqueueCommandBuffers_Log",
        [this](auto &next, uint64_t pRenderThread, uint64_t ppCmdBuffers, int nBufferCount)
        {
            SPDLOG_LOGGER_INFO(logger, "======== CRenderThread_EnqueueCommandBuffers: pRenderThread=0x{:016X}, nBufferCount={} ========", pRenderThread, nBufferCount);

            // ppCmdBuffers 是 __int64** 类型，每项是一个指向命令缓冲区结构的指针
            // 命令缓冲区结构: +0x00 = 指向命令数据的指针, +0x08 = 标志字节
            __int64 **pBufferArray = reinterpret_cast<__int64 **>(ppCmdBuffers);

            for (int i = 0; i < nBufferCount; i++)
            {
                __int64 *pCmdBufferStruct = pBufferArray[i];
                if (pCmdBufferStruct)
                {
                    // pCmdBufferStruct[0] = 命令数据指针
                    // *((uint8_t*)pCmdBufferStruct + 8) = 标志
                    void *pCmdData = reinterpret_cast<void *>(pCmdBufferStruct[0]);
                    uint8_t flags = *reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(pCmdBufferStruct) + 8);

                    SPDLOG_LOGGER_INFO(logger, "  Buffer[{}]: pStruct=0x{:016X}, pCmdData=0x{:016X}, flags=0x{:02X}", i, (uintptr_t) pCmdBufferStruct, (uintptr_t) pCmdData, flags);

                    if (pCmdData)
                    {
                        ParseAllRenderCmdBlocks(pCmdData);
                    }
                }
            }

            return next(pRenderThread, ppCmdBuffers, nBufferCount);
        });
    */

    // 方式3 (可选): Hook CRenderDeviceDx11::Enqueue (更上层，在过滤空指针之前)
    // 签名: void __fastcall CRenderDeviceDx11::Enqueue(
    //           __int64 pRenderDevice,     // RCX
    //           __int64* pCmdBufferArray,  // RDX - 每项16字节 (指针+标志)
    //           unsigned int nInputCount)  // R8
    //
    // 输入数组结构 (每项16字节):
    //   +0x00: __int64 pDisplayList  - 指向显示列表/命令缓冲区的指针
    //   +0x08: uint8_t flags         - 标志位
    //
    // 显示列表结构:
    //   +0x00: void* pCmdData        - 指向实际命令数据的指针
    //   ...
    hook.InstallHook("CRenderDeviceDx11_Enqueue", (LPVOID) (this->base + 0x2FC30), FuncType::fastcall);
    hook.AddHook(
        "CRenderDeviceDx11_Enqueue_Log",
        [this](auto &next, uint64_t pRenderDevice, uint64_t pCmdBufferArray, uint32_t nInputCount)
        {
            // is_async = [pRenderDevice + 0x40] + 0x20
            char *is_async = reinterpret_cast<char *>(*reinterpret_cast<uintptr_t *>(pRenderDevice + 0x40) + 0x20);
            *is_async = 0;

            // SPDLOG_LOGGER_INFO(logger, "======== CRenderDeviceDx11::Enqueue: pDevice=0x{:016X}, nCount={} ========", pRenderDevice, nInputCount);

            // // pCmdBufferArray 是 __int64* 类型
            // // 每项16字节 (2个QWORD): [pDisplayList][flags]
            // // pDisplayList 指向显示列表结构，其 +0x00 处是实际命令数据指针
            // __int64 *pArray = reinterpret_cast<__int64 *>(pCmdBufferArray);

            // for (uint32_t i = 0; i < nInputCount; i++)
            // {
            //     // 每项占用2个QWORD (16字节)
            //     __int64 pDisplayList = pArray[i * 2];                    // 第一个QWORD: 显示列表指针
            //     uint8_t flags = static_cast<uint8_t>(pArray[i * 2 + 1]); // 第二个QWORD的低字节: 标志

            //     SPDLOG_LOGGER_INFO(logger, "  Entry[{}]: pDisplayList=0x{:016X}, flags=0x{:02X}", i, pDisplayList, flags);

            //     if (pDisplayList)
            //     {
            //         // 显示列表结构的 +0x00 处是实际命令数据指针
            //         void *pCmdData = *reinterpret_cast<void **>(pDisplayList + 0x20);
            //         SPDLOG_LOGGER_INFO(logger, "    -> pCmdData=0x{:016X}", (uintptr_t) pCmdData);

            //         if (pCmdData)
            //         {
            //             ParseAllRenderCmdBlocks(pCmdData);
            //         }
            //     }
            // }

            return next(pRenderDevice, pCmdBufferArray, nInputCount);
        });
}
