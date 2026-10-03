#include <nn/gfx/detail/gfx_CommandBuffer-api.nvn.8.h>

#include <nn/gfx/detail/gfx_Buffer-api.nvn.8.h>
#include <nn/gfx/detail/gfx_CommonHelper.h>
#include <nn/gfx/detail/gfx_DescriptorPool-api.nvn.8.h>
#include <nn/gfx/detail/gfx_Device-api.nvn.8.h>
#include <nn/gfx/detail/gfx_MemoryPool-api.nvn.8.h>
#include <nn/gfx/detail/gfx_NvnHelper.h>
#include <nn/gfx/detail/gfx_Pipeline-api.nvn.8.h>
#include <nn/gfx/detail/gfx_RootSignature-api.nvn.8.h>
#include <nn/gfx/detail/gfx_Shader-api.nvn.8.h>
#include <nn/gfx/detail/gfx_State-api.nvn.8.h>
#include <nn/gfx/detail/gfx_Texture-api.nvn.8.h>
#include <nn/gfx/gfx_DescriptorSlot.h>
#include <nn/gfx/gfx_StateInfo.h>
#include <nn/gfx/gfx_TextureInfo.h>
#include <nn/util/util_BytePtr.h>
#include <nvn/nvn_FuncPtrInline.h>

#include <algorithm>

