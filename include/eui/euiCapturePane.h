#pragma once
#include <eui/euiDynamicCapturePane.h>
#include <prim/seadBitFlag.h>
namespace eui {
class CapturePane : public nn::ui2d::Pane {
public:
    CapturePane(const nn::ui2d::ResPane* pResource, const nn::ui2d::BuildArgSet& rArgs);
    ~CapturePane() override;
    NN_RUNTIME_TYPEINFO(nn::ui2d::Pane);
    void Calculate(nn::ui2d::DrawInfo&, CalculateContext&, bool) override;
    void Draw(nn::ui2d::DrawInfo&, nn::gfx::CommandBuffer&) override;
    void initialize_(LayoutEx* pLayout);
    static void setupCaptureOutputAlpha255_(nn::ui2d::Pane* pPane, sead::BitFlag<u8>* pFlags);
    u8 mFlags;
    bool _d3;
    u16 _d4;
    sead::Color4f* mClearColor;
    agl::utl::MultiFilter* mMultiFilter;
    nn::ui2d::PlacementTextureInfo mTextureInfo;
    agl::TextureData* mTexture;
    agl::RenderBuffer mRenderBuffer;
    agl::RenderTargetColor mRenderTarget;
    agl::GPUMemVoidAddr mTextureMemory;
};
static_assert(sizeof(CapturePane) == 0x300, "CapturePane size");
}
