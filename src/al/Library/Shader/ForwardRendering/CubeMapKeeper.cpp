#include "Library/Shader/ForwardRendering/CubeMapKeeper.hpp"

#include <attributes.h>
#include <common/aglDisplayList.h>
#include <common/aglDrawContext.h>
#include <common/aglTextureData.h>
#include <common/aglTextureSampler.h>
#include <driver/aglGraphicsDriverMgr.h>
#include <g3d/aglNW4FToNN.h>
#include <gfx/seadGraphics.h>
#include <prim/seadEnum.h>

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/File/FileUtil.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/Model/SimpleModelEnv.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Shader/DeferredRendering/SamplerLocation.hpp"
#include "Library/Shader/ForwardRendering/ShaderSamplerSetter.hpp"
#include "Project/Base/StringOpUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Draw/GpuMemAllocator.hpp"
#include "Project/Draw/GraphicsAreaDirector.hpp"
#include "Project/Draw/GraphicsParamKeeper.hpp"

namespace al {

// clang-format off
SEAD_ENUM(CategoryLight, Map , Obj , Num)
SEAD_ENUM(RoughnessType, Mirror , HighGlossy , MiddleGlossy , LowGlossy , Matte , Irradiance , TypeNum)
// clang-format on

}  // namespace al

namespace {

/**
 * @brief Size and mip level count of a roughness cube map.
 */
struct RoughnessTextureSize {
    s32 size;
    s32 mipLevelNum;
};

const RoughnessTextureSize cRoughnessTextureSizes[] = {
    {128, al::calcMaxMipLevelNum(128)}, {64, al::calcMaxMipLevelNum(64)},
    {32, al::calcMaxMipLevelNum(32)},   {16, al::calcMaxMipLevelNum(16)},
    {16, al::calcMaxMipLevelNum(16)},   {8, al::calcMaxMipLevelNum(8)},
};

/**
 * @brief Creates the samplers of every roughness type of a light category.
 * @param pArray Roughness array to fill.
 * @param pCubeMapName Cube map name.
 * @param category Light category.
 * @param pTextureFile Texture file holding the cube map textures.
 */
void loadRoughnessTextures(al::RoughnessArray* pArray, const char* pCubeMapName, s32 category,
                           void* pTextureFile) {
    for (s32 roughness = 0; roughness <= al::RoughnessType::Irradiance; roughness++) {
        al::StringTmp<256> samplerName("%s_%s%s", pCubeMapName, al::CategoryLight::text(category),
                                       al::RoughnessType::text(roughness));
        agl::TextureData textureData;
        al::InitializeAGLTextureNoResSetup(&textureData, pTextureFile, samplerName.cstr());
        pArray->mSamplers.pushBack(new agl::TextureSampler(textureData));
    }
}

/**
 * @brief Records a display list binding a texture to a sampler location.
 * @param pAllocator Allocator of the display list's command memory.
 * @param rTextureData Texture to bind.
 * @param rLocation Sampler location to bind it to.
 * @return The recorded display list.
 */
agl::DisplayList* createSamplerDisplayList(al::GpuMemAllocator* pAllocator,
                                           const agl::TextureData& rTextureData,
                                           const agl::SamplerLocation& rLocation) {
    agl::DisplayList* displayList = new agl::DisplayList();
    agl::DrawContext context;
    context.setCommandBuffer(displayList);
    sead::Graphics::instance()->lockDrawContext();
    agl::GPUMemAddrBase memory = pAllocator->allocMemory("DisplayList", 0x400, 4);
    displayList->beginDisplayListBuffer(memory, 0x400, true);
    {
        agl::TextureSampler sampler(rTextureData);
        sampler.activate(&context, rLocation, -1, false);
    }

    displayList->endDisplayList();
    sead::Graphics::instance()->unlockDrawContext();
    return displayList;
}
}  // namespace

