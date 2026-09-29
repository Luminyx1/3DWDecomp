#pragma once

#include <eui/euiDynamicCapturePane.h>
#include <nn/ui2d/ui2d_DrawInfo.h>

namespace sead {
class FrameBuffer;
class GraphicsContext;
class Viewport;
class DrawContext;
}

namespace eui {

class DrawInfoEx : public nn::ui2d::DrawInfo {
public:
    struct RenderBufferInfo {
        const sead::FrameBuffer* pFrameBuffer;
        const sead::GraphicsContext* pGraphicsContext;
        const sead::Viewport* pViewport;
        const sead::Viewport* pScissor;
        sead::DrawContext* pDrawContext;
    };

    NN_RUNTIME_TYPEINFO(nn::ui2d::DrawInfo);
    void freeDynamicTexture();
    static void applyRenderBufferInfo(const RenderBufferInfo* pInfo);

    const RenderBufferInfo* m_pRenderBufferInfo;
    bool _1A8;
    nn::util::IntrusiveList<DynamicCapturePane,
        nn::util::IntrusiveListMemberNodeTraits<DynamicCapturePane,
            &DynamicCapturePane::m_CaptureLink>> m_DynamicCapturePanes;
};

}  // namespace eui
