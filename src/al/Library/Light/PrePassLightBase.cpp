#include "Library/Light/PrePassLightBase.hpp"

#include <random/seadGlobalRandom.h>

#include "common/aglTextureSampler.h"
#include "shadow/aglDepthShadow.h"

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Collision/PartsConnectorUtil.hpp"
#include "Library/Connector/MtxConnector.hpp"
#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Light/DirectionalLightKeeper.hpp"
#include "Library/Light/LightIntensityDirector.hpp"
#include "Library/Light/PrePassLightKeeper.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Shadow/ShadowDirector.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Texture/TextureUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Collision/CollisionUtil.hpp"

namespace {
using namespace al;

NERVE_DECL(PrePassLightBase, Wait)
NERVE_DECL(PrePassLightBase, Appear)
NERVE_DECL(PrePassLightBase, Kill)
NERVE_DECL(PrePassLightBase, Dead)

NERVES_MAKE_NOSTRUCT(PrePassLightBase, Wait, Appear, Kill, Dead)

PrePassLightKeeper* getKeeper(const PrePassLightBase* pLight) {
    return pLight->getGraphicsSystemInfo()->getPrePassLightKeeper();
}

f32 calcScaleAverage(const sead::Vector3f& rScale) {
    return (rScale.x + rScale.y + rScale.z) / 3.0f;
}

void requestAppearCore(PrePassLightBase* pLight, s32 step) {
    if (isNerve(pLight, &NrvPrePassLightBaseAppear)) {
        if (pLight->mAppearStep > step) {
            pLight->mAppearStep = step;
        }

        return;
    }

    pLight->mIsKillByUser = false;

    if (step < 0) {
        step = pLight->mAppearFrame;
    }

    pLight->mAppearStep = step;

    if (step >= 1) {
        pLight->mCurrentColorStart = sead::Color4f::cBlack;
        pLight->mCurrentColor = sead::Color4f::cBlack;
        setNerve(pLight, &NrvPrePassLightBaseAppear);
    } else {
        setNerve(pLight, &NrvPrePassLightBaseWait);
    }

    if (!pLight->isActive()) {
        getKeeper(pLight)->pushBackLight(pLight);
    }
}

void requestPointLight(const LppPointParam* pParam, PrePassLightBase* pLight,
                       const sead::Vector3f* pPos, f32 radius) {
    PrePassLightKeeper* keeper = getKeeper(pLight);
    sead::Vector3f trans;
    sead::Vector3f scale;
    pLight->mMtxConnector->calcConnectInfo(&trans, nullptr, &scale, pLight->mOffset,
                                           sead::Vector3f::zero);
    f32 rate = pLight->calcRandomRate(keeper->isPaused());
    f32 scaleAverage = calcScaleAverage(scale);
    sead::Color4f color;
    pLight->calcColor(&color, rate);
    keeper->requestPointLight(pPos != nullptr ? *pPos : trans, rate * (scaleAverage * radius),
                              color, pParam->mDampPower, pParam->mSpecExpansion,
                              pLight->mIsEnableSpecular, pLight->mIsEnableSpecularColor,
                              pLight->mSpecularColor, pLight->mLightShaderFunc);
    pLight->resetRequest();
}
}  // namespace

/**
 * Sets the clipping info of a light placement actor.
 * @param pActor Placement actor.
 * @param radius Clipping radius.
 * @param pPos Clipping position.
 */
void PrePassLightPlacementFuncImpl::setClippingInfoImpl(al::LiveActor* pActor, f32 radius,
                                                        const sead::Vector3f* pPos) {
    al::setClippingInfo(pActor, radius, pPos);
}

/**
 * Makes the SRT matrix of a light placement actor.
 * @param pMtx Output matrix.
 * @param pActor Placement actor.
 */
void PrePassLightPlacementFuncImpl::makeMtxSRT(sead::Matrix34f* pMtx, const al::LiveActor* pActor) {
    al::makeMtxSRT(pMtx, pActor);
}

