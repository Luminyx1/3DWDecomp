#include "Library/Play/Camera/CameraPoserBossBattle.hpp"

#include <gfx/seadCamera.h>
#include <math/seadMathCalcCommon.h>

#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Yaml/ByamlUtil.hpp"

namespace {
using namespace al;

// The nerve keeper of a camera poser holds the poser itself, not its IUseNerve base.
CameraPoserBossBattle* getPoser(al::NerveKeeper* pKeeper) {
    return reinterpret_cast<CameraPoserBossBattle*>(pKeeper->mKeeperUser);
}

/**
 * Calculates the horizontal rotation requested with the camera stick.
 * @param pPoser Camera poser.
 * @return Rotation in degrees.
 */
f32 calcStickRotateDegreeH(const CameraPoser_RS* pPoser) {
    sead::Vector2f stick = sead::Vector2f::zero;
    alCameraPoserFunction::calcCameraRotateStick(&stick, pPoser);
    return sead::Mathf::abs(stick.x) < 0.3f ? 0.0f : stick.x * -2.0f;
}

/**
 * Calculates the vertical rotation requested with the camera stick.
 * @param pPoser Camera poser.
 * @return Rotation in degrees.
 */
f32 calcStickRotateDegreeV(const CameraPoser_RS* pPoser) {
    sead::Vector2f stick = sead::Vector2f::zero;
    alCameraPoserFunction::calcCameraRotateStick(&stick, pPoser);
    return sead::Mathf::abs(stick.y) < 0.3f ? 0.0f : -stick.y;
}

/**
 * Checks whether the target is far off the line of sight of the camera.
 * @param pPoser Camera poser.
 * @return Whether the horizontal angle to the target exceeds 30 degrees.
 */
bool isTargetOutOfSight(const CameraPoserBossBattle* pPoser) {
    sead::Vector3f cameraDir = pPoser->getAt() - pPoser->getEye();
    sead::Vector3f targetTrans = sead::Vector3f::zero;
    alCameraPoserFunction::calcTargetTrans(&targetTrans, pPoser);
    sead::Vector3f toTarget = pPoser->getAt() - targetTrans;
    return sead::Mathf::abs(calcAngleOnPlaneDegree(cameraDir, toTarget, sead::Vector3f::ey)) >
           30.0f;
}

class CameraPoserBossBattleNrvTower : public al::Nerve {
public:
    void execute(al::NerveKeeper* pKeeper) const override { getPoser(pKeeper)->exeTower(); }
};

class CameraPoserBossBattleNrvFollow : public al::Nerve {
public:
    void execute(al::NerveKeeper* pKeeper) const override { getPoser(pKeeper)->exeFollow(); }

    void executeOnEnd(al::NerveKeeper* pKeeper) const override {
        getPoser(pKeeper)->endFollow();
    }
};

class CameraPoserBossBattleNrvFollowNear : public al::Nerve {
public:
    void execute(al::NerveKeeper* pKeeper) const override { getPoser(pKeeper)->exeFollowNear(); }
};

NERVES_MAKE_NOSTRUCT(CameraPoserBossBattle, Tower, Follow, FollowNear)
}  // namespace

