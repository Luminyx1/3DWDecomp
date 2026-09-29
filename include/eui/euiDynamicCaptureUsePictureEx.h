#pragma once

#include <eui/euiPictureEx.h>

namespace eui {
class DynamicCapturePane;

class DynamicCaptureUsePictureEx : public PictureEx {
public:
    DynamicCaptureUsePictureEx(u8 textureCount);
    explicit DynamicCaptureUsePictureEx(const nn::ui2d::TextureInfo& rTexture);
    DynamicCaptureUsePictureEx(const nn::ui2d::ResPicture* pResource,
        const nn::ui2d::ResPicture* pOverride, const nn::ui2d::BuildArgSet& rArgs);
    DynamicCaptureUsePictureEx(const DynamicCaptureUsePictureEx& rOther);
    /** @brief Destroys the pane without owning its capture source. */
    ~DynamicCaptureUsePictureEx() override = default;
    NN_RUNTIME_TYPEINFO(PictureEx);
    void setupDynamicCapture(DynamicCapturePane* pCapture, u8 textureIndex);
    void Calculate(nn::ui2d::DrawInfo& rDrawInfo, CalculateContext& rContext, bool force) override;
    void DrawSelf(nn::ui2d::DrawInfo& rDrawInfo, nn::gfx::CommandBuffer& rCommands) override;

private:
    DynamicCapturePane* m_pCapture;
    u8 mTextureIndex;
};
}  // namespace eui
