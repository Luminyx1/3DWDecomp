#include "Library/Math/ParabolicPath.hpp"

#include "Library/Math/MathUtil.hpp"

namespace al {

/**
 * Constructs a straight path along the Z axis.
 */
ParabolicPath::ParabolicPath() = default;

/**
 * Initializes the path between two points, using the up vector's length as the maximum height.
 * @param rStart Start position.
 * @param rEnd End position.
 * @param rUp Up vector scaled by the maximum height.
 */
void ParabolicPath::initFromUpVector(const sead::Vector3f& rStart, const sead::Vector3f& rEnd,
                                     const sead::Vector3f& rUp) {
    f32 scalar;
    sead::Vector3f upDir;
    separateScalarAndDirection(&scalar, &upDir, rUp);
    initFromUpVector(rStart, rEnd, upDir, scalar);
}

/**
 * Initializes the path between two points.
 * @param rStart Start position.
 * @param rEnd End position.
 * @param rUp Up direction.
 * @param maxHeight Maximum height above the start.
 */
void ParabolicPath::initFromUpVector(const sead::Vector3f& rStart, const sead::Vector3f& rEnd,
                                     const sead::Vector3f& rUp, f32 maxHeight) {
    mUp.set(rUp);
    sead::Vector3f diff = rEnd - rStart;
    f32 verticalDistance = diff.dot(mUp);
    mHorizontalDirection = diff - (mUp * verticalDistance);
    separateScalarAndDirection(&mHorizontalDistance, &mHorizontalDirection, mHorizontalDirection);
    calcParabolicFunctionParam(&mGravity, &mInitialVelY, maxHeight, verticalDistance);
    mStart.set(rStart);
}

/**
 * Initializes the path between two points, reaching the height of a projected end point.
 * @param rStart Start position.
 * @param rEnd End position.
 * @param rProjectedEnd Point above the end position marking the maximum height.
 */
void ParabolicPath::initFromMaxHeight(const sead::Vector3f& rStart, const sead::Vector3f& rEnd,
                                      const sead::Vector3f& rProjectedEnd) {
    sead::Vector3f up;
    f32 scalar;
    separateScalarAndDirection(&scalar, &up, rProjectedEnd - rEnd);
    initFromUpVector(rStart, rEnd, up, (rProjectedEnd - rStart).dot(up));
}

/**
 * Initializes the path between two points, peaking a fixed height above the higher point.
 * @param rStart Start position.
 * @param rEnd End position.
 * @param rUp Up direction.
 * @param height Height to add.
 */
void ParabolicPath::initFromUpVectorAddHeight(const sead::Vector3f& rStart,
                                              const sead::Vector3f& rEnd, const sead::Vector3f& rUp,
                                              f32 height) {
    f32 verticalDistance = sead::Mathf::clampMin((rEnd - rStart).dot(rUp), 0.0f);
    initFromUpVector(rStart, rEnd, rUp, verticalDistance + height);
}

/**
 * Approximates the length of a part of the path.
 * @param start Start progress.
 * @param end End progress.
 * @param iterations Number of segments to sum.
 * @return Approximate length.
 */
f32 ParabolicPath::getLength(f32 start, f32 end, s32 iterations) const {
    s32 steps = sead::Mathi::clampMin(iterations, 1);
    f32 stepSize = (end - start) / steps;
    f32 squaredHStepSize = sead::Mathf::square(stepSize * mHorizontalDistance);
    f32 vDist = (mGravity * start + mInitialVelY) * start;
    f32 length = 0.0f;

    for (s32 i = 0; i < steps;) {
        i++;
        f32 prevVDist = vDist;
        f32 curStep = start + i * stepSize;
        vDist = (mGravity * curStep + mInitialVelY) * curStep;
        length += sead::Mathf::sqrt(squaredHStepSize + sead::Mathf::square(vDist - prevVDist));
    }

    return length;
}

/**
 * Approximates the length of the whole path.
 * @param iterations Number of segments to sum.
 * @return Approximate length.
 */
f32 ParabolicPath::getTotalLength(s32 iterations) const {
    return getLength(0.0f, 1.0f, iterations);
}

/**
 * Calculates a position with separate horizontal and vertical progress.
 * @param pPos Receives the position.
 * @param h Horizontal progress.
 * @param v Vertical progress.
 */
void ParabolicPath::calcPositionHV(sead::Vector3f* pPos, f32 h, f32 v) const {
    f32 hDist = mHorizontalDistance * h;
    f32 vDist = (mGravity * v + mInitialVelY) * v;
    *pPos = mStart + (vDist * mUp) + (hDist * mHorizontalDirection);
}

/**
 * Calculates a position on the path.
 * @param pPos Receives the position.
 * @param prog Progress along the path.
 */
void ParabolicPath::calcPosition(sead::Vector3f* pPos, f32 prog) const {
    calcPositionHV(pPos, prog, prog);
}

/**
 * Calculates a position on the path, easing out the horizontal progress.
 * @param pPos Receives the position.
 * @param prog Progress along the path.
 */
void ParabolicPath::calcPositionEaseOutH(sead::Vector3f* pPos, f32 prog) const {
    calcPositionHV(pPos, easeOut(prog), prog);
}

/**
 * Calculates the direction of the path at a progress.
 * @param pDir Receives the direction.
 * @param prog Progress along the path.
 * @param stepSize Progress step used for sampling.
 */
void ParabolicPath::calcDirection(sead::Vector3f* pDir, f32 prog, f32 stepSize) const {
    f32 prog1, prog2;

    if (prog < stepSize) {
        prog1 = 0.0f;
        prog2 = stepSize;
    } else if ((1.0f - stepSize) < prog) {
        prog2 = 1.0f;
        prog1 = 1.0f - stepSize;
    } else {
        prog2 = prog + stepSize;
        prog1 = prog;
    }

    sead::Vector3f pos1, pos2;
    calcPosition(&pos1, prog1);
    calcPosition(&pos2, prog2);
    *pDir = pos2 - pos1;
    normalizeOrZero(pDir);
}

/**
 * Calculates the progress speed matching a gravity acceleration.
 * @param frames Gravity acceleration.
 * @return Progress per step.
 */
f32 ParabolicPath::calcPathSpeedFromGravityAccel(f32 frames) const {
    return sead::Mathf::abs(frames / mGravity);
}

/**
 * Calculates the progress speed matching an average speed.
 * @param frames Average speed.
 * @return Progress per step.
 */
f32 ParabolicPath::calcPathSpeedFromAverageSpeed(f32 frames) const {
    return frames / getTotalLength(10);
}

/**
 * Calculates the progress speed matching a horizontal speed.
 * @param frames Horizontal speed.
 * @return Progress per step.
 */
f32 ParabolicPath::calcPathSpeedFromHorizontalSpeed(f32 frames) const {
    return frames / mHorizontalDistance;
}

/**
 * Calculates the number of steps needed with a gravity acceleration.
 * @param frames Gravity acceleration.
 * @return Number of steps.
 */
s32 ParabolicPath::calcPathTimeFromGravityAccel(f32 frames) const {
    return 1.0f / calcPathSpeedFromGravityAccel(frames);
}

/**
 * Calculates the number of steps needed with an average speed.
 * @param frames Average speed.
 * @return Number of steps.
 */
s32 ParabolicPath::calcPathTimeFromAverageSpeed(f32 frames) const {
    return 1.0f / calcPathSpeedFromAverageSpeed(frames);
}

/**
 * Calculates the number of steps needed with a horizontal speed.
 * @param frames Horizontal speed.
 * @return Number of steps.
 */
s32 ParabolicPath::calcPathTimeFromHorizontalSpeed(f32 frames) const {
    return 1.0f / calcPathSpeedFromHorizontalSpeed(frames);
}

}  // namespace al