namespace al {

/**
 * Constructs a prepass light.
 * @param pName Light name.
 */
PrePassLightBase::PrePassLightBase(const char* pName)
    : NerveExecutor(pName), sead::TListNode<PrePassLightBase*>(this), mName(pName),
      mMtxConnector(new MtxConnector()), mRandom(sead::GlobalRandom::instance()) {
    initNerve(&NrvPrePassLightBaseWait, 0);
}

/**
 * Reads the placement parameters of the light.
 * @param rInfo Actor init info.
 */
void PrePassLightBase::init(const ActorInitInfo& rInfo) {
    mGraphicsSystemInfo = rInfo.getActorSceneInfo().graphicsSystemInfo;
    tryGetArg(&mColor.r, rInfo, "ColorRed");
    tryGetArg(&mColor.g, rInfo, "ColorGreen");
    tryGetArg(&mColor.b, rInfo, "ColorBlue");
    tryGetArg(&mIsEnableSpecular, rInfo, "IsEnableSpecular");
    tryGetArg(&mSpecularColor.r, rInfo, "SpecularColorRed");
    tryGetArg(&mSpecularColor.g, rInfo, "SpecularColorGreen");
    tryGetArg(&mSpecularColor.b, rInfo, "SpecularColorBlue");
    tryGetArg(&mIsEnableSpecularColor, rInfo, "IsEnableSpecularColor");
    tryGetArg(&mIsIndirectIllumination, rInfo, "IsIndirectIllumination");
    tryGetArg(&mRandomCeil, rInfo, "RandomCeil");
    tryGetArg(&mLightShaderFunc, rInfo, "LightShaderFunc");
    tryGetArg(&mAppearFrame, rInfo, "AppearFrame");
    tryGetArg(&mKillFrame, rInfo, "KillFrame");
    tryGetArg(&mOffset.x, rInfo, "OffsetX");
    tryGetArg(&mOffset.y, rInfo, "OffsetY");
    tryGetArg(&mOffset.z, rInfo, "OffsetZ");
    mCollisionDirector = rInfo.getActorSceneInfo().collisionDirector;
}

/**
 * Reads the placement parameters of a point light.
 * @param rInfo Actor init info.
 */
void LppPointParam::initByInfo(const ActorInitInfo& rInfo) {
    tryGetArg(&mRadius, rInfo, "Radius");

    if (!tryGetArg(&mDampPower, rInfo, "PointLightDampPower")) {
        tryGetArg(&mDampPower, rInfo, "DampPower");
    }

    tryGetArg(&mSpecExpansion, rInfo, "SpecularExpansion");
}

/**
 * Reads the placement parameters of a line light.
 * @param rInfo Actor init info.
 */
void LppLineParam::initByInfo(const ActorInitInfo& rInfo) {
    tryGetArg(&mRadius, rInfo, "Radius");
    tryGetArg(&mLength, rInfo, "LineLightLength");
}

/**
 * Reads the placement parameters of a spot light.
 * @param rInfo Actor init info.
 */
void LppSpotParam::initByInfo(const ActorInitInfo& rInfo) {
    tryGetArg(&mDegree, rInfo, "SpotLightDegree");
    tryGetArg(&mLength, rInfo, "SpotLightLength");
    tryGetArg(&mAngleDamp, rInfo, "SpotLightAngleDamp");

    if (!tryGetArg(&mDistDamp, rInfo, "LightDistDamp")) {
        mDistDamp = 1.0f;
    }

    tryGetArg(&mIsEnableCollisionCheck, rInfo, "IsEnableCollisionCheck");
    tryGetArg(&mCollisionOffset, rInfo, "AfterCollisionCheckOffset");
    tryGetArg(&mLengthChangeRate, rInfo, "LengthChangeRate");
    tryGetArg(&mShadowType, rInfo, "ShadowType");
    tryGetArg(&mPcf, rInfo, "Pcf");

    if (mAngleDamp <= 0.0f) {
        mAngleDamp = 0.01f;
    }

    mCurrentLength = mLength;
}

/**
 * Reads the placement parameters of an orthographic projection light.
 * @param rInfo Actor init info.
 */
void LppProjOrthoParam::initByInfo(const ActorInitInfo& rInfo) {
    tryGetArg(&mNear, rInfo, "ProjLightNear");
    tryGetArg(&mDistDamp, rInfo, "LightDistDamp");
    tryGetArg(&mShadowType, rInfo, "ShadowType");
    tryGetArg(&mPcf, rInfo, "Pcf");
    tryGetArg(&mIsUseParentYRotation, rInfo, "UseParentYRotation");
    mTexInfo.initByInfo(rInfo);
}

/**
 * Reads the projection texture parameters and loads the texture.
 * @param rInfo Actor init info.
 */
void LppTexInfo::initByInfo(const ActorInitInfo& rInfo) {
    bool isEnableTexOption = mIsEnableTexOption;
    isEnableTexOption |= tryGetArg(&mTexMoveSpeed.x, rInfo, "TexSpeedX");
    mIsEnableTexOption = isEnableTexOption;
    isEnableTexOption |= tryGetArg(&mTexMoveSpeed.y, rInfo, "TexSpeedY");
    mIsEnableTexOption = isEnableTexOption;
    isEnableTexOption |= tryGetArg(&mTexScale.x, rInfo, "TexScaleX");
    mIsEnableTexOption = isEnableTexOption;
    isEnableTexOption |= tryGetArg(&mTexScale.y, rInfo, "TexScaleY");
    mIsEnableTexOption = isEnableTexOption;
    mTexMoveSpeedFrame = mTexMoveSpeed * 0.01f;

    if (mTextureBaseName != nullptr) {
        if (isEqualString(mTextureBaseName, "None")) {
            return;
        }
    } else {
        const char* textureBaseName = nullptr;
        tryGetStringArg(&textureBaseName, rInfo, "ProjTex");

        if (textureBaseName == nullptr) {
            mTextureBaseName = "None";
            return;
        }

        if (isEqualString(textureBaseName, "None")) {
            return;
        }

        mTextureBaseName = textureBaseName;
    }

    StringTmp<128> textureName("ProjTex%s", mTextureBaseName);
    StringTmp<128> archiveName("ObjectData/%s", textureName.cstr());
    StringTmp<128> resourceName("ProjTex%s", mTextureBaseName);
    makeTextureDataFromArchive(&mTextureData, archiveName.cstr(), resourceName.cstr(),
                               textureName.cstr());
    mSampler = new agl::TextureSampler(mTextureData);
    mSampler->setWrap(1, 1, 1);
}

/**
 * Calculates the horizontal size of the projection.
 * @param pSizeX Output size along X.
 * @param pSizeZ Output size along Z.
 */
void LppProjOrthoParam::calcSizeXZ(f32* pSizeX, f32* pSizeZ) const {
    if (pSizeX != nullptr) {
        *pSizeX = mScale.x * 50.0f;
    }

    if (pSizeZ != nullptr) {
        *pSizeZ = mScale.z * 50.0f;
    }
}

/**
 * Reads the placement parameters of a projection light.
 * @param rInfo Actor init info.
 */
void LppProjParam::initByInfo(const ActorInitInfo& rInfo) {
    tryGetArg(&mNear, rInfo, "ProjLightNear");
    tryGetArg(&mFar, rInfo, "ProjLightFar");
    tryGetArg(&mDistDamp, rInfo, "LightDistDamp");
    tryGetArg(&mFovyDegree, rInfo, "ProjLightFovyDegree");
    tryGetArg(&mAspect, rInfo, "ProjLightAspect");
    tryGetArg(&mShadowType, rInfo, "ShadowType");
    tryGetArg(&mPcf, rInfo, "Pcf");
    mTexInfo.initByInfo(rInfo);
}

/**
 * Scrolls the projection texture.
 */
void LppTexInfo::updateTexOffset() {
    mTexOffset += mTexMoveSpeedFrame;
    mTexOffset.x = wrapValue(mTexOffset.x, mTexScale.x);
    mTexOffset.y = wrapValue(mTexOffset.y, mTexScale.y);
}

/**
 * Destroys the projection texture.
 */
LppTexInfo::~LppTexInfo() {
    if (mSampler != nullptr) {
        delete mSampler;
        mSampler = nullptr;
    }
}

}  // namespace al

