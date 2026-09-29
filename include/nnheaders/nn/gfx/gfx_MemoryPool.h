#pragma once

#include <nn/gfx/detail/gfx_MemoryPool-api.nvn.8.h>
#include <nn/gfx/gfx_Common.h>

namespace nn::gfx {

template <class TTarget>
class TMemoryPool : public detail::MemoryPoolImpl<typename detail::TargetVariation<TTarget>::Type> {
    NN_NO_COPY(TMemoryPool);

    typedef detail::MemoryPoolImpl<typename detail::TargetVariation<TTarget>::Type> Impl;

public:
    typedef typename Impl::Target Target;
    typedef MemoryPoolInfo InfoType;

    static size_t GetPoolMemoryAlignment(TDevice<TTarget>* pDevice, const InfoType& info) {
        return Impl::GetPoolMemoryAlignment(pDevice, info);
    }
    static size_t GetPoolMemorySizeGranularity(TDevice<TTarget>* pDevice, const InfoType& info) {
        return Impl::GetPoolMemorySizeGranularity(pDevice, info);
    }

    TMemoryPool() {}

    void Initialize(TDevice<TTarget>* pDevice, const InfoType& info) {
        return Impl::Initialize(pDevice, info);
    }
    void Finalize(TDevice<TTarget>* pDevice) { return Impl::Finalize(pDevice); }
    void* Map() const;
    void Unmap() const;
    void FlushMappedRange(ptrdiff_t, size_t) const;
    void InvalidateMappedRange(ptrdiff_t, size_t) const;
    void SetUserPtr(void*);
    void* GetUserPtr();
    const void* GetUserPtr() const;
};

}  // namespace nn::gfx
