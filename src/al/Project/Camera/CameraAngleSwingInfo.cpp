#include "Project/Camera/CameraAngleSwingInfo.hpp"

#include "Library/Math/MathUtil.hpp"
#include "Library/Yaml/ByamlUtil.hpp"

namespace al {

/**
 * Creates swing info with the default limits.
 */
CameraAngleSwingInfo::CameraAngleSwingInfo() {}

/**
 * Loads the swing parameters.
 * @param rIter Camera parameter iterator.
 */
void CameraAngleSwingInfo::load(const ByamlIter& rIter) {
    tryGetByamlBool(&isInvalidSwing, rIter, "IsInvalidSwing");

    if (isInvalidSwing) {
        return;
    }

    tryGetByamlF32(&maxSwingDegreeH, rIter, "MaxSwingDegreeH");
    tryGetByamlF32(&maxSwingDegreeV, rIter, "MaxSwingDegreeV");
}

/**
 * Moves the swing angle towards the stick direction.
 * @param rStick Stick input.
 * @param sensitivityScale Scale for the target interpolation rate.
 */
void CameraAngleSwingInfo::update(const sead::Vector2f& rStick, f32 sensitivityScale) {
    if (isInvalidSwing) {
        currentAngle = {0.0f, 0.0f};
        return;
    }

    sead::Vector2f target = {-rStick.x * maxSwingDegreeH, rStick.y * maxSwingDegreeV};
    lerpVec(&target, currentAngle, target, targetLerpRate * sensitivityScale);
    lerpVec(&currentAngle, currentAngle, target, angleLerpRate);
}

/**
 * Rotates the look-at position around the camera by the swing angle.
 * @param pCamera Camera to modify.
 */
void CameraAngleSwingInfo::makeLookAtCamera(sead::LookAtCamera* pCamera) const {
    sead::Vector3f front = pCamera->getAt() - pCamera->getPos();
    f32 distance = front.length();
    normalize(&front);
    sead::Vector3f up = sead::Vector3f::ey;
    verticalizeVec(&up, front, up);

    if (!tryNormalizeOrZero(&up)) {
        return;
    }

    rotateVectorDegree(&front, front, up, currentAngle.x);
    normalize(&front);
    sead::Vector3f side;
    side.setCross(front, up);
    normalize(&side);
    rotateVectorDegree(&front, front, side, currentAngle.y);
    pCamera->setAt(pCamera->getPos() + front * distance);
}

}  // namespace al