namespace al {

/**
 * Creates a camera that frames the player together with a boss.
 * @param pName Camera name.
 * @param pPos Position of the boss.
 */
CameraPoserBossBattle::CameraPoserBossBattle(const char* pName, const sead::Vector3f* pPos)
    : CameraPoser_RS(pName), mBossPos(pPos) {}

/**
 * Starts in the tower state.
 */
void CameraPoserBossBattle::init() {
    initNerve(&NrvCameraPoserBossBattleTower, 0);
}

/**
 * Loads the offsets, distances, angles and state change thresholds.
 * @param rIter Camera parameter iterator.
 */
void CameraPoserBossBattle::loadParam(const ByamlIter& rIter) {
    tryGetByamlF32(&mOffsetY, rIter, "OffsetY");
    tryGetByamlF32(&mOffsetYNear, rIter, "OffsetYNear");
    tryGetByamlF32(&mCameraDistanceNear, rIter, "CameraDistanceNear");
    tryGetByamlF32(&mCameraDistanceFar, rIter, "CameraDistanceFar");
    tryGetByamlF32(&mAngleDegreeVNear, rIter, "AngleDegreeVNear");
    tryGetByamlF32(&mAngleDegreeVFar, rIter, "AngleDegreeVFar");
    tryGetByamlF32(&mToFollowDistance, rIter, "ToFollowDistance");
    tryGetByamlF32(&mToFollowCylinderHeight, rIter, "ToFollowCylinderHeight");
    tryGetByamlF32(&mOutOfRangeDistance, rIter, "OutOfRangeDistance");
    tryGetByamlF32(&mOutOfRangeDistanceMax, rIter, "OutOfRangeDistanceMax");
}

/**
 * Takes over the pose of the previous camera and starts in the tower state.
 * @param rInfo Start info.
 */
void CameraPoserBossBattle::start(const CameraStartInfo& rInfo) {
    const sead::LookAtCamera& camera = alCameraPoserFunction::getLookAtCamera(this);
    mStartEye.set(camera.getPos());
    mTowerEye.set(camera.getPos());
    mEye.set(camera.getPos());
    mAt.set(camera.getAt());
    mUp.set(sead::Vector3f::ey);
    mPrevAngleDegreeV = mAngleDegreeV;
    mPrevOffsetY = mOffsetY + mOffsetYAdd;
    setNerve(this, &NrvCameraPoserBossBattleTower);
}

/**
 * Places the camera at the current distance and vertical angle and keeps the target in frame.
 * @param pCamera Camera to write the pose to.
 */
void CameraPoserBossBattle::makeLookAtCamera(sead::LookAtCamera* pCamera) const {
    sead::Vector3f dir = mEye - mAt;
    sead::Vector3f front = dir;
    tryNormalizeOrDirZ(&front);
    rotateVectorDegree(&dir, dir, front.cross(sead::Vector3f::ey), mAngleDegreeV);
    f32 distance = mDistance + mDistanceAdd;
    f32 length = dir.length();

    if (length > 0.0f) {
        dir *= distance / length;
    }

    pCamera->setPos(mAt + dir);

    sead::Vector3f targetTrans = sead::Vector3f::zero;
    alCameraPoserFunction::calcTargetTrans(&targetTrans, this);
    alCameraPoserFunction::makeCameraKeepInFrameV(pCamera, targetTrans, this, 0.0f, 80.0f);
}

/**
 * Does nothing, the camera is updated by its nerves.
 */
void CameraPoserBossBattle::update() {}

/**
 * The player can always rotate this camera.
 * @return true
 */
bool CameraPoserBossBattle::isEnableRotateByPad() const {
    return true;
}

/**
 * Sets the position of the boss.
 * @param pPos Position of the boss.
 */
void CameraPoserBossBattle::setPosPtr(const sead::Vector3f* pPos) {
    mBossPos = pPos;
}

/**
 * Switches to the follow state if the target came close to the boss.
 * @return Whether the state was changed.
 */
bool CameraPoserBossBattle::tryChangeFollowCamera() {
    if (isCameraTargetOutOfRangeY()) {
        return false;
    }

    sead::Vector3f targetTrans = sead::Vector3f::zero;
    alCameraPoserFunction::calcTargetTrans(&targetTrans, this);
    sead::Vector3f toBoss = *mBossPos - targetTrans;
    verticalizeVec(&toBoss, sead::Vector3f::ey, toBoss);

    if (mToFollowDistance < toBoss.length()) {
        return false;
    }

    setNerve(this, &NrvCameraPoserBossBattleFollow);
    return true;
}

/**
 * Checks whether the target is above the follow cylinder below the boss.
 * @return Whether the target is out of range.
 */
bool CameraPoserBossBattle::isCameraTargetOutOfRangeY() const {
    sead::Vector3f targetTrans = sead::Vector3f::zero;
    alCameraPoserFunction::calcTargetTrans(&targetTrans, this);
    return targetTrans.y > mBossPos->y - mToFollowCylinderHeight;
}

/**
 * Switches back to the tower state if the target went away from the boss.
 * @return Whether the state was changed.
 */
bool CameraPoserBossBattle::tryChangeTowerCamera() {
    if (isCameraTargetOutOfRangeY()) {
        return false;
    }

    sead::Vector3f targetTrans = sead::Vector3f::zero;
    alCameraPoserFunction::calcTargetTrans(&targetTrans, this);
    sead::Vector3f toBoss = *mBossPos - targetTrans;
    verticalizeVec(&toBoss, sead::Vector3f::ey, toBoss);
    f32 distance = mToFollowDistance + 50.0f;

    if (toBoss.squaredLength() < distance * distance) {
        return false;
    }

    mStartEye.set(mEye);
    setNerve(this, &NrvCameraPoserBossBattleTower);
    return true;
}

/**
 * Calculates how far the target may move away from the boss horizontally.
 * @return The out of range distance.
 */
f32 CameraPoserBossBattle::calcOutOfRangeDistance() const {
    sead::Vector3f targetTrans;
    alCameraPoserFunction::calcTargetTrans(&targetTrans, this);
    sead::Vector3f toBoss = *mBossPos - targetTrans;
    verticalizeVec(&toBoss, sead::Vector3f::ey, toBoss);
    return sead::Mathf::max(mOutOfRangeDistance, toBoss.length() + 100.0f);
}

/**
 * Looks at the boss from behind the target and widens the view if the target goes away.
 */
void CameraPoserBossBattle::exeTower() {
    f32 rate = calcNerveRate(this, 90);
    f32 offsetY = lerpValue(rate, mPrevOffsetY, mOffsetY + mOffsetYAdd);

    if (isLessStep(this, 90)) {
        mDistance = lerpValue(0.1f, mDistance, mCameraDistanceFar);
    }

    sead::Vector3f prevAt = mAt;
    sead::Vector3f targetTrans = sead::Vector3f::zero;
    alCameraPoserFunction::calcTargetTrans(&targetTrans, this);
    mAt.set(*mBossPos);
    mAt.y -= offsetY;
    sead::Vector3f toBoss = *mBossPos - targetTrans;
    verticalizeVec(&toBoss, sead::Vector3f::ey, toBoss);
    f32 distanceH = toBoss.length();

    sead::Vector3f dir = sead::Vector3f::zero;
    f32 degreeH = calcStickRotateDegreeH(this);

    if (isNearZero(degreeH, 0.001f)) {
        calcDirH(&dir, prevAt, targetTrans);

        if (isNearZero(dir, 0.001f)) {
            dir.set(sead::Vector3f::ez);
        }

        f32 distance = mDistance;
        f32 length = dir.length();

        if (length > 0.0f) {
            dir *= distance / length;
        }
    } else {
        dir = mEye - mAt;
        rotateVectorDegreeY(&dir, degreeH);
    }

    mAngleDegreeV += calcStickRotateDegreeV(this);
    mAngleDegreeV = sead::Mathf::clamp(mAngleDegreeV, 0.0f, 60.0f);

    f32 easeRate = calcNerveEaseOutRate(this, 60);

    if (easeRate < 1.0f || !isNearZero(degreeH, 0.001f)) {
        mTowerDir.set(dir);
    }

    sead::Vector3f towerDir;
    lerpVec(&towerDir, mTowerDir, dir, 0.35f);
    mTowerEye = mAt + towerDir;
    mTowerDir.set(towerDir);

    if (!isNearZero(degreeH, 0.001f)) {
        mStartEye.set(mTowerEye);
    }

    sead::Vector3f eye = sead::Vector3f::zero;
    lerpVec(&eye, mStartEye, mTowerEye, easeRate);
    mEye.set(eye);

    if (isTargetOutOfSight(this)) {
        setNerve(this, &NrvCameraPoserBossBattleFollow);
        return;
    }

    if (tryChangeFollowCamera()) {
        mPrevOffsetY = offsetY;
        mPrevDistanceAdd = mDistanceAdd;
        return;
    }

    f32 outOfRange = (distanceH - mOutOfRangeDistance) * 0.5f;
    mOffsetYAdd = sead::Mathf::min(1200.0f - mOffsetY, outOfRange);
    f32 distanceAddMax = 4500.0f - mDistance;
    mDistanceAdd =
        lerpValue(rate, mPrevDistanceAdd, sead::Mathf::min(distanceAddMax, outOfRange));
}

/**
 * Follows the target from behind when it is close to the boss.
 */
void CameraPoserBossBattle::exeFollow() {
    f32 rate = calcNerveRate(this, 90);
    f32 offsetY = lerpValue(rate, mPrevOffsetY, mOffsetY);

    if (!isNearZero(mPrevOffsetY - mOffsetY, 0.001f) && isLessEqualStep(this, 90)) {
        mAt.set(*mBossPos);
        mAt.y -= offsetY;
    }

    if (isLessStep(this, 90)) {
        mDistance = lerpValue(0.1f, mDistance, mCameraDistanceFar);

        if (mIsInterpoleAngleV) {
            mAngleDegreeV = lerpValue(rate, mPrevAngleDegreeV, mAngleDegreeVFar);
        }
    } else {
        mIsInterpoleAngleV = false;
    }

    sead::Vector3f dir = mEye - mAt;
    rotateVectorDegreeY(&dir, calcStickRotateDegreeH(this));
    f32 degreeV = calcStickRotateDegreeV(this);

    if (mIsInterpoleAngleV && degreeV != 0.0f) {
        mIsInterpoleAngleV = false;
    }

    mAngleDegreeV += degreeV;
    mAngleDegreeV = sead::Mathf::clamp(mAngleDegreeV, 0.0f, 60.0f);
    mEye = mAt + dir;

    if (mIsValidFollowNear) {
        sead::Vector3f targetTrans = sead::Vector3f::zero;
        alCameraPoserFunction::calcTargetTrans(&targetTrans, this);
        sead::Vector3f toBoss = *mBossPos - targetTrans;
        verticalizeVec(&toBoss, sead::Vector3f::ey, toBoss);

        if (!isCameraTargetOutOfRangeY() && toBoss.length() < mToFollowDistance) {
            mPrevOffsetY = offsetY;
            mPrevAngleDegreeV = mAngleDegreeV;
            setNerve(this, &NrvCameraPoserBossBattleFollowNear);
            return;
        }
    }

    if (isTargetOutOfSight(this)) {
        return;
    }

    if (isGreaterStep(this, 15) && tryChangeTowerCamera()) {
        mPrevOffsetY = offsetY;
        mPrevDistanceAdd = mDistanceAdd;
    }
}

/**
 * Stops interpolating the vertical angle.
 */
void CameraPoserBossBattle::endFollow() {
    mIsInterpoleAngleV = false;
}

/**
 * Follows the target from a closer position when it is right below the boss.
 */
void CameraPoserBossBattle::exeFollowNear() {
    f32 rate = calcNerveRate(this, 20);
    f32 offsetY = lerpValue(rate, mOffsetY, mOffsetYNear);

    if (isLessEqualStep(this, 20)) {
        mAt.set(*mBossPos);
        mAt.y -= offsetY;
        mDistance = lerpValue(rate, mDistance, mCameraDistanceNear);
        mAngleDegreeV = lerpValue(rate, mPrevAngleDegreeV, mAngleDegreeVNear);
    }

    sead::Vector3f dir = mEye - mAt;
    rotateVectorDegreeY(&dir, calcStickRotateDegreeH(this));
    mAngleDegreeV += calcStickRotateDegreeV(this);
    mAngleDegreeV = sead::Mathf::clamp(mAngleDegreeV, 0.0f, 60.0f);
    mEye = mAt + dir;

    if (tryChangeTowerCamera()) {
        mPrevOffsetY = offsetY;
        return;
    }

    sead::Vector3f targetTrans = sead::Vector3f::zero;
    alCameraPoserFunction::calcTargetTrans(&targetTrans, this);
    sead::Vector3f toBoss = *mBossPos - targetTrans;
    verticalizeVec(&toBoss, sead::Vector3f::ey, toBoss);

    if (!mIsValidFollowNear || mToFollowDistance + 50.0f < toBoss.length()) {
        mPrevAngleDegreeV = mAngleDegreeV;
        mPrevOffsetY = offsetY;
        mIsInterpoleAngleV = true;
        setNerve(this, &NrvCameraPoserBossBattleFollow);
    }
}

}  // namespace al
