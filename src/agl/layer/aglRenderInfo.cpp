#include "layer/aglRenderInfo.h"

#include <gfx/seadPrimitiveRenderer.h>
#include <gfx/seadViewport.h>

#include "common/aglDrawContext.h"
#include "common/aglRenderBuffer.h"
#include "layer/aglLayer.h"

namespace agl::lyr {

/**
 * Constructs the render information for drawing a layer.
 * @param pDrawContext draw context to draw with
 * @param displayIndex index of the display being drawn
 * @param frameworkType framework type of the draw
 * @param pFrameBuffer frame buffer to draw into
 * @param isDrawDebug whether debug drawing is enabled
 * @param pLayer layer being drawn
 */
RenderInfo::RenderInfo(DrawContext* pDrawContext, s32 displayIndex, FrameworkType frameworkType,
                       const RenderBuffer* pFrameBuffer, bool isDrawDebug, const Layer* pLayer)
    : mRenderStep(-1), mFrameworkType(frameworkType), mDisplayIndex(displayIndex),
      mFrameBuffer(pFrameBuffer), mLayer(pLayer), mLayerIndex(0), mCamera(nullptr),
      mProjection(nullptr), mViewport(nullptr), mIsDrawDebug(isDrawDebug),
      mDrawContext(pDrawContext)
{
    mLayerIndex = pLayer->getLayerIndex();
    mCamera = pLayer->getRenderCamera();
    mProjection = mLayer->getRenderProjection();
    mViewport = &mLayer->getViewport();
    mIsDrawDebug &= !mLayer->mFlag.isOn(1 << 2);
}

/**
 * Constructs the render information for drawing without a layer.
 * @param pDrawContext draw context to draw with
 * @param displayIndex index of the display being drawn
 * @param pFrameBuffer frame buffer to draw into
 */
RenderInfo::RenderInfo(DrawContext* pDrawContext, s32 displayIndex,
                       const RenderBuffer* pFrameBuffer)
    : mRenderStep(0), mFrameworkType(FrameworkType(0)), mDisplayIndex(displayIndex),
      mFrameBuffer(pFrameBuffer), mLayer(nullptr), mLayerIndex(-1), mCamera(nullptr),
      mProjection(nullptr), mViewport(nullptr), mIsDrawDebug(true), mDrawContext(pDrawContext)
{
}

/**
 * Gets the frame buffer being drawn into.
 * @return frame buffer
 */
const RenderBuffer* RenderInfo::getFrameBuffer() const
{
    return mFrameBuffer;
}

/**
 * Binds the frame buffer and applies the layer viewport.
 * @param pDrawContext draw context to bind with
 */
void RenderInfo::bindFrameBufferAndApplyViewport(DrawContext* pDrawContext) const
{
    mFrameBuffer->bind(pDrawContext);
    mViewport->apply(pDrawContext, *mFrameBuffer);
}

/**
 * Sets up the primitive renderer with the camera, projection and viewport of this render.
 */
void RenderInfo::setUpPrimitiveRenderer() const
{
    sead::PrimitiveRenderer* pRenderer = sead::PrimitiveRenderer::instance();
    pRenderer->setCamera(*mCamera);
    pRenderer->setProjection(*mProjection);
    mViewport->apply(mDrawContext, *mFrameBuffer);
}

}  // namespace agl::lyr
