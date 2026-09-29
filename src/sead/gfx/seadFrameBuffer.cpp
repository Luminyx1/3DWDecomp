#include <gfx/seadFrameBuffer.h>

namespace sead
{
/**
 * Binds this frame buffer by forwarding to the implementation.
 * @param pDrawContext Draw context to bind with.
 */
void FrameBuffer::bind(DrawContext* pDrawContext) const
{
    bindImpl_(pDrawContext);
}

/**
 * Clears a single render target; does nothing by default.
 */
void FrameBuffer::clearMRT(DrawContext*, u32, const Color4f&) const {}

/**
 * Constructs a frame buffer referring to an area of another frame buffer.
 * @param pOriginal Frame buffer being referenced.
 * @param rVirtualSize Virtual size.
 * @param rPhysicalArea Physical area relative to the original buffer.
 */
ReferenceFrameBuffer::ReferenceFrameBuffer(const FrameBuffer* pOriginal,
                                           const Vector2f& rVirtualSize,
                                           const BoundBox2f& rPhysicalArea)
    : FrameBuffer(rVirtualSize, rPhysicalArea), mOriginalFrameBuffer(pOriginal)
{
    BoundBox2f area = rPhysicalArea;
    area.offset(pOriginal->getPhysicalArea().getMin());
    setPhysicalArea(area);
}

/**
 * Constructs a frame buffer referring to an area of another frame buffer.
 * @param pOriginal Frame buffer being referenced.
 * @param rVirtualSize Virtual size.
 * @param rPhysicalPos Physical position relative to the original buffer.
 * @param rPhysicalSize Physical size.
 */
ReferenceFrameBuffer::ReferenceFrameBuffer(const FrameBuffer* pOriginal,
                                           const Vector2f& rVirtualSize,
                                           const Vector2f& rPhysicalPos,
                                           const Vector2f& rPhysicalSize)
    : FrameBuffer(rVirtualSize, rPhysicalPos.x, rPhysicalPos.y, rPhysicalSize.x, rPhysicalSize.y),
      mOriginalFrameBuffer(pOriginal)
{
    Vector2f max = rPhysicalSize + pOriginal->getPhysicalArea().getMin();
    Vector2f min = pOriginal->getPhysicalArea().getMin() + rPhysicalPos;
    setPhysicalArea(BoundBox2f(min, max + rPhysicalPos));
}

FrameBuffer::~FrameBuffer() = default;

ReferenceFrameBuffer::~ReferenceFrameBuffer() = default;

/**
 * Clears the original frame buffer.
 * @param pDrawContext Draw context.
 * @param clearFlag Buffers to clear.
 * @param rColor Clear color.
 * @param depth Clear depth.
 * @param stencil Clear stencil value.
 */
void ReferenceFrameBuffer::clear(DrawContext* pDrawContext, u32 clearFlag, const Color4f& rColor,
                                 f32 depth, u32 stencil) const
{
    mOriginalFrameBuffer->clear(pDrawContext, clearFlag, rColor, depth, stencil);
}

/**
 * Clears a render target of the original frame buffer.
 * @param pDrawContext Draw context.
 * @param target Render target index.
 * @param rColor Clear color.
 */
void ReferenceFrameBuffer::clearMRT(DrawContext* pDrawContext, u32 target,
                                    const Color4f& rColor) const
{
    mOriginalFrameBuffer->clearMRT(pDrawContext, target, rColor);
}

/**
 * Binds the original frame buffer.
 * @param pDrawContext Draw context.
 */
void ReferenceFrameBuffer::bindImpl_(DrawContext* pDrawContext) const
{
    mOriginalFrameBuffer->bindImpl_(pDrawContext);
}

}  // namespace sead
