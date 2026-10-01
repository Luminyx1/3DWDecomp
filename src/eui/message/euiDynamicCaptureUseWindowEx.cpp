#include <eui/euiDynamicCaptureUseWindowEx.h>

#include <eui/euiDynamicCapturePane.h>
#include <eui/euiUtility.h>

namespace eui {

/**
 * @brief Creates a pane without a capture source.
 * @param[in] frameCount Number of window frames to allocate.
 * @param[in] textureCount Number of texture slots to allocate.
 */
DynamicCaptureUseWindowEx::DynamicCaptureUseWindowEx(u8 frameCount, u8 textureCount)
    : WindowEx(frameCount, textureCount), m_pCapture(nullptr), mTextureIndex(0) {}

/**
 * @brief Builds the pane from layout resources.
 * @param[in] pResource Base pane resource to construct from.
 * @param[in] pOverride Override pane resource passed to the NintendoWare constructor.
 * @param[in] rArgs Layout construction arguments and resource context.
 */
DynamicCaptureUseWindowEx::DynamicCaptureUseWindowEx(const nn::ui2d::ResWindow* pResource,
    const nn::ui2d::ResWindow* pOverride, const nn::ui2d::BuildArgSet& rArgs)
    : WindowEx(pResource, pOverride, rArgs), m_pCapture(nullptr), mTextureIndex(0) {}

/**
 * @brief Copies the pane and its capture-source association.
 * @param[in] rOther Source object to copy.
 */
DynamicCaptureUseWindowEx::DynamicCaptureUseWindowEx(const DynamicCaptureUseWindowEx& rOther)
    : WindowEx(rOther), m_pCapture(rOther.m_pCapture), mTextureIndex(rOther.mTextureIndex) {}

/**
 * @brief Selects the capture source and destination texture slot.
 * @param[in] pCapture Capture pane supplying the texture; null clears the association.
 * @param[in] textureIndex Destination material texture slot for the capture.
 */
void DynamicCaptureUseWindowEx::setupDynamicCapture(DynamicCapturePane* pCapture, u8 textureIndex) {
    m_pCapture = pCapture;
    mTextureIndex = textureIndex;
}

/**
 * @brief Supplies capture dimensions before calculating the pane.
 * @param[in,out] rDrawInfo Drawing state used to calculate or render the pane.
 * @param[in,out] rContext Calculation context containing the layout dimensions.
 * @param[in] force Whether to force pane calculation, forwarded to the base implementation.
 */
void DynamicCaptureUseWindowEx::Calculate(nn::ui2d::DrawInfo& rDrawInfo, CalculateContext& rContext, bool force) {
    if (m_pCapture != nullptr) {
        m_pCapture->applyTextureInfoToMaterialForCalculate(this,
            rContext.pLayoutInformation->size, mTextureIndex);
    }

    WindowEx::Calculate(rDrawInfo, rContext, force);
}

/**
 * @brief Draws the pane when its captured texture is available.
 * @param[in,out] rDrawInfo Drawing state used to calculate or render the pane.
 * @param[in,out] rCommands Command buffer that receives the draw commands.
 */
void DynamicCaptureUseWindowEx::DrawSelf(nn::ui2d::DrawInfo& rDrawInfo, nn::gfx::CommandBuffer& rCommands) {
    if (m_pCapture != nullptr && m_pCapture->getDynamicTexture() != nullptr) {
        ApplyTextureInfoToMaterial(this, m_pCapture->getTextureInfo(), mTextureIndex);
        WindowEx::DrawSelf(rDrawInfo, rCommands);
    }
}

}  // namespace eui
