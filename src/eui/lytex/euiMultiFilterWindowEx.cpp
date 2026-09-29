#include <eui/euiMultiFilterWindowEx.h>

#include <eui/euiDrawInfoEx.h>
#include <eui/euiFrameBufferMultiFilter.h>
#include <eui/euiUtility.h>
#include <basis/seadNew.h>
#include <nn/ui2d/ui2d_BuildArgSet.h>

namespace eui {

/** @brief Creates a pane without a framebuffer filter. */
MultiFilterWindowEx::MultiFilterWindowEx(u8 frameCount, u8 textureCount)
    : WindowEx(frameCount, textureCount), m_pFilter(nullptr) {}

/** @brief Builds the pane and initializes its framebuffer filter. */
MultiFilterWindowEx::MultiFilterWindowEx(const nn::ui2d::ResWindow* pResource,
    const nn::ui2d::ResWindow* pOverride, const nn::ui2d::BuildArgSet& rArgs)
    : WindowEx(pResource, pOverride, rArgs), m_pFilter(nullptr) {
    auto* pHeap = GetNwAllocatorHeap();
    m_pFilter = new (pHeap, 8) FrameBufferMultiFilter;
    m_pFilter->initialize(pHeap, *this, reinterpret_cast<LayoutEx*>(rArgs.m_pPartsLayout));
}

/** @brief Copies the pane and creates a filter for the destination layout. */
MultiFilterWindowEx::MultiFilterWindowEx(const MultiFilterWindowEx& rOther, LayoutEx* pLayout)
    : WindowEx(rOther), m_pFilter(nullptr) {
    auto* pHeap = GetNwAllocatorHeap();
    m_pFilter = new (pHeap, 8) FrameBufferMultiFilter;
    m_pFilter->initialize(pHeap, *this, pLayout);
}

/** @brief Releases the framebuffer filter owned by the pane. */
MultiFilterWindowEx::~MultiFilterWindowEx() {
    if (m_pFilter) {
        delete m_pFilter;
        m_pFilter = nullptr;
    }
}

/** @brief Draws the filtered capture unless capture rendering is already in progress. */
void MultiFilterWindowEx::DrawSelf(nn::ui2d::DrawInfo& rDrawInfo, nn::gfx::CommandBuffer& rCommands) {
    auto& rDrawInfoEx = static_cast<DrawInfoEx&>(rDrawInfo);
    if (rDrawInfoEx._1A8) {
        return;
    }
    const auto* pTexture = m_pFilter->captureAndFilter(*this, rDrawInfoEx);
    if (pTexture) {
        m_pFilter->applyTextureDataToWindowMaterial(this, &m_TextureInfo, pTexture, rDrawInfoEx);
        WindowEx::DrawSelf(rDrawInfo, rCommands);
        m_pFilter->freeResultTexture(pTexture);
    } else {
        WindowEx::DrawSelf(rDrawInfo, rCommands);
    }
}

}  // namespace eui
