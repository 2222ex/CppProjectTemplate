#include "parse_render_command.h"

#include "base/logger.h"
#include "base/stdafx.h"

std::shared_ptr<spdlog::logger> logger = Logger::Log();

// 命令类型枚举
enum RenderCmdType : uint16_t
{
    CMD_END = 0x00,
    CMD_LINK_NEXT_BLOCK = 0x01,
    CMD_EXECUTE_BLOCK = 0x04,
    CMD_SET_THREAD_EVENT = 0x06,
    CMD_MAP_BUFFER = 0x08,
    CMD_MAP_BUFFER_MULTIPLE = 0x09,
    CMD_RENDERDOC_BEGIN = 0x0A,
    CMD_RENDERDOC_END = 0x0B,
    CMD_RENDERDOC_WINDOW = 0x0C,
    CMD_SUBMIT_DEPENDENCY = 0x0D,
    CMD_RELEASE_QUERY = 0x0E,
    CMD_WAIT_QUERY = 0x0F,
    CMD_CREATE_RESOURCE = 0x10,
    CMD_READ_BUFFER = 0x11,
    CMD_UNKNOWN_12 = 0x12,
    CMD_UNKNOWN_13 = 0x13,
    CMD_SET_VIEWPORT = 0x15,
    CMD_CLEAR_STATE = 0x16,
    CMD_OM_SET_RENDER_TARGETS = 0x17,
    CMD_CS_SET_UAV = 0x18,
    CMD_IA_SET_INPUT_LAYOUT = 0x19,
    CMD_UPDATE_SUBRESOURCE = 0x1A,
    CMD_UPDATE_SUBRESOURCE_INLINE = 0x1B,
    CMD_DRAW_INSTANCED = 0x1C,
    CMD_DRAW_INDEXED_INSTANCED = 0x1D,
    CMD_DISPATCH = 0x1E,
    CMD_DISPATCH_INDIRECT = 0x1F,
    CMD_DRAW_INSTANCED_INDIRECT = 0x20,
    CMD_DRAW_INDEXED_INDIRECT = 0x21,
    CMD_SO_SET_TARGETS = 0x22,
    CMD_UNKNOWN_24 = 0x24,
    CMD_UNKNOWN_25 = 0x25,
    CMD_IA_SET_INDEX_BUFFER = 0x26,
    CMD_SET_SHADER_WITH_CB = 0x27,
    CMD_BATCH_SHADER_RESOURCES = 0x28,
    CMD_RS_SET_SCISSOR_RECTS = 0x29,
    CMD_GS_SET_SHADER = 0x2A,
    CMD_VS_SET_SHADER = 0x2B,
    CMD_PS_SET_SHADER = 0x2C,
    CMD_IA_SET_VERTEX_BUFFER = 0x2D,
    CMD_IA_SET_VERTEX_BUFFERS = 0x2E,
    CMD_UPDATE_BUFFER = 0x2F,
    CMD_UPDATE_BUFFER_MAP = 0x30,
    CMD_CS_SET_SHADER = 0x31,
    CMD_DS_SET_SHADER = 0x32,
    CMD_HS_SET_SHADER = 0x33,
    CMD_GENERATE_MIPS = 0x34,
    CMD_OM_SET_RT_WITH_UAV = 0x35,
    CMD_UNKNOWN_36 = 0x36,
    CMD_UNKNOWN_37 = 0x37,
    CMD_HS_SET_SHADER_2 = 0x38,
    CMD_COPY_RESOURCE_TO_TEX = 0x39,
    CMD_BLIT = 0x3A,
    CMD_COPY_RESOURCE = 0x3B,
    CMD_COPY_SUBRESOURCE_REGION = 0x3C,
    CMD_UPDATE_SUBRESOURCE_REGION = 0x3D,
    CMD_PIX_BEGIN_EVENT = 0x3E,
    CMD_PIX_END_EVENT = 0x3F,
    CMD_PIX_BEGIN_EVENT_2 = 0x40,
    CMD_NVAPI_VRR_CONTROL = 0x41,
    CMD_FLUSH = 0x42,
    CMD_RELEASE_SHADER_RESOURCES = 0x43,
    CMD_RELEASE_RESOURCE = 0x44,
    CMD_SET_SWAPCHAIN_RESOURCE = 0x45,
    CMD_FLUSH_INTERNAL = 0x46,
    CMD_SETUP_RENDER_STATE = 0x47,
};

