#include <nn/gfx/detail/gfx_State-api.nvn.8.h>

#include <algorithm>

#include <nn/gfx/detail/gfx_NvnHelper.h>
#include <nn/gfx/detail/gfx_Shader-api.nvn.8.h>
#include <nn/gfx/gfx_StateInfo.h>
#include <nn/util/util_BitArray.h>
#include <nn/util/util_BytePtr.h>
#include <nvn/nvn_FuncPtrInline.h>

namespace nn::gfx::detail {

typedef RasterizerStateImpl<ApiVariationNvn8> RasterizerStateImplNvn8;
typedef BlendStateImpl<ApiVariationNvn8> BlendStateImplNvn8;
typedef DepthStencilStateImpl<ApiVariationNvn8> DepthStencilStateImplNvn8;
typedef VertexStateImpl<ApiVariationNvn8> VertexStateImplNvn8;
typedef TessellationStateImpl<ApiVariationNvn8> TessellationStateImplNvn8;
typedef ViewportScissorStateImpl<ApiVariationNvn8> ViewportScissorStateImplNvn8;

/**
 * Constructs an uninitialized rasterizer state.
 */
RasterizerStateImplNvn8::RasterizerStateImpl() {}

/**
 * Destroys the rasterizer state. Finalize must have been called beforehand.
 */
RasterizerStateImplNvn8::~RasterizerStateImpl() {}

/**
 * Builds the NVN polygon and multisample states from a rasterizer description.
 *
 * @param pDevice Device owning the state (unused).
 * @param rInfo Rasterizer description.
 */
void RasterizerStateImplNvn8::Initialize(DeviceImpl<ApiVariationNvn8>* pDevice,
                                         const InfoType& rInfo) {
    NVNpolygonState* pPolygonState = reinterpret_cast<NVNpolygonState*>(&nvnPolygonState);
    NVNmultisampleState* pMultisampleState =
        reinterpret_cast<NVNmultisampleState*>(&nvnMultisampleState);

    nvnPolygonStateSetDefaults(pPolygonState);
    nvnPolygonStateSetCullFace(pPolygonState, Nvn::GetCullMode(rInfo.GetCullMode()));
    nvnPolygonStateSetFrontFace(pPolygonState, Nvn::GetFrontFace(rInfo.GetFrontFace()));
    nvnPolygonStateSetPolygonMode(pPolygonState, Nvn::GetFillMode(rInfo.GetFillMode()));

    nvnDepthBias = rInfo.GetDepthBias();
    nvnDepthBiasClamp = rInfo.GetDepthBiasClamp();
    nvnSlopeScaledDepthBias = rInfo.GetSlopeScaledDepthBias();

    bool isPolygonOffsetEnabled = nvnDepthBias != 0.0f || nvnDepthBiasClamp != 0.0f ||
                                  nvnSlopeScaledDepthBias != 0;
    nvnPolygonStateSetPolygonOffsetEnables(
        pPolygonState, isPolygonOffsetEnabled ?
                           (NVN_POLYGON_OFFSET_ENABLE_POINT | NVN_POLYGON_OFFSET_ENABLE_LINE |
                            NVN_POLYGON_OFFSET_ENABLE_FILL) :
                           NVN_POLYGON_OFFSET_ENABLE_NONE);

    nvnSampleMask = rInfo.GetMultisampleStateInfo().GetSampleMask();
    nvnMultisampleStateSetDefaults(pMultisampleState);
    nvnMultisampleStateSetMultisampleEnable(pMultisampleState, rInfo.IsMultisampleEnabled());
    int sampleCount = rInfo.GetMultisampleStateInfo().GetSampleCount();
    nvnMultisampleStateSetSamples(pMultisampleState, sampleCount > 1 ? sampleCount : 0);
    nvnMultisampleStateSetAlphaToCoverageEnable(
        pMultisampleState, rInfo.GetMultisampleStateInfo().IsAlphaToCoverageEnabled());

    flags.SetBit(Flag_MultisampleEnabled, rInfo.IsMultisampleEnabled());
    flags.SetBit(Flag_DepthClipEnabled, rInfo.IsDepthClipEnabled());
    flags.SetBit(Flag_RasterEnabled, rInfo.IsRasterEnabled());
    flags.SetBit(Flag_ConservativeRasterEnabled, rInfo.GetConservativeRasterizationMode() ==
                                                     ConservativeRasterizationMode_Enable);

    state = State_Initialized;
}

/**
 * Finalizes the rasterizer state.
 *
 * @param pDevice Device the state was initialized with (unused).
 */
void RasterizerStateImplNvn8::Finalize(DeviceImpl<ApiVariationNvn8>* pDevice) {
    state = State_NotInitialized;
}

/**
 * Returns the size of the external memory needed for the per-target NVN blend states.
 *
 * @param rInfo Blend description.
 * @return Required size in bytes (one NVNblendState per blend target).
 */
size_t BlendStateImplNvn8::GetRequiredMemorySize(const InfoType& rInfo) {
    return sizeof(NVNblendState) * rInfo.GetBlendTargetCount();
}

/**
 * Constructs an uninitialized blend state.
 */
BlendStateImplNvn8::BlendStateImpl() {}

/**
 * Destroys the blend state. Finalize must have been called beforehand.
 */
BlendStateImplNvn8::~BlendStateImpl() {}

/**
 * Sets the external memory used for the per-target NVN blend states.
 *
 * @param pMemory Memory block, at least GetRequiredMemorySize() bytes.
 * @param size Size of the memory block in bytes.
 */
void BlendStateImplNvn8::SetMemory(void* pMemory, size_t size) {
    pNvnBlendStateData = pMemory;
    memorySize = size;
}

/**
 * Returns the external memory set with SetMemory.
 *
 * @return Memory block holding the NVN blend states.
 */
void* BlendStateImplNvn8::GetMemory() {
    return pNvnBlendStateData;
}

/**
 * Returns the external memory set with SetMemory.
 *
 * @return Memory block holding the NVN blend states.
 */
void* BlendStateImplNvn8::GetMemory() const {
    return pNvnBlendStateData;
}

/**
 * Builds the NVN color, channel mask and per-target blend states from a blend description.
 *
 * @param pDevice Device owning the state (unused).
 * @param rInfo Blend description.
 */
void BlendStateImplNvn8::Initialize(DeviceImpl<ApiVariationNvn8>* pDevice,
                                    const InfoType& rInfo) {
    NVNcolorState* pColorState = reinterpret_cast<NVNcolorState*>(&nvnColorState);
    NVNchannelMaskState* pChannelMaskState =
        reinterpret_cast<NVNchannelMaskState*>(&nvnChannelMaskState);
    NVNblendState* pBlendStateData = pNvnBlendStateData;
    const BlendTargetStateInfo* pBlendInfo = rInfo.GetBlendTargetStateInfoArray();

    nvnBlendConstant[0] = rInfo.GetBlendConstant(ColorChannel_Red);
    nvnBlendConstant[1] = rInfo.GetBlendConstant(ColorChannel_Green);
    nvnBlendConstant[2] = rInfo.GetBlendConstant(ColorChannel_Blue);
    nvnBlendConstant[3] = rInfo.GetBlendConstant(ColorChannel_Alpha);

    nvnColorStateSetDefaults(pColorState);
    nvnChannelMaskStateSetDefaults(pChannelMaskState);

    targetCount = rInfo.GetBlendTargetCount();

    for (int index = 0; index < rInfo.GetBlendTargetCount(); ++index) {
        int infoIndex = rInfo.IsIndependentBlendEnabled() ? index : 0;
        const BlendTargetStateInfo& rTarget = pBlendInfo[infoIndex];
        NVNblendState* pBlendState = &pBlendStateData[index];

        nvnColorStateSetBlendEnable(pColorState, index, rTarget.IsBlendEnabled());
        nvnBlendStateSetDefaults(pBlendState);
        nvnBlendStateSetBlendTarget(pBlendState, index);

        nvnBlendStateSetBlendFunc(
            pBlendState, Nvn::GetBlendFunction(rTarget.GetSourceColorBlendFactor()),
            Nvn::GetBlendFunction(rTarget.GetDestinationColorBlendFactor()),
            Nvn::GetBlendFunction(rTarget.GetSourceAlphaBlendFactor()),
            Nvn::GetBlendFunction(rTarget.GetDestinationAlphaBlendFactor()));

        nvnBlendStateSetBlendEquation(pBlendState,
                                      Nvn::GetBlendEquation(rTarget.GetColorBlendFunction()),
                                      Nvn::GetBlendEquation(rTarget.GetAlphaBlendFunction()));

        int channelMask = rTarget.GetChannelMask();
        nvnChannelMaskStateSetChannelMask(pChannelMaskState, index,
                                          (channelMask & ChannelMask_Red) == ChannelMask_Red,
                                          (channelMask & ChannelMask_Green) == ChannelMask_Green,
                                          (channelMask & ChannelMask_Blue) == ChannelMask_Blue,
                                          (channelMask & ChannelMask_Alpha) == ChannelMask_Alpha);
    }

    if (rInfo.IsLogicOperationEnabled()) {
        nvnColorStateSetLogicOp(pColorState, Nvn::GetLogicOperation(rInfo.GetLogicOperation()));
    }

    state = State_Initialized;
}

/**
 * Finalizes the blend state.
 *
 * @param pDevice Device the state was initialized with (unused).
 */
void BlendStateImplNvn8::Finalize(DeviceImpl<ApiVariationNvn8>* pDevice) {
    state = State_NotInitialized;
}

/**
 * Constructs an uninitialized depth/stencil state.
 */
DepthStencilStateImplNvn8::DepthStencilStateImpl() {}

/**
 * Destroys the depth/stencil state. Finalize must have been called beforehand.
 */
DepthStencilStateImplNvn8::~DepthStencilStateImpl() {}

/**
 * Builds the NVN depth/stencil state and stencil reference values from a description.
 *
 * @param pDevice Device owning the state (unused).
 * @param rInfo Depth/stencil description.
 */
void DepthStencilStateImplNvn8::Initialize(DeviceImpl<ApiVariationNvn8>* pDevice,
                                           const DepthStencilStateInfo& rInfo) {
    NVNdepthStencilState* pDepthStencilState =
        reinterpret_cast<NVNdepthStencilState*>(&nvnDepthStencilState);

    flag = rInfo.ToData()->flag;

    nvnDepthStencilStateSetDefaults(pDepthStencilState);
    nvnDepthStencilStateSetDepthTestEnable(pDepthStencilState, rInfo.IsDepthTestEnabled());
    nvnDepthStencilStateSetDepthWriteEnable(pDepthStencilState, rInfo.IsDepthWriteEnabled());
    nvnDepthStencilStateSetStencilTestEnable(pDepthStencilState, rInfo.IsStencilTestEnabled());

    nvnDepthStencilStateSetDepthFunc(pDepthStencilState,
                                     Nvn::GetDepthFunction(rInfo.GetDepthComparisonFunction()));

    const StencilStateInfo& rBack = rInfo.GetBackStencilStateInfo();
    nvnDepthStencilStateSetStencilFunc(pDepthStencilState, NVN_FACE_BACK,
                                       Nvn::GetStencilFunction(rBack.GetComparisonFunction()));
    nvnDepthStencilStateSetStencilOp(pDepthStencilState, NVN_FACE_BACK,
                                     Nvn::GetStencilOperation(rBack.GetStencilFailOperation()),
                                     Nvn::GetStencilOperation(rBack.GetDepthFailOperation()),
                                     Nvn::GetStencilOperation(rBack.GetDepthPassOperation()));

    const StencilStateInfo& rFront = rInfo.GetFrontStencilStateInfo();
    nvnDepthStencilStateSetStencilFunc(pDepthStencilState, NVN_FACE_FRONT,
                                       Nvn::GetStencilFunction(rFront.GetComparisonFunction()));
    nvnDepthStencilStateSetStencilOp(pDepthStencilState, NVN_FACE_FRONT,
                                     Nvn::GetStencilOperation(rFront.GetStencilFailOperation()),
                                     Nvn::GetStencilOperation(rFront.GetDepthFailOperation()),
                                     Nvn::GetStencilOperation(rFront.GetDepthPassOperation()));

    nvnStencilBackRef = rBack.GetStencilRef();
    nvnStencilFrontRef = rFront.GetStencilRef();
    nvnStencilValueMask = rInfo.GetStencilReadMask();
    nvnStencilMask = rInfo.GetStencilWriteMask();

    state = State_Initialized;
}

/**
 * Finalizes the depth/stencil state.
 *
 * @param pDevice Device the state was initialized with (unused).
 */
void DepthStencilStateImplNvn8::Finalize(DeviceImpl<ApiVariationNvn8>* pDevice) {
    state = State_NotInitialized;
}

/**
 * Returns the size of the external memory needed for the NVN vertex stream and attribute states.
 *
 * @param rInfo Vertex state description.
 * @return Required size in bytes (stream states followed by attribute states up to the highest
 *         used slot; an attribute without a fixed slot, or with a name, reserves all 16 slots).
 */
size_t VertexStateImplNvn8::GetRequiredMemorySize(const InfoType& rInfo) {
    int maxSlot = -1;
    const VertexAttributeStateInfo* pAttribStates = rInfo.GetVertexAttributeStateInfoArray();

    for (int idxAttribute = 0, attributeCount = rInfo.GetVertexAttributeCount();
         idxAttribute < attributeCount; ++idxAttribute) {
        const VertexAttributeStateInfo& rAttribState = pAttribStates[idxAttribute];

        if (rAttribState.GetShaderSlot() < 0) {
            maxSlot = 15;
        } else {
            // Named attributes are resolved against the shader later, so reserve every slot.
            maxSlot = rAttribState.GetNamePtr() != nullptr ?
                          15 :
                          std::max(maxSlot, static_cast<int>(rAttribState.GetShaderSlot()));
        }
    }

    return sizeof(NVNvertexStreamState) * rInfo.GetVertexBufferCount() +
           sizeof(NVNvertexAttribState) * (maxSlot + 1);
}

/**
 * Constructs an uninitialized vertex state.
 */
VertexStateImplNvn8::VertexStateImpl() {}

/**
 * Destroys the vertex state. Finalize must have been called beforehand.
 */
VertexStateImplNvn8::~VertexStateImpl() {}

/**
 * Sets the external memory used for the NVN vertex stream and attribute states.
 *
 * @param pMemory Memory block, at least GetRequiredMemorySize() bytes.
 * @param size Size of the memory block in bytes.
 */
void VertexStateImplNvn8::SetMemory(void* pMemory, size_t size) {
    pNvnVertexStreamState = pMemory;
    memorySize = size;
}

/**
 * Returns the external memory set with SetMemory.
 *
 * @return Memory block holding the NVN vertex states.
 */
void* VertexStateImplNvn8::GetMemory() {
    return pNvnVertexStreamState;
}

/**
 * Returns the external memory set with SetMemory.
 *
 * @return Memory block holding the NVN vertex states.
 */
const void* VertexStateImplNvn8::GetMemory() const {
    return pNvnVertexStreamState;
}

/**
 * Builds the NVN vertex stream and attribute states from a vertex state description.
 *
 * @param pDevice Device owning the state (unused).
 * @param rInfo Vertex state description.
 * @param pVertexShader Optional vertex shader used to resolve attribute slots by name.
 */
void VertexStateImplNvn8::Initialize(DeviceImpl<ApiVariationNvn8>* pDevice, const InfoType& rInfo,
                                     const ShaderImpl<ApiVariationNvn8>* pVertexShader) {
    vertexStreamStateCount = rInfo.GetVertexBufferCount();
    pNvnVertexAttribState = nn::util::BytePtr(pNvnVertexStreamState.ptr)
                                .Advance(sizeof(NVNvertexStreamState) *
                                         rInfo.GetVertexBufferCount())
                                .Get();

    for (int idxBuffer = 0; idxBuffer < rInfo.GetVertexBufferCount(); ++idxBuffer) {
        const VertexBufferStateInfo& rSrc = rInfo.GetVertexBufferStateInfoArray()[idxBuffer];
        NVNvertexStreamState* pDst =
            static_cast<NVNvertexStreamState*>(pNvnVertexStreamState) + idxBuffer;

        nvnVertexStreamStateSetDefaults(pDst);
        nvnVertexStreamStateSetStride(pDst, rSrc.GetStride());
        nvnVertexStreamStateSetDivisor(pDst, rSrc.GetDivisor());
    }

    const int maxAttribs = 256;
    Bit32 bitArrayMemory[8] = {};
    nn::util::BitArray setAttribs(bitArrayMemory, sizeof(bitArrayMemory), maxAttribs);
    setAttribs.reset();
    int maxSlot = -1;
    int setAttribCount = 0;

    for (int idxAttrib = 0; idxAttrib < rInfo.GetVertexAttributeCount(); ++idxAttrib) {
        const VertexAttributeStateInfo& rSrc =
            rInfo.GetVertexAttributeStateInfoArray()[idxAttrib];

        int idxDst = rSrc.GetShaderSlot();
        if (pVertexShader != nullptr && rSrc.GetNamePtr() != nullptr) {
            idxDst = pVertexShader->GetInterfaceSlot(ShaderStage_Vertex, ShaderInterfaceType_Input,
                                                     rSrc.GetNamePtr());
        }

        if (idxDst >= 0) {
            NVNvertexAttribState* pDst =
                static_cast<NVNvertexAttribState*>(pNvnVertexAttribState) + idxDst;

            nvnVertexAttribStateSetDefaults(pDst);
            nvnVertexAttribStateSetFormat(pDst, Nvn::GetAttributeFormat(rSrc.GetFormat()),
                                          rSrc.GetOffset());
            nvnVertexAttribStateSetStreamIndex(pDst, rSrc.GetBufferIndex());

            setAttribs.set(idxDst, true);
            maxSlot = std::max(maxSlot, idxDst);
            ++setAttribCount;
        }
    }

    vertexAttributeStateCount = rInfo.GetVertexAttributeCount() != 0 ? maxSlot + 1 : 0;

    // Slots in [0, maxSlot] that no attribute was assigned to must still be reset to defaults.
    if (maxSlot >= setAttribCount) {
        for (int idxSlot = 0; idxSlot <= maxSlot; ++idxSlot) {
            if (!setAttribs.test(idxSlot)) {
                nvnVertexAttribStateSetDefaults(
                    static_cast<NVNvertexAttribState*>(pNvnVertexAttribState) + idxSlot);
            }
        }
    }

    state = State_Initialized;
}

/**
 * Finalizes the vertex state.
 *
 * @param pDevice Device the state was initialized with (unused).
 */
void VertexStateImplNvn8::Finalize(DeviceImpl<ApiVariationNvn8>* pDevice) {
    state = State_NotInitialized;
}

/**
 * Constructs an uninitialized tessellation state.
 */
TessellationStateImplNvn8::TessellationStateImpl() {
    state = State_NotInitialized;
}

/**
 * Destroys the tessellation state. Finalize must have been called beforehand.
 */
TessellationStateImplNvn8::~TessellationStateImpl() {}

/**
 * Initializes the tessellation state from a description.
 *
 * @param pDevice Device owning the state (unused).
 * @param rInfo Tessellation description (patch control point count).
 */
void TessellationStateImplNvn8::Initialize(DeviceImpl<ApiVariationNvn8>* pDevice,
                                           const TessellationStateInfo& rInfo) {
    patchSize = rInfo.GetPatchControlPointCount();
    state = State_Initialized;
}

/**
 * Finalizes the tessellation state.
 *
 * @param pDevice Device the state was initialized with (unused).
 */
void TessellationStateImplNvn8::Finalize(DeviceImpl<ApiVariationNvn8>* pDevice) {
    state = State_NotInitialized;
}

/**
 * Returns the size of the external memory needed for viewports beyond the first.
 *
 * @param rInfo Viewport/scissor description.
 * @return Required size in bytes (viewport, depth range and scissor per extra viewport).
 */
size_t ViewportScissorStateImplNvn8::GetRequiredMemorySize(const ViewportScissorStateInfo& rInfo) {
    int extraViewportCount = rInfo.GetViewportCount() - 1;
    return size_t(40) * extraViewportCount;
}

/**
 * Constructs an uninitialized viewport/scissor state.
 */
ViewportScissorStateImplNvn8::ViewportScissorStateImpl() {
    state = State_NotInitialized;
}

/**
 * Destroys the viewport/scissor state. Finalize must have been called beforehand.
 */
ViewportScissorStateImplNvn8::~ViewportScissorStateImpl() {}

/**
 * Sets the external memory used for the extra viewports and scissors.
 *
 * @param pMemory Memory block, at least GetRequiredMemorySize() bytes.
 * @param size Size of the memory block in bytes.
 */
void ViewportScissorStateImplNvn8::SetMemory(void* pMemory, size_t size) {
    pWorkMemory = pMemory;
    memorySize = size;
}

/**
 * Returns the external memory set with SetMemory.
 *
 * @return Memory block holding the extra viewports and scissors.
 */
void* ViewportScissorStateImplNvn8::GetMemory() {
    return pWorkMemory;
}

/**
 * Returns the external memory set with SetMemory.
 *
 * @return Memory block holding the extra viewports and scissors.
 */
const void* ViewportScissorStateImplNvn8::GetMemory() const {
    return pWorkMemory;
}

/**
 * Stores the viewports and scissors from a description. The first viewport/scissor is kept
 * inline; the remaining ones are written to the external memory.
 *
 * @param pDevice Device owning the state (unused).
 * @param rInfo Viewport/scissor description.
 */
void ViewportScissorStateImplNvn8::Initialize(DeviceImpl<ApiVariationNvn8>* pDevice,
                                              const ViewportScissorStateInfo& rInfo) {
    flag = rInfo.ToData()->flag;
    viewportCount = rInfo.GetViewportCount();

    const ViewportStateInfo* pViewports = rInfo.GetViewportStateInfoArray();
    const ScissorStateInfo* pScissors = rInfo.GetScissorStateInfoArray();

    viewport[0] = pViewports->GetOriginX();
    viewport[1] = pViewports->GetOriginY();
    viewport[2] = pViewports->GetWidth();
    viewport[3] = pViewports->GetHeight();
    depthRange[0] = pViewports->GetMinDepth();
    depthRange[1] = pViewports->GetMaxDepth();

    if (rInfo.IsScissorEnabled()) {
        scissor[0] = pScissors->GetOriginX();
        scissor[1] = pScissors->GetOriginY();
        scissor[2] = pScissors->GetWidth();
        scissor[3] = pScissors->GetHeight();
    } else {
        scissor[0] = 0;
        scissor[1] = 0;
        scissor[2] = 0x7FFFFFFF;
        scissor[3] = 0x7FFFFFFF;
    }

    int extraViewportCount = rInfo.GetViewportCount() - 1;

    nn::util::BytePtr ptr(pWorkMemory.ptr);
    float* pViewportArray = ptr.Get<float>();
    float* pDepthRangeArray = ptr.Advance(size_t(16) * extraViewportCount).Get<float>();
    int32_t* pScissorArray = ptr.Advance(size_t(8) * extraViewportCount).Get<int32_t>();

    for (int idx = 1; idx < viewportCount; ++idx) {
        int idxExtra = idx - 1;
        const ViewportStateInfo& rViewport = pViewports[idx];

        pViewportArray[4 * idxExtra + 0] = rViewport.GetOriginX();
        pViewportArray[4 * idxExtra + 1] = rViewport.GetOriginY();
        pViewportArray[4 * idxExtra + 2] = rViewport.GetWidth();
        pViewportArray[4 * idxExtra + 3] = rViewport.GetHeight();
        pDepthRangeArray[2 * idxExtra + 0] = rViewport.GetMinDepth();
        pDepthRangeArray[2 * idxExtra + 1] = rViewport.GetMaxDepth();
    }

    for (int idx = 1; idx < viewportCount; ++idx) {
        int idxExtra = idx - 1;
        const ScissorStateInfo& rScissor = pScissors[idx];

        if (rInfo.IsScissorEnabled()) {
            pScissorArray[4 * idxExtra + 0] = rScissor.GetOriginX();
            pScissorArray[4 * idxExtra + 1] = rScissor.GetOriginY();
            pScissorArray[4 * idxExtra + 2] = rScissor.GetWidth();
            pScissorArray[4 * idxExtra + 3] = rScissor.GetHeight();
        } else {
            pScissorArray[4 * idxExtra + 0] = 0;
            pScissorArray[4 * idxExtra + 1] = 0;
            pScissorArray[4 * idxExtra + 2] = 0x7FFFFFFF;
            pScissorArray[4 * idxExtra + 3] = 0x7FFFFFFF;
        }
    }

    state = State_Initialized;
}

/**
 * Finalizes the viewport/scissor state.
 *
 * @param pDevice Device the state was initialized with (unused).
 */
void ViewportScissorStateImplNvn8::Finalize(DeviceImpl<ApiVariationNvn8>* pDevice) {
    state = State_NotInitialized;
}

}  // namespace nn::gfx::detail
