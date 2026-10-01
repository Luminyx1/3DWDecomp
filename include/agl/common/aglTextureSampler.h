#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>
#include <nvn/nvn.h>
#include <prim/seadBitFlag.h>
#include <thread/seadAtomic.h>
#include <thread/seadThread.h>
#include <time/seadTickSpan.h>

#include "common/aglTextureData.h"
#include "driver/aglNVNsampler.h"

namespace agl {

class DrawContext;
class ShaderLocation;

namespace detail {

class SamplerObject {
public:
    SamplerObject();

    f32 mBorderColor[4];
    f32 mMinLod;
    f32 mMaxLod;
    f32 mLodBias;
    u8 mMagFilter;
    u8 mMinFilter;
    u8 mMipFilter;
    u8 mWrapX;
    u8 mWrapY;
    u8 mWrapZ;
    u8 mMaxAnisotropy;
    u8 mCompareFunc;
};
static_assert(sizeof(SamplerObject) == 0x24);

class CompSel {
public:
    CompSel();
    void setDefault(TextureFormat format);

    u8 mR;
    u8 mG;
    u8 mB;
    u8 mA;
};
static_assert(sizeof(CompSel) == 0x4);

}  // namespace detail

class TextureSampler {
public:
    enum UpdateFlag {
        cUpdateFlag_TextureData = 1 << 0,
        cUpdateFlag_SamplerMask = 0x3e,
        cUpdateFlag_Lock = 1u << 31,
    };

    enum Flag {
        cFlag_UseTextureView = 1 << 1,
        cFlag_CompareEnable = 1 << 2,
        cFlag_TextureViewApplied = 1 << 6,
    };

    TextureSampler();
    explicit TextureSampler(const TextureData& rTextureData);

    void applyTextureData(const TextureData& rTextureData);
    bool activate(DrawContext* pDrawContext, const ShaderLocation& rLocation, s32 unused,
                  bool unused2) const;
    void setReference() const;

    const TextureData& getTextureData() const { return mTextureData; }

    void setWrap(u8 wrapX, u8 wrapY, u8 wrapZ)
    {
        mSamplerObject.mWrapX = wrapX;
        mSamplerObject.mWrapY = wrapY;
        mSamplerObject.mWrapZ = wrapZ;
        setUpdateFlag_(1 << 1);
    }

    void setWrapDirect(u8 wrapX, u8 wrapY, u8 wrapZ)
    {
        mSamplerObject.mWrapX = wrapX;
        mSamplerObject.mWrapY = wrapY;
        mSamplerObject.mWrapZ = wrapZ;
        sead::detail::atomicReadModifyWrite(reinterpret_cast<volatile u32*>(&mUpdateFlags),
                                            [](u32 value) { return value | (1 << 1); });
    }

    void setWrapX(u8 wrap)
    {
        mSamplerObject.mWrapX = wrap;
        setUpdateFlag_(1 << 1);
    }

    void setWrapY(u8 wrap)
    {
        mSamplerObject.mWrapY = wrap;
        setUpdateFlag_(1 << 1);
    }

    void setFilter(u8 magFilter, u8 minFilter, u8 mipFilter)
    {
        mSamplerObject.mMagFilter = magFilter;
        mSamplerObject.mMinFilter = minFilter;
        mSamplerObject.mMipFilter = mipFilter;
        setUpdateFlag_(1 << 2);
    }

    void setFilterDirect(u8 magFilter, u8 minFilter, u8 mipFilter)
    {
        mSamplerObject.mMagFilter = magFilter;
        mSamplerObject.mMinFilter = minFilter;
        mSamplerObject.mMipFilter = mipFilter;
        sead::detail::atomicReadModifyWrite(reinterpret_cast<volatile u32*>(&mUpdateFlags),
                                            [](u32 value) { return value | (1 << 2); });
    }

    void setUseTextureView(bool enable)
    {
        mFlags.change(cFlag_UseTextureView, enable);
        setUpdateFlag_(cUpdateFlag_TextureData);
    }

    void setStencilMode(bool enable)
    {
        mFlags.change(1 << 4, enable);
        setUpdateFlag_(cUpdateFlag_TextureData);
    }

    void setCompSel(TextureCompSel r, TextureCompSel g, TextureCompSel b, TextureCompSel a)
    {
        mCompSel.mR = r;
        mCompSel.mG = g;
        mCompSel.mB = b;
        mCompSel.mA = a;
        setUseTextureView(true);
    }

