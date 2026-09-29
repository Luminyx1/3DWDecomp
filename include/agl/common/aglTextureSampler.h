#pragma once

#include <basis/seadTypes.h>
#include <nvn/nvn.h>
#include <prim/seadBitFlag.h>

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

private:
    void initRegs_(u32 flags) const;

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