// 获取命令名称
const char *GetCmdName(uint16_t cmdType)
{
    switch (cmdType)
    {
    case CMD_END:
        return "End";
    case CMD_LINK_NEXT_BLOCK:
        return "LinkNextBlock";
    case CMD_EXECUTE_BLOCK:
        return "ExecuteBlock";
    case CMD_SET_THREAD_EVENT:
        return "SetThreadEvent";
    case CMD_MAP_BUFFER:
        return "MapBuffer";
    case CMD_MAP_BUFFER_MULTIPLE:
        return "MapBufferMultiple";
    case CMD_RENDERDOC_BEGIN:
        return "RenderDocBegin";
    case CMD_RENDERDOC_END:
        return "RenderDocEnd";
    case CMD_RENDERDOC_WINDOW:
        return "RenderDocWindow";
    case CMD_SUBMIT_DEPENDENCY:
        return "SubmitDependency";
    case CMD_RELEASE_QUERY:
        return "ReleaseQuery";
    case CMD_WAIT_QUERY:
        return "WaitQuery";
    case CMD_CREATE_RESOURCE:
        return "CRenderThreadDx11::OnReadPixels";
    case CMD_READ_BUFFER:
        return "ReadBuffer";
    case CMD_SET_VIEWPORT:
        return "SetViewport";
    case CMD_CLEAR_STATE:
        return "CContext::TID3D11DeviceContext_SetShader_<1, 0>";
    case CMD_OM_SET_RENDER_TARGETS:
        return "CContext::TID3D11DeviceContext_SetShader_<1, 3>";
    case CMD_CS_SET_UAV:
        return "CContext::TID3D11DeviceContext_SetShader_<1, 5>";
    case CMD_IA_SET_INPUT_LAYOUT:
        return "CContext::TID3D11DeviceContext_SetShader_<1, 4>";
    case CMD_UPDATE_SUBRESOURCE:
        return "unk_26";
    case CMD_UPDATE_SUBRESOURCE_INLINE:
        return "unk_27";
    case CMD_DRAW_INSTANCED:
        return "CContext::TID3D11DeviceContext_Draw_<9>_28";
    case CMD_DRAW_INDEXED_INSTANCED:
        return "CContext::TID3D11DeviceContext_DrawInstanced_<1>_29";
    case CMD_DISPATCH:
        return "CContext::TID3D11DeviceContext_DrawIndexedInstanced_<9>_30";
    case CMD_DISPATCH_INDIRECT:
        return "CContext::TID3D11DeviceContext_DrawIndexedInstanced_<9>_31";
    case CMD_DRAW_INSTANCED_INDIRECT:
        return "unk_32";
    case CMD_DRAW_INDEXED_INDIRECT:
        return "unk_33";
    case CMD_SO_SET_TARGETS:
        return "MultiDrawIndexedInstanced";
    case CMD_UNKNOWN_24:
        return "unk_36";
    case CMD_IA_SET_INDEX_BUFFER:
        return "unk_38";
    case CMD_SET_SHADER_WITH_CB:
        return "unk_39";
    case CMD_BATCH_SHADER_RESOURCES:
        return "BatchShaderResources";
    case CMD_RS_SET_SCISSOR_RECTS:
        return "CContext::TID3D11DeviceContext_RSSetScissorRects_<1>";
    case CMD_GS_SET_SHADER:
        return "unk_42";
    case CMD_VS_SET_SHADER:
        return "CContext::TID3D11DeviceContext_IASetInputLayout_<7>";
    case CMD_PS_SET_SHADER:
        return "CContext::TID3D11DeviceContext_IASetIndexBuffer_<1>";
    case CMD_IA_SET_VERTEX_BUFFER:
        return "IASetVertexBuffer";
    case CMD_IA_SET_VERTEX_BUFFERS:
        return "IASetVertexBuffers";
    case CMD_UPDATE_BUFFER:
        return "UpdateBuffer";
    case CMD_UPDATE_BUFFER_MAP:
        return "UpdateBufferMap";
    case CMD_CS_SET_SHADER:
        return "CSSetShader";
    case CMD_DS_SET_SHADER:
        return "DSSetShader";
    case CMD_HS_SET_SHADER:
        return "HSSetShader";
    case CMD_GENERATE_MIPS:
        return "GenerateMips";
    case CMD_OM_SET_RT_WITH_UAV:
        return "OMSetRTWithUAV";
    case CMD_HS_SET_SHADER_2:
        return "HSSetShader2";
    case CMD_COPY_RESOURCE_TO_TEX:
        return "CopyResourceToTexture";
    case CMD_BLIT:
        return "Blit";
    case CMD_COPY_RESOURCE:
        return "CopyResource";
    case CMD_COPY_SUBRESOURCE_REGION:
        return "CopySubresourceRegion";
    case CMD_UPDATE_SUBRESOURCE_REGION:
        return "UpdateSubresourceRegion";
    case CMD_PIX_BEGIN_EVENT:
        return "PIXBeginEvent";
    case CMD_PIX_END_EVENT:
        return "PIXEndEvent";
    case CMD_PIX_BEGIN_EVENT_2:
        return "PIXBeginEvent2";
    case CMD_NVAPI_VRR_CONTROL:
        return "NvAPIVRRControl";
    case CMD_FLUSH:
        return "Flush";
    case CMD_RELEASE_SHADER_RESOURCES:
        return "ReleaseShaderResources";
    case CMD_RELEASE_RESOURCE:
        return "ReleaseResource";
    case CMD_SET_SWAPCHAIN_RESOURCE:
        return "SetSwapChainResource";
    case CMD_FLUSH_INTERNAL:
        return "FlushInternal";
    case CMD_SETUP_RENDER_STATE:
        return "SetupRenderState";
    default:
        return "Unknown";
    }
}