namespace al {

/**
 * @brief Names every roughness array after the cube map and its light category.
 */
void CubeMapInfo::setCategoryName() {
    s32 arrayNum = mRoughnessArrays.size();
    for (s32 i = 0; i < arrayNum; i++) {
        mRoughnessArrays[i]->mName.format("%s[%s]", mName.cstr(), CategoryLight::text(i));
    }
}

/**
 * @brief Creates the keeper and its category light settings.
 * @param pGraphicsSystemInfo Graphics system info.
 * @param pPlayerHolder Player holder (unused).
 */
ShaderCubeMapKeeper::ShaderCubeMapKeeper(GraphicsSystemInfo* pGraphicsSystemInfo,
                                         PlayerHolder* pPlayerHolder)
    : mGraphicsSystemInfo(pGraphicsSystemInfo),
      mParamFilePath(new GraphicsParamFilePath("CubeMapMgr", "aglcube")) {
    mGpuMemAllocator = pGraphicsSystemInfo->getGpuMemAllocator();
    mStandardLightInfoHolder = new CategoryLightInfoHolder("standard", 1.0f);
    mCharacterLightInfoHolder = new CategoryLightInfoHolder("character", 1.5f);
}

/**
 * @brief Destroys every cube map and the category light settings.
 */
ShaderCubeMapKeeper::~ShaderCubeMapKeeper() {
    while (!mCubeMapInfos.isEmpty()) {
        delete mCubeMapInfos.popBack();
    }

    if (mStandardLightInfoHolder != nullptr) {
        delete mStandardLightInfoHolder;
        mStandardLightInfoHolder = nullptr;
    }

    if (mCharacterLightInfoHolder != nullptr) {
        delete mCharacterLightInfoHolder;
        mCharacterLightInfoHolder = nullptr;
    }
}

/**
 * @brief Loads the cube maps of a stage and its category light settings.
 * @param pResource Stage resource.
 * @param pName Stage name.
 * @param pKit Actor kit (unused).
 */
void ShaderCubeMapKeeper::initStageResource(const Resource* pResource, const char* pName,
                                            const LiveActorKit* pKit) {
    mStageTexResInfo.tryLoad(pName);
    mStandardLightInfoHolder->initStageResource(pResource);
    mCharacterLightInfoHolder->initStageResource(pResource);
    makeTextureByInfo(mStageTexResInfo, false, nullptr, nullptr);
    mIsInitialized = true;
}

/**
 * @brief Finds the cube map archive of a name and takes its model and texture files.
 * @param pName Cube map name.
 */
void ShaderCubeMapKeeper::TexResInfo::tryLoad(const char* pName) {
    StringTmp<256> archiveName("ObjectData/CubeMap%s", pName);

    if (!isExistArchive(archiveName)) {
        archiveName.format("CubeMapTextureData/CubeMap%s", pName);

        if (!isExistArchive(archiveName)) {
            archiveName.format("CubeMapTextureData/%s", pName);

            if (!isExistArchive(archiveName)) {
                return;
            }
        }
    }

    Resource* resource = findOrCreateResource(archiveName, nullptr);
    if (resource == nullptr) {
        return;
    }

    StringTmp<256> fileName("CubeMap%s.bfres", pName);

    if (!resource->isExistFile(fileName)) {
        fileName.format("%s.bfres", pName);

        if (!resource->isExistFile(fileName)) {
            return;
        }
    }

    mTextureFile = resource->getOtherFile(fileName, nullptr);
    resource->getFileSize(fileName);
    mResFile = resource->getResFile();
}

/**
 * @brief Creates a cube map for every texture set of a model file that is not registered yet.
 * Textures are named "<cube map>_<light category><roughness type>".
 * @param rInfo Resources to take the textures from.
 * @param isUnused Unused.
 * @param pName Only create the cube map of this name (all cube maps when null).
 * @param pUnused Unused.
 */
void ShaderCubeMapKeeper::makeTextureByInfo(const TexResInfo& rInfo, bool isUnused,
                                            const char* pName, const char* pUnused) {
    if (rInfo.mResFile == nullptr || agl::g3d::ResFile::GetTextureCount(rInfo.mResFile) <= 0) {
        return;
    }

    s32 textureNum = agl::g3d::ResFile::GetTextureCount(rInfo.mResFile);
    for (s32 i = 0; i < textureNum; i++) {
        const char* textureName = agl::g3d::ResFile::GetTextureName(rInfo.mResFile, i);
        const char* separator = searchSubString(textureName, "_");

        if (separator == nullptr) {
            continue;
        }

        char cubeMapName[256] = {};
        s32 nameLength = 0;
        for (; &textureName[nameLength] != separator; nameLength++) {
            cubeMapName[nameLength] = textureName[nameLength];
        }

        cubeMapName[nameLength] = '\0';

        if (pName != nullptr && !isEqualString(cubeMapName, pName)) {
            continue;
        }

        if (findCubeMapInfoByName(cubeMapName) != nullptr) {
            continue;
        }

        CubeMapInfo* cubeMapInfo = new CubeMapInfo(2, nullptr);
        cubeMapInfo->mName.format("%s", cubeMapName);
        cubeMapInfo->setCategoryName();
        mCubeMapInfos.pushBack(cubeMapInfo);

        for (s32 category = 0; category < 2; category++) {
            RoughnessArray* roughnessArray = cubeMapInfo->mRoughnessArrays[category];
            loadRoughnessTextures(roughnessArray, cubeMapName, category, rInfo.mTextureFile);

            RoughnessArrayDL& displayLists = roughnessArray->mDisplayLists;
            for (s32 j = 0; j < roughnessArray->mSamplers.size(); j++) {
                const agl::TextureData& textureData =
                    roughnessArray->mSamplers[j]->getTextureData();
                GpuMemAllocator* allocator = mGpuMemAllocator;

                if (j == RoughnessType::Irradiance) {
                    displayLists.mIrradiance = createSamplerDisplayList(
                        allocator, textureData, getSamplerLocationCubeMapIrradiance());
                } else {
                    displayLists.mRoughness[j] = createSamplerDisplayList(
                        allocator, textureData, getSamplerLocationCubeMapRoughness());
                    displayLists.mRefract[j] = createSamplerDisplayList(
                        allocator, textureData, getSamplerLocationCubeMapRoughnessRefract());
                }
            }
        }
    }
}

/**
 * @brief Loads the default cube maps and selects the default one.
 */
void ShaderCubeMapKeeper::endInit() {
    mDefaultTexResInfo.tryLoad("DefaultCubeMapStage");
    makeTextureByInfo(mDefaultTexResInfo, false, nullptr, nullptr);
    mDefaultCubeMapInfo = findCubeMapInfoByName("Default");

    if (mDefaultCubeMapInfo == nullptr) {
        mDefaultCubeMapInfo = findCubeMapInfoByName("DefaultPreset");
    }

    mCurrentCubeMapInfo = mDefaultCubeMapInfo;
}

/**
 * @brief Finds a cube map by name.
 * @param pName Cube map name.
 * @return The cube map, or null if there is none of this name.
 */
CubeMapInfo* ShaderCubeMapKeeper::findCubeMapInfoByName(const char* pName) const {
    s32 index = findCubeMapIndexByName(pName);
    if (index == -1) {
        return nullptr;
    }

    return mCubeMapInfos[index];
}

/**
 * @brief Creates an empty display list set.
 */
RoughnessArrayDL::RoughnessArrayDL() {
    for (s32 i = 0; i < 5; i++) {
        mRoughness[i] = nullptr;
    }

    for (s32 i = 0; i < 5; i++) {
        mRefract[i] = nullptr;
    }

    mIrradiance = nullptr;
}

/**
 * @brief Checks whether the textures are loaded.
 */
bool RoughnessArray::isFileLoaded() const {
    return mSamplers.size() != 0;
}

/**
 * @brief Checks whether the textures of the first light category are loaded.
 */
bool CubeMapInfo::isFileLoaded() const {
    return mRoughnessArrays(0)->isFileLoaded();
}

/**
 * @brief Destroys every roughness array.
 */
CubeMapInfo::~CubeMapInfo() {
    while (!mRoughnessArrays.isEmpty()) {
        delete mRoughnessArrays.popBack();
    }

    mRoughnessArrays.freeBuffer();
}

/**
 * @brief Binds the texture of a roughness type, or the black cube map when there is none.
 * @param roughness Roughness type.
 * @param isRefract Whether to bind it as the refraction cube map.
 * @param isForceBlack Whether to bind the black cube map.
 * @return Always true.
 */
ALWAYS_INLINE bool RoughnessArray::activateTexture(s32 roughness, bool isRefract,
                                                   bool isForceBlack) const {
    const agl::SamplerLocation* location = &getSamplerLocationCubeMapRoughness();

    if (roughness == RoughnessType::Irradiance) {
        location = &getSamplerLocationCubeMapIrradiance();
    } else if (isRefract) {
        location = &getSamplerLocationCubeMapRoughnessRefract();
    }

    if (!isForceBlack && isFileLoaded()) {
        const agl::DisplayList* displayList;
        if (roughness == RoughnessType::Irradiance) {
            displayList = mDisplayLists.mIrradiance;
        } else if (isRefract) {
            displayList = mDisplayLists.mRefract[roughness];
        } else {
            displayList = mDisplayLists.mRoughness[roughness];
        }

        nvnCommandBufferCallCommands(
            agl::driver::getNvnCommandBuffer(GameFrameworkNx::getAglDrawContext()), 1,
            displayList->getHandlePtr());
    } else {
        getBlackCubeSampler()->activate(GameFrameworkNx::getAglDrawContext(), *location, -1,
                                        false);
    }

    return true;
}

/**
 * @brief Destroys every sampler and display list.
 */
RoughnessArray::~RoughnessArray() {
    for (s32 i = 0; i < mSamplers.size(); i++) {
        delete mSamplers[i];
    }

    for (s32 i = 0; i < 5; i++) {
        delete mDisplayLists.mRoughness[i];
    }

    for (s32 i = 0; i < 5; i++) {
        delete mDisplayLists.mRefract[i];
    }

    delete mDisplayLists.mIrradiance;
}

/**
 * @brief Binds a texture of a cube map.
 * @param index Cube map index (-1 for the current cube map).
 * @param roughness Roughness type.
 * @param category Light category.
 * @param isRefract Whether to bind it as the refraction cube map.
 * @return Always true.
 */
bool ShaderCubeMapKeeper::activateCubeMapTexture(s32 index, s32 roughness, s32 category,
                                                 bool isRefract) const {
    const RoughnessArray* roughnessArray = findCubeMapInfo(index)->mRoughnessArrays[category];
    return roughnessArray->activateTexture(roughness, isRefract, false);
}

/**
 * @brief Gets the cube map to use for an index: the forced one, else the indexed one (clamped),
 * else the current one.
 * @param index Cube map index (-1 for the current cube map).
 */
CubeMapInfo* ShaderCubeMapKeeper::findCubeMapInfo(s32 index) const {
    if (mForceCubeMapInfo != nullptr) {
        return mForceCubeMapInfo;
    }

    if (mCubeMapInfos.size() == 0) {
        return mDefaultCubeMapInfo;
    }

    if (index == -1) {
        if (mCurrentCubeMapInfo != nullptr) {
            return mCurrentCubeMapInfo;
        }

        return mDefaultCubeMapInfo;
    }

    return mCubeMapInfos[sead::Mathi::clamp(index, 0, mCubeMapInfos.size() - 1)];
}

/**
 * @brief Gets the light preset name of the current cube map.
 */
const char* ShaderCubeMapKeeper::tryGetCurrentCubeMapLightPresetName() const {
    if (mCurrentCubeMapInfo != nullptr) {
        return mCurrentCubeMapInfo->mName.cstr();
    }

    return "Default";
}

/**
 * @brief Gets the light preset name of a cube map name (the name itself).
 */
const char* ShaderCubeMapKeeper::tryGetCubeMapLightPresetName(const char* pName) const {
    return pName;
}

/**
 * @brief Gets the light preset name of a cube map index.
 */
const char* ShaderCubeMapKeeper::tryGetCubeMapLightPresetName(s32 index) const {
    return findCubeMapInfo(index)->mName.cstr();
}

/**
 * @brief Checks whether cube maps are drawn (never in this game).
 */
bool ShaderCubeMapKeeper::isDrawCubeMap() const {
    return false;
}

/**
 * @brief Sets the name of the cube map to use.
 */
void ShaderCubeMapKeeper::setCubeMap(const char* pName) {
    mCubeMapName = pName;
}

/**
 * @brief Selects the cube map requested by the current graphics area.
 */
void ShaderCubeMapKeeper::updateCubeMapKeeper() {
    GraphicsAreaDirector* areaDirector = mGraphicsSystemInfo->getGraphicsAreaDirector();
    if (areaDirector == nullptr) {
        return;
    }

    CurrentGraphicsAreaParam areaParam;
    areaDirector->getCurrentGraphicsAreaParam(&areaParam,
                                              GraphicsAreaParamType::CubeMapCapturePoint);

    if (areaParam.mRate > 0.5f) {
        if (areaParam.mParamName != nullptr) {
            mCurrentCubeMapInfo = tryFindCubeMapInfoByName(areaParam.mParamName);
        } else {
            mCurrentCubeMapInfo = tryFindCubeMapInfoByName("Default");
        }
    }
}

/**
 * @brief Finds a cube map by name, falling back to the default one.
 * @param pName Cube map name.
 */
CubeMapInfo* ShaderCubeMapKeeper::tryFindCubeMapInfoByName(const char* pName) const {
    CubeMapInfo* cubeMapInfo = findCubeMapInfoByName(pName);
    if (cubeMapInfo != nullptr) {
        return cubeMapInfo;
    }

    if (mDefaultCubeMapInfo != nullptr) {
        return mDefaultCubeMapInfo;
    }

    if (mCubeMapInfos.size() > 0) {
        return mCubeMapInfos(0);
    }

    return nullptr;
}

/**
 * @brief Finds the index of a cube map.
 * @param pName Cube map name.
 * @return The index, or -1 if there is no cube map of this name.
 */
s32 ShaderCubeMapKeeper::findCubeMapIndexByName(const char* pName) const {
    s32 infoNum = mCubeMapInfos.size();
    for (s32 i = 0; i < infoNum; i++) {
        if (isEqualString(sead::SafeString(pName), mCubeMapInfos[i]->mName)) {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Gets the light settings of the current cube map for a light category.
 * @param category Light category.
 */
const CategoryLightInfo* ShaderCubeMapKeeper::getCurrentCategoryLightInfo(s32 category) const {
    if (mCurrentCubeMapInfo == nullptr) {
        return nullptr;
    }

    return getCategoryLightInfoHolder(category)->tryGetLightInfo(
        mCurrentCubeMapInfo->mName.cstr());
}

/**
 * @brief Gets the light settings of a light category (1: characters, else the standard one).
 */
CategoryLightInfoHolder* ShaderCubeMapKeeper::getCategoryLightInfoHolder(s32 category) const {
    return category == 1 ? mCharacterLightInfoHolder : mStandardLightInfoHolder;
}

/**
 * @brief Gets the irradiance map of the current cube map.
 * @param category Light category.
 * @param rPos Position (unused).
 */
const agl::TextureSampler* ShaderCubeMapKeeper::getIrradiance(s32 category,
                                                              const sead::Vector3f& rPos) const {
    return findCubeMapInfo(-1)->mRoughnessArrays(category)->mSamplers[RoughnessType::Irradiance];
}

/**
 * @brief Gets a roughness map of the current cube map.
 * @param roughness Roughness type.
 * @param category Light category.
 */
const agl::TextureSampler* ShaderCubeMapKeeper::getRoughnessCubeMap(s32 roughness,
                                                                    s32 category) const {
    return findCubeMapInfo(-1)->mRoughnessArrays(category)->mSamplers[roughness];
}

}  // namespace al

namespace CubeMapFunction {

/**
 * @brief Gets the cube map keeper of an actor's scene.
 */
al::ShaderCubeMapKeeper* getShaderCubeMapKeeper(const al::LiveActor* pActor) {
    return pActor->getSceneInfo()->graphicsSystemInfo->getShaderCubeMapKeeper();
}

}  // namespace CubeMapFunction
