#pragma once

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

    void setMipLevelNum_(s32, bool);
    s32 getMinHeight_() const;
    s32 getMinSlice_() const;
    void getTextureFormatName() const;
    u32 calcMipByteSize(u32) const;
    bool isCompressedFormat() const;
    bool isRenderTargetCompressAvailable() const;
    bool isDepthFormat() const;
    bool hasStencil() const;
    void invalidateCPUCache() const;
    void flushCPUCache() const;
    void copyToAll(DrawContext* pDrawContext, const TextureData* pDst) const;
    void copyTo(DrawContext* pDrawContext, const TextureData* pDst, s32 dstSlice, s32 dstMipLevel,
                s32 srcSlice, s32 srcMipLevel) const;
    void initializeNVNtextureBuilder(NVNtextureBuilder* pBuilder) const;
    void updateNVNtexture();
    void updateNVNtextureView(const NVNtextureView* pView);
    void initialize_(TextureType type, TextureFormat format, u32 width, u32 height, u32 slice,
                     u32 mipLevelNum, TextureAttribute attribute, MultiSampleType multiSampleType,
                     bool calcSize);
    void setDebugLabel(const sead::SafeString& debug_label);
    void getDebugLabel() const;

    const GPUMemVoidAddr& getImagePtr() const { return mImagePtr; }
    const detail::Surface& getSurface() const { return mSurface; }
    u16 getTextureType() const { return mSurface.getTextureType(); }
    u16 getWidth() const { return mSurface.getWidth(); }
    u16 getHeight() const { return mSurface.getHeight(); }
    u16 getDepth() const { return mSurface.getDepth(); }
    u8 getMultiSampleType() const { return mSurface.getMultiSampleType(); }
    u8 getMipLevelNum() const { return mSurface.getMipLevelNum(); }
    u8 getTextureAttribute() const { return mSurface.getTextureAttribute(); }
    u16 getTextureFormat() const { return mTextureFormat; }
    const driver::NVNtexture_& getTexture() const { return mTexture; }
    driver::NVNtexture_& getTexture() { return mTexture; }
    s32 getTextureID() const { return mTexture.getTextureID(); }

private:
    GPUMemVoidAddr mImagePtr;
    GPUMemVoidAddr mMipPtr;
    detail::Surface mSurface;
    u16 mTextureFormat;
    u8 mMaxMipLevel;
    driver::NVNtexture_ mTexture;
    const char* mDebugLabel;  // "agl::TextureData string"
};
static_assert(sizeof(TextureData) == 0x128);

}  // namespace agl
