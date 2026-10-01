#include "Library/Joint/JointMasher.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Model/ModelShapeUtil.hpp"

namespace al {

/**
 * Constructs a masher with room for a number of joints.
 * @param pActor Owning actor.
 * @param pIsValid Flag enabling the squash.
 * @param maxJoints Maximum number of joints.
 */
JointMasher::JointMasher(const LiveActor* pActor, const bool* pIsValid, s32 maxJoints)
    : mActor(pActor), mIsValid(pIsValid) {
    mMashInfos.allocBuffer(maxJoints, nullptr);
}

/**
 * Registers a joint to squash with a scale rate.
 * @param pJointName Name of the joint.
 * @param rate Scale applied to the joint.
 */
void JointMasher::append(const char* pJointName, f32 rate) {
    s32 jointIndex = getJointIndex(mActor->getModelKeeper(), pJointName);
    mMashInfos.emplaceBack(MashInfo{jointIndex, rate});
    appendJointId(jointIndex);
}

void JointMasher::calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) {
    if (!*mIsValid) {
        return;
    }

    for (auto& info : mMashInfos) {
        if (info.jointIndex == jointIndex) {
            f32 rate = info.rate;
            sead::Vector3f trans;
            pMtx->getTranslation(trans);
            pMtx->scaleBases(rate, rate, rate);
            pMtx->setTranslation(trans);
            return;
        }
    }
}

}  // namespace al
