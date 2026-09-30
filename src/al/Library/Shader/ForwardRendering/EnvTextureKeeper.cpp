#include "Library/Shader/ForwardRendering/EnvTextureKeeper.hpp"

#include <cstdlib>
#include <nn/g3d/g3d_ResMaterial.h>

#include "Library/Model/Function/alModelFunction.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {

/**
 * @brief Marks every id as invalid.
 */
void EnvTexId::invalidateAll() {
    mCubeMapId = -1;
    mRefractCubeMapId = -1;
    mRoughness = -1;
    mRefract = -1;
    mFresnel = -1;
    mThickness = -1;
    mLightCategory = -1;
    mIrradiance = -1;
    mMirrorTexId = -1;
}

/**
 * @brief Marks every id as "not cached yet", so the next check always reports a change.
 */
void EnvTexId::initForCache() {
    mCubeMapId = -100;
    mRefractCubeMapId = -100;
    mRoughness = -100;
    mRefract = -100;
    mFresnel = -100;
    mThickness = -100;
    mLightCategory = -100;
    mIrradiance = -100;
    mMirrorTexId = -100;
}

/**
 * @brief Checks whether an id is neither invalid nor uncached.
 */
bool EnvTexId::isEnableTexId(s32 id) {
    return id != -1 && id != -100;
}

/**
 * @brief Copies every valid id of another set over this one.
 */
void EnvTexId::change(const EnvTexId& rOther) {
    if (isEnableTexId(rOther.mCubeMapId)) {
        mCubeMapId = rOther.mCubeMapId;
    }

    if (isEnableTexId(rOther.mRefractCubeMapId)) {
        mRefractCubeMapId = rOther.mRefractCubeMapId;
    }

    if (isEnableTexId(rOther.mRoughness)) {
        mRoughness = rOther.mRoughness;
    }

    if (isEnableTexId(rOther.mRefract)) {
        mRefract = rOther.mRefract;
    }

    if (isEnableTexId(rOther.mIrradiance)) {
        mIrradiance = rOther.mIrradiance;
    }

    if (isEnableTexId(rOther.mFresnel)) {
        mFresnel = rOther.mFresnel;
    }

    if (isEnableTexId(rOther.mThickness)) {
        mThickness = rOther.mThickness;
    }

    if (isEnableTexId(rOther.mLightCategory)) {
        mLightCategory = rOther.mLightCategory;
    }

    if (isEnableTexId(rOther.mMirrorTexId)) {
        mMirrorTexId = rOther.mMirrorTexId;
    }
}

/**
 * @brief Takes the fresnel id of an info and reports whether it changed.
 */
bool EnvTexId::checkAndSetFresnelChange(const EnvTexInfo& rInfo) {
    bool isChanged = mFresnel != rInfo.getFresnel();
    mFresnel = rInfo.getFresnel();
    return isChanged;
}

/**
 * @brief Gets the fresnel id, preferring the override.
 */
s32 EnvTexInfo::getFresnel() const {
    return EnvTexId::isEnableTexId(mOverride.mFresnel) ? mOverride.mFresnel : mBase.mFresnel;
}

/**
 * @brief Takes the thickness id of an info and reports whether it changed.
 */
bool EnvTexId::checkAndSetThicknessChange(const EnvTexInfo& rInfo) {
    bool isChanged = mThickness != rInfo.getThickness();
    mThickness = rInfo.getThickness();
    return isChanged;
}

/**
 * @brief Gets the thickness id, preferring the override.
 */
s32 EnvTexInfo::getThickness() const {
    return EnvTexId::isEnableTexId(mOverride.mThickness) ? mOverride.mThickness :
                                                            mBase.mThickness;
}

/**
 * @brief Takes the mirror texture id of an info and reports whether it changed.
 */
bool EnvTexId::checkAndSetMirrorTexChange(const EnvTexInfo& rInfo) {
    bool isChanged = mMirrorTexId != rInfo.getMirrorTexId();
    mMirrorTexId = rInfo.getMirrorTexId();
    return isChanged;
}

/**
 * @brief Gets the mirror texture id, preferring the override.
 */
s32 EnvTexInfo::getMirrorTexId() const {
    return EnvTexId::isEnableTexId(mOverride.mMirrorTexId) ? mOverride.mMirrorTexId :
                                                              mBase.mMirrorTexId;
}

