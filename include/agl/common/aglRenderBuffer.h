#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>
#include <gfx/seadFrameBuffer.h>
#include <math/seadBoundBox.h>
#include <math/seadVector.h>
#include <prim/seadRuntimeTypeInfo.h>

namespace sead {
class DisplayBuffer;
class DrawContext;
class Viewport;
}  // namespace sead

namespace agl {

class DrawContext;
class RenderTargetColor;
class RenderTargetDepth;
class TextureData;

class RenderBuffer : public sead::FrameBuffer {
    SEAD_RTTI_OVERRIDE(RenderBuffer, sead::FrameBuffer)
public:
    static constexpr s32 cRenderTargetColorMax = 8;

    RenderBuffer();
    RenderBuffer(const sead::Vector2f& rVirtualSize, const sead::BoundBox2f& rPhysicalArea);
    RenderBuffer(const sead::Vector2f& rVirtualSize, f32 physicalX, f32 physicalY, f32 physicalW,
                 f32 physicalH);
    ~RenderBuffer() override;

    void copyToDisplayBuffer(sead::DrawContext* pDrawContext,
                             const sead::DisplayBuffer* pDisplayBuffer) const override;
    void clear(sead::DrawContext* pDrawContext, u32 clearFlag, const sead::Color4f& rColor, f32 depth,
               u32 stencil) const override;
    void bindImpl_(sead::DrawContext* pDrawContext) const override;

    void initialize_();
    void setRenderTargetColorNullAll();
    void adjustPhysicalAreaAndVirtualSizeFromColorTarget(u32 colorIndex);
    void invalidateGPUCache(DrawContext* pDrawContext) const;
    void bind_(DrawContext* pDrawContext, u16 srgbBitmap) const;
    void clear(DrawContext* pDrawContext, u32 target, u32 clearFlag, const sead::Color4f& rColor,
               f32 depth, u32 stencil) const;
    void fastClear(DrawContext* pDrawContext, u32 target, u32 clearFlag, const sead::Color4f& rColor,
                   f32 depth, u32 stencil, const sead::Viewport& rViewport, bool unused) const;
    void drawFlipYGL_(DrawContext* pDrawContext, bool flipX, bool flipY) const;
    void clearDrawQuad(DrawContext* pDrawContext, u32 target, u32 clearFlag,
                       const sead::Color4f& rColor, f32 depth, u32 stencil, bool unused) const;
    static bool checkRenderState();
    static bool initTextureDataFromBoundColor(DrawContext* pDrawContext, TextureData* pTextureData,
                                              s32 colorIndex);
    bool initTextureDataFromColor(DrawContext* pDrawContext, TextureData* pTextureData,
                                  s32 colorIndex) const;
    static bool initTextureDataFromBoundDepth(DrawContext* pDrawContext,
                                              TextureData* pTextureData);
    bool initTextureDataFromDepth(DrawContext* pDrawContext, TextureData* pTextureData) const;
    static bool copyTextureDataFromBoundColor(DrawContext* pDrawContext,
                                              const TextureData* pTextureData, s32 colorIndex);
    bool copyTextureDataFromColor(DrawContext* pDrawContext, const TextureData* pTextureData,
                                  s32 colorIndex) const;
    static bool copyTextureDataFromBoundDepth(DrawContext* pDrawContext,
                                              const TextureData* pTextureData);
    bool copyTextureDataFromDepth(DrawContext* pDrawContext,
                                  const TextureData* pTextureData) const;
    bool checkValidTextureSize_(const TextureData& pTextureData, s32 colorIndex) const;

    RenderTargetColor* getRenderTargetColor(s32 index = 0) const { return mRenderTargetColor[index]; }
    RenderTargetDepth* getRenderTargetDepth() const { return mRenderTargetDepth; }
    void setRenderTargetColor(RenderTargetColor* pTarget, s32 index = 0)
    {
        mRenderTargetColor[index] = pTarget;
    }
    void setRenderTargetDepth(RenderTargetDepth* pTarget) { mRenderTargetDepth = pTarget; }

    static bool sIsSRGBWrite;

private:
    RenderTargetColor* mRenderTargetColor[cRenderTargetColorMax];
    RenderTargetDepth* mRenderTargetDepth;
};
static_assert(sizeof(RenderBuffer) == 0x68);

}  // namespace agl
