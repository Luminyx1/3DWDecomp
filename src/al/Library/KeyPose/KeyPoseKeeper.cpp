#include "Library/KeyPose/KeyPoseKeeper.hpp"

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Project/Joint/KeyPose.hpp"

namespace al {
KeyPoseKeeper::KeyPoseKeeper() = default;

void KeyPoseKeeper::init(const ActorInitInfo& rInfo) {
    mKeyPoseCount = calcLinkNestNum(rInfo, "KeyMoveNext") + 1;
    tryGetArg((s32*)&mMoveType, rInfo, "MoveType");
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

void KeyPoseKeeper::setCurrentPoseIndex(s32 index) {
    if (index >= 0 && index < mKeyPoseCount) {
        mIsGoingToEnd = true;
        mIsStop = false;
        mIsRestart = false;
        mKeyPoseCurrentIdx = index;
    }
}

void KeyPoseKeeper::reset() {
    mKeyPoseCurrentIdx = 0;
    mIsGoingToEnd = true;
    mIsStop = false;
    mIsRestart = false;
}

const KeyPose& KeyPoseKeeper::getKeyPose(s32 idx) const {
    return mKeyPoses[idx];
}

const KeyPose& KeyPoseKeeper::getCurrentKeyPose() const {
    return getKeyPose(mKeyPoseCurrentIdx);
}

const KeyPose& KeyPoseKeeper::getNextKeyPose() const {
    return getKeyPose(calcNextPoseIndex());
}

s32 KeyPoseKeeper::calcNextPoseIndex() const {
    if (!mIsGoingToEnd) {
        if (mKeyPoseCurrentIdx - 1 < 0)
            return mKeyPoseCount - 1;

        return mKeyPoseCurrentIdx - 1;
    } else {
        if (mKeyPoseCurrentIdx + 1 >= mKeyPoseCount)
            return 0;

        return mKeyPoseCurrentIdx + 1;
    }
}

void KeyPoseKeeper::next() {
    mKeyPoseCurrentIdx = calcNextPoseIndex();

    switch (mMoveType) {
    case MoveType::Turn:
        if (isLastKey())
            mIsGoingToEnd = !mIsGoingToEnd;

        break;
    case MoveType::Stop:
        if (isLastKey())
            mIsStop = true;

        break;
    case MoveType::Restart:
        if (isLastKey())
            mIsRestart = true;

        break;
    default:
        break;
    }
}

bool KeyPoseKeeper::isLastKey() const {
    return mIsGoingToEnd ? mKeyPoseCurrentIdx + 1 >= mKeyPoseCount : mKeyPoseCurrentIdx < 1;
}

void KeyPoseKeeper::reverse() {
    mIsGoingToEnd = !mIsGoingToEnd;
}

bool KeyPoseKeeper::isFirstKey() const {
    return mKeyPoseCurrentIdx == 0;
}

void KeyPoseKeeper::setMoveTypeLoop() {
    mMoveType = MoveType::Loop;
}

void KeyPoseKeeper::setMoveTypeTurn() {
    mMoveType = MoveType::Turn;
}

void KeyPoseKeeper::setMoveTypeStop() {
    mMoveType = MoveType::Stop;
}

void KeyPoseKeeper::setMoveTypeRestart() {
    mMoveType = MoveType::Restart;
}

}  // namespace al
