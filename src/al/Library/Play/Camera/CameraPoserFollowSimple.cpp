#include "Library/Play/Camera/CameraPoserFollowSimple.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Yaml/ByamlUtil.hpp"

namespace al {

/**
 * Creates a camera that follows the target from behind.
 * @param pName Camera name.
 */
CameraPoserFollowSimple::CameraPoserFollowSimple(const char* pName) : CameraPoser_RS(pName) {}

/**
 * Loads the height offset, distance, angle and rotation flags.
 * @param rIter Camera parameter iterator.
 */
void CameraPoserFollowSimple::loadParam(const ByamlIter& rIter) {
    tryGetByamlF32(&mOffsetY, rIter, "OffsetY");
    tryGetByamlF32(&mDistance, rIter, "Distance");
    tryGetByamlF32(&mAngle, rIter, "Angle");
    tryGetByamlBool(&mIsRotateH, rIter, "IsRotateH");
    tryGetByamlBool(&mIsResetAngleIfSwitchTarget, rIter, "IsResetAngleIfSwitchTarget");
}

/**
 * Keeps the horizontal direction of the previous camera, or resets on the first frame.
 * @param rInfo Start info.
 */
void CameraPoserFollowSimple::start(const CameraStartInfo& rInfo) {
    if (alCameraPoserFunction::isSceneCameraFirstCalc(this)) {
        reset();
        return;
    }

    const sead::LookAtCamera& camera = alCameraPoserFunction::getLookAtCamera(this);
    sead::Vector3f dir = {camera.getPos().x - camera.getAt().x, 0.0f,
                          camera.getPos().z - camera.getAt().z};
    tryNormalizeOrDirZ(&dir);
    mEye.set(mDistance * dir + mAt);
}

/**
 * Follows the target at a fixed distance and angle, rotating horizontally by stick input.
 */
void CameraPoserFollowSimple::update() {
    if (alCameraPoserFunction::isChangeTarget(this) && mIsResetAngleIfSwitchTarget) {
        reset();
    }

    alCameraPoserFunction::calcTargetTrans(&mAt, this);
    mAt.y += mOffsetY;
    sead::Vector3f dir = {mEye.x - mAt.x, 0.0f, mEye.z - mAt.z};
    tryNormalizeOrDirZ(&dir);
    if (mIsRotateH) {
        f32 stickH = alCameraPoserFunction::calcCameraRotateStickH(this);
        rotateVectorDegreeY(&dir, sead::Mathf::abs(stickH) < 0.3f ? 0.0f : stickH * -2.0f);
    }

    sead::Vector3f eyeDir = dir;
    sead::Vector3f side;
    side.setCross(dir, mUp);
    rotateVectorDegree(&eyeDir, eyeDir, side, mAngle);
    f32 distance = mDistance;
    f32 length = eyeDir.length();
    if (length > 0.0f) {
        eyeDir *= distance / length;
    }

    mEye.set(mAt + eyeDir);
}

/**
 * Places the camera behind the target.
 */
void CameraPoserFollowSimple::reset() {
    alCameraPoserFunction::calcTargetTrans(&mAt, this);
    mAt.y += mOffsetY;
    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    alCameraPoserFunction::calcTargetFront(&front, this);
    mEye = mAt - mDistance * front;
}

}  // namespace al
