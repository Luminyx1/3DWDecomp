#include "Library/Rail/RailRider.hpp"

#include "Library/Math/MathUtil.hpp"
#include "Library/Rail/Rail.hpp"

namespace al {

/**
 * @brief Constructs a rider at the start of a rail.
 * @param pRail The rail to ride.
 */
RailRider::RailRider(const Rail* pRail) : mRail(pRail) {
    syncPosDir();
}

/**
 * @brief Moves to the start of the rail.
 */
void RailRider::moveToRailStart() {
    mCoord = 0.0f;
    syncPosDir();
}

/**
 * @brief Moves by the current speed in the current direction.
 */
void RailRider::move() {
    mCoord += mIsMoveForwards ? mSpeed : -mSpeed;
    syncPosDir();
}

/**
 * @brief Normalizes the coordinate and updates the position and direction.
 */
void RailRider::syncPosDir() {
    mCoord = mRail->normalizeLength(mCoord);
    mRail->calcPosDir(&mPosition, &mDirection, mCoord);
}

/**
 * @brief Moves to a coordinate.
 * @param coord The distance from the start of the rail.
 */
void RailRider::setCoord(f32 coord) {
    mCoord = coord;
    syncPosDir();
}

/**
 * @brief Moves to the end of the rail.
 */
void RailRider::moveToRailEnd() {
    mCoord = mRail->getTotalLength();
    syncPosDir();
}

/**
 * @brief Moves to the end the rider is coming from.
 */
void RailRider::moveToBegin() {
    if (mIsMoveForwards) {
        moveToRailStart();
    } else {
        moveToRailEnd();
    }
}

/**
 * @brief Moves to the end the rider is going to.
 */
void RailRider::moveToGoal() {
    if (mIsMoveForwards) {
        moveToRailEnd();
    } else {
        moveToRailStart();
    }
}

/**
 * @brief Moves to the point on the rail nearest to a position.
 * @param rPos The position.
 */
void RailRider::moveToNearestRail(const sead::Vector3f& rPos) {
    mCoord = mRail->calcNearestRailPosCoord(rPos, 20.0f);
    syncPosDir();
}

/**
 * @brief Computes the up direction at the current coordinate.
 * @param pUp Where the up direction is written.
 */
void RailRider::getUpDir(sead::Vector3f* pUp) {
    mCoord = mRail->normalizeLength(mCoord);
    mRail->calcUpDir(pUp, mCoord);
}

/**
 * @brief Reverses the moving direction.
 */
void RailRider::reverse() {
    mIsMoveForwards = !mIsMoveForwards;
}

/**
 * @brief Makes the rider move towards the start.
 */
void RailRider::setMoveGoingStart() {
    mIsMoveForwards = false;
}

/**
 * @brief Makes the rider move towards the end.
 */
void RailRider::setMoveGoingEnd() {
    mIsMoveForwards = true;
}

/**
 * @brief Sets the speed.
 * @param speed The new speed.
 */
void RailRider::setSpeed(f32 speed) {
    mSpeed = speed;
}

/**
 * @brief Adds to the speed.
 * @param speed The speed to add.
 */
void RailRider::addSpeed(f32 speed) {
    mSpeed += speed;
}

/**
 * @brief Scales the speed.
 * @param scale The scale factor.
 */
void RailRider::scaleSpeed(f32 scale) {
    mSpeed *= scale;
}

/**
 * @brief Checks whether the rider reached the end it is moving to.
 * @return True if the goal was reached.
 */
bool RailRider::isReachedGoal() const {
    if (mIsMoveForwards) {
        return isReachedRailEnd();
    }
    return isReachedRailStart();
}

/**
 * @brief Checks whether the rider is at the end of an open rail.
 * @return True if at the end.
 */
bool RailRider::isReachedRailEnd() const {
    if (mRail->isClosed()) {
        return false;
    }
    return isNearZero(mCoord - mRail->getTotalLength(), 0.001f);
}

/**
 * @brief Checks whether the rider is at the start of an open rail.
 * @return True if at the start.
 */
bool RailRider::isReachedRailStart() const {
    if (mRail->isClosed()) {
        return false;
    }
    return isNearZero(mCoord, 0.001f);
}

/**
 * @brief Checks whether the rider is at either end of an open rail.
 * @return True if at the start or at the end.
 */
bool RailRider::isReachedEdge() const {
    return isReachedRailStart() || isReachedRailEnd();
}

}  // namespace al
