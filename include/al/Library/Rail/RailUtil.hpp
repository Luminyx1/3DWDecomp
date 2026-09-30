#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class IUseRail;
class LiveActor;
class RailKeeper;
struct PlacementInfo;

void setRailPosToStart(const LiveActor* pActor);
void setRailPosToEnd(const LiveActor* pActor);
void setRailPosToNearestPos(const LiveActor* pActor, const sead::Vector3f& rPos);
void setRailPosToCoord(const LiveActor* pActor, f32 coord);
void setRailPosToRailPoint(const LiveActor* pActor, s32 index);
void setSyncRailToStart(LiveActor* pActor);
void syncRailTrans(LiveActor* pActor);
void setSyncRailToEnd(LiveActor* pActor);
void setSyncRailToNearestPos(LiveActor* pActor, const sead::Vector3f& rPos);
void setSyncRailToNearestPos(LiveActor* pActor);
void setSyncRailToCoord(LiveActor* pActor, f32 coord);
void setSyncRailToRailPoint(LiveActor* pActor, s32 index);
bool moveRail(const LiveActor* pActor, f32 speed);
bool isRailReachedGoal(const LiveActor* pActor);
bool moveRailLoop(const LiveActor* pActor, f32 speed);
f32 getRailCoord(const LiveActor* pActor);
bool isRailGoingToEnd(const LiveActor* pActor);
f32 getRailTotalLength(const LiveActor* pActor);
bool moveRailTurn(const LiveActor* pActor, f32 speed, f32 goalCoord);
void reverseRail(const LiveActor* pActor);
bool isRailReachedNearGoal(const LiveActor* pActor, f32 goalCoord);
bool moveRailPause(const LiveActor* pActor, f32 speed);
s32 getRailPartIndex(const LiveActor* pActor);
bool isRailReachedNearEndRailPoint(const LiveActor* pActor, f32 margin);
bool turnToRailDir(LiveActor* pActor, f32 degree);
const sead::Vector3f& getRailDir(const LiveActor* pActor);
bool turnToRailDirImmediately(LiveActor* pActor);
const sead::Vector3f& getRailPos(const LiveActor* pActor);
void syncRailTransOffset(LiveActor* pActor, const sead::Vector3f& rOffset);
bool moveSyncRail(LiveActor* pActor, f32 speed);
bool moveSyncRailOffset(LiveActor* pActor, f32 speed, const sead::Vector3f& rOffset);
bool moveSyncRailLoop(LiveActor* pActor, f32 speed);
bool moveSyncRailTurn(LiveActor* pActor, f32 speed);
bool moveSyncRailPause(LiveActor* pActor, f32 speed);
f32 calcNearestRailCoord(const LiveActor* pActor, const sead::Vector3f& rPos);
f32 calcNearestRailCoord(const RailKeeper* pRailKeeper, const sead::Vector3f& rPos);
f32 calcNearestRailPos(sead::Vector3f* pRailPos, const LiveActor* pActor,
                       const sead::Vector3f& rPos);
f32 calcNearestRailPos(sead::Vector3f* pRailPos, const RailKeeper* pRailKeeper,
                       const sead::Vector3f& rPos);
s32 calcRailPointNum(const LiveActor* pActor, f32 coordStart, f32 coordEnd);
void calcRailPointPos(sead::Vector3f* pPos, const LiveActor* pActor, s32 index);
f32 calcRailToGoalLength(const LiveActor* pActor);
f32 calcRailPartRate(const LiveActor* pActor);
f32 calcRailToNextRailPointLength(const LiveActor* pActor);
f32 calcRailToPreviousRailPointLength(const LiveActor* pActor);
s32 getRailNum(const LiveActor* pActor);
s32 getRailPointNum(const LiveActor* pActor);
s32 getRailPointNum(const RailKeeper* pRailKeeper);
const sead::Vector3f& getRailPos(const RailKeeper* pRailKeeper);
void getRailUpDir(const LiveActor* pActor, sead::Vector3f* pUp);
f32 getRailPartLength(const LiveActor* pActor, s32 index);
s32 getRailPointNo(const LiveActor* pActor);
bool isLoopRail(const LiveActor* pActor);
bool isRailReachedEnd(const LiveActor* pActor);
void getRailPartAccels(const LiveActor* pActor, s32 index, f32* pAccelStart, f32* pAccelEnd);
bool getRailPartAngleS(const LiveActor* pActor, s32 index, f32* pAngle);
bool getRailPartAngleE(const LiveActor* pActor, s32 index, f32* pAngle);
f32 getRailPartRate(const LiveActor* pActor, s32 index, f32 coord);
bool isExistRail(const LiveActor* pActor);
bool isRailReachedStart(const LiveActor* pActor);
bool isRailReachedNearGoal(const LiveActor* pActor, f32 goalMarginEnd, f32 goalMarginStart);
bool isRailReachedEdge(const LiveActor* pActor);
bool isRailReachedNearRailPoint(const LiveActor* pActor, f32 margin);
bool isRailReachedNearStartRailPoint(const LiveActor* pActor, f32 margin);
bool isRailPlusDir(const LiveActor* pActor, const sead::Vector3f& rDir);
bool isRailPlusPoseSide(const LiveActor* pActor);
bool isRailPlusPoseUp(const LiveActor* pActor);
bool isRailPlusPoseFront(const LiveActor* pActor);
void calcRailPosAtCoord(sead::Vector3f* pPos, const LiveActor* pActor, f32 coord);
void calcRailMoveDir(sead::Vector3f* pDir, const LiveActor* pActor);
void calcRailDirAtCoord(sead::Vector3f* pDir, const LiveActor* pActor, f32 coord);
void calcRailDirAtCoord(sead::Vector3f* pDir, const RailKeeper* pRailKeeper, f32 coord);
void calcRailPosFront(sead::Vector3f* pPos, const LiveActor* pActor, f32 offset);
f32 calcRailCoordByPoint(const LiveActor* pActor, s32 index);
void calcRailClippingInfo(sead::Vector3f* pPos, f32* pRadius, const LiveActor* pActor, f32 step,
                          f32 offset);
void calcRailClippingInfo(sead::Vector3f* pPos, f32* pRadius, const RailKeeper* pRailKeeper,
                          f32 step, f32 offset);
void setRailClippingInfo(sead::Vector3f* pPos, LiveActor* pActor, f32 step, f32 offset);
void setRailClippingInfo(sead::Vector3f* pPos, LiveActor* pActor, const RailKeeper* pRailKeeper,
                         f32 step, f32 offset);
s32 getRailPointNum(const IUseRail* pRailHolder);
PlacementInfo* getRailPointInfo(const IUseRail* pRailHolder, s32 index);
}  // namespace al
