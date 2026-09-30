#include "Library/Play/Camera/CameraPoserRace.hpp"

#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Yaml/ByamlUtil.hpp"

namespace al {

/**
 * Creates a camera that follows behind the moving direction of the target.
 * @param pName Camera name.
 */
CameraPoserRace::CameraPoserRace(const char* pName) : CameraPoser_RS(pName) {}

/**
 * Initializes the arrow collider.
 */
void CameraPoserRace::init() {
    alCameraPoserFunction::initCameraArrowCollider(this);
}

/**
 * Loads the offset, distance, angle, rotation rates and velocity flag.
 * @param rIter Camera parameter iterator.
 */
void CameraPoserRace::loadParam(const ByamlIter& rIter) {
    tryGetByamlF32(&mOffsetY, rIter, "OffsetY");
    tryGetByamlF32(&mDistance, rIter, "Distance");
    tryGetByamlF32(&mAngleDegreeV, rIter, "AngleDegreeV");
    tryGetByamlF32(&mRotateRate1, rIter, "RotateRate1");
    tryGetByamlF32(&mRotateRate2, rIter, "RotateRate2");
    tryGetByamlBool(&mIsTurnToVelocity, rIter, "IsTurnToVelocity");
}

void CameraPoserRace::start(const CameraStartInfo& rInfo) {
    mRotateAngle = 0.0f;
    calcTargetFrontLocal(&mFrontDir, true);
}

void CameraPoserRace::calcTargetFrontLocal(sead::Vector3f* pFront, bool isUnused) const {
    if (mFrontDirPtr) {
        *pFront = *mFrontDirPtr;
        pFront->y = 0.0f;
        normalize(pFront);
        return;
    }

    if (mIsTurnToVelocity) {
        sead::Vector3f velocity = {0.0f, 0.0f, 0.0f};
        alCameraPoserFunction::calcTargetVelocityH(&velocity, this);
        velocity.y = 0.0f;

        if (tryNormalizeOrZero(&velocity)) {
            *pFront = velocity;
            return;
        }
    }

    alCameraPoserFunction::calcTargetFront(pFront, this);
    pFront->y = 0.0f;
    normalize(pFront);
}

/**
 * Turns towards the target direction and places the camera behind it.
 */
void CameraPoserRace::update() {
    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    calcTargetFrontLocal(&front, false);
    f32 angle = calcAngleOnPlaneDegree(mFrontDir, front, sead::Vector3f::ey) * mRotateRate1;
    mRotateAngle = lerpValue(mRotateRate2, mRotateAngle, angle);
    rotateVectorDegreeY(&mFrontDir, mRotateAngle);
    mFrontDir.y = 0.0f;
    normalize(&mFrontDir);
    mUp = sead::Vector3f::ey;
    alCameraPoserFunction::setLookAtPosToTargetAddOffset(this, {0.0f, mOffsetY, 0.0f});
    sead::Vector3f dir = -mFrontDir;
    sead::Vector3f side;
    side.setCross(mFrontDir, sead::Vector3f::ey);
    side = -side;
    rotateVectorDegree(&dir, dir, side, mAngleDegreeV);
    f32 distance = mDistance;
    f32 length = dir.length();

    if (length > 0.0f) {
        dir *= distance / length;
    }

    mEye.set(mAt + dir);
}

}  // namespace al