/**
 * Initializes a light placement actor.
 * @param pActor Placement actor.
 * @param pLight Light of the actor.
 * @param pMtx Matrix the light follows.
 * @param rInfo Actor init info.
 */
void LppFunction::initLppActor(al::LiveActor* pActor, al::PrePassLightBase* pLight,
                               sead::Matrix34f* pMtx, const al::ActorInitInfo& rInfo) {
    al::initExecutorUpdate(pActor, rInfo, "グラフィックス要求者");
    al::initActorSceneInfo(pActor, rInfo);
    al::initActorPoseTRSV(pActor);
    al::initActorSRT(pActor, rInfo);
    al::initActorClipping(pActor, rInfo);
    al::initStageSwitch(pActor, rInfo);
    al::trySyncStageSwitchAppear(pActor);
    al::attachMtxConnectorToMtxPtr(pLight->mMtxConnector, pMtx);
}

/**
 * Reinitializes the rotation of a light placement actor around the parent's Y axis.
 * @param pActor Placement actor.
 * @param rInfo Actor init info.
 */
void LppFunction::recalculateRotation(al::LiveActor* pActor, const al::ActorInitInfo& rInfo) {
    al::initActorSRT_ParentY(pActor, rInfo);
}

namespace al {

/**
 * Makes the light appear.
 */
void PrePassLightBase::appear() {
    if (mIsKillByUser) {
        return;
    }

    requestAppearCore(this, mAppearFrame);
}

/**
 * Makes the light appear on user request.
 * @param step Fade in frames, or a negative value for the default.
 */
void PrePassLightBase::requestAppearByUser(s32 step) {
    requestAppearCore(this, step);
}

/**
 * Kills the light on user request.
 * @param step Fade out frames, or a negative value for the default.
 */
void PrePassLightBase::requestKillByUser(s32 step) {
    requestKillCore(step);
    mIsKillByUser = true;
}

/**
 * Starts fading out the light.
 * @param step Fade out frames, or a negative value for the default.
 */
void PrePassLightBase::requestKillCore(s32 step) {
    mKillStep = step < 0 ? mKillFrame : step;

    if (!isActive()) {
        return;
    }

    if (isNerve(this, &NrvPrePassLightBaseKill)) {
        if (mKillStep > step) {
            mKillStep = step;
        }

        return;
    }

    if (mKillStep >= 1) {
        const sead::Color4f& color = getColor();
        mCurrentColorStart = color;
        mCurrentColor = color;
        setNerve(this, &NrvPrePassLightBaseKill);
    } else {
        getKeeper(this)->eraseLight(this);
        setNerve(this, &NrvPrePassLightBaseDead);
    }
}

/**
 * Starts fading out the light with the default frames.
 */
void PrePassLightBase::requestKill() {
    requestKillCore(mKillFrame);
}

/**
 * Kills the light.
 */
void PrePassLightBase::requestKillDirect() {
    requestKillCore(-1);
}

/**
 * Gets the current light color.
 * @return The fading color, the user color or the placement color.
 */
const sead::Color4f& PrePassLightBase::getColor() const {
    if (isNerve(this, &NrvPrePassLightBaseKill) || isNerve(this, &NrvPrePassLightBaseAppear)) {
        return mCurrentColor;
    }

    return mIsUserColor ? mUserColor : mColor;
}

/**
 * Overrides the light color for one frame.
 * @param rColor Color.
 */
void PrePassLightBase::requestUserColor(const sead::Color4f& rColor) {
    mIsUserColor = true;
    mUserColor = rColor;
}

/**
 * Overrides the specular color for one frame.
 * @param rColor Color.
 */
void PrePassLightBase::requestUserSpecularColor(const sead::Color4f& rColor) {
    mIsUserSpecularColor = true;
    mUserSpecularColor = rColor;
}

/**
 * Calculates the requested light color.
 * @param pColor Output color.
 * @param rate Intensity rate.
 */
void PrePassLightBase::calcColor(sead::Color4f* pColor, f32 rate) const {
    *pColor = getColor();
    *pColor *= rate;

    if (!mIsIndirectIllumination) {
        return;
    }

    LightIntensityDirector* director = mGraphicsSystemInfo->getLightIntensityDirector();
    sead::Color4f dirColor = mGraphicsSystemInfo->getDirectionalLightKeeper()->getCurrentColor();
    dirColor.r = dirColor.r > 1.0f ? dirColor.r : 1.0f;
    dirColor.g = dirColor.g > 1.0f ? dirColor.g : 1.0f;
    dirColor.b = dirColor.b > 1.0f ? dirColor.b : 1.0f;

    if ((dirColor.r > 1.0f || dirColor.g > 1.0f || dirColor.b > 1.0f) &&
        director->getExposure() >= 1.0f) {
        return;
    }

    *pColor *= dirColor;
}

/**
 * Calculates the connected pose of the light.
 * @param pTrans Output position.
 * @param pQuat Output rotation.
 * @param pScale Output scale.
 */
void PrePassLightBase::calcConnectorInfo(sead::Vector3f* pTrans, sead::Quatf* pQuat,
                                         sead::Vector3f* pScale) const {
    mMtxConnector->calcConnectInfo(pTrans, pQuat, pScale, mOffset, calcRotateOffsetRadian());
}

/**
 * Updates the nerve and clears the per-frame color requests.
 */
void PrePassLightBase::resetRequest() {
    updateNerve();
    mIsUserColor = false;
    mIsUserSpecularColor = false;
}

/**
 * Fades in the light.
 */
void PrePassLightBase::exeAppear() {
    const sead::Color4f& color = mColor;
    s32 step = mAppearStep;
    f32 rate = calcNerveRate(this, step);
    mCurrentColor.setLerp(mCurrentColorStart, color, rate);

    if (isGreaterStep(this, step)) {
        setNerve(this, &NrvPrePassLightBaseWait);
    }
}

/**
 * Keeps the light on.
 */
void PrePassLightBase::exeWait() {}

/**
 * Fades out the light.
 */
void PrePassLightBase::exeKill() {
    s32 step = mKillStep;
    f32 rate = calcNerveRate(this, step);
    mCurrentColor.setLerp(mCurrentColorStart, sead::Color4f::cBlack, rate);

    if (isGreaterStep(this, step)) {
        getKeeper(this)->eraseLight(this);
        setNerve(this, &NrvPrePassLightBaseDead);
    }
}

/**
 * Keeps the light off.
 */
void PrePassLightBase::exeDead() {}

/**
 * Updates the random flicker rate.
 * @param isPaused Whether the scene is paused.
 * @return The flicker rate.
 */
f32 PrePassLightBase::calcRandomRate(bool isPaused) {
    if (isPaused) {
        return mRandomRate;
    }

    if (mRandomCeil > 0.0f) {
        mRandomRate = 1.0f - mRandomCeil * mRandom->getF32();
    } else {
        mRandomRate = 1.0f;
    }

    return mRandomRate;
}

/**
 * Does nothing; only some light types set up shadows.
 * @param view View index.
 * @param pKeeper Prepass light keeper.
 * @param pDirector Shadow director.
 */
void PrePassLightBase::trySetupShadow(s32 view, PrePassLightKeeper* pKeeper,
                                      ShadowDirector* pDirector) {}

}  // namespace al

