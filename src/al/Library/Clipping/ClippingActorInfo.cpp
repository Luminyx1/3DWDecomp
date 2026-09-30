#include "Library/Clipping/ClippingActorInfo.hpp"

#include "Library/Clipping/ClippingJudge.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementId.hpp"

namespace al {
/**
 * Constructs the clipping info of an actor with a default sphere.
 * @param pActor actor to clip
 */
ClippingActorInfo::ClippingActorInfo(LiveActor* pActor)
    : mActor(pActor), mGroupClippingId(new PlacementId()) {
    setTypeToSphere(300.0f, nullptr);
}

/**
 * Sets the clipping shape to a sphere.
 * @param radius radius of the sphere
 * @param pPos center of the sphere, or nullptr to use the actor position
 */
void ClippingActorInfo::setTypeToSphere(f32 radius, const sead::Vector3f* pPos) {
    mClippingRadius = radius;

    if (!pPos) {
        pPos = getTransPtr(mActor);
    }

    mTransPtr = pPos;
}

/**
 * Starts or ends the clipping of the actor depending on the frustum.
 * @param pJudge clipping judge
 */
void ClippingActorInfo::updateClipping(const ClippingJudge* pJudge) {
    bool isJudgedClipped = judgeClipping(pJudge);
    bool isActorClipped = isClipped(mActor);

    if (isJudgedClipped) {
        if (!isActorClipped) {
            mActor->startClipped();
        }
    } else if (isActorClipped) {
        mActor->endClipped();
    }
}

/**
 * Checks whether the actor is outside the frustum.
 * @param pJudge clipping judge
 * @return true if the actor should be clipped
 */
bool ClippingActorInfo::judgeClipping(const ClippingJudge* pJudge) const {
    s32 farClipLevel = mFarClipLevel;

    if (mViewGroupFarClipFlag && *mViewGroupFarClipFlag) {
        farClipLevel = 0;
    }

    return pJudge->isJudgedToClipFrustum(*mTransPtr, mClippingRadius, mNearClipDistance,
                                         farClipLevel);
}

/**
 * Checks whether the actor is clipped with a group.
 * @return true if the actor has a clipping group id
 */
bool ClippingActorInfo::isGroupClipping() const {
    return mGroupClippingId->mPlacementID != nullptr;
}

/**
 * Reads the clipping group id of the actor.
 * @param rInfo actor init info
 */
void ClippingActorInfo::setGroupClippingId(const ActorInitInfo& rInfo) {
    alPlacementFunction::getClippingGroupId(mGroupClippingId, rInfo);
}
}  // namespace al
