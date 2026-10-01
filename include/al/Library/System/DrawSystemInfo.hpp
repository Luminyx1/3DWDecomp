#pragma once

namespace agl {
class DrawContext;
}

namespace sead {
class FrameBuffer;
}

namespace al {
struct DrawSystemInfo {
    DrawSystemInfo(sead::FrameBuffer* pDockedFrameBuffer, sead::FrameBuffer* pHandheldFrameBuffer,
                   agl::DrawContext* pDrawContext)
        : mDockedFrameBuffer(pDockedFrameBuffer), mHandheldFrameBuffer(pHandheldFrameBuffer),
          mDrawContext(pDrawContext) {}

    sead::FrameBuffer* mDockedFrameBuffer;
    sead::FrameBuffer* mHandheldFrameBuffer;
    bool mIsDocked = false;
    agl::DrawContext* mDrawContext;
};
}  // namespace al
