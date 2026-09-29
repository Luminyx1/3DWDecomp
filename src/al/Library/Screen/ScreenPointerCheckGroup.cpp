#include "Library/Screen/ScreenPointCheckGroup.hpp"

namespace al {

/**
 * @brief Constructs an empty group.
 * @param capacity The maximum number of targets.
 */
ScreenPointCheckGroup::ScreenPointCheckGroup(s32 capacity) : mCapacity(capacity) {
    mTargets = new ScreenPointTarget*[capacity];
    for (s32 i = 0; i < mCapacity; i++) {
        mTargets[i] = nullptr;
    }
}

/**
 * @brief Moves a target into the valid part of the group.
 * @param pTarget The target to validate.
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
 * @brief Moves a target out of the valid part of the group.
 * @param pTarget The target to invalidate.
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
 * @brief Gets a target by index.
 * @param index The index.
 * @return The target.
 */
ScreenPointTarget* ScreenPointCheckGroup::getTarget(s32 index) const {
    return mTargets[index];
}

/**
 * @brief Adds a target to the group.
 * @param pTarget The target.
 */
void ScreenPointCheckGroup::setTarget(ScreenPointTarget* pTarget) {
    mTargets[mTargetNum] = pTarget;
    mTargetNum++;
}

}  // namespace al