// 解析并打印命令详情
void ParseCmdDetails(const uint8_t *pCmd, uint16_t cmdType, uint16_t cmdLen)
{
    const uint8_t *pParams = pCmd + 4; // 跳过头部

    switch (cmdType)
    {
    case CMD_DRAW_INSTANCED:
    {
        uint32_t vertexCount = *(uint32_t *) (pParams);
        uint32_t instanceCount = *(uint32_t *) (pParams + 4);
        // SPDLOG_LOGGER_INFO(logger, "    VertexCount={}, InstanceCount={}", vertexCount, instanceCount);
        break;
    }
    case CMD_DRAW_INDEXED_INSTANCED:
    {
        uint32_t indexCount = *(uint32_t *) (pParams);
        uint32_t instanceCount = *(uint32_t *) (pParams + 4);
        uint32_t startIndex = *(uint32_t *) (pParams + 8);
        int32_t baseVertex = *(int32_t *) (pParams + 12);
        // SPDLOG_LOGGER_INFO(logger, "    IndexCount={}, InstanceCount={}, StartIndex={}, BaseVertex={}", indexCount, instanceCount, startIndex, baseVertex);
        break;
    }
    case CMD_DISPATCH:
    {
        uint32_t groupX = *(uint32_t *) (pParams);
        uint32_t groupY = *(uint32_t *) (pParams + 4);
        uint32_t groupZ = *(uint32_t *) (pParams + 8);
        // SPDLOG_LOGGER_INFO(logger, "    ThreadGroups=({}, {}, {})", groupX, groupY, groupZ);
        break;
    }
    case CMD_VS_SET_SHADER:
    case CMD_PS_SET_SHADER:
    case CMD_GS_SET_SHADER:
    case CMD_CS_SET_SHADER:
    case CMD_DS_SET_SHADER:
    case CMD_HS_SET_SHADER:
    {
        uint64_t pShader = *(uint64_t *) (pParams);
        // SPDLOG_LOGGER_INFO(logger, "    pShader=0x{:016X}", pShader);
        break;
    }
    case CMD_OM_SET_RENDER_TARGETS:
    {
        uint64_t pRTV = *(uint64_t *) (pParams);
        // SPDLOG_LOGGER_INFO(logger, "    pRenderTargetView=0x{:016X}", pRTV);
        break;
    }
    case CMD_CS_SET_UAV:
    {
        uint64_t pUAV = *(uint64_t *) (pParams);
        // SPDLOG_LOGGER_INFO(logger, "    pUnorderedAccessView=0x{:016X}", pUAV);
        break;
    }
    case CMD_IA_SET_INPUT_LAYOUT:
    {
        uint64_t pLayout = *(uint64_t *) (pParams);
        // SPDLOG_LOGGER_INFO(logger, "    pInputLayout=0x{:016X}", pLayout);
        break;
    }
    case CMD_MAP_BUFFER:
    {
        uint64_t pBufferInfo = *(uint64_t *) (pParams);
        // SPDLOG_LOGGER_INFO(logger, "    pBufferInfo=0x{:016X}", pBufferInfo);
        break;
    }
    case CMD_LINK_NEXT_BLOCK:
    {
        uint64_t pNextBlock = *(uint64_t *) (pParams);
        // SPDLOG_LOGGER_INFO(logger, "    pNextBlock=0x{:016X}", pNextBlock);
        break;
    }
    case CMD_BATCH_SHADER_RESOURCES:
    {
        // 0x28 命令比较复杂，简单打印一些关键信息
        SPDLOG_LOGGER_INFO(logger, "    (DataLen={})", cmdLen - 4);

        uint32_t v2 = *(uint32_t *) (pCmd + 4);
        uint64_t *v3 = (uint64_t *) (pCmd + 80);

        // SetConstantBuffers
        if ((v2 & 0x10) != 0)
        {
            v3 += *(__int16 *) (pCmd + 10);
        }

        // SetSamplers
        if ((v2 & 0x40) != 0)
        {
            v3 += *(__int16 *) (pCmd + 50);
        }

        // PSSetShaderResources
        if ((v2 & 0x20) != 0)
        {
            uint32_t StartSlot = (unsigned int) *(__int16 *) (pCmd + 28);
            uint32_t NumViews = (unsigned int) *(__int16 *) (pCmd + 30);
            uint64_t *pShaderResourceViews = v3;
            SPDLOG_LOGGER_INFO(logger, "PSSetShaderResources: StartSlot={}, NumViews={}", StartSlot, NumViews);
        }

        break;
    }
    case CMD_SET_SHADER_WITH_CB:
    {
        uint32_t flags = *(uint32_t *) (pParams);
        int32_t slot = *(int32_t *) (pParams + 4);
        // SPDLOG_LOGGER_INFO(logger, "    Flags=0x{:08X}, Slot={}", flags, slot);
        break;
    }
    case CMD_SET_VIEWPORT:
    {
        uint32_t count = *(uint32_t *) (pParams);
        // SPDLOG_LOGGER_INFO(logger, "    ViewportCount={}", count);
        break;
    }
    default:
        // 对于其他命令，打印前16字节参数作为参考
        if (cmdLen > 4)
        {
            std::string hexStr;
            int paramLen = (cmdLen - 4 > 16) ? 16 : (cmdLen - 4);
            for (int i = 0; i < paramLen; i++)
            {
                char buf[4];
                snprintf(buf, sizeof(buf), "%02X ", pParams[i]);
                hexStr += buf;
            }
            if (cmdLen - 4 > 16)
                hexStr += "...";
            // SPDLOG_LOGGER_INFO(logger, "    Params: {}", hexStr);
        }
        break;
    }
}

