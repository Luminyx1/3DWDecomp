#pragma once

#include <basis/seadTypes.h>
#include <nvn/nvn.h>
#include <prim/seadBitFlag.h>
#include <thread/seadAtomic.h>
#include <thread/seadThread.h>
#include <time/seadTickSpan.h>

#include "common/aglGPUMemAddr.h"
#include "common/aglTextureData.h"

namespace agl {

class DrawContext;

template <typename T>
class RenderTarget : public TextureData {
public:
    enum UpdateFlag {
        cUpdateFlag_MipLevel = 1 << 0,
        cUpdateFlag_Slice = 1 << 1,
        cUpdateFlag_TextureData = 1 << 2,
        cUpdateFlag_Lock = 1u << 31,
    };

    RenderTarget()
        : mUpdateFlags(0xff), mSlice(0), mMipLevel(0), mFlags(0), mZCullSize(0),
          mZCullAlignment(0x800)
    {
        nvnTextureViewSetDefaults(&mTextureView);
    }

    RenderTarget(u32 mipLevel, u32 slice)
        : mUpdateFlags(0xff), mSlice(slice), mMipLevel(mipLevel), mFlags(0), mZCullSize(0),
          mZCullAlignment(0x800)
    {
        nvnTextureViewSetDefaults(&mTextureView);
    }

    void applyTextureData(const TextureData& rTextureData, u32 mipLevel, u32 slice);

    void updateRegs_() const
    {
        volatile u32* pFlags = reinterpret_cast<volatile u32*>(&mUpdateFlags);
        u32 flags = sead::detail::atomicReadModifyWrite(
            pFlags, [](u32 value) { return value | cUpdateFlag_Lock; });
        if (flags & ~cUpdateFlag_Lock)
        {
            if (flags & cUpdateFlag_Lock)
            {
                while (*pFlags & cUpdateFlag_Lock)
                {
                    sead::Thread::sleep(sead::TickSpan::makeFromMicroSeconds(1));
                }
            }
            else
            {
                static_cast<const T*>(this)->initRegs_(flags);
            }
        }
        sead::detail::atomicReadModifyWrite(pFlags, [](u32) { return 0u; });
    }

    const NVNtextureView* getTextureView() const { return &mTextureView; }
    u16 getSlice() const { return mSlice; }
    u8 getMipLevel() const { return mMipLevel; }

protected:
    void changeUpdateFlag_(u32 mask, bool on)
    {
        sead::detail::atomicReadModifyWrite(
            reinterpret_cast<volatile u32*>(&mUpdateFlags),
            [mask, on](u32 value) { return (value & ~mask) | (on ? mask : 0); });
    }

    mutable u32 mUpdateFlags;
    u16 mSlice;
    u8 mMipLevel;
    sead::BitFlag8 mFlags;
    u32 mZCullSize;
    u32 mZCullAlignment;
    GPUMemVoidAddr mZCullBuffer;
    NVNtextureView mTextureView;
};

class RenderTargetColor : public RenderTarget<RenderTargetColor> {
public:
    RenderTargetColor();
    RenderTargetColor(const TextureData& rTextureData, u32 mipLevel, u32 slice);
    ~RenderTargetColor();

    void onApplyTextureData_();
    void initRegs_(u32 index) const;
    void invalidateGPUCache(DrawContext* pDrawContext) const;
    void invalidateCPUCache() const;
    void expandAuxBuffer(DrawContext* pDrawContext) const;
};
static_assert(sizeof(RenderTargetColor) == 0x178);

class RenderTargetDepth : public RenderTarget<RenderTargetDepth> {
public:
    RenderTargetDepth();
    RenderTargetDepth(const TextureData& rTextureData, u32 mipLevel, u32 slice);
    ~RenderTargetDepth();

    void onApplyTextureData_();
    void initRegs_(u32 index) const;
    void invalidateGPUCache(DrawContext* pDrawContext) const;
    void invalidateCPUCache() const;
    void expandHiZBuffer(DrawContext* pDrawContext) const;
    void expandHiZBufferTo(DrawContext* pDrawContext, const TextureData* pDst, u32 dstSlice,
                           u32 dstMipLevel) const;
    void expandHiZBufferAllSlice(DrawContext* pDrawContext) const;
    void expandHiZBufferToAllSlice(DrawContext* pDrawContext, const TextureData* pDst) const;
};
static_assert(sizeof(RenderTargetDepth) == 0x178);

template <typename T>
void RenderTarget<T>::applyTextureData(const TextureData& rTextureData, u32 mipLevel, u32 slice)
{
    TextureData::operator=(rTextureData);
    changeUpdateFlag_(cUpdateFlag_TextureData, true);
    if (mSlice != slice)
    {
        changeUpdateFlag_(cUpdateFlag_Slice, true);
        mSlice = slice;
    }
    if (mMipLevel != mipLevel)
    {
        sead::detail::atomicReadModifyWrite(reinterpret_cast<volatile u32*>(&mUpdateFlags),
                                            [](u32 value) { return value | cUpdateFlag_MipLevel; });
        mMipLevel = mipLevel;
    }
    static_cast<T*>(this)->onApplyTextureData_();
}

}  // namespace agl
