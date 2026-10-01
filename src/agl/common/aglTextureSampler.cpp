#include "common/aglTextureSampler.h"

#include <nvn/nvn_FuncPtrInline.h>
#include <thread/seadAtomic.h>
#include <thread/seadThread.h>
#include <time/seadTickSpan.h>

#include "common/aglShaderLocation.h"
#include "driver/aglNVNMgr.h"

namespace agl {

namespace utl {

class PrimitiveTexture {
public:
    static PrimitiveTexture* sInstance;

    const TextureSampler* getDefaultSampler() const { return mDefaultSampler; }

private:
    u8 _0[0x38];
    const TextureSampler* mDefaultSampler;
};

}  // namespace utl

/**
 * Constructs a sampler for the default primitive texture, if it exists.
 */
TextureSampler::TextureSampler()
    : mUpdateFlags(0xff), mFlags(0x20), mName("agl::TextureSampler")
{
    if (utl::PrimitiveTexture::sInstance != nullptr)
    {
        applyTextureData(utl::PrimitiveTexture::sInstance->getDefaultSampler()->getTextureData());
    }
}

/**
 * Sets the texture to sample from.
 * @param rTextureData texture to sample from
 */
void TextureSampler::applyTextureData(const TextureData& rTextureData)
{
    mTextureData = rTextureData;
    sead::detail::atomicReadModifyWrite(reinterpret_cast<volatile u32*>(&mUpdateFlags),
                                        [](u32 value) { return value | cUpdateFlag_TextureData; });
}

/**
 * Constructs a sampler for a texture.
 * @param rTextureData texture to sample from
 */
TextureSampler::TextureSampler(const TextureData& rTextureData)
    : mUpdateFlags(0xff), mFlags(0x20), mName("agl::TextureSampler(copy)")
{
    applyTextureData(rTextureData);
}

/**
 * Updates the NVN state if needed and binds the texture and sampler.
 * @param pDrawContext draw context
 * @param rLocation sampler location
 * @param unused unused
 * @param unused2 unused
 * @return whether the location is valid
 */
bool TextureSampler::activate(DrawContext* pDrawContext, const ShaderLocation& rLocation,
                              s32 unused, bool unused2) const
{
    if (!rLocation.isValid())
    {
        return false;
    }

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

    driver::NVNMgr::instance()->nvnCommandBufferBindTexture(pDrawContext, mSampler.getHandle(),
                                                            rLocation,
                                                            mTextureData.getTextureID());
    return true;
}

/**
 * Marks the texture as referenced.
 */
void TextureSampler::setReference() const
{
    mTextureData.getTexture().setReference_();
}

/**
 * Rebuilds the NVN sampler and texture view.
 * @param flags update flags
 */
void TextureSampler::initRegs_(u32 flags) const
{
    bool textureChanged = flags & cUpdateFlag_TextureData;
    bool updated = textureChanged;

    if (flags & cUpdateFlag_SamplerMask)
    {
        NVNsamplerBuilder builder = {};
        nvnSamplerBuilderSetDevice(&builder, driver::NVNMgr::instance()->getNvnDevice());
        nvnSamplerBuilderSetDefaults(&builder);
        nvnSamplerBuilderSetMinMagFilter(
            &builder,
            NVNminFilter(mSamplerObject.mMipFilter * 2 + mSamplerObject.mMinFilter),
            NVNmagFilter(mSamplerObject.mMagFilter));
        nvnSamplerBuilderSetWrapMode(&builder, NVNwrapMode(mSamplerObject.mWrapX),
                                     NVNwrapMode(mSamplerObject.mWrapY),
                                     NVNwrapMode(mSamplerObject.mWrapZ));
        nvnSamplerBuilderSetLodClamp(&builder, mSamplerObject.mMinLod, mSamplerObject.mMaxLod);
        nvnSamplerBuilderSetLodBias(&builder, mSamplerObject.mLodBias);
        nvnSamplerBuilderSetCompare(&builder,
                                    NVNcompareMode(mFlags.isOn(cFlag_CompareEnable)),
                                    NVNcompareFunc(mSamplerObject.mCompareFunc));
        nvnSamplerBuilderSetBorderColor(&builder, mSamplerObject.mBorderColor);
        nvnSamplerBuilderSetMaxAnisotropy(&builder, mSamplerObject.mMaxAnisotropy);
        nvnSamplerBuilderSetReductionFilter(&builder, NVNsamplerReduction(0));

        NVNsampler sampler = {};
        nvnSamplerInitialize(&sampler, &builder);
        updated = textureChanged | mSampler.registerSampler(sampler, mName);
        nvnSamplerFinalize(&sampler);
    }

    if (textureChanged)
    {
        if (mFlags.isOn(cFlag_UseTextureView))
        {
            NVNtextureView view;
            nvnTextureViewSetDefaults(&view);
            nvnTextureViewSetSwizzle(&view, NVNtextureSwizzle(mCompSel.mR),
                                     NVNtextureSwizzle(mCompSel.mG),
                                     NVNtextureSwizzle(mCompSel.mB),
                                     NVNtextureSwizzle(mCompSel.mA));
            mTextureData.updateNVNtextureView(&view);
            mFlags.set(cFlag_TextureViewApplied);
        }
        else if (mFlags.isOn(cFlag_TextureViewApplied))
        {
            mTextureData.updateNVNtexture();
            mFlags.reset(cFlag_TextureViewApplied);
        }
    }

    if (updated)
    {
        mSampler.updateTextureId(mTextureData.getTextureID());
    }
}

}  // namespace agl