/**
 * @brief 解析渲染命令块
 * @param pCmdBlock 命令块指针 (从 RenderCommandBuffer+0x20 获取)
 * @param maxBytes 最大解析字节数，防止越界 (默认 64KB)
 * @return 解析的命令数量
 */
int ParseRenderCmdBlock(const void *pCmdBlock, size_t maxBytes = 65536)
{
    if (!pCmdBlock)
    {
        SPDLOG_LOGGER_INFO(logger, "ParseRenderCmdBlock: pCmdBlock is NULL");
        return 0;
    }

    SPDLOG_LOGGER_INFO(logger, "========== Parsing RenderCmdBlock at 0x{:016X} ==========", (uintptr_t) pCmdBlock);

    const uint8_t *pData = static_cast<const uint8_t *>(pCmdBlock);
    size_t offset = 0;
    int cmdCount = 0;
    bool hasNextBlock = false;
    uint64_t nextBlockAddr = 0;

    while (offset + 4 <= maxBytes)
    {
        // 读取命令头
        uint16_t rawType = *(uint16_t *) (pData + offset);
        uint16_t cmdLen = *(uint16_t *) (pData + offset + 2);

        // 提取实际命令类型 (低15位) 和释放标志 (高位)
        uint16_t cmdType = rawType & 0x7FFF;
        bool needRelease = (rawType & 0x8000) != 0;

        // 验证命令长度
        if (cmdLen < 4 || offset + cmdLen > maxBytes)
        {
            SPDLOG_LOGGER_INFO(logger, "[{}] Offset 0x{:04X}: Invalid cmdLen={}, stopping parse", cmdCount, offset, cmdLen);
            break;
        }

        // 检查是否是有效命令类型
        if (cmdType > 0x47)
        {
            SPDLOG_LOGGER_INFO(logger, "[{}] Offset 0x{:04X}: Unknown cmdType=0x{:04X}, stopping parse", cmdCount, offset, cmdType);
            break;
        }

        // 打印命令信息
        const char *cmdName = GetCmdName(cmdType);
        SPDLOG_LOGGER_INFO(logger, "[{:3}] Offset 0x{:04X}: Type=0x{:02X} ({:<24}) Len={:4} {}", cmdCount, offset, cmdType, cmdName, cmdLen, needRelease ? "[Release]" : "");

        // 解析命令详情
        ParseCmdDetails(pData + offset, cmdType, cmdLen);

        cmdCount++;

        // 处理特殊命令
        if (cmdType == CMD_END)
        {
            SPDLOG_LOGGER_INFO(logger, "========== End of CmdBlock, Total {} commands ==========", cmdCount);
            break;
        }
        else if (cmdType == CMD_LINK_NEXT_BLOCK)
        {
            nextBlockAddr = *(uint64_t *) (pData + offset + 8);
            hasNextBlock = true;
            SPDLOG_LOGGER_INFO(logger, "========== LinkNextBlock -> 0x{:016X}, Total {} commands ==========", nextBlockAddr, cmdCount);
            break;
        }

        // 移动到下一条命令
        offset += cmdLen;
    }

    if (offset >= maxBytes)
    {
        SPDLOG_LOGGER_INFO(logger, "========== Reached maxBytes limit, Total {} commands ==========", cmdCount);
    }

    return cmdCount;
}

