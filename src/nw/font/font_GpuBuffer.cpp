#include <nn/font/font_GpuBuffer.h>

#include <atomic>
#include <new>
#include <nn/util/util_BitUtil.h>

namespace nn {
namespace font {

namespace {
using BufferImpl = nn::gfx::detail::BufferImpl<nn::gfx::ApiVariationNvn8>;
}

/**
 * Allocates and initializes the GPU buffers.
 * @param pDevice gfx device
 * @param rArg initialization arguments
 * @return always true
 */
bool GpuBuffer::Initialize(nn::gfx::Device* pDevice, const InitializeArg& rArg) {
    nn::gfx::BufferInfo info;
    info.SetDefault();
    info.SetGpuAccessFlags(rArg.gpuAccessFlag);
    m_BufferAlignment = BufferImpl::GetBufferAlignment(pDevice, info);

    if (rArg.isUnallocated) {
        m_Flags |= Flag_Unallocated;
        return true;
    }

    m_BufferSize = rArg.bufferSize;
    m_BufferCount = rArg.bufferCount;
    info.SetSize(m_BufferSize);

    m_pBuffers = static_cast<nn::gfx::Buffer*>(
        rArg.pAllocateFunction(sizeof(nn::gfx::Buffer) * m_BufferCount, 4, rArg.pUserData));
    m_pGpuAddresses = static_cast<nn::gfx::GpuAddress*>(
        rArg.pAllocateFunction(sizeof(nn::gfx::GpuAddress) * m_BufferCount, 4, rArg.pUserData));

    for (uint32_t i = 0; i < m_BufferCount; i++) {
        new (&m_pBuffers[i]) nn::gfx::Buffer();
        ptrdiff_t offset = nn::util::align_up(m_BufferSize, m_BufferAlignment) * i;
        m_pBuffers[i].Initialize(pDevice, info, rArg.pMemoryPool, rArg.memoryPoolOffset + offset,
                                 m_BufferSize);
        m_pBuffers[i].GetGpuAddress(&m_pGpuAddresses[i]);
    }

    if (rArg.isAtomicAllocation) {
        m_Flags |= Flag_AtomicAllocation;
        m_pAtomicAllocatedSize =
            static_cast<uint64_t*>(rArg.pAllocateFunction(8, 4, rArg.pUserData));
        m_pAtomicAllocatedSize2 =
            static_cast<uint64_t*>(rArg.pAllocateFunction(8, 4, rArg.pUserData));
    }

    return true;
}

/**
 * Finalizes and frees the GPU buffers.
 * @param pDevice gfx device
 * @param pFreeFunction function that frees memory
 * @param pUserData user data passed to the callback
 */
void GpuBuffer::Finalize(nn::gfx::Device* pDevice, FreeFunction pFreeFunction, void* pUserData) {
    if (m_Flags & Flag_AtomicAllocation) {
        pFreeFunction(m_pAtomicAllocatedSize, pUserData);
        pFreeFunction(m_pAtomicAllocatedSize2, pUserData);
    }

    if (m_pBuffers != nullptr) {
        for (uint32_t i = 0; i < m_BufferCount; i++) {
            m_pBuffers[i].Finalize(pDevice);
        }

        pFreeFunction(m_pBuffers, pUserData);
    }

    if (m_pGpuAddresses != nullptr) {
        pFreeFunction(m_pGpuAddresses, pUserData);
    }

    m_pBuffers = nullptr;
    m_pGpuAddresses = nullptr;
    m_BufferSize = 0;
    m_BufferAlignment = 0;
    m_BufferCount = 0;
    m_MappedBufferIndex = -1;
    m_GpuAccessBufferIndex = 0;
    m_pMappedPointer = nullptr;
}

/**
 * Maps one of the buffers and resets the allocation counters.
 * @param bufferIndex index of the buffer to map
 */
void GpuBuffer::Map(int bufferIndex) {
    if (m_MappedBufferIndex >= 0) {
        return;
    }

    m_MappedBufferIndex = bufferIndex;

    if (m_pBuffers != nullptr) {
        m_pMappedPointer = m_pBuffers[bufferIndex].Map();
    }

    if (m_Flags & Flag_AtomicAllocation) {
        reinterpret_cast<std::atomic<uint64_t>*>(m_pAtomicAllocatedSize)->store(0);
        reinterpret_cast<std::atomic<uint64_t>*>(m_pAtomicAllocatedSize2)->store(0);
    } else {
        m_AllocatedSize = 0;
        m_AllocatedSize2 = 0;
    }
}

/**
 * Unmaps the mapped buffer and resets the allocation counters.
 */
void GpuBuffer::Unmap() {
    if (m_MappedBufferIndex < 0) {
        return;
    }

    if (m_pBuffers != nullptr) {
        m_pBuffers[m_MappedBufferIndex].Unmap();
    }

    m_MappedBufferIndex = -1;
    m_pMappedPointer = nullptr;

    if (m_Flags & Flag_AtomicAllocation) {
        reinterpret_cast<std::atomic<uint64_t>*>(m_pAtomicAllocatedSize)->store(0);
        reinterpret_cast<std::atomic<uint64_t>*>(m_pAtomicAllocatedSize2)->store(0);
    } else {
        m_AllocatedSize = 0;
        m_AllocatedSize2 = 0;
    }
}

}  // namespace font
}  // namespace nn
