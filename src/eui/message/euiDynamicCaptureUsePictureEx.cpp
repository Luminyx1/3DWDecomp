#include <eui/euiDynamicCaptureUsePictureEx.h>

#include <eui/euiDynamicCapturePane.h>
#include <eui/euiUtility.h>

namespace eui {

/** @brief Creates a pane without a capture source. */
DynamicCaptureUsePictureEx::DynamicCaptureUsePictureEx(u8 textureCount)
    : PictureEx(textureCount), m_pCapture(nullptr), mTextureIndex(0) {}

/** @brief Creates a picture from a texture without a capture source. */
DynamicCaptureUsePictureEx::DynamicCaptureUsePictureEx(const nn::ui2d::TextureInfo& rTexture)
    : PictureEx(rTexture), m_pCapture(nullptr), mTextureIndex(0) {}

/** @brief Builds the pane from layout resources. */
DynamicCaptureUsePictureEx::DynamicCaptureUsePictureEx(const nn::ui2d::ResPicture* pResource,
    const nn::ui2d::ResPicture* pOverride, const nn::ui2d::BuildArgSet& rArgs)
    : PictureEx(pResource, pOverride, rArgs), m_pCapture(nullptr), mTextureIndex(0) {}

/** @brief Copies the pane and its capture-source association. */
DynamicCaptureUsePictureEx::DynamicCaptureUsePictureEx(const DynamicCaptureUsePictureEx& rOther)
    : PictureEx(rOther), m_pCapture(rOther.m_pCapture), mTextureIndex(rOther.mTextureIndex) {}

/** @brief Selects the capture source and destination texture slot. */
void DynamicCaptureUsePictureEx::setupDynamicCapture(DynamicCapturePane* pCapture, u8 textureIndex) {
    m_pCapture = pCapture;
    mTextureIndex = textureIndex;
}

/** @brief Supplies capture dimensions before calculating the pane. */
void DynamicCaptureUsePictureEx::Calculate(nn::ui2d::DrawInfo& rDrawInfo, CalculateContext& rContext, bool force) {
    if (m_pCapture) {
        m_pCapture->applyTextureInfoToMaterialForCalculate(this,
            rContext.pLayoutInformation->size, mTextureIndex);
    }
    PictureEx::Calculate(rDrawInfo, rContext, force);
}

/** @brief Draws the pane when its captured texture is available. */
void DynamicCaptureUsePictureEx::DrawSelf(nn::ui2d::DrawInfo& rDrawInfo, nn::gfx::CommandBuffer& rCommands) {
    if (m_pCapture && m_pCapture->getDynamicTexture()) {
        ApplyTextureInfoToMaterial(this, m_pCapture->getTextureInfo(), mTextureIndex);
        PictureEx::DrawSelf(rDrawInfo, rCommands);
    }
}

}  // namespace eui
