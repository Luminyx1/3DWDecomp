#include "Library/Light/LightUtil.hpp"

#include <math/seadQuat.h>

#include "Library/Collision/PartsConnectorUtil.hpp"
#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Light/LppBase.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Shader/DeferredRendering/CubeMapDirector.hpp"
#include "Library/Yaml/MacroUtil.hpp"
#include "Project/Light/ActorPrePassLightKeeper.hpp"

namespace {
template <typename T>
al::PrePassLight<T>* tryGetPrePassLight(const al::LiveActor* pActor, const char* pName) {
    volatile s32 lightType = T::cLightType;
    al::PrePassLightBase* light = pActor->mLightKeeper->getLightBase(pName);
    if (light && light->getLightType() == lightType) {
        return static_cast<al::PrePassLight<T>*>(light);
    }

    return nullptr;
}

void calcLightPoseInfo(const al::PrePassLightBase* pLight, sead::Vector3f* pPos,
                       sead::Vector3f* pDir) {
    sead::Quatf quat;
    {
        sead::Vector3f rotate = pLight->mRotateOffsetDegree * sead::Mathf::deg2rad(1.0f);
        al::calcConnectInfo(pLight->mMtxConnector, pPos, &quat, nullptr, pLight->mOffset, rotate);
    }

    if (pDir) {
        sead::Vector3f up;
        al::calcQuatUp(&up, quat);
        pDir->x = -up.x;
        pDir->y = -up.y;
        pDir->z = -up.z;
        al::normalizeOrZero(pDir);
    }
}
}  // namespace

