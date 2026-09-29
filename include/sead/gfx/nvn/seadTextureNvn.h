#pragma once

#include <gfx/seadTexture.h>
#include <nvn/nvn.h>

namespace sead
{
class TextureNvn : public Texture
{
    SEAD_RTTI_OVERRIDE(TextureNvn, Texture)

public:
    const NVNtexture* getNvnTexture() const { return mNvnTexture; }
    NVNtextureHandle getTextureHandle() const { return mTextureHandle; }

private:
    const NVNtexture* mNvnTexture;
    NVNtextureHandle mTextureHandle;
};

}  // namespace sead
