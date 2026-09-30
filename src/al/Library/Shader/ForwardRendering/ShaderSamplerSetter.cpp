#include "Library/Shader/ForwardRendering/ShaderSamplerSetter.hpp"

#include <cmath>
#include <nn/g3d/g3d_ResFile.h>

#include <common/aglDisplayList.h>
#include <common/aglDrawContext.h>
#include <common/aglGPUMemAddr.h>
#include <common/aglShaderLocation.h>
#include <common/aglTextureDataInitializer.h>
#include <g3d/aglNW4FToNN.h>
#include <g3d/aglTextureDataInitializerG3D.h>
#include <gfx/seadGraphics.h>
#include <utility/aglDynamicTextureAllocator.h>
#include <utility/aglPrimitiveTexture.h>
#include <utility/aglResParameter.h>

#include "Library/File/FileUtil.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Draw/GpuMemAllocator.hpp"

namespace {
/**
 * @brief Gets the CPU pointer of a GPU memory address.
 */
template <typename T>
T* getAddrPtr(const agl::GPUMemAddrBase& rAddr) {
    return reinterpret_cast<T*>(
        static_cast<u8*>(nvnMemoryPoolMap(rAddr.getMemoryPool()->getDriverPool())) +
        rAddr.getByteOffset());
}
}  // namespace

