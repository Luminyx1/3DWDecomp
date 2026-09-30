#pragma once

#include <nn/ui2d/ui2d_Pane.h>
#include <nn/ui2d/ui2d_TextureInfo.h>
#include <common/aglRenderBuffer.h>
#include <common/aglRenderTarget.h>

namespace nn::ui2d { class TextureInfo; }
namespace agl::utl { class MultiFilter; }

namespace eui {
class LayoutEx;

class DynamicCapturePane : public nn::ui2d::Pane {
public:
    DynamicCapturePane(const nn::ui2d::ResPane*, const nn::ui2d::BuildArgSet&);
    DynamicCapturePane(const DynamicCapturePane&, LayoutEx*);
    ~DynamicCapturePane() override;
    NN_RUNTIME_TYPEINFO(nn::ui2d::Pane);
    void Calculate(nn::ui2d::DrawInfo&, CalculateContext&, bool) override;
    void Draw(nn::ui2d::DrawInfo&, nn::gfx::CommandBuffer&) override;
    void freeDynamicTexture();
    void applyTextureInfoToMaterialForCalculate(nn::ui2d::Pane*, const nn::ui2d::Size&, int);
    void initialize_(LayoutEx* pLayout);

    const nn::ui2d::TextureInfo& getTextureInfo() const {
        return m_TextureInfo;
    }
    const agl::TextureData* getDynamicTexture() const { return m_pDynamicTexture; }

    nn::util::IntrusiveListNode m_CaptureLink;
    u8 mFlags;
    sead::Color4f* m_pClearColor;
    agl::utl::MultiFilter* m_pMultiFilter;
    nn::ui2d::PlacementTextureInfo m_TextureInfo;
    const agl::TextureData* m_pDynamicTexture;
    agl::RenderBuffer m_RenderBuffer;
    agl::RenderTargetColor m_RenderTarget;
};
static_assert(sizeof(DynamicCapturePane) == 0x300, "DynamicCapturePane size");

}  // namespace eui
