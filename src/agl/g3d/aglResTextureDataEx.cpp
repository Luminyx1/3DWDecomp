#include "g3d/aglResTextureDataEx.h"

#include <cstring>

#include "common/aglTextureData.h"

namespace agl::g3d {

/**
 * Constructs an empty texture resource bound to its own texture and texture view.
 */
ResTextureDataEx::ResTextureDataEx()
{
    std::memset(&mResTextureData, 0, sizeof(mResTextureData));
    mResTextureData.pTexture.Set(&mTexture);
    mResTextureData.pTextureView.Set(&mTextureView);
    mTextureView.ToData()->userPtr = &mResTextureData;
}

/**
 * Unbinds the texture view from the resource.
 */
ResTextureDataEx::~ResTextureDataEx()
{
    mTextureView.ToData()->userPtr = nullptr;
}

/**
 * Makes a texture resource refer to an agl texture.
 * @param pResTextureDataEx texture resource to fill
 * @param rTextureData source texture
 * @param pName name of the texture (unused)
 * @return the texture resource data
 */
nn::gfx::ResTextureData* TextureUtilG3D::convertToResTexture(ResTextureDataEx* pResTextureDataEx,
                                                             const TextureData& rTextureData,
                                                             const char* pName)
{
    return convertToResTexture(&pResTextureDataEx->mResTextureData, rTextureData, pName);
}

/**
 * Makes texture resource data refer to an agl texture.
 * @param pResTextureData texture resource data to fill
 * @param rTextureData source texture
 * @param pName name of the texture (unused)
 * @return pResTextureData
 */
nn::gfx::ResTextureData* TextureUtilG3D::convertToResTexture(nn::gfx::ResTextureData* pResTextureData,
                                                             const TextureData& rTextureData,
                                                             const char* pName)
{
    rTextureData.getTexture().setReference_();
    pResTextureData->userDescriptorSlot.value = static_cast<u32>(rTextureData.getTextureID());
    return pResTextureData;
}

}  // namespace agl::g3d
