#include <eui/euiPictureEx.h>

#include <gfx/nin/seadGraphicsNvn.h>
#include <nn/ui2d/ui2d_Material.h>

// Original local build-result storage; its data section has not been split yet.
extern nn::ui2d::BuildResultInformation lbl_7102122AB0 __attribute__((visibility("hidden")));

namespace eui {

/** @brief Creates a picture and initializes its texture state and vertex colors. */
PictureEx::PictureEx(u8 textureCount) : Picture(textureCount) {
    if (textureCount == 0) {
        GetMaterial()->SetTextureNum(0);
    } else if (textureCount == 1) {
        setupForSingleTexture_();
    }
    initializeVertexColor_();
}

/** @brief Configures one clamped texture with linear filtering. */
void PictureEx::setupForSingleTexture_() {
    GetMaterial()->SetTextureNum(1);
    GetMaterial()->GetFirstTexMap()->SetWrapMode(nn::ui2d::TexWrap_Clamp, nn::ui2d::TexWrap_Clamp);
    GetMaterial()->GetFirstTexMap()->SetFilter(nn::ui2d::TexFilter_Linear, nn::ui2d::TexFilter_Linear);
}

/** @brief Initializes all four vertices to opaque white. */
void PictureEx::initializeVertexColor_() {
    const nn::util::Unorm8x4 color = {{255, 255, 255, 255}};
    for (int i = 0; i < 4; ++i) {
        SetVertexColor(i, color);
    }
}

/** @brief Creates a picture from an existing texture. */
PictureEx::PictureEx(const nn::ui2d::TextureInfo& rTexture) : Picture(rTexture) {
    setupForSingleTexture_();
    initializeVertexColor_();
}

/** @brief Builds a picture from layout resources using the active graphics device. */
PictureEx::PictureEx(const nn::ui2d::ResPicture* pResource,
                     const nn::ui2d::ResPicture* pOverride, const nn::ui2d::BuildArgSet& rArgs)
    : Picture(&lbl_7102122AB0,
              reinterpret_cast<nn::gfx::Device*>(sead::GraphicsNvn::instance()->getGfxDevice()),
              pResource, pOverride, rArgs) {}

/** @brief Copies a picture and its graphics resources. */
PictureEx::PictureEx(const PictureEx& rOther)
    : Picture(rOther, reinterpret_cast<nn::gfx::Device*>(sead::GraphicsNvn::instance()->getGfxDevice())) {}

static_assert(sizeof(PictureEx) == 0x100, "PictureEx size");

}  // namespace eui
