#include "gfx/seadViewport.h"

#include "gfx/seadDrawContext.h"
#include "gfx/seadFrameBuffer.h"
#include "gfx/seadProjection.h"
#include "math/seadGeometry.h"
#include "nvn/nvn_FuncPtrInline.h"

namespace sead
{
/**
 * Constructs an undefined viewport with the default device posture.
 */
Viewport::Viewport()
{
    setUndef();
}

/**
 * Constructs a viewport from a corner and a size.
 * @param x Left edge.
 * @param y Top edge.
 * @param w Width.
 * @param h Height.
 */
Viewport::Viewport(f32 x, f32 y, f32 w, f32 h) : BoundBox2f(x, y, x + w, y + h) {}

/**
 * Constructs a viewport from a bounding box.
 * @param rBoundBox Viewport area.
 */
Viewport::Viewport(const BoundBox2f& rBoundBox) : BoundBox2f(rBoundBox) {}

/**
 * Constructs a viewport covering a frame buffer.
 * @param rFrameBuffer Frame buffer to cover.
 */
Viewport::Viewport(const LogicalFrameBuffer& rFrameBuffer)
{
    setByFrameBuffer(rFrameBuffer);
}

/**
 * Sets the viewport to cover a frame buffer, accounting for rotation.
 * @param rFrameBuffer Frame buffer to cover.
 */
void Viewport::setByFrameBuffer(const LogicalFrameBuffer& rFrameBuffer)
{
    switch (mDevicePosture)
    {
    case Graphics::cDevicePosture_Same:
    case Graphics::cDevicePosture_RotateHalfAround:
    case Graphics::cDevicePosture_FlipX:
    case Graphics::cDevicePosture_FlipY:
        set(0.0f, 0.0f, rFrameBuffer.getVirtualSize().x, rFrameBuffer.getVirtualSize().y);
        break;
    case Graphics::cDevicePosture_RotateRight:
    case Graphics::cDevicePosture_RotateLeft:
        set(0.0f, 0.0f, rFrameBuffer.getVirtualSize().y, rFrameBuffer.getVirtualSize().x);
        break;
    default:
        break;
    }
}

/**
 * Applies the scissor, viewport and depth range to a draw context.
 * @param pContext Draw context.
 * @param rFrameBuffer Target frame buffer.
 */
void Viewport::apply(DrawContext* pContext, const LogicalFrameBuffer& rFrameBuffer) const
{
    Vector2f pos;
    getOnFrameBufferPos(&pos, rFrameBuffer);
    Vector2f size;
    getOnFrameBufferSize(&size, rFrameBuffer);
    pos.y = rFrameBuffer.getPhysicalArea().getSizeY() - size.y - pos.y;

    auto* cmd = static_cast<NVNcommandBuffer*>(
        pContext->getCommandBuffer()->ToData()->pNvnCommandBuffer.ptr);
    nvnCommandBufferSetScissor(cmd, pos.x, pos.y, static_cast<u32>(size.x),
                               static_cast<u32>(size.y));
    nvnCommandBufferSetViewport(cmd, pos.x, pos.y, static_cast<u32>(size.x),
                                static_cast<u32>(size.y));
    nvnCommandBufferSetDepthRange(cmd, mMinDepth, mMaxDepth);
}

void Viewport::getOnFrameBufferPos(Vector2f* pPos, const LogicalFrameBuffer& rFrameBuffer) const
{
    *pPos = getMin();

    const Vector2f& virtual_size = rFrameBuffer.getVirtualSize();
    switch (mDevicePosture)
    {
    case Graphics::cDevicePosture_RotateRight:
        pPos->set(pPos->y, virtual_size.y - getSizeX() - pPos->x);
        break;
    case Graphics::cDevicePosture_RotateLeft:
        pPos->set(virtual_size.x - getSizeY() - pPos->y, pPos->x);
        break;
    case Graphics::cDevicePosture_RotateHalfAround:
        pPos->set(virtual_size.x - getSizeX() - pPos->x, virtual_size.y - getSizeY() - pPos->y);
        break;
    case Graphics::cDevicePosture_FlipX:
        pPos->set(virtual_size.x - getSizeX() - pPos->x, pPos->y);
        break;
    case Graphics::cDevicePosture_FlipY:
        pPos->set(pPos->x, virtual_size.y - getSizeY() - pPos->y);
        break;
    default:
        break;
    }

    pPos->x /= rFrameBuffer.getVirtualSize().x;
    pPos->y /= rFrameBuffer.getVirtualSize().y;
    pPos->x *= rFrameBuffer.getPhysicalArea().getSizeX();
    pPos->y *= rFrameBuffer.getPhysicalArea().getSizeY();
    pPos->x += rFrameBuffer.getPhysicalArea().getMin().x;
    pPos->y += rFrameBuffer.getPhysicalArea().getMin().y;
}

/**
 * Computes the viewport size in frame buffer pixels.
 * @param pSize Receives the size.
 * @param rFrameBuffer Target frame buffer.
 */
void Viewport::getOnFrameBufferSize(Vector2f* pSize, const LogicalFrameBuffer& rFrameBuffer) const
{
    pSize->set(getSizeX(), getSizeY());
    if (mDevicePosture == Graphics::cDevicePosture_RotateRight ||
        mDevicePosture == Graphics::cDevicePosture_RotateLeft)
    {
        pSize->set(pSize->y, pSize->x);
    }

    pSize->x /= rFrameBuffer.getVirtualSize().x;
    pSize->y /= rFrameBuffer.getVirtualSize().y;
    pSize->x *= rFrameBuffer.getPhysicalArea().getSizeX();
    pSize->y *= rFrameBuffer.getPhysicalArea().getSizeY();
}

/**
 * Applies the viewport and depth range to a draw context.
 * @param pContext Draw context.
 * @param rFrameBuffer Target frame buffer.
 */
void Viewport::applyViewport(DrawContext* pContext, const LogicalFrameBuffer& rFrameBuffer) const
{
    Vector2f pos;
    getOnFrameBufferPos(&pos, rFrameBuffer);
    Vector2f size;
    getOnFrameBufferSize(&size, rFrameBuffer);
    pos.y = rFrameBuffer.getPhysicalArea().getSizeY() - size.y - pos.y;

    auto* cmd = static_cast<NVNcommandBuffer*>(
        pContext->getCommandBuffer()->ToData()->pNvnCommandBuffer.ptr);
    nvnCommandBufferSetViewport(cmd, pos.x, pos.y, static_cast<u32>(size.x),
                                static_cast<u32>(size.y));
    nvnCommandBufferSetDepthRange(cmd, mMinDepth, mMaxDepth);
}

/**
 * Applies the scissor rectangle to a draw context.
 * @param pContext Draw context.
 * @param rFrameBuffer Target frame buffer.
 */
void Viewport::applyScissor(DrawContext* pContext, const LogicalFrameBuffer& rFrameBuffer) const
{
    Vector2f pos;
    getOnFrameBufferPos(&pos, rFrameBuffer);
    Vector2f size;
    getOnFrameBufferSize(&size, rFrameBuffer);
    pos.y = rFrameBuffer.getPhysicalArea().getSizeY() - size.y - pos.y;

    auto* cmd = static_cast<NVNcommandBuffer*>(
        pContext->getCommandBuffer()->ToData()->pNvnCommandBuffer.ptr);
    nvnCommandBufferSetScissor(cmd, pos.x, pos.y, static_cast<u32>(size.x),
                               static_cast<u32>(size.y));
}

/**
 * Projects a screen position onto the viewport.
 * @param pDst Receives the viewport position.
 * @param rScreenPos Normalized screen position.
 */
void Viewport::project(Vector2f* pDst, const Vector3f& rScreenPos) const
{
    f32 w = getSizeX() * 0.5f;
    pDst->x = rScreenPos.x * w;
    f32 h = getSizeY() * 0.5f;
    pDst->y = rScreenPos.y * h;
}

/**
 * Projects a screen position onto the viewport.
 * @param pDst Receives the viewport position.
 * @param rScreenPos Normalized screen position.
 */
void Viewport::project(Vector2f* pDst, const Vector2f& rScreenPos) const
{
    f32 w = getSizeX() * 0.5f;
    pDst->x = rScreenPos.x * w;
    f32 h = getSizeY() * 0.5f;
    pDst->y = rScreenPos.y * h;
}

/**
 * Converts a viewport position into a world position.
 * @param pDst Receives the world position.
 * @param rPos Viewport position.
 * @param rProjection Projection to use.
 * @param rCamera Camera to use.
 */
void Viewport::unproject(Vector3f* pDst, const Vector2f& rPos, const Projection& rProjection,
                         const Camera& rCamera) const
{
    Vector3f screen_pos;
    screen_pos.x = rPos.x / (getSizeX() * 0.5f);
    screen_pos.y = rPos.y / (getSizeY() * 0.5f);
    screen_pos.z = 0.0f;
    rProjection.unproject(pDst, screen_pos, rCamera);
}

/**
 * Converts a viewport position into a world-space ray.
 * @param pDst Receives the ray.
 * @param rPos Viewport position.
 * @param rProjection Projection to use.
 * @param rCamera Camera to use.
 */
void Viewport::unprojectRay(Ray<Vector3f>* pDst, const Vector2f& rPos,
                            const Projection& rProjection, const Camera& rCamera) const
{
    Vector3f screen_pos;
    screen_pos.x = rPos.x / (getSizeX() * 0.5f);
    screen_pos.y = rPos.y / (getSizeY() * 0.5f);
    screen_pos.z = 0.0f;
    rProjection.unprojectRay(pDst, screen_pos, rCamera);
}

}  // namespace sead