/**
 * @brief 递归解析所有链接的命令块
 * @param pCmdBlock 第一个命令块指针
 * @param maxBlocks 最大解析块数，防止无限循环
 */
void ParseAllRenderCmdBlocks(const void *pCmdBlock, int maxBlocks)
{
    SPDLOG_LOGGER_INFO(logger, "ParseAllRenderCmdBlocks");

    SPDLOG_LOGGER_INFO(logger, "pCmdBlock= 0x{:016X}", (uintptr_t) pCmdBlock);

    const uint8_t *pCurrentBlock = static_cast<const uint8_t *>(pCmdBlock);
    int blockIndex = 0;

    while (pCurrentBlock && blockIndex < maxBlocks)
    {
        SPDLOG_LOGGER_INFO(logger, ">>>>>>>>>> Block #{} at 0x{:016X} <<<<<<<<<<", blockIndex, (uintptr_t) pCurrentBlock);

        // 查找 LinkNextBlock 命令来获取下一个块地址
        const uint8_t *pData = pCurrentBlock;
        size_t offset = 0;
        const uint8_t *pNextBlock = nullptr;

        while (offset < 65536)
        {
            uint16_t rawType = *(uint16_t *) (pData + offset);
            uint16_t cmdLen = *(uint16_t *) (pData + offset + 2);
            uint16_t cmdType = rawType & 0x7FFF;

            if (cmdLen < 4)
            {
                SPDLOG_LOGGER_INFO(logger, "Invalid cmdLen, stopping parse");
                break;
            }
            else if (cmdType > 0x47)
            {
                SPDLOG_LOGGER_INFO(logger, "Unknown cmdType=0x{:04X}, stopping parse", cmdType);
                break;
            }

            if (cmdType == CMD_END)
            {
                ParseRenderCmdBlock(pCurrentBlock);
                pNextBlock = nullptr;
                break;
            }
            else if (cmdType == CMD_LINK_NEXT_BLOCK)
            {
                ParseRenderCmdBlock(pCurrentBlock);
                pNextBlock = *(const uint8_t **) (pData + offset + 8);
                break;
            }

            offset += cmdLen;
        }

        pCurrentBlock = pNextBlock;
        blockIndex++;
    }

    SPDLOG_LOGGER_INFO(logger, "Finished parsing {} blocks \n", blockIndex);
}