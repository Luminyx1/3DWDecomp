#include <eui/euiDynamicCaptureUsePictureEx.h>

#include <eui/euiDynamicCapturePane.h>
#include <eui/euiUtility.h>

namespace eui {

/**
 * @brief Creates a pane without a capture source.
 * @param[in] textureCount Number of texture slots to allocate.
 */
DynamicCaptureUsePictureEx::DynamicCaptureUsePictureEx(u8 textureCount)
    : PictureEx(textureCount), m_pCapture(nullptr), mTextureIndex(0) {}

/**
 * @brief Creates a picture from a texture without a capture source.
 * @param[in] rTexture Texture used to initialize the picture.
 */
DynamicCaptureUsePictureEx::DynamicCaptureUsePictureEx(const nn::ui2d::TextureInfo& rTexture)
    : PictureEx(rTexture), m_pCapture(nullptr), mTextureIndex(0) {}

/**
 * @brief Builds the pane from layout resources.
 * @param[in] pResource Base pane resource to construct from.
 * @param[in] pOverride Override pane resource passed to the NintendoWare constructor.
 * @param[in] rArgs Layout construction arguments and resource context.
 */
DynamicCaptureUsePictureEx::DynamicCaptureUsePictureEx(const nn::ui2d::ResPicture* pResource,
    const nn::ui2d::ResPicture* pOverride, const nn::ui2d::BuildArgSet& rArgs)
    : PictureEx(pResource, pOverride, rArgs), m_pCapture(nullptr), mTextureIndex(0) {}

/**
 * @brief Copies the pane and its capture-source association.
 * @param[in] rOther Source object to copy.
 */
DynamicCaptureUsePictureEx::DynamicCaptureUsePictureEx(const DynamicCaptureUsePictureEx& rOther)
    : PictureEx(rOther), m_pCapture(rOther.m_pCapture), mTextureIndex(rOther.mTextureIndex) {}

/**
 * @brief Selects the capture source and destination texture slot.
 * @param[in] pCapture Capture pane supplying the texture; null clears the association.
 * @param[in] textureIndex Destination material texture slot for the capture.
 */
void DynamicCaptureUsePictureEx::setupDynamicCapture(DynamicCapturePane* pCapture, u8 textureIndex) {
    m_pCapture = pCapture;
    mTextureIndex = textureIndex;
}

/**
 * @brief Supplies capture dimensions before calculating the pane.
 * @param[in,out] rDrawInfo Drawing state used to calculate or render the pane.
 * @param[in,out] rContext Calculation context containing the layout dimensions.
 * @param[in] force Whether to force pane calculation, forwarded to the base implementation.
 */
void DynamicCaptureUsePictureEx::Calculate(nn::ui2d::DrawInfo& rDrawInfo, CalculateContext& rContext, bool force) {
    if (m_pCapture) {
        m_pCapture->applyTextureInfoToMaterialForCalculate(this,
            rContext.pLayoutInformation->size, mTextureIndex);
    }

    PictureEx::Calculate(rDrawInfo, rContext, force);
}

/**
 * @brief Draws the pane when its captured texture is available.
 * @param[in,out] rDrawInfo Drawing state used to calculate or render the pane.
 * @param[in,out] rCommands Command buffer that receives the draw commands.
 */
void DynamicCaptureUsePictureEx::DrawSelf(nn::ui2d::DrawInfo& rDrawInfo, nn::gfx::CommandBuffer& rCommands) {
    if (m_pCapture && m_pCapture->getDynamicTexture()) {
        ApplyTextureInfoToMaterial(this, m_pCapture->getTextureInfo(), mTextureIndex);
        PictureEx::DrawSelf(rDrawInfo, rCommands);
    }
}

}  // namespace eui
