#include <nn/gfx/detail/gfx_MemoryPool-api.nvn.8.h>

#include <nn/gfx/detail/gfx_Device-api.nvn.8.h>
#include <nn/gfx/detail/gfx_MemoryPoolInfo-api.nvn.8.h>
#include <nn/gfx/detail/gfx_NvnHelper.h>
#include <nn/gfx/gfx_MemoryPoolInfo.h>
#include <nvn/nvn_FuncPtrInline.h>

namespace nn::gfx::detail {

typedef MemoryPoolImpl<ApiVariationNvn8> MemoryPoolImplNvn8;

/**
 * Returns the required alignment of memory handed to a memory pool.
 *
 * @param pDevice Device the pool will be created on (unused).
 * @param rInfo Pool description (unused).
 * @return Alignment in bytes (NVN memory pool alignment, 4 KiB).
 */
size_t MemoryPoolImplNvn8::GetPoolMemoryAlignment(DeviceImpl<ApiVariationNvn8>* pDevice,
                                                  const InfoType& rInfo) {
    return 0x1000;
}

/**
 * Returns the required alignment of memory handed to a memory pool.
 *
 * @param pDevice Device the pool will be created on (unused).
 * @param rInfo NVN-native pool description (unused).
 * @return Alignment in bytes (NVN memory pool alignment, 4 KiB).
 */
size_t
MemoryPoolImplNvn8::GetPoolMemoryAlignment(DeviceImpl<ApiVariationNvn8>* pDevice,
                                           const MemoryPoolInfoImpl<ApiVariationNvn8>& rInfo) {
    return 0x1000;
}

/**
 * Returns the granularity that the size of a memory pool must be a multiple of.
 *
 * @param pDevice Device the pool will be created on (unused).
 * @param rInfo Pool description (unused).
 * @return Size granularity in bytes (4 KiB).
 */
size_t MemoryPoolImplNvn8::GetPoolMemorySizeGranularity(DeviceImpl<ApiVariationNvn8>* pDevice,
                                                        const InfoType& rInfo) {
    return 0x1000;
}

/**
 * Returns the granularity that the size of a memory pool must be a multiple of.
 *
 * @param pDevice Device the pool will be created on (unused).
 * @param rInfo NVN-native pool description (unused).
 * @return Size granularity in bytes (4 KiB).
 */
size_t MemoryPoolImplNvn8::GetPoolMemorySizeGranularity(
    DeviceImpl<ApiVariationNvn8>* pDevice, const MemoryPoolInfoImpl<ApiVariationNvn8>& rInfo) {
    return 0x1000;
}

/**
 * Constructs an uninitialized memory pool, clearing all of its data.
 */
MemoryPoolImplNvn8::MemoryPoolImpl() {
    state = State_NotInitialized;
}

/**
 * Destroys the memory pool object. Finalize must have been called beforehand.
 */
MemoryPoolImplNvn8::~MemoryPoolImpl() {}

/**
 * Initializes the memory pool from a gfx pool description.
 *
 * @param pDevice Device to create the pool on.
 * @param rInfo Pool description (property flags and backing memory).
 */
void MemoryPoolImplNvn8::Initialize(DeviceImpl<Target>* pDevice, const InfoType& rInfo) {
    pNvnMemoryPool = nvnMemoryPool;
    pMemory = rInfo.GetPoolMemory();

    NVNmemoryPoolBuilder builder;
    nvnMemoryPoolBuilderSetDevice(&builder, pDevice->ToData()->pNvnDevice);
    Nvn::ConvertToNvnMemoryPoolBuilder(&builder, rInfo);
    nvnMemoryPoolInitialize(pNvnMemoryPool, &builder);

    flags.SetBit(Flag_Shared, false);
    state = State_Initialized;
}

/**
 * Initializes the memory pool from an already configured NVN memory pool builder.
 *
 * @param pDevice Device to create the pool on.
 * @param rInfo NVN-native pool description.
 */
void MemoryPoolImplNvn8::Initialize(DeviceImpl<Target>* pDevice,
                                    const MemoryPoolInfoImpl<Target>& rInfo) {
    pNvnMemoryPool = nvnMemoryPool;

    NVNmemoryPoolBuilder builder = rInfo.nvnMemoryPoolBuilder;
    pMemory = nvnMemoryPoolBuilderGetMemory(&builder);
    nvnMemoryPoolBuilderSetDevice(&builder, pDevice->ToData()->pNvnDevice);
    nvnMemoryPoolInitialize(pNvnMemoryPool, &builder);

    flags.SetBit(Flag_Shared, false);
    state = State_Initialized;
}

/**
 * Finalizes the memory pool, releasing the underlying NVN pool.
 *
 * @param pDevice Device the pool was created on (unused).
 */
void MemoryPoolImplNvn8::Finalize(DeviceImpl<Target>* pDevice) {
    nvnMemoryPoolFinalize(pNvnMemoryPool);
    pMemory = nullptr;
    state = State_NotInitialized;
}

/**
 * Maps the pool memory for CPU access.
 *
 * @return CPU address of the pool memory.
 */
void* MemoryPoolImplNvn8::Map() const {
    return nvnMemoryPoolMap(pNvnMemoryPool);
}

/**
 * Unmaps the pool memory. NVN pools stay mapped, so this does nothing.
 */
void MemoryPoolImplNvn8::Unmap() const {}

/**
 * Flushes CPU writes in a mapped range so that the GPU can see them.
 *
 * @param offset Byte offset of the range within the pool.
 * @param size Size of the range in bytes.
 */
void MemoryPoolImplNvn8::FlushMappedRange(ptrdiff_t offset, size_t size) const {
    nvnMemoryPoolFlushMappedRange(pNvnMemoryPool, offset, size);
}

/**
 * Invalidates CPU caches for a mapped range so that GPU writes become visible.
 *
 * @param offset Byte offset of the range within the pool.
 * @param size Size of the range in bytes.
 */
void MemoryPoolImplNvn8::InvalidateMappedRange(ptrdiff_t offset, size_t size) const {
    nvnMemoryPoolInvalidateMappedRange(pNvnMemoryPool, offset, size);
}

/**
 * Sets a debug label on the underlying NVN memory pool.
 *
 * @param pDevice Device the pool was created on (unused).
 * @param pLabel Label string.
 */
void MemoryPoolImplNvn8::SetDebugLabel(DeviceImpl<Target>* pDevice, const char* pLabel) {
    nvnMemoryPoolSetDebugLabel(pNvnMemoryPool, pLabel);
}

}  // namespace nn::gfx::detail
