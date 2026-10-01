#include <nn/ui2d/ui2d_TextureContainer.h>
#include <nn/gfx/gfx_ResTexture.h>
#include <nn/gfx/gfx_Texture.h>

namespace nn::ui2d {

// pDevice owns the GPU texture and view; a missing or uninitialized texture is left alone.
void ResourceTextureInfo::Finalize(nn::gfx::Device* pDevice) {
    auto* resource = m_pResource;

    if (resource == nullptr) return;
    auto* texture = static_cast<nn::gfx::Texture*>(resource->ToData().pTexture.Get());

    if (!texture || !nn::font::IsInitialized(*texture)) return;
    texture->nn::gfx::detail::TextureImpl<nn::gfx::ApiVariationNvn8>::Finalize(pDevice);
    auto* view = static_cast<nn::gfx::TextureView*>(resource->ToData().pTextureView.Get());
    view->nn::gfx::detail::TextureViewImpl<nn::gfx::ApiVariationNvn8>::Finalize(pDevice);
    m_pResource = nullptr;
}

TextureSize ResourceTextureInfo::GetSize() const {
    const auto& info = m_pResource->ToData().textureInfoData;
    return TextureSize(info.width, info.height);
}

int ResourceTextureInfo::GetFormat() const {
    return m_pResource->ToData().textureInfoData.imageFormat;
}

bool ResourceTextureInfo::IsValid() const {
    if (m_pResource == nullptr) return false;
    const auto size = GetSize();
    return size.width != 0 && size.height != 0;
}

const nn::gfx::TextureView* ResourceTextureInfo::GetTextureView() const {
    return static_cast<const nn::gfx::TextureView*>(m_pResource->ToData().pTextureView.Get());
}

nn::gfx::TextureView* ResourceTextureInfo::GetTextureView() {
    return static_cast<nn::gfx::TextureView*>(m_pResource->ToData().pTextureView.Get());
}

}  // namespace nn::ui2d
