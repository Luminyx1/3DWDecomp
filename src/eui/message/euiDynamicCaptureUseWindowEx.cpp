#include <eui/euiDynamicCaptureUseWindowEx.h>

#include <eui/euiDynamicCapturePane.h>
#include <eui/euiUtility.h>

namespace eui {

/** @brief Creates a pane without a capture source. */
DynamicCaptureUseWindowEx::DynamicCaptureUseWindowEx(u8 frameCount, u8 textureCount)
    : WindowEx(frameCount, textureCount), m_pCapture(nullptr), mTextureIndex(0) {}

/** @brief Builds the pane from layout resources. */
DynamicCaptureUseWindowEx::DynamicCaptureUseWindowEx(const nn::ui2d::ResWindow* pResource,
    const nn::ui2d::ResWindow* pOverride, const nn::ui2d::BuildArgSet& rArgs)
    : WindowEx(pResource, pOverride, rArgs), m_pCapture(nullptr), mTextureIndex(0) {}

/** @brief Copies the pane and its capture-source association. */
DynamicCaptureUseWindowEx::DynamicCaptureUseWindowEx(const DynamicCaptureUseWindowEx& rOther)
    : WindowEx(rOther), m_pCapture(rOther.m_pCapture), mTextureIndex(rOther.mTextureIndex) {}

/** @brief Selects the capture source and destination texture slot. */
void DynamicCaptureUseWindowEx::setupDynamicCapture(DynamicCapturePane* pCapture, u8 textureIndex) {
    m_pCapture = pCapture;
    mTextureIndex = textureIndex;
}

/** @brief Supplies capture dimensions before calculating the pane. */
void DynamicCaptureUseWindowEx::Calculate(nn::ui2d::DrawInfo& rDrawInfo, CalculateContext& rContext, bool force) {
    if (m_pCapture) {
        m_pCapture->applyTextureInfoToMaterialForCalculate(this,
            rContext.pLayoutInformation->size, mTextureIndex);
    }
    WindowEx::Calculate(rDrawInfo, rContext, force);
}

/** @brief Draws the pane when its captured texture is available. */
void DynamicCaptureUseWindowEx::DrawSelf(nn::ui2d::DrawInfo& rDrawInfo, nn::gfx::CommandBuffer& rCommands) {
    if (m_pCapture && m_pCapture->getDynamicTexture()) {
        ApplyTextureInfoToMaterial(this, m_pCapture->getTextureInfo(), mTextureIndex);
        WindowEx::DrawSelf(rDrawInfo, rCommands);
    }
}

}  // namespace eui
