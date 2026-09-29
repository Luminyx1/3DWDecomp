#include <gfx/seadFrameBuffer.h>

namespace sead
{
LogicalFrameBuffer::~LogicalFrameBuffer() = default;

FrameBuffer::~FrameBuffer() = default;

void FrameBuffer::clearMRT(DrawContext*, u32, const Color4f&) const {}

void FrameBuffer::bind(DrawContext* pDrawContext) const
{
    bindImpl_(pDrawContext);
}
}  // namespace sead
