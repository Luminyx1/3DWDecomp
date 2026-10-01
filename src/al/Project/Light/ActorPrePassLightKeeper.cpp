#include "Project/Light/ActorPrePassLightKeeper.hpp"

#include "Library/Light/LppBase.hpp"
#include "Library/Collision/PartsConnectorUtil.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/MacroUtil.hpp"
#include "Project/Base/StringUtil.hpp"

#define AL_YAML_PARAM_GROUP(NAME)                                                                  \
    alYamlMacroUtil::YamlParamGroup NAME =                                                         \
        (alYamlMacroUtil::YamlParamGroup::sCurrent = &NAME, alYamlMacroUtil::YamlParamGroup());
#define AL_YAML_PARAM(TYPE, NAME) alYamlMacroUtil::YamlParam_##TYPE instance_##NAME(#NAME);

namespace np_KeeperInfo {
AL_YAML_PARAM_GROUP(KeeperInfo)
AL_YAML_PARAM(bool, IsIgnoreHideModel)
}  // namespace np_KeeperInfo

namespace np_LightCommon {
AL_YAML_PARAM_GROUP(LightCommon)
AL_YAML_PARAM(YamlString, Name)
AL_YAML_PARAM(YamlString, LppLightType)
AL_YAML_PARAM(YamlString, LppLightShaderFunc)
AL_YAML_PARAM(YamlString, ActorJointName)
AL_YAML_PARAM(V3f, Offset)
AL_YAML_PARAM(V3f, RotateOffset)
AL_YAML_PARAM(YamlColor, Color)
AL_YAML_PARAM(YamlColor, SpecularColor)
AL_YAML_PARAM(bool, IsEnableSpecular)
AL_YAML_PARAM(bool, IsEnableSpecularColor)
AL_YAML_PARAM(s32, KillFrame)
AL_YAML_PARAM(s32, AppearFrame)
AL_YAML_PARAM(bool, IsIndirectIllumination)
}  // namespace np_LightCommon

namespace np_UserColorParam {
AL_YAML_PARAM_GROUP(UserColorParam)
AL_YAML_PARAM(YamlString, Name)
AL_YAML_PARAM(YamlColor, Color)
}  // namespace np_UserColorParam

namespace np_PointLight {
AL_YAML_PARAM_GROUP(PointLight)
AL_YAML_PARAM(f32, Radius)
AL_YAML_PARAM(f32, DampPower)
AL_YAML_PARAM(f32, RandomCeil)
AL_YAML_PARAM(f32, SpecExpansion)
}  // namespace np_PointLight

namespace np_SpotLight {
AL_YAML_PARAM_GROUP(SpotLight)
AL_YAML_PARAM(f32, Degree)
AL_YAML_PARAM(f32, Length)
AL_YAML_PARAM(f32, AngleDamp)
AL_YAML_PARAM(f32, DistDamp)
AL_YAML_PARAM(f32, SpecExpansion)
AL_YAML_PARAM(bool, IsEnableCollisionCheck)
AL_YAML_PARAM(f32, CollisionOffset)
AL_YAML_PARAM(f32, LengthChangeRate)
AL_YAML_PARAM(s32, ShadowType)
AL_YAML_PARAM(f32, Pcf)
}  // namespace np_SpotLight

namespace np_LineLight {
AL_YAML_PARAM_GROUP(LineLight)
AL_YAML_PARAM(f32, Radius)
AL_YAML_PARAM(f32, Length)
AL_YAML_PARAM(f32, SpecExpansion)
}  // namespace np_LineLight

namespace np_ProjOrthoLight {
AL_YAML_PARAM_GROUP(ProjOrthoLight)
AL_YAML_PARAM(f32, Near)
AL_YAML_PARAM(f32, DistDamp)
AL_YAML_PARAM(V3f, Scale)
AL_YAML_PARAM(YamlString, TextureBaseName)
AL_YAML_PARAM(bool, IsEnableTexOption)
AL_YAML_PARAM(V2f, TexMoveSpeed)
AL_YAML_PARAM(V2f, TexScale)
AL_YAML_PARAM(s32, ShadowType)
AL_YAML_PARAM(f32, Pcf)
}  // namespace np_ProjOrthoLight

