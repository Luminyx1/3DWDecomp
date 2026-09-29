#include "Library/Screen/ScreenPointDirector.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Screen/ScreenPointCheckGroup.hpp"
#include "Library/Screen/ScreenPointTarget.hpp"
#include "Library/Screen/ScreenPointer.hpp"
#include "Library/Screen/ScreenUtil.hpp"

namespace {
/**
 * @brief Sorts hit infos by ascending distance with a cocktail shaker sort.
 * @param pArray The hit infos to sort.
 */
inline void sortHitInfoByDistance(sead::ObjArray<al::ScreenPointTargetHitInfo>* pArray) {
    s32 size = pArray->size();
    if (size < 2) {
        return;
    }

    al::ScreenPointTargetHitInfo** hitInfos = pArray->data();
    s32 low = 0;
    s32 high = size - 1;
    while (low < high) {
        s32 last = low;
        for (s32 i = low; i < high; i++) {
            if (hitInfos[i]->mDistance > hitInfos[i + 1]->mDistance) {
                al::ScreenPointTargetHitInfo* tmp = hitInfos[i + 1];
                hitInfos[i + 1] = hitInfos[i];
                hitInfos[i] = tmp;
                last = i;
            }
        }
        high = last;

        last = high;
        for (s32 i = high; i > low; i--) {
            if (hitInfos[i]->mDistance < hitInfos[i - 1]->mDistance) {
                al::ScreenPointTargetHitInfo* tmp = hitInfos[i - 1];
                hitInfos[i - 1] = hitInfos[i];
                hitInfos[i] = tmp;
                last = i;
            }
        }
        low = last;
    }
}
}  // namespace

namespace al {

/**
 * @brief Constructs a director with a check group.
 * @param maxTargets The maximum number of targets, or 0 or less for the default.
 */
ScreenPointDirector::ScreenPointDirector(s32 maxTargets) {
    s32 num = maxTargets < 1 ? 0x500 : maxTargets;
    mCheckGroup = new ScreenPointCheckGroup(num);
}

/**
 * @brief Registers a target to the check group.
 * @param pTarget The target.
 */
void ScreenPointDirector::registerTarget(ScreenPointTarget* pTarget) {
    mCheckGroup->setTarget(pTarget);
}

/**
 * @brief Assigns the check group to a target.
 * @param pTarget The target.
 */
void ScreenPointDirector::setCheckGroup(ScreenPointTarget* pTarget) {
    pTarget->mCheckGroup = mCheckGroup;
}

/**
 * @brief Collects the valid targets hit by a segment, sorted by distance.
 * @param pHitInfos Where the hits are written.
 * @param maxHits The maximum number of hits.
 * @param rStart The segment start.
 * @param rEnd The segment end.
 * @return True if any target was hit.
 */
bool ScreenPointDirector::hitCheckSegment(sead::ObjArray<ScreenPointTargetHitInfo>* pHitInfos,
                                          s32 maxHits, const sead::Vector3f& rStart,
                                          const sead::Vector3f& rEnd) {
    pHitInfos->clear();
    for (s32 i = 0; i < mCheckGroup->getValidTargetNum(); i++) {
        sead::Vector3f hitPos = sead::Vector3f::zero;
        sead::Vector3f hitNormal = sead::Vector3f::zero;
        ScreenPointTarget* target = mCheckGroup->getTarget(i);
        if ((target->mPos - rStart).length() <= target->mRadius ||
            !checkHitSegmentSphereNearDepth(target->mPos, rStart, rEnd, target->mRadius, &hitPos,
                                            &hitNormal)) {
            continue;
        }

        f32 distance = (hitPos - rStart).length();
        ScreenPointTargetHitInfo hitInfo = {target, distance, hitPos, hitNormal};
        if (pHitInfos->size() >= maxHits) {
            break;
        }
        pHitInfos->pushBack(hitInfo);
    }

    sortHitInfoByDistance(pHitInfos);
    return pHitInfos->size() > 0;
}

/**
 * @brief Collects the valid targets hit by a circle on screen, sorted by distance.
 * @param pHitInfos Where the hits are written.
 * @param maxHits The maximum number of hits.
 * @param rPos The circle center in screen space.
 * @param radius The circle radius.
 * @return True if any target was hit.
 */
bool ScreenPointDirector::hitCheckScreenCircle(sead::ObjArray<ScreenPointTargetHitInfo>* pHitInfos,
                                               s32 maxHits, const sead::Vector2f& rPos, f32 radius) {
    pHitInfos->clear();
    for (s32 i = 0; i < mCheckGroup->getValidTargetNum(); i++) {
        sead::Vector3f hitPos = sead::Vector3f::zero;
        sead::Vector3f hitNormal = sead::Vector3f::zero;
        ScreenPointTarget* target = mCheckGroup->getTarget(i);
        const IUseCamera* camera = target->mActor;
        f32 screenRadius = calcScreenRadiusFromWorldRadius(target->mPos, camera, target->mRadius);
        sead::Vector2f screenPos;
        calcScreenPosFromWorldPos(&screenPos, camera, target->mPos, 0);
        if (screenRadius + radius < (screenPos - rPos).length()) {
            continue;
        }

        f32 distance = (screenPos - rPos).length();
        ScreenPointTargetHitInfo hitInfo = {target, distance, hitPos, hitNormal};
        if (pHitInfos->size() >= maxHits) {
            break;
        }
        pHitInfos->pushBack(hitInfo);
    }

    sortHitInfoByDistance(pHitInfos);
    return pHitInfos->size() > 0;
}

}  // namespace al
