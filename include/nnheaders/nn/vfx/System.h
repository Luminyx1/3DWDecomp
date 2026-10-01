/**
 * @file System.h
 * @brief VFX system implementation.
 */

#pragma once

#include <nn/gfx/gfx_DescriptorSlot.h>
#include <nn/gfx/gfx_Types.h>
#include <nn/types.h>
#include <nn/vfx/Callback.h>
#include <nn/vfx/Config.h>
#include <nn/vfx/EmitterSet.h>
#include <nn/vfx/Resource.h>

// this class is massive
namespace nn {
namespace vfx {

class Handle;
class Heap;
struct ViewParam;

enum BufferSwapMode {
    BufferSwapMode_None = 0,
    BufferSwapMode_Swap = 1,
    BufferSwapMode_Auto = 2,
};

typedef bool (*RegisterTextureViewSlot)(gfx::DescriptorSlot* pSlot,
                                        const gfx::TextureView& rTextureView, void* pUserData);
typedef bool (*RegisterSamplerSlot)(gfx::DescriptorSlot* pSlot, const gfx::Sampler& rSampler,
                                    void* pUserData);
typedef void (*UnregisterTextureViewSlot)(gfx::DescriptorSlot* pSlot,
                                          const gfx::TextureView& rTextureView, void* pUserData);
typedef void (*UnregisterSamplerSlot)(gfx::DescriptorSlot* pSlot, const gfx::Sampler& rSampler,
                                      void* pUserData);

enum TextureSlotId {
    TextureSlotId_FrameBuffer,
    TextureSlotId_DepthBuffer,
};

struct RenderingInfo {
    u8 _0[0x8];
    u32 m_RenderingEmitterFlag[64];
    u8 _108[0x7a8 - 0x108];
    gfx::DescriptorSlot m_TextureSlot[2];
    u8 _7b8[0x9b8 - 0x7b8];
    detail::Shader* m_pCurrentShader;
    u8 _9c0[0x9e8 - 0x9c0];
};

class System {
public:
    System(nn::vfx::Config const&);

    virtual ~System();
    virtual void Initialize(nn::vfx::Heap*, nn::vfx::Heap*, nn::vfx::Config const&);

    bool CreateEmitterSetId(Handle* pHandle, s32 emitterSetId, s32 resourceId, s32 groupId,
                            Heap* pHeap, bool isDelay);
    void BeginFrame();
    void SwapBuffer();
    void Calculate(EmitterSet* pEmitterSet, f32 frameRate, BufferSwapMode swapMode);
    void Calculate(s32 groupId, f32 frameRate, BufferSwapMode swapMode);
    void KillAllEmitterSet();
    bool EntryResource(Heap* pHeap, void* pData, s32 resourceId, bool isDelay,
                       Resource* pResource);
    void RegisterTextureViewToDescriptorPool(RegisterTextureViewSlot pFunc, void* pUserData);
    void RegisterSamplerToDescriptorPool(RegisterSamplerSlot pFunc, void* pUserData);
    void BatchCalculationComputeShaderEmitter(gfx::CommandBuffer* pCommandBuffer, void* pUserParam,
                                              u32 groupBitFlag);
    void Draw(s32 processingIndex, gfx::CommandBuffer* pCommandBuffer, s32 groupId,
              u32 drawPathFlag, bool isDoComputeShaderProcess, bool isDrawViewDepend,
              void* pUserParam);
    void SetViewParam(s32 processingIndex, gfx::CommandBuffer* pCommandBuffer,
                      ViewParam* pViewParam);

    Resource* GetResource(s32 resourceId) const {
        Resource* resource = m_Resources[resourceId];

        if (resource == nullptr) {
            return nullptr;
        }

        return resource->IsInitialized() ? resource : nullptr;
    }

    EmitterSet* GetEmitterSetHead(u8 groupId) const { return m_EmitterSetHead[groupId]; }
    s32 GetRenderingInfoNum() const { return m_RenderingInfoNum; }
    const RenderingInfo& GetRenderingInfo(s32 index) const { return m_RenderingInfos[index]; }
    s32 GetResourceNum() const { return m_ResourceNum; }

    void SetCallback(s32 id, const CallbackSet& rCallbackSet) {
        m_IsEnableCallback[id] = true;
        m_CallbackSet[id] = rCallbackSet;
        m_CallbackSet[id].endianFlip = EndianFlipCallbackImpl;
    }

    void SetCustomFieldCallback(CustomFieldCallback callback) { m_CustomFieldCallback = callback; }

    void SetTextureSlot(s32 processingIndex, TextureSlotId id, gfx::DescriptorSlot slot) {
        if (processingIndex < m_RenderingInfoNum) {
            m_RenderingInfos[processingIndex].m_TextureSlot[id] = slot;
        }
    }

    detail::Shader* GetCurrentShader(s32 processingIndex) const {
        return m_RenderingInfos[processingIndex].m_pCurrentShader;
    }

    void SetDrawPathRenderStateSetCallback(DrawPathCallbackId id, DrawPathFlag flag,
                                           DrawPathRenderStateSetCallback callback);

    s32 m_RenderingInfoNum;
    RenderingInfo* m_RenderingInfos;
    u8 _18[0x68 - 0x18];
    s32 m_ResourceNum;
    u8 _6c[0x98 - 0x6c];
    Resource** m_Resources;
    u8 _a0[0xb0 - 0xa0];
    EmitterSet* m_EmitterSetHead[64];
    u8 _2b0[0x8c8 - 0x2b0];
    bool m_IsEnableCallback[16];
    u8 _8d8[0x8e0 - 0x8d8];
    CallbackSet m_CallbackSet[16];
    u8 _e60[0x1200 - 0xe60];
    CustomFieldCallback m_CustomFieldCallback;
    u8 _1208[0x1708 - 0x1208];
};

static_assert(sizeof(System) == 0x1708);
}  // namespace vfx
}  // namespace nn