namespace al {

/**
 * @brief Takes the draw context lock when asked to.
 */
DynamicTexAlloc::DynamicTexAlloc(bool isLock) : mIsLock(isLock) {
    if (isLock) {
        sead::Graphics::instance()->lockDrawContext();
    }
}

/**
 * @brief Releases the draw context lock if it was taken.
 */
DynamicTexAlloc::~DynamicTexAlloc() {
    if (mIsLock) {
        sead::Graphics::instance()->unlockDrawContext();
    }
}

/**
 * @brief Gets the dynamic texture allocator.
 */
agl::utl::DynamicTextureAllocator* DynamicTexAlloc::getAlloc() const {
    return agl::utl::DynamicTextureAllocator::instance();
}

/**
 * @brief Frees a texture made by the dynamic texture allocator.
 */
void DynamicTexAlloc::freeTex(agl::TextureData* pTexture) {
    getAlloc()->free(pTexture);
}

/**
 * @brief Computes the number of mip levels of a texture of the given size.
 */
s32 calcMaxMipLevelNum(s32 size) {
    return std::logf(size) * 1.442695f + 1.0f;
}

/**
 * @brief Gets the white 2D primitive sampler.
 */
const agl::TextureSampler* getWhite2DSampler() {
    return agl::utl::PrimitiveTexture::instance()->getTextureSampler(
        agl::utl::PrimitiveTexture::cType_White2D);
}

/**
 * @brief Gets the black 2D primitive sampler.
 */
const agl::TextureSampler* getBlack2DSampler() {
    return agl::utl::PrimitiveTexture::instance()->getTextureSampler(
        agl::utl::PrimitiveTexture::cType_Black2D);
}

/**
 * @brief Gets the red 2D primitive sampler.
 */
const agl::TextureSampler* getRed2DSampler() {
    return agl::utl::PrimitiveTexture::instance()->getTextureSampler(
        agl::utl::PrimitiveTexture::cType_Red2D);
}

/**
 * @brief Gets the green 2D primitive sampler.
 */
const agl::TextureSampler* getGreen2DSampler() {
    return agl::utl::PrimitiveTexture::instance()->getTextureSampler(
        agl::utl::PrimitiveTexture::cType_Green2D);
}

/**
 * @brief Gets the blue 2D primitive sampler.
 */
const agl::TextureSampler* getBlue2DSampler() {
    return agl::utl::PrimitiveTexture::instance()->getTextureSampler(
        agl::utl::PrimitiveTexture::cType_Blue2D);
}

/**
 * @brief Gets the black cube map primitive sampler.
 */
const agl::TextureSampler* getBlackCubeSampler() {
    return agl::utl::PrimitiveTexture::instance()->getTextureSampler(
        agl::utl::PrimitiveTexture::cType_BlackCube);
}

/**
 * @brief Gets the white 2D primitive texture.
 */
const agl::TextureData& getWhite2DTexture() {
    return getWhite2DSampler()->getTextureData();
}

/**
 * @brief Gets the black 2D primitive texture.
 */
const agl::TextureData& getBlack2DTexture() {
    return getBlack2DSampler()->getTextureData();
}

/**
 * @brief Gets the red 2D primitive texture.
 */
const agl::TextureData& getRed2DTexture() {
    return getRed2DSampler()->getTextureData();
}

/**
 * @brief Gets the green 2D primitive texture.
 */
const agl::TextureData& getGreen2DTexture() {
    return getGreen2DSampler()->getTextureData();
}

/**
 * @brief Gets the blue 2D primitive texture.
 */
const agl::TextureData& getBlue2DTexture() {
    return getBlue2DSampler()->getTextureData();
}

/**
 * @brief Gets the black cube map primitive texture.
 */
const agl::TextureData& getBlackCubeTexture() {
    return getBlackCubeSampler()->getTextureData();
}

/**
 * @brief Finds a texture by name.
 */
TextureInfo* TextureInfoArray::findTexture(const char* pName) const {
    s32 num = size();
    for (s32 i = 0; i < num; i++) {
        TextureInfo* info = at(i);
        if (isEqualString(pName, info->mName.cstr())) {
            return info;
        }
    }

    return nullptr;
}

/**
 * @brief Finds the index of a texture by name (-1 when missing).
 */
s32 TextureInfoArray::findTextureIndex(const char* pName) const {
    s32 num = size();
    for (s32 i = 0; i < num; i++) {
        if (isEqualString(pName, at(i)->mName.cstr())) {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Creates a sampler location and searches it in a shader program.
 */
agl::SamplerLocation* makeSamplerLoc(const agl::ShaderProgram& rProgram, const char* pName) {
    agl::SamplerLocation* location = new agl::SamplerLocation(pName);
    location->search(rProgram);
    return location;
}

/**
 * @brief Initializes a texture from a texture of a bfres without setting up the file.
 */
void InitializeAGLTextureNoResSetup(agl::TextureData* pTextureData, void* pResFile,
                                    const sead::SafeString& rName) {
    nn::g3d::ResFile* resFile = nn::g3d::ResFile::ResCast(pResFile);
    agl::g3d::TextureDataInitializerG3D::initialize(
        pTextureData, *agl::g3d::ResFile::GetTexture(resFile, rName.cstr()));
}

/**
 * @brief Loads every texture of a bfres of an archive into a name-sorted array.
 */
void loadTextureInfoArray(TextureInfoArray* pArray, const char* pArchiveName,
                          const char* pFileName, const char* pUnused, bool isCreateSampler) {
    Resource* resource = findOrCreateResource(pArchiveName, nullptr);
    void* file = resource->getOtherFile(pFileName, nullptr);
    const nn::g3d::ResFile* resFile = resource->getResFile();
    s32 textureNum = agl::g3d::ResFile::GetTextureCount(resFile);
    pArray->allocBuffer(textureNum, nullptr);
    for (s32 i = 0; i < textureNum; i++) {
        TextureInfo* info = new TextureInfo();
        info->mTextureData = new agl::TextureData();
        info->mName.format("%s", agl::g3d::ResFile::GetTextureName(resFile, i));
        InitializeAGLTextureNoResSetup(info->mTextureData, file, info->mName.cstr());
        info->mTextureData->flushCPUCache();
        if (isCreateSampler) {
            info->mSampler = new agl::TextureSampler(*info->mTextureData);
        } else {
            info->mSampler = nullptr;
        }

        pArray->pushBack(info);
    }

    pArray->sort();
}

/**
 * @brief Loads the textures of an archive when the archive exists.
 */
void tryLoadTextureInfoArray(TextureInfoArray* pArray, const char* pArchiveName,
                             const char* pFileName, const char* pUnused, bool isCreateSampler) {
    if (isExistArchive(pArchiveName)) {
        loadTextureInfoArray(pArray, pArchiveName, pFileName, pUnused, isCreateSampler);
    }
}

/**
 * @brief Destroys a texture info made by loadTextureInfoArray.
 */
void freeTextureInfo(TextureInfo* pInfo) {
    if (pInfo->mSampler != nullptr) {
        delete pInfo->mSampler;
        pInfo->mSampler = nullptr;
    }

    delete pInfo->mTextureData;
    delete pInfo;
}

/**
 * @brief Creates a look-up texture and its image memory.
 */
LutTexture::LutTexture(agl::TextureFormat format, u32 width, u32 height, u32 depth)
    : mWidth(width), mHeight(height), mDepth(depth), mFormat(format) {
    create();
}

/**
 * @brief Allocates the image memory and sets up the texture and sampler.
 */
void LutTexture::create() {
    initTexData();
    mMemBlock.allocBuffer_(mTextureData.getImageByteSize(), nullptr, 0x2000,
                           agl::MemoryAttribute(0));
    agl::GPUMemAddrBase addr(mMemBlock, 0);
    initCore();
}

/**
 * @brief Destroys the display list and the image memory.
 */
LutTexture::~LutTexture() {
    if (mDisplayList != nullptr) {
        delete mDisplayList;
        mDisplayList = nullptr;
    }

    if (agl::GPUMemAddrBase(mMemBlock, 0).isValid()) {
        agl::GPUMemAddrBase(mMemBlock, 0).invalidate();
    }
}

/**
 * @brief Records the activation of the sampler into a display list.
 */
void LutTexture::createDisplayList(GpuMemAllocator* pAllocator, agl::DrawContext* pDrawContext,
                                   const agl::SamplerLocation& rLocation, sead::Heap* pHeap) {
    mDisplayList = new agl::DisplayList();
    agl::DrawContext drawContext;
    drawContext.setCommandBuffer(mDisplayList);
    sead::Graphics::instance()->lockDrawContext();
    agl::GPUMemAddrBase buffer = pAllocator->allocMemory("DisplayList", 0x400, 4);
    mDisplayList->beginDisplayListBuffer(buffer, 0x400, true);
    mSampler.activate(&drawContext, rLocation, -1, false);
    mDisplayList->endDisplayList();
    sead::Graphics::instance()->unlockDrawContext();
}

/**
 * @brief Sets up the texture format, picking 1D, 2D or 3D from the size.
 */
void LutTexture::initTexData() {
    if (mHeight == 0) {
        mTextureData.initialize_(agl::TextureType::cTextureType_1D, mFormat, mWidth, 1, 1, 1,
                                 agl::TextureAttribute(0), agl::MultiSampleType(0), true);
    } else if (mDepth == 0) {
        mTextureData.initialize_(agl::TextureType::cTextureType_2D, mFormat, mWidth, mHeight, 1,
                                 1, agl::TextureAttribute(0), agl::MultiSampleType(0), true);
    } else {
        mTextureData.initialize_(agl::TextureType::cTextureType_3D, mFormat, mWidth, mHeight,
                                 mDepth, 1, agl::TextureAttribute(0), agl::MultiSampleType(0),
                                 true);
    }
}

/**
 * @brief Binds the image memory to the texture and sets up the sampler.
 */
void LutTexture::initCore() {
    mTextureData.setImagePtr(agl::GPUMemAddrBase(mMemBlock, 0), 0);
    agl::TextureDataInitializerRAW::copyTileImage(&mTextureData,
                                                  agl::GPUMemAddrBase(mMemBlock, 0), 0);
    mSampler.applyTextureData(mTextureData);
    mSampler.setWrap(6, 6, 6);
    mSampler.setFilter(1, 1, 0);
}

/**
 * @brief Writes a 16-bit texel.
 */
void LutTexture::storeU16(u16 value, s32 index) {
    agl::GPUMemAddr<u16> addr(agl::GPUMemAddrBase(mMemBlock, 0));
    getAddrPtr<u16>(addr)[index] = value;
}

/**
 * @brief Sets up the texture and sampler again.
 */
void LutTexture::reinit() {
    initTexData();
    initCore();
}

/**
 * @brief Samples the curve into the texture, from t = 1 down to t = 0.
 */
void LutCurve::updateTexData() {
    agl::GPUMemAddr<f32> addr(agl::GPUMemAddrBase(mMemBlock, 0));
    for (u32 i = 0; i < mWidth; i++) {
        getAddrPtr<f32>(addr)[i] = mCurve.interpolateToF32(0, 1.0f - (f32)i / (f32)mWidth);
    }

    reinit();
}

/**
 * @brief Creates the parameter file with its curve object.
 */
CurveIo::CurveIo(const char* pName, const char* pObjName, const char* pParamName,
                 const char* pFileName, const char* pArchiveName)
    : mParamIO(pName, 0), mObjName(pObjName), mParamName(pParamName), mFileName(pFileName),
      mArchiveName(pArchiveName) {
    mParamIO.addObj(&mParamObj, pObjName);
}

/**
 * @brief Applies the parameter file of the archive when it exists.
 */
void CurveIo::loadResource() {
    StringTmp<256> archivePath("ObjectData/%s", mArchiveName);
    if (!isExistArchive(archivePath.cstr())) {
        return;
    }

    Resource* resource = findOrCreateResource(archivePath.cstr(), nullptr);
    StringTmp<256> filePath("%s.b%s", mFileName, mParamIO.getType().cstr());
    if (resource != nullptr && resource->isExistFile(filePath)) {
        mParamIO.applyResParameterArchive(
            agl::utl::ResParameterArchive(resource->getOtherFile(filePath, nullptr)));
    }
}

/**
 * @brief Activates a sampler on the slot of the given name of a shader program.
 */
ShaderSamplerSetter::ShaderSamplerSetter(agl::DrawContext* pDrawContext,
                                         const agl::ShaderProgram* pProgram,
                                         const agl::TextureSampler* pSampler, const char* pName,
                                         bool isUnused) {
    if (pSampler != nullptr) {
        agl::SamplerLocation location(pName);
        location.search(*pProgram);
        pSampler->activate(pDrawContext, location, -1, false);
    }
}

}  // namespace al