/**
 * @brief Takes the reflection cube map, roughness and light category of an info and reports
 * whether any of them changed.
 */
bool EnvTexId::checkAndSetReflectCubeMapChange(const EnvTexInfo& rInfo, s32 defaultCategory) {
    s32 category = rInfo.getLightCategory(defaultCategory);
    bool isChanged = (mCubeMapId != rInfo.getCubeMapId()) | (mRoughness != rInfo.getRoughness()) |
                     (mLightCategory != category);
    mCubeMapId = rInfo.getCubeMapId();
    mRoughness = rInfo.getRoughness();
    mLightCategory = category;
    return isChanged;
}

/**
 * @brief Gets the light category, preferring the override, then the material, then the default.
 */
s32 EnvTexInfo::getLightCategory(s32 defaultCategory) const {
    if (mOverride.mLightCategory != -1) {
        return mOverride.mLightCategory;
    }

    if (mBase.mLightCategory != -1) {
        return mBase.mLightCategory;
    }

    return defaultCategory;
}

/**
 * @brief Gets the reflection cube map id, preferring the override.
 */
s32 EnvTexInfo::getCubeMapId() const {
    return EnvTexId::isEnableTexId(mOverride.mCubeMapId) ? mOverride.mCubeMapId : mBase.mCubeMapId;
}

/**
 * @brief Gets the roughness id, preferring the override.
 */
s32 EnvTexInfo::getRoughness() const {
    return EnvTexId::isEnableTexId(mOverride.mRoughness) ? mOverride.mRoughness : mBase.mRoughness;
}

/**
 * @brief Takes the refraction settings and light category of an info and reports whether any of
 * them changed.
 */
bool EnvTexId::checkAndSetRefractChange(const EnvTexInfo& rInfo, s32 defaultCategory) {
    s32 category = rInfo.getLightCategory(defaultCategory);
    bool isChanged = (mRefractCubeMapId != rInfo.getRefractCubeMapId()) |
                     (mRoughness != rInfo.getRoughness()) | (mRefract != rInfo.getRefract()) |
                     (mLightCategory != category);
    mRefractCubeMapId = rInfo.getRefractCubeMapId();
    mRoughness = rInfo.getRoughness();
    mRefract = rInfo.getRefract();
    mLightCategory = category;
    return isChanged;
}

/**
 * @brief Gets the refraction cube map id, preferring the override.
 */
s32 EnvTexInfo::getRefractCubeMapId() const {
    return EnvTexId::isEnableTexId(mOverride.mRefractCubeMapId) ? mOverride.mRefractCubeMapId :
                                                                   mBase.mRefractCubeMapId;
}

/**
 * @brief Gets the refraction type, preferring the override.
 */
s32 EnvTexInfo::getRefract() const {
    return EnvTexId::isEnableTexId(mOverride.mRefract) ? mOverride.mRefract : mBase.mRefract;
}

/**
 * @brief Takes the irradiance id and light category of an info and reports whether either
 * changed.
 */
bool EnvTexId::checkAndSetIrradianceChange(const EnvTexInfo& rInfo, s32 defaultCategory) {
    s32 category = rInfo.getLightCategory(defaultCategory);
    bool isChanged = (mIrradiance != rInfo.getIrradiance()) | (mLightCategory != category);
    mIrradiance = rInfo.getIrradiance();
    mLightCategory = category;
    return isChanged;
}

/**
 * @brief Gets the irradiance id, preferring the override.
 */
s32 EnvTexInfo::getIrradiance() const {
    return EnvTexId::isEnableTexId(mOverride.mIrradiance) ? mOverride.mIrradiance :
                                                             mBase.mIrradiance;
}

/**
 * @brief Creates an info with default material settings and no overrides.
 */
EnvTexInfo::EnvTexInfo() {
    setRoughness(0);
    setRefract(0);
    setFresnel(0);
    setThickness(0);
    setLightCategory(-1);
    setMirrorTexId(-1);
}

/**
 * @brief Creates an info from the render info of a material.
 */
