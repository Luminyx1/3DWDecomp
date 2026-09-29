#include "driver/aglGraphicsDriverMgr.h"

#include <nvn/nvn_FuncPtrInline.h>

#include "common/aglDisplayList.h"
#include "common/aglDrawContext.h"

namespace agl::driver
{
SEAD_SINGLETON_DISPOSER_IMPL(GraphicsDriverMgr)

/**
 * Constructs the driver manager without a default command buffer.
 */
GraphicsDriverMgr::GraphicsDriverMgr()
{
    mDefaultCommandBuffer = nullptr;
    _30 = nullptr;
}

/**
 * Destroys the driver manager.
 */
GraphicsDriverMgr::~GraphicsDriverMgr() = default;

/**
 * Creates the default command buffer.
 * @param pHeap heap to allocate the command buffer from
 */
void GraphicsDriverMgr::initialize_(sead::Heap* pHeap)
{
    mDefaultCommandBuffer = new (pHeap, 8) DisplayList();
}

/**
 * Gets the default command buffer.
 * @return the default command buffer
 */
DisplayList* GraphicsDriverMgr::getDefaultCommandBuffer()
{
    return mDefaultCommandBuffer;
}

/**
 * Waits for the GPU to finish drawing using a temporary draw context.
 */
void GraphicsDriverMgr::waitDrawDone() const
{
    DrawContext drawContext;
    drawContext.setCommandBufferTemporary();
    waitDrawDone(&drawContext);
}

/**
 * Sets the point size limits (unsupported on NVN).
 * @param pDrawContext draw context
 * @param min minimum point size
 * @param max maximum point size
 */
void GraphicsDriverMgr::setPointLimits(DrawContext* pDrawContext, f32 min, f32 max) const {}

/**
 * Sets the point size.
 * @param pDrawContext draw context
 * @param pointSize point size
 */
void GraphicsDriverMgr::setPointSize(DrawContext* pDrawContext, f32 pointSize) const
{
    nvnCommandBufferSetPointSize(getNvnCommandBuffer(pDrawContext), pointSize);
}

/**
 * Sets the line width.
 * @param pDrawContext draw context
 * @param lineWidth line width
 */
void GraphicsDriverMgr::setLineWidth(DrawContext* pDrawContext, f32 lineWidth) const
{
    nvnCommandBufferSetLineWidth(getNvnCommandBuffer(pDrawContext), lineWidth);
}

/**
 * Sets the primitive restart index, disabling primitive restart for an index of zero.
 * @param pDrawContext draw context
 * @param index primitive restart index
 */
void GraphicsDriverMgr::setPrimitiveRestartIndex(DrawContext* pDrawContext,
                                                 PrimitiveRestartIndex index) const
{
    nvnCommandBufferSetPrimitiveRestart(getNvnCommandBuffer(pDrawContext), index != 0, index);
}

/**
 * Enables or disables depth clamping.
 * @param pDrawContext draw context
 * @param enable whether depth clamping is enabled
 */
void GraphicsDriverMgr::setDepthClamp(DrawContext* pDrawContext, bool enable) const
{
    nvnCommandBufferSetDepthClamp(getNvnCommandBuffer(pDrawContext), enable);
}

/**
 * Sets the polygon offset without clamping.
 * @param pDrawContext draw context
 * @param factor slope factor
 * @param units constant offset units
 */
void GraphicsDriverMgr::setPolygonOffset(DrawContext* pDrawContext, f32 factor, f32 units) const
{
    nvnCommandBufferSetPolygonOffsetClamp(getNvnCommandBuffer(pDrawContext), factor, units, 0.0f);
}

/**
 * Generates the host IO message (empty in release builds).
 * @param pContext host IO context
 */
void GraphicsDriverMgr::genMessage(sead::hostio::Context* pContext) {}

/**
 * Handles a host IO property event (empty in release builds).
 * @param pEvent property event
 */
void GraphicsDriverMgr::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent) {}

}  // namespace agl::driver
