#pragma once
#include <eui/euiDynamicCapturePane.h>
#include <prim/seadBitFlag.h>
namespace eui {
class CapturePane : public nn::ui2d::Pane {
public:
    CapturePane(const nn::ui2d::ResPane* pResource, const nn::ui2d::BuildArgSet& rArgs);
    CapturePane(const CapturePane& rOther, LayoutEx* pLayout);
    ~CapturePane() override;
    NN_RUNTIME_TYPEINFO(nn::ui2d::Pane);
    void Calculate(nn::ui2d::DrawInfo&, CalculateContext&, bool) override;
    void Draw(nn::ui2d::DrawInfo&, nn::gfx::CommandBuffer&) override;
    void initialize_(LayoutEx* pLayout);
    static sead::Color4f* setupClearColor_(sead::Heap* pHeap, nn::ui2d::Pane* pPane,
                                          LayoutEx* pLayout, sead::BitFlag8* pFlags);
    void initializeCaptureTextureData_(sead::Heap* pHeap, const char* pName);
    static const agl::TextureData* drawCapture_(nn::ui2d::Pane* pPane, nn::ui2d::DrawInfo& rDrawInfo,
        sead::BitFlag8* pFlags, agl::utl::MultiFilter* pFilter, agl::RenderBuffer* pBuffer,
        agl::RenderTargetColor* pTarget, sead::Color4f* pClearColor, nn::gfx::CommandBuffer& rCommands);
    static void setupCaptureOutputAlpha255_(nn::ui2d::Pane* pPane, sead::BitFlag8* pFlags);
    sead::BitFlag8 mFlags;
    bool mCaptureRequired;
    bool mAlwaysCapture;
    bool mCalculated;
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
