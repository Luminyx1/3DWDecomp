#include "Project/Draw/GpuPerf.hpp"

#include <frame_control/aglGPUStressChecker.h>
#include <gfx/seadFrameBuffer.h>
#include <gfx/seadViewport.h>

namespace al {

/**
 * Creates and enables the GPU stress checker.
 */
GpuPerf::GpuPerf() {
    agl::fctr::GPUStressChecker::createInstance(nullptr);
    agl::fctr::GPUStressChecker::instance()->initialize(nullptr);
    agl::fctr::GPUStressChecker::instance()->setFlag(agl::fctr::GPUStressChecker::cFlag_Enable);
}

/**
 * Starts a GPU time measurement.
 * @param pContext Draw context.
 */
void GpuPerf::beginPerf(agl::DrawContext* pContext) {
    agl::fctr::GPUStressChecker::instance()->start(pContext);
}

/**
 * Ends a GPU time measurement.
 * @param pContext Draw context.
 */
void GpuPerf::endPerf(agl::DrawContext* pContext) {
    agl::fctr::GPUStressChecker::instance()->end(pContext);
}

/**
 * Updates the GPU stress checker.
 */
void GpuPerf::update() {
    agl::fctr::GPUStressChecker::instance()->calc();
}

/**
 * Draws the measured GPU load.
 * @param pContext Draw context.
 * @param pFrameBuffer Frame buffer to draw to.
 */
void GpuPerf::drawResult(agl::DrawContext* pContext, const sead::FrameBuffer* pFrameBuffer) const {
    sead::Viewport viewport(*pFrameBuffer);
    agl::fctr::GPUStressChecker::instance()->drawDebug(pContext, *pFrameBuffer, viewport);
}

}  // namespace al
