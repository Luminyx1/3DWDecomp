#pragma once

#include <eui/euiWindowEx.h>

namespace eui {
class DynamicCapturePane;

class DynamicCaptureUseWindowEx : public WindowEx {
public:
    DynamicCaptureUseWindowEx(u8 frameCount, u8 textureCount);
    DynamicCaptureUseWindowEx(const nn::ui2d::ResWindow* pResource,
        const nn::ui2d::ResWindow* pOverride, const nn::ui2d::BuildArgSet& rArgs);
    DynamicCaptureUseWindowEx(const DynamicCaptureUseWindowEx& rOther);
    /** @brief Destroys the pane without owning its capture source. */
    ~DynamicCaptureUseWindowEx() override = default;
    NN_RUNTIME_TYPEINFO(WindowEx);
    void setupDynamicCapture(DynamicCapturePane* pCapture, u8 textureIndex);
    void Calculate(nn::ui2d::DrawInfo& rDrawInfo, CalculateContext& rContext, bool force) override;
    void DrawSelf(nn::ui2d::DrawInfo& rDrawInfo, nn::gfx::CommandBuffer& rCommands) override;

private:
    DynamicCapturePane* m_pCapture;
    u8 mTextureIndex;
};
}  // namespace eui
