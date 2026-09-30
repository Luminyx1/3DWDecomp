#include <eui/euiScissorPane.h>
#include <eui/euiDrawInfoEx.h>
#include <gfx/seadFrameBuffer.h>
#include <gfx/seadViewport.h>
#include <math/seadBoundBox.hpp>
#include <algorithm>

namespace eui {

// Construct an empty pane with the default layout state.
ScissorPane::ScissorPane() = default;

// pResource contains the serialized pane; rArgs supplies the layout build context.
ScissorPane::ScissorPane(const nn::ui2d::ResPane* pResource,
                         const nn::ui2d::BuildArgSet& rArgs)
    : Pane(pResource, rArgs) {}

// rOther supplies the pane properties to copy, without copying its child tree.
ScissorPane::ScissorPane(const ScissorPane& rOther) : Pane(rOther) {}

ScissorPane::~ScissorPane() = default;

// rDrawInfo supplies the layout and render target; rCommandBuffer receives pane drawing.
// Restrict drawing to the transformed pane rectangle, then restore the caller's scissor.
void ScissorPane::Draw(nn::ui2d::DrawInfo& rDrawInfo,
                       nn::gfx::CommandBuffer& rCommandBuffer) {
    auto& drawInfo = static_cast<DrawInfoEx&>(rDrawInfo);
    if (!(mFlags & 1) || !drawInfo.m_pRenderBufferInfo || !mAlphaInfluence) {
        Pane::Draw(rDrawInfo, rCommandBuffer);
        return;
    }
    const auto* original = drawInfo.m_pRenderBufferInfo;
    auto info = *original;
    const auto* frameBuffer = info.pFrameBuffer;
    sead::Viewport scissor(*frameBuffer);
    info.pScissor = &scissor;
    const auto& targetSize = original->pFrameBuffer->getVirtualSize();
    const auto& layoutSize = rDrawInfo.m_pLayoutInformation->size;
    const float scaleX = targetSize.x / layoutSize.width;
    const float scaleY = targetSize.y / layoutSize.height;
    const float width = mSizeX * mGlobalMtx[0];
    const float halfX = scaleX * ((width > 0 ? width : -width) * 0.5f);
    const float height = mSizeY * mGlobalMtx[5];
    const float halfY = scaleY * ((height > 0 ? height : -height) * 0.5f);
    float centerX = scaleX * mGlobalMtx[3];
    float centerY = scaleY * mGlobalMtx[7];
    if ((mOriginFlags & 3) == 1) centerX += halfX;
    else if ((mOriginFlags & 3) == 2) centerX -= halfX;
    if (((mOriginFlags >> 2) & 3) == 1) centerY -= halfY;
    else if (((mOriginFlags >> 2) & 3) == 2) centerY += halfY;
    centerX = targetSize.x * 0.5f + centerX;
    centerY = targetSize.y * 0.5f + centerY;
    float minX = centerX - halfX, minY = centerY - halfY;
    float maxX = halfX + centerX, maxY = halfY + centerY;
    minX = std::max(minX, 0.0f);
    minY = std::max(minY, 0.0f);
    if (maxX > targetSize.x) maxX = targetSize.x;
    if (maxY > targetSize.y) maxY = targetSize.y;
    if (maxX - minX >= targetSize.x) { minX = 0; maxX = targetSize.x; }
    if (maxY - minY >= targetSize.y) { minY = 0; maxY = targetSize.y; }
    scissor.setMin({minX, minY});
    scissor.setMax({maxX, maxY});
    if (!(maxX < minX || maxY < minY)) {
        scissor.applyScissor(info.pDrawContext, *frameBuffer);
        drawInfo.m_pRenderBufferInfo = &info;
        Pane::Draw(rDrawInfo, rCommandBuffer);
        drawInfo.m_pRenderBufferInfo = original;
        original->pViewport->applyScissor(original->pDrawContext, *original->pFrameBuffer);
    }
}

}  // namespace eui
