#pragma once

#include <basis/seadTypes.h>

namespace nn::g3d {
class ResMaterial;
class ResRenderInfo;
class ShadingModelObj;
}  // namespace nn::g3d

namespace al {
class EnvTexInfo;

/**
 * @brief Set of environment texture ids (cube maps, fresnel, thickness, light category, ...)
 * currently bound for a model. -1 means invalid, -100 means "not cached yet".
 */
class EnvTexId {
    friend class EnvTexInfo;

public:
    EnvTexId();

    void invalidateAll();
    void initForCache();
    static bool isEnableTexId(s32 id);
    void change(const EnvTexId& rOther);
    bool checkAndSetFresnelChange(const EnvTexInfo& rInfo);
    bool checkAndSetThicknessChange(const EnvTexInfo& rInfo);
    bool checkAndSetMirrorTexChange(const EnvTexInfo& rInfo);
    bool checkAndSetReflectCubeMapChange(const EnvTexInfo& rInfo, s32 defaultCategory);
    bool checkAndSetRefractChange(const EnvTexInfo& rInfo, s32 defaultCategory);
    bool checkAndSetIrradianceChange(const EnvTexInfo& rInfo, s32 defaultCategory);

    s32 getCubeMapId() const { return mCubeMapId; }
    s32 getRefractCubeMapId() const { return mRefractCubeMapId; }
    s32 getRoughness() const { return mRoughness; }
    s32 getRefract() const { return mRefract; }
    s32 getFresnel() const { return mFresnel; }
    s32 getThickness() const { return mThickness; }
    s32 getLightCategory() const { return mLightCategory; }
    s32 getIrradiance() const { return mIrradiance; }
    s32 getMirrorTexId() const { return mMirrorTexId; }

    void setCubeMapId(s32 id) { mCubeMapId = id; }

    void setRefractCubeMapId(s32 id) { mRefractCubeMapId = id; }

    void setRoughness(s32 roughness) { mRoughness = roughness; }

    void setRefract(s32 refract) { mRefract = refract; }

    void setFresnel(s32 fresnel) { mFresnel = fresnel; }

    void setThickness(s32 thickness) { mThickness = thickness; }

    void setLightCategory(s32 category) { mLightCategory = category; }

    void setIrradiance(s32 irradiance) { mIrradiance = irradiance; }

    void setMirrorTexId(s32 id) { mMirrorTexId = id; }

private:
    s32 mCubeMapId;
    s32 mRefractCubeMapId;
    s32 mRoughness;
    s32 mRefract;
    s32 mFresnel;
    s32 mThickness;
    s32 mLightCategory;
    s32 mIrradiance;
    s32 mMirrorTexId;
};

static_assert(sizeof(EnvTexId) == 0x24);

/**
 * @brief Environment texture settings of a material: ids read from the material's render info,
 * plus runtime overrides (an override wins when it is a valid id).
 */
class EnvTexInfo {
public:
    EnvTexInfo();
    EnvTexInfo(const nn::g3d::ResMaterial& rMaterial, const nn::g3d::ShadingModelObj& rShadingModel);

    s32 getFresnel() const;
    s32 getThickness() const;
    s32 getMirrorTexId() const;
    s32 getLightCategory(s32 defaultCategory) const;
    s32 getCubeMapId() const;
    s32 getRoughness() const;
    s32 getRefractCubeMapId() const;
    s32 getRefract() const;
    s32 getIrradiance() const;
    void clearAll();
    bool isIndirectRefract() const;
    bool isRefractCubeMap() const;

    const EnvTexId& getBaseId() const { return mBase; }

    const EnvTexId& getOverrideId() const { return mOverride; }

    void setOverrideCubeMapId(s32 id) { mOverride.mCubeMapId = id; }

    void setOverrideIrradiance(s32 irradiance) { mOverride.mIrradiance = irradiance; }

    void setOverrideRefractCubeMapId(s32 id) { mOverride.mRefractCubeMapId = id; }

private:
    void setRoughness(s32 roughness) {
        mBase.mRoughness = roughness;
        mOverride.mRoughness = -1;
    }

    void setRefract(s32 refract) {
        mBase.mRefract = refract;
        mOverride.mRefract = -1;
    }

    void setFresnel(s32 fresnel) {
        mBase.mFresnel = fresnel;
        mOverride.mFresnel = -1;
    }

    void setThickness(s32 thickness) {
        mBase.mThickness = thickness;
        mOverride.mThickness = -1;
    }

    void setLightCategory(s32 category) {
        mBase.mLightCategory = category;
        mOverride.mLightCategory = -1;
    }

    void setMirrorTexId(s32 id) {
        mBase.mMirrorTexId = id;
        mOverride.mMirrorTexId = -1;
    }

    EnvTexId mBase;
    EnvTexId mOverride;
};

static_assert(sizeof(EnvTexInfo) == 0x48);

}  // namespace al

namespace alEnvTexFunction {
s32 calcRoughnessType(const nn::g3d::ResMaterial& rMaterial);
s32 calcRefractType(const nn::g3d::ResMaterial& rMaterial,
                    const nn::g3d::ShadingModelObj& rShadingModel);
s32 calcFresnelType(const nn::g3d::ResMaterial& rMaterial);
s32 calcThicknessType(const nn::g3d::ResMaterial& rMaterial,
                      const nn::g3d::ShadingModelObj& rShadingModel);
s32 calcLightCategory(const nn::g3d::ResMaterial& rMaterial);
s32 calcLightCategory(const nn::g3d::ResRenderInfo* pRenderInfo);
}  // namespace alEnvTexFunction
