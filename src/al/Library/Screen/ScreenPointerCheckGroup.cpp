#include "Library/Screen/ScreenPointCheckGroup.hpp"

namespace al {
/**
 * Creates a check group for a fixed number of targets.
 * @param maxTargets maximum number of targets
 */
ScreenPointCheckGroup::ScreenPointCheckGroup(s32 maxTargets) : mMaxTargets(maxTargets) {
    mTargets = new ScreenPointTarget*[maxTargets];
    for (s32 i = 0; i < mMaxTargets; i++) {
        mTargets[i] = nullptr;
    }
}

/**
 * Moves a target into the valid part of the group.
 * @param pTarget target to validate
 */
void ScreenPointCheckGroup::setValid(ScreenPointTarget* pTarget) {
    for (s32 i = mValidTargetNum; i < mTargetNum; i++) {
        if (mTargets[i] == pTarget) {
            mTargets[i] = mTargets[mValidTargetNum];
            mTargets[mValidTargetNum] = pTarget;
            mValidTargetNum++;
            return;
        }
    }
}

/**
 * Moves a target out of the valid part of the group.
 * @param pTarget target to invalidate
 */
void ScreenPointCheckGroup::setInvalid(ScreenPointTarget* pTarget) {
    for (s32 i = 0; i < mValidTargetNum; i++) {
        if (mTargets[i] == pTarget) {
            mTargets[i] = mTargets[mValidTargetNum - 1];
            mTargets[mValidTargetNum - 1] = pTarget;
            mValidTargetNum--;
            return;
        }
    }
}

/**
 * Returns a target by index.
 * @param index target index
 * @return the target
 */
ScreenPointTarget* ScreenPointCheckGroup::getTarget(s32 index) const {
    return mTargets[index];
}

/**
 * Adds a target to the group.
 * @param pTarget target to add
 */
void ScreenPointCheckGroup::setTarget(ScreenPointTarget* pTarget) {
    mTargets[mTargetNum] = pTarget;
    mTargetNum++;
}
}  // namespace al
