#pragma once

#include <postfx/aglBlurFilter.h>

namespace al {
class BlurFilter : public agl::pfx::BlurFilter {
public:
    BlurFilter();
    ~BlurFilter() override;

    void init(s32 width, s32 height, const sead::LogicalFrameBuffer& rFrameBuffer,
              const sead::Viewport& rViewport, agl::utl::ImageFilter2D::ReduceScale scale);
    void draw(agl::DrawContext* pDrawContext, const agl::RenderBuffer& rRenderBuffer,
              agl::TextureSampler& rSampler);
};
}  // namespace al
