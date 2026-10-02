#include "Library/Play/Camera/CameraPoserInnerTower.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Camera/CameraOffsetPreset.hpp"

namespace {
using namespace al;

struct ModeEntry {
    CameraPoserInnerTower::Mode mode;
    const char* name;
};

const ModeEntry sModeTable[] = {
    {CameraPoserInnerTower::Mode::AxisMoveFixTargetCenter, "軸移動＋対象中心固定"},
    {CameraPoserInnerTower::Mode::AxisMoveFixLookDistance, "軸移動＋注視距離固定"},
    {CameraPoserInnerTower::Mode::Follow, "追随"},
};

/**
 * Looks up a camera mode by its name.
 * @param pName Mode name.
 * @return The mode, or the first mode if the name is unknown.
 */
CameraPoserInnerTower::Mode findMode(const char* pName) {
    for (s32 i = 0; i < 3; i++) {
        if (isEqualString(pName, sModeTable[i].name)) {
            return sModeTable[i].mode;
        }
    }

    return CameraPoserInnerTower::Mode::AxisMoveFixTargetCenter;
}

}  // namespace

namespace al {

/**
 * Creates a camera that circles around the axis of a tower from the inside.
 * @param pName Camera name.
 */
CameraPoserInnerTower::CameraPoserInnerTower(const char* pName) : CameraPoser_RS(pName) {}

/**
 * Sets up the camera helpers and the offset preset.
 */
void CameraPoserInnerTower::init() {
    alCameraPoserFunction::initCameraVerticalAbsorber(this);
    alCameraPoserFunction::initCameraAngleCtrlWithRelativeH(this);
    alCameraPoserFunction::initCameraArrowCollider(this);
    alCameraPoserFunction::initCameraMoveLimit(this);
    initLookAtInterpole(0.2f);
    alCameraPoserFunction::invalidateCollider(this);
    alCameraPoserFunction::initCameraDefaultAngleRangeV(this, 0.0f, 85.0f);
    mOffsetPreset = new CameraOffsetPreset();
    alCameraPoserFunction::initSnapShotCameraCtrlZoomRollMove(this, false, false);
}

/**
 * Reads the tower axis from the linked placement and its height from the scale.
 * @param rInfo Placement info of the camera.
 */
void CameraPoserInnerTower::initByPlacementObj(const PlacementInfo& rInfo) {
    PlacementInfo axisInfo;
    getLinksInfo(&axisInfo, rInfo, "InnerTowerCameraAxis");
    getTrans(&mAxisPos, axisInfo);
    sead::Vector3f scale = {0.0f, 0.0f, 0.0f};
    tryGetScale(&scale, axisInfo);
    mAxisHeight = scale.y * 500.0f;
    tryInitAreaLimitter(rInfo);
}

/**
 * Loads the offset preset, distance, mode and follow limits.
 * @param rIter Camera parameter iterator.
 */
void CameraPoserInnerTower::loadParam(const ByamlIter& rIter) {
    mOffsetPreset->loadParam(rIter);
    tryGetByamlF32(&mDistance, rIter, "Distance");
    const char* modeName = tryGetByamlKeyStringOrNULL(rIter, "ModeName");

    if (modeName == nullptr) {
        return;
    }

    mMode = findMode(modeName);

    if (mMode != Mode::Follow) {
        return;
    }

    alCameraPoserFunction::validateCollider(this);
    mIsLimitFollowDistance = tryGetByamlKeyBoolOrFalse(rIter, "IsLimitFollowDistance");

    if (mIsLimitFollowDistance) {
        tryGetByamlF32(&mLimitFollowDistance, rIter, "LimitFollowDistance");
    }
}

/**
 * Places the camera around the tower axis according to the mode, then applies the
 * horizontal angle input.
 */
void CameraPoserInnerTower::update() {
    sead::Vector3f target = {0.0f, 0.0f, 0.0f};
    alCameraPoserFunction::calcTargetTrans(&target, this);
    target += mOffsetPreset->getOffset();
    f32 angleV = alCameraPoserFunction::getCameraAngleV(this);

    switch (mMode) {
    case Mode::AxisMoveFixTargetCenter: {
        sead::Vector3f axisPos = {mAxisPos.x, target.y, mAxisPos.z};
        f32 distance = (axisPos - target).length();
        f32 height = distance * sead::Mathf::tan(sead::Mathf::deg2rad(angleV));
        mEye.set(mAxisPos.x, height + target.y, mAxisPos.z);
        mAt.set(target);
        break;
    }
    case Mode::AxisMoveFixLookDistance: {
        sead::Vector3f base = {mAxisPos.x, target.y, mAxisPos.z};
        sead::Vector3f dir = {target.x - base.x, 0.0f, target.z - base.z};

        if (!tryNormalizeOrZero(&dir)) {
            return;
        }

        sead::Vector3f offset = mDistance * dir;
        f32 rad = sead::Mathf::deg2rad(angleV);
        mAt = base + offset * sead::Mathf::cos(rad);
        mEye = base + (mDistance * sead::Vector3f::ey) * sead::Mathf::sin(rad);
        break;
    }
    case Mode::Follow: {
        mAt.set(target);
        f32 axisX = mAxisPos.x;
        f32 axisZ = mAxisPos.z;

        if (mIsLimitFollowDistance) {
            sead::Vector3f diff = {axisX - target.x, 0.0f, axisZ - target.z};

            if (mLimitFollowDistance < diff.length()) {
                sead::Vector3f dir = diff;
                normalize(&dir);
                mAt = dir * (diff.length() - mLimitFollowDistance) + target;
            }
        }

        sead::Vector3f dir = {axisX - mAt.x, 0.0f, axisZ - mAt.z};

        if (!tryNormalizeOrZero(&dir)) {
            return;
        }

        f32 rad = sead::Mathf::deg2rad(angleV);
        f32 cos = sead::Mathf::cos(rad);
        f32 x = cos * dir.x;
        f32 z = cos * dir.z;
        sead::Vector3f eyeDir = {x, sead::Mathf::sin(rad), z};
        mEye = mAt + eyeDir * mDistance;
        break;
    }
    default:
        break;
    }

    sead::Vector3f dir = mEye - mAt;
    rotateVectorDegreeY(&dir, -alCameraPoserFunction::getCameraAngleH(this));
    mEye.set(mAt + dir);
}

}  // namespace al
