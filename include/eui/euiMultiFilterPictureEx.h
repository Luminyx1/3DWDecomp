#pragma once

#include <eui/euiPictureEx.h>
#include <nn/ui2d/ui2d_TextureInfo.h>

namespace eui {
class LayoutEx;
class FrameBufferMultiFilter;

class MultiFilterPictureEx : public PictureEx {
public:
    MultiFilterPictureEx(u8 textureCount);
    MultiFilterPictureEx(const nn::ui2d::ResPicture* pResource,
        const nn::ui2d::ResPicture* pOverride, const nn::ui2d::BuildArgSet& rArgs);
    MultiFilterPictureEx(const MultiFilterPictureEx& rOther, LayoutEx* pLayout);
    ~MultiFilterPictureEx() override;
    NN_RUNTIME_TYPEINFO(PictureEx);
    void DrawSelf(nn::ui2d::DrawInfo& rDrawInfo, nn::gfx::CommandBuffer& rCommands) override;

private:
    FrameBufferMultiFilter* m_pFilter;
    nn::ui2d::PlacementTextureInfo m_TextureInfo;
};
}  // namespace eui
