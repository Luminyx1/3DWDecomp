#include "Project/Framework/Application/GpuPerf.hpp"
#include <frame_control/aglGPUStressChecker.h>
#include <gfx/seadFrameBuffer.h>
#include <gfx/seadViewport.h>

namespace al {
    /** @brief Creates and initializes the GPU stress checker. */
    GpuPerf::GpuPerf() {
        agl::fctr::GPUStressChecker::createInstance(nullptr);
        agl::fctr::GPUStressChecker::instance()->initialize(nullptr);
        agl::fctr::GPUStressChecker::instance()->setFlag(1);
    }

    /**
     * @brief Starts a GPU time measurement.
     * @param pDrawContext The draw context to measure in.
     */
    void GpuPerf::beginPerf(agl::DrawContext* pDrawContext) {
        agl::fctr::GPUStressChecker::instance()->start(pDrawContext);
    }

    /**
     * @brief Ends a GPU time measurement.
     * @param pDrawContext The draw context to measure in.
     */
    void GpuPerf::endPerf(agl::DrawContext* pDrawContext) {
        agl::fctr::GPUStressChecker::instance()->end(pDrawContext);
    }

    /** @brief Updates the measured GPU load. */
    void GpuPerf::update() {
        agl::fctr::GPUStressChecker::instance()->calc();
    }

    /**
     * @brief Draws the measured GPU load.
     * @param pDrawContext The draw context to draw with.
     * @param pFrameBuffer The frame buffer to draw to.
     */
    void GpuPerf::drawResult(agl::DrawContext* pDrawContext, const sead::FrameBuffer* pFrameBuffer) const {
        sead::Viewport viewport(*pFrameBuffer);
        agl::fctr::GPUStressChecker::instance()->drawDebug(pDrawContext, *pFrameBuffer, viewport);
    }
};
