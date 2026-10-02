#include "Library/Play/Camera/CameraPoserFix.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace {

/**
 * Calculates the direction from the look at position to the camera.
 * @param pDir Output direction.
 * @param pPoser Camera poser whose zone rotation is applied.
 * @param angleH Horizontal angle in degrees.
 * @param rAngleV Vertical angle in degrees.
 */
inline void calcViewDir(sead::Vector3f* pDir, const al::CameraPoser_RS* pPoser, f32 angleH,
                        const f32& rAngleV) {
    f32 zoneAngleH = alCameraPoserFunction::calcZoneRotateAngleH(angleH, pPoser);
    f32 x = sinf(sead::Mathf::deg2rad(zoneAngleH)) * cosf(sead::Mathf::deg2rad(rAngleV));
    f32 y = sinf(sead::Mathf::deg2rad(rAngleV));
    f32 z = cosf(sead::Mathf::deg2rad(zoneAngleH)) * cosf(sead::Mathf::deg2rad(rAngleV));
    pDir->set(x, y, z);
    al::normalize(pDir);
}

}  // namespace

namespace al {

/**
 * Creates a fixed camera. The absolute and doorway variants cannot switch to the subjective
 * camera.
 * @param pName Camera name.
 */
CameraPoserFix::CameraPoserFix(const char* pName) : CameraPoser_RS(pName) {
    if (isEqualString(pName, getFixAbsoluteCameraName())) {
        alCameraPoserFunction::invalidateChangeSubjective(this);
    } else if (isEqualString(pName, getFixDoorwayCameraName())) {
        alCameraPoserFunction::invalidateChangeSubjective(this);
        alCameraPoserFunction::initAngleSwing(this);
    } else {
        alCameraPoserFunction::initAngleSwing(this);
        alCameraPoserFunction::validateCtrlSubjective(this);
    }

    initOrthoProjectionParam();
}

/**
 * Initializes the snapshot controller.
 */
void CameraPoserFix::init() {
    alCameraPoserFunction::initSnapShotCameraCtrlZoomAutoReset(this);
}

/**
 * Derives the look at position, distance and angles from a camera and a look at position.
 * @param rCameraPos Camera position.
 * @param rLookAtPos Look at position.
 */
void CameraPoserFix::initCameraPosAndLookAtPos(const sead::Vector3f& rCameraPos,
                                               const sead::Vector3f& rLookAtPos) {
    mLookAtPos.set(rLookAtPos);
    mDistance = (rLookAtPos - rCameraPos).length();

    sead::Vector3f viewDir = rCameraPos - rLookAtPos;
    normalize(&viewDir);
    mAngleV = sead::Mathf::rad2deg(asinf(viewDir.y));

    sead::Vector3f viewDirPlane = viewDir;
    viewDirPlane.y = 0.0f;
    tryNormalizeOrDirZ(&viewDirPlane);
    mAngleH = calcAngleOnPlaneDegree(sead::Vector3f::ez, viewDirPlane, sead::Vector3f::ey);
}

/**
 * Loads the look at position, distance and angles.
 * @param rIter Camera parameter iterator.
 */
void CameraPoserFix::loadParam(const ByamlIter& rIter) {
    tryGetByamlV3f(&mLookAtPos, rIter, "LookAtPos");
    tryGetByamlF32(&mDistance, rIter, "Distance");
    tryGetByamlF32(&mAngleV, rIter, "AngleV");
    tryGetByamlF32(&mAngleH, rIter, "AngleH");
    tryGetByamlBool(&mIsCalcNearestAtFromPreAt, rIter, "IsCalcNearestAtFromPreAt");
}

/**
 * Remembers the previous look at position and updates the pose.
 * @param rInfo Start info.
 */
void CameraPoserFix::start(const CameraStartInfo& rInfo) {
    mPreLookAtPos.set(alCameraPoserFunction::getPreLookAtPos(this));
    update();
}

/**
 * @return The fixed look at position.
 */
sead::Vector3f& CameraPoserFix::getFixedLookAt() {
    return mLookAtPos;
}

/**
 * Sets the fixed look at position.
 * @param rLookAtPos New look at position.
 */
void CameraPoserFix::setFixedLookAt(sead::Vector3f& rLookAtPos) {
    mLookAtPos = rLookAtPos;
}

/**
 * Sets the distance and height offsets.
 * @param distanceOffset Offset added to the distance.
 * @param heightOffset Offset added to the look at height.
 */
void CameraPoserFix::setPhaseOffsets(f32 distanceOffset, f32 heightOffset) {
    mHeightOffset = heightOffset;
    mDistanceOffset = distanceOffset;
}

/**
 * Sets a distance offset that is scaled by a rate every frame.
 * @param offset Initial offset.
 * @param decayRate Rate the offset is multiplied with each frame.
 */
void CameraPoserFix::setDistanceInitOffset(f32 offset, f32 decayRate) {
    mDistanceInitOffset = offset;
    mDistanceInitOffsetRate = decayRate;
}

/**
 * Starts turning the camera towards new angles.
 * @param angleH Target horizontal angle.
 * @param angleV Target vertical angle.
 * @param rate Interpolation rate per frame.
 */
void CameraPoserFix::setRedirectAngles(f32 angleH, f32 angleV, f32 rate) {
    mIsRedirect = true;
    mIsSecondCameraDone = false;
    mCurrentAngleH = mAngleH;
    mRedirectRate = rate;
    mRedirectAngleH = angleH;
    mRedirectAngleV = angleV;
    mCurrentAngleV = mAngleV;
}

/**
 * @return Whether turning to the redirect angles finished.
 */
bool CameraPoserFix::isSecondCameraDone() {
    return mIsSecondCameraDone;
}

/**
 * Updates the camera pose, either moving back to the stored camera or looking at the fixed
 * position from the current angles.
 */
void CameraPoserFix::update() {
    if (mIsReturnDone) {
        return;
    }

    mUp.set(sead::Vector3f::ey);

    if (mIsReturn) {
        if (mReturnStepMax == 0) {
            mIsReturn = false;
            mIsReturnDone = true;
            mEye.set(mReturnCameraPos);
            mAt.set(mReturnLookAtPos);
            return;
        }

        f32 rate = static_cast<f32>(mReturnStep) / static_cast<f32>(mReturnStepMax);
        mEye = mReturnCameraPos * rate + mReturnStartCameraPos * (1.0f - rate);
        mAt = mLookAtPos * (1.0f - rate) + mReturnLookAtPos * rate;

        if (mReturnStep >= mReturnStepMax) {
            mIsReturn = false;
            mIsReturnDone = true;
            mEye.set(mReturnCameraPos);
            mAt.set(mReturnLookAtPos);
            return;
        }

        mReturnStep++;
        return;
    }

    mAt.setMul(mViewMtx, mLookAtPos + sead::Vector3f(0.0f, mHeightOffset, 0.0f));

    sead::Vector3f viewDir;

    if (mIsRedirect) {
        mCurrentAngleH = lerpValue(mRedirectRate, mCurrentAngleH, mRedirectAngleH);
        mCurrentAngleV = lerpValue(mRedirectRate, mCurrentAngleV, mRedirectAngleV);

        if (isNearZero(mCurrentAngleH - mRedirectAngleH, 1.0f)) {
            mCurrentAngleH = mRedirectAngleH;
            mCurrentAngleV = mRedirectAngleV;
            mIsRedirect = false;
            mIsSecondCameraDone = true;
        }

        calcViewDir(&viewDir, this, mCurrentAngleH, mCurrentAngleV);
    } else {
        calcViewDir(&viewDir, this, mAngleH, mAngleV);
    }

    mDistanceInitOffset *= mDistanceInitOffsetRate;
    f32 distance = mDistance + mDistanceOffset + mDistanceInitOffset;
    mEye.set(distance * viewDir + mAt);

    if (mIsCalcNearestAtFromPreAt) {
        sead::Vector3f offset = mPreLookAtPos - mEye;
        parallelizeVec(&offset, viewDir, offset);

        if (!isNearZero(offset) && viewDir.dot(offset) < 0.0f) {
            mAt.set(offset + mEye);
        }
    }
}

/**
 * Cancels returning to the stored camera.
 */
void CameraPoserFix::resetReturn() {
    mIsReturn = false;
    mIsReturnDone = false;
}

/**
 * Stores the camera to return to.
 * @param rCameraPos Camera position.
 * @param rLookAtPos Look at position.
 */
void CameraPoserFix::storeCamera(const sead::Vector3f& rCameraPos,
                                 const sead::Vector3f& rLookAtPos) {
    mReturnCameraPos.set(rCameraPos);
    mReturnLookAtPos.set(rLookAtPos);
    mReturnDistance = (mReturnLookAtPos - mReturnCameraPos).length();
}

/**
 * Starts returning to the stored look at position from the given angles.
 * @param step Number of frames the return takes.
 * @param angleH Horizontal angle of the return camera.
 * @param angleV Vertical angle of the return camera.
 * @param distance Distance of the return camera, or 0 to use the stored distance.
 */
void CameraPoserFix::setReturnWithAngles(s32 step, f32 angleH, f32 angleV, f32 distance) {
    if (mIsReturn) {
        return;
    }

    setReturn(step);

    if (distance == 0.0f) {
        distance = mReturnDistance;
    }

    f32 radianH = sead::Mathf::deg2rad(angleH);
    mReturnCameraPos.x = mReturnLookAtPos.x + distance * sinf(radianH);
    mReturnCameraPos.z = mReturnLookAtPos.z - distance * cosf(radianH);
    mReturnCameraPos.y = mReturnLookAtPos.y + distance * sinf(sead::Mathf::deg2rad(angleV));
}

/**
 * Starts returning to the stored camera.
 * @param step Number of frames the return takes.
 */
void CameraPoserFix::setReturn(s32 step) {
    if (mIsReturn) {
        return;
    }

    mIsReturn = true;
    mIsReturnDone = false;
    mReturnStepMax = step;
    mReturnStep = 0;
    mReturnStartCameraPos.set(mEye);
    mLookAtPos.set(mAt);
}

/**
 * @return Name of the camera that is fixed absolutely.
 */
const char* CameraPoserFix::getFixAbsoluteCameraName() {
    return "完全固定";
}

/**
 * @return Name of the camera used for doorways.
 */
const char* CameraPoserFix::getFixDoorwayCameraName() {
    return "出入口専用固定";
}

}  // namespace al
