#include "Library/KeyPose/KeyPoseKeeperUtil.hpp"

#include <math/seadBoundBox.h>

#include "Library/KeyPose/KeyPoseKeeper.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Project/Joint/KeyPose.hpp"

namespace al {

/**
 * Creates and initializes a key pose keeper for an actor.
 * @param rInfo Init info of the owning actor.
 * @return The new keeper.
 */
KeyPoseKeeper* createKeyPoseKeeper(const ActorInitInfo& rInfo) {
    KeyPoseKeeper* keyPoseKeeper = new KeyPoseKeeper();
    keyPoseKeeper->init(rInfo);

    return keyPoseKeeper;
}

/**
 * Returns the keeper to its first key pose.
 * @param pKeyPoseKeeper The keeper.
 */
void resetKeyPose(KeyPoseKeeper* pKeyPoseKeeper) {
    pKeyPoseKeeper->reset();
}

/**
 * Advances the keeper to its next key pose.
 * @param pKeyPoseKeeper The keeper.
 */
void nextKeyPose(KeyPoseKeeper* pKeyPoseKeeper) {
    pKeyPoseKeeper->next();
}

/**
 * Resets the keeper and reads the first key pose.
 * @param pKeyPoseKeeper The keeper.
 * @param pTrans Receives the first translation; may be nullptr.
 * @param pQuat Receives the first rotation; may be nullptr.
 */
void restartKeyPose(KeyPoseKeeper* pKeyPoseKeeper, sead::Vector3f* pTrans, sead::Quatf* pQuat) {
    resetKeyPose(pKeyPoseKeeper);

    const KeyPose& keyPose = pKeyPoseKeeper->getKeyPose(0);

    if (pTrans != nullptr) {
        pTrans->set(keyPose.getTrans());
    }

    if (pQuat != nullptr) {
        pQuat->set(keyPose.getQuat());
    }
}

/**
 * @param pKeyPoseKeeper The keeper.
 * @return Translation of the current key pose.
 */
const sead::Vector3f& getCurrentKeyTrans(const KeyPoseKeeper* pKeyPoseKeeper) {
    return pKeyPoseKeeper->getCurrentKeyPose().getTrans();
}

/**
 * @param pKeyPoseKeeper The keeper.
 * @return Translation of the next key pose.
 */
const sead::Vector3f& getNextKeyTrans(const KeyPoseKeeper* pKeyPoseKeeper) {
    return pKeyPoseKeeper->getNextKeyPose().getTrans();
}

/**
 * @param pKeyPoseKeeper The keeper.
 * @return Rotation of the current key pose.
 */
const sead::Quatf& getCurrentKeyQuat(const KeyPoseKeeper* pKeyPoseKeeper) {
    return pKeyPoseKeeper->getCurrentKeyPose().getQuat();
}

/**
 * @param pKeyPoseKeeper The keeper.
 * @return Rotation of the next key pose.
 */
const sead::Quatf& getNextKeyQuat(const KeyPoseKeeper* pKeyPoseKeeper) {
    return pKeyPoseKeeper->getNextKeyPose().getQuat();
}

/**
 * @param pKeyPoseKeeper The keeper.
 * @return Placement of the current key pose.
 */
const PlacementInfo& getCurrentKeyPlacementInfo(const KeyPoseKeeper* pKeyPoseKeeper) {
    return pKeyPoseKeeper->getCurrentKeyPose().getPlacementInfo();
}

/**
 * @param pKeyPoseKeeper The keeper.
 * @return Placement of the next key pose.
 */
const PlacementInfo& getNextKeyPlacementInfo(const KeyPoseKeeper* pKeyPoseKeeper) {
    return pKeyPoseKeeper->getNextKeyPose().getPlacementInfo();
}

/**
 * @param pKeyPoseKeeper The keeper.
 * @return Number of key poses.
 */
s32 getKeyPoseCount(const KeyPoseKeeper* pKeyPoseKeeper) {
    return pKeyPoseKeeper->getKeyPoseCount();
}

/**
 * Reads the translation of a key pose.
 * @param pOut Receives the translation.
 * @param pKeyPoseKeeper The keeper.
 * @param idx Index of the key pose.
 */
void getKeyPoseTrans(sead::Vector3f* pOut, const KeyPoseKeeper* pKeyPoseKeeper, s32 idx) {
    pOut->set(pKeyPoseKeeper->getKeyPose(idx).getTrans());
}

/**
 * Reads the rotation of a key pose.
 * @param pOut Receives the rotation.
 * @param pKeyPoseKeeper The keeper.
 * @param idx Index of the key pose.
 */
void getKeyPoseQuat(sead::Quatf* pOut, const KeyPoseKeeper* pKeyPoseKeeper, s32 idx) {
    pOut->set(pKeyPoseKeeper->getKeyPose(idx).getQuat());
}

/**
 * Interpolates the translation between the current and next key pose, eased by the current key's
 * "InterpolateType".
 * @param pOut Receives the translation.
 * @param pKeyPoseKeeper The keeper.
 * @param rate Progress from the current to the next key pose (0-1).
 */
void calcLerpKeyTrans(sead::Vector3f* pOut, const KeyPoseKeeper* pKeyPoseKeeper, f32 rate) {
    const KeyPose& current = pKeyPoseKeeper->getCurrentKeyPose();
    const KeyPose& next = pKeyPoseKeeper->getNextKeyPose();

    s32 interpolateType = 0;
    tryGetArg(&interpolateType, current.getPlacementInfo(), "InterpolateType");

    lerpVec(pOut, current.getTrans(), next.getTrans(), easeByType(rate, interpolateType));
}

/**
 * Interpolates the rotation between the current and next key pose, eased by the current key's
 * "InterpolateType".
 * @param pOut Receives the rotation.
 * @param pKeyPoseKeeper The keeper.
 * @param rate Progress from the current to the next key pose (0-1).
 */
void calcSlerpKeyQuat(sead::Quatf* pOut, const KeyPoseKeeper* pKeyPoseKeeper, f32 rate) {
    const KeyPose& current = pKeyPoseKeeper->getCurrentKeyPose();
    const KeyPose& next = pKeyPoseKeeper->getNextKeyPose();

    s32 interpolateType = 0;
    tryGetArg(&interpolateType, current.getPlacementInfo(), "InterpolateType");

    slerpQuat(pOut, current.getQuat(), next.getQuat(), easeByType(rate, interpolateType));
}

/**
 * @param pKeyPoseKeeper The keeper.
 * @return Whether the current key has "IsPlaySign" set.
 */
bool isMoveSignKey(const KeyPoseKeeper* pKeyPoseKeeper) {
    bool isPlaySign = false;
    tryGetArg(&isPlaySign, getCurrentKeyPlacementInfo(pKeyPoseKeeper), "IsPlaySign");

    return isPlaySign;
}

/**
 * @param pKeyPoseKeeper The keeper.
 * @return Whether the current key pose is the last one in the current direction.
 */
bool isLastKey(const KeyPoseKeeper* pKeyPoseKeeper) {
    return pKeyPoseKeeper->isLastKey();
}

/**
 * @param pKeyPoseKeeper The keeper.
 * @return Whether the current key pose is the first one.
 */
bool isFirstKey(const KeyPoseKeeper* pKeyPoseKeeper) {
    return pKeyPoseKeeper->isFirstKey();
}

/**
 * @param pKeyPoseKeeper The keeper.
 * @return Whether the keeper stopped at its last key pose.
 */
bool isStop(const KeyPoseKeeper* pKeyPoseKeeper) {
    return pKeyPoseKeeper->isStop();
}

/**
 * @param pKeyPoseKeeper The keeper.
 * @return Whether the keeper requested a restart at its last key pose.
 */
bool isRestart(const KeyPoseKeeper* pKeyPoseKeeper) {
    return pKeyPoseKeeper->isRestart();
}

/**
 * @param pKeyPoseKeeper The keeper.
 * @return Distance between the current and next key translations.
 */
f32 calcDistanceNextKeyTrans(const KeyPoseKeeper* pKeyPoseKeeper) {
    return (getCurrentKeyTrans(pKeyPoseKeeper) - getNextKeyTrans(pKeyPoseKeeper)).length();
}

/**
 * @param pKeyPoseKeeper The keeper.
 * @param speed Movement speed per frame.
 * @return Frames needed to reach the next key at the given speed (at least 1).
 */
s32 calcTimeToNextKeyMove(const KeyPoseKeeper* pKeyPoseKeeper, f32 speed) {
    return sead::Mathi::clampMin(static_cast<s32>(calcDistanceNextKeyTrans(pKeyPoseKeeper) / speed),
                                 1);
}

/**
 * Calculates the direction from the current to the next key translation.
 * @param pOut Receives the normalized direction, or the Z axis if both keys coincide.
 * @param pKeyPoseKeeper The keeper.
 */
void calcDirToNextKey(sead::Vector3f* pOut, const KeyPoseKeeper* pKeyPoseKeeper) {
    const sead::Vector3f& currentTrans = getCurrentKeyTrans(pKeyPoseKeeper);
    const sead::Vector3f& nextTrans = getNextKeyTrans(pKeyPoseKeeper);

    pOut->x = nextTrans.x - currentTrans.x;
    pOut->y = nextTrans.y - currentTrans.y;
    pOut->z = nextTrans.z - currentTrans.z;

    if (normalizeOrZero(pOut)) {
        pOut->set(sead::Vector3f::ez);
    }
}

/**
 * @param pKeyPoseKeeper The keeper.
 * @return The current key's "Speed", or -1 if unset or negative.
 */
f32 calcKeyMoveSpeed(const KeyPoseKeeper* pKeyPoseKeeper) {
    f32 speed = -1.0f;
    tryGetArg(&speed, getCurrentKeyPlacementInfo(pKeyPoseKeeper), "Speed");

    if (speed < 0.0f) {
        return -1.0f;
    }

    return speed;
}

/**
 * @param pKeyPoseKeeper The keeper.
 * @return The current key's "SpeedByTime", or -1 if unset or negative.
 */
f32 calcKeyMoveSpeedByTime(const KeyPoseKeeper* pKeyPoseKeeper) {
    s32 speedByTime = -1;
    tryGetArg(&speedByTime, getCurrentKeyPlacementInfo(pKeyPoseKeeper), "SpeedByTime");

    if (speedByTime < 0.0f) {
        return -1.0f;
    }

    return speedByTime;
}

/**
 * @param pKeyPoseKeeper The keeper.
 * @return The current key's "WaitTime", at least -1.
 */
s32 calcKeyMoveWaitTime(const KeyPoseKeeper* pKeyPoseKeeper) {
    s32 waitTime = -1;
    tryGetArg(&waitTime, getCurrentKeyPlacementInfo(pKeyPoseKeeper), "WaitTime");

    return sead::Mathi::max(waitTime, -1);
}

/**
 * Calculates the frames needed to move to the next key, from "SpeedByTime" or "Speed".
 * @param pKeyPoseKeeper The keeper.
 * @return The move time in frames; 60 if neither parameter is set.
 */
s32 calcKeyMoveMoveTime(const KeyPoseKeeper* pKeyPoseKeeper) {
    s32 moveTime = static_cast<s32>(calcKeyMoveSpeedByTime(pKeyPoseKeeper));

    if (moveTime >= 1) {
        return moveTime;
    }

    f32 speed = calcKeyMoveSpeed(pKeyPoseKeeper);
    return speed > 0.0f ? calcTimeToNextKeyMove(pKeyPoseKeeper, speed) : 60;
}

/**
 * Calculates a clipping sphere enclosing all key translations.
 * @param pPos Receives the sphere center.
 * @param pRadius Receives the sphere radius.
 * @param pKeyPoseKeeper The keeper.
 * @param offset Extra radius added on top.
 */
void calcKeyMoveClippingInfo(sead::Vector3f* pPos, f32* pRadius,
                             const KeyPoseKeeper* pKeyPoseKeeper, f32 offset) {
    s32 count = pKeyPoseKeeper->getKeyPoseCount();
    sead::BoundBox3f box;

    for (s32 i = 0; i < count; i++) {
        box.addPoint(pKeyPoseKeeper->getKeyPose(i).getTrans());
    }

    *pPos = box.getCenter();
    *pRadius = (box.getMax() - box.getMin()).length() * 0.5f + offset;
}

/**
 * Sets the actor's clipping sphere to enclose all key translations.
 * @param pActor The actor.
 * @param pPos Receives the sphere center, which the actor's clipping keeps pointing at.
 * @param pKeyPoseKeeper The keeper.
 */
void setKeyMoveClippingInfo(LiveActor* pActor, sead::Vector3f* pPos,
                            const KeyPoseKeeper* pKeyPoseKeeper) {
    f32 radius = 0.0f;
    calcKeyMoveClippingInfo(pPos, &radius, pKeyPoseKeeper, getClippingRadius(pActor));
    setClippingInfo(pActor, radius, pPos);
}

}  // namespace al