namespace np_ProjLight {
AL_YAML_PARAM_GROUP(ProjLight)
AL_YAML_PARAM(f32, Near)
AL_YAML_PARAM(f32, Far)
AL_YAML_PARAM(f32, FovyDegree)
AL_YAML_PARAM(f32, Aspect)
AL_YAML_PARAM(f32, DistDamp)
AL_YAML_PARAM(YamlString, TextureBaseName)
AL_YAML_PARAM(bool, IsEnableTexOption)
AL_YAML_PARAM(V2f, TexMoveSpeed)
AL_YAML_PARAM(V2f, TexScale)
AL_YAML_PARAM(s32, ShadowType)
AL_YAML_PARAM(f32, Pcf)
}  // namespace np_ProjLight

#undef AL_YAML_PARAM
#undef AL_YAML_PARAM_GROUP

namespace al {
/**
 * Binds the common light parameter group to this info.
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

/**
 * Binds the point light parameter group to a point light.
 * @param pLight Light to set up.
 */
void ActorPrePassLightKeeper::setupPrePassPointLightParam(LppPoint* pLight) const {
    auto& group = np_PointLight::PointLight;
    group.readyToSetPtr();
    group.setParamPtr("Radius", &pLight->mParam.mRadius);
    group.setParamPtr("DampPower", &pLight->mParam.mDampPower);
    group.setParamPtr("RandomCeil", &pLight->mRandomCeil);
    group.setParamPtr("SpecExpansion", &pLight->mParam.mSpecExpansion);
}

/**
 * Binds the spot light parameter group to a spot light.
 * @param pLight Light to set up.
 */
void ActorPrePassLightKeeper::setupPrePassSpotLightParam(LppSpot* pLight) const {
    auto& group = np_SpotLight::SpotLight;
    group.readyToSetPtr();
    group.setParamPtr("Degree", &pLight->mParam.mDegree);
    group.setParamPtr("Length", &pLight->mParam.mLength);
    group.setParamPtr("AngleDamp", &pLight->mParam.mAngleDamp);
    group.setParamPtr("DistDamp", &pLight->mParam.mDistDamp);
    group.setParamPtr("SpecExpansion", &pLight->mParam.mSpecExpansion);
    group.setParamPtr("IsEnableCollisionCheck", &pLight->mParam.mIsEnableCollisionCheck);
    group.setParamPtr("CollisionOffset", &pLight->mParam.mCollisionOffset);
    group.setParamPtr("LengthChangeRate", &pLight->mParam.mLengthChangeRate);
    group.setParamPtr("ShadowType", &pLight->mParam.mShadowType);
    group.setParamPtr("Pcf", &pLight->mParam.mPcf);
}

/**
 * Binds the line light parameter group to a line light.
 * @param pLight Light to set up.
 */
void ActorPrePassLightKeeper::setupPrePassLineLightParam(LppLine* pLight) const {
    auto& group = np_LineLight::LineLight;
    group.readyToSetPtr();
    group.setParamPtr("Radius", &pLight->mParam.mRadius);
    group.setParamPtr("Length", &pLight->mParam.mLength);
    group.setParamPtr("SpecExpansion", &pLight->mParam.mSpecExpansion);
}

/**
 * Binds the orthographic projection light parameter group to a light.
 * @param pLight Light to set up.
 */
void ActorPrePassLightKeeper::setupPrePassProjOrthoLightParam(LppProjOrtho* pLight) const {
    auto& group = np_ProjOrthoLight::ProjOrthoLight;
    group.readyToSetPtr();
    group.setParamPtr("Near", &pLight->mParam.mNear);
    group.setParamPtr("DistDamp", &pLight->mParam.mDistDamp);
    group.setParamPtr("Scale", &pLight->mParam.mScale);
    group.setParamPtr("TextureBaseName", &pLight->mParam.mTexInfo.mTextureBaseName);
    group.setParamPtr("IsEnableTexOption", &pLight->mParam.mTexInfo.mIsEnableTexOption);
    group.setParamPtr("TexMoveSpeed", &pLight->mParam.mTexInfo.mTexMoveSpeed);
    group.setParamPtr("TexScale", &pLight->mParam.mTexInfo.mTexScale);
    group.setParamPtr("ShadowType", &pLight->mParam.mShadowType);
    group.setParamPtr("Pcf", &pLight->mParam.mPcf);
}

/**
 * Binds the projection light parameter group to a projection light.
 * @param pLight Light to set up.
 */
void ActorPrePassLightKeeper::setupPrePassProjLightParam(LppProj* pLight) const {
    auto& group = np_ProjLight::ProjLight;
    group.readyToSetPtr();
    group.setParamPtr("Near", &pLight->mParam.mNear);
    group.setParamPtr("Far", &pLight->mParam.mFar);
    group.setParamPtr("FovyDegree", &pLight->mParam.mFovyDegree);
    group.setParamPtr("Aspect", &pLight->mParam.mAspect);
    group.setParamPtr("DistDamp", &pLight->mParam.mDistDamp);
    group.setParamPtr("TextureBaseName", &pLight->mParam.mTexInfo.mTextureBaseName);
    group.setParamPtr("IsEnableTexOption", &pLight->mParam.mTexInfo.mIsEnableTexOption);
    group.setParamPtr("TexMoveSpeed", &pLight->mParam.mTexInfo.mTexMoveSpeed);
    group.setParamPtr("TexScale", &pLight->mParam.mTexInfo.mTexScale);
    group.setParamPtr("ShadowType", &pLight->mParam.mShadowType);
    group.setParamPtr("Pcf", &pLight->mParam.mPcf);
}

/**
 * Reads the common light parameters.
 * @param rIter Light parameter iterator.
 */
void ActorPrePassLightKeeper::LightBaseInfo::readIter(const ByamlIter& rIter) {
    setPtr();
    np_LightCommon::LightCommon.readParam(rIter);
}

/**
 * Binds the user color parameter group to this color.
 */
void ActorPrePassLightKeeper::UserColor::setPtr() {
    auto& group = np_UserColorParam::UserColorParam;
    group.readyToSetPtr();
    group.setParamPtr("Name", &mName);
    group.setParamPtr("Color", &mColor);
}

/**
 * Reads the user color parameters.
 * @param rIter User color iterator.
 */
void ActorPrePassLightKeeper::UserColor::readIter(const ByamlIter& rIter) {
    setPtr();
    np_UserColorParam::UserColorParam.readParam(rIter);
}

/**
 * Constructs the prepass light keeper.
 * @param isIgnoreYaml Whether the prepass light settings are ignored.
 */
ActorPrePassLightKeeper::ActorPrePassLightKeeper(bool isIgnoreYaml)
    : mIsIgnorePrePassYaml(isIgnoreYaml) {}

/**
 * Destroys the lights.
 */
ActorPrePassLightKeeper::~ActorPrePassLightKeeper() {
    while (!mLightBaseArray.isEmpty()) {
        PrePassLightBase* light = mLightBaseArray.popBack();

        if (light != nullptr) {
            delete light;
        }
    }

    mLightBaseArray.freeBuffer();
}

/**
 * Creates the lights and user colors described by the actor's prepass light settings.
 * @param pActor Actor owning the lights.
 * @param rInfo Actor init info.
 * @param rIter Prepass light settings.
 * @return Whether the settings contain lights.
 */
bool ActorPrePassLightKeeper::init(LiveActor* pActor, const ActorInitInfo& rInfo,
                                   const ByamlIter& rIter) {
    mParentActor = pActor;

    if (mIsIgnorePrePassYaml) {
        return false;
    }

    auto& keeperInfo = np_KeeperInfo::KeeperInfo;
    keeperInfo.readyToSetPtr();
    keeperInfo.setParamPtr("IsIgnoreHideModel", &mIsIgnoreHideModel);
    keeperInfo.readParam(rIter);

    ByamlIter lightsIter;
    rIter.tryGetIterByKey(&lightsIter, "PrePassLights");

    if (!lightsIter.isValid()) {
        mLightBaseArray.allocBuffer(1, nullptr);
        return false;
    }

    if (!lightsIter.isValid() || !lightsIter.isTypeArray()) {
        mIsIgnorePrePassYaml = true;
        return false;
    }

    s32 lightNum = lightsIter.getSize();
    mLightBaseArray.allocBuffer(lightNum, nullptr);

    for (s32 i = 0; i < lightNum; i++) {
        ByamlIter lightIter;
        lightsIter.tryGetIterByIndex(&lightIter, i);

        LightBaseInfo info;
        info.readIter(lightIter);

        if (info.mName == nullptr || info.mLppLightType == nullptr) {
            continue;
        }

        const char* type = info.mLppLightType;
        PrePassLightBase* light = nullptr;

        if (isEqualString(type, LppLightType::text(LppLightType::無し))) {
            continue;
        }

        if (isEqualString(type, LppLightType::text(LppLightType::点光源))) {
            LppPoint* point = new LppPoint(info.mName);
            mLightBaseArray.pushBack(point);
            setLightBaseInfo(point, info);

            s32 shaderFunc = 0;

            if (info.mLppLightShaderFunc != nullptr &&
                !isEqualString(info.mLppLightShaderFunc,
                               LppLightShaderFunc::text(LppLightShaderFunc::通常))) {
                shaderFunc = isEqualString(info.mLppLightShaderFunc,
                                           LppLightShaderFunc::text(LppLightShaderFunc::裏面));
            }

            point->mLightShaderFunc = shaderFunc;
            point->init(rInfo);
            point->appear();
            setupPrePassPointLightParam(point);
            np_PointLight::PointLight.readParam(lightIter);

            if (point->mParam.mDampPower < 1.0f) {
                point->mParam.mDampPower = 1.0f;
            }

            light = point;
        } else if (isEqualString(type, LppLightType::text(LppLightType::スポットライト))) {
            LppSpot* spot = new LppSpot(info.mName);
            mLightBaseArray.pushBack(spot);
            setLightBaseInfo(spot, info);
            spot->init(rInfo);
            spot->appear();
            setupPrePassSpotLightParam(spot);
            np_SpotLight::SpotLight.readParam(lightIter);
            spot->mParam.mCurrentLength = spot->mParam.mLength;
            light = spot;
        } else if (isEqualString(type, LppLightType::text(LppLightType::投影))) {
            LppProj* proj = new LppProj(info.mName);
            mLightBaseArray.pushBack(proj);
            setLightBaseInfo(proj, info);
            setupPrePassProjLightParam(proj);
            np_ProjLight::ProjLight.readParam(lightIter);
            proj->init(rInfo);
            proj->appear();
            light = proj;
        } else if (isEqualString(type, LppLightType::text(LppLightType::正射影))) {
            LppProjOrtho* projOrtho = new LppProjOrtho(info.mName);
            mLightBaseArray.pushBack(projOrtho);
            setLightBaseInfo(projOrtho, info);
            setupPrePassProjOrthoLightParam(projOrtho);
            np_ProjOrthoLight::ProjOrthoLight.readParam(lightIter);
            projOrtho->init(rInfo);
            projOrtho->appear();
            light = projOrtho;
        } else if (isEqualString(type, LppLightType::text(LppLightType::ラインライト))) {
            LppLine* line = new LppLine(info.mName);
            mLightBaseArray.pushBack(line);
            setLightBaseInfo(line, info);
            line->init(rInfo);
            line->appear();
            setupPrePassLineLightParam(line);
            np_LineLight::LineLight.readParam(lightIter);
            light = line;
        } else {
            continue;
        }

        if (info.mActorJointName != nullptr && isExistJoint(pActor, info.mActorJointName)) {
            attachMtxConnectorToJoint(light->mMtxConnector, pActor, info.mActorJointName);
        } else {
            attachMtxConnectorToActor(light->mMtxConnector, pActor, nullptr);
        }
    }

    ByamlIter colorsIter;
    rIter.tryGetIterByKey(&colorsIter, "UserColors");

    if (!colorsIter.isValid()) {
        mUserColorArray.allocBuffer(1, nullptr);
        return true;
    }

    s32 colorNum = colorsIter.getSize();
    mUserColorArray.allocBuffer(colorNum > 1 ? colorNum : 1, nullptr);

    for (s32 i = 0; i < colorNum; i++) {
        ByamlIter colorIter;
        colorsIter.tryGetIterByIndex(&colorIter, i);

        UserColor* color = new UserColor();
        color->readIter(colorIter);
        mUserColorArray.pushBack(color);
    }

    return true;
}

/**
 * Allocates the light array.
 * @param num Maximum number of lights.
 */
void ActorPrePassLightKeeper::initLightNum(s32 num) {
    mLightBaseArray.allocBuffer(num, nullptr);
}

/**
 * Applies the placement info of a light.
 * @param pLight Light to set up.
 * @param rInfo Light info.
 */
void ActorPrePassLightKeeper::setLightBaseInfo(PrePassLightBase* pLight, const LightBaseInfo& rInfo) {
    static_cast<sead::BaseVec3<f32>&>(pLight->mOffset) = rInfo.mOffset;
    static_cast<sead::BaseVec3<f32>&>(pLight->mRotateOffsetDegree) = rInfo.mRotateOffset;
    pLight->mColor = rInfo.mColor;
    pLight->mSpecularColor = rInfo.mSpecularColor;
    pLight->mIsEnableSpecularColor = rInfo.mIsEnableSpecularColor;
    pLight->_90 = rInfo.mIsEnableSpecular;
    pLight->mIsEnableSpecular = rInfo.mIsEnableSpecular;
    pLight->mKillFrame = rInfo.mKillFrame;
    pLight->mAppearFrame = rInfo.mAppearFrame;
    pLight->mIsIndirectIllumination = rInfo.mIsIndirectIllumination;
}

/**
 * Kills the lights of dead actors.
 */
void ActorPrePassLightKeeper::initAfterPlacement() {
    if (!isDead(mParentActor)) {
        return;
    }

    for (PrePassLightBase& light : mLightBaseArray) {
        light.requestKillDirect();
    }
}

/**
 * Makes the lights appear.
 * @param isHideModel Whether the actor model is hidden.
 */
void ActorPrePassLightKeeper::appear(bool isHideModel) {
    if (mIsIgnorePrePassYaml) {
        return;
    }

    if (!mIsIgnoreHideModel && isHideModel) {
        return;
    }

    for (PrePassLightBase& light : mLightBaseArray) {
        light.appear();
    }
}

/**
 * Kills the lights.
 */
void ActorPrePassLightKeeper::requestKill() {
    if (mIsIgnorePrePassYaml) {
        return;
    }

    for (PrePassLightBase& light : mLightBaseArray) {
        light.requestKill();
    }
}

/**
 * Kills the lights when the actor model gets hidden.
 */
void ActorPrePassLightKeeper::hideModel() {
    if (mIsIgnorePrePassYaml || mIsIgnoreHideModel) {
        return;
    }

    for (PrePassLightBase& light : mLightBaseArray) {
        light.requestKill();
    }
}

/**
 * Updates the lights for a hidden model.
 * @param isHide Whether the model is hidden.
 */
void ActorPrePassLightKeeper::updateHideModel(bool isHide) {}

/**
 * Finds a light by name.
 * @param pName Light name.
 * @return The light, or nullptr.
 */
PrePassLightBase* ActorPrePassLightKeeper::getLightBase(const char* pName) const {
    if (mIsIgnorePrePassYaml) {
        return nullptr;
    }

    s32 num = mLightBaseArray.size();

    for (s32 i = 0; i < num; i++) {
        PrePassLightBase* light = mLightBaseArray[i];

        if (isEqualString(light->mName, pName)) {
            return light;
        }
    }

    return nullptr;
}

/**
 * Gets a light by index.
 * @param index Light index.
 * @return The light, or nullptr.
 */
PrePassLightBase* ActorPrePassLightKeeper::getLightBase(s32 index) const {
    if (mIsIgnorePrePassYaml) {
        return nullptr;
    }

    return mLightBaseArray[index];
}

/**
 * Finds a user color by name.
 * @param pName Color name.
 * @return The color, or black.
 */
const sead::Color4f& ActorPrePassLightKeeper::findUserColor(const char* pName) const {
    s32 num = mUserColorArray.size();

    for (s32 i = 0; i < num; i++) {
        UserColor* color = mUserColorArray[i];

        if (isEqualString(pName, color->mName)) {
            return color->mColor;
        }
    }

    return sead::Color4f::cBlack;
}
}  // namespace al
