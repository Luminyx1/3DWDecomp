#include "Library/Screen/ScreenPointDirector.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Screen/ScreenFunction.hpp"
#include "Library/Screen/ScreenPointCheckGroup.hpp"
#include "Library/Screen/ScreenPointTarget.hpp"
#include "Library/Screen/ScreenPointer.hpp"

namespace al {
namespace {

/**
 * Compares two hit infos by distance.
 * @param pA first hit info
 * @param pB second hit info
 * @return negative, zero or positive like a three-way comparison
 */
inline s32 compareHitInfoDistance(const ScreenPointTargetHitInfo* pA,
                                  const ScreenPointTargetHitInfo* pB) {
    if (pA->mDistance < pB->mDistance) {
        return -1;
    }

    if (pA->mDistance > pB->mDistance) {
        return 1;
    }

    return 0;
}

/**
 * Sorts hit infos by ascending distance.
 * @param pHitInfos hit infos to sort
 */
inline void sortHitInfoByDistance(ScreenPointTargetHitInfoArray* pHitInfos) {
    pHitInfos->shakerSort_<ScreenPointTargetHitInfo>(compareHitInfoDistance);
}

}  // namespace

/**
 * Creates the screen point director.
 * @param maxTargets maximum number of targets, or a non-positive value for the default
 */
ScreenPointDirector::ScreenPointDirector(s32 maxTargets) : mCheckGroup(nullptr) {
    if (maxTargets < 1) {
        maxTargets = 0x500;
    }

    mCheckGroup = new ScreenPointCheckGroup(maxTargets);
}

/**
 * Registers a target.
 * @param pTarget target to register
 */
void ScreenPointDirector::registerTarget(ScreenPointTarget* pTarget) {
    mCheckGroup->setTarget(pTarget);
}

/**
 * Assigns the check group of a target.
 * @param pTarget target
 */
void ScreenPointDirector::setCheckGroup(ScreenPointTarget* pTarget) {
    pTarget->setCheckGroup(mCheckGroup);
}

/**
 * Collects the valid targets hit by a segment, sorted by distance from the segment start.
 * @param pHitInfos array receiving the hits
 * @param maxHits maximum number of hits to collect
 * @param rStart segment start
 * @param rEnd segment end
 * @return whether any target was hit
 */
bool ScreenPointDirector::hitCheckSegment(ScreenPointTargetHitInfoArray* pHitInfos, s32 maxHits,
                                          const sead::Vector3f& rStart,
                                          const sead::Vector3f& rEnd) {
    pHitInfos->clear();

    for (s32 i = 0; i < mCheckGroup->getValidTargetNum(); i++) {
        sead::Vector3f hitPos = sead::Vector3f::zero;
        sead::Vector3f hitNormal = sead::Vector3f::zero;
        ScreenPointTarget* target = mCheckGroup->getTarget(i);
        sead::Vector3f startToTarget = target->getPos() - rStart;
        f32 radius = target->getRadius();

        if (startToTarget.length() <= radius) {
            continue;
        }

        if (!checkHitSegmentSphereNearDepth(target->getPos(), rStart, rEnd, radius, &hitPos,
                                            &hitNormal)) {
            continue;
        }

        ScreenPointTargetHitInfo hitInfo = {target, (hitPos - rStart).length(), hitPos, hitNormal};

        if (pHitInfos->size() >= maxHits) {
            break;
        }

        pHitInfos->emplaceBack(hitInfo);
    }

    if (pHitInfos->size() >= 2) {
        sortHitInfoByDistance(pHitInfos);
    }

    return pHitInfos->size() > 0;
}

/**
 * Collects the valid targets overlapping a circle on screen, sorted by screen distance.
 * @param pHitInfos array receiving the hits
 * @param maxHits maximum number of hits to collect
 * @param rPos circle center on screen
 * @param radius circle radius on screen
 * @return whether any target was hit
 */
bool ScreenPointDirector::hitCheckScreenCircle(ScreenPointTargetHitInfoArray* pHitInfos,
                                               s32 maxHits, const sead::Vector2f& rPos,
                                               f32 radius) {
    pHitInfos->clear();

    for (s32 i = 0; i < mCheckGroup->getValidTargetNum(); i++) {
        sead::Vector3f hitPos = sead::Vector3f::zero;
        sead::Vector3f hitNormal = sead::Vector3f::zero;
        ScreenPointTarget* target = mCheckGroup->getTarget(i);
        f32 targetRadius = target->getRadius();
        const sead::Vector3f& targetPos = target->getPos();
        const IUseCamera* camera = target->getHost();
        f32 screenRadius = calcScreenRadiusFromWorldRadius(targetPos, camera, targetRadius);
        sead::Vector2f screenPos;
        calcScreenPosFromWorldPos(&screenPos, camera, targetPos, 0);
        sead::Vector2f posToTarget = screenPos - rPos;

        if (screenRadius + radius < posToTarget.length()) {
            continue;
        }

        ScreenPointTargetHitInfo hitInfo = {target, posToTarget.length(), hitPos, hitNormal};

        if (pHitInfos->size() >= maxHits) {
            break;
        }

        pHitInfos->emplaceBack(hitInfo);
    }

    if (pHitInfos->size() >= 2) {
        sortHitInfoByDistance(pHitInfos);
    }

    return pHitInfos->size() > 0;
}
}  // namespace al
