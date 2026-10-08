/**
 * @file vfx_BufferAllocator.h
 * @brief GPU buffer allocator used by the VFX runtime (minimal declaration).
 */

#pragma once

#include <nn/gfx/gfx_GpuAddress.h>
#include <nn/types.h>

namespace nn {
namespace vfx {
namespace detail {

/**
 * Sub-allocates CPU/GPU visible memory out of a single gfx buffer.
 * Only the members touched by the decompiled code are named.
 */
class BufferAllocator {
public:
    void* Alloc(size_t size, size_t alignment);
    void Free(void* pMemory, bool isImmediate);

    /** @return the alignment every allocation is rounded up to */
    size_t GetAlignment() const { return m_Alignment; }

    /**
     * Computes the GPU address of a CPU pointer inside the managed buffer.
     * @param pOutAddress receives the GPU address
     * @param pCpuAddress CPU address inside the managed buffer
     */
    void GetGpuAddress(nn::gfx::GpuAddress* pOutAddress, const void* pCpuAddress) const {
        *pOutAddress = m_GpuAddress;
        pOutAddress->Offset(static_cast<const u8*>(pCpuAddress) -
                            static_cast<const u8*>(m_pBufferTop));
    }

private:
    u8 _0[0x170];
    void* m_pBufferTop;
    u8 _178[0x1C8 - 0x178];
    size_t m_Alignment;
    u8 _1d0[0x1D8 - 0x1D0];
    nn::gfx::GpuAddress m_GpuAddress;
};

}  // namespace detail
}  // namespace vfx
}  // namespace nn
