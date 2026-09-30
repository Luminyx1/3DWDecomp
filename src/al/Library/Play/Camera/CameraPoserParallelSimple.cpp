#include "Library/Play/Camera/CameraPoserParallelSimple.hpp"

#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Yaml/ByamlUtil.hpp"

namespace al {

/**
 * Creates a camera that looks at the target from a fixed direction.
 * @param pName Camera name.
 */
CameraPoserParallelSimple::CameraPoserParallelSimple(const char* pName) : CameraPoser_RS(pName) {}

/**
 * Initializes the camera move limit.
 */
void CameraPoserParallelSimple::init() {
    alCameraPoserFunction::initCameraMoveLimit(this);
}

/**
 * Loads the distance, look-at offset and angles.
 * @param rIter Camera parameter iterator.
 */
void CameraPoserParallelSimple::loadParam(const ByamlIter& rIter) {
    tryGetByamlF32(&mDistance, rIter, "Distance");
    tryGetByamlV3f(&mLookAtOffset, rIter, "LookAtOffset");
    tryGetByamlF32(&mAngleH, rIter, "AngleH");
    tryGetByamlF32(&mAngleV, rIter, "AngleV");
}

/**
 * Places the camera at a fixed direction and distance from the offset target position.
 */
void CameraPoserParallelSimple::update() {
    mUp = sead::Vector3f::ey;
    alCameraPoserFunction::calcTargetTrans(&mAt, this);
    sead::Vector3f offset = mLookAtOffset;
    rotateVectorDegreeY(&offset, mAngleH);
    mAt += offset;
    sead::Vector3f dir = sead::Vector3f::ez;
    rotateVectorDegreeY(&dir, mAngleH);
    sead::Vector3f side;
    side.setCross(dir, sead::Vector3f::ey);
    rotateVectorDegree(&dir, dir, side, mAngleV);
    f32 distance = mDistance;
    f32 length = dir.length();
    if (length > 0.0f) {
        dir *= distance / length;
    }
    mEye = mAt + dir;
}

}  // namespace al
