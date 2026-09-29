#include "Library/Rail/RailUtil.hpp"

#include <algorithm>
#include <math/seadBoundBox.h>

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Rail/IUseRail.hpp"
#include "Library/Rail/Rail.hpp"
#include "Library/Rail/RailKeeper.hpp"
#include "Library/Rail/RailRider.hpp"

namespace {
/**
 * @brief Gets the rail rider of an actor.
 * @param pActor The actor.
 * @return The rail rider.
 */
inline al::RailRider* getRailRider(const al::LiveActor* pActor) {
    return pActor->mRailKeeper->getRailRider();
}

/**
 * @brief Gets the rail of an actor.
 * @param pActor The actor.
 * @return The rail.
 */
inline const al::Rail* getRail(const al::LiveActor* pActor) {
    return pActor->mRailKeeper->getRail();
}

}  // namespace

namespace al {

/**
 * @brief Moves the rail rider of an actor to the start of its rail.
 * @param pActor The actor.
 */
void setRailPosToStart(const LiveActor* pActor) {
    getRailRider(pActor)->moveToRailStart();
}

/**
 * @brief Moves the rail rider of an actor to the end of its rail.
 * @param pActor The actor.
 */
void setRailPosToEnd(const LiveActor* pActor) {
    getRailRider(pActor)->moveToRailEnd();
}

/**
 * @brief Moves the rail rider of an actor to the point on its rail nearest to a position.
 * @param pActor The actor.
 * @param rPos The position.
 */
void setRailPosToNearestPos(const LiveActor* pActor, const sead::Vector3f& rPos) {
    getRailRider(pActor)->moveToNearestRail(rPos);
}

/**
 * @brief Moves the rail rider of an actor to a coordinate.
 * @param pActor The actor.
 * @param coord The distance from the start of the rail.
 */
void setRailPosToCoord(const LiveActor* pActor, f32 coord) {
    getRailRider(pActor)->setCoord(coord);
}

/**
 * @brief Moves the rail rider of an actor to a rail point.
 * @param pActor The actor.
 * @param index The rail point index.
 */
void setRailPosToRailPoint(const LiveActor* pActor, s32 index) {
    setRailPosToCoord(pActor, calcRailCoordByPoint(pActor, index));
}

/**
 * @brief Moves an actor to the start of its rail.
 * @param pActor The actor.
 */
void setSyncRailToStart(LiveActor* pActor) {
    setRailPosToStart(pActor);
    syncRailTrans(pActor);
}

/**
 * @brief Moves an actor to the position of its rail rider.
 * @param pActor The actor.
 */
void syncRailTrans(LiveActor* pActor) {
    setTrans(pActor, getRailPos(pActor));
}

/**
 * @brief Moves an actor to the end of its rail.
 * @param pActor The actor.
 */
void setSyncRailToEnd(LiveActor* pActor) {
    setRailPosToEnd(pActor);
    syncRailTrans(pActor);
}

/**
 * @brief Moves an actor to the point on its rail nearest to a position.
 * @param pActor The actor.
 * @param rPos The position.
 */
void setSyncRailToNearestPos(LiveActor* pActor, const sead::Vector3f& rPos) {
    setRailPosToNearestPos(pActor, rPos);
    syncRailTrans(pActor);
}

/**
 * @brief Moves an actor to the point on its rail nearest to itself.
 * @param pActor The actor.
 */
void setSyncRailToNearestPos(LiveActor* pActor) {
    setSyncRailToNearestPos(pActor, getTrans(pActor));
}

/**
 * @brief Moves an actor to a coordinate on its rail.
 * @param pActor The actor.
 * @param coord The distance from the start of the rail.
 */
void setSyncRailToCoord(LiveActor* pActor, f32 coord) {
    setRailPosToCoord(pActor, coord);
    syncRailTrans(pActor);
}

/**
 * @brief Moves an actor to a rail point.
 * @param pActor The actor.
 * @param index The rail point index.
 */
void setSyncRailToRailPoint(LiveActor* pActor, s32 index) {
    setRailPosToRailPoint(pActor, index);
    syncRailTrans(pActor);
}

/**
 * @brief Moves the rail rider of an actor.
 * @param pActor The actor.
 * @param speed The distance to move.
 * @return True if the goal was reached.
 */
bool moveRail(const LiveActor* pActor, f32 speed) {
    RailRider* railRider = getRailRider(pActor);
    railRider->setSpeed(speed);
    railRider->move();
    return isRailReachedGoal(pActor);
}

/**
 * @brief Checks whether the rail rider of an actor reached its goal.
 * @param pActor The actor.
 * @return True if the goal was reached.
 */
bool isRailReachedGoal(const LiveActor* pActor) {
    return getRailRider(pActor)->isReachedGoal();
}

/**
 * @brief Moves the rail rider of an actor, wrapping around at the goal.
 * @param pActor The actor.
 * @param speed The distance to move.
 * @return True if the rider wrapped around.
 */
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

/**
 * @brief Gets the coordinate of the rail rider of an actor.
 * @param pActor The actor.
 * @return The distance from the start of the rail.
 */
f32 getRailCoord(const LiveActor* pActor) {
    return getRailRider(pActor)->getCoord();
}

/**
 * @brief Checks whether the rail rider of an actor moves towards the end.
 * @param pActor The actor.
 * @return True if moving towards the end.
 */
bool isRailGoingToEnd(const LiveActor* pActor) {
    return getRailRider(pActor)->isMoveForwards();
}

/**
 * @brief Gets the length of the rail of an actor.
 * @param pActor The actor.
 * @return The length.
 */
f32 getRailTotalLength(const LiveActor* pActor) {
    return getRail(pActor)->getTotalLength();
}

/**
 * @brief Moves the rail rider of an actor, turning around near the goal.
 * @param pActor The actor.
 * @param speed The distance to move, negative to move backwards.
 * @param goalDistance The distance to the goal at which to turn, or 0 to turn at the goal.
 * @return True if the rider turned around.
 */
bool moveRailTurn(const LiveActor* pActor, f32 speed, f32 goalDistance) {
    if (speed < 0.0f) {
        reverseRail(pActor);
    }

    moveRail(pActor, sead::Mathf::abs(speed));
    bool isReversed = goalDistance <= 0.0f ? isRailReachedGoal(pActor) :
                                             isRailReachedNearGoal(pActor, goalDistance);

    if (isReversed) {
        reverseRail(pActor);
    }
    if (speed < 0.0f) {
        reverseRail(pActor);
    }
    return isReversed;
}

/**
 * @brief Reverses the rail rider of an actor.
 * @param pActor The actor.
 */
void reverseRail(const LiveActor* pActor) {
    getRailRider(pActor)->reverse();
}

/**
 * @brief Checks whether the rail rider of an actor is near its goal.
 * @param pActor The actor.
 * @param goalDistance The tolerance.
 * @return True if near the goal.
 */
bool isRailReachedNearGoal(const LiveActor* pActor, f32 goalDistance) {
    return isRailReachedNearGoal(pActor, goalDistance, goalDistance);
}

/**
 * @brief Moves the rail rider of an actor, turning around at some rail points.
 * @param pActor The actor.
 * @param speed The distance to move.
 * @return True if the rider is at a rail point where it pauses.
 */
bool moveRailPause(const LiveActor* pActor, f32 speed) {
    bool isPause = false;
    s32 index = getRailPartIndex(pActor);
    if ((index == 1 || index == 11 || index == 22) && isRailReachedNearEndRailPoint(pActor, speed)) {
        if (isRailReachedNearGoal(pActor, speed)) {
            reverseRail(pActor);
        }
        isPause = true;
    }

    moveRail(pActor, sead::Mathf::abs(speed));
    return isPause;
}

/**
 * @brief Gets the index of the rail part the rail rider of an actor is on.
 * @param pActor The actor.
 * @return The part index.
 */
s32 getRailPartIndex(const LiveActor* pActor) {
    return getRail(pActor)->getIncludedSectionIndex(getRailCoord(pActor));
}

/**
 * @brief Checks whether the rail rider of an actor is near the end of its rail part.
 * @param pActor The actor.
 * @param epsilon The tolerance.
 * @return True if near the end of the part.
 */
bool isRailReachedNearEndRailPoint(const LiveActor* pActor, f32 epsilon) {
    return getRail(pActor)->isNearEndRailPoint(getRailCoord(pActor), epsilon);
}

/**
 * @brief Turns an actor towards its moving direction on the rail.
 * @param pActor The actor.
 * @param degree The maximum angle to turn.
 * @return True if the actor faces the moving direction.
 */
bool turnToRailDir(LiveActor* pActor, f32 degree) {
    sead::Vector3f moveDir = isRailGoingToEnd(pActor) ? getRailDir(pActor) : -getRailDir(pActor);

    if (tryGetQuatPtr(pActor)) {
        return turnQuatFrontToDirDegreeH(pActor, moveDir, degree);
    }
    return turnDirectionDegree(pActor, getFrontPtr(pActor), moveDir, degree);
}

/**
 * @brief Gets the direction of the rail at the rail rider of an actor.
 * @param pActor The actor.
 * @return The direction.
 */
const sead::Vector3f& getRailDir(const LiveActor* pActor) {
    return getRailRider(pActor)->getDirection();
}

/**
 * @brief Turns an actor to its moving direction on the rail at once.
 * @param pActor The actor.
 * @return True if the actor faces the moving direction.
 */
bool turnToRailDirImmediately(LiveActor* pActor) {
    return turnToRailDir(pActor, 180.0f);
}

/**
 * @brief Gets the position of the rail rider of an actor.
 * @param pActor The actor.
 * @return The position.
 */
const sead::Vector3f& getRailPos(const LiveActor* pActor) {
    return getRailRider(pActor)->getPosition();
}

/**
 * @brief Moves an actor to the position of its rail rider plus an offset.
 * @param pActor The actor.
 * @param rOffset The offset.
 */
void syncRailTransOffset(LiveActor* pActor, const sead::Vector3f& rOffset) {
    setTrans(pActor, getRailPos(pActor) + rOffset);
}

/**
 * @brief Moves an actor along its rail.
 * @param pActor The actor.
 * @param speed The distance to move.
 * @return True if the goal was reached.
 */
bool moveSyncRail(LiveActor* pActor, f32 speed) {
    bool isReachedGoal = moveRail(pActor, speed);
    syncRailTrans(pActor);
    return isReachedGoal;
}

/**
 * @brief Moves an actor along its rail with an offset.
 * @param pActor The actor.
 * @param speed The distance to move.
 * @param rOffset The offset from the rail.
 * @return True if the goal was reached.
 */
bool moveSyncRailOffset(LiveActor* pActor, f32 speed, const sead::Vector3f& rOffset) {
    bool isReachedGoal = moveRail(pActor, speed);
    syncRailTransOffset(pActor, rOffset);
    return isReachedGoal;
}

/**
 * @brief Moves an actor along its rail, wrapping around at the goal.
 * @param pActor The actor.
 * @param speed The distance to move.
 * @return True if the actor wrapped around.
 */
bool moveSyncRailLoop(LiveActor* pActor, f32 speed) {
    bool isReachedGoal = moveRailLoop(pActor, speed);
    syncRailTrans(pActor);
    if (isReachedGoal) {
        resetPosition(pActor, false);
    }
    return isReachedGoal;
}

/**
 * @brief Moves an actor along its rail, turning around at the goal.
 * @param pActor The actor.
 * @param speed The distance to move.
 * @return True if the actor turned around.
 */
bool moveSyncRailTurn(LiveActor* pActor, f32 speed) {
    bool isReversed = moveRailTurn(pActor, speed, 0.0f);
    syncRailTrans(pActor);
    return isReversed;
}

/**
 * @brief Moves an actor along its rail, pausing at some rail points.
 * @param pActor The actor.
 * @param speed The distance to move.
 * @return True if the actor is at a rail point where it pauses.
 */
bool moveSyncRailPause(LiveActor* pActor, f32 speed) {
    bool isPause = moveRailPause(pActor, speed);
    syncRailTrans(pActor);
    return isPause;
}

/**
 * @brief Finds the coordinate on the rail of an actor nearest to a position.
 * @param pActor The actor.
 * @param rPos The position.
 * @return The distance from the start of the rail.
 */
f32 calcNearestRailCoord(const LiveActor* pActor, const sead::Vector3f& rPos) {
    return getRail(pActor)->calcNearestRailPosCoord(rPos, 20.0f);
}

/**
 * @brief Finds the coordinate on a rail nearest to a position.
 * @param pKeeper The rail keeper.
 * @param rPos The position.
 * @return The distance from the start of the rail.
 */
f32 calcNearestRailCoord(const RailKeeper* pKeeper, const sead::Vector3f& rPos) {
    return pKeeper->getRail()->calcNearestRailPosCoord(rPos, 20.0f);
}

/**
 * @brief Finds the point on the rail of an actor nearest to a position.
 * @param pRailPos Where the nearest point is written.
 * @param pActor The actor.
 * @param rPos The position.
 * @return The distance of the nearest point from the start of the rail.
 */
f32 calcNearestRailPos(sead::Vector3f* pRailPos, const LiveActor* pActor, const sead::Vector3f& rPos) {
    return getRail(pActor)->calcNearestRailPos(pRailPos, rPos, 20.0f);
}

/**
 * @brief Finds the point on a rail nearest to a position.
 * @param pRailPos Where the nearest point is written.
 * @param pKeeper The rail keeper.
 * @param rPos The position.
 * @return The distance of the nearest point from the start of the rail.
 */
f32 calcNearestRailPos(sead::Vector3f* pRailPos, const RailKeeper* pKeeper,
                       const sead::Vector3f& rPos) {
    return pKeeper->getRail()->calcNearestRailPos(pRailPos, rPos, 20.0f);
}

/**
 * @brief Counts the rail points between two coordinates on the rail of an actor.
 * @param pActor The actor.
 * @param coord1 The start distance.
 * @param coord2 The end distance.
 * @return The number of rail points.
 */
s32 calcRailPointNum(const LiveActor* pActor, f32 coord1, f32 coord2) {
    return pActor->mRailKeeper->getRail()->calcRailPointNum(coord1, coord2);
}

/**
 * @brief Computes the position of a rail point of an actor.
 * @param pPos Where the position is written.
 * @param pActor The actor.
 * @param index The rail point index.
 */
void calcRailPointPos(sead::Vector3f* pPos, const LiveActor* pActor, s32 index) {
    getRail(pActor)->calcRailPointPos(pPos, index);
}

/**
 * @brief Computes the distance from the rail rider of an actor to its goal.
 * @param pActor The actor.
 * @return The distance, or the rail length for closed rails.
 */
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

/**
 * @brief Computes how far the rail rider of an actor is along its current part.
 * @param pActor The actor.
 * @return The rate in the moving direction.
 */
f32 calcRailPartRate(const LiveActor* pActor) {
    const Rail* rail = getRail(pActor);
    f32 partDistance;
    f32 remaining;
    f32 sectionLength = rail->getIncludedSectionLength(&partDistance, &remaining, getRailCoord(pActor));
    if (isRailGoingToEnd(pActor)) {
        return partDistance / sectionLength;
    }
    return remaining / sectionLength;
}

/**
 * @brief Computes the distance from the rail rider of an actor to the next rail point.
 * @param pActor The actor.
 * @return The distance.
 */
f32 calcRailToNextRailPointLength(const LiveActor* pActor) {
    const Rail* rail = getRail(pActor);
    f32 remaining;
    f32 partDistance;
    rail->getIncludedSectionLength(&partDistance, &remaining, getRailCoord(pActor));
    if (isRailGoingToEnd(pActor)) {
        return remaining;
    }
    return partDistance;
}

/**
 * @brief Computes the distance from the rail rider of an actor to the previous rail point.
 * @param pActor The actor.
 * @return The distance.
 */
f32 calcRailToPreviousRailPointLength(const LiveActor* pActor) {
    const Rail* rail = getRail(pActor);
    f32 remaining;
    f32 partDistance;
    rail->getIncludedSectionLength(&partDistance, &remaining, getRailCoord(pActor));
    if (isRailGoingToEnd(pActor)) {
        return partDistance;
    }
    return remaining;
}

/**
 * @brief Gets the number of rail parts of an actor.
 * @param pActor The actor.
 * @return The number of parts.
 */
s32 getRailNum(const LiveActor* pActor) {
    return getRail(pActor)->getRailPartCount();
}

/**
 * @brief Gets the number of rail points of an actor.
 * @param pActor The actor.
 * @return The number of rail points.
 */
s32 getRailPointNum(const LiveActor* pActor) {
    return getRail(pActor)->getRailPointsCount();
}

/**
 * @brief Gets the number of rail points of a rail.
 * @param pKeeper The rail keeper.
 * @return The number of rail points.
 */
s32 getRailPointNum(const RailKeeper* pKeeper) {
    return pKeeper->getRail()->getRailPointsCount();
}

/**
 * @brief Gets the position of the rider of a rail.
 * @param pKeeper The rail keeper, may be null.
 * @return The position, or zero without a rider.
 */
const sead::Vector3f& getRailPos(const RailKeeper* pKeeper) {
    if (pKeeper && pKeeper->getRailRider()) {
        return pKeeper->getRailRider()->getPosition();
    }
    return sead::Vector3f::zero;
}

/**
 * @brief Computes the up direction of the rail at the rail rider of an actor.
 * @param pActor The actor.
 * @param pUp Where the up direction is written.
 */
void getRailUpDir(const LiveActor* pActor, sead::Vector3f* pUp) {
    getRailRider(pActor)->getUpDir(pUp);
}

/**
 * @brief Gets the length of a rail part of an actor.
 * @param pActor The actor.
 * @param index The part index.
 * @return The length.
 */
f32 getRailPartLength(const LiveActor* pActor, s32 index) {
    return getRail(pActor)->getPartLength(index);
}

/**
 * @brief Gets the index of the rail point the rail rider of an actor last passed.
 * @param pActor The actor.
 * @return The rail point index.
 */
s32 getRailPointNo(const LiveActor* pActor) {
    if (isLoopRail(pActor)) {
        return getRailPartIndex(pActor);
    }

    if (isRailReachedEnd(pActor)) {
        return getRailPointNum(pActor) - 1;
    }

    return getRailPartIndex(pActor);
}

/**
 * @brief Checks whether the rail of an actor is closed.
 * @param pActor The actor.
 * @return True if the rail is closed.
 */
bool isLoopRail(const LiveActor* pActor) {
    return getRail(pActor)->isClosed();
}

/**
 * @brief Checks whether the rail rider of an actor is at the end of its rail.
 * @param pActor The actor.
 * @return True if at the end.
 */
bool isRailReachedEnd(const LiveActor* pActor) {
    return getRailRider(pActor)->isReachedRailEnd();
}

/**
 * @brief Gets the accelerations of a rail part of an actor.
 * @param pActor The actor.
 * @param index The part index.
 * @param pAccelStart Where the acceleration at the start is written.
 * @param pAccelEnd Where the acceleration at the end is written.
 */
void getRailPartAccels(const LiveActor* pActor, s32 index, f32* pAccelStart, f32* pAccelEnd) {
    pActor->mRailKeeper->getRail()->getAccels(index, pAccelStart, pAccelEnd);
}

/**
 * @brief Gets the start angle of a rail part of an actor.
 * @param pActor The actor.
 * @param index The part index.
 * @param pAngle Where the angle is written.
 * @return True if the angle was set.
 */
bool getRailPartAngleS(const LiveActor* pActor, s32 index, f32* pAngle) {
    return pActor->mRailKeeper->getRail()->getAngleS(index, pAngle);
}

/**
 * @brief Gets the end angle of a rail part of an actor.
 * @param pActor The actor.
 * @param index The part index.
 * @param pAngle Where the angle is written.
 * @return True if the angle was set.
 */
bool getRailPartAngleE(const LiveActor* pActor, s32 index, f32* pAngle) {
    return pActor->mRailKeeper->getRail()->getAngleE(index, pAngle);
}

/**
 * @brief Computes how far a coordinate is along a rail part of an actor.
 * @param pActor The actor.
 * @param index The part index.
 * @param coord The distance from the start of the rail.
 * @return The rate, at most 1.
 */
f32 getRailPartRate(const LiveActor* pActor, s32 index, f32 coord) {
    f32 partLength = getRail(pActor)->getPartLength(index);
    s32 length = 0;
    for (s32 i = 0; i < index; i++) {
        length += getRail(pActor)->getPartLength(i);
    }
    return std::min((coord - length) / partLength, 1.0f);
}

/**
 * @brief Checks whether an actor has a rail.
 * @param pActor The actor.
 * @return True if the actor has a rail.
 */
bool isExistRail(const LiveActor* pActor) {
    return pActor->mRailKeeper && pActor->mRailKeeper->isValid();
}

/**
 * @brief Checks whether the rail rider of an actor is at the start of its rail.
 * @param pActor The actor.
 * @return True if at the start.
 */
bool isRailReachedStart(const LiveActor* pActor) {
    return getRailRider(pActor)->isReachedRailStart();
}

/**
 * @brief Checks whether the rail rider of an actor is near its goal.
 * @param pActor The actor.
 * @param startDistance The tolerance when moving towards the start.
 * @param endDistance The tolerance when moving towards the end.
 * @return True if near the goal.
 */
bool isRailReachedNearGoal(const LiveActor* pActor, f32 startDistance, f32 endDistance) {
    if (isLoopRail(pActor)) {
        return false;
    }

    if (isRailGoingToEnd(pActor)) {
        if (getRailTotalLength(pActor) - endDistance <= getRailCoord(pActor)) {
            return true;
        }
    } else {
        if (startDistance >= getRailCoord(pActor)) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Checks whether the rail rider of an actor is at either end of its rail.
 * @param pActor The actor.
 * @return True if at an end.
 */
bool isRailReachedEdge(const LiveActor* pActor) {
    return getRailRider(pActor)->isReachedEdge();
}

/**
 * @brief Checks whether the rail rider of an actor is near a rail point.
 * @param pActor The actor.
 * @param epsilon The tolerance.
 * @return True if near a rail point.
 */
bool isRailReachedNearRailPoint(const LiveActor* pActor, f32 epsilon) {
    return getRail(pActor)->isNearRailPoint(getRailCoord(pActor), epsilon);
}

/**
 * @brief Checks whether the rail rider of an actor is near the start of its rail part.
 * @param pActor The actor.
 * @param epsilon The tolerance.
 * @return True if near the start of the part.
 */
bool isRailReachedNearStartRailPoint(const LiveActor* pActor, f32 epsilon) {
    return getRail(pActor)->isNearStartRailPoint(getRailCoord(pActor), epsilon);
}

/**
 * @brief Checks whether a direction points along the rail of an actor.
 * @param pActor The actor.
 * @param rDir The direction.
 * @return True if the direction does not point against the rail.
 */
bool isRailPlusDir(const LiveActor* pActor, const sead::Vector3f& rDir) {
    return rDir.dot(getRailDir(pActor)) >= 0.0f;
}

/**
 * @brief Checks whether the side direction of an actor points along its rail.
 * @param pActor The actor.
 * @return True if the side direction does not point against the rail.
 */
bool isRailPlusPoseSide(const LiveActor* pActor) {
    sead::Vector3f sideDir;
    calcSideDir(&sideDir, pActor);
    return isRailPlusDir(pActor, sideDir);
}

/**
 * @brief Checks whether the up direction of an actor points along its rail.
 * @param pActor The actor.
 * @return True if the up direction does not point against the rail.
 */
bool isRailPlusPoseUp(const LiveActor* pActor) {
    sead::Vector3f upDir;
    calcUpDir(&upDir, pActor);
    return isRailPlusDir(pActor, upDir);
}

/**
 * @brief Checks whether the front direction of an actor points along its rail.
 * @param pActor The actor.
 * @return True if the front direction does not point against the rail.
 */
bool isRailPlusPoseFront(const LiveActor* pActor) {
    sead::Vector3f frontDir;
    calcFrontDir(&frontDir, pActor);
    return isRailPlusDir(pActor, frontDir);
}

/**
 * @brief Computes a position on the rail of an actor.
 * @param pPos Where the position is written.
 * @param pActor The actor.
 * @param coord The distance from the start of the rail.
 */
void calcRailPosAtCoord(sead::Vector3f* pPos, const LiveActor* pActor, f32 coord) {
    getRail(pActor)->calcPos(pPos, coord);
}

/**
 * @brief Computes the moving direction of the rail rider of an actor.
 * @param pDir Where the direction is written.
 * @param pActor The actor.
 */
void calcRailMoveDir(sead::Vector3f* pDir, const LiveActor* pActor) {
    pDir->set(isRailGoingToEnd(pActor) ? getRailDir(pActor) : -getRailDir(pActor));
}

/**
 * @brief Computes a direction on the rail of an actor.
 * @param pDir Where the direction is written.
 * @param pActor The actor.
 * @param coord The distance from the start of the rail.
 */
void calcRailDirAtCoord(sead::Vector3f* pDir, const LiveActor* pActor, f32 coord) {
    getRail(pActor)->calcDirection(pDir, coord);
}

/**
 * @brief Computes a direction on a rail.
 * @param pDir Where the direction is written.
 * @param pKeeper The rail keeper.
 * @param coord The distance from the start of the rail.
 */
void calcRailDirAtCoord(sead::Vector3f* pDir, const RailKeeper* pKeeper, f32 coord) {
    pKeeper->getRail()->calcDirection(pDir, coord);
}

/**
 * @brief Computes the position on the rail ahead of the rail rider of an actor.
 * @param pPos Where the position is written.
 * @param pActor The actor.
 * @param offset The distance ahead in the moving direction.
 */
void calcRailPosFront(sead::Vector3f* pPos, const LiveActor* pActor, f32 offset) {
    if (!isRailGoingToEnd(pActor)) {
        offset = -offset;
    }

    f32 coord = getRailCoord(pActor) + offset;
    const Rail* rail = getRail(pActor);

    if (rail->isClosed()) {
        f32 totalLength = rail->getTotalLength();
        rail->calcPos(pPos, modf(coord + totalLength, totalLength) + 0.0f);
        return;
    }

    f32 totalLength = rail->getTotalLength();
    f32 clampedCoord = coord > totalLength ? totalLength : coord;
    rail->calcPos(pPos, coord < 0.0f ? 0.0f : clampedCoord);
}

/**
 * @brief Gets the coordinate of a rail point of an actor.
 * @param pActor The actor.
 * @param index The rail point index.
 * @return The distance from the start of the rail.
 */
f32 calcRailCoordByPoint(const LiveActor* pActor, s32 index) {
    return getRail(pActor)->getLengthToPoint(index);
}

/**
 * @brief Computes the bounding sphere of the rail of an actor.
 * @param pPos Where the sphere center is written.
 * @param pRadius Where the sphere radius is written.
 * @param pActor The actor.
 * @param step The sampling step.
 * @param offset The margin added to the radius.
 */
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

/**
 * @brief Computes the bounding sphere of a rail.
 * @param pPos Where the sphere center is written.
 * @param pRadius Where the sphere radius is written.
 * @param pKeeper The rail keeper.
 * @param step The sampling step.
 * @param offset The margin added to the radius.
 */
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

/**
 * @brief Sets the clipping sphere of an actor to the bounding sphere of its rail.
 * @param pPos Where the sphere center is written, and used by the clipping.
 * @param pActor The actor.
 * @param step The sampling step.
 * @param offset The margin added to the radius.
 */
void setRailClippingInfo(sead::Vector3f* pPos, LiveActor* pActor, f32 step, f32 offset) {
    f32 radius = 0.0f;
    calcRailClippingInfo(pPos, &radius, pActor, step, offset);
    setClippingInfo(pActor, radius, pPos);
}

/**
 * @brief Sets the clipping sphere of an actor to the bounding sphere of a rail.
 * @param pPos Where the sphere center is written, and used by the clipping.
 * @param pActor The actor.
 * @param pKeeper The rail keeper.
 * @param step The sampling step.
 * @param offset The margin added to the radius.
 */
void setRailClippingInfo(sead::Vector3f* pPos, LiveActor* pActor, const RailKeeper* pKeeper,
                         f32 step, f32 offset) {
    f32 radius = 0.0f;
    calcRailClippingInfo(pPos, &radius, pKeeper, step, offset);
    setClippingInfo(pActor, radius, pPos);
}

/**
 * @brief Gets the number of rail points of a rail holder.
 * @param pRailHolder The rail holder.
 * @return The number of rail points.
 */
s32 getRailPointNum(const IUseRail* pRailHolder) {
    return pRailHolder->getRailRider()->getRail()->getRailPointsCount();
}

/**
 * @brief Gets the placement info of a rail point of a rail holder.
 * @param pRailHolder The rail holder.
 * @param index The rail point index.
 * @return The placement info.
 */
PlacementInfo* getRailPointInfo(const IUseRail* pRailHolder, s32 index) {
    return pRailHolder->getRailRider()->getRail()->getRailPoint(index);
}

}  // namespace al
