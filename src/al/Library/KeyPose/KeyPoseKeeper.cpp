#include "Library/KeyPose/KeyPoseKeeper.hpp"

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Project/Joint/KeyPose.hpp"

namespace al {

/**
 * Constructs an empty keeper that loops through its key poses.
 */
KeyPoseKeeper::KeyPoseKeeper() = default;

/**
 * Creates one key pose for the actor's placement and one for every "KeyMoveNext" link after it.
 * @param rInfo Init info of the owning actor.
 */
void KeyPoseKeeper::init(const ActorInitInfo& rInfo) {
    mKeyPoseCount = calcLinkNestNum(rInfo, "KeyMoveNext") + 1;
    tryGetArg(reinterpret_cast<s32*>(&mMoveType), rInfo, "MoveType");
    mKeyPoses = new KeyPose[mKeyPoseCount];

    mKeyPoses[0].init(rInfo.getPlacementInfo());

    PlacementInfo currentInfo = rInfo.getPlacementInfo();
    PlacementInfo nextInfo;

    for (s32 i = 0; i < mKeyPoseCount - 1; i++) {
        getLinksInfo(&nextInfo, currentInfo, "KeyMoveNext");
        mKeyPoses[i + 1].init(nextInfo);
        currentInfo = nextInfo;
    }
}

/**
 * Jumps to a key pose and restarts moving towards the end.
 * @param index Index of the key pose; ignored if out of range.
 */
void KeyPoseKeeper::setCurrentPoseIndex(s32 index) {
    if (index >= 0 && index < mKeyPoseCount) {
        mIsGoingToEnd = true;
        mIsStop = false;
        mIsRestart = false;
        mKeyPoseCurrentIdx = index;
    }
}

/**
 * Returns to the first key pose, moving towards the end.
 */
void KeyPoseKeeper::reset() {
    mKeyPoseCurrentIdx = 0;
    mIsGoingToEnd = true;
    mIsStop = false;
    mIsRestart = false;
}

/**
 * @param idx Index of the key pose.
 * @return The key pose at the given index.
 */
const KeyPose& KeyPoseKeeper::getKeyPose(s32 idx) const {
    return mKeyPoses[idx];
}

/**
 * @return The key pose the keeper is currently at.
 */
const KeyPose& KeyPoseKeeper::getCurrentKeyPose() const {
    return getKeyPose(mKeyPoseCurrentIdx);
}

/**
 * @return The key pose the keeper moves to next.
 */
const KeyPose& KeyPoseKeeper::getNextKeyPose() const {
    return getKeyPose(calcNextPoseIndex());
}

/**
 * Calculates the index of the next key pose in the current direction, wrapping around.
 * @return The index of the next key pose.
 */
s32 KeyPoseKeeper::calcNextPoseIndex() const {
    if (!mIsGoingToEnd) {
        if (mKeyPoseCurrentIdx - 1 < 0) {
            return mKeyPoseCount - 1;
        }

        return mKeyPoseCurrentIdx - 1;
    } else {
        if (mKeyPoseCurrentIdx + 1 >= mKeyPoseCount) {
            return 0;
        }

        return mKeyPoseCurrentIdx + 1;
    }
}

/**
 * Advances to the next key pose and applies the move type when the last key is reached.
 */
void KeyPoseKeeper::next() {
    mKeyPoseCurrentIdx = calcNextPoseIndex();

    switch (mMoveType) {
    case MoveType::Turn:
        if (isLastKey()) {
            mIsGoingToEnd = !mIsGoingToEnd;
        }

        break;
    case MoveType::Stop:
        if (isLastKey()) {
            mIsStop = true;
        }

        break;
    case MoveType::Restart:
        if (isLastKey()) {
            mIsRestart = true;
        }

        break;
    default:
        break;
    }
}

/**
 * @return Whether the current key pose is the last one in the current direction.
 */
bool KeyPoseKeeper::isLastKey() const {
    return mIsGoingToEnd ? mKeyPoseCurrentIdx + 1 >= mKeyPoseCount : mKeyPoseCurrentIdx < 1;
}

/**
 * Flips the moving direction.
 */
void KeyPoseKeeper::reverse() {
    mIsGoingToEnd = !mIsGoingToEnd;
}

/**
 * @return Whether the current key pose is the first one.
 */
bool KeyPoseKeeper::isFirstKey() const {
    return mKeyPoseCurrentIdx == 0;
}

/**
 * Makes the keeper wrap around to the first key pose after the last one.
 */
void KeyPoseKeeper::setMoveTypeLoop() {
    mMoveType = MoveType::Loop;
}

/**
 * Makes the keeper turn around at the last key pose.
 */
void KeyPoseKeeper::setMoveTypeTurn() {
    mMoveType = MoveType::Turn;
}

/**
 * Makes the keeper stop at the last key pose.
 */
void KeyPoseKeeper::setMoveTypeStop() {
    mMoveType = MoveType::Stop;
}

/**
 * Makes the keeper request a restart at the last key pose.
 */
void KeyPoseKeeper::setMoveTypeRestart() {
    mMoveType = MoveType::Restart;
}

}  // namespace al
