#pragma once

#include <eui/euiWindowEx.h>
#include <nn/ui2d/ui2d_TextureInfo.h>

namespace eui {
class LayoutEx;
class FrameBufferMultiFilter;

class MultiFilterWindowEx : public WindowEx {
public:
    MultiFilterWindowEx(u8 frameCount, u8 textureCount);
    MultiFilterWindowEx(const nn::ui2d::ResWindow* pResource,
        const nn::ui2d::ResWindow* pOverride, const nn::ui2d::BuildArgSet& rArgs);
    MultiFilterWindowEx(const MultiFilterWindowEx& rOther, LayoutEx* pLayout);
    ~MultiFilterWindowEx() override;
    NN_RUNTIME_TYPEINFO(WindowEx);
    void DrawSelf(nn::ui2d::DrawInfo& rDrawInfo, nn::gfx::CommandBuffer& rCommands) override;

private:
    FrameBufferMultiFilter* m_pFilter;
    nn::ui2d::PlacementTextureInfo m_TextureInfo;
};
}  // namespace eui
