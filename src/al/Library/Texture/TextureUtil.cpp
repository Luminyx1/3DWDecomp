#include "Library/Texture/TextureUtil.hpp"

#include <nn/g3d/g3d_ResFile.h>

#include <common/aglGPUMemBlock.h>
#include <common/aglShaderLocation.h>
#include <common/aglTextureData.h>
#include <common/aglTextureSampler.h>
#include <g3d/aglNW4FToNN.h>
#include <g3d/aglResTextureDataEx.h>
#include <g3d/aglTextureDataInitializerG3D.h>
#include <math/seadMathCalcCommon.h>
#include <nn/gfx/gfx_ResTextureData.h>
#include <nn/gfx/gfx_Texture.h>

#include "Library/Math/MathUtil.hpp"
#include "Library/Memory/Util.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Project/Base/StringUtil.hpp"

namespace {
using TextureImpl = nn::gfx::detail::TextureImpl<nn::gfx::ApiVariationNvn8>;
using TextureViewImpl = nn::gfx::detail::TextureViewImpl<nn::gfx::ApiVariationNvn8>;

/**
 * @brief Allocates the image memory of a texture from the current heap.
 */
void allocTextureImage(agl::TextureData* pTextureData, agl::MemoryAttribute attribute) {
    u64 size = pTextureData->getImageByteSize();
    sead::Heap* heap = al::getCurrentHeap();
    s32 alignment = pTextureData->getAlignment();
    agl::GPUMemBlock<u8>* block = new (heap, 8) agl::GPUMemBlock<u8>;
    block->allocBuffer_(size, heap, alignment, attribute);
    pTextureData->setImagePtr(agl::GPUMemAddrBase(*block, 0), 0);
}

/**
 * @brief Gets the CPU pointer of the image of a linear texture.
 */
u8* getImagePtr(const agl::TextureData* pTextureData) {
    agl::GPUMemVoidAddr image = pTextureData->getImagePtr();
    return static_cast<u8*>(nvnMemoryPoolMap(image.getMemoryPool()->getDriverPool())) +
           image.getByteOffset();
}

/**
 * @brief Converts a half float to a float.
 */
f32 convertF16ToF32(u16 value) {
    u32 sign = (value << 16) & 0x80000000;
    u32 exponent = (value >> 10) & 0x1f;
    u32 mantissa = value & 0x3ff;
    if (exponent != 0) {
        exponent += 112;
    } else if (mantissa != 0) {
        exponent = 113;
        do {
            mantissa <<= 1;
            exponent--;
        } while ((mantissa & 0x400) == 0);
        mantissa &= 0x3ff;
    } else {
        exponent = 0;
    }

    u32 bits = sign | (mantissa << 13) | ((exponent & 0xff) << 23);
    return *reinterpret_cast<f32*>(&bits);
}
}  // namespace

