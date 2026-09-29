#include "Library/Rail/Rail.hpp"

#include "Library/Math/MathUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Library/Rail/RailPart.hpp"

namespace al {

/**
 * @brief Constructs an empty rail.
 */
Rail::Rail() = default;

/**
 * @brief Builds the rail parts from the rail points of a placement.
 * @param rInfo The rail placement info.
 */
void Rail::init(const PlacementInfo& rInfo) {
    mIsClosed = false;
    tryGetArg(&mIsClosed, rInfo, "IsClosed");
    PlacementInfo railPointsInfo;
    tryGetPlacementInfoByKey(&railPointsInfo, rInfo, "RailPoints");
    mRailPointsCount = getCountPlacementInfo(railPointsInfo);
    if (mRailPointsCount <= 0) {
        return;
    }

    mRailPoints = new PlacementInfo*[mRailPointsCount];
    for (s32 i = 0; i < mRailPointsCount; i++) {
        mRailPoints[i] = new PlacementInfo();
        tryGetPlacementInfoByIndex(mRailPoints[i], railPointsInfo, i);
    }

    if (mRailPointsCount == 1) {
        mRailPartCount = 1;
        mRailParts = new RailPart[1];
        PlacementInfo partInfo;
        tryGetPlacementInfoByIndex(&partInfo, railPointsInfo, 0);
        sead::Vector3f pos = sead::Vector3f::zero;
        tryGetRailPointPos(&pos, partInfo);
        mRailParts->init(pos, pos, pos, pos);
        return;
    }

    mRailPartCount = mRailPointsCount + mIsClosed - 1;
    mRailParts = new RailPart[mRailPartCount];

    f32 totalLength = 0.0f;
    for (s32 i = 0; i < mRailPartCount; i++) {
        PlacementInfo startInfo;
        PlacementInfo endInfo;
        tryGetPlacementInfoByIndex(&startInfo, railPointsInfo, i);
        tryGetPlacementInfoByIndex(&endInfo, railPointsInfo, (i + 1) % mRailPointsCount);

        sead::Vector3f start = sead::Vector3f::zero;
        sead::Vector3f startHandle = sead::Vector3f::zero;
        sead::Vector3f endHandle = sead::Vector3f::zero;
        sead::Vector3f end = sead::Vector3f::zero;
        tryGetRailPointPos(&start, startInfo);
        getRailPointHandleNext(&startHandle, startInfo);
        getRailPointHandlePrev(&endHandle, endInfo);
        tryGetRailPointPos(&end, endInfo);
        mRailParts[i].init(start, startHandle, endHandle, end);

        f32 accelStart = 0.0f;
        f32 accelEnd = 0.0f;
        tryGetArg(&accelStart, startInfo, "acceleration");
        tryGetArg(&accelEnd, endInfo, "acceleration");
        sead::Vector3f upStart;
        sead::Vector3f upEnd;
        tryGetUp(&upStart, startInfo);
        tryGetUp(&upEnd, endInfo);
        mRailParts[i].setUp(upStart, upEnd);
        mRailParts[i].setAccel(accelStart, accelEnd);

        bool isUseSetAngleStart = false;
        bool isUseSetAngleEnd;
        f32 angle;
        tryGetArg(&isUseSetAngleStart, startInfo, "isUseSetAngle");
        if (isUseSetAngleStart) {
            angle = 0.0f;
            tryGetArg(&angle, startInfo, "angle");
            mRailParts[i].setAngleS(angle);
        }

        tryGetArg(&isUseSetAngleEnd, endInfo, "isUseSetAngle");
        if (isUseSetAngleEnd) {
            angle = 0.0f;
            tryGetArg(&angle, endInfo, "angle");
            mRailParts[i].setAngleE(angle);
        }

        totalLength += mRailParts[i].getPartLength();
        mRailParts[i].setTotalDistance(totalLength);
    }
}

/**
 * @brief Computes a point on the rail.
 * @param pPos Where the point is written.
 * @param distance The distance from the start of the rail.
 */
void Rail::calcPos(sead::Vector3f* pPos, f32 distance) const {
    const RailPart* part = nullptr;
    f32 partDistance = 0.0f;
    getIncludedSection(&part, &partDistance, distance);
    part->calcPos(pPos, part->calcCurveParam(partDistance));
}

/**
 * @brief Finds the rail part that contains a distance.
 * @param pPart Where the part is written, may be null.
 * @param pPartDistance Where the distance within the part is written, may be null.
 * @param distance The distance from the start of the rail.
 * @return The index of the part.
 */
s32 Rail::getIncludedSection(const RailPart** pPart, f32* pPartDistance, f32 distance) const {
    f32 distanceOnRail = normalizeLength(distance);
    f32 startDistanceOnRail = 0.0f;
    s32 index = -1;
    for (s32 i = 0; i < mRailPartCount; i++) {
        if (distanceOnRail <= mRailParts[i].getTotalDistance()) {
            if (i <= 0) {
                startDistanceOnRail = distanceOnRail;
            } else {
                startDistanceOnRail = distanceOnRail - mRailParts[i - 1].getTotalDistance();
            }
            index = i;
            break;
        }
    }

    if (pPart) {
        *pPart = &mRailParts[index];
    }
    if (pPartDistance) {
        *pPartDistance =
            sead::Mathf::clamp(startDistanceOnRail, 0.0f, (*pPart)->getPartLength());
    }

    return index;
}

/**
 * @brief Computes the up direction at a distance.
 * @param pUp Where the normalized up direction is written.
 * @param distance The distance from the start of the rail.
 */
void Rail::calcUpDir(sead::Vector3f* pUp, f32 distance) const {
    f32 distanceOnRail = normalizeLength(distance);
    f32 partDistance = distanceOnRail;
    s32 index = -1;
    for (s32 i = 0; i < mRailPartCount; i++) {
        if (distanceOnRail <= mRailParts[i].getTotalDistance()) {
            index = i;
            break;
        }
        partDistance -= mRailParts[i].getPartLength();
    }

    mRailParts[index].getPartLength();
    mRailParts[index].calcUpDir(pUp, partDistance);
    pUp->normalize();
}

/**
 * @brief Wraps a distance for closed rails or clamps it for open ones.
 * @param distance The distance from the start of the rail.
 * @return The distance on the rail.
 */
f32 Rail::normalizeLength(f32 distance) const {
    if (mIsClosed) {
        f32 distanceOnRail = modf(distance, getTotalLength());
        if (distanceOnRail < 0.0f) {
            distanceOnRail += getTotalLength();
        }
        return distanceOnRail;
    }

    return sead::Mathf::clamp(distance, 0.0f, getTotalLength());
}

/**
 * @brief Computes the direction at a distance.
 * @param pDir Where the direction is written.
 * @param distance The distance from the start of the rail.
 */
void Rail::calcDirection(sead::Vector3f* pDir, f32 distance) const {
    const RailPart* part = nullptr;
    f32 partDistance = 0.0f;
    getIncludedSection(&part, &partDistance, distance);
    part->calcDir(pDir, part->calcCurveParam(partDistance));
}

/**
 * @brief Computes the position and the direction at a distance.
 * @param pPos Where the position is written.
 * @param pDir Where the direction is written.
 * @param distance The distance from the start of the rail.
 */
void Rail::calcPosDir(sead::Vector3f* pPos, sead::Vector3f* pDir, f32 distance) const {
    const RailPart* part = nullptr;
    f32 partDistance = 0.0f;
    getIncludedSection(&part, &partDistance, distance);
    f32 curveParam = part->calcCurveParam(partDistance);
    part->calcPos(pPos, curveParam);
    part->calcDir(pDir, curveParam);
}

/**
 * @brief Gets the length of the whole rail.
 * @return The length.
 */
f32 Rail::getTotalLength() const {
    return mRailParts[mRailPartCount - 1].getTotalDistance();
}

/**
 * @brief Gets the length of a rail part.
 * @param index The part index.
 * @return The length.
 */
f32 Rail::getPartLength(s32 index) const {
    return mRailParts[index].getPartLength();
}

/**
 * @brief Gets the distance from the start of the rail to a rail point.
 * @param index The rail point index.
 * @return The distance.
 */
f32 Rail::getLengthToPoint(s32 index) const {
    if (index == 0) {
        return 0.0f;
    }
    return mRailParts[index - 1].getTotalDistance();
}

/**
 * @brief Computes the position of a rail point.
 * @param pPos Where the position is written.
 * @param index The rail point index.
 */
void Rail::calcRailPointPos(sead::Vector3f* pPos, s32 index) const {
    if (mIsClosed || index != mRailPointsCount - 1) {
        return mRailParts[index].calcStartPos(pPos);
    }

    return mRailParts[index - 1].calcEndPos(pPos);
}

/**
 * @brief Finds the rail point nearest to a position.
 * @param pRailPos Where the rail point position is written.
 * @param rPos The position.
 */
void Rail::calcNearestRailPointPos(sead::Vector3f* pRailPos, const sead::Vector3f& rPos) const {
    if (mRailPointsCount == 0) {
        return;
    }

    sead::Vector3f pointPos = sead::Vector3f::zero;
    calcRailPointPos(&pointPos, 0);
    f32 bestDistance = (rPos - pointPos).squaredLength();

    for (s32 i = 1; i < mRailPointsCount; i++) {
        calcRailPointPos(&pointPos, i);
        if ((rPos - pointPos).squaredLength() < bestDistance) {
            bestDistance = (rPos - pointPos).squaredLength();
            *pRailPos = pointPos;
        }
    }
}

/**
 * @brief Finds the distance on the rail nearest to a position.
 * @param rPos The position.
 * @param interval The sampling step.
 * @return The distance from the start of the rail.
 */
f32 Rail::calcNearestRailPosCoord(const sead::Vector3f& rPos, f32 interval) const {
    f32 bestDistance = sead::Mathf::maxNumber();
    f32 bestLength = sead::Mathf::maxNumber();
    s32 bestIndex = 0;
    for (s32 i = 0; i < mRailPartCount; i++) {
        RailPart* part = &mRailParts[i];
        f32 length;
        f32 distance = part->calcNearestLength(&length, rPos, part->getPartLength(), interval);
        if (distance < bestDistance) {
            bestDistance = distance;
            bestIndex = i;
            bestLength = length;
        }
    }

    if (bestIndex > 0) {
        bestLength = bestLength + mRailParts[bestIndex - 1].getTotalDistance();
    }
    return bestLength;
}

/**
 * @brief Finds the point on the rail nearest to a position.
 * @param pRailPos Where the nearest point is written.
 * @param rPos The position.
 * @param interval The sampling step.
 * @return The distance of the nearest point from the start of the rail.
 */
f32 Rail::calcNearestRailPos(sead::Vector3f* pRailPos, const sead::Vector3f& rPos,
                             f32 interval) const {
    f32 coord = calcNearestRailPosCoord(rPos, interval);
    const RailPart* part = nullptr;
    f32 partDistance = 0.0f;
    getIncludedSection(&part, &partDistance, coord);
    part->calcPos(pRailPos, part->calcCurveParam(partDistance));
    return coord;
}

/**
 * @brief Checks whether a distance is near a rail point.
 * @param distance The distance from the start of the rail.
 * @param epsilon The tolerance.
 * @return True if near the start or the end of a part.
 */
bool Rail::isNearRailPoint(f32 distance, f32 epsilon) const {
    const RailPart* part = nullptr;
    f32 partDistance;
    getIncludedSection(&part, &partDistance, distance);

    return (part->getPartLength() - partDistance) < epsilon;
}

/**
 * @brief Checks whether a distance is near the end of its part.
 * @param distance The distance from the start of the rail.
 * @param epsilon The tolerance.
 * @return True if near the end of the part.
 */
bool Rail::isNearEndRailPoint(f32 distance, f32 epsilon) const {
    const RailPart* part = nullptr;
    f32 partDistance;
    getIncludedSection(&part, &partDistance, distance);

    return (part->getPartLength() - partDistance) < epsilon;
}

/**
 * @brief Checks whether a distance is near the start of its part.
 * @param distance The distance from the start of the rail.
 * @param epsilon The tolerance.
 * @return True if near the start of the part.
 */
bool Rail::isNearStartRailPoint(f32 distance, f32 epsilon) const {
    const RailPart* part = nullptr;
    f32 partDistance;
    getIncludedSection(&part, &partDistance, distance);

    return partDistance < epsilon;
}

/**
 * @brief Counts the rail points between two distances.
 * @param distance1 The start distance.
 * @param distance2 The end distance.
 * @return The number of rail points.
 */
s32 Rail::calcRailPointNum(f32 distance1, f32 distance2) {
    if ((distance2 - distance1) < 0.01f) {
        return 0;
    }

    const RailPart* part1 = nullptr;
    const RailPart* part2 = nullptr;
    f32 partDistance1;
    f32 partDistance2;
    s32 section1 = getIncludedSection(&part1, &partDistance1, distance1);
    s32 section2 = getIncludedSection(&part2, &partDistance2, distance2);

    return ((section2 - section1) + (partDistance1 < 0.01f)) +
           ((part2->getPartLength() - partDistance2) < 0.01f);
}

/**
 * @brief Gets the length of the part that contains a distance.
 * @param pPartDistance Where the distance within the part is written.
 * @param pRemaining Where the remaining length of the part is written.
 * @param distance The distance from the start of the rail.
 * @return The length of the part.
 */
f32 Rail::getIncludedSectionLength(f32* pPartDistance, f32* pRemaining, f32 distance) const {
    const RailPart* part = nullptr;
    getIncludedSection(&part, pPartDistance, distance);
    f32 partLength = part->getPartLength();
    if (pPartDistance && pRemaining) {
        *pRemaining = partLength - *pPartDistance;
    }
    return partLength;
}

/**
 * @brief Gets the index of the part that contains a distance.
 * @param distance The distance from the start of the rail.
 * @return The part index.
 */
s32 Rail::getIncludedSectionIndex(f32 distance) const {
    return getIncludedSection(nullptr, nullptr, distance);
}

/**
 * @brief Gets the accelerations of a part.
 * @param index The part index.
 * @param pAccelStart Where the acceleration at the start is written.
 * @param pAccelEnd Where the acceleration at the end is written.
 */
void Rail::getAccels(s32 index, f32* pAccelStart, f32* pAccelEnd) {
    if (index >= 0 && index < mRailPartCount) {
        mRailParts[index].getAccels(pAccelStart, pAccelEnd);
    }
}

/**
 * @brief Gets the start angle of a part.
 * @param index The part index.
 * @param pAngle Where the angle is written.
 * @return True if the angle was set.
 */
bool Rail::getAngleS(s32 index, f32* pAngle) {
    if (index >= 0 && index < mRailPartCount) {
        return mRailParts[index].getAngleS(pAngle);
    }
    return false;
}

/**
 * @brief Gets the end angle of a part.
 * @param index The part index.
 * @param pAngle Where the angle is written.
 * @return True if the angle was set.
 */
bool Rail::getAngleE(s32 index, f32* pAngle) {
    if (index >= 0 && index < mRailPartCount) {
        return mRailParts[index].getAngleE(pAngle);
    }
    return false;
}

/**
 * @brief Checks whether any part of the rail is a curve.
 * @return True if a part is a bezier curve.
 */
bool Rail::isIncludeBezierRailPart() const {
    for (s32 i = 0; i < mRailPartCount; i++) {
        if (mRailParts[i].isBezierCurve()) {
            return true;
        }
    }
    return false;
}

}  // namespace al
