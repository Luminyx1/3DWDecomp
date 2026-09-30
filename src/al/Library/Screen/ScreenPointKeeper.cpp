#include "Library/Screen/ScreenPointKeeper.hpp"

#include "Library/Screen/ScreenPointTarget.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Creates a keeper for a fixed number of screen point targets.
 * @param maxTargets maximum number of targets
 */
ScreenPointKeeper::ScreenPointKeeper(s32 maxTargets) : mMaxTargets(maxTargets) {
    mTargets = new ScreenPointTarget*[maxTargets];
    for (s32 i = 0; i < mMaxTargets; i++) {
        mTargets[i] = nullptr;
    }
}

/**
 * Creates and adds a screen point target.
 * @param pHost actor owning the target
 * @param pName target name
 * @param radius target radius
 * @param pFollowPos position to follow, or nullptr
 * @param pFollowMtx matrix to follow, or nullptr
 * @param rOffset offset from the followed position or matrix
 * @return the new target
 */
ScreenPointTarget* ScreenPointKeeper::addTarget(LiveActor* pHost, const char* pName, f32 radius,
                                                const sead::Vector3f* pFollowPos,
                                                const sead::Matrix34f* pFollowMtx,
                                                const sead::Vector3f& rOffset) {
    ScreenPointTarget* target =
        new ScreenPointTarget(pHost, pName, radius, pFollowPos, pFollowMtx, rOffset);
    mTargets[mTargetNum] = target;
    mTargetNum++;
    return target;
}

/**
 * Updates the position of every target.
 */
void ScreenPointKeeper::update() {
    for (s32 i = 0; i < mTargetNum; i++) {
        mTargets[i]->update();
    }
}

/**
 * Returns a target by index.
 * @param index target index
 * @return the target
 */
ScreenPointTarget* ScreenPointKeeper::getTarget(s32 index) const {
    return mTargets[index];
}

/**
 * Validates every target.
 */
void ScreenPointKeeper::validate() {
    for (s32 i = 0; i < mTargetNum; i++) {
        mTargets[i]->validate();
    }
}

/**
 * Invalidates every target.
 */
void ScreenPointKeeper::invalidate() {
    for (s32 i = 0; i < mTargetNum; i++) {
        mTargets[i]->invalidate();
    }
}

/**
 * Validates every target on behalf of the system.
 */
void ScreenPointKeeper::validateBySystem() {
    for (s32 i = 0; i < mTargetNum; i++) {
        mTargets[i]->validateBySystem();
    }
}

/**
 * Invalidates every target on behalf of the system.
 */
void ScreenPointKeeper::invalidateBySystem() {
    for (s32 i = 0; i < mTargetNum; i++) {
        mTargets[i]->invalidateBySystem();
    }
}

/**
 * Returns a target by name, or the only target if there is just one.
 * @param pName target name
 * @return the target, or nullptr
 */
ScreenPointTarget* ScreenPointKeeper::getTarget(const char* pName) const {
    if (mTargetNum == 1) {
        return mTargets[0];
    }

    for (s32 i = 0; i < mTargetNum; i++) {
        if (isEqualString(mTargets[i]->getName(), pName)) {
            return mTargets[i];
        }
    }

    return nullptr;
}
}  // namespace al
