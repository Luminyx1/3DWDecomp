#pragma once

#include <nn/ui2d/ui2d_TextureInfo.h>

namespace nn::gfx { class ResTexture; }

namespace nn::ui2d {

class ResourceTextureInfo : public TextureInfo {
public:
    NN_RUNTIME_TYPEINFO(TextureInfo);
    ~ResourceTextureInfo() override = default;
    void Finalize(nn::gfx::Device* pDevice) override;
    TextureSize GetSize() const override;
    int GetFormat() const override;
    bool IsValid() const override;
    const nn::gfx::TextureView* GetTextureView() const override;
    nn::gfx::TextureView* GetTextureView() override;

    nn::gfx::ResTexture* m_pResource;
};

static_assert(sizeof(ResourceTextureInfo) == 0x18, "ResourceTextureInfo size");
}  // namespace nn::ui2d
