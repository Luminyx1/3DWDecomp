#include <eui/euiDrawInfoEx.h>

#include <gfx/seadFrameBuffer.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadViewport.h>

namespace eui {

/** @brief Releases temporary capture textures and unlinks their panes. */
void DrawInfoEx::freeDynamicTexture() {
    auto it = m_DynamicCapturePanes.begin();
    while (it != m_DynamicCapturePanes.end()) {
        auto current = it++;
        current->freeDynamicTexture();
        m_DynamicCapturePanes.erase(current);
    }
}

/**
 * @brief Applies the framebuffer, viewport, optional scissor, and graphics state.
 * @param[in] pInfo Framebuffer, viewport, scissor, and graphics state to apply.
 */
void DrawInfoEx::applyRenderBufferInfo(const RenderBufferInfo* pInfo) {
    pInfo->pFrameBuffer->bind(pInfo->pDrawContext);
    if (pInfo->pScissor) {
        pInfo->pViewport->applyViewport(pInfo->pDrawContext, *pInfo->pFrameBuffer);
        pInfo->pScissor->applyScissor(pInfo->pDrawContext, *pInfo->pFrameBuffer);
    } else {
        pInfo->pViewport->apply(pInfo->pDrawContext, *pInfo->pFrameBuffer);
    }

    pInfo->pGraphicsContext->apply(pInfo->pDrawContext);
}

static_assert(sizeof(nn::ui2d::DrawInfo) == 0x1a0, "DrawInfo size");
static_assert(sizeof(DrawInfoEx) == 0x1c0, "DrawInfoEx size");

}  // namespace eui
