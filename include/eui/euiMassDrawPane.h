#pragma once

#include <eui/euiPictureEx.h>
#include <common/aglTextureData.h>
#include <container/seadBuffer.h>
#include <math/seadVector.h>

namespace eui {

class MassDrawPane : public PictureEx {
public:
    explicit MassDrawPane(u8 textureCount);
    explicit MassDrawPane(const nn::ui2d::TextureInfo& rTexture);
    MassDrawPane(const nn::ui2d::ResPicture* pResource,
        const nn::ui2d::ResPicture* pOverride, const nn::ui2d::BuildArgSet& rArgs);
    MassDrawPane(const MassDrawPane& rOther);
    ~MassDrawPane() override;
    NN_RUNTIME_TYPEINFO(PictureEx);
    void initialize(sead::Heap* pHeap, int count);
    void Calculate(nn::ui2d::DrawInfo& rDrawInfo, CalculateContext& rContext, bool force) override;
    void DrawSelf(nn::ui2d::DrawInfo& rDrawInfo, nn::gfx::CommandBuffer& rCommands) override;

private:
    void initializeTextureData_();
    agl::TextureData mTexture;
    sead::Buffer<u8> mAlphas;
    sead::Buffer<u8> mIndices;
    sead::Buffer<sead::Vector2f> mPositions;
    bool mTextureInitialized = false;
};
static_assert(sizeof(MassDrawPane) == 0x260, "MassDrawPane size");

}  // namespace eui