namespace al {

/**
 * @brief Wraps an agl texture as a resource texture.
 */
TextureReplacer::TextureReplacer(const agl::TextureData* pTextureData)
    : mTextureData(pTextureData) {
    mResTextureData = new nn::gfx::ResTextureData();
    TextureViewImpl* textureView = new TextureViewImpl();
    mResTextureData->pTextureView.Set(textureView);
    TextureImpl* texture = new TextureImpl();
    mResTextureData->pTexture.Set(texture);
    mResTexture = agl::g3d::TextureUtilG3D::convertToResTexture(mResTextureData, *pTextureData,
                                                                nullptr);
    const agl::TextureData* textureData = mTextureData;
    textureData->getTexture().setReference_();
    u32 textureId = textureData->getTextureID();
    mResTexture->userDescriptorSlot.value = textureId;
    TextureImpl* resTexture = static_cast<TextureImpl*>(mResTexture->pTexture.Get());
    resTexture->ToData()->pNvnTexture =
        const_cast<NVNtexture*>(mTextureData->getTexture().getTexture());
    TextureViewImpl* view = static_cast<TextureViewImpl*>(mResTexture->pTextureView.Get());
    view->ToData()->pNvnTexture = const_cast<NVNtexture*>(mTextureData->getTexture().getTexture());
    view->ToData()->userPtr = mResTexture;
    mTextureRef = new TextureRefData(mResTexture->pTextureView.Get(), textureId);
}

/**
 * @brief Creates a replacer without a texture; call setup() before use.
 */
TextureReplacer::TextureReplacer() {
    mResTextureData = new nn::gfx::ResTextureData();
    TextureViewImpl* textureView = new TextureViewImpl();
    mResTextureData->pTextureView.Set(textureView);
    TextureImpl* texture = new TextureImpl();
    mResTextureData->pTexture.Set(texture);
    mTextureRef = new TextureRefData();
}

/**
 * @brief Wraps another agl texture.
 */
void TextureReplacer::setup(const agl::TextureData* pTextureData) {
    mTextureData = pTextureData;
    mResTexture = agl::g3d::TextureUtilG3D::convertToResTexture(mResTextureData, *mTextureData,
                                                                nullptr);
    const agl::TextureData* textureData = mTextureData;
    textureData->getTexture().setReference_();
    u32 textureId = textureData->getTextureID();
    mResTexture->userDescriptorSlot.value = textureId;
    TextureImpl* resTexture = static_cast<TextureImpl*>(mResTexture->pTexture.Get());
    resTexture->ToData()->pNvnTexture =
        const_cast<NVNtexture*>(mTextureData->getTexture().getTexture());
    TextureViewImpl* view = static_cast<TextureViewImpl*>(mResTexture->pTextureView.Get());
    view->ToData()->pNvnTexture = const_cast<NVNtexture*>(mTextureData->getTexture().getTexture());
    view->ToData()->userPtr = mResTexture;
    mTextureRef->mTextureView = mResTexture->pTextureView.Get();
    mTextureRef->mDescriptorSlot = textureId;
}

/**
 * @brief Does nothing in this game.
 */
void TextureReplacer::replace(LiveActor* pActor, const char* pMaterialName,
                              const char* pTextureName) {}

/**
 * @brief Refreshes the resource texture after the agl texture changed.
 */
void TextureReplacer::update() {
    mResTexture = agl::g3d::TextureUtilG3D::convertToResTexture(mResTextureData, *mTextureData,
                                                                nullptr);
    const agl::TextureData* textureData = mTextureData;
    textureData->getTexture().setReference_();
    u32 textureId = textureData->getTextureID();
    mResTexture->userDescriptorSlot.value = textureId;
    TextureImpl* resTexture = static_cast<TextureImpl*>(mResTexture->pTexture.Get());
    resTexture->ToData()->pNvnTexture =
        const_cast<NVNtexture*>(mTextureData->getTexture().getTexture());
    TextureViewImpl* view = static_cast<TextureViewImpl*>(mResTexture->pTextureView.Get());
    view->ToData()->pNvnTexture = const_cast<NVNtexture*>(mTextureData->getTexture().getTexture());
    view->ToData()->userPtr = mResTexture;
}

/**
 * @brief Gets the texture reference to bind to materials.
 */
const TextureRefData* TextureReplacer::getTextureRef() const {
    return mTextureRef;
}

/**
 * @brief Creates an empty texture unit.
 */
TextureUnit::TextureUnit(const char* pName) : mName(pName) {}

/**
 * @brief Destroys the texture, its sampler and its image memory.
 */
void TextureUnit::finalize() {
    if (mImage.isValid()) {
        mImage.deleteGPUMemBlock();
        mImage.invalidate();
    }

    if (mTexture != nullptr) {
        delete mTexture;
        mTexture = nullptr;
    }

    if (mSampler != nullptr) {
        delete mSampler;
        mSampler = nullptr;
    }
}

/**
 * @brief Gets the texture width.
 */
u32 TextureUnit::getWidth() const {
    return mTexture->getWidth(0);
}

/**
 * @brief Gets the texture height.
 */
u32 TextureUnit::getHeight() const {
    return mTexture->getHeight(0);
}

/**
 * @brief Gets the texture depth.
 */
u32 TextureUnit::getDepth() const {
    return mTexture->getMipSlice(0);
}

/**
 * @brief Checks whether the texture was created as 1D.
 */
bool TextureUnit::is1D() const {
    return mInitArg.mWidth > 1 && mInitArg.mHeight <= 1 && mInitArg.mDepth <= 1;
}

/**
 * @brief Checks whether the texture was created as 2D.
 */
bool TextureUnit::is2D() const {
    return mInitArg.mWidth > 1 && mInitArg.mHeight > 1 && mInitArg.mDepth <= 1;
}

/**
 * @brief Checks whether the texture was created as 3D.
 */
bool TextureUnit::is3D() const {
    return mInitArg.mWidth > 1 && mInitArg.mHeight > 1 && mInitArg.mDepth > 1;
}

/**
 * @brief Checks whether the texture was created as a cube map.
 */
bool TextureUnit::isCubemap() const {
    return mInitArg.mIsCubemap;
}

/**
 * @brief Makes the CPU see what the GPU wrote.
 */
void TextureUnit::invalidateGpuCacheRead(agl::DrawContext* pDrawContext) {
    mTexture->invalidateCPUCache();
}

/**
 * @brief Does nothing on this platform.
 */
void TextureUnit::invalidateGpuCacheWrite(agl::DrawContext* pDrawContext) {}

/**
 * @brief Creates the texture, its image memory and its sampler (once).
 */
bool TextureUnit::tryCreateTexture(const TextureInitArg& rArg) {
    if (mSampler != nullptr) {
        return false;
    }

    mInitArg = rArg;
    mSampler = new agl::TextureSampler();
    mTexture = new agl::TextureData();
    if (rArg.mIsCubemap) {
        mTexture->initialize_(agl::TextureType(8), rArg.mFormat, rArg.mWidth, rArg.mHeight, 6,
                              rArg.mMipLevelNum, agl::TextureAttribute(0),
                              agl::MultiSampleType(0), true);
    } else if (rArg.mWidth > 1 && rArg.mHeight <= 1 && rArg.mDepth <= 1) {
        mTexture->initialize_(agl::TextureType::cTextureType_1D, rArg.mFormat, rArg.mWidth, 1, 1,
                              rArg.mMipLevelNum, agl::TextureAttribute(0), agl::MultiSampleType(0),
                              true);
    } else if (rArg.mWidth > 1 && rArg.mHeight > 1 && rArg.mDepth <= 1) {
        mTexture->initialize_(agl::TextureType::cTextureType_2D, rArg.mFormat, rArg.mWidth,
                              rArg.mHeight, 1, rArg.mMipLevelNum, agl::TextureAttribute(0),
                              agl::MultiSampleType(0), true);
    } else if (rArg.mWidth > 1 && rArg.mHeight > 1 && rArg.mDepth > 1) {
        mTexture->initialize_(agl::TextureType::cTextureType_3D, rArg.mFormat, rArg.mWidth,
                              rArg.mHeight, rArg.mDepth, rArg.mMipLevelNum,
                              agl::TextureAttribute(0), agl::MultiSampleType(0), true);
    }

    u64 size = mTexture->getImageByteSize();
    sead::Heap* heap = getCurrentHeap();
    s32 alignment = mTexture->getAlignment();
    agl::GPUMemBlock<u8>* block = new (heap, 8) agl::GPUMemBlock<u8>;
    block->allocBuffer_(size, heap, alignment, agl::MemoryAttribute(0));
    mImage = agl::GPUMemVoidAddr(*block, 0);
    mTexture->setImagePtr(mImage, 0);
    mSampler->applyTextureData(*mTexture);
    mSampler->setWrap(rArg.mWrapX, rArg.mWrapY, rArg.mWrapZ);
    mSampler->setFilter(rArg.mMagFilter, rArg.mMinFilter, rArg.mMipFilter);
    applyCompSel(mInitArg.mCompSel);
    return true;
}

/**
 * @brief Sets the component selection of the sampler from a preset.
 */
void TextureUnit::applyCompSel(const CompSelType& rType) {
    switch (rType) {
    case 0:
        mSampler->setUseTextureView(false);
        break;
    case 1:
        mSampler->setCompSel(agl::cTextureCompSel_R, agl::cTextureCompSel_R, agl::cTextureCompSel_R,
                             agl::cTextureCompSel_R);
        break;
    case 2:
        mSampler->setCompSel(agl::cTextureCompSel_G, agl::cTextureCompSel_G, agl::cTextureCompSel_G,
                             agl::cTextureCompSel_G);
        break;
    case 3:
        mSampler->setCompSel(agl::cTextureCompSel_B, agl::cTextureCompSel_B, agl::cTextureCompSel_B,
                             agl::cTextureCompSel_B);
        break;
    case 4:
        mSampler->setCompSel(agl::cTextureCompSel_A, agl::cTextureCompSel_A, agl::cTextureCompSel_A,
                             agl::cTextureCompSel_A);
        break;
    }
}

/**
 * @brief Not supported in this game.
 */
agl::TextureData* createTexture(s32 width, s32 height, u8** ppImage) {
    return nullptr;
}

/**
 * @brief Projects a position onto the view plane and maps it to texture coordinates.
 */
void calcOrthoProjectedTexCoord(sead::Vector2f* pTexCoord, const sead::Matrix34f& rViewMtx,
                                const sead::Vector3f& rOrigin, const sead::Vector3f& rPos,
                                f32 width, f32 height) {
    sead::Vector3f diff = rPos - rOrigin;
    f32 u = diff.dot(sead::Vector3f(rViewMtx(0, 0), rViewMtx(1, 0), rViewMtx(2, 0)));
    f32 v = diff.dot(sead::Vector3f(rViewMtx(0, 2), rViewMtx(1, 2), rViewMtx(2, 2)));
    pTexCoord->x = (u / (width * 0.5f)) * 0.5f + 0.5f;
    pTexCoord->y = (v / (height * 0.5f)) * 0.5f + 0.5f;
}

/**
 * @brief Checks whether a texel position lies inside a texture.
 */
bool isInsideTexture(const sead::Vector2i& rPos, const agl::TextureData* pTextureData) {
    if (rPos.x < 0 || (u32)rPos.x >= pTextureData->getWidth(0)) {
        return false;
    }

    if (rPos.y < 0) {
        return false;
    }

    return (u32)rPos.y < pTextureData->getHeight(0);
}

/**
 * @brief Creates a 2D texture with its image memory.
 */
agl::TextureData* createAglTextureData(agl::TextureFormat format, s32 width, s32 height,
                                       s32 mipLevelNum, agl::TextureAttribute attribute) {
    agl::TextureData* textureData = new agl::TextureData();
    initAglTextureData(textureData, format, width, height, mipLevelNum, attribute);
    return textureData;
}

/**
 * @brief Sets up a 2D texture and allocates its image memory.
 */
void initAglTextureData(agl::TextureData* pTextureData, agl::TextureFormat format, s32 width,
                        s32 height, s32 mipLevelNum, agl::TextureAttribute attribute) {
    pTextureData->initialize_(agl::TextureType::cTextureType_2D, format, width, height, 1,
                              mipLevelNum, attribute, agl::MultiSampleType(0), true);
    allocTextureImage(pTextureData, agl::MemoryAttribute(4));
}

/**
 * @brief Creates a linear 2D texture with its image memory.
 */
agl::TextureData* createAglTextureDataLinear(agl::TextureFormat format, s32 width, s32 height,
                                             s32 mipLevelNum) {
    agl::TextureData* textureData = new agl::TextureData();
    initAglTextureDataLinear(textureData, format, width, height, mipLevelNum);
    return textureData;
}

/**
 * @brief Sets up a linear 2D texture and allocates its image memory.
 */
void initAglTextureDataLinear(agl::TextureData* pTextureData, agl::TextureFormat format,
                              s32 width, s32 height, s32 mipLevelNum) {
    pTextureData->initialize_(agl::TextureType::cTextureType_2D, format, width, height, 1,
                              mipLevelNum, agl::TextureAttribute(1), agl::MultiSampleType(0),
                              true);
    allocTextureImage(pTextureData, agl::MemoryAttribute(8));
}

/**
 * @brief Destroys a texture and its image memory.
 */
void destroyAglTextureAndImage(agl::TextureData** ppTextureData) {
    if (ppTextureData == nullptr || *ppTextureData == nullptr) {
        return;
    }

    if ((*ppTextureData)->getImagePtr().isValid()) {
        (*ppTextureData)->getImagePtr().deleteGPUMemBlock();
        (*ppTextureData)->getImagePtr().invalidate();
    }

    if (*ppTextureData != nullptr) {
        delete *ppTextureData;
        *ppTextureData = nullptr;
    }

    *ppTextureData = nullptr;
}

/**
 * @brief Loads a texture of a bfres of an archive.
 */
void makeTextureDataFromArchive(agl::TextureData* pTextureData, const char* pArchiveName,
                                const char* pFileName, const char* pTextureName) {
    Resource* resource = findOrCreateResource(pArchiveName, nullptr);
    void* file = resource->getOtherFile(StringTmp<256>("%s.bfres", pFileName).cstr(), nullptr);
    if (file != nullptr) {
        agl::g3d::TextureDataInitializerG3D::initialize(
            pTextureData,
            *agl::g3d::ResFile::GetTexture(nn::g3d::ResFile::ResCast(file), pTextureName));
    }
}

/**
 * @brief Loads a texture of a bfres of an archive if both exist.
 */
bool tryMakeTextureDataFromArchive(agl::TextureData* pTextureData, const char* pArchiveName,
                                   const char* pFileName, const char* pTextureName) {
    Resource* resource = findOrCreateResource(pArchiveName, nullptr);
    void* file = resource->getOtherFile(StringTmp<256>("%s.bfres", pFileName).cstr(), nullptr);
    if (file == nullptr) {
        return false;
    }

    nn::gfx::ResTexture* texture =
        agl::g3d::ResFile::GetTexture(nn::g3d::ResFile::ResCast(file), pTextureName);
    if (texture == nullptr) {
        return false;
    }

    agl::g3d::TextureDataInitializerG3D::initialize(pTextureData, *texture);
    return true;
}

/**
 * @brief Not supported for tiled textures in this game.
 */
sead::Color4u8 getColor(const agl::TextureData* pTextureData, s32 x, s32 y) {
    s32 width = pTextureData->getMipWidth(0);
    s32 height = pTextureData->getMipHeight(0);
    return sead::Color4u8::cBlack;
}

/**
 * @brief Reads a texel of a linear RGBA8 texture.
 */
sead::Color4u8 getColorFromLinearTexture(const agl::TextureData* pTextureData, s32 x, s32 y) {
    s32 height = pTextureData->getMipHeight(0);
    u8* image = getImagePtr(pTextureData);
    s32 pitch = pTextureData->getSurface().mStride / 4;
    s32 offset = (pitch * y + x) * 4;
    return sead::Color4u8(image[offset], image[offset + 1], image[offset + 2], image[offset + 3]);
}

/**
 * @brief Reads a channel of a texel of a linear half float texture.
 */
f32 getF32FromLinearTextureF16(const agl::TextureData* pTextureData, s32 x, s32 y, s32 channel,
                               s32 channelNum) {
    s32 height = pTextureData->getMipHeight(0);
    u32 rowPixels = pTextureData->getSurface().mStride / (channelNum * 2);
    u16* texel = reinterpret_cast<u16*>(getImagePtr(pTextureData)) +
                 (s32)((rowPixels * y + x) * channelNum);
    return convertF16ToF32(texel[channel]);
}

/**
 * @brief Reads a channel of a texel of a linear float texture.
 */
f32 getF32FromLinearTextureF32(const agl::TextureData* pTextureData, s32 x, s32 y, s32 channel,
                               s32 channelNum) {
    s32 height = pTextureData->getMipHeight(0);
    u32 rowPixels = pTextureData->getSurface().mStride / (channelNum * 4);
    f32* texel = reinterpret_cast<f32*>(getImagePtr(pTextureData)) +
                 (s32)((rowPixels * y + x) * channelNum);
    return texel[channel];
}

/**
 * @brief Reads a normal from a linear two-channel signed normal map texel.
 */
void calcNormalFromLinearTexture(sead::Vector3f* pNormal, const agl::TextureData* pTextureData,
                                 s32 x, s32 y) {
    s32 height = pTextureData->getMipHeight(0);
    u32 rowPixels = pTextureData->getSurface().mStride / 2;
    s8* texel = reinterpret_cast<s8*>(getImagePtr(pTextureData)) + (s32)(rowPixels * y + x) * 2;
    pNormal->x = texel[0] / 127.0f;
    pNormal->z = texel[1] / 127.0f;
    pNormal->y = sead::Mathf::sqrt(
        sead::Mathf::clampMin(1.0f - (pNormal->x * pNormal->x + pNormal->z * pNormal->z), 0.0f));
    normalize(pNormal);
}

/**
 * @brief Activates a sampler on the slot of the given name of a shader program.
 */
void activateSampler(agl::DrawContext* pDrawContext, const agl::TextureSampler* pSampler,
                     const agl::ShaderProgram* pProgram, const char* pName) {
    agl::SamplerLocation location(pName);
    location.search(*pProgram);
    pSampler->activate(pDrawContext, location, -1, false);
}

}  // namespace al
