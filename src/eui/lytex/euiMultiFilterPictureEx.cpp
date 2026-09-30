#include <eui/euiMultiFilterPictureEx.h>

#include <eui/euiDrawInfoEx.h>
#include <eui/euiFrameBufferMultiFilter.h>
#include <eui/euiUtility.h>
#include <basis/seadNew.h>
#include <nn/ui2d/ui2d_BuildArgSet.h>

namespace eui {

/**
 * @brief Creates a pane without a framebuffer filter.
 * @param[in] textureCount Number of texture slots to allocate.
 */
MultiFilterPictureEx::MultiFilterPictureEx(u8 textureCount)
    : PictureEx(textureCount), m_pFilter(nullptr) {}

/**
 * @brief Builds the pane and initializes its framebuffer filter.
 * @param[in] pResource Base pane resource to construct from.
 * @param[in] pOverride Override pane resource passed to the NintendoWare constructor.
 * @param[in] rArgs Layout construction arguments and resource context.
 */
MultiFilterPictureEx::MultiFilterPictureEx(const nn::ui2d::ResPicture* pResource,
    const nn::ui2d::ResPicture* pOverride, const nn::ui2d::BuildArgSet& rArgs)
    : PictureEx(pResource, pOverride, rArgs), m_pFilter(nullptr) {
    auto* pHeap = GetNwAllocatorHeap();
    m_pFilter = new (pHeap, 8) FrameBufferMultiFilter;
    m_pFilter->initialize(pHeap, *this, reinterpret_cast<LayoutEx*>(rArgs.m_pPartsLayout));
}

/**
 * @brief Copies the pane and creates a filter for the destination layout.
 * @param[in] rOther Source object to copy.
 * @param[in] pLayout Layout that owns the new pane.
 */
MultiFilterPictureEx::MultiFilterPictureEx(const MultiFilterPictureEx& rOther, LayoutEx* pLayout)
    : PictureEx(rOther) {
    auto* pHeap = GetNwAllocatorHeap();
    m_pFilter = new (pHeap, 8) FrameBufferMultiFilter;
    m_pFilter->initialize(pHeap, *this, pLayout);
}

/** @brief Releases the framebuffer filter owned by the pane. */
MultiFilterPictureEx::~MultiFilterPictureEx() {
    if (m_pFilter) {
        delete m_pFilter;
        m_pFilter = nullptr;
    }
}

/**
 * @brief Draws the filtered capture unless capture rendering is already in progress.
 * @param[in,out] rDrawInfo Drawing state used to calculate or render the pane.
 * @param[in,out] rCommands Command buffer that receives the draw commands.
 */
void MultiFilterPictureEx::DrawSelf(nn::ui2d::DrawInfo& rDrawInfo, nn::gfx::CommandBuffer& rCommands) {
    auto& rDrawInfoEx = static_cast<DrawInfoEx&>(rDrawInfo);

    if (rDrawInfoEx._1A8) {
        return;
    }

    const auto* pTexture = m_pFilter->captureAndFilter(*this, rDrawInfoEx);

    if (pTexture) {
        m_pFilter->applyTextureDataToPictureMaterial(this, &m_TextureInfo, pTexture, rDrawInfoEx);
        PictureEx::DrawSelf(rDrawInfo, rCommands);
        m_pFilter->freeResultTexture(pTexture);
    } else {
        PictureEx::DrawSelf(rDrawInfo, rCommands);
    }
}

}  // namespace eui
