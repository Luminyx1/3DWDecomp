#pragma once

#include <nn/gfx/gfx_ResTextureData.h>
#include <nn/gfx/gfx_Texture.h>

namespace agl {
class TextureData;
}  // namespace agl

namespace agl::g3d {

class ResTextureDataEx {
public:
    ResTextureDataEx();
    virtual ~ResTextureDataEx();

    nn::gfx::ResTextureData* getResTextureData() { return &mResTextureData; }

private:
    friend class TextureUtilG3D;

    nn::gfx::ResTextureData mResTextureData;
    nn::gfx::detail::TextureImpl<nn::gfx::ApiVariationNvn8> mTexture;
    nn::gfx::detail::TextureViewImpl<nn::gfx::ApiVariationNvn8> mTextureView;
};

static_assert(sizeof(ResTextureDataEx) == 0x1c8);

class TextureUtilG3D {
public:
    static nn::gfx::ResTextureData* convertToResTexture(ResTextureDataEx* pResTextureDataEx,
                                                        const TextureData& rTextureData,
                                                        const char* pName);
    static nn::gfx::ResTextureData* convertToResTexture(nn::gfx::ResTextureData* pResTextureData,
                                                        const TextureData& rTextureData,
                                                        const char* pName);
};

}  // namespace agl::g3d