namespace al {
/**
 * Checks whether a prepass light is active.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @return Whether the light is registered to the light keeper.
 */
bool isActivePrePassLight(const LiveActor* pActor, const char* pName) {
    PrePassLightBase* light = pActor->mLightKeeper->getLightBase(pName);
    if (!light) {
        return false;
    }

    return light->isActive();
}

/**
 * Checks whether a prepass light exists.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @return Whether the light exists.
 */
bool isExistPrePassLight(const LiveActor* pActor, const char* pName) {
    ActorPrePassLightKeeper* keeper = pActor->mLightKeeper;
    if (!keeper) {
        return false;
    }

    return keeper->getLightBase(pName) != nullptr;
}

/**
 * Gets a prepass line light.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @return The light, or nullptr.
 */
PrePassLightBase* getPrePassLineLight(const LiveActor* pActor, const char* pName) {
    ActorPrePassLightKeeper* keeper = pActor->mLightKeeper;
    if (!keeper) {
        return nullptr;
    }

    return keeper->getLightBase(pName);
}

/**
 * Makes a prepass light appear.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @param step Fade step.
 */
void appearPrePassLight(const LiveActor* pActor, const char* pName, s32 step) {
    PrePassLightBase* light = pActor->mLightKeeper->getLightBase(pName);
    if (light) {
        light->requestAppearByUser(step);
    }
}

/**
 * Kills a prepass light.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @param step Fade step.
 */
void killPrePassLight(const LiveActor* pActor, const char* pName, s32 step) {
    PrePassLightBase* light = pActor->mLightKeeper->getLightBase(pName);
    if (light) {
        light->requestKillByUser(step);
    }
}

/**
 * Makes every prepass light of an actor appear.
 * @param pActor Actor owning the lights.
 * @param step Fade step.
 */
void appearPrePassLightAll(const LiveActor* pActor, s32 step) {
    ActorPrePassLightKeeper* keeper = pActor->mLightKeeper;
    if (!keeper) {
        return;
    }

    s32 num = keeper->getLightNum();
    for (s32 i = 0; i < num; i++) {
        keeper->getLightBase(i)->requestAppearByUser(step);
    }
}

/**
 * Kills every prepass light of an actor.
 * @param pActor Actor owning the lights.
 * @param step Fade step.
 */
void killPrePassLightAll(const LiveActor* pActor, s32 step) {
    ActorPrePassLightKeeper* keeper = pActor->mLightKeeper;
    if (!keeper) {
        return;
    }

    s32 num = keeper->getLightNum();
    for (s32 i = 0; i < num; i++) {
        keeper->getLightBase(i)->requestKillByUser(step);
    }
}

/**
 * Sets the offset of a prepass light.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @param rOffset Light offset.
 */
void setPrePassLightOffset(const LiveActor* pActor, const char* pName, const sead::Vector3f& rOffset) {
    PrePassLightBase* light = pActor->mLightKeeper->getLightBase(pName);
    if (light) {
        light->mOffset = rOffset;
    }
}

/**
 * Requests a user color for a prepass light.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @param rColor Light color.
 */
void requestPrePassLightColor(const LiveActor* pActor, const char* pName, const sead::Color4f& rColor) {
    PrePassLightBase* light = pActor->mLightKeeper->getLightBase(pName);
    if (light) {
        light->requestUserColor(rColor);
    }
}

/**
 * Requests a scaled light color for a prepass light.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @param rate Color scale.
 */
void requestPrePassLightColor(const LiveActor* pActor, const char* pName, f32 rate) {
    PrePassLightBase* light = pActor->mLightKeeper->getLightBase(pName);
    if (light) {
        light->requestUserColor(light->mColor * rate);
    }
}

/**
 * Requests a scaled user color for a prepass light.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @param pColorName User color name.
 * @param rate Color scale.
 */
void requestPrePassLightColor(const LiveActor* pActor, const char* pName, const char* pColorName,
                              f32 rate) {
    sead::Color4f color = pActor->mLightKeeper->findUserColor(pColorName) * rate;
    PrePassLightBase* light = pActor->mLightKeeper->getLightBase(pName);
    if (light) {
        light->requestUserColor(color);
    }
}

/**
 * Gets a user color of the prepass lights.
 * @param pActor Actor owning the lights.
 * @param pName User color name.
 * @return The user color.
 */
const sead::Color4f& getPrePassUserColor(const LiveActor* pActor, const char* pName) {
    return pActor->mLightKeeper->findUserColor(pName);
}

/**
 * Enables or disables the specular of a prepass light.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @param isEnable Whether the specular is enabled.
 */
void setEnablePrePassLightSpecular(const LiveActor* pActor, const char* pName, bool isEnable) {
    PrePassLightBase* light = pActor->mLightKeeper->getLightBase(pName);
    if (light) {
        light->mIsEnableSpecular = isEnable;
    }
}

/**
 * Gets the radius of a prepass point light.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @return The radius, or -1 if the light is not a point light.
 */
f32 getPrePassPointLightRadius(const LiveActor* pActor, const char* pName) {
    PrePassLight<LppPointParam>* light = tryGetPrePassLight<LppPointParam>(pActor, pName);
    if (!light) {
        return -1.0f;
    }

    return light->mParam.mRadius;
}

/**
 * Sets the radius of a prepass point light.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @param radius Light radius.
 */
void setPrePassPointLightRadius(const LiveActor* pActor, const char* pName, f32 radius) {
    PrePassLight<LppPointParam>* light = tryGetPrePassLight<LppPointParam>(pActor, pName);
    if (light) {
        light->mParam.mRadius = radius;
    }
}

/**
 * Gets the shape and pose of a prepass spot light.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @param pDegree Output cone angle, may be nullptr.
 * @param pLength Output length, may be nullptr.
 * @param pPos Output position.
 * @param pDir Output direction, may be nullptr.
 */
void getPrePassSpotLightInfo(const LiveActor* pActor, const char* pName, f32* pDegree, f32* pLength,
                             sead::Vector3f* pPos, sead::Vector3f* pDir) {
    PrePassLight<LppSpotParam>* light = tryGetPrePassLight<LppSpotParam>(pActor, pName);
    if (pDegree) {
        *pDegree = light->mParam.mDegree;
    }

    if (pLength) {
        *pLength = light->mParam.mLength;
    }

    calcLightPoseInfo(light, pPos, pDir);
}

/**
 * Sets the length of a prepass spot light.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @param length Light length.
 */
void setPrePassSpotLightLength(const LiveActor* pActor, const char* pName, f32 length) {
    tryGetPrePassLight<LppSpotParam>(pActor, pName)->mParam.mLength = length;
}

/**
 * Gets the current length of a prepass spot light.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @return The current length.
 */
f32 getPrePassSpotLightCurrentLength(const LiveActor* pActor, const char* pName) {
    return tryGetPrePassLight<LppSpotParam>(pActor, pName)->mParam.mCurrentLength;
}

/**
 * Sets the cone angle of a prepass spot light.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @param degree Cone angle.
 */
void setPrePassSpotLightDegree(const LiveActor* pActor, const char* pName, f32 degree) {
    tryGetPrePassLight<LppSpotParam>(pActor, pName)->mParam.mDegree = degree;
}

/**
 * Checks whether a prepass spot light is blocked by collision.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @return Whether the light strikes a collision.
 */
bool isPrePassSpotLightStrikeCollision(const LiveActor* pActor, const char* pName) {
    return tryGetPrePassLight<LppSpotParam>(pActor, pName)->mParam.mIsStrikeCollision;
}

/**
 * Gets the shape and pose of a prepass projection light.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @param pFovyDegree Output field of view, may be nullptr.
 * @param pLength Output length, may be nullptr.
 * @param pPos Output position.
 * @param pDir Output direction, may be nullptr.
 */
void getPrePassProjLightInfo(const LiveActor* pActor, const char* pName, f32* pFovyDegree,
                             f32* pLength, sead::Vector3f* pPos, sead::Vector3f* pDir) {
    PrePassLight<LppProjParam>* light = tryGetPrePassLight<LppProjParam>(pActor, pName);
    if (pFovyDegree) {
        *pFovyDegree = light->mParam.mFovyDegree;
    }

    if (pLength) {
        *pLength = light->mParam.mFar - light->mParam.mNear;
    }

    calcLightPoseInfo(light, pPos, pDir);
}

/**
 * Gets the shape and pose of a prepass orthographic projection light.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @param pLength Output length, may be nullptr.
 * @param pSize Output size, may be nullptr.
 * @param pPos Output position.
 * @param pDir Output direction, may be nullptr.
 */
void getPrePassProjOrthoLightInfo(const LiveActor* pActor, const char* pName, f32* pLength,
                                  sead::Vector3f* pSize, sead::Vector3f* pPos,
                                  sead::Vector3f* pDir) {
    PrePassLight<LppProjOrthoParam>* light = tryGetPrePassLight<LppProjOrthoParam>(pActor, pName);
    if (pSize) {
        light->mParam.calcSizeXZ(&pSize->x, &pSize->z);
        pSize->y = light->mParam.mFarRate * 100.0f - light->mParam.mNear;
    }

    if (pLength) {
        *pLength = light->mParam.mFarRate * 100.0f - light->mParam.mNear;
    }

    calcLightPoseInfo(light, pPos, pDir);
}

/**
 * Sets the far distance of a prepass projection light.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @param far Far distance.
 */
void setPrePassProjLightFar(const LiveActor* pActor, const char* pName, f32 far) {
    tryGetPrePassLight<LppProjParam>(pActor, pName)->mParam.mFar = far;
}

/**
 * Sets the field of view of a prepass projection light.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @param fovyDegree Field of view.
 */
void setPrePassProjLightFovyDegree(const LiveActor* pActor, const char* pName, f32 fovyDegree) {
    tryGetPrePassLight<LppProjParam>(pActor, pName)->mParam.mFovyDegree = fovyDegree;
}

/**
 * Sets the shadow of a prepass projection light.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @param isEnable Whether the shadow is enabled.
 * @param isSoft Whether the shadow is soft.
 * @param param Shadow parameter.
 */
void setPrePassProjLightShadow(const LiveActor* pActor, const char* pName, bool isEnable,
                               bool isSoft, f32 param) {
    PrePassLight<LppProjParam>* light = tryGetPrePassLight<LppProjParam>(pActor, pName);
    if (isEnable) {
        light->mParam.mShadowType = (param != 0.0f && isSoft) ? 2 : 1;
        light->mParam.mShadowParam = param;
    } else {
        light->mParam.mShadowType = 0;
    }
}

/**
 * Sets the shadow of a prepass orthographic projection light.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @param isEnable Whether the shadow is enabled.
 * @param isSoft Whether the shadow is soft.
 * @param param Shadow parameter.
 */
void setPrePassProjOrthoLightShadow(const LiveActor* pActor, const char* pName, bool isEnable,
                                    bool isSoft, f32 param) {
    PrePassLight<LppProjOrthoParam>* light = tryGetPrePassLight<LppProjOrthoParam>(pActor, pName);
    if (isEnable) {
        light->mParam.mShadowType = (param != 0.0f && isSoft) ? 2 : 1;
        light->mParam.mShadowParam = param;
    } else {
        light->mParam.mShadowType = 0;
    }
}

/**
 * Sets the shadow of a prepass spot light.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @param isEnable Whether the shadow is enabled.
 * @param isSoft Whether the shadow is soft.
 * @param param Shadow parameter.
 */
void setPrePassSpotLightShadow(const LiveActor* pActor, const char* pName, bool isEnable,
                               bool isSoft, f32 param) {
    PrePassLight<LppSpotParam>* light = tryGetPrePassLight<LppSpotParam>(pActor, pName);
    if (isEnable) {
        light->mParam.mShadowType = (param != 0.0f && isSoft) ? 2 : 1;
        light->mParam.mShadowParam = param;
    } else {
        light->mParam.mShadowType = 0;
    }
}

/**
 * Connects a prepass light to a matrix.
 * @param pActor Actor owning the light.
 * @param pName Light name.
 * @param pMtx Matrix to follow.
 */
void initPrePassLightMtxConnector(const LiveActor* pActor, const char* pName,
                                  const sead::Matrix34f* pMtx) {
    PrePassLightBase* light = pActor->mLightKeeper->getLightBase(pName);
    if (light) {
        attachMtxConnectorToMtxPtr(light->mMtxConnector, pMtx);
    }
}

/**
 * Gets the irradiance cube map sampler of the scene.
 * @param pActor Actor of the scene.
 * @return The irradiance sampler.
 */
const agl::TextureSampler* getTexIrradianceObj(const LiveActor* pActor) {
    return static_cast<GraphicsSystemInfo*>(pActor->getSceneInfo()->_78)
        ->mCubeMapDirector->getIrradianceSampler(1);
}

/**
 * Binds the light parameter group to this info.
 */
void ActorPrePassLightKeeper::LightBaseInfo::setPtr() {
    auto& group = np_LightCommon::LightCommon;
    group.readyToSetPtr();
    group.setParamPtr("Name", &mName);
    group.setParamPtr("LppLightType", &mLppLightType);
    group.setParamPtr("LppLightShaderFunc", &mLppLightShaderFunc);
    group.setParamPtr("ActorJointName", &mActorJointName);
    group.setParamPtr("Offset", &mOffset);
    group.setParamPtr("RotateOffset", &mRotateOffset);
    group.setParamPtr("Color", &mColor);
    group.setParamPtr("SpecularColor", &mSpecularColor);
    group.setParamPtr("IsEnableSpecular", &mIsEnableSpecular);
    group.setParamPtr("IsEnableSpecularColor", &mIsEnableSpecularColor);
    group.setParamPtr("KillFrame", &mKillFrame);
    group.setParamPtr("AppearFrame", &mAppearFrame);
    group.setParamPtr("IsIndirectIllumination", &mIsIndirectIllumination);
}
}  // namespace al
