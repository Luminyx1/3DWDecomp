#pragma once

#include <nn/gfx/gfx_Types.h>
#include <prim/seadSafeString.h>
#include "common/aglGPUMemAddr.h"
#include "detail/aglSurface.h"
#include "driver/aglNVNtexture.h"

namespace agl {

class DrawContext;

class TextureData {
public:
    class CompressToWork {
    public:
        explicit CompressToWork(const TextureData&);

        void* _0;
        void* _8;
        void* _10;
        void* _18;
        u32 _20;
        void* _28;
        void* _30;
        u32 _38;
        void* _40;
        detail::Surface mSurface;
        driver::NVNtexture_ mTexture;
    };

    TextureData();

    void initialize_(TextureType type, TextureFormat format, u32 width, u32 height, u32 slice,
                     u32 mipLevelNum, TextureAttribute attribute, MultiSampleType multiSampleType,
                     bool calcSize);
    void initializeSize_(u32 width, u32 height, u32 slice);
    void setMipLevelNum_(u32 mipLevelNum, bool calcSize);
    u32 getMinHeight_() const;
    u32 getMinSlice_() const;
    sead::SafeString getTextureFormatName() const;
    u32 calcMipByteSize(u32 mipLevel) const;
    bool isCompressedFormat() const;
    void compressTo(DrawContext* pDrawContext, const TextureData* pDestination,
                    s32 slice, s32 mipLevel) const;
    bool isRenderTargetCompressAvailable() const;
    bool isDepthFormat() const;
    bool hasStencil() const;
    void invalidateCPUCache() const;
    void flushCPUCache() const;
    void copyToAll(DrawContext* pDrawContext, const TextureData* pDst) const;
    void copyTo_(DrawContext* pDrawContext, const TextureData* pDst, s32 dstSlice, s32 dstMipLevel,
                 s32 srcSlice, s32 srcMipLevel, bool) const;
    void copyTo(DrawContext* pDrawContext, const TextureData* pDst, s32 slice,
                s32 mipLevel) const;
    void copyTo(DrawContext* pDrawContext, const TextureData* pDst, s32 dstSlice, s32 dstMipLevel,
                s32 srcSlice, s32 srcMipLevel) const;
    void setDebugLabel(const sead::SafeString& rDebugLabel);
    sead::SafeString getDebugLabel() const;
    void setImagePtr(GPUMemVoidAddr imagePtr, u32 releaseOnly);
    void setImagePtr(GPUMemVoidAddr imagePtr) { setImagePtr(imagePtr, 0); }
    void updateNVNtexture();
    void shareImagePtr(const TextureData& rOther);
    void setMipPtr(GPUMemVoidAddr mipPtr);
    void setCompSel(TextureCompSel r, TextureCompSel g, TextureCompSel b, TextureCompSel a);
    void setCompSelDefault();
    void initializeFromNVNtexture(const NVNtexture& rTexture);
    void initializeNVNtexture(NVNtexture* pTexture, GPUMemVoidAddr imagePtr) const;
    void updateNVNtextureView(const NVNtextureView* pView);
    void initializeNVNtextureBuilder(NVNtextureBuilder* pBuilder) const;
    void initializeGfxTexture(nn::gfx::Texture* pTexture) const;
    void releaseNVNtexture();
    void forceChangeUNormToSRGB();
    void forceChangeSRGBToUNorm();
    bool isSRGB() const;
    bool isUNorm() const;
    bool isEnableChangeToSRGB() const;
    bool isEnableChangeToUNorm() const;
    void initializeCubeMapArray(TextureFormat format, u32 width, u32 height, u32 arrayNum,
                                u32 mipLevelNum, TextureAttribute attribute);

    GPUMemVoidAddr getImagePtr() const { return mImagePtr; }
    GPUMemVoidAddr getMipPtr() const { return mMipPtr; }
    const detail::Surface& getSurface() const { return mSurface; }
    u16 getTextureType() const { return mSurface.getTextureType(); }
    u16 getWidth() const { return mSurface.getWidth(); }
    u16 getHeight() const { return mSurface.getHeight(); }
    u16 getDepth() const { return mSurface.getDepth(); }
    u8 getMultiSampleType() const { return mSurface.getMultiSampleType(); }
    u8 getMipLevelNum() const { return mSurface.getMipLevelNum(); }
    u8 getTextureAttribute() const { return mSurface.getTextureAttribute(); }
    u16 getTextureFormat() const { return mTextureFormat; }
    u32 getAlignment() const { return mSurface.mAlignment; }
    u32 getImageByteSize() const { return mSurface.mStorageSize; }
    const driver::NVNtexture_& getTexture() const { return mTexture; }
    driver::NVNtexture_& getTexture() { return mTexture; }
    s32 getTextureID() const { return mTexture.getTextureID(); }

    bool isMultiSample() const {
        return mSurface.mTarget == NVN_TEXTURE_TARGET_2D_MULTISAMPLE ||
               mSurface.mTarget == NVN_TEXTURE_TARGET_2D_MULTISAMPLE_ARRAY;
    }

    u32 getWidth(s32 mipLevel) const {
        u32 width = mSurface.mWidth >> mipLevel;
        return width > 1 ? width : 1;
    }

    u32 getHeight(s32 mipLevel) const {
        s32 min = getMinHeight_();
        s32 height = mSurface.mHeight >> mipLevel;
        return height < min ? min : height;
    }

    s32 getMipWidth(s32 mipLevel) const {
        s32 width = mSurface.mWidth >> mipLevel;
        return width > 1 ? width : 1;
    }

    s32 getMipHeight(s32 mipLevel) const {
        s32 min = getMinHeight_();
        s32 height = mSurface.mHeight >> mipLevel;
        return height < min ? min : height;
    }

    s32 getMipSlice(s32 mipLevel) const {
        s32 min = getMinSlice_();
        s32 slice = mSurface.mDepth >> mipLevel;
        return min > slice ? min : slice;
    }

private:
    GPUMemVoidAddr mImagePtr;
    GPUMemVoidAddr mMipPtr;
    detail::Surface mSurface;
    u16 mTextureFormat;
    u8 mMaxMipLevel;
    driver::NVNtexture_ mTexture;
    const char* mDebugLabel;
};
static_assert(sizeof(TextureData) == 0x128);

}  // namespace agl
