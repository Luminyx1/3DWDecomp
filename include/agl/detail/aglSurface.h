#pragma once

#include <basis/seadTypes.h>
#include <nvn/nvn.h>
#include "common/aglTextureEnum.h"

namespace agl::detail {

class CompSel;

/// Storage of a detail::CompSel (red, green, blue and alpha selectors).
struct CompSelData {
    u8 mR;
    u8 mG;
    u8 mB;
    u8 mA;
};

struct SurfaceBase {
    u16 mWidth;
    u16 mHeight;
    u16 mDepth;
    u16 mFormat;  // NVNformat
    u8 mSamples;
    u8 mLevels;
    u16 mTarget;  // NVNtextureTarget
    u32 mAlignment;
    u32 mStorageSize;
    u32 _14;
};
static_assert(sizeof(SurfaceBase) == 0x18);

class Surface : public SurfaceBase {
public:
    enum AttributeFlag {
        cAttribute_Linear = 1 << 0,
        cAttribute_Compressible = 1 << 1,
        cAttribute_RenderTarget = 1 << 2,
    };

    Surface();
    void initialize(TextureType type, TextureFormat format, u32 mipLevelNum,
                    TextureAttribute attribute, MultiSampleType multiSampleType);
    void initializeSize(u32 width, u32 height, u32 depth);
    void copyFrom(const SurfaceBase& rBase);
    void calcSizeAndAlignment();
    void setupNVNtextureBuilder(NVNtextureBuilder* pBuilder) const;
    void printInfo() const;
    void copyFrom(const NVNtexture& rTexture);

    u16 getWidth() const { return mWidth; }
    u16 getHeight() const { return mHeight; }
    u16 getDepth() const { return mDepth; }
    u8 getMultiSampleType() const { return mSamples; }
    u8 getMipLevelNum() const { return mLevels; }
    u16 getTextureType() const { return mTarget; }
    u8 getTextureAttribute() const { return mAttribute; }

    u16 mStorageClass;
    u8 mAttribute;
    u8 mPixelByteSize;
    u32 mStride;
    CompSelData mCompSel;

    CompSel& getCompSel() { return *reinterpret_cast<CompSel*>(&mCompSel); }
    const CompSel& getCompSel() const { return *reinterpret_cast<const CompSel*>(&mCompSel); }
};
static_assert(sizeof(Surface) == 0x24);

}  // namespace agl::detail