    void setMagFilter(u8 filter)
    {
        mSamplerObject.mMagFilter = filter;
        setUpdateFlag_(1 << 2);
    }

    void setMinFilter(u8 filter)
    {
        mSamplerObject.mMinFilter = filter;
        setUpdateFlag_(1 << 2);
    }

    void setBorderColor(const sead::Color4f& rColor)
    {
        mSamplerObject.mBorderColor[0] = rColor.r;
        mSamplerObject.mBorderColor[1] = rColor.g;
        mSamplerObject.mBorderColor[2] = rColor.b;
        mSamplerObject.mBorderColor[3] = rColor.a;
        setUpdateFlag_(1 << 5);
    }

    void setBorderColorAsColor(const sead::Color4f& rColor)
    {
        *reinterpret_cast<sead::Color4f*>(mSamplerObject.mBorderColor) = rColor;
        setUpdateFlag_(1 << 5);
    }

    void setBorderColorDirect(const sead::Color4f& rColor)
    {
        *reinterpret_cast<sead::Color4f*>(mSamplerObject.mBorderColor) = rColor;
        sead::detail::atomicReadModifyWrite(reinterpret_cast<volatile u32*>(&mUpdateFlags),
                                            [](u32 value) { return value | (1 << 5); });
    }

    TextureSampler& operator=(const TextureSampler& rOther)
    {
        mSampler = rOther.mSampler;
        mTextureData = rOther.mTextureData;
        __builtin_memcpy(&mSamplerObject, &rOther.mSamplerObject,
                         reinterpret_cast<const u8*>(&mUpdateFlags) -
                             reinterpret_cast<const u8*>(&mSamplerObject));
        sead::detail::atomicReadModifyWrite(reinterpret_cast<volatile u32*>(&mUpdateFlags),
                                            [](u32) { return 0x7fffffffu; });
        mFlags = rOther.mFlags;
        mName = rOther.mName;
        return *this;
    }

    f32 getMinLod() const { return mSamplerObject.mMinLod; }
    f32 getMaxLod() const { return mSamplerObject.mMaxLod; }
    f32 getLodBias() const { return mSamplerObject.mLodBias; }
    u8 getMagFilter() const { return mSamplerObject.mMagFilter; }
    u8 getMinFilter() const { return mSamplerObject.mMinFilter; }
    u8 getMipFilter() const { return mSamplerObject.mMipFilter; }
    void setLod(f32 minLod, f32 maxLod, f32 lodBias)
    {
        mSamplerObject.mMinLod = minLod;
        mSamplerObject.mMaxLod = maxLod;
        mSamplerObject.mLodBias = lodBias;
        setUpdateFlag_(1 << 3);
    }
    void setMinLod(f32 lod)
    {
        mSamplerObject.mMinLod = lod;
        setUpdateFlag_(1 << 3);
    }
    void setMaxLod(f32 lod)
    {
        mSamplerObject.mMaxLod = lod;
        setUpdateFlag_(1 << 3);
    }
    void setLodBias(f32 bias)
    {
        mSamplerObject.mLodBias = bias;
        setUpdateFlag_(1 << 3);
    }
    bool isSeamlessCubeMap() const { return mFlags.isOn(1 << 5); }
    void setSeamlessCubeMap(bool enable)
    {
        mFlags.change(1 << 5, enable);
        setUpdateFlag_(1 << 6);
    }

    void setDepthCompareEnable(bool enable)
    {
        mFlags.change(cFlag_CompareEnable, enable);
        setUpdateFlag_(1 << 4);
    }

    void setDepthCompareFunc(u8 func)
    {
        mSamplerObject.mCompareFunc = func;
        setDepthCompareEnable(true);
    }

    void updateRegs() const
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
                initRegs_(flags);
            }
        }
        sead::detail::atomicReadModifyWrite(pFlags, [](u32) { return 0u; });
    }

private:
    void initRegs_(u32 flags) const;

    void setUpdateFlag_(u32 flag)
    {
        sead::detail::atomicReadModifyWrite(reinterpret_cast<volatile u32*>(&mUpdateFlags),
                                            [flag](u32 value) { return (value & ~flag) | flag; });
    }

    mutable driver::NVNsampler_ mSampler;
    mutable TextureData mTextureData;
    detail::SamplerObject mSamplerObject;
    detail::CompSel mCompSel;
    mutable u32 mUpdateFlags;
    mutable sead::BitFlag8 mFlags;
    const char* mName;
};
static_assert(sizeof(TextureSampler) == 0x170);

}  // namespace agl
