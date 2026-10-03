#include <nn/gfx/gfx_Interoperation-api.nvn.8.h>

#include <nn/gfx/gfx_Buffer.h>
#include <nn/gfx/gfx_CommandBuffer.h>
#include <nn/gfx/gfx_DescriptorPool.h>
#include <nn/gfx/gfx_Device.h>
#include <nn/gfx/gfx_MemoryPool.h>
#include <nn/gfx/gfx_Queue.h>
#include <nn/gfx/gfx_Sampler.h>
#include <nn/gfx/gfx_Shader.h>
#include <nn/gfx/gfx_Texture.h>
#include <nn/gfx/detail/gfx_CommonHelper.h>
#include <nn/gfx/detail/gfx_NvnHelper.h>
#include <nvn/nvn_FuncPtrInline.h>

namespace nn::gfx {

/**
 * Wraps an externally created NVN device in a gfx device.
 *
 * @param pGfxDevice Device object to initialize as shared.
 * @param pNvnDevice NVN device to wrap.
 */
void TInteroperation<ApiVariationNvn8>::ConvertToGfxDevice(TDevice<ApiVariationNvn8>* pGfxDevice,
                                                           NVNdevice* pNvnDevice) {
    TDevice<ApiVariationNvn8>::DataType& obj = pGfxDevice->ToData();
    detail::UseMiddleWare();
    obj.state = obj.State_Initialized;
    obj.flags.SetBit(obj.Flag_Shared, true);
    obj.pNvnDevice = pNvnDevice;
    obj.supportedFeatures = detail::Nvn::GetDeviceFeature(pNvnDevice);
}

/**
 * Wraps an externally created NVN queue in a gfx queue.
 *
 * @param pGfxQueue Queue object to initialize as shared.
 * @param pNvnQueue NVN queue to wrap.
 * @param pDevice Device the queue belongs to.
 */
void TInteroperation<ApiVariationNvn8>::ConvertToGfxQueue(TQueue<ApiVariationNvn8>* pGfxQueue,
                                                          NVNqueue* pNvnQueue,
                                                          TDevice<ApiVariationNvn8>* pDevice) {
    TQueue<ApiVariationNvn8>::DataType& obj = pGfxQueue->ToData();
    obj.state = obj.State_Initialized;
    obj.flags.SetBit(obj.Flag_Shared, true);
    obj.pNvnQueue = pNvnQueue;
    obj.pNnDevice = pDevice;
    obj.pImpl = nullptr;
}

/**
 * Wraps an externally created NVN memory pool in a gfx memory pool.
 *
 * @param pGfxMemoryPool Memory pool object to initialize as shared.
 * @param pNvnMemoryPool NVN memory pool to wrap.
 * @param pMemory CPU address of the pool's storage.
 */
void TInteroperation<ApiVariationNvn8>::ConvertToGfxMemoryPool(
    TMemoryPool<ApiVariationNvn8>* pGfxMemoryPool, NVNmemoryPool* pNvnMemoryPool, void* pMemory) {
    TMemoryPool<ApiVariationNvn8>::DataType& obj = pGfxMemoryPool->ToData();
    obj.state = obj.State_Initialized;
    obj.flags.SetBit(obj.Flag_Shared, true);
    obj.pNvnMemoryPool = pNvnMemoryPool;
    obj.pMemory = pMemory;
}

/**
 * Wraps an externally created NVN texture pool in a texture-view descriptor pool.
 *
 * @param pGfxDescriptorPool Descriptor pool object to initialize as shared.
 * @param pNvnTexturePool NVN texture pool to wrap.
 */
void TInteroperation<ApiVariationNvn8>::ConvertToGfxDescriptorPool(
    TDescriptorPool<ApiVariationNvn8>* pGfxDescriptorPool, NVNtexturePool* pNvnTexturePool) {
    TDescriptorPool<ApiVariationNvn8>::DataType& obj = pGfxDescriptorPool->ToData();
    obj.descriptorPoolType = DescriptorPoolType_TextureView;
    obj.slotCount = nvnTexturePoolGetSize(pNvnTexturePool);
    obj.pDescriptorPool = pNvnTexturePool;
    obj.reservedSlots = 256;
    obj.flags.SetBit(obj.Flag_Shared, true);
    obj.state = obj.State_Initialized;
}

/**
 * Wraps an externally created NVN sampler pool in a sampler descriptor pool.
 *
 * @param pGfxDescriptorPool Descriptor pool object to initialize as shared.
 * @param pNvnSamplerPool NVN sampler pool to wrap.
 */
void TInteroperation<ApiVariationNvn8>::ConvertToGfxDescriptorPool(
    TDescriptorPool<ApiVariationNvn8>* pGfxDescriptorPool, NVNsamplerPool* pNvnSamplerPool) {
    TDescriptorPool<ApiVariationNvn8>::DataType& obj = pGfxDescriptorPool->ToData();
    obj.descriptorPoolType = DescriptorPoolType_Sampler;
    obj.slotCount = nvnSamplerPoolGetSize(pNvnSamplerPool);
    obj.pDescriptorPool = pNvnSamplerPool;
    obj.reservedSlots = 256;
    obj.flags.SetBit(obj.Flag_Shared, true);
    obj.state = obj.State_Initialized;
}

/**
 * Wraps an externally created NVN buffer in a gfx buffer.
 *
 * @param pGfxBuffer Buffer object to initialize as shared.
 * @param pNvnBuffer NVN buffer to wrap.
 */
void TInteroperation<ApiVariationNvn8>::ConvertToGfxBuffer(TBuffer<ApiVariationNvn8>* pGfxBuffer,
                                                           NVNbuffer* pNvnBuffer) {
    TBuffer<ApiVariationNvn8>::DataType& obj = pGfxBuffer->ToData();
    NVNmemoryPoolFlags memoryPoolFlags = nvnMemoryPoolGetFlags(nvnBufferGetMemoryPool(pNvnBuffer));
    obj.state = obj.State_Initialized;
    obj.flags.SetBit(obj.Flag_CpuCached, memoryPoolFlags & NVN_MEMORY_POOL_FLAGS_CPU_CACHED);
    obj.flags.SetBit(obj.Flag_Shared, true);
    obj.pNvnBuffer = pNvnBuffer;
}

/**
 * Wraps an externally created NVN command buffer in a gfx command buffer.
 *
 * @param pGfxCommandBuffer Command buffer object to initialize as shared.
 * @param pDevice Device the command buffer belongs to.
 * @param pNvnCommandBuffer NVN command buffer to wrap.
 */
void TInteroperation<ApiVariationNvn8>::ConvertToGfxCommandBuffer(
    TCommandBuffer<ApiVariationNvn8>* pGfxCommandBuffer, TDevice<ApiVariationNvn8>* pDevice,
    NVNcommandBuffer* pNvnCommandBuffer) {
    TCommandBuffer<ApiVariationNvn8>::DataType& obj = pGfxCommandBuffer->ToData();
    obj.state = obj.State_Initialized;
    obj.flags.SetBit(obj.Flag_Shared, true);
    obj.pNnDevice = pDevice;
    obj.pNvnCommandBuffer = pNvnCommandBuffer;
}

/**
 * Wraps an externally created NVN sampler in a gfx sampler.
 *
 * @param pGfxSampler Sampler object to initialize as shared.
 * @param pNvnSampler NVN sampler to wrap.
 */
void TInteroperation<ApiVariationNvn8>::ConvertToGfxSampler(TSampler<ApiVariationNvn8>* pGfxSampler,
                                                            NVNsampler* pNvnSampler) {
    TSampler<ApiVariationNvn8>::DataType& obj = pGfxSampler->ToData();
    obj.state = obj.State_Initialized;
    obj.flags.SetBit(obj.Flag_Shared, true);
    obj.pNvnSampler = pNvnSampler;
}

/**
 * Wraps an externally created NVN program in a gfx shader.
 *
 * @param pGfxShader Shader object to initialize as shared.
 * @param pNvnProgram NVN program to wrap.
 * @param isSeparationEnabled Whether the program uses separable stages.
 * @param shaderBits ShaderStageBit mask of the stages the program contains.
 */
void TInteroperation<ApiVariationNvn8>::ConvertToGfxShader(TShader<ApiVariationNvn8>* pGfxShader,
                                                           NVNprogram* pNvnProgram,
                                                           bool isSeparationEnabled,
                                                           int shaderBits) {
    TShader<ApiVariationNvn8>::DataType& obj = pGfxShader->ToData();
    obj.state = obj.State_Initialized;
    obj.flags.SetBit(obj.Flag_Shared, true);
    obj.flags.SetBit(obj.Flag_SeparationEnable, isSeparationEnabled);
    obj.nvnShaderStageBits = (shaderBits & ShaderStageBit_Compute) != 0 ?
                                 NVN_SHADER_STAGE_COMPUTE_BIT :
                                 NVN_SHADER_STAGE_ALL_GRAPHICS_BITS;
    obj.pNvnProgram = pNvnProgram;
    obj.pReflection = nullptr;
}

/**
 * Wraps an externally created NVN texture in a gfx texture.
 *
 * @param pGfxTexture Texture object to initialize as shared.
 * @param pNvnTexture NVN texture to wrap.
 */
void TInteroperation<ApiVariationNvn8>::ConvertToGfxTexture(TTexture<ApiVariationNvn8>* pGfxTexture,
                                                            NVNtexture* pNvnTexture) {
    TTexture<ApiVariationNvn8>::DataType& obj = pGfxTexture->ToData();
    obj.state = obj.State_Initialized;
    obj.flags.SetBit(obj.Flag_Shared, true);
    obj.pNvnTexture = pNvnTexture;
}

/**
 * Wraps an externally created NVN texture and view in a gfx texture view.
 *
 * @param pGfxTextureView Texture view object to initialize as shared.
 * @param pNvnTexture NVN texture the view refers to.
 * @param pNvnTextureView NVN texture view to wrap.
 */
void TInteroperation<ApiVariationNvn8>::ConvertToGfxTextureView(
    TTextureView<ApiVariationNvn8>* pGfxTextureView, NVNtexture* pNvnTexture,
    NVNtextureView* pNvnTextureView) {
    TTextureView<ApiVariationNvn8>::DataType& obj = pGfxTextureView->ToData();
    obj.state = obj.State_Initialized;
    obj.flags.SetBit(obj.Flag_Shared, true);
    obj.pNvnTexture = pNvnTexture;
    obj.pNvnTextureView = pNvnTextureView;
}

/**
 * Wraps an externally created NVN texture and view in a gfx color target view.
 *
 * @param pGfxColorTargetView Color target view object to initialize as shared.
 * @param pNvnTexture NVN texture the view refers to.
 * @param pNvnTextureView NVN texture view to wrap.
 */
void TInteroperation<ApiVariationNvn8>::ConvertToGfxColorTargetView(
    TColorTargetView<ApiVariationNvn8>* pGfxColorTargetView, NVNtexture* pNvnTexture,
    NVNtextureView* pNvnTextureView) {
    TColorTargetView<ApiVariationNvn8>::DataType& obj = pGfxColorTargetView->ToData();
    obj.state = obj.State_Initialized;
    obj.flags.SetBit(obj.Flag_Shared, true);
    obj.pNvnTexture = pNvnTexture;
    obj.pNvnTextureView = pNvnTextureView;
}

/**
 * Wraps an externally created NVN texture and view in a gfx depth stencil view.
 *
 * @param pGfxDepthStencilView Depth stencil view object to initialize as shared.
 * @param pNvnTexture NVN texture the view refers to.
 * @param pNvnTextureView NVN texture view to wrap.
 */
void TInteroperation<ApiVariationNvn8>::ConvertToGfxDepthStencilView(
    TDepthStencilView<ApiVariationNvn8>* pGfxDepthStencilView, NVNtexture* pNvnTexture,
    NVNtextureView* pNvnTextureView) {
    TDepthStencilView<ApiVariationNvn8>::DataType& obj = pGfxDepthStencilView->ToData();
    obj.state = obj.State_Initialized;
    obj.flags.SetBit(obj.Flag_Shared, true);
    obj.pNvnTexture = pNvnTexture;
    obj.pNvnTextureView = pNvnTextureView;
}

/**
 * Converts a gfx image format to the matching NVN format.
 *
 * @param format Image format to convert.
 * @return The NVN format.
 */
NVNformat TInteroperation<ApiVariationNvn8>::ConvertToNvnFormat(ImageFormat format) {
    return detail::Nvn::GetImageFormat(format);
}

/**
 * Converts a gfx vertex attribute format to the matching NVN format.
 *
 * @param format Attribute format to convert.
 * @return The NVN format.
 */
NVNformat TInteroperation<ApiVariationNvn8>::ConvertToNvnFormat(AttributeFormat format) {
    return detail::Nvn::GetAttributeFormat(format);
}

}  // namespace nn::gfx
