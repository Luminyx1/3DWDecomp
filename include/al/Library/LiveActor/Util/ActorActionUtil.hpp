#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class HitSensor;
class LiveActor;
class Nerve;

void startAction(LiveActor* pActor, const char* pActionName);
s32 startActionAtRandomFrame(LiveActor* pActor, const char* pActionName);
bool tryStartAction(LiveActor* pActor, const char* pActionName);
bool tryStartActionIfNotPlaying(LiveActor* pActor, const char* pActionName);
bool isActionPlaying(const LiveActor* pActor, const char* pActionName);
void tryStartActionNoAnim(LiveActor* pActor, const char* pActionName);
void tryStartEffectAction(LiveActor* pActor, const char* pActionName);
bool isActionEnd(const LiveActor* pActor);
bool isExistAction(const LiveActor* pActor);
bool isExistAction(const LiveActor* pActor, const char* pActionName);
bool isActionOneTime(const LiveActor* pActor, const char* pActionName);
f32 getActionFrame(const LiveActor* pActor);
f32 getActionFrameMax(const LiveActor* pActor, const char* pActionName);
f32 getActionFrameRate(const LiveActor* pActor);
const char* getActionName(const LiveActor* pActor);
void setActionFrame(LiveActor* pActor, f32 frame);
void trySetActionFrame(LiveActor* pActor, f32 frame);
void setActionFrameRate(LiveActor* pActor, f32 frameRate);
void trySetActionFrameRate(LiveActor* pActor, f32 frameRate);
void stopAction(LiveActor* pActor);
void restartAction(LiveActor* pActor);
void tryUpdateSeEffect(LiveActor* pActor, f32 frameFrom, f32 frameTo);
void copyAction(LiveActor* pActor, const LiveActor* pSrcActor);
void startNerveAction(LiveActor* pActor, const char* pActionName);
void setNerveAtActionEnd(LiveActor* pActor, const Nerve* pNerve);
void resetNerveActionForInit(LiveActor* pActor);
void startHitReaction(const LiveActor* pActor, const char* pName);
void startHitReactionHitEffect(const LiveActor* pActor, const char* pName, const HitSensor* pOther, const HitSensor* pSelf);
void startHitReactionHitEffect(const LiveActor* pActor, const char* pName, const sead::Vector3f& rPos);
void startHitReactionHitEffect(const LiveActor* pActor, const char* pName, const sead::Matrix34f* pMtx);
void startHitReactionBlowHit(const LiveActor* pActor, const HitSensor* pOther, const HitSensor* pSelf);
void startHitReactionBlowHit(const LiveActor* pActor, const sead::Vector3f& rPos);
void startHitReactionBlowHit(const LiveActor* pActor);
void startHitReactionBlowHitDirect(const LiveActor* pActor, const HitSensor* pOther, const HitSensor* pSelf);
void startHitReactionBlowHitDirect(const LiveActor* pActor, const sead::Vector3f& rPos);
void startHitReactionBlowHitDirect(const LiveActor* pActor);
void startHitReactionAppear(const LiveActor* pActor);
void startHitReactionDisappear(const LiveActor* pActor);
void startHitReactionBreak(const LiveActor* pActor);
void startHitReactionDeath(const LiveActor* pActor);
void startHitReactionGet(const LiveActor* pActor);
void startHitReactionStart(const LiveActor* pActor);
void startHitReactionEnd(const LiveActor* pActor);
void startHitReactionHit(const LiveActor* pActor);
void startHitReactionExplode(const LiveActor* pActor);
void startHitReactionOnGround(const LiveActor* pActor);
void startHitReactionPressDown(const LiveActor* pActor);
}  // namespace al
