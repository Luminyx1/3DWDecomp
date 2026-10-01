#include "Library/Rail/RailUtil.hpp"

#include <algorithm>
#include <math/seadBoundBox.h>
#include <math/seadMathCalcCommon.h>

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Rail/IUseRail.hpp"
#include "Library/Rail/Rail.hpp"
#include "Library/Rail/RailKeeper.hpp"
#include "Library/Rail/RailRider.hpp"

namespace al {
sead::Quatf* tryGetQuatPtr(LiveActor* pActor);
sead::Vector3f* getFrontPtr(LiveActor* pActor);
bool turnQuatFrontToDirDegreeH(LiveActor* pActor, const sead::Vector3f& rDir, f32 degree);
bool turnDirectionDegree(const LiveActor* pActor, sead::Vector3f* pDir,
                         const sead::Vector3f& rTarget, f32 degree);
void setClippingInfo(LiveActor* pActor, f32 radius, const sead::Vector3f* pPos);

static RailRider* getRailRider(const LiveActor* pActor) {
    return pActor->mRailKeeper->getRailRider();
}

static Rail* getRail(const LiveActor* pActor) {
    return pActor->mRailKeeper->getRail();
}

void setRailPosToStart(const LiveActor* pActor) {
    getRailRider(pActor)->moveToRailStart();
}

void setRailPosToEnd(const LiveActor* pActor) {
    getRailRider(pActor)->moveToRailEnd();
}

void setRailPosToNearestPos(const LiveActor* pActor, const sead::Vector3f& rPos) {
    getRailRider(pActor)->moveToNearestRail(rPos);
}

void setRailPosToCoord(const LiveActor* pActor, f32 coord) {
    getRailRider(pActor)->setCoord(coord);
}

void setRailPosToRailPoint(const LiveActor* pActor, s32 index) {
    setRailPosToCoord(pActor, calcRailCoordByPoint(pActor, index));
}

void setSyncRailToStart(LiveActor* pActor) {
    setRailPosToStart(pActor);
    syncRailTrans(pActor);
}

void syncRailTrans(LiveActor* pActor) {
    setTrans(pActor, getRailPos(pActor));
}

void setSyncRailToEnd(LiveActor* pActor) {
    setRailPosToEnd(pActor);
    syncRailTrans(pActor);
}

void setSyncRailToNearestPos(LiveActor* pActor, const sead::Vector3f& rPos) {
    setRailPosToNearestPos(pActor, rPos);
    syncRailTrans(pActor);
}

void setSyncRailToNearestPos(LiveActor* pActor) {
    setSyncRailToNearestPos(pActor, getTrans(pActor));
}

void setSyncRailToCoord(LiveActor* pActor, f32 coord) {
    setRailPosToCoord(pActor, coord);
    syncRailTrans(pActor);
}

void setSyncRailToRailPoint(LiveActor* pActor, s32 index) {
    setRailPosToRailPoint(pActor, index);
    syncRailTrans(pActor);
}

bool moveRail(const LiveActor* pActor, f32 speed) {
    RailRider* railRider = getRailRider(pActor);
    railRider->setSpeed(speed);
    railRider->move();
    return isRailReachedGoal(pActor);
}

bool isRailReachedGoal(const LiveActor* pActor) {
    return getRailRider(pActor)->isReachedGoal();
}

bool moveRailLoop(const LiveActor* pActor, f32 speed) {
    f32 coord = getRailCoord(pActor);
    moveRail(pActor, speed);

    if (isRailReachedGoal(pActor)) {
        if (isRailGoingToEnd(pActor)) {
            setRailPosToCoord(pActor, (coord + speed) - getRailTotalLength(pActor));
        } else {
            setRailPosToCoord(pActor, getRailTotalLength(pActor) - (speed - coord));
        }

        return true;
    }

    return false;
}

f32 getRailCoord(const LiveActor* pActor) {
    return getRailRider(pActor)->getCoord();
}

bool isRailGoingToEnd(const LiveActor* pActor) {
    return getRailRider(pActor)->isMoveForwards();
}

f32 getRailTotalLength(const LiveActor* pActor) {
    return getRail(pActor)->getTotalLength();
}

bool moveRailTurn(const LiveActor* pActor, f32 speed, f32 goalCoord) {
    if (speed < 0.0f) {
        reverseRail(pActor);
    }

    moveRail(pActor, sead::Mathf::abs(speed));
    bool isReversed = goalCoord <= 0.0f ? isRailReachedGoal(pActor) :
                                          isRailReachedNearGoal(pActor, goalCoord);

    if (isReversed) {
        reverseRail(pActor);
    }

    if (speed < 0.0f) {
        reverseRail(pActor);
    }

    return isReversed;
}

void reverseRail(const LiveActor* pActor) {
    getRailRider(pActor)->reverse();
}

bool isRailReachedNearGoal(const LiveActor* pActor, f32 goalCoord) {
    return isRailReachedNearGoal(pActor, goalCoord, goalCoord);
}

bool moveRailPause(const LiveActor* pActor, f32 speed) {
    bool isPaused = false;
    s32 index = getRailPartIndex(pActor);

    if ((index == 1 || index == 11 || index == 22) &&
        isRailReachedNearEndRailPoint(pActor, speed)) {
        if (isRailReachedNearGoal(pActor, speed)) {
            reverseRail(pActor);
        }

        isPaused = true;
    }

    moveRail(pActor, sead::Mathf::abs(speed));
    return isPaused;
}

s32 getRailPartIndex(const LiveActor* pActor) {
    return getRail(pActor)->getIncludedSectionIndex(getRailCoord(pActor));
}

bool isRailReachedNearEndRailPoint(const LiveActor* pActor, f32 margin) {
    return getRail(pActor)->isNearEndRailPoint(getRailCoord(pActor), margin);
}

bool turnToRailDir(LiveActor* pActor, f32 degree) {
    sead::Vector3f moveDir = isRailGoingToEnd(pActor) ? getRailDir(pActor) : -getRailDir(pActor);

    if (tryGetQuatPtr(pActor)) {
        return turnQuatFrontToDirDegreeH(pActor, moveDir, degree);
    }

    return turnDirectionDegree(pActor, getFrontPtr(pActor), moveDir, degree);
}

const sead::Vector3f& getRailDir(const LiveActor* pActor) {
    return getRailRider(pActor)->getDirection();
}

bool turnToRailDirImmediately(LiveActor* pActor) {
    return turnToRailDir(pActor, 180.0f);
}

const sead::Vector3f& getRailPos(const LiveActor* pActor) {
    return getRailRider(pActor)->getPosition();
}

void syncRailTransOffset(LiveActor* pActor, const sead::Vector3f& rOffset) {
    sead::Vector3f trans = getRailPos(pActor) + rOffset;
    setTrans(pActor, trans);
}

bool moveSyncRail(LiveActor* pActor, f32 speed) {
    bool isReachedGoal = moveRail(pActor, speed);
    syncRailTrans(pActor);
    return isReachedGoal;
}

bool moveSyncRailOffset(LiveActor* pActor, f32 speed, const sead::Vector3f& rOffset) {
    bool isReachedGoal = moveRail(pActor, speed);
    syncRailTransOffset(pActor, rOffset);
    return isReachedGoal;
}

bool moveSyncRailLoop(LiveActor* pActor, f32 speed) {
    bool isReachedGoal = moveRailLoop(pActor, speed);
    syncRailTrans(pActor);

    if (isReachedGoal) {
        resetPosition(pActor, false);
    }

    return isReachedGoal;
}

bool moveSyncRailTurn(LiveActor* pActor, f32 speed) {
    bool isReversed = moveRailTurn(pActor, speed, 0.0f);
    syncRailTrans(pActor);
    return isReversed;
}

bool moveSyncRailPause(LiveActor* pActor, f32 speed) {
    bool isPaused = moveRailPause(pActor, speed);
    syncRailTrans(pActor);
    return isPaused;
}

f32 calcNearestRailCoord(const LiveActor* pActor, const sead::Vector3f& rPos) {
    return getRail(pActor)->calcNearestRailPosCoord(rPos, 20.0f);
}

f32 calcNearestRailCoord(const RailKeeper* pRailKeeper, const sead::Vector3f& rPos) {
    return pRailKeeper->getRail()->calcNearestRailPosCoord(rPos, 20.0f);
}

f32 calcNearestRailPos(sead::Vector3f* pRailPos, const LiveActor* pActor,
                       const sead::Vector3f& rPos) {
    return getRail(pActor)->calcNearestRailPos(pRailPos, rPos, 20.0f);
}

f32 calcNearestRailPos(sead::Vector3f* pRailPos, const RailKeeper* pRailKeeper,
                       const sead::Vector3f& rPos) {
    return pRailKeeper->getRail()->calcNearestRailPos(pRailPos, rPos, 20.0f);
}

s32 calcRailPointNum(const LiveActor* pActor, f32 coordStart, f32 coordEnd) {
    return getRail(pActor)->calcRailPointNum(coordStart, coordEnd);
}

void calcRailPointPos(sead::Vector3f* pPos, const LiveActor* pActor, s32 index) {
    getRail(pActor)->calcRailPointPos(pPos, index);
}

f32 calcRailToGoalLength(const LiveActor* pActor) {
    const Rail* rail = getRail(pActor);
    f32 length = rail->getTotalLength();

    if (rail->isClosed()) {
        return length;
    }

    f32 coord = getRailCoord(pActor);

    if (isRailGoingToEnd(pActor)) {
        return length - coord;
    }

    return coord;
}

f32 calcRailPartRate(const LiveActor* pActor) {
    const Rail* rail = getRail(pActor);
    f32 partDistance;
    f32 length;
    f32 sectionLength = rail->getIncludedSectionLength(&partDistance, &length, getRailCoord(pActor));

    if (isRailGoingToEnd(pActor)) {
        return partDistance / sectionLength;
    }

    return length / sectionLength;
}

f32 calcRailToNextRailPointLength(const LiveActor* pActor) {
    const Rail* rail = getRail(pActor);
    f32 length;
    f32 partDistance;
    rail->getIncludedSectionLength(&partDistance, &length, getRailCoord(pActor));

    if (isRailGoingToEnd(pActor)) {
        return length;
    }

    return partDistance;
}

f32 calcRailToPreviousRailPointLength(const LiveActor* pActor) {
    const Rail* rail = getRail(pActor);
    f32 length;
    f32 partDistance;
    rail->getIncludedSectionLength(&partDistance, &length, getRailCoord(pActor));

    if (isRailGoingToEnd(pActor)) {
        return partDistance;
    }

    return length;
}

s32 getRailNum(const LiveActor* pActor) {
    return getRail(pActor)->getRailPartCount();
}

s32 getRailPointNum(const LiveActor* pActor) {
    return getRail(pActor)->getRailPointsCount();
}

s32 getRailPointNum(const RailKeeper* pRailKeeper) {
    return pRailKeeper->getRail()->getRailPointsCount();
}

const sead::Vector3f& getRailPos(const RailKeeper* pRailKeeper) {
    if (pRailKeeper != nullptr && pRailKeeper->getRailRider() != nullptr) {
        return pRailKeeper->getRailRider()->getPosition();
    }

    return sead::Vector3f::zero;
}

void getRailUpDir(const LiveActor* pActor, sead::Vector3f* pUp) {
    getRailRider(pActor)->getUpDir(pUp);
}

f32 getRailPartLength(const LiveActor* pActor, s32 index) {
    return getRail(pActor)->getPartLength(index);
}

s32 getRailPointNo(const LiveActor* pActor) {
    if (isLoopRail(pActor)) {
        return getRailPartIndex(pActor);
    }

    if (isRailReachedEnd(pActor)) {
        return getRailPointNum(pActor) - 1;
    }

    return getRailPartIndex(pActor);
}

bool isLoopRail(const LiveActor* pActor) {
    return getRail(pActor)->isClosed();
}

bool isRailReachedEnd(const LiveActor* pActor) {
    return getRailRider(pActor)->isReachedRailEnd();
}

void getRailPartAccels(const LiveActor* pActor, s32 index, f32* pAccelStart, f32* pAccelEnd) {
    getRail(pActor)->getAccels(index, pAccelStart, pAccelEnd);
}

bool getRailPartAngleS(const LiveActor* pActor, s32 index, f32* pAngle) {
    return getRail(pActor)->getAngleS(index, pAngle);
}

bool getRailPartAngleE(const LiveActor* pActor, s32 index, f32* pAngle) {
    return getRail(pActor)->getAngleE(index, pAngle);
}

f32 getRailPartRate(const LiveActor* pActor, s32 index, f32 coord) {
    f32 partLength = getRail(pActor)->getPartLength(index);
    s32 length = 0;

    for (s32 i = 0; i < index; i++) {
        length += getRail(pActor)->getPartLength(i);
    }

    return std::min((coord - length) / partLength, 1.0f);
}

bool isExistRail(const LiveActor* pActor) {
    return (pActor->mRailKeeper != nullptr) && pActor->mRailKeeper->isValid();
}

bool isRailReachedStart(const LiveActor* pActor) {
    return getRailRider(pActor)->isReachedRailStart();
}

bool isRailReachedNearGoal(const LiveActor* pActor, f32 goalMarginEnd, f32 goalMarginStart) {
    if (isLoopRail(pActor)) {
        return false;
    }

    if (isRailGoingToEnd(pActor)) {
        if (getRailTotalLength(pActor) - goalMarginStart <= getRailCoord(pActor)) {
            return true;
        }
    } else if (getRailCoord(pActor) <= goalMarginEnd) {
        return true;
    }

    return false;
}

bool isRailReachedEdge(const LiveActor* pActor) {
    return getRailRider(pActor)->isReachedEdge();
}

bool isRailReachedNearRailPoint(const LiveActor* pActor, f32 margin) {
    return getRail(pActor)->isNearRailPoint(getRailCoord(pActor), margin);
}

bool isRailReachedNearStartRailPoint(const LiveActor* pActor, f32 margin) {
    return getRail(pActor)->isNearStartRailPoint(getRailCoord(pActor), margin);
}

bool isRailPlusDir(const LiveActor* pActor, const sead::Vector3f& rDir) {
    return rDir.dot(getRailDir(pActor)) >= 0.0f;
}

bool isRailPlusPoseSide(const LiveActor* pActor) {
    sead::Vector3f side;
    calcSideDir(&side, pActor);
    return isRailPlusDir(pActor, side);
}

bool isRailPlusPoseUp(const LiveActor* pActor) {
    sead::Vector3f up;
    calcUpDir(&up, pActor);
    return isRailPlusDir(pActor, up);
}

bool isRailPlusPoseFront(const LiveActor* pActor) {
    sead::Vector3f front;
    calcFrontDir(&front, pActor);
    return isRailPlusDir(pActor, front);
}

void calcRailPosAtCoord(sead::Vector3f* pPos, const LiveActor* pActor, f32 coord) {
    getRail(pActor)->calcPos(pPos, coord);
}

void calcRailMoveDir(sead::Vector3f* pDir, const LiveActor* pActor) {
    pDir->set(isRailGoingToEnd(pActor) ? getRailDir(pActor) : -getRailDir(pActor));
}

void calcRailDirAtCoord(sead::Vector3f* pDir, const LiveActor* pActor, f32 coord) {
    getRail(pActor)->calcDirection(pDir, coord);
}

void calcRailDirAtCoord(sead::Vector3f* pDir, const RailKeeper* pRailKeeper, f32 coord) {
    pRailKeeper->getRail()->calcDirection(pDir, coord);
}

void calcRailPosFront(sead::Vector3f* pPos, const LiveActor* pActor, f32 offset) {
    if (!isRailGoingToEnd(pActor)) {
        offset = -offset;
    }

    f32 coordOffset = offset + getRailCoord(pActor);
    const Rail* rail = getRail(pActor);

    if (rail->isClosed()) {
        f32 totalLength = rail->getTotalLength();
        rail->calcPos(pPos, modf(coordOffset + totalLength, totalLength) + 0.0f);
        return;
    }

    f32 length = rail->getTotalLength();
    f32 clamped = (coordOffset <= length) ? coordOffset : length;
    rail->calcPos(pPos, (coordOffset < 0.0f) ? 0.0f : clamped);
}

f32 calcRailCoordByPoint(const LiveActor* pActor, s32 index) {
    return getRail(pActor)->getLengthToPoint(index);
}

void calcRailClippingInfo(sead::Vector3f* pPos, f32* pRadius, const LiveActor* pActor, f32 step,
                          f32 offset) {
    f32 totalLength = getRailTotalLength(pActor);
    s32 stepNum = totalLength / step;

    sead::Vector3f railPos;
    sead::Vector3f lastRailPos;
    calcRailPosAtCoord(&railPos, pActor, 0.0f);
    calcRailPosAtCoord(&lastRailPos, pActor, totalLength);

    sead::BoundBox3f boundBox(
        {sead::Mathf::min(railPos.x, lastRailPos.x), sead::Mathf::min(railPos.y, lastRailPos.y),
         sead::Mathf::min(railPos.z, lastRailPos.z)},
        {sead::Mathf::max(railPos.x, lastRailPos.x), sead::Mathf::max(railPos.y, lastRailPos.y),
         sead::Mathf::max(railPos.z, lastRailPos.z)});

    for (s32 i = 1; i < stepNum; i++) {
        sead::Vector3f pos;
        getRail(pActor)->calcPos(&pos, i * step);
        boundBox.addPoint(pos);
    }

    boundBox.getCenter(pPos);
    *pRadius = 0.0f;

    for (s32 i = 0; i < stepNum; i++) {
        sead::Vector3f pos;
        getRail(pActor)->calcPos(&pos, i * step);
        *pRadius = sead::Mathf::max((pos - *pPos).length(), *pRadius);
    }

    *pRadius = sead::Mathf::max((lastRailPos - *pPos).length(), *pRadius);
    *pRadius += offset;
}

void calcRailClippingInfo(sead::Vector3f* pPos, f32* pRadius, const RailKeeper* pKeeper, f32 step,
                          f32 offset) {
    f32 totalLength = pKeeper->getRail()->getTotalLength();
    s32 stepNum = totalLength / step;

    sead::Vector3f railPos;
    sead::Vector3f lastRailPos;
    pKeeper->getRail()->calcPos(&railPos, 0.0f);
    pKeeper->getRail()->calcPos(&lastRailPos, totalLength);

    sead::BoundBox3f boundBox(
        {sead::Mathf::min(railPos.x, lastRailPos.x), sead::Mathf::min(railPos.y, lastRailPos.y),
         sead::Mathf::min(railPos.z, lastRailPos.z)},
        {sead::Mathf::max(railPos.x, lastRailPos.x), sead::Mathf::max(railPos.y, lastRailPos.y),
         sead::Mathf::max(railPos.z, lastRailPos.z)});

    for (s32 i = 1; i < stepNum; i++) {
        sead::Vector3f pos;
        pKeeper->getRail()->calcPos(&pos, i * step);
        boundBox.addPoint(pos);
    }

    boundBox.getCenter(pPos);
    *pRadius = 0.0f;

    for (s32 i = 0; i < stepNum; i++) {
        sead::Vector3f pos;
        pKeeper->getRail()->calcPos(&pos, i * step);
        *pRadius = sead::Mathf::max((pos - *pPos).length(), *pRadius);
    }

    *pRadius = sead::Mathf::max((lastRailPos - *pPos).length(), *pRadius);
    *pRadius += offset;
}

void setRailClippingInfo(sead::Vector3f* pPos, LiveActor* pActor, f32 step, f32 offset) {
    f32 radius = 0.0f;
    calcRailClippingInfo(pPos, &radius, pActor, step, offset);
    setClippingInfo(pActor, radius, pPos);
}

void setRailClippingInfo(sead::Vector3f* pPos, LiveActor* pActor, const RailKeeper* pRailKeeper,
                         f32 step, f32 offset) {
    f32 radius = 0.0f;
    calcRailClippingInfo(pPos, &radius, pRailKeeper, step, offset);
    setClippingInfo(pActor, radius, pPos);
}

s32 getRailPointNum(const IUseRail* pRailHolder) {
    return pRailHolder->getRailRider()->getRail()->getRailPointsCount();
}

PlacementInfo* getRailPointInfo(const IUseRail* pRailHolder, s32 index) {
    return pRailHolder->getRailRider()->getRail()->getRailPoint(index);
}

}  // namespace al
