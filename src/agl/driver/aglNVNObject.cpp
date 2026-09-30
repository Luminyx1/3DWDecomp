#include <cstring>
#include <nn/gfx/gfx_Interoperation-api.nvn.8.h>
#include <nvn/nvn_FuncPtrInline.h>

#include "driver/aglNVNMgr.h"
#include "driver/aglNVNimage.h"
#include "driver/aglNVNsampler.h"
#include "driver/aglNVNtexture.h"

namespace agl::driver {

/**
 * Constructs an unregistered texture.
 */
NVNtexture_::NVNtexture_() : mTextureID(-1), _c4(0)
{
    std::memset(&mTexture, 0, sizeof(NVNtexture));
}

/**
 * Releases the texture registration.
 */
NVNtexture_::~NVNtexture_()
{
    releaseTexture();
}

/**
 * Releases the texture registration and finalizes the owned NVN texture when unused.
 */
void NVNtexture_::releaseTexture()
{
    if (mTextureID != -1 && NVNMgr::instance()->releaseTexture(mTextureID) && (_c4 & 1))
    {
        nvnTextureFinalize(&mTexture);
        _c4 &= ~1;
    }

    mTextureID = -1;
}

/**
 * Copies a texture and takes a reference to its registration.
 * @param other texture to copy
 */
NVNtexture_::NVNtexture_(const NVNtexture_& other) : _c4(0)
{
    std::memcpy(&mTexture, &other.mTexture, sizeof(NVNtexture));
    mTextureID = other.mTextureID;
    _c4 = other._c4;

    if (mTextureID != -1 && !NVNMgr::instance()->countupTexture(mTextureID))
    {
        _c4 |= 1;
        mTextureID = NVNMgr::instance()->registerTexture(&mTexture, nullptr, "copy");
    }
}

/**
 * Sets the texture ID.
 * @param newID new texture ID
 */
void NVNtexture_::updateTexId_(s32 newID)
{
    mTextureID = newID;
}

/**
 * Releases this texture, then copies another one and takes a reference to its registration.
 * @param other texture to copy
 * @return this texture
 */
NVNtexture_& NVNtexture_::operator=(const NVNtexture_& other)
{
    releaseTexture();
    std::memcpy(&mTexture, &other.mTexture, sizeof(NVNtexture));
    mTextureID = other.mTextureID;
    _c4 = other._c4;

    if (mTextureID != -1 && !NVNMgr::instance()->countupTexture(mTextureID))
    {
        _c4 |= 1;
        mTextureID = NVNMgr::instance()->registerTexture(&mTexture, nullptr, "copy");
    }

    return *this;
}

/**
 * Registers an NVN texture or texture view with the NVN manager.
 * @param pTexture texture to register, or nullptr to only register a view
 * @param pView texture view
 * @param pName debug name
 * @param isOwner whether this object finalizes the NVN texture
 * @return whether the registration changed
 */
bool NVNtexture_::registerTexture(const NVNtexture* pTexture, const NVNtextureView* pView,
                                  const char* pName, bool isOwner)
{
    if (pTexture)
    {
        if (NVNMgr::isEqual(*pTexture, mTexture))
        {
            return false;
        }

        releaseTexture();

        if (isOwner)
        {
            _c4 |= 1;
        }
        else
        {
            _c4 &= ~1;
        }

        mTextureID = NVNMgr::instance()->registerTexture(pTexture, pView, pName);
        std::memcpy(&mTexture, pTexture, sizeof(NVNtexture));
        return true;
    }

    if (pView)
    {
        NVNMgr::instance()->releaseTexture(mTextureID);
        mTextureID = NVNMgr::instance()->registerTexture(&mTexture, pView, pName);
        return true;
    }

    return false;
}

/**
 * Sets an NVN texture registered elsewhere.
 * @param rTexture NVN texture
 * @param textureId texture ID of the registration
 */
void NVNtexture_::setDirect(const NVNtexture& rTexture, s32 textureId)
{
    releaseTexture();
    std::memcpy(&mTexture, &rTexture, sizeof(NVNtexture));
    _c4 &= ~1;
    mTextureID = textureId;

    if (mTextureID != -1 && !NVNMgr::instance()->countupTexture(mTextureID))
    {
        _c4 |= 1;
        mTextureID = NVNMgr::instance()->registerTexture(&mTexture, nullptr, "copy");
    }
}

/**
 * Wraps the NVN texture into a gfx texture.
 * @param pTexture gfx texture to initialize
 */
void NVNtexture_::initializeGfxTexture(nn::gfx::Texture* pTexture) const
{
    nn::gfx::TInteroperation<nn::gfx::ApiVariationNvn8>::ConvertToGfxTexture(
        pTexture, const_cast<NVNtexture*>(&mTexture));
}

/**
 * Marks the registered texture as referenced.
 */
void NVNtexture_::setReference_() const
{
    if (mTextureID != -1)
    {
        NVNMgr* pMgr = NVNMgr::instance();
        pMgr->mTextures[mTextureID - pMgr->mTextureIdBase].mFlags |= 2;
    }
}

/**
 * Constructs an unregistered sampler.
 */
NVNsampler_::NVNsampler_() : _0(nullptr), _8(-1) {}

/**
 * Releases the sampler registration.
 */
NVNsampler_::~NVNsampler_()
{
    releaseSampler();
}

/**
 * Releases the sampler registration.
 */
void NVNsampler_::releaseSampler()
{
    if (_8 != -1)
    {
        NVNMgr::instance()->releaseSampler(_8);
        _8 = -1;
    }
}

/**
 * Copies a sampler and takes a reference to its registration.
 * @param other sampler to copy
 */
NVNsampler_::NVNsampler_(const NVNsampler_& other) : _0(other._0), _8(other._8)
{
    if (_8 != -1)
    {
        NVNMgr::instance()->countupSampler(_8);
    }
}

/**
 * Releases this sampler, then copies another one and takes a reference to its registration.
 * @param other sampler to copy
 * @return this sampler
 */
NVNsampler_& NVNsampler_::operator=(const NVNsampler_& other)
{
    releaseSampler();
    _0 = other._0;
    _8 = other._8;

    if (_8 != -1)
    {
        NVNMgr::instance()->countupSampler(_8);
    }

    return *this;
}

/**
 * Registers an NVN sampler with the NVN manager.
 * @param rSampler sampler to register
 * @param pName debug name
 * @return whether the sampler ID changed
 */
bool NVNsampler_::registerSampler(const NVNsampler& rSampler, const char* pName)
{
    if (_8 != -1 && NVNMgr::instance()->isEqual(_8, rSampler))
    {
        return false;
    }

    s32 oldId = _8;
    releaseSampler();
    u32 newId = NVNMgr::instance()->registerSampler(&rSampler, pName);
    _8 = newId;
    return newId != oldId;
}

/**
 * Updates the combined texture handle for a texture.
 * @param textureId texture ID
 */
void NVNsampler_::updateTextureId(s32 textureId)
{
    if (_8 != -1)
    {
        _0 = reinterpret_cast<void*>(
            nvnDeviceGetTextureHandle(NVNMgr::instance()->getNvnDevice(), textureId, _8));
    }
    else
    {
        _0 = nullptr;
    }
}

/**
 * Constructs an image without a handle.
 */
NVNimage_::NVNimage_() : mImageId(0) {}

/**
 * Destroys the image.
 */
NVNimage_::~NVNimage_() = default;

/**
 * Copies an image handle.
 * @param other image to copy
 */
NVNimage_::NVNimage_(const NVNimage_& other) : mImageId(other.mImageId) {}

/**
 * Copies an image handle.
 * @param other image to copy
 * @return this image
 */
NVNimage_& NVNimage_::operator=(const NVNimage_& other)
{
    mImageId = other.mImageId;
    return *this;
}

/**
 * Updates the image handle for an image ID.
 * @param id image ID
 */
void NVNimage_::updateImageId(s32 id)
{
    mImageId = nvnDeviceGetImageHandle(NVNMgr::instance()->getNvnDevice(), id);
}

}  // namespace agl::driver
