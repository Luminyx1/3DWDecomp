#include <nn/gfx/detail/gfx_Buffer-api.nvn.8.h>

#include <nn/gfx/detail/gfx_BufferInfo-api.nvn.8.h>
#include <nn/gfx/detail/gfx_Device-api.nvn.8.h>
#include <nn/gfx/detail/gfx_MemoryPool-api.nvn.8.h>
#include <nn/gfx/detail/gfx_NvnHelper.h>
#include <nn/gfx/gfx_BufferInfo.h>
#include <nn/gfx/gfx_GpuAddress.h>
#include <nvn/nvn_FuncPtrInline.h>

#include <algorithm>

namespace nn::gfx::detail {

typedef BufferImpl<ApiVariationNvn8> BufferImplNvn8;
typedef BufferTextureViewImpl<ApiVariationNvn8> BufferTextureViewImplNvn8;

namespace {

/**
 * Computes the alignment a buffer needs for the given GPU access flags.
 *
 * @param pDevice Device the buffer will be created on.
 * @param gpuAccessFlags Combination of GpuAccess flags.
 * @return Required alignment in bytes.
 */
size_t GetBufferAlignmentImpl(DeviceImpl<ApiVariationNvn8>* pDevice, int gpuAccessFlags) {
    int alignment = 8;

    if (gpuAccessFlags & GpuAccess_ConstantBuffer) {
        int uniformBufferAlignment;
        nvnDeviceGetInteger(pDevice->ToData()->pNvnDevice,
                            NVN_DEVICE_INFO_UNIFORM_BUFFER_ALIGNMENT, &uniformBufferAlignment);
        alignment = std::max(alignment, uniformBufferAlignment);
    }

    if (gpuAccessFlags & GpuAccess_IndirectBuffer) {
        int indirectBufferAlignment;
        nvnDeviceGetInteger(pDevice->ToData()->pNvnDevice,
                            NVN_DEVICE_INFO_INDIRECT_DRAW_ALIGNMENT, &indirectBufferAlignment);
        alignment = std::max(alignment, indirectBufferAlignment);

        nvnDeviceGetInteger(pDevice->ToData()->pNvnDevice,
                            NVN_DEVICE_INFO_INDIRECT_DISPATCH_ALIGNMENT, &indirectBufferAlignment);
        alignment = std::max(alignment, indirectBufferAlignment);
    }

    if (gpuAccessFlags & GpuAccess_QueryBuffer) {
        int counterAlignment;
        nvnDeviceGetInteger(pDevice->ToData()->pNvnDevice, NVN_DEVICE_INFO_COUNTER_ALIGNMENT,
                            &counterAlignment);
        alignment = std::max(alignment, counterAlignment);
    }

    if (gpuAccessFlags & GpuAccess_UnorderedAccessBuffer) {
        alignment = std::max(alignment, 32);
    }

    if (gpuAccessFlags & (GpuAccess_DepthStencil | GpuAccess_ColorBuffer | GpuAccess_Texture)) {
        alignment = std::max(alignment, 512);
    }

    return alignment;
}

/**
 * Configures a texture builder describing a buffer texture view.
 *
 * @param pBuilder Builder to configure.
 * @param pDevice NVN device.
 * @param rInfo Buffer texture view description.
 */
void SetupTextureBuilder(NVNtextureBuilder* pBuilder, NVNdevice* pDevice,
                         const BufferTextureViewInfo& rInfo) {
    const BufferImplNvn8* pBuffer = rInfo.GetBufferPtr();
    NVNbuffer* pNvnBuffer = pBuffer->ToData()->pNvnBuffer;
    NVNformat nvnFormat = Nvn::GetImageFormat(rInfo.GetImageFormat());

    NVNmemoryPool* pNvnMemoryPool = nvnBufferGetMemoryPool(pNvnBuffer);
    ptrdiff_t bufferOffset = nvnBufferGetMemoryOffset(pNvnBuffer);

    nvnTextureBuilderSetDevice(pBuilder, pDevice);
    nvnTextureBuilderSetDefaults(pBuilder);
    nvnTextureBuilderSetWidth(pBuilder, rInfo.GetSize());
    nvnTextureBuilderSetHeight(pBuilder, 1);
    nvnTextureBuilderSetDepth(pBuilder, 1);
    nvnTextureBuilderSetFormat(pBuilder, nvnFormat);
    nvnTextureBuilderSetTarget(pBuilder, NVN_TEXTURE_TARGET_BUFFER);
    nvnTextureBuilderSetFlags(pBuilder, NVN_TEXTURE_FLAGS_IMAGE);
    nvnTextureBuilderSetStorage(pBuilder, pNvnMemoryPool, rInfo.GetOffset() + bufferOffset);
}

}  // namespace

/**
 * Returns the alignment required for the memory pool offset of a buffer.
 *
 * @param pDevice Device the buffer will be created on.
 * @param rInfo Buffer description.
 * @return Required alignment in bytes.
 */
size_t BufferImplNvn8::GetBufferAlignment(DeviceImpl<ApiVariationNvn8>* pDevice,
                                          const InfoType& rInfo) {
    return GetBufferAlignmentImpl(pDevice, rInfo.GetGpuAccessFlags());
}

/**
 * Returns the alignment required for the memory pool offset of a buffer.
 *
 * @param pDevice Device the buffer will be created on.
 * @param rInfo NVN-native buffer description.
 * @return Required alignment in bytes.
 */
size_t BufferImplNvn8::GetBufferAlignment(DeviceImpl<ApiVariationNvn8>* pDevice,
                                          const BufferInfoImpl<ApiVariationNvn8>& rInfo) {
    return GetBufferAlignmentImpl(pDevice, rInfo.ToData()->gpuAccessFlags);
}

/**
 * Constructs an uninitialized buffer, clearing all of its data.
 */
BufferImplNvn8::BufferImpl() {
    state = State_NotInitialized;
}

/**
 * Destroys the buffer object. Finalize must have been called beforehand.
 */
BufferImplNvn8::~BufferImpl() {}

/**
 * Initializes the buffer in a memory pool.
 *
 * @param pDevice Device to create the buffer on.
 * @param rInfo Buffer description.
 * @param pMemoryPool Memory pool providing the storage.
 * @param memoryPoolOffset Byte offset of the storage within the pool.
 * @param memoryPoolSize Size of the storage in bytes (unused).
 */
void BufferImplNvn8::Initialize(DeviceImpl<Target>* pDevice, const BufferInfo& rInfo,
                                MemoryPoolImpl<Target>* pMemoryPool, ptrdiff_t memoryPoolOffset,
                                size_t memoryPoolSize) {
    NVNbufferBuilder builder;
    nvnBufferBuilderSetDefaults(&builder);
    nvnBufferBuilderSetDevice(&builder, pDevice->ToData()->pNvnDevice);
    nvnBufferBuilderSetStorage(&builder, pMemoryPool->ToData()->pNvnMemoryPool, memoryPoolOffset,
                               rInfo.GetSize());

    pNvnBuffer = &nvnBuffer;
    nvnBufferInitialize(pNvnBuffer, &builder);

    int nvnMemoryPoolFlags = nvnMemoryPoolGetFlags(pMemoryPool->ToData()->pNvnMemoryPool);
    flags.SetBit(Flag_CpuCached, nvnMemoryPoolFlags & NVN_MEMORY_POOL_FLAGS_CPU_CACHED);
    flags.SetBit(Flag_Shared, false);
    state = State_Initialized;
}

/**
 * Initializes the buffer in a memory pool from an already configured NVN buffer builder.
 *
 * @param pDevice Device to create the buffer on.
 * @param rInfo NVN-native buffer description.
 * @param pMemoryPool Memory pool providing the storage.
 * @param memoryPoolOffset Byte offset of the storage within the pool.
 * @param memoryPoolSize Size of the storage in bytes (unused).
 */
void BufferImplNvn8::Initialize(DeviceImpl<Target>* pDevice, const BufferInfoImpl<Target>& rInfo,
                                MemoryPoolImpl<Target>* pMemoryPool, ptrdiff_t memoryPoolOffset,
                                size_t memoryPoolSize) {
    NVNbufferBuilder builder = rInfo.ToData()->nvnBufferBuilder;
    nvnBufferBuilderSetDevice(&builder, pDevice->ToData()->pNvnDevice);
    nvnBufferBuilderSetStorage(&builder, pMemoryPool->ToData()->pNvnMemoryPool, memoryPoolOffset,
                               rInfo.ToData()->size);

    pNvnBuffer = &nvnBuffer;
    nvnBufferInitialize(pNvnBuffer, &builder);

    int nvnMemoryPoolFlags = nvnMemoryPoolGetFlags(pMemoryPool->ToData()->pNvnMemoryPool);
    flags.SetBit(Flag_CpuCached, nvnMemoryPoolFlags & NVN_MEMORY_POOL_FLAGS_CPU_CACHED);
    flags.SetBit(Flag_Shared, false);
    state = State_Initialized;
}

/**
 * Finalizes the buffer, releasing the underlying NVN buffer.
 *
 * @param pDevice Device the buffer was created on (unused).
 */
void BufferImplNvn8::Finalize(DeviceImpl<Target>* pDevice) {
    nvnBufferFinalize(pNvnBuffer);
    pNvnBuffer = nullptr;
    state = State_NotInitialized;
}

/**
 * Maps the buffer memory for CPU access.
 *
 * @return CPU address of the buffer contents.
 */
void* BufferImplNvn8::Map() const {
    return nvnBufferMap(pNvnBuffer);
}

/**
 * Unmaps the buffer memory. NVN buffers stay mapped, so this does nothing.
 */
void BufferImplNvn8::Unmap() const {}

/**
 * Flushes CPU writes in a mapped range so that the GPU can see them. Only needed for buffers
 * in CPU-cached memory pools.
 *
 * @param offset Byte offset of the range within the buffer.
 * @param size Size of the range in bytes.
 */
void BufferImplNvn8::FlushMappedRange(ptrdiff_t offset, size_t size) const {
    if (flags.GetBit(Flag_CpuCached)) {
        nvnBufferFlushMappedRange(pNvnBuffer, offset, size);
    }
}

/**
 * Invalidates CPU caches for a mapped range so that GPU writes become visible. Only needed for
 * buffers in CPU-cached memory pools.
 *
 * @param offset Byte offset of the range within the buffer.
 * @param size Size of the range in bytes.
 */
void BufferImplNvn8::InvalidateMappedRange(ptrdiff_t offset, size_t size) const {
    if (flags.GetBit(Flag_CpuCached)) {
        nvnBufferInvalidateMappedRange(pNvnBuffer, offset, size);
    }
}

/**
 * Retrieves the GPU address of the start of the buffer.
 *
 * @param pOutGpuAddress Receives the GPU address.
 */
void BufferImplNvn8::GetGpuAddress(GpuAddress* pOutGpuAddress) const {
    pOutGpuAddress->ToData()->value = nvnBufferGetAddress(pNvnBuffer);
    pOutGpuAddress->ToData()->impl = 0;
}

/**
 * Sets a debug label on the underlying NVN buffer.
 *
 * @param pDevice Device the buffer was created on (unused).
 * @param pLabel Label string.
 */
void BufferImplNvn8::SetDebugLabel(DeviceImpl<Target>* pDevice, const char* pLabel) {
    nvnBufferSetDebugLabel(pNvnBuffer, pLabel);
}

/**
 * Returns the alignment required for the offset of a buffer texture view.
 *
 * @param pDevice Device the view will be created on.
 * @param rInfo Buffer texture view description.
 * @return Required alignment in bytes.
 */
size_t BufferTextureViewImplNvn8::GetOffsetAlignment(DeviceImpl<Target>* pDevice,
                                                     const InfoType& rInfo) {
    NVNtextureBuilder builder;
    SetupTextureBuilder(&builder, pDevice->ToData()->pNvnDevice, rInfo);

    return nvnTextureBuilderGetStorageAlignment(&builder);
}

/**
 * Constructs an uninitialized buffer texture view, clearing all of its data.
 */
BufferTextureViewImplNvn8::BufferTextureViewImpl() {
    state = State_NotInitialized;
}

/**
 * Destroys the buffer texture view object. Finalize must have been called beforehand.
 */
BufferTextureViewImplNvn8::~BufferTextureViewImpl() {}

/**
 * Initializes the buffer texture view.
 *
 * @param pDevice Device to create the view on.
 * @param rInfo Buffer texture view description.
 */
void BufferTextureViewImplNvn8::Initialize(DeviceImpl<Target>* pDevice, const InfoType& rInfo) {
    pNvnTexture = &nvnTexture;

    NVNtextureBuilder builder;
    SetupTextureBuilder(&builder, pDevice->ToData()->pNvnDevice, rInfo);
    nvnTextureInitialize(pNvnTexture, &builder);

    flags.SetBit(Flag_Shared, false);
    state = State_Initialized;
}

/**
 * Finalizes the buffer texture view, releasing the underlying NVN texture.
 *
 * @param pDevice Device the view was created on (unused).
 */
void BufferTextureViewImplNvn8::Finalize(DeviceImpl<Target>* pDevice) {
    nvnTextureFinalize(pNvnTexture);
    pNvnTexture = nullptr;
    state = State_NotInitialized;
}

}  // namespace nn::gfx::detail
