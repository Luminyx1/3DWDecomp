#include "Library/Play/Camera/CameraPoserLookDown.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Yaml/ByamlUtil.hpp"

namespace al {

/**
 * Creates a camera that looks down on the target.
 * @param pName Camera name.
 */
CameraPoserLookDown::CameraPoserLookDown(const char* pName) : CameraPoser_RS(pName) {}

/**
 * Loads the offset, distance, angle and rotation flags.
 * @param rIter Camera parameter iterator.
 */
void CameraPoserLookDown::loadParam(const ByamlIter& rIter) {
    tryGetByamlF32(&mOffsetY, rIter, "OffsetY");
    tryGetByamlF32(&mDistance, rIter, "Distance");
    tryGetByamlF32(&mAngle, rIter, "Angle");
    tryGetByamlBool(&mIsRotateH, rIter, "IsRotateH");
    tryGetByamlBool(&mIsRotateV, rIter, "IsRotateV");
    tryGetByamlBool(&mIsResetAngleIfSwitchTarget, rIter, "IsResetAngleIfSwitchTarget");
    tryGetByamlBool(&mIsForceFollow, rIter, "IsForceFollow");
    tryGetByamlBool(&mIsPosYLocked, rIter, "IsPosYLocked");
}

/**
 * Takes over the pose of the previous camera.
 * @param rInfo Start info.
 */
void CameraPoserLookDown::start(const CameraStartInfo& rInfo) {
    const sead::LookAtCamera& camera = alCameraPoserFunction::getLookAtCamera(this);
    mEye = camera.getPos();
    mAt = camera.getAt();
    mUp = camera.getUp();
    mStartAt.set(camera.getAt());
}

void CameraPoserLookDown::update() {
    sead::Vector3f prevAt = mAt;
    if (mIsPosYLocked) {
        f32 offsetY = mOffsetY;
        sead::Vector3f trans;
        alCameraPoserFunction::calcTargetTrans(&trans, this);
        if (trans.y < mStartAt.y) {
            trans.y = mStartAt.y;
        }

        mAt = trans;
        mAt.y = offsetY + mAt.y;
    } else {
        alCameraPoserFunction::calcTargetTrans(&mAt, this);
    }

    sead::Vector3f dir = mEye - prevAt;
    if (mIsForceFollow) {
        dir = mEye - mAt;
    }

    dir.y = 0.0f;
    if (isNearZero(dir, 0.001f)) {
        dir = sead::Vector3f::ez;
    }

    f32 distance = mDistance;
    f32 length = dir.length();
    if (length > 0.0f) {
        dir *= distance / length;
    }

    if (mIsRotateH) {
        sead::Vector3f back = -dir;
        sead::Vector2f stick = sead::Vector2f::zero;
        alCameraPoserFunction::calcCameraRotateStick(&stick, this);
        rotateVectorDegreeY(&back, sead::Mathf::abs(stick.x) < 0.3f ? 0.0f : stick.x * -2.0f);
        dir = -back;
    }

    mEye.set(mAt + dir);

    if (mIsRotateV) {
        sead::Vector2f stick = sead::Vector2f::zero;
        alCameraPoserFunction::calcCameraRotateStick(&stick, this);
        mAngle -= sead::Mathf::abs(stick.y) < 0.3f ? 0.0f : stick.y;
        mAngle = sead::Mathf::clamp(mAngle, 10.0f, 80.0f);
    }
}

void CameraPoserLookDown::makeLookAtCamera(sead::LookAtCamera* pCamera) const {
    sead::Vector3f dir = mEye - mAt;
    sead::Vector3f front = dir;
    tryNormalizeOrDirZ(&front);
    sead::Vector3f side;
    side.setCross(front, sead::Vector3f::ey);
    rotateVectorDegree(&dir, dir, side, mAngle);
    f32 distance = mDistance;
    f32 length = dir.length();
    if (length > 0.0f) {
        dir *= distance / length;
    }

    pCamera->setPos(mAt + dir);
}

}  // namespace al
