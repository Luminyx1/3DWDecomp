#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
class KeyPoseKeeper;
class LiveActor;
struct PlacementInfo;

KeyPoseKeeper* createKeyPoseKeeper(const ActorInitInfo& rInfo);
KeyPoseKeeper* createKeyPoseKeeper(const PlacementInfo& rInfo);
void resetKeyPose(KeyPoseKeeper* pKeyPoseKeeper);
void nextKeyPose(KeyPoseKeeper* pKeyPoseKeeper);
void restartKeyPose(KeyPoseKeeper* pKeyPoseKeeper, sead::Vector3f* pTrans, sead::Quatf* pQuat);
void reverseKeyPose(KeyPoseKeeper* pKeyPoseKeeper);
const sead::Vector3f& getCurrentKeyTrans(const KeyPoseKeeper* pKeyPoseKeeper);
const sead::Vector3f& getNextKeyTrans(const KeyPoseKeeper* pKeyPoseKeeper);
const sead::Quatf& getCurrentKeyQuat(const KeyPoseKeeper* pKeyPoseKeeper);
const sead::Quatf& getNextKeyQuat(const KeyPoseKeeper* pKeyPoseKeeper);
const PlacementInfo& getCurrentKeyPlacementInfo(const KeyPoseKeeper* pKeyPoseKeeper);
const PlacementInfo& getNextKeyPlacementInfo(const KeyPoseKeeper* pKeyPoseKeeper);
s32 getKeyPoseCount(const KeyPoseKeeper* pKeyPoseKeeper);
void getKeyPoseTrans(sead::Vector3f* pOut, const KeyPoseKeeper* pKeyPoseKeeper, s32 idx);
void getKeyPoseQuat(sead::Quatf* pOut, const KeyPoseKeeper* pKeyPoseKeeper, s32 idx);
void calcLerpKeyTrans(sead::Vector3f* pOut, const KeyPoseKeeper* pKeyPoseKeeper, f32 rate);
void calcSlerpKeyQuat(sead::Quatf* pOut, const KeyPoseKeeper* pKeyPoseKeeper, f32 rate);
bool isMoveSignKey(const KeyPoseKeeper* pKeyPoseKeeper);
bool isLastKey(const KeyPoseKeeper* pKeyPoseKeeper);
bool isFirstKey(const KeyPoseKeeper* pKeyPoseKeeper);
bool isGoingToEnd(const KeyPoseKeeper* pKeyPoseKeeper);
bool isStop(const KeyPoseKeeper* pKeyPoseKeeper);
bool isRestart(const KeyPoseKeeper* pKeyPoseKeeper);
f32 calcDistanceNextKeyTrans(const KeyPoseKeeper* pKeyPoseKeeper);
s32 calcTimeToNextKeyMove(const KeyPoseKeeper* pKeyPoseKeeper, f32 speed);
void calcDirToNextKey(sead::Vector3f* pOut, const KeyPoseKeeper* pKeyPoseKeeper);
f32 calcKeyMoveSpeed(const KeyPoseKeeper* pKeyPoseKeeper);
f32 calcKeyMoveSpeedByTime(const KeyPoseKeeper* pKeyPoseKeeper);
s32 calcKeyMoveWaitTime(const KeyPoseKeeper* pKeyPoseKeeper);
s32 calcKeyMoveMoveTime(const KeyPoseKeeper* pKeyPoseKeeper);
void calcKeyMoveClippingInfo(sead::Vector3f* pPos, f32* pRadius,
                             const KeyPoseKeeper* pKeyPoseKeeper, f32 offset);
void setKeyMoveClippingInfo(LiveActor* pActor, sead::Vector3f* pPos,
                            const KeyPoseKeeper* pKeyPoseKeeper);
}  // namespace al
