#pragma once

#include <common/aglGPUMemAddr.h>
#include <utility/aglImageFilter2D.h>

namespace agl {
class DrawContext;
class RenderBuffer;
class TextureData;
}  // namespace agl

namespace sead {
class LogicalFrameBuffer;
class Viewport;
}  // namespace sead

namespace al {
class BlurFilter;

class ScreenCapture {
public:
    ScreenCapture(s32 width, s32 height);
    virtual ~ScreenCapture();

    void initBlur(const sead::LogicalFrameBuffer& rFrameBuffer, const sead::Viewport& rViewport,
                  agl::utl::ImageFilter2D::ReduceScale scale);
    void enableBlur(bool isEnable);
    void copyImageFromFrameBuffer(agl::DrawContext* pDrawContext,
                                  const agl::RenderBuffer* pRenderBuffer);
    void drawCaptureImage(agl::DrawContext* pDrawContext,
                          const agl::RenderBuffer* pRenderBuffer) const;

private:
    BlurFilter* mBlurFilter = nullptr;
    agl::TextureData* mTextureData = nullptr;
    agl::GPUMemVoidAddr mImageAddr;
};
}  // namespace al
