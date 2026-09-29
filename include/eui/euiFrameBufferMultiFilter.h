#pragma once

#include <common/aglRenderBuffer.h>
#include <common/aglRenderTarget.h>
#include <nn/ui2d/ui2d_Pane.h>

namespace nn::ui2d { class TextureInfo; }
namespace agl::utl { class MultiFilter; }

namespace eui {
class LayoutEx;
class DrawInfoEx;
class WindowEx;
class PictureEx;

class FrameBufferMultiFilter {
public:
    FrameBufferMultiFilter();
    virtual ~FrameBufferMultiFilter();
    void initialize(sead::Heap* pHeap, const nn::ui2d::Pane& rPane, LayoutEx* pLayout);
    const agl::TextureData* captureAndFilter(const nn::ui2d::Pane& rPane, DrawInfoEx& rDrawInfo);
    void applyTextureDataToWindowMaterial(WindowEx*, nn::ui2d::TextureInfo*,
        const agl::TextureData*, const DrawInfoEx&);
    void applyTextureDataToPictureMaterial(PictureEx*, nn::ui2d::TextureInfo*,
        const agl::TextureData*, const DrawInfoEx&);
    void freeResultTexture(const agl::TextureData* pTexture);

private:
    agl::utl::MultiFilter* m_pMultiFilter;
    agl::RenderBuffer m_RenderBuffer;
    agl::RenderTargetColor m_RenderTarget;
    u32 _1F0;
    u8 mFlags;
    u8 _1F5;
    u8 mAlpha;
};
static_assert(sizeof(FrameBufferMultiFilter) == 0x1f8, "FrameBufferMultiFilter size");

}  // namespace eui
