#include "Library/Fog/FogDirector.hpp"

#include <gfx/seadCamera.h>

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/LiveActor/LiveActor.hpp"

namespace al {

/**
 * Registers the fog parameters and sets their default values.
 */
void FogParam::init() {
    mColor.init(sead::Color4f::cWhite, "Color", "Color", &mParamObj);
    mMulColor.init(sead::Color4f::cWhite, "MulColor", "MulColor", &mParamObj);
    mIntensityMax.init(0.0f, "IntensityMax", "IntensityMax", "Min=0.0f, Max=1.0f", &mParamObj);
    mStart.init(0.0f, "Start", "Start", "Min=0.f, Max=10000.f", &mParamObj);
    mEnd.init(10000.0f, "End", "End", "Min=100.f, Max=10000.f", &mParamObj);
}

/**
 * Initializes the parameters through the virtual init.
 */
void FogParam::initSystem() {
    init();
}

/**
 * Compares two fog parameters.
 * @param rOther Parameter to compare with.
 * @return Whether both parameters are (nearly) equal.
 */
bool FogParam::operator==(const FogParam& rOther) const {
    return getEnd() == rOther.getEnd() && getStart() == rOther.getStart() &&
           *mIntensityMax == *rOther.mIntensityMax && isNear(*mColor, *rOther.mColor, 0.001f) &&
           isNear(*mMulColor, *rOther.mMulColor, 0.001f);
}

/**
 * Copies the parameter values.
 * @param rOther Parameter to copy from.
 * @return This parameter.
 */
FogParam& FogParam::operator=(const FogParam& rOther) {
    *mColor = *rOther.mColor;
    *mMulColor = *rOther.mMulColor;
    *mIntensityMax = *rOther.mIntensityMax;
    *mStart = *rOther.mStart;
    *mEnd = *rOther.mEnd;
    return *this;
}

/**
 * Interpolates between two fog parameters.
 * @param rA Start parameter.
 * @param rB End parameter.
 * @param rate Interpolation rate.
 */
void FogParam::interp(const FogParam& rA, const FogParam& rB, f32 rate) {
    mIntensityMax.copyLerp(rA.mIntensityMax, rB.mIntensityMax, rate);
    *mStart = lerpValueNew(rA.getStart(), rB.getStart(), rate);
    *mEnd = lerpValueNew(rA.getEnd(), rB.getEnd(), rate);
    mColor.copyLerp(rA.mColor, rB.mColor, rate);
    mMulColor.copyLerp(rA.mMulColor, rB.mMulColor, rate);
}

/**
 * Compares two height fog parameters.
 * @param rOther Parameter to compare with.
 * @return Whether both parameters are (nearly) equal.
 */
bool YFogParam::operator==(const YFogParam& rOther) const {
    if (getEnd() == rOther.getEnd()) {
        if (getStart() == rOther.getStart()) {
            if (*mIntensityMax == *rOther.mIntensityMax) {
                if (isNear(*mColor, *rOther.mColor, 0.001f)) {
                    if (isNear(*mMulColor, *rOther.mMulColor, 0.001f)) {
                        return *mIsFollowCamera == *rOther.mIsFollowCamera;
                    }
                }
            }
        }
    }

    return false;
}

/**
 * Copies the parameter values.
 * @param rOther Parameter to copy from.
 * @return This parameter.
 */
YFogParam& YFogParam::operator=(const YFogParam& rOther) {
    *mColor = *rOther.mColor;
    *mMulColor = *rOther.mMulColor;
    *mIntensityMax = *rOther.mIntensityMax;
    *mStart = *rOther.mStart;
    *mEnd = *rOther.mEnd;
    *mIsFollowCamera = *rOther.mIsFollowCamera;
    return *this;
}

/**
 * Interpolates between two height fog parameters.
 * @param rA Start parameter.
 * @param rB End parameter.
 * @param rate Interpolation rate.
 */
void YFogParam::interp(const YFogParam& rA, const YFogParam& rB, f32 rate) {
    mIntensityMax.copyLerp(rA.mIntensityMax, rB.mIntensityMax, rate);
    *mStart = lerpValueNew(rA.getStart(), rB.getStart(), rate);
    *mEnd = lerpValueNew(rA.getEnd(), rB.getEnd(), rate);
    mColor.copyLerp(rA.mColor, rB.mColor, rate);
    mMulColor.copyLerp(rA.mMulColor, rB.mMulColor, rate);
    mIsFollowCamera.copy(rate < 0.5f ? rA.mIsFollowCamera : rB.mIsFollowCamera);
}

/**
 * Registers the height fog parameters and sets their default values.
 */
void YFogParam::init() {
    FogParam::init();
    mCameraYPos = 0.0f;
    mIsValidCameraYPos = false;
    mIsFollowCamera.init(false, "IsFollowCamera", "カメラ追従", &mParamObj);
}

/**
 * @brief Moves the height fog with the camera height when following the camera.
 */
void YFogParam::trySetCameraYPos(const sead::Camera* pCamera) {
    if (*mIsFollowCamera) {
        sead::Vector3f cameraPos;
        pCamera->getWorldPosByMatrix(&cameraPos);
        mCameraYPos = cameraPos.y;
    } else if (!mIsValidCameraYPos) {
        mCameraYPos = 0.0f;
    }
}

/**
 * @brief Creates the fog and height fog parameter keepers.
 */
FogDirector::FogDirector(GraphicsSystemInfo* pInfo)
    : mFogKeeper(pInfo, 0xc, "Fog", "aglfog", nullptr),
      mYFogKeeper(pInfo, 0xd, "YFog", "aglfog", nullptr) {}

/**
 * @brief Loads the fog parameter files of a stage.
 */
void FogDirector::initStageResource(const Resource* pResource, const char* pStageName) {
    mFogKeeper.initStageResource(pResource, pStageName);
    mYFogKeeper.initStageResource(pResource, pStageName);
}

/**
 * @brief Starts from the default parameters and stops applying requests directly.
 */
void FogDirector::endInit() {
    mFogKeeper.getCurrentParam() = mFogKeeper.getDefaultParam();
    mFogKeeper.getRequestInterp().endInit();
    mYFogKeeper.getCurrentParam() = mYFogKeeper.getDefaultParam();
    mYFogKeeper.getRequestInterp().endInit();
}

/**
 * @brief Drops the requests of this frame.
 */
void FogDirector::clear() {
    mFogKeeper.getRequestInterp().clearRequest();
    mFogKeeper.setForceUpdate(false);
    mYFogKeeper.getRequestInterp().clearRequest();
    mYFogKeeper.setForceUpdate(false);
}

/**
 * @brief Applies the graphics area requests and interpolates the parameters.
 */
void FogDirector::updateRequest() {
    mFogKeeper.updateRequest();
    mYFogKeeper.updateRequest();
}

/**
 * @brief Updates the height fog camera height.
 */
void FogDirector::updateCamera(const sead::Camera* pCamera) {
    mYFogKeeper.getCurrentParam().trySetCameraYPos(pCamera);
}

/**
 * @brief Requests a distance fog.
 */
void FogDirector::requestFog(s32 priority, s32 step, const FogParam& rParam) {
    mFogKeeper.setForceUpdate(true);
    mFogKeeper.getRequestInterp().requestParam(priority, step, rParam);
}

/**
 * @brief Requests a height fog.
 */
void FogDirector::requestYFog(s32 priority, s32 step, const YFogParam& rParam) {
    mYFogKeeper.setForceUpdate(true);
    mYFogKeeper.getRequestInterp().requestParam(priority, step, rParam);
}

/**
 * @brief Checks whether the distance fog uses a multiply color.
 */
bool FogDirector::isUsingMulFog() const {
    const FogParam& param = mFogKeeper.getCurrentParam();
    if (*param.mIntensityMax > 0.0f) {
        const sead::Color4f& mulColor = *param.mMulColor;
        return mulColor.r != 1.0f || mulColor.g != 1.0f || mulColor.b != 1.0f;
    }

    return false;
}

/**
 * @brief Checks whether the height fog uses a multiply color.
 */
bool FogDirector::isUsingMulYFog() const {
    const YFogParam& param = mYFogKeeper.getCurrentParam();
    if (*param.mIntensityMax > 0.0f) {
        const sead::Color4f& mulColor = *param.mMulColor;
        return mulColor.r != 1.0f || mulColor.g != 1.0f || mulColor.b != 1.0f;
    }

    return false;
}

}  // namespace al

namespace FogFunction {

/**
 * @brief Gets the fog director of the scene of an actor.
 */
al::FogDirector* getFogDirector(const al::LiveActor* pActor) {
    return static_cast<al::GraphicsSystemInfo*>(pActor->getSceneInfo()->_78)->mFogDirector;
}

}  // namespace FogFunction
