#pragma once

#include <nn/gfx/detail/gfx_Buffer-api.nvn.8.h>
#include <nn/gfx/gfx_Common.h>

namespace nn::gfx {

template <class TTarget>
class TBuffer : public detail::BufferImpl<typename detail::TargetVariation<TTarget>::Type> {
    NN_NO_COPY(TBuffer);

    typedef detail::BufferImpl<typename detail::TargetVariation<TTarget>::Type> Impl;

public:
    typedef typename Impl::Target Target;
    typedef BufferInfo InfoType;

    static const bool IsMemoryPoolRequired = true;

    static size_t GetBufferAlignment(TDevice<TTarget>* pDevice, const InfoType& info) {
        return Impl::GetBufferAlignment(pDevice, info);
    }

    TBuffer() {}
    void Initialize(TDevice<TTarget>* pDevice, const InfoType& info,
                    TMemoryPool<TTarget>* pMemoryPool, ptrdiff_t memoryPoolOffset,
                    size_t memoryPoolSize) {
        return Impl::Initialize(pDevice, info, pMemoryPool, memoryPoolOffset, memoryPoolSize);
    }

    void Finalize(TDevice<Target>* pDevice) { return Impl::Finalize(pDevice); }

    void* Map() const { return Impl::Map(); }

    template <typename T>
    T* Map() const {
        return static_cast<T*>(Impl::Map());
    }
    void Unmap() const { return Impl::Unmap(); }
    void FlushMappedRange(ptrdiff_t offset, size_t size) const {
        return Impl::FlushMappedRange(offset, size);
    }
    void InvalidateMappedRange(ptrdiff_t offset, size_t size) const {
        return Impl::InvalidateMappedRange(offset, size);
    }
    void GetGpuAddress(GpuAddress* pOutGpuAddress) const {
        return Impl::GetGpuAddress(pOutGpuAddress);
    }
    void SetUserPtr(void*);
    void* GetUserPtr();
    const void* GetUserPtr() const;
};

}  // namespace nn::gfx