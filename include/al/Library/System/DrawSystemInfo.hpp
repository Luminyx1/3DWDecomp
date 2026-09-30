#pragma once

namespace agl {
class DrawContext;
}

namespace sead {
class FrameBuffer;
}

namespace al {
struct DrawSystemInfo {
    sead::FrameBuffer* mDockedFrameBuffer;
    sead::FrameBuffer* mHandheldFrameBuffer;
    bool mIsDocked;
    agl::DrawContext* mDrawContext;
};
}  // namespace al