EnvTexInfo::EnvTexInfo(const nn::g3d::ResMaterial& rMaterial,
                       const nn::g3d::ShadingModelObj& rShadingModel) {
    setRoughness(alEnvTexFunction::calcRoughnessType(rMaterial));
    setRefract(alEnvTexFunction::calcRefractType(rMaterial, rShadingModel));
    setFresnel(alEnvTexFunction::calcFresnelType(rMaterial));
    setThickness(alEnvTexFunction::calcThicknessType(rMaterial, rShadingModel));
    setLightCategory(alEnvTexFunction::calcLightCategory(rMaterial));
    setMirrorTexId(-1);
}

}  // namespace al

namespace alEnvTexFunction {

/**
 * @brief Reads the roughness preset of a material (4 when it has none).
 */
s32 calcRoughnessType(const nn::g3d::ResMaterial& rMaterial) {
    const nn::g3d::ResRenderInfo* renderInfo = rMaterial.FindRenderInfo("roughness_preset");
    if (renderInfo == nullptr) {
        return 4;
    }

    return alModelFunction::getRoughnessPresetIndex(renderInfo);
}

/**
 * @brief Reads the refraction type of a material (6 for indirect shaders, else the refraction
 * roughness preset).
 */
s32 calcRefractType(const nn::g3d::ResMaterial& rMaterial,
                    const nn::g3d::ShadingModelObj& rShadingModel) {
    if (alModelFunction::isShaderIndirect(rShadingModel)) {
        return 6;
    }

    const nn::g3d::ResRenderInfo* renderInfo =
        rMaterial.FindRenderInfo("refract_roughness_preset");
    if (renderInfo == nullptr) {
        return 4;
    }

    return alModelFunction::getRoughnessPresetIndex(renderInfo);
}

/**
 * @brief Reads the fresnel curve index of a material.
 */
s32 calcFresnelType(const nn::g3d::ResMaterial& rMaterial) {
    const nn::g3d::ResRenderInfo* renderInfo = rMaterial.FindRenderInfo("fresnel_curve");
    if (renderInfo == nullptr) {
        return 0;
    }

    if (renderInfo->GetArrayLength() == 0) {
        return 0;
    }

    const char* str = renderInfo->GetString(0);
    char* end = nullptr;
    return strtol(str, &end, 0);
}

/**
 * @brief Reads the thickness curve index of a material (-1 when the shader has no thickness).
 */
s32 calcThicknessType(const nn::g3d::ResMaterial& rMaterial,
                      const nn::g3d::ShadingModelObj& rShadingModel) {
    if (!alModelFunction::isShaderUsingThickness(rShadingModel)) {
        return -1;
    }

    const nn::g3d::ResRenderInfo* renderInfo = rMaterial.FindRenderInfo("thickness_curve");
    if (renderInfo == nullptr) {
        return 0;
    }

    if (renderInfo->GetArrayLength() == 0) {
        return 0;
    }

    const char* str = renderInfo->GetString(0);
    char* end = nullptr;
    return strtol(str, &end, 0);
}

/**
 * @brief Reads the light category of a material ("1" -> 0, "2" -> 1, else -1).
 */
s32 calcLightCategory(const nn::g3d::ResMaterial& rMaterial) {
    const nn::g3d::ResRenderInfo* renderInfo = rMaterial.FindRenderInfo("light_category");
    if (renderInfo == nullptr) {
        return -1;
    }

    if (renderInfo->GetArrayLength() == 0) {
        return -1;
    }

    const char* category = renderInfo->GetString(0);
    if (al::isEqualString(category, "1")) {
        return 0;
    }

    return al::isEqualString(category, "2") ? 1 : -1;
}

}  // namespace alEnvTexFunction

namespace al {

/**
 * @brief Removes every override.
 */
void EnvTexInfo::clearAll() {
    mOverride.mCubeMapId = -1;
    mOverride.mRefractCubeMapId = -1;
    mOverride.mRoughness = -1;
    mOverride.mRefract = -1;
    mOverride.mFresnel = -1;
    mOverride.mThickness = -1;
    mOverride.mLightCategory = -1;
    mOverride.mIrradiance = -1;
    mOverride.mMirrorTexId = -1;
}

/**
 * @brief Checks whether the material refracts through the indirect texture.
 */
bool EnvTexInfo::isIndirectRefract() const {
    return mBase.mRefract == 6;
}

/**
 * @brief Checks whether the material refracts through a cube map.
 */
bool EnvTexInfo::isRefractCubeMap() const {
    return static_cast<u32>(mBase.mRefract) < 6;
}

}  // namespace al
