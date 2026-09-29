#pragma once

#include <basis/seadTypes.h>

namespace agl {
    class DrawContext;
};

namespace sead {
    class FrameBuffer;
};

namespace al {
    /// Measures the GPU load through agl's GPU stress checker.
    class GpuPerf {
    public:
        GpuPerf();

        void beginPerf(agl::DrawContext* pDrawContext);
        void endPerf(agl::DrawContext* pDrawContext);
        void update();
        void drawResult(agl::DrawContext* pDrawContext, const sead::FrameBuffer* pFrameBuffer) const;
    };
};
