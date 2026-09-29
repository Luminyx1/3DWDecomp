#include "Library/Screen/ScreenPointKeeper.hpp"

#include "Library/Screen/ScreenPointTarget.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {

/**
 * @brief Constructs an empty keeper.
 * @param maxNumTargets The maximum number of targets.
 */
ScreenPointKeeper::ScreenPointKeeper(s32 maxNumTargets) : mMaxNumTargets(maxNumTargets) {
    mTargets = new ScreenPointTarget*[maxNumTargets];
    for (s32 i = 0; i < mMaxNumTargets; i++) {
        mTargets[i] = nullptr;
    }
}

/**
 * @brief Creates and adds a target.
 * @param pActor The actor that owns the target.
 * @param pName The target name.
 * @param radius The target radius.
 * @param pTrans The position the target follows when no joint is given.
 * @param pJointMtx The joint matrix the target follows, may be null.
 * @param rOffset The offset from the followed position.
 * @return The new target.
 */
ScreenPointTarget* ScreenPointKeeper::addTarget(LiveActor* pActor, const char* pName, f32 radius,
                                                const sead::Vector3f* pTrans,
                                                const sead::Matrix34f* pJointMtx,
                                                const sead::Vector3f& rOffset) {
    ScreenPointTarget* target = new ScreenPointTarget(pActor, pName, radius, pTrans, pJointMtx, rOffset);
    mTargets[mCurNumTargets] = target;
    mCurNumTargets++;
    return target;
}

/**
 * @brief Updates the positions of all targets.
 */
void ScreenPointKeeper::update() {
    for (s32 i = 0; i < mCurNumTargets; i++) {
        mTargets[i]->update();
    }
}

/**
 * @brief Gets a target by index.
 * @param index The index.
 * @return The target.
 */
ScreenPointTarget* ScreenPointKeeper::getTarget(s32 index) const {
    return mTargets[index];
}

/**
 * @brief Validates all targets.
 */
void ScreenPointKeeper::validate() {
    for (s32 i = 0; i < mCurNumTargets; i++) {
        mTargets[i]->validate();
    }
}

/**
 * @brief Invalidates all targets.
 */
void ScreenPointKeeper::invalidate() {
    for (s32 i = 0; i < mCurNumTargets; i++) {
        mTargets[i]->invalidate();
    }
}

/**
 * @brief Validates all targets on behalf of the system.
 */
void ScreenPointKeeper::validateBySystem() {
    for (s32 i = 0; i < mCurNumTargets; i++) {
        mTargets[i]->validateBySystem();
    }
}

/**
 * @brief Invalidates all targets on behalf of the system.
 */
void ScreenPointKeeper::invalidateBySystem() {
    for (s32 i = 0; i < mCurNumTargets; i++) {
        mTargets[i]->invalidateBySystem();
    }
}

/**
 * @brief Gets a target by name. With a single target, it is returned regardless of its name.
 * @param pName The target name.
 * @return The target, or null if none matches.
 */
ScreenPointTarget* ScreenPointKeeper::getTarget(const char* pName) const {
    if (mCurNumTargets == 1) {
        return mTargets[0];
    }

    for (s32 i = 0; i < mCurNumTargets; i++) {
        if (isEqualString(mTargets[i]->mName, pName)) {
            return mTargets[i];
        }
    }

    return nullptr;
}

}  // namespace al
