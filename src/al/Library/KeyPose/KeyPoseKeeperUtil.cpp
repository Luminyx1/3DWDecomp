#include "Library/KeyPose/KeyPoseKeeperUtil.hpp"

#include <math/seadBoundBox.h>

#include "Library/KeyPose/KeyPoseKeeper.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Project/Joint/KeyPose.hpp"

namespace al {
KeyPoseKeeper* createKeyPoseKeeper(const ActorInitInfo& info) {
    KeyPoseKeeper* keyPoseKeeper = new KeyPoseKeeper();
    keyPoseKeeper->init(info);

    return keyPoseKeeper;
}

void resetKeyPose(KeyPoseKeeper* keyPoseKeeper) {
    keyPoseKeeper->reset();
}

void nextKeyPose(KeyPoseKeeper* keyPoseKeeper) {
    keyPoseKeeper->next();
}

void restartKeyPose(KeyPoseKeeper* keyPoseKeeper, sead::Vector3f* pos, sead::Quatf* orientation) {
    resetKeyPose(keyPoseKeeper);

    const KeyPose& keyPose = keyPoseKeeper->getKeyPose(0);

    if (pos)
        pos->set(keyPose.getTrans());

    if (orientation)
        orientation->set(keyPose.getQuat());
}


const sead::Vector3f& getCurrentKeyTrans(const KeyPoseKeeper* keyPoseKeeper) {
    return keyPoseKeeper->getCurrentKeyPose().getTrans();
}

const sead::Vector3f& getNextKeyTrans(const KeyPoseKeeper* keyPoseKeeper) {
    return keyPoseKeeper->getNextKeyPose().getTrans();
}

const sead::Quatf& getCurrentKeyQuat(const KeyPoseKeeper* keyPoseKeeper) {
    return keyPoseKeeper->getCurrentKeyPose().getQuat();
}

const sead::Quatf& getNextKeyQuat(const KeyPoseKeeper* keyPoseKeeper) {
    return keyPoseKeeper->getNextKeyPose().getQuat();
}

const PlacementInfo& getCurrentKeyPlacementInfo(const KeyPoseKeeper* keyPoseKeeper) {
    return keyPoseKeeper->getCurrentKeyPose().getPlacementInfo();
}

const PlacementInfo& getNextKeyPlacementInfo(const KeyPoseKeeper* keyPoseKeeper) {
    return keyPoseKeeper->getNextKeyPose().getPlacementInfo();
}

s32 getKeyPoseCount(const KeyPoseKeeper* keyPoseKeeper) {
    return keyPoseKeeper->getKeyPoseCount();
}

void getKeyPoseTrans(sead::Vector3f* out, const KeyPoseKeeper* keyPoseKeeper, s32 idx) {
    out->set(keyPoseKeeper->getKeyPose(idx).getTrans());
}

void getKeyPoseQuat(sead::Quatf* out, const KeyPoseKeeper* keyPoseKeeper, s32 idx) {
    out->set(keyPoseKeeper->getKeyPose(idx).getQuat());
}

void calcLerpKeyTrans(sead::Vector3f* out, const KeyPoseKeeper* keyPoseKeeper, f32 rate) {
    const KeyPose& current = keyPoseKeeper->getCurrentKeyPose();
    const KeyPose& next = keyPoseKeeper->getNextKeyPose();

    s32 interpolateType = 0;
    tryGetArg(&interpolateType, current.getPlacementInfo(), "InterpolateType");

    lerpVec(out, current.getTrans(), next.getTrans(), easeByType(rate, interpolateType));
}

void calcSlerpKeyQuat(sead::Quatf* out, const KeyPoseKeeper* keyPoseKeeper, f32 rate) {
    const KeyPose& current = keyPoseKeeper->getCurrentKeyPose();
    const KeyPose& next = keyPoseKeeper->getNextKeyPose();

    s32 interpolateType = 0;
    tryGetArg(&interpolateType, current.getPlacementInfo(), "InterpolateType");

    slerpQuat(out, current.getQuat(), next.getQuat(), easeByType(rate, interpolateType));
}

bool isMoveSignKey(const KeyPoseKeeper* keyPoseKeeper) {
    bool isPlaySign = false;
    tryGetArg(&isPlaySign, getCurrentKeyPlacementInfo(keyPoseKeeper), "IsPlaySign");

    return isPlaySign;
}

bool isLastKey(const KeyPoseKeeper* keyPoseKeeper) {
    return keyPoseKeeper->isLastKey();
}

bool isFirstKey(const KeyPoseKeeper* keyPoseKeeper) {
    return keyPoseKeeper->isFirstKey();
}


bool isStop(const KeyPoseKeeper* keyPoseKeeper) {
    return keyPoseKeeper->isStop();
}

bool isRestart(const KeyPoseKeeper* keyPoseKeeper) {
    return keyPoseKeeper->isRestart();
}

f32 calcDistanceNextKeyTrans(const KeyPoseKeeper* keyPoseKeeper) {
    return (getCurrentKeyTrans(keyPoseKeeper) - getNextKeyTrans(keyPoseKeeper)).length();
}

s32 calcTimeToNextKeyMove(const KeyPoseKeeper* keyPoseKeeper, f32 speed) {
    return sead::Mathi::clampMin((s32)(calcDistanceNextKeyTrans(keyPoseKeeper) / speed), 1);
}

void calcDirToNextKey(sead::Vector3f* out, const KeyPoseKeeper* keyPoseKeeper) {
    const sead::Vector3f& currTrans = getCurrentKeyTrans(keyPoseKeeper);
    const sead::Vector3f& nextTrans = getNextKeyTrans(keyPoseKeeper);

    out->x = nextTrans.x - currTrans.x;
    out->y = nextTrans.y - currTrans.y;
    out->z = nextTrans.z - currTrans.z;

    if (normalizeOrZero(out)) {
        out->set(sead::Vector3f::ez);
    }
}

f32 calcKeyMoveSpeed(const KeyPoseKeeper* keyPoseKeeper) {
    f32 speed = -1.0f;
    tryGetArg(&speed, getCurrentKeyPlacementInfo(keyPoseKeeper), "Speed");

    if (speed < 0.0f)
        return -1.0f;

    return speed;
}

f32 calcKeyMoveSpeedByTime(const KeyPoseKeeper* keyPoseKeeper) {
    s32 speed = -1;
    tryGetArg(&speed, getCurrentKeyPlacementInfo(keyPoseKeeper), "SpeedByTime");

    if (speed < 0.0f)
        return -1.0f;

    return speed;
}

s32 calcKeyMoveWaitTime(const KeyPoseKeeper* keyPoseKeeper) {
    s32 waitTime = -1;
    tryGetArg(&waitTime, getCurrentKeyPlacementInfo(keyPoseKeeper), "WaitTime");

    return sead::Mathi::max(waitTime, -1);
}

s32 calcKeyMoveMoveTime(const KeyPoseKeeper* keyPoseKeeper) {
    s32 t = (s32)calcKeyMoveSpeedByTime(keyPoseKeeper);
    if (t >= 1)
        return t;

    f32 v = calcKeyMoveSpeed(keyPoseKeeper);
    return v > 0.0f ? calcTimeToNextKeyMove(keyPoseKeeper, v) : 60;
}

f32 getClippingRadius(const LiveActor* pActor);
void setClippingInfo(LiveActor* pActor, f32 radius, const sead::Vector3f* pPos);

void calcKeyMoveClippingInfo(sead::Vector3f* pPos, f32* pRadius, const KeyPoseKeeper* pKeeper,
                             f32 offset) {
    s32 count = pKeeper->getKeyPoseCount();
    sead::BoundBox3f box;
    box.setUndef();
    for (s32 i = 0; i < count; i++) {
        box.addPoint(pKeeper->getKeyPose(i).getTrans());
    }
    box.getCenter(pPos);
    *pRadius = (box.getMax() - box.getMin()).length() * 0.5f + offset;
}

void setKeyMoveClippingInfo(LiveActor* pActor, sead::Vector3f* pPos,
                            const KeyPoseKeeper* pKeeper) {
    f32 radius = getClippingRadius(pActor);
    s32 count = pKeeper->getKeyPoseCount();
    sead::BoundBox3f box;
    box.setUndef();
    for (s32 i = 0; i < count; i++) {
        box.addPoint(pKeeper->getKeyPose(i).getTrans());
    }
    *pPos = box.getCenter();
    setClippingInfo(pActor, radius + (box.getMax() - box.getMin()).length() * 0.5f, pPos);
}
}  // namespace al
