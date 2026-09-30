#pragma once

namespace agl {
class DrawContext;
}

namespace sead {
class FrameBuffer;
}

namespace al {

class GpuPerf {
public:
    GpuPerf();

    void beginPerf(agl::DrawContext* pContext);
    void endPerf(agl::DrawContext* pContext);
    void update();
    void drawResult(agl::DrawContext* pContext, const sead::FrameBuffer* pFrameBuffer) const;
};

}  // namespace al