namespace nn::gfx::detail {

typedef CommandBufferImpl<ApiVariationNvn8> CommandBufferImplNvn8;

namespace {

/** Domain id passed to the NVN debug group functions ('gfx'). */
const uint32_t DebugGroupDomainId = 0x676678;

/**
 * Reinterprets the value stored in a descriptor slot as a pointer.
 *
 * @param rSlot Descriptor slot to read.
 * @return Pointer held by the slot.
 */
template <typename T>
T* ToPtr(const DescriptorSlot& rSlot) {
    return reinterpret_cast<T*>(rSlot.ToData()->value);
}

/**
 * Computes the NVN Y/Z offsets and height/depth of a copy region. Array layers of 1D and 2D
 * array textures are addressed through the Y and Z axes respectively.
 *
 * @param pOffsetY Receives the Y offset.
 * @param pHeight Receives the height.
 * @param pOffsetZ Receives the Z offset.
 * @param pDepth Receives the depth.
 * @param rRegion Copy region to convert.
 * @param target NVN target of the texture being copied.
 */
void GetNvnCopyRegion(int* pOffsetY, int* pHeight, int* pOffsetZ, int* pDepth,
                      const TextureCopyRegion& rRegion, NVNtextureTarget target) {
    *pOffsetY = rRegion.GetOffsetV();
    *pHeight = rRegion.GetHeight();
    *pOffsetZ = rRegion.GetOffsetW();
    *pDepth = rRegion.GetDepth();

    switch (target) {
    case NVN_TEXTURE_TARGET_1D_ARRAY:
        *pOffsetY = rRegion.GetSubresource().GetArrayIndex();
        *pHeight = std::max(rRegion.GetArrayLength(), 1);
        break;

    case NVN_TEXTURE_TARGET_2D_ARRAY:
    case NVN_TEXTURE_TARGET_2D_MULTISAMPLE_ARRAY:
        *pOffsetZ = rRegion.GetSubresource().GetArrayIndex();
        *pDepth = std::max(rRegion.GetArrayLength(), 1);
        break;

    default:
        break;
    }
}

/**
 * Computes the buffer row and image strides of a buffer <-> texture copy.
 *
 * @param pRowStride Receives the row stride in bytes (0 = tightly packed).
 * @param pImageStride Receives the image stride in bytes (0 = tightly packed).
 * @param rRegion Buffer/texture copy region.
 * @param pTexture NVN texture taking part in the copy.
 */
void GetNvnCopyStride(ptrdiff_t* pRowStride, ptrdiff_t* pImageStride,
                      const BufferTextureCopyRegion& rRegion, NVNtexture* pTexture) {
    int rowStride = 0;
    int imageStride = 0;

    if (rRegion.GetBufferImageWidth() != 0 || rRegion.GetBufferImageHeight() != 0) {
        NVNformat nvnFormat = nvnTextureGetFormat(pTexture);
        ImageFormat imageFormat = Nvn::GetGfxImageFormat(nvnFormat);
        ChannelFormat channelFormat = static_cast<ChannelFormat>(imageFormat >> 8);

        int width = rRegion.GetBufferImageWidth() != 0 ?
                        rRegion.GetBufferImageWidth() :
                        rRegion.GetTextureCopyRegion().GetWidth();
        rowStride = width * GetBytePerPixel(channelFormat);

        if (IsCompressedFormat(channelFormat)) {
            rowStride /= GetBlockWidth(channelFormat) * GetBlockHeight(channelFormat);
        }
    }

    if (rRegion.GetBufferImageHeight() != 0) {
        imageStride = rRegion.GetBufferImageHeight() * rowStride;
    }

    *pRowStride = rowStride;
    *pImageStride = imageStride;
}

/**
 * Binds a combined texture/sampler handle to a shader stage.
 *
 * @param pNnCb Command buffer to record into.
 * @param stage Shader stage to bind to.
 * @param slot Texture binding slot.
 * @param nvnTextureId NVN texture descriptor id.
 * @param nvnSamplerId NVN sampler descriptor id.
 */
void SetTextureAndSampler(CommandBufferImplNvn8* pNnCb, ShaderStage stage, int slot,
                          unsigned int nvnTextureId, unsigned int nvnSamplerId) {
    const DeviceImpl<ApiVariationNvn8>* pNnDevice = pNnCb->ToData()->pNnDevice;
    NVNtextureHandle textureHandle =
        nvnDeviceGetTextureHandle(pNnDevice->ToData()->pNvnDevice, nvnTextureId, nvnSamplerId);

    nvnCommandBufferBindTexture(pNnCb->ToData()->pNvnCommandBuffer, Nvn::GetShaderStage(stage),
                                slot, textureHandle);
}

/**
 * NVN memory callback; forwards out-of-memory events to the user's gfx callbacks.
 *
 * @param pNvnCommandBuffer NVN command buffer that ran out of memory (unused).
 * @param event Kind of memory that ran out.
 * @param minSize Minimum amount of memory that must be added.
 * @param pCallbackData The owning CommandBufferImpl.
 */
void CommandBufferMemoryCallbackProcedure(NVNcommandBuffer* pNvnCommandBuffer,
                                          NVNcommandBufferMemoryEvent event, size_t minSize,
                                          void* pCallbackData) {
    CommandBufferImplNvn8* pThis = static_cast<CommandBufferImplNvn8*>(pCallbackData);
    CommandBufferImplNvn8::DataType& rData = pThis->ToData();

    TCommandBuffer<ApiVariationNvn8>* pCommandBuffer =
        reinterpret_cast<TCommandBuffer<ApiVariationNvn8>*>(pThis);
    OutOfMemoryEventArg arg{minSize};

    switch (event) {
    case NVN_COMMAND_BUFFER_MEMORY_EVENT_OUT_OF_COMMAND_MEMORY:
        reinterpret_cast<CommandBufferImplNvn8::OutOfMemoryEventCallback>(
            rData.pOutOfCommandMemoryCallback.ptr)(pCommandBuffer, arg);
        break;

    case NVN_COMMAND_BUFFER_MEMORY_EVENT_OUT_OF_CONTROL_MEMORY:
        reinterpret_cast<CommandBufferImplNvn8::OutOfMemoryEventCallback>(
            rData.pOutOfControlMemoryCallback.ptr)(pCommandBuffer, arg);
        break;

    default:
        NN_UNEXPECTED_DEFAULT;
        break;
    }
}

}  // namespace

/**
 * Returns the required alignment of command memory.
 *
 * @param pDevice Device the command buffer is created on.
 * @return Alignment in bytes.
 */
size_t CommandBufferImplNvn8::GetCommandMemoryAlignment(DeviceImpl<ApiVariationNvn8>* pDevice) {
    int align;
    nvnDeviceGetInteger(pDevice->ToData()->pNvnDevice,
                        NVN_DEVICE_INFO_COMMAND_BUFFER_COMMAND_ALIGNMENT, &align);
    return align;
}

/**
 * Returns the required alignment of control memory.
 *
 * @param pDevice Device the command buffer is created on.
 * @return Alignment in bytes.
 */
size_t CommandBufferImplNvn8::GetControlMemoryAlignment(DeviceImpl<ApiVariationNvn8>* pDevice) {
    int align;
    nvnDeviceGetInteger(pDevice->ToData()->pNvnDevice,
                        NVN_DEVICE_INFO_COMMAND_BUFFER_CONTROL_ALIGNMENT, &align);
    return align;
}

/**
 * Constructs an uninitialized command buffer.
 */
CommandBufferImplNvn8::CommandBufferImpl() {
    state = State_NotInitialized;
}

/**
 * Destroys the command buffer. Finalize must have been called beforehand.
 */
CommandBufferImplNvn8::~CommandBufferImpl() {}

/**
 * Initializes the NVN command buffer and installs the memory callback.
 *
 * @param pDevice Device to create the command buffer on.
 * @param rInfo Command buffer description (unused).
 */
void CommandBufferImplNvn8::Initialize(DeviceImpl<ApiVariationNvn8>* pDevice,
                                       const InfoType& rInfo) {
    pNnDevice = pDevice;
    NVNdevice* pNvnDevice = pDevice->ToData()->pNvnDevice;

    pNvnCommandBuffer = &nvnCommandBuffer;

    nvnCommandBufferInitialize(pNvnCommandBuffer, pNvnDevice);

    pOutOfCommandMemoryCallback = nullptr;
    pOutOfControlMemoryCallback = nullptr;

    nvnCommandBufferSetMemoryCallback(pNvnCommandBuffer, CommandBufferMemoryCallbackProcedure);
    nvnCommandBufferSetMemoryCallbackData(pNvnCommandBuffer, this);

    hNvnCommandBuffer = 0;

    flags.SetBit(Flag_ConservativeRasterSupported, pDevice->ToData()->supportedFeatures.GetBit(
                                                       NvnDeviceFeature_SupportConservativeRaster));
    flags.SetBit(Flag_Shared, false);

    state = State_Initialized;
}

/**
 * Releases the recorded command handle and finalizes the NVN command buffer.
 *
 * @param pDevice Device the command buffer was created on (unused).
 */
void CommandBufferImplNvn8::Finalize(DeviceImpl<ApiVariationNvn8>* pDevice) {
    if (hNvnCommandBuffer != 0) {
        nvnDeviceFinalizeCommandHandle(pNnDevice->ToData()->pNvnDevice, hNvnCommandBuffer);
        hNvnCommandBuffer = 0;
    }

    nvnCommandBufferFinalize(pNvnCommandBuffer);
    state = State_NotInitialized;
}

/**
 * Hands a range of a memory pool to the command buffer as command memory.
 *
 * @param pMemoryPool Memory pool holding the command memory.
 * @param offset Offset of the range in the pool.
 * @param size Size of the range in bytes.
 */
void CommandBufferImplNvn8::AddCommandMemory(MemoryPoolImpl<ApiVariationNvn8>* pMemoryPool,
                                             ptrdiff_t offset, size_t size) {
    nvnCommandBufferAddCommandMemory(pNvnCommandBuffer, pMemoryPool->ToData()->pNvnMemoryPool,
                                     offset, size);
}

/**
 * Hands CPU memory to the command buffer as control memory.
 *
 * @param pMemory Control memory.
 * @param size Size of the memory in bytes.
 */
void CommandBufferImplNvn8::AddControlMemory(void* pMemory, size_t size) {
    nvnCommandBufferAddControlMemory(pNvnCommandBuffer, pMemory, size);
}

/**
 * Sets the callback invoked when the command buffer runs out of command memory.
 *
 * @param pCallback Callback to invoke.
 */
void CommandBufferImplNvn8::SetOutOfCommandMemoryEventCallback(
    OutOfMemoryEventCallback pCallback) {
    pOutOfCommandMemoryCallback = reinterpret_cast<void (*)()>(pCallback);
}

/**
 * Sets the callback invoked when the command buffer runs out of control memory.
 *
 * @param pCallback Callback to invoke.
 */
void CommandBufferImplNvn8::SetOutOfControlMemoryEventCallback(
    OutOfMemoryEventCallback pCallback) {
    pOutOfControlMemoryCallback = reinterpret_cast<void (*)()>(pCallback);
}

/**
 * Resets the command buffer (no-op on NVN).
 */
void CommandBufferImplNvn8::Reset() {}

/**
 * Starts recording commands, releasing the previously recorded command handle.
 */
void CommandBufferImplNvn8::Begin() {
    if (hNvnCommandBuffer != 0) {
        nvnDeviceFinalizeCommandHandle(pNnDevice->ToData()->pNvnDevice, hNvnCommandBuffer);
        hNvnCommandBuffer = 0;
    }

    nvnCommandBufferBeginRecording(pNvnCommandBuffer);
    state = State_Begun;
}

/**
 * Ends recording and keeps the resulting command handle.
 */
void CommandBufferImplNvn8::End() {
    hNvnCommandBuffer = nvnCommandBufferEndRecording(pNvnCommandBuffer);
    state = State_Initialized;
}

/**
 * Dispatches a compute workload.
 *
 * @param groupCountX Number of work groups along X.
 * @param groupCountY Number of work groups along Y.
 * @param groupCountZ Number of work groups along Z.
 */
void CommandBufferImplNvn8::Dispatch(int groupCountX, int groupCountY, int groupCountZ) {
    nvnCommandBufferDispatchCompute(pNvnCommandBuffer, groupCountX, groupCountY, groupCountZ);
}

/**
 * Draws non-indexed primitives.
 *
 * @param primitiveTopology Primitive topology.
 * @param vertexCount Number of vertices.
 * @param vertexOffset First vertex.
 */
void CommandBufferImplNvn8::Draw(PrimitiveTopology primitiveTopology, int vertexCount,
                                 int vertexOffset) {
    nvnCommandBufferDrawArrays(pNvnCommandBuffer, Nvn::GetDrawPrimitive(primitiveTopology),
                               vertexOffset, vertexCount);
}

/**
 * Draws instanced non-indexed primitives.
 *
 * @param primitiveTopology Primitive topology.
 * @param vertexCountPerInstance Number of vertices per instance.
 * @param vertexOffset First vertex.
 * @param instanceCount Number of instances.
 * @param baseInstance First instance.
 */
void CommandBufferImplNvn8::Draw(PrimitiveTopology primitiveTopology, int vertexCountPerInstance,
                                 int vertexOffset, int instanceCount, int baseInstance) {
    nvnCommandBufferDrawArraysInstanced(pNvnCommandBuffer, Nvn::GetDrawPrimitive(primitiveTopology),
                                        vertexOffset, vertexCountPerInstance, baseInstance,
                                        instanceCount);
}

/**
 * Draws indexed primitives.
 *
 * @param primitiveTopology Primitive topology.
 * @param indexFormat Format of the indices.
 * @param rIndexBufferAddress GPU address of the index buffer.
 * @param indexCount Number of indices.
 * @param baseVertex Value added to every index.
 */
void CommandBufferImplNvn8::DrawIndexed(PrimitiveTopology primitiveTopology,
                                        IndexFormat indexFormat,
                                        const GpuAddress& rIndexBufferAddress, int indexCount,
                                        int baseVertex) {
    nvnCommandBufferDrawElementsBaseVertex(pNvnCommandBuffer,
                                           Nvn::GetDrawPrimitive(primitiveTopology),
                                           Nvn::GetIndexFormat(indexFormat), indexCount,
                                           Nvn::GetBufferAddress(rIndexBufferAddress), baseVertex);
}

/**
 * Draws instanced indexed primitives.
 *
 * @param primitiveTopology Primitive topology.
 * @param indexFormat Format of the indices.
 * @param rIndexBufferAddress GPU address of the index buffer.
 * @param indexCountPerInstance Number of indices per instance.
 * @param baseVertex Value added to every index.
 * @param instanceCount Number of instances.
 * @param baseInstance First instance.
 */
void CommandBufferImplNvn8::DrawIndexed(PrimitiveTopology primitiveTopology,
                                        IndexFormat indexFormat,
                                        const GpuAddress& rIndexBufferAddress,
                                        int indexCountPerInstance, int baseVertex,
                                        int instanceCount, int baseInstance) {
    nvnCommandBufferDrawElementsInstanced(
        pNvnCommandBuffer, Nvn::GetDrawPrimitive(primitiveTopology),
        Nvn::GetIndexFormat(indexFormat), indexCountPerInstance,
        Nvn::GetBufferAddress(rIndexBufferAddress), baseVertex, baseInstance, instanceCount);
}

/**
 * Dispatches a compute workload whose group counts are read from GPU memory.
 *
 * @param rIndirectBufferAddress GPU address of the dispatch arguments.
 */
void CommandBufferImplNvn8::DispatchIndirect(const GpuAddress& rIndirectBufferAddress) {
    nvnCommandBufferDispatchComputeIndirect(pNvnCommandBuffer,
                                            Nvn::GetBufferAddress(rIndirectBufferAddress));
}

/**
 * Draws non-indexed primitives with arguments read from GPU memory.
 *
 * @param primitiveTopology Primitive topology.
 * @param rIndirectBufferAddress GPU address of the draw arguments.
 */
void CommandBufferImplNvn8::DrawIndirect(PrimitiveTopology primitiveTopology,
                                         const GpuAddress& rIndirectBufferAddress) {
    nvnCommandBufferDrawArraysIndirect(pNvnCommandBuffer, Nvn::GetDrawPrimitive(primitiveTopology),
                                       Nvn::GetBufferAddress(rIndirectBufferAddress));
}

/**
 * Draws indexed primitives with arguments read from GPU memory.
 *
 * @param primitiveTopology Primitive topology.
 * @param indexFormat Format of the indices.
 * @param rIndexBufferAddress GPU address of the index buffer.
 * @param rIndirectBufferAddress GPU address of the draw arguments.
 */
void CommandBufferImplNvn8::DrawIndexedIndirect(PrimitiveTopology primitiveTopology,
                                                IndexFormat indexFormat,
                                                const GpuAddress& rIndexBufferAddress,
                                                const GpuAddress& rIndirectBufferAddress) {
    nvnCommandBufferDrawElementsIndirect(pNvnCommandBuffer,
                                         Nvn::GetDrawPrimitive(primitiveTopology),
                                         Nvn::GetIndexFormat(indexFormat),
                                         Nvn::GetBufferAddress(rIndexBufferAddress),
                                         Nvn::GetBufferAddress(rIndirectBufferAddress));
}

/**
 * Issues several indirect non-indexed draws whose count is read from GPU memory.
 *
 * @param primitiveTopology Primitive topology.
 * @param rIndirectBufferAddress GPU address of the draw argument array.
 * @param rCountBufferAddress GPU address of the draw count.
 * @param maxDrawCount Maximum number of draws.
 * @param stride Stride between draw arguments in bytes.
 */
void CommandBufferImplNvn8::MultiDrawIndirectCount(PrimitiveTopology primitiveTopology,
                                                   const GpuAddress& rIndirectBufferAddress,
                                                   const GpuAddress& rCountBufferAddress,
                                                   int maxDrawCount, ptrdiff_t stride) {
    nvnCommandBufferMultiDrawArraysIndirectCount(
        pNvnCommandBuffer, Nvn::GetDrawPrimitive(primitiveTopology),
        Nvn::GetBufferAddress(rIndirectBufferAddress), Nvn::GetBufferAddress(rCountBufferAddress),
        maxDrawCount, stride);
}

/**
 * Issues several indirect indexed draws whose count is read from GPU memory.
 *
 * @param primitiveTopology Primitive topology.
 * @param indexFormat Format of the indices.
 * @param rIndexBufferAddress GPU address of the index buffer.
 * @param rIndirectBufferAddress GPU address of the draw argument array.
 * @param rCountBufferAddress GPU address of the draw count.
 * @param maxDrawCount Maximum number of draws.
 * @param stride Stride between draw arguments in bytes.
 */
void CommandBufferImplNvn8::MultiDrawIndexedIndirectCount(
    PrimitiveTopology primitiveTopology, IndexFormat indexFormat,
    const GpuAddress& rIndexBufferAddress, const GpuAddress& rIndirectBufferAddress,
    const GpuAddress& rCountBufferAddress, int maxDrawCount, ptrdiff_t stride) {
    nvnCommandBufferMultiDrawElementsIndirectCount(
        pNvnCommandBuffer, Nvn::GetDrawPrimitive(primitiveTopology),
        Nvn::GetIndexFormat(indexFormat), Nvn::GetBufferAddress(rIndexBufferAddress),
        Nvn::GetBufferAddress(rIndirectBufferAddress), Nvn::GetBufferAddress(rCountBufferAddress),
        maxDrawCount, stride);
}

/**
 * Binds all states and the shader of a pipeline.
 *
 * @param pPipeline Pipeline to bind.
 */
void CommandBufferImplNvn8::SetPipeline(const PipelineImpl<ApiVariationNvn8>* pPipeline) {
    const PipelineImplData<ApiVariationNvn8>& rPipeline = pPipeline->ToData();

    if (rPipeline.nnPipelineType == rPipeline.PipelineType_Graphics) {
        SetRasterizerState(nn::gfx::DataToAccessor(rPipeline.nnRasterizerState));
        SetBlendState(nn::gfx::DataToAccessor(rPipeline.nnBlendState));
        SetDepthStencilState(nn::gfx::DataToAccessor(rPipeline.nnDepthStencilState));
        SetVertexState(nn::gfx::DataToAccessor(rPipeline.nnVertexState));

        if (rPipeline.flags.GetBit(rPipeline.Flag_HasTessellationState)) {
            SetTessellationState(nn::gfx::DataToAccessor(rPipeline.nnTessellationState));
        }
    }

    SetShader(rPipeline.pShader, ShaderStageBit_All);
}

/**
 * Binds a rasterizer state.
 *
 * @param pRasterizerState Rasterizer state to bind.
 */
void CommandBufferImplNvn8::SetRasterizerState(
    const RasterizerStateImpl<ApiVariationNvn8>* pRasterizerState) {
    const RasterizerStateImplData<ApiVariationNvn8>& rState = pRasterizerState->ToData();

    nvnCommandBufferBindPolygonState(
        pNvnCommandBuffer, reinterpret_cast<const NVNpolygonState*>(&rState.nvnPolygonState));
    nvnCommandBufferSetPolygonOffsetClamp(pNvnCommandBuffer, rState.nvnSlopeScaledDepthBias,
                                          rState.nvnDepthBias, rState.nvnDepthBiasClamp);
    nvnCommandBufferBindMultisampleState(
        pNvnCommandBuffer,
        reinterpret_cast<const NVNmultisampleState*>(&rState.nvnMultisampleState));

    if (rState.flags.GetBit(rState.Flag_MultisampleEnabled)) {
        nvnCommandBufferSetSampleMask(pNvnCommandBuffer, rState.nvnSampleMask);
    }

    nvnCommandBufferSetDepthClamp(pNvnCommandBuffer,
                                  !rState.flags.GetBit(rState.Flag_DepthClipEnabled));
    nvnCommandBufferSetRasterizerDiscard(pNvnCommandBuffer,
                                         !rState.flags.GetBit(rState.Flag_RasterEnabled));

    if (flags.GetBit(Flag_ConservativeRasterSupported)) {
        nvnCommandBufferSetConservativeRasterEnable(
            pNvnCommandBuffer, rState.flags.GetBit(rState.Flag_ConservativeRasterEnabled));
    }
}

/**
 * Binds a blend state.
 *
 * @param pBlendState Blend state to bind.
 */
void CommandBufferImplNvn8::SetBlendState(const BlendStateImpl<ApiVariationNvn8>* pBlendState) {
    const BlendStateImplData<ApiVariationNvn8>& rState = pBlendState->ToData();
    const NVNblendState* pBlendStates = rState.pNvnBlendStateData;

    for (int i = 0, targetCount = rState.targetCount; i < targetCount; ++i) {
        nvnCommandBufferBindBlendState(pNvnCommandBuffer, pBlendStates + i);
    }

    nvnCommandBufferBindChannelMaskState(
        pNvnCommandBuffer,
        reinterpret_cast<const NVNchannelMaskState*>(&rState.nvnChannelMaskState));
    nvnCommandBufferBindColorState(
        pNvnCommandBuffer, reinterpret_cast<const NVNcolorState*>(&rState.nvnColorState));
    nvnCommandBufferSetBlendColor(pNvnCommandBuffer, rState.nvnBlendConstant);
}

/**
 * Binds a depth/stencil state.
 *
 * @param pDepthStencilState Depth/stencil state to bind.
 */
void CommandBufferImplNvn8::SetDepthStencilState(
    const DepthStencilStateImpl<ApiVariationNvn8>* pDepthStencilState) {
    const DepthStencilStateImplData<ApiVariationNvn8>& rState = pDepthStencilState->ToData();

    nvnCommandBufferBindDepthStencilState(
        pNvnCommandBuffer,
        reinterpret_cast<const NVNdepthStencilState*>(rState.nvnDepthStencilState));
    nvnCommandBufferSetStencilValueMask(pNvnCommandBuffer, NVN_FACE_FRONT_AND_BACK,
                                        rState.nvnStencilValueMask);
    nvnCommandBufferSetStencilMask(pNvnCommandBuffer, NVN_FACE_FRONT_AND_BACK,
                                   rState.nvnStencilMask);
    nvnCommandBufferSetStencilRef(pNvnCommandBuffer, NVN_FACE_BACK, rState.nvnStencilBackRef);
    nvnCommandBufferSetStencilRef(pNvnCommandBuffer, NVN_FACE_FRONT, rState.nvnStencilFrontRef);

    if (!rState.flag.GetBit(rState.Flag_DepthBoundsTestEnable)) {
        nvnCommandBufferSetDepthBounds(pNvnCommandBuffer, false, 0.0f, 1.0f);
    }
}

/**
 * Binds a vertex state.
 *
 * @param pVertexState Vertex state to bind.
 */
void CommandBufferImplNvn8::SetVertexState(const VertexStateImpl<ApiVariationNvn8>* pVertexState) {
    const VertexStateImplData<ApiVariationNvn8>& rState = pVertexState->ToData();

    nvnCommandBufferBindVertexAttribState(pNvnCommandBuffer, rState.vertexAttributeStateCount,
                                          rState.pNvnVertexAttribState);
    nvnCommandBufferBindVertexStreamState(pNvnCommandBuffer, rState.vertexStreamStateCount,
                                          rState.pNvnVertexStreamState);
}

/**
 * Binds a tessellation state.
 *
 * @param pTessellationState Tessellation state to bind.
 */
void CommandBufferImplNvn8::SetTessellationState(
    const TessellationStateImpl<ApiVariationNvn8>* pTessellationState) {
    const TessellationStateImplData<ApiVariationNvn8>& rState = pTessellationState->ToData();

    nvnCommandBufferSetPatchSize(pNvnCommandBuffer, rState.patchSize);
}

/**
 * Binds a shader program.
 *
 * @param pShader Shader to bind, or nullptr to unbind the given stages.
 * @param stageBits gfx shader stage bits to bind for separable (or null) shaders.
 */
void CommandBufferImplNvn8::SetShader(const ShaderImpl<ApiVariationNvn8>* pShader, int stageBits) {
    NVNprogram* pProgram = nullptr;
    int shaderStageBits = 0;

    if (pShader != nullptr) {
        pProgram = pShader->ToData()->pNvnProgram;
    }

    if (pShader != nullptr &&
        !pShader->ToData()->flags.GetBit(pShader->ToData()->Flag_SeparationEnable)) {
        shaderStageBits = pShader->ToData()->nvnShaderStageBits;
    } else {
        shaderStageBits = Nvn::GetShaderStageBits(stageBits);
    }

    nvnCommandBufferBindProgram(pNvnCommandBuffer, pProgram, shaderStageBits);
}

/**
 * Binds color and depth/stencil render targets.
 *
 * @param colorTargetCount Number of color targets.
 * @param ppColorTargets Color target views; null entries are unbound.
 * @param pDepthStencil Depth/stencil view, or nullptr.
 */
void CommandBufferImplNvn8::SetRenderTargets(
    int colorTargetCount, const ColorTargetViewImpl<ApiVariationNvn8>* const* ppColorTargets,
    const DepthStencilViewImpl<ApiVariationNvn8>* pDepthStencil) {
    const int MaxRenderTarget = 8;

    NVNtexture* pNvnColorTargets[MaxRenderTarget] = {};
    NVNtextureView* pNvnColorTargetViews[MaxRenderTarget] = {};

    for (int idxTarget = 0; idxTarget < colorTargetCount; ++idxTarget) {
        const ColorTargetViewImpl<ApiVariationNvn8>* pColorTarget = ppColorTargets[idxTarget];

        if (pColorTarget != nullptr) {
            pNvnColorTargets[idxTarget] = pColorTarget->ToData()->pNvnTexture;
            pNvnColorTargetViews[idxTarget] = pColorTarget->ToData()->pNvnTextureView;
        }
    }

    NVNtexture* pDepthTarget = nullptr;
    NVNtextureView* pDepthTargetView = nullptr;

    if (pDepthStencil != nullptr) {
        pDepthTarget = pDepthStencil->ToData()->pNvnTexture;
        pDepthTargetView = pDepthStencil->ToData()->pNvnTextureView;
    }

    nvnCommandBufferSetRenderTargets(pNvnCommandBuffer, colorTargetCount, pNvnColorTargets,
                                     pNvnColorTargetViews, pDepthTarget, pDepthTargetView);
}

/**
 * Binds a vertex buffer.
 *
 * @param bufferIndex Vertex buffer binding index.
 * @param rVertexBuffer GPU address of the vertex data.
 * @param stride Vertex stride (unused; part of the vertex state on NVN).
 * @param size Size of the vertex data in bytes.
 */
void CommandBufferImplNvn8::SetVertexBuffer(int bufferIndex, const GpuAddress& rVertexBuffer,
                                            ptrdiff_t stride, size_t size) {
    nvnCommandBufferBindVertexBuffer(pNvnCommandBuffer, bufferIndex,
                                     Nvn::GetBufferAddress(rVertexBuffer), size);
}

/**
 * Binds viewports, depth ranges and scissors.
 *
 * @param pViewportScissorState Viewport/scissor state to bind.
 */
void CommandBufferImplNvn8::SetViewportScissorState(
    const ViewportScissorStateImpl<ApiVariationNvn8>* pViewportScissorState) {
    const ViewportScissorStateImplData<ApiVariationNvn8>& rState =
        pViewportScissorState->ToData();

    nvnCommandBufferSetDepthRange(pNvnCommandBuffer, rState.depthRange[0], rState.depthRange[1]);
    nvnCommandBufferSetViewports(pNvnCommandBuffer, 0, 1, rState.viewport);
    nvnCommandBufferSetScissors(pNvnCommandBuffer, 0, 1, rState.scissor);

    int viewportCount = rState.viewportCount;

    if (viewportCount > 1) {
        int extraViewportCount = viewportCount - 1;

        nn::util::BytePtr ptr(rState.pWorkMemory.ptr);

        float* pViewportArray = ptr.Get<float>();
        float* pDepthRangeArray = ptr.Advance(16 * extraViewportCount).Get<float>();
        int32_t* pScissorArray = ptr.Advance(8 * extraViewportCount).Get<int32_t>();

        nvnCommandBufferSetViewports(pNvnCommandBuffer, 1, extraViewportCount, pViewportArray);
        nvnCommandBufferSetDepthRanges(pNvnCommandBuffer, 1, extraViewportCount,
                                       pDepthRangeArray);
        nvnCommandBufferSetScissors(pNvnCommandBuffer, 1, extraViewportCount, pScissorArray);
    }
}

/**
 * Copies a range of one buffer into another.
 *
 * @param pDstBuffer Destination buffer.
 * @param dstOffset Offset in the destination buffer.
 * @param pSrcBuffer Source buffer.
 * @param srcOffset Offset in the source buffer.
 * @param size Number of bytes to copy.
 */
void CommandBufferImplNvn8::CopyBuffer(BufferImpl<ApiVariationNvn8>* pDstBuffer,
                                       ptrdiff_t dstOffset,
                                       const BufferImpl<ApiVariationNvn8>* pSrcBuffer,
                                       ptrdiff_t srcOffset, size_t size) {
    NVNbufferAddress src = nvnBufferGetAddress(pSrcBuffer->ToData()->pNvnBuffer) + srcOffset;
    NVNbufferAddress dst = nvnBufferGetAddress(pDstBuffer->ToData()->pNvnBuffer) + dstOffset;
    nvnCommandBufferCopyBufferToBuffer(pNvnCommandBuffer, src, dst, size, NVN_COPY_FLAGS_NONE);
}

/**
 * Copies a region of one texture into another.
 *
 * @param pDstTexture Destination texture.
 * @param rDstSubresource Destination mip level / array layer.
 * @param dstOffsetU Destination X offset.
 * @param dstOffsetV Destination Y offset.
 * @param dstOffsetW Destination Z offset.
 * @param pSrcTexture Source texture.
 * @param rSrcCopyRegion Source region.
 */
void CommandBufferImplNvn8::CopyImage(TextureImpl<ApiVariationNvn8>* pDstTexture,
                                      const TextureSubresource& rDstSubresource, int dstOffsetU,
                                      int dstOffsetV, int dstOffsetW,
                                      const TextureImpl<ApiVariationNvn8>* pSrcTexture,
                                      const TextureCopyRegion& rSrcCopyRegion) {
    NVNtextureTarget srcTarget = nvnTextureGetTarget(pSrcTexture->ToData()->pNvnTexture);
    NVNtextureTarget dstTarget = nvnTextureGetTarget(pDstTexture->ToData()->pNvnTexture);

    int srcV;
    int srcHeight;
    int srcW;
    int srcDepth;
    GetNvnCopyRegion(&srcV, &srcHeight, &srcW, &srcDepth, rSrcCopyRegion, srcTarget);

    NVNcopyRegion srcRegion;
    srcRegion.xoffset = rSrcCopyRegion.GetOffsetU();
    srcRegion.yoffset = srcV;
    srcRegion.zoffset = srcW;
    srcRegion.width = rSrcCopyRegion.GetWidth();
    srcRegion.height = srcHeight;
    srcRegion.depth = srcDepth;

    NVNcopyRegion dstRegion;
    dstRegion.xoffset = dstOffsetU;
    dstRegion.yoffset = dstTarget == NVN_TEXTURE_TARGET_1D_ARRAY ?
                            rDstSubresource.GetArrayIndex() :
                            dstOffsetV;
    dstRegion.zoffset = (dstTarget == NVN_TEXTURE_TARGET_2D_ARRAY ||
                         dstTarget == NVN_TEXTURE_TARGET_2D_MULTISAMPLE_ARRAY) ?
                            rDstSubresource.GetArrayIndex() :
                            dstOffsetW;
    dstRegion.width = srcRegion.width;
    dstRegion.height = srcRegion.height;
    dstRegion.depth = srcRegion.depth;

    NVNtextureView srcView;
    nvnTextureViewSetDefaults(&srcView);
    nvnTextureViewSetLevels(&srcView, rSrcCopyRegion.GetSubresource().GetMipLevel(), 1);

    NVNtextureView dstView;
    nvnTextureViewSetDefaults(&dstView);
    nvnTextureViewSetLevels(&dstView, rDstSubresource.GetMipLevel(), 1);

    nvnCommandBufferCopyTextureToTexture(pNvnCommandBuffer, pSrcTexture->ToData()->pNvnTexture,
                                         &srcView, &srcRegion, pDstTexture->ToData()->pNvnTexture,
                                         &dstView, &dstRegion, NVN_COPY_FLAGS_NONE);
}

/**
 * Copies buffer data into a texture region, honoring the buffer image layout.
 *
 * @param pDstTexture Destination texture.
 * @param pSrcBuffer Source buffer.
 * @param rCopyRegion Buffer layout and texture region.
 */
void CommandBufferImplNvn8::CopyBufferToImage(TextureImpl<ApiVariationNvn8>* pDstTexture,
                                              const BufferImpl<ApiVariationNvn8>* pSrcBuffer,
                                              const BufferTextureCopyRegion& rCopyRegion) {
    NVNtextureTarget target = nvnTextureGetTarget(pDstTexture->ToData()->pNvnTexture);

    const TextureCopyRegion& rDstRegion = rCopyRegion.GetTextureCopyRegion();

    int offsetY;
    int height;
    int offsetZ;
    int depth;
    GetNvnCopyRegion(&offsetY, &height, &offsetZ, &depth, rDstRegion, target);

    NVNcopyRegion region;
    region.xoffset = rCopyRegion.GetTextureCopyRegion().GetOffsetU();
    region.yoffset = offsetY;
    region.zoffset = offsetZ;
    region.width = rCopyRegion.GetTextureCopyRegion().GetWidth();
    region.height = height;
    region.depth = depth;

    NVNtextureView view;
    nvnTextureViewSetDefaults(&view);
    nvnTextureViewSetLevels(&view,
                            rCopyRegion.GetTextureCopyRegion().GetSubresource().GetMipLevel(), 1);

    NVNbufferAddress bufferAddress =
        nvnBufferGetAddress(pSrcBuffer->ToData()->pNvnBuffer) + rCopyRegion.GetBufferOffset();

    ptrdiff_t rowStride;
    ptrdiff_t imageStride;
    GetNvnCopyStride(&rowStride, &imageStride, rCopyRegion, pDstTexture->ToData()->pNvnTexture);

    nvnCommandBufferSetCopyRowStride(pNvnCommandBuffer, rowStride);
    nvnCommandBufferSetCopyImageStride(pNvnCommandBuffer, imageStride);
    nvnCommandBufferCopyBufferToTexture(pNvnCommandBuffer, bufferAddress,
                                        pDstTexture->ToData()->pNvnTexture, &view, &region,
                                        NVN_COPY_FLAGS_NONE);
}

/**
 * Copies a texture region into a buffer, honoring the buffer image layout.
 *
 * @param pDstBuffer Destination buffer.
 * @param pSrcTexture Source texture.
 * @param rCopyRegion Buffer layout and texture region.
 */
void CommandBufferImplNvn8::CopyImageToBuffer(BufferImpl<ApiVariationNvn8>* pDstBuffer,
                                              const TextureImpl<ApiVariationNvn8>* pSrcTexture,
                                              const BufferTextureCopyRegion& rCopyRegion) {
    NVNtextureTarget target = nvnTextureGetTarget(pSrcTexture->ToData()->pNvnTexture);

    const TextureCopyRegion& rSrcRegion = rCopyRegion.GetTextureCopyRegion();

    int offsetY;
    int height;
    int offsetZ;
    int depth;
    GetNvnCopyRegion(&offsetY, &height, &offsetZ, &depth, rSrcRegion, target);

    NVNcopyRegion region;
    region.xoffset = rSrcRegion.GetOffsetU();
    region.yoffset = offsetY;
    region.zoffset = offsetZ;
    region.width = rSrcRegion.GetWidth();
    region.height = height;
    region.depth = depth;

    NVNtextureView view;
    nvnTextureViewSetDefaults(&view);
    nvnTextureViewSetLevels(&view, rSrcRegion.GetSubresource().GetMipLevel(), 1);

    NVNbufferAddress bufferAddress =
        nvnBufferGetAddress(pDstBuffer->ToData()->pNvnBuffer) + rCopyRegion.GetBufferOffset();

    ptrdiff_t rowStride;
    ptrdiff_t imageStride;
    GetNvnCopyStride(&rowStride, &imageStride, rCopyRegion, pSrcTexture->ToData()->pNvnTexture);

    nvnCommandBufferSetCopyRowStride(pNvnCommandBuffer, rowStride);
    nvnCommandBufferSetCopyImageStride(pNvnCommandBuffer, imageStride);
    nvnCommandBufferCopyTextureToBuffer(pNvnCommandBuffer, pSrcTexture->ToData()->pNvnTexture,
                                        &view, &region, bufferAddress, NVN_COPY_FLAGS_NONE);
}

/**
 * Copies tightly packed buffer data into a texture region.
 *
 * @param pDstTexture Destination texture.
 * @param rDstRegion Destination region.
 * @param pSrcBuffer Source buffer.
 * @param srcOffset Offset in the source buffer.
 */
void CommandBufferImplNvn8::CopyBufferToImage(TextureImpl<ApiVariationNvn8>* pDstTexture,
                                              const TextureCopyRegion& rDstRegion,
                                              const BufferImpl<ApiVariationNvn8>* pSrcBuffer,
                                              ptrdiff_t srcOffset) {
    NVNtextureTarget target = nvnTextureGetTarget(pDstTexture->ToData()->pNvnTexture);

    int offsetY;
    int height;
    int offsetZ;
    int depth;
    GetNvnCopyRegion(&offsetY, &height, &offsetZ, &depth, rDstRegion, target);

    NVNcopyRegion region;
    region.xoffset = rDstRegion.GetOffsetU();
    region.yoffset = offsetY;
    region.zoffset = offsetZ;
    region.width = rDstRegion.GetWidth();
    region.height = height;
    region.depth = depth;

    NVNtextureView view;
    nvnTextureViewSetDefaults(&view);
    nvnTextureViewSetLevels(&view, rDstRegion.GetSubresource().GetMipLevel(), 1);

    NVNbufferAddress bufferAddress =
        nvnBufferGetAddress(pSrcBuffer->ToData()->pNvnBuffer) + srcOffset;

    nvnCommandBufferSetCopyRowStride(pNvnCommandBuffer, 0);
    nvnCommandBufferSetCopyImageStride(pNvnCommandBuffer, 0);
    nvnCommandBufferCopyBufferToTexture(pNvnCommandBuffer, bufferAddress,
                                        pDstTexture->ToData()->pNvnTexture, &view, &region,
                                        NVN_COPY_FLAGS_NONE);
}

/**
 * Copies a texture region into a buffer, tightly packed.
 *
 * @param pDstBuffer Destination buffer.
 * @param dstOffset Offset in the destination buffer.
 * @param pSrcTexture Source texture.
 * @param rSrcRegion Source region.
 */
void CommandBufferImplNvn8::CopyImageToBuffer(BufferImpl<ApiVariationNvn8>* pDstBuffer,
                                              ptrdiff_t dstOffset,
                                              const TextureImpl<ApiVariationNvn8>* pSrcTexture,
                                              const TextureCopyRegion& rSrcRegion) {
    NVNtextureTarget target = nvnTextureGetTarget(pSrcTexture->ToData()->pNvnTexture);

    int offsetY;
    int height;
    int offsetZ;
    int depth;
    GetNvnCopyRegion(&offsetY, &height, &offsetZ, &depth, rSrcRegion, target);

    NVNcopyRegion region;
    region.xoffset = rSrcRegion.GetOffsetU();
    region.yoffset = offsetY;
    region.zoffset = offsetZ;
    region.width = rSrcRegion.GetWidth();
    region.height = height;
    region.depth = depth;

    NVNtextureView view;
    nvnTextureViewSetDefaults(&view);
    nvnTextureViewSetLevels(&view, rSrcRegion.GetSubresource().GetMipLevel(), 1);

    NVNbufferAddress bufferAddress =
        nvnBufferGetAddress(pDstBuffer->ToData()->pNvnBuffer) + dstOffset;

    nvnCommandBufferSetCopyRowStride(pNvnCommandBuffer, 0);
    nvnCommandBufferSetCopyImageStride(pNvnCommandBuffer, 0);
    nvnCommandBufferCopyTextureToBuffer(pNvnCommandBuffer, pSrcTexture->ToData()->pNvnTexture,
                                        &view, &region, bufferAddress, NVN_COPY_FLAGS_NONE);
}

/**
 * Copies a texture region into another, scaling and filtering if the sizes differ.
 *
 * @param pDstTexture Destination texture.
 * @param rDstCopyRegion Destination region.
 * @param pSrcTexture Source texture.
 * @param rSrcCopyRegion Source region.
 * @param copyFlags gfx copy flags (bit 0 = linear filter).
 */
void CommandBufferImplNvn8::BlitImage(TextureImpl<ApiVariationNvn8>* pDstTexture,
                                      const TextureCopyRegion& rDstCopyRegion,
                                      const TextureImpl<ApiVariationNvn8>* pSrcTexture,
                                      const TextureCopyRegion& rSrcCopyRegion, int copyFlags) {
    NVNtextureTarget dstTarget = nvnTextureGetTarget(pDstTexture->ToData()->pNvnTexture);

    int dstV;
    int dstHeight;
    int dstW;
    int dstDepth;
    GetNvnCopyRegion(&dstV, &dstHeight, &dstW, &dstDepth, rDstCopyRegion, dstTarget);

    NVNtextureTarget srcTarget = nvnTextureGetTarget(pSrcTexture->ToData()->pNvnTexture);

    int srcV;
    int srcHeight;
    int srcW;
    int srcDepth;
    GetNvnCopyRegion(&srcV, &srcHeight, &srcW, &srcDepth, rSrcCopyRegion, srcTarget);

    NVNcopyRegion dstRegion;
    dstRegion.xoffset = rDstCopyRegion.GetOffsetU();
    dstRegion.yoffset = dstV;
    dstRegion.zoffset = dstW;
    dstRegion.width = rDstCopyRegion.GetWidth();
    dstRegion.height = dstHeight;
    dstRegion.depth = dstDepth;

    NVNcopyRegion srcRegion;
    srcRegion.xoffset = rSrcCopyRegion.GetOffsetU();
    srcRegion.yoffset = srcV;
    srcRegion.zoffset = srcW;
    srcRegion.width = rSrcCopyRegion.GetWidth();
    srcRegion.height = srcHeight;
    srcRegion.depth = srcDepth;

    NVNtextureView dstView;
    nvnTextureViewSetDefaults(&dstView);
    nvnTextureViewSetLevels(&dstView, rDstCopyRegion.GetSubresource().GetMipLevel(), 1);

    NVNtextureView srcView;
    nvnTextureViewSetDefaults(&srcView);
    nvnTextureViewSetLevels(&srcView, rSrcCopyRegion.GetSubresource().GetMipLevel(), 1);

    int nvnCopyFlags = 0;
    nvnCopyFlags |= copyFlags & 1;
    nvnCommandBufferCopyTextureToTexture(pNvnCommandBuffer, pSrcTexture->ToData()->pNvnTexture,
                                         &srcView, &srcRegion, pDstTexture->ToData()->pNvnTexture,
                                         &dstView, &dstRegion, nvnCopyFlags);
}

/**
 * Fills a buffer range with a 32-bit value.
 *
 * @param pBuffer Buffer to fill.
 * @param offset Offset of the range.
 * @param size Size of the range in bytes.
 * @param value Fill value.
 */
void CommandBufferImplNvn8::ClearBuffer(BufferImpl<ApiVariationNvn8>* pBuffer, ptrdiff_t offset,
                                        size_t size, uint32_t value) {
    NVNbufferAddress bufferAddress = nvnBufferGetAddress(pBuffer->ToData()->pNvnBuffer) + offset;
    nvnCommandBufferClearBuffer(pNvnCommandBuffer, bufferAddress, size, value);
}

/**
 * Clears a color target to a floating-point color.
 *
 * @param pColorTarget Color target to clear.
 * @param red Red component.
 * @param green Green component.
 * @param blue Blue component.
 * @param alpha Alpha component.
 * @param pArrayRange Array layers to clear, or nullptr for the whole view.
 */
void CommandBufferImplNvn8::ClearColor(ColorTargetViewImpl<ApiVariationNvn8>* pColorTarget,
                                       float red, float green, float blue, float alpha,
                                       const TextureArrayRange* pArrayRange) {
    ClearColorValue clearColor{{red, green, blue, alpha}};
    ClearColorTarget(pColorTarget, clearColor, pArrayRange);
}

/**
 * Clears a color target.
 *
 * @param pColorTarget Color target to clear.
 * @param rClearColor Clear color.
 * @param pArrayRange Array layers to clear, or nullptr for the whole view.
 */
void CommandBufferImplNvn8::ClearColorTarget(ColorTargetViewImpl<ApiVariationNvn8>* pColorTarget,
                                             const ClearColorValue& rClearColor,
                                             const TextureArrayRange* pArrayRange) {
    const NVNtexture* const pNvnTexture = pColorTarget->ToData()->pNvnTexture;
    const NVNtextureView* const pNvnTextureView = pColorTarget->ToData()->pNvnTextureView;

    NVNcopyRegion region{};

    int level = 0;
    NVNtextureTarget target;
    int layerCount = 0;
    bool hasLayers;

    if (pNvnTextureView != nullptr) {
        int levelCount;
        int minLayer;

        nvnTextureViewGetLevels(pNvnTextureView, &level, &levelCount);
        nvnTextureViewGetTarget(pNvnTextureView, &target);
        hasLayers = nvnTextureViewGetLayers(pNvnTextureView, &minLayer, &layerCount) == true;
    } else {
        target = nvnTextureGetTarget(pNvnTexture);
        hasLayers = false;
    }

    region.width = nvnTextureGetWidth(pNvnTexture);
    region.height = nvnTextureGetHeight(pNvnTexture);
    region.depth = nvnTextureGetDepth(pNvnTexture);

    region.width = std::max(region.width >> level, 1);

    if (target != NVN_TEXTURE_TARGET_1D && target != NVN_TEXTURE_TARGET_1D_ARRAY) {
        region.height = std::max(region.height >> level, 1);

        if (target == NVN_TEXTURE_TARGET_3D) {
            region.depth = std::max(region.depth >> level, 1);

            if (hasLayers) {
                region.depth = layerCount;
            }
        }
    }

    if (pArrayRange != nullptr) {
        if (target == NVN_TEXTURE_TARGET_1D_ARRAY) {
            region.yoffset = pArrayRange->GetBaseArrayIndex();
            region.height = pArrayRange->GetArrayLength();
        } else if (target == NVN_TEXTURE_TARGET_2D_ARRAY ||
                   target == NVN_TEXTURE_TARGET_2D_MULTISAMPLE_ARRAY) {
            region.zoffset = pArrayRange->GetBaseArrayIndex();
            region.depth = pArrayRange->GetArrayLength();
        }
    } else if (hasLayers) {
        if (target == NVN_TEXTURE_TARGET_1D_ARRAY) {
            region.height = layerCount;
        } else if (target == NVN_TEXTURE_TARGET_2D_ARRAY ||
                   target == NVN_TEXTURE_TARGET_2D_MULTISAMPLE_ARRAY) {
            region.depth = layerCount;
        }
    }

    nvnCommandBufferClearTexture(pNvnCommandBuffer, pNvnTexture, pNvnTextureView, &region,
                                 rClearColor.valueFloat, NVN_CLEAR_COLOR_MASK_RGBA);
}

/**
 * Clears a depth/stencil target.
 *
 * @param pDepthStencil Depth/stencil target to clear.
 * @param depth Depth clear value.
 * @param stencil Stencil clear value.
 * @param clearMode Which aspects to clear.
 * @param pArrayRange Array layers to clear (unused).
 */
void CommandBufferImplNvn8::ClearDepthStencil(
    DepthStencilViewImpl<ApiVariationNvn8>* pDepthStencil, float depth, int stencil,
    DepthStencilClearMode clearMode, const TextureArrayRange* pArrayRange) {
    const NVNtexture* const pNvnTexture = pDepthStencil->ToData()->pNvnTexture;
    const NVNtextureView* const pNvnTextureView = pDepthStencil->ToData()->pNvnTextureView;

    nvnCommandBufferSetScissor(pNvnCommandBuffer, 0, 0, 0x7FFFFFFF, 0x7FFFFFFF);
    nvnCommandBufferSetRenderTargets(pNvnCommandBuffer, 0, nullptr, nullptr, pNvnTexture,
                                     pNvnTextureView);

    switch (clearMode) {
    case DepthStencilClearMode_Depth:
        nvnCommandBufferClearDepthStencil(pNvnCommandBuffer, depth, true, stencil, 0);
        break;

    case DepthStencilClearMode_Stencil:
        nvnCommandBufferClearDepthStencil(pNvnCommandBuffer, depth, false, stencil, -1);
        break;

    case DepthStencilClearMode_DepthStencil:
        nvnCommandBufferClearDepthStencil(pNvnCommandBuffer, depth, true, stencil, -1);
        break;

    default:
        NN_UNEXPECTED_DEFAULT;
        break;
    }
}

/**
 * Resolves a multisampled color target into a texture.
 *
 * @param pDstTexture Destination texture.
 * @param dstMipLevel Destination mip level (unused).
 * @param dstStartArrayIndex Destination array layer (unused).
 * @param pSrcColorTarget Multisampled source color target.
 * @param pSrcArrayRange Source array layers (unused).
 */
void CommandBufferImplNvn8::Resolve(TextureImpl<ApiVariationNvn8>* pDstTexture, int dstMipLevel,
                                    int dstStartArrayIndex,
                                    const ColorTargetViewImpl<ApiVariationNvn8>* pSrcColorTarget,
                                    const TextureArrayRange* pSrcArrayRange) {
    const NVNtexture* pSrcNvnTexture = pSrcColorTarget->ToData()->pNvnTexture;

    nvnCommandBufferDownsample(pNvnCommandBuffer, pSrcNvnTexture,
                               pDstTexture->ToData()->pNvnTexture);
}

/**
 * Makes GPU writes of the given kinds visible to later commands.
 *
 * @param gpuAccessFlags GpuAccess bits that were written.
 */
void CommandBufferImplNvn8::FlushMemory(int gpuAccessFlags) {
    int barrier = 0;
    barrier |= (gpuAccessFlags & (GpuAccess_Image | GpuAccess_QueryBuffer | GpuAccess_DepthStencil |
                                  GpuAccess_ColorBuffer | GpuAccess_UnorderedAccessBuffer)) ?
                   NVN_BARRIER_ORDER_PRIMITIVES_BIT :
                   0;

    if (barrier != 0) {
        nvnCommandBufferBarrier(pNvnCommandBuffer, barrier);
    }
}

/**
 * Invalidates GPU caches for the given kinds of reads.
 *
 * @param gpuAccessFlags GpuAccess bits that will be read.
 */
void CommandBufferImplNvn8::InvalidateMemory(int gpuAccessFlags) {
    int barrier = 0;

    barrier |=
        (gpuAccessFlags & GpuAccess_IndirectBuffer) ? NVN_BARRIER_ORDER_INDIRECT_DATA_BIT : 0;

    barrier |= (gpuAccessFlags & (GpuAccess_Image | GpuAccess_Texture)) ?
                   NVN_BARRIER_INVALIDATE_TEXTURE_BIT :
                   0;

    barrier |= (gpuAccessFlags & (GpuAccess_ShaderCode | GpuAccess_UnorderedAccessBuffer |
                                  GpuAccess_ConstantBuffer)) ?
                   NVN_BARRIER_INVALIDATE_SHADER_BIT :
                   0;

    barrier |= (gpuAccessFlags & GpuAccess_Descriptor) ?
                   NVN_BARRIER_INVALIDATE_TEXTURE_DESCRIPTOR_BIT :
                   0;

    if (barrier != 0) {
        nvnCommandBufferBarrier(pNvnCommandBuffer, barrier);
    }
}

/**
 * Calls the commands recorded in another command buffer.
 *
 * @param pNestedCommandBuffer Command buffer to call.
 */
void CommandBufferImplNvn8::CallCommandBuffer(const CommandBufferImplNvn8* pNestedCommandBuffer) {
    NVNcommandHandle nvnCommandHandle = pNestedCommandBuffer->ToData()->hNvnCommandBuffer;

    nvnCommandBufferCallCommands(pNvnCommandBuffer, 1, &nvnCommandHandle);
}

/**
 * Copies the commands recorded in another command buffer into this one.
 *
 * @param pNestedCommandBuffer Command buffer to copy.
 */
void CommandBufferImplNvn8::CopyCommandBuffer(const CommandBufferImplNvn8* pNestedCommandBuffer) {
    NVNcommandHandle nvnCommandHandle = pNestedCommandBuffer->ToData()->hNvnCommandBuffer;

    nvnCommandBufferCopyCommands(pNvnCommandBuffer, 1, &nvnCommandHandle);
}

/**
 * Inserts the barrier needed for a buffer state transition.
 *
 * @param pBuffer Buffer changing state (unused).
 * @param oldState Previous BufferState bits.
 * @param oldStageBits Previous pipeline stage bits (unused).
 * @param newState New BufferState bits.
 * @param newStageBits New pipeline stage bits.
 */
void CommandBufferImplNvn8::SetBufferStateTransition(BufferImpl<ApiVariationNvn8>* pBuffer,
                                                     int oldState, int oldStageBits,
                                                     int newState, int newStageBits) {
    int barrier = 0;

    if ((oldState & (BufferState_QueryBuffer | BufferState_UnorderedAccessBuffer)) ||
        (newState & (BufferState_QueryBuffer | BufferState_UnorderedAccessBuffer))) {
        barrier |=
            (newStageBits & (PipelineStageBit_ComputeShader | PipelineStageBit_GeometryShader |
                             PipelineStageBit_DomainShader | PipelineStageBit_HullShader |
                             PipelineStageBit_VertexShader | PipelineStageBit_VertexInput)) ?
                NVN_BARRIER_ORDER_PRIMITIVES_BIT :
                0;

        barrier |= (newStageBits & (PipelineStageBit_RenderTarget | PipelineStageBit_PixelShader)) ?
                       NVN_BARRIER_ORDER_FRAGMENTS_BIT :
                       0;

        barrier |=
            (newState & BufferState_IndirectArgument) ? NVN_BARRIER_ORDER_INDIRECT_DATA_BIT : 0;
    }

    barrier |= (newState & (BufferState_UnorderedAccessBuffer | BufferState_ConstantBuffer)) ?
                   NVN_BARRIER_INVALIDATE_SHADER_BIT :
                   0;

    nvnCommandBufferBarrier(pNvnCommandBuffer, barrier);
}

/**
 * Inserts the barrier needed for a texture state transition.
 *
 * @param pTexture Texture changing state (unused).
 * @param pRange Subresources changing state (unused).
 * @param oldState Previous TextureState bits.
 * @param oldStageBits Previous pipeline stage bits (unused).
 * @param newState New TextureState bits.
 * @param newStageBits New pipeline stage bits.
 */
void CommandBufferImplNvn8::SetTextureStateTransition(TextureImpl<ApiVariationNvn8>* pTexture,
                                                      const TextureSubresourceRange* pRange,
                                                      int oldState, int oldStageBits,
                                                      int newState, int newStageBits) {
    int barrier = 0;

    const int writeStates = TextureState_Clear | TextureState_DepthWrite |
                            TextureState_ColorTarget | TextureState_ShaderWrite;

    if ((oldState & writeStates) != 0 || (newState & writeStates) != 0) {
        barrier |=
            (newStageBits & (PipelineStageBit_ComputeShader | PipelineStageBit_GeometryShader |
                             PipelineStageBit_DomainShader | PipelineStageBit_HullShader |
                             PipelineStageBit_VertexShader | PipelineStageBit_VertexInput)) ?
                NVN_BARRIER_ORDER_PRIMITIVES_BIT :
                0;

        barrier |= (newStageBits & (PipelineStageBit_RenderTarget | PipelineStageBit_PixelShader)) ?
                       NVN_BARRIER_ORDER_FRAGMENTS_BIT :
                       0;

        barrier |=
            (newState & (TextureState_Clear | TextureState_DepthWrite | TextureState_DepthRead)) ?
                NVN_BARRIER_ORDER_PRIMITIVES_BIT :
                0;
    }

    barrier |= (newState & TextureState_ShaderRead) ? NVN_BARRIER_INVALIDATE_TEXTURE_BIT : 0;

    nvnCommandBufferBarrier(pNvnCommandBuffer, barrier);
}

/**
 * Binds a texture or sampler descriptor pool.
 *
 * @param pDescriptorPool Descriptor pool to bind.
 */
void CommandBufferImplNvn8::SetDescriptorPool(
    const DescriptorPoolImpl<ApiVariationNvn8>* pDescriptorPool) {
    switch (pDescriptorPool->ToData()->descriptorPoolType) {
    case DescriptorPoolType_TextureView:
        nvnCommandBufferSetTexturePool(pNvnCommandBuffer,
                                       pDescriptorPool->ToData()->pDescriptorPool);
        break;

    case DescriptorPoolType_Sampler:
        nvnCommandBufferSetSamplerPool(pNvnCommandBuffer,
                                       pDescriptorPool->ToData()->pDescriptorPool);
        break;

    default:
        break;
    }
}

/**
 * Starts a query by resetting its counter.
 *
 * @param target Query to start.
 */
void CommandBufferImplNvn8::BeginQuery(QueryTarget target) {
    if (target != QueryTarget_ComputeShaderInvocations) {
        NVNcounterType counterType = Nvn::GetCounterType(target);
        nvnCommandBufferResetCounter(pNvnCommandBuffer, counterType);
    }
}

/**
 * Ends a query by writing its counter to memory.
 *
 * @param rDstBufferAddress GPU address that receives the result.
 * @param target Query to end.
 */
void CommandBufferImplNvn8::EndQuery(const GpuAddress& rDstBufferAddress, QueryTarget target) {
    if (target != QueryTarget_ComputeShaderInvocations) {
        NVNcounterType counterType = Nvn::GetCounterType(target);
        nvnCommandBufferReportCounter(pNvnCommandBuffer, counterType,
                                      Nvn::GetBufferAddress(rDstBufferAddress));
    }
}

/**
 * Writes a GPU timestamp to memory.
 *
 * @param rDstBufferAddress GPU address that receives the timestamp.
 */
void CommandBufferImplNvn8::WriteTimestamp(const GpuAddress& rDstBufferAddress) {
    nvnCommandBufferReportCounter(pNvnCommandBuffer, NVN_COUNTER_TYPE_TIMESTAMP,
                                  Nvn::GetBufferAddress(rDstBufferAddress));
}

/**
 * Enables the depth bounds test with the given range.
 *
 * @param minDepthBounds Minimum depth.
 * @param maxDepthBounds Maximum depth.
 */
void CommandBufferImplNvn8::SetDepthBounds(float minDepthBounds, float maxDepthBounds) {
    nvnCommandBufferSetDepthBounds(pNvnCommandBuffer, true, minDepthBounds, maxDepthBounds);
}

/**
 * Sets the rasterized line width.
 *
 * @param lineWidth Line width in pixels.
 */
void CommandBufferImplNvn8::SetLineWidth(float lineWidth) {
    nvnCommandBufferSetLineWidth(pNvnCommandBuffer, lineWidth);
}

/**
 * Sets a range of viewports and their depth ranges.
 *
 * @param firstViewport Index of the first viewport.
 * @param viewportCount Number of viewports.
 * @param pViewports Viewport descriptions.
 */
void CommandBufferImplNvn8::SetViewports(int firstViewport, int viewportCount,
                                         const ViewportStateInfo* pViewports) {
    const int MaxViewports = 16;

    float viewports[MaxViewports * 4];
    float depthRanges[MaxViewports * 2];
    float* pViewport = viewports;
    float* pDepthRange = depthRanges;

    for (int idxViewport = 0; idxViewport < viewportCount; ++idxViewport) {
        const ViewportStateInfo& rViewport = pViewports[idxViewport];

        *pViewport++ = rViewport.GetOriginX();
        *pViewport++ = rViewport.GetOriginY();
        *pViewport++ = rViewport.GetWidth();
        *pViewport++ = rViewport.GetHeight();
        *pDepthRange++ = rViewport.GetMinDepth();
        *pDepthRange++ = rViewport.GetMaxDepth();
    }

    nvnCommandBufferSetViewports(pNvnCommandBuffer, firstViewport, viewportCount, viewports);
    nvnCommandBufferSetDepthRanges(pNvnCommandBuffer, firstViewport, viewportCount, depthRanges);
}

/**
 * Sets a range of scissor rectangles.
 *
 * @param firstScissor Index of the first scissor.
 * @param scissorCount Number of scissors.
 * @param pScissors Scissor descriptions.
 */
void CommandBufferImplNvn8::SetScissors(int firstScissor, int scissorCount,
                                        const ScissorStateInfo* pScissors) {
    const int MaxScissors = 16;

    int scissors[MaxScissors * 4];
    int* pScissor = scissors;

    for (int idxScissor = 0; idxScissor < scissorCount; ++idxScissor) {
        const ScissorStateInfo& rScissor = pScissors[idxScissor];

        *pScissor++ = rScissor.GetOriginX();
        *pScissor++ = rScissor.GetOriginY();
        *pScissor++ = rScissor.GetWidth();
        *pScissor++ = rScissor.GetHeight();
    }

    nvnCommandBufferSetScissors(pNvnCommandBuffer, firstScissor, scissorCount, scissors);
}

/**
 * Updates part of a uniform buffer inline in the command stream.
 *
 * @param rDstBufferAddress GPU address of the uniform buffer.
 * @param bufferSize Size of the uniform buffer in bytes.
 * @param dstOffset Offset of the update in the buffer.
 * @param dataSize Size of the update in bytes.
 * @param pData Data to write.
 */
void CommandBufferImplNvn8::UpdateBuffer(const GpuAddress& rDstBufferAddress, size_t bufferSize,
                                         ptrdiff_t dstOffset, size_t dataSize,
                                         const void* pData) {
    nvnCommandBufferUpdateUniformBuffer(pNvnCommandBuffer,
                                        Nvn::GetBufferAddress(rDstBufferAddress), bufferSize,
                                        dstOffset, dataSize, pData);
}

/**
 * Binds a constant buffer from a buffer descriptor slot.
 *
 * @param slot Binding slot.
 * @param stage Shader stage.
 * @param rConstantBufferDescriptor Descriptor slot holding the buffer address and size.
 */
void CommandBufferImplNvn8::SetConstantBuffer(int slot, ShaderStage stage,
                                              const DescriptorSlot& rConstantBufferDescriptor) {
    nn::util::ConstBytePtr pDescriptor(ToPtr<void*>(rConstantBufferDescriptor));
    NVNbufferAddress address = *pDescriptor.Get<NVNbufferAddress>();
    size_t size = *pDescriptor.Advance(8).Get<size_t>();

    nvnCommandBufferBindUniformBuffer(pNvnCommandBuffer, Nvn::GetShaderStage(stage), slot, address,
                                      size);
}

/**
 * Binds a storage buffer from a buffer descriptor slot.
 *
 * @param slot Binding slot.
 * @param stage Shader stage.
 * @param rUnorderedAccessBufferDescriptor Descriptor slot holding the buffer address and size.
 */
void CommandBufferImplNvn8::SetUnorderedAccessBuffer(
    int slot, ShaderStage stage, const DescriptorSlot& rUnorderedAccessBufferDescriptor) {
    nn::util::ConstBytePtr pDescriptor(ToPtr<void*>(rUnorderedAccessBufferDescriptor));
    NVNbufferAddress address = *pDescriptor.Get<NVNbufferAddress>();
    size_t size = *pDescriptor.Advance(8).Get<size_t>();

    nvnCommandBufferBindStorageBuffer(pNvnCommandBuffer, Nvn::GetShaderStage(stage), slot, address,
                                      size);
}

/**
 * Binds a texture and sampler from descriptor slots.
 *
 * @param slot Binding slot.
 * @param stage Shader stage.
 * @param rTextureDescriptor Texture descriptor slot.
 * @param rSamplerDescriptor Sampler descriptor slot.
 */
void CommandBufferImplNvn8::SetTextureAndSampler(int slot, ShaderStage stage,
                                                 const DescriptorSlot& rTextureDescriptor,
                                                 const DescriptorSlot& rSamplerDescriptor) {
    detail::SetTextureAndSampler(this, stage, slot, rTextureDescriptor.ToData()->value,
                                 rSamplerDescriptor.ToData()->value);
}

/**
 * Binds a texture for texel fetches from a descriptor slot.
 *
 * @param slot Binding slot.
 * @param stage Shader stage.
 * @param rTextureDescriptor Texture descriptor slot.
 */
void CommandBufferImplNvn8::SetTexture(int slot, ShaderStage stage,
                                       const DescriptorSlot& rTextureDescriptor) {
    NVNtextureHandle textureHandle = nvnDeviceGetTexelFetchHandle(
        pNnDevice->ToData()->pNvnDevice, rTextureDescriptor.ToData()->value);

    nvnCommandBufferBindTexture(pNvnCommandBuffer, Nvn::GetShaderStage(stage), slot, textureHandle);
}

/**
 * Binds an image from a descriptor slot.
 *
 * @param slot Binding slot.
 * @param stage Shader stage.
 * @param rImageDescriptor Image descriptor slot.
 */
void CommandBufferImplNvn8::SetImage(int slot, ShaderStage stage,
                                     const DescriptorSlot& rImageDescriptor) {
    NVNtextureHandle imageHandle =
        nvnDeviceGetImageHandle(pNnDevice->ToData()->pNvnDevice, rImageDescriptor.ToData()->value);

    nvnCommandBufferBindImage(pNvnCommandBuffer, Nvn::GetShaderStage(stage), slot, imageHandle);
}

/**
 * Binds a constant buffer by GPU address.
 *
 * @param slot Binding slot.
 * @param stage Shader stage.
 * @param rConstantBufferAddress GPU address of the buffer.
 * @param size Size of the buffer in bytes.
 */
void CommandBufferImplNvn8::SetConstantBuffer(int slot, ShaderStage stage,
                                              const GpuAddress& rConstantBufferAddress,
                                              size_t size) {
    nvnCommandBufferBindUniformBuffer(pNvnCommandBuffer, Nvn::GetShaderStage(stage), slot,
                                      Nvn::GetBufferAddress(rConstantBufferAddress), size);
}

/**
 * Binds a storage buffer by GPU address.
 *
 * @param slot Binding slot.
 * @param stage Shader stage.
 * @param rUnorderedAccessBufferAddress GPU address of the buffer.
 * @param size Size of the buffer in bytes.
 */
void CommandBufferImplNvn8::SetUnorderedAccessBuffer(
    int slot, ShaderStage stage, const GpuAddress& rUnorderedAccessBufferAddress, size_t size) {
    nvnCommandBufferBindStorageBuffer(pNvnCommandBuffer, Nvn::GetShaderStage(stage), slot,
                                      Nvn::GetBufferAddress(rUnorderedAccessBufferAddress), size);
}

/**
 * Binds a texture view and sampler object directly (unsupported on NVN; no-op).
 *
 * @param slot Binding slot (unused).
 * @param stage Shader stage (unused).
 * @param pTextureView Texture view (unused).
 * @param pSampler Sampler (unused).
 */
void CommandBufferImplNvn8::SetTextureAndSampler(
    int slot, ShaderStage stage, const TextureViewImpl<ApiVariationNvn8>* pTextureView,
    const SamplerImpl<ApiVariationNvn8>* pSampler) {}

/**
 * Binds a texture view as an image directly (unsupported on NVN; no-op).
 *
 * @param slot Binding slot (unused).
 * @param stage Shader stage (unused).
 * @param pTextureView Texture view (unused).
 */
void CommandBufferImplNvn8::SetImage(int slot, ShaderStage stage,
                                     const TextureViewImpl<ApiVariationNvn8>* pTextureView) {}

/**
 * Opens a named debug group in the command stream.
 *
 * @param pDescription Name of the group.
 */
void CommandBufferImplNvn8::PushDebugGroup(const char* pDescription) {
    nvnCommandBufferPushDebugGroupDynamic(pNvnCommandBuffer, DebugGroupDomainId, pDescription);
}

/**
 * Closes the innermost debug group.
 */
void CommandBufferImplNvn8::PopDebugGroup() {
    nvnCommandBufferPopDebugGroupId(pNvnCommandBuffer, DebugGroupDomainId);
}

}  // namespace nn::gfx::detail
