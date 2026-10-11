#pragma once
#include <math/seadQuat.h>
namespace al { class LiveActor; }

class BrosMoveStepKeeper;

namespace rc {
bool isCurrentStepKuribo(const BrosMoveStepKeeper* pMoveStepKeeper);
bool isCurrentStepPoseFixed(const BrosMoveStepKeeper* pMoveStepKeeper);
const sead::Quatf& getCurrentKeyQuat(const BrosMoveStepKeeper* pMoveStepKeeper);
bool isTurnToDefaultDirectionCurrent(const al::LiveActor* pActor, const BrosMoveStepKeeper* pMoveStepKeeper);
bool isFaceToDefaultDirectionCurrent(const al::LiveActor* pActor, const BrosMoveStepKeeper* pMoveStepKeeper);
void turnToTargetOrDefaultDirectionCurrent(al::LiveActor* pActor, const BrosMoveStepKeeper* pMoveStepKeeper, f32 degree);
bool tryRequestSupportFreezeSyncCurrentStepKuribo(const BrosMoveStepKeeper* pMoveStepKeeper);
bool tryRequestEndSupportFreezeSyncCurrentStepKuribo(const BrosMoveStepKeeper* pMoveStepKeeper);
}  // namespace rc
