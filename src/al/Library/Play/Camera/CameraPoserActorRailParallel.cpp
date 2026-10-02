#include "Library/Play/Camera/CameraPoserActorRailParallel.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Rail/RailUtil.hpp"
#include "Library/Yaml/ByamlUtil.hpp"

namespace al {

/**
 * Constructs a poser that follows the rail position of an actor.
 * @param pName Poser name.
 * @param pRailKeeper Rail keeper of the followed actor.
 */
CameraPoserActorRailParallel::CameraPoserActorRailParallel(const char* pName,
                                                           const RailKeeper* pRailKeeper)
    : CameraPoser_RS(pName), mRailKeeper(pRailKeeper) {}

/**
 * Initializes the camera move limit.
 */
void CameraPoserActorRailParallel::init() {
    alCameraPoserFunction::initCameraMoveLimit(this);
}

/**
 * Loads the camera offset, distance, angles and follow rate.
 * @param rIter Camera parameter iterator.
 */
void CameraPoserActorRailParallel::loadParam(const ByamlIter& rIter) {
    tryGetByamlV3f(&mOffset, rIter, "Offset");
    tryGetByamlF32(&mDistance, rIter, "Distance");
    tryGetByamlF32(&mAngleDegreeH, rIter, "AngleDegreeH");
    tryGetByamlF32(&mAngleDegreeV, rIter, "AngleDegreeV");
    tryGetByamlF32(&mFollowRate, rIter, "FollowRate");
}

/**
 * Calculates a camera position around a look-at position.
 * @param pPos Output camera position.
 * @param rAt Look-at position.
 * @param distance Distance from the look-at position.
 * @param angleDegreeH Horizontal angle, in degrees.
 * @param angleDegreeV Vertical angle, in degrees.
 */
static void calcCameraPos(sead::Vector3f* pPos, const sead::Vector3f& rAt, f32 distance,
                          f32 angleDegreeH, f32 angleDegreeV) {
    f32 angleH = sead::Mathf::deg2rad(angleDegreeH);
    sead::Vector3f dir(sead::Mathf::sin(angleH), 0.0f, sead::Mathf::cos(angleH));
    f32 angleV = sead::Mathf::deg2rad(angleDegreeV);
    f32 lengthH = sead::Mathf::cos(angleV);
    f32 length = dir.length();

    if (length > 0.0f) {
        f32 scale = lengthH / length;
        dir.x *= scale;
        dir.y *= scale;
        dir.z *= scale;
    }

    dir.y = sead::Mathf::sin(angleV);
    *pPos = rAt + dir * distance;
}

/**
 * Places the camera next to the rail position of the actor.
 * @param rInfo Camera start info.
 */
void CameraPoserActorRailParallel::start(const CameraStartInfo& rInfo) {
    mAt = getRailPos(mRailKeeper) + mOffset;
    calcCameraPos(&mEye, mAt, mDistance, mAngleDegreeH, mAngleDegreeV);
}

/**
 * Follows the rail position of the actor.
 */
void CameraPoserActorRailParallel::update() {
    sead::Vector3f target = getRailPos(mRailKeeper) + mOffset;
    lerpVec(&mAt, mAt, target, mFollowRate);
    calcCameraPos(&mEye, mAt, mDistance, mAngleDegreeH, mAngleDegreeV);
}

}  // namespace al