namespace LppFunction {

/**
 * Declares point lights to the light keeper.
 * @param rParam Light parameters.
 * @param rLight Light.
 * @param num Number of lights.
 */
template <>
void declareLpp<al::LppPointParam>(const al::LppPointParam& rParam,
                                   const al::PrePassLightBase& rLight, s32 num) {
    if (rLight.mLightShaderFunc == al::LppLightShaderFunc::通常 ||
        rLight.mLightShaderFunc == al::LppLightShaderFunc::裏面) {
        getKeeper(&rLight)->mPointLightNum += num;
    }
}

/**
 * Declares line lights to the light keeper.
 * @param rParam Light parameters.
 * @param rLight Light.
 * @param num Number of lights.
 */
template <>
void declareLpp<al::LppLineParam>(const al::LppLineParam& rParam,
                                  const al::PrePassLightBase& rLight, s32 num) {
    getKeeper(&rLight)->mLineLightNum += num;
}

/**
 * Declares spot lights to the light keeper.
 * @param rParam Light parameters.
 * @param rLight Light.
 * @param num Number of lights.
 */
template <>
void declareLpp<al::LppSpotParam>(const al::LppSpotParam& rParam,
                                  const al::PrePassLightBase& rLight, s32 num) {
    getKeeper(&rLight)->mSpotLightNum += num;
}

/**
 * Declares projection lights to the light keeper.
 * @param rParam Light parameters.
 * @param rLight Light.
 * @param num Number of lights.
 */
template <>
void declareLpp<al::LppProjParam>(const al::LppProjParam& rParam,
                                  const al::PrePassLightBase& rLight, s32 num) {
    getKeeper(&rLight)->mProjLightNum += num;
}

/**
 * Declares orthographic projection lights to the light keeper.
 * @param rParam Light parameters.
 * @param rLight Light.
 * @param num Number of lights.
 */
template <>
void declareLpp<al::LppProjOrthoParam>(const al::LppProjOrthoParam& rParam,
                                       const al::PrePassLightBase& rLight, s32 num) {
    getKeeper(&rLight)->mProjLightNum += num;
}

/**
 * Requests a point light at the light's position.
 * @param pParam Light parameters.
 * @param pLight Light.
 */
template <>
void requestLpp<al::LppPointParam>(al::LppPointParam* pParam, al::PrePassLightBase* pLight) {
    requestPointLight(pParam, pLight, nullptr, pParam->mRadius);
}

/**
 * Requests a point light at a given position.
 * @param pParam Light parameters.
 * @param pLight Light.
 * @param rPos Light position.
 */
void requestLppPoint(al::LppPointParam* pParam, al::PrePassLightBase* pLight,
                     const sead::Vector3f& rPos) {
    requestPointLight(pParam, pLight, &rPos, pParam->mRadius);
}

/**
 * Requests a line light.
 * @param pParam Light parameters.
 * @param pLight Light.
 */
template <>
void requestLpp<al::LppLineParam>(al::LppLineParam* pParam, al::PrePassLightBase* pLight) {
    al::PrePassLightKeeper* keeper = getKeeper(pLight);
    sead::Vector3f trans;
    sead::Vector3f scale;
    sead::Quatf quat;
    pLight->calcConnectorInfo(&trans, &quat, &scale);
    f32 rate = pLight->calcRandomRate(keeper->isPaused());
    f32 scaleAverage = calcScaleAverage(scale);
    sead::Vector3f front;
    al::calcQuatFront(&front, quat);
    sead::Vector3f begin;
    sead::Vector3f end;
    begin.setScaleAdd(pParam->mLength * -0.5f, front, trans);
    end.setScaleAdd(pParam->mLength * 0.5f, front, trans);
    sead::Color4f color;
    pLight->calcColor(&color, rate);
    keeper->requestLineLight(begin, end, rate * (scaleAverage * pParam->mRadius), color,
                             pParam->mSpecExpansion, pLight->mIsEnableSpecular);
    pLight->resetRequest();
}

/**
 * Requests a spot light.
 * @param pParam Light parameters.
 * @param pLight Light.
 */
template <>
void requestLpp<al::LppSpotParam>(al::LppSpotParam* pParam, al::PrePassLightBase* pLight) {
    al::PrePassLightKeeper* keeper = getKeeper(pLight);
    sead::Vector3f trans;
    sead::Vector3f scale;
    sead::Quatf quat;
    pLight->calcConnectorInfo(&trans, &quat, &scale);
    f32 rate = pLight->calcRandomRate(keeper->isPaused());
    f32 scaleAverage = calcScaleAverage(scale);
    sead::Vector3f up;
    al::calcQuatUp(&up, quat);
    sead::Color4f color;
    pLight->calcColor(&color, rate);
    sead::Color4f specularColor = pLight->mSpecularColor * rate;
    f32 length = scaleAverage * pParam->mLength;

    if (pParam->mIsEnableCollisionCheck) {
        sead::Vector3f hitPos;
        if (alCollisionUtil::getFirstPolyOnArrow(pLight, &hitPos, nullptr, trans, -(up * length),
                                                 nullptr, nullptr)) {
            f32 hitLength = (trans - hitPos).length() + pParam->mCollisionOffset;
            pParam->mHitPos.set(hitPos);
            pParam->mIsHitCollision = true;

            if (hitLength < length) {
                length = hitLength;
            }
        } else {
            pParam->mIsHitCollision = false;
        }
    }

    pParam->mCurrentLength += (length - pParam->mCurrentLength) * pParam->mLengthChangeRate;
    sead::Vector3f dir = -up;
    pParam->mShadowIndex = keeper->requestSpotLight(
        trans, dir, pParam->mDegree * sead::Mathf::deg2rad(1.0f), rate * pParam->mCurrentLength,
        color, pParam->mAngleDamp, pParam->mDistDamp, pParam->mSpecExpansion,
        pLight->mIsEnableSpecular, pLight->mIsEnableSpecularColor, specularColor);
    pLight->resetRequest();
}

/**
 * Requests a projection light.
 * @param pParam Light parameters.
 * @param pLight Light.
 */
template <>
void requestLpp<al::LppProjParam>(al::LppProjParam* pParam, al::PrePassLightBase* pLight) {
    al::PrePassLightKeeper* keeper = getKeeper(pLight);
    sead::Vector3f trans;
    sead::Vector3f up;
    sead::Vector3f front;
    sead::Quatf quat;
    pLight->calcConnectorInfo(&trans, &quat, nullptr);
    f32 rate = pLight->calcRandomRate(keeper->isPaused());
    al::calcQuatUp(&up, quat);
    al::calcQuatFront(&front, quat);
    sead::Color4f color;
    pLight->calcColor(&color, rate);
    sead::Color4f specularColor = pLight->mSpecularColor * rate;
    al::LppTexInfo& texInfo = pParam->mTexInfo;
    pParam->mShadowIndex = keeper->requestProjLight(
        trans, -up, -front, color, pParam->mNear, rate * pParam->mFar,
        pParam->mFovyDegree * sead::Mathf::deg2rad(1.0f), rate * pParam->mAspect,
        pParam->mDistDamp, pLight->mIsEnableSpecular, pLight->mIsEnableSpecularColor,
        specularColor, texInfo.mSampler, texInfo.mIsEnableTexOption, texInfo.mTexOffset,
        texInfo.mTexScale);

    if (!keeper->isPaused()) {
        texInfo.updateTexOffset();
    }

    pLight->resetRequest();
}

/**
 * Requests an orthographic projection light.
 * @param pParam Light parameters.
 * @param pLight Light.
 */
template <>
void requestLpp<al::LppProjOrthoParam>(al::LppProjOrthoParam* pParam,
                                       al::PrePassLightBase* pLight) {
    al::PrePassLightKeeper* keeper = getKeeper(pLight);
    sead::Vector3f trans;
    sead::Vector3f up;
    sead::Vector3f front;
    sead::Vector3f scale;
    sead::Quatf quat;
    pLight->calcConnectorInfo(&trans, &quat, &scale);
    f32 rate = pLight->calcRandomRate(keeper->isPaused());
    al::calcQuatUp(&up, quat);
    al::calcQuatFront(&front, quat);
    sead::Color4f color;
    pLight->calcColor(&color, rate);
    sead::Color4f specularColor = pLight->mSpecularColor * rate;
    scale.x = pParam->mScale.x * scale.x;
    scale.y = pParam->mScale.y * scale.y;
    scale.z = pParam->mScale.z * scale.z;
    al::LppTexInfo& texInfo = pParam->mTexInfo;
    pParam->mShadowIndex = keeper->requestProjLightOrtho(
        trans, -up, -front, color, pParam->mNear, scale.y * 100.0f, scale.z * 50.0f, -(scale.z * 50.0f),
        -(scale.x * 50.0f), scale.x * 50.0f,
        pParam->mDistDamp, pLight->mIsEnableSpecular, pLight->mIsEnableSpecularColor,
        specularColor, texInfo.mSampler, texInfo.mIsEnableTexOption, texInfo.mTexOffset,
        texInfo.mTexScale);

    if (!keeper->isPaused()) {
        texInfo.updateTexOffset();
    }

    pLight->resetRequest();
}

/**
 * Calculates the clipping sphere of a point light.
 * @param pParam Light parameters.
 * @param pLight Light.
 * @param pPos Output center.
 * @param pRadius Output radius.
 */
template <>
void calcClippingInfoLpp<al::LppPointParam>(al::LppPointParam* pParam, al::PrePassLightBase* pLight,
                                            sead::Vector3f* pPos, f32* pRadius) {
    sead::Vector3f scale;
    pLight->calcConnectorInfo(pPos, nullptr, &scale);

    if (pRadius != nullptr) {
        f32 scaleAverage = calcScaleAverage(scale);
        *pRadius = pParam->mRadius * scaleAverage;
    }
}

/**
 * Calculates the clipping sphere of a line light.
 * @param pParam Light parameters.
 * @param pLight Light.
 * @param pPos Output center.
 * @param pRadius Output radius.
 */
template <>
void calcClippingInfoLpp<al::LppLineParam>(al::LppLineParam* pParam, al::PrePassLightBase* pLight,
                                           sead::Vector3f* pPos, f32* pRadius) {
    sead::Vector3f scale;
    pLight->calcConnectorInfo(pPos, nullptr, &scale);

    if (pRadius != nullptr) {
        f32 scaleAverage = calcScaleAverage(scale);
        *pRadius = pParam->mRadius * scaleAverage + pParam->mLength * 0.5f;
    }
}

/**
 * Calculates the clipping sphere of a spot light.
 * @param pParam Light parameters.
 * @param pLight Light.
 * @param pPos Output center.
 * @param pRadius Output radius.
 */
template <>
void calcClippingInfoLpp<al::LppSpotParam>(al::LppSpotParam* pParam, al::PrePassLightBase* pLight,
                                           sead::Vector3f* pPos, f32* pRadius) {
    sead::Vector3f scale;
    pLight->calcConnectorInfo(pPos, nullptr, &scale);

    if (pRadius != nullptr) {
        f32 scaleAverage = calcScaleAverage(scale);
        *pRadius = pParam->mLength * scaleAverage;
    }
}

/**
 * Calculates the clipping sphere of a projection light.
 * @param pParam Light parameters.
 * @param pLight Light.
 * @param pPos Output center.
 * @param pRadius Output radius.
 */
template <>
void calcClippingInfoLpp<al::LppProjParam>(al::LppProjParam* pParam, al::PrePassLightBase* pLight,
                                           sead::Vector3f* pPos, f32* pRadius) {
    sead::Vector3f scale;
    pLight->calcConnectorInfo(pPos, nullptr, &scale);

    if (pRadius != nullptr) {
        *pRadius = 10000.0f;
    }
}

/**
 * Calculates the clipping sphere of an orthographic projection light.
 * @param pParam Light parameters.
 * @param pLight Light.
 * @param pPos Output center.
 * @param pRadius Output radius.
 */
template <>
void calcClippingInfoLpp<al::LppProjOrthoParam>(al::LppProjOrthoParam* pParam,
                                                al::PrePassLightBase* pLight, sead::Vector3f* pPos,
                                                f32* pRadius) {
    sead::Vector3f scale;
    pLight->calcConnectorInfo(pPos, nullptr, &scale);

    if (pRadius != nullptr) {
        *pRadius = 10000.0f;
    }
}

/**
 * Does nothing; point lights have no shadow.
 * @param pKeeper Prepass light keeper.
 * @param pDirector Shadow director.
 * @param pParam Light parameters.
 * @param view View index.
 */
template <>
void trySetupShadowLpp<al::LppPointParam>(al::PrePassLightKeeper* pKeeper,
                                          al::ShadowDirector* pDirector,
                                          al::LppPointParam* pParam, s32 view) {}

/**
 * Does nothing; line lights have no shadow.
 * @param pKeeper Prepass light keeper.
 * @param pDirector Shadow director.
 * @param pParam Light parameters.
 * @param view View index.
 */
template <>
void trySetupShadowLpp<al::LppLineParam>(al::PrePassLightKeeper* pKeeper,
                                         al::ShadowDirector* pDirector, al::LppLineParam* pParam,
                                         s32 view) {}

/**
 * Sets the depth shadow map of a spot light.
 * @param pKeeper Prepass light keeper.
 * @param pDirector Shadow director.
 * @param pParam Light parameters.
 * @param view View index.
 */
template <>
void trySetupShadowLpp<al::LppSpotParam>(al::PrePassLightKeeper* pKeeper,
                                         al::ShadowDirector* pDirector, al::LppSpotParam* pParam,
                                         s32 view) {
    agl::sdw::DepthShadow* depthShadow = pDirector->getDepthShadow();

    if (pParam->mShadowIndex < 0) {
        return;
    }

    const sead::Matrix44f& shadowMtx = depthShadow->getUnit(0).getTexMtx();
    const agl::TextureSampler* shadowMap = depthShadow->getShadowMap().getDepthSampler();
    pKeeper->setSpotLightShadowMap(
        pParam->mShadowIndex, view, shadowMap, shadowMtx,
        static_cast<agl::lght::LightPrePass::ShadowType>(pParam->mShadowType), pParam->mPcf, false);
}

/**
 * Sets the depth shadow map of a projection light.
 * @param pKeeper Prepass light keeper.
 * @param pDirector Shadow director.
 * @param pParam Light parameters.
 * @param view View index.
 */
template <>
void trySetupShadowLpp<al::LppProjParam>(al::PrePassLightKeeper* pKeeper,
                                         al::ShadowDirector* pDirector, al::LppProjParam* pParam,
                                         s32 view) {
    agl::sdw::DepthShadow* depthShadow = pDirector->getDepthShadow();

    if (pParam->mShadowIndex < 0) {
        return;
    }

    const sead::Matrix44f& shadowMtx = depthShadow->getUnit(0).getTexMtx();
    const agl::TextureSampler* shadowMap = depthShadow->getShadowMap().getDepthSampler();
    pKeeper->setProjLightShadowMap(
        pParam->mShadowIndex, view, shadowMap, shadowMtx,
        static_cast<agl::lght::LightPrePass::ShadowType>(pParam->mShadowType), pParam->mPcf, false);
}

/**
 * Sets the depth shadow map of an orthographic projection light.
 * @param pKeeper Prepass light keeper.
 * @param pDirector Shadow director.
 * @param pParam Light parameters.
 * @param view View index.
 */
template <>
void trySetupShadowLpp<al::LppProjOrthoParam>(al::PrePassLightKeeper* pKeeper,
                                              al::ShadowDirector* pDirector,
                                              al::LppProjOrthoParam* pParam, s32 view) {
    agl::sdw::DepthShadow* depthShadow = pDirector->getDepthShadow();

    if (pParam->mShadowIndex < 0) {
        return;
    }

    const sead::Matrix44f& shadowMtx = depthShadow->getUnit(0).getTexMtx();
    const agl::TextureSampler* shadowMap = depthShadow->getShadowMap().getDepthSampler();
    pKeeper->setProjLightShadowMap(
        pParam->mShadowIndex, view, shadowMap, shadowMtx,
        static_cast<agl::lght::LightPrePass::ShadowType>(pParam->mShadowType), pParam->mPcf, false);
}

}  // namespace LppFunction
