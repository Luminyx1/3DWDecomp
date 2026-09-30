#include "Library/Obj/SimpleCircleShadowXZ.hpp"

#include <math/seadQuat.h>

#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Obj/PartsFunction.hpp"

namespace al {
/**
 * Constructs a circle shadow projected on the XZ plane.
 * @param pName actor name
 */
SimpleCircleShadowXZ::SimpleCircleShadowXZ(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the shadow model and appears.
 * @param pHost host actor
 * @param rInfo actor init info
 * @param pArchiveName model archive name
 * @param pSuffix archive suffix
 */
void SimpleCircleShadowXZ::initSimpleCircleShadow(LiveActor* pHost, const ActorInitInfo& rInfo,
                                                  const char* pArchiveName, const char* pSuffix) {
    mHost = pHost;
    initActorWithArchiveNameNoPlacementInfo(this, rInfo, pArchiveName, pSuffix);
    invalidateClipping(this);
    makeActorAppeared();
}

/**
 * Updates the pose and appears.
 */
void SimpleCircleShadowXZ::makeActorAppeared() {
    updatePose();
    LiveActor::makeActorAppeared();
    mIsHostHidden = false;
}

/**
 * Interpolates the offset, scale and rotation and places the shadow below the host.
 */
void SimpleCircleShadowXZ::updatePose() {
    if (mInterpoleStep != mInterpoleFrame) {
        mInterpoleStep++;
        f32 rate = static_cast<f32>(mInterpoleStep) / static_cast<f32>(mInterpoleFrame);
        mOffset.x = mStartOffset.x + rate * (mEndOffset.x - mStartOffset.x);
        mOffset.y = mStartOffset.y + rate * (mEndOffset.y - mStartOffset.y);
        mOffset.z = mStartOffset.z + rate * (mEndOffset.z - mStartOffset.z);
        sead::Vector3f* scale = tryGetScalePtr(this);
        scale->x = mStartScale.x + rate * (mEndScale.x - mStartScale.x);
        scale->y = mStartScale.y + rate * (mEndScale.y - mStartScale.y);
        scale->z = mStartScale.z + rate * (mEndScale.z - mStartScale.z);
        mRotate.x = mStartRotate.x + rate * (mEndRotate.x - mStartRotate.x);
        mRotate.y = mStartRotate.y + rate * (mEndRotate.y - mStartRotate.y);
        mRotate.z = mStartRotate.z + rate * (mEndRotate.z - mStartRotate.z);
    }

    sead::Matrix34f hostMtx = *mHost->getBaseMtx();
    hostMtx.m[1][3] = 0.0f;
    sead::Vector3f trans;
    trans.setMul(hostMtx, mOffset);
    setTransX(this, trans.x);
    setTransY(this, trans.y);
    setTransZ(this, trans.z);
    sead::Vector3f up;
    hostMtx.getBase(up, 1);
    sead::Vector3f front;
    hostMtx.getBase(front, 2);
    normalizeOrDirZ(&front);
    sead::Quatf quat;
    makeQuatFrontUp(&quat, front, up);
    sead::Quatf rotate;
    rotate.setRPY(sead::Mathf::deg2rad(mRotate.x), sead::Mathf::deg2rad(mRotate.y),
                  sead::Mathf::deg2rad(mRotate.z));
    quat = quat * rotate;
    updatePoseQuat(this, quat);
}

/**
 * Hides with the host and follows it while visible.
 */
void SimpleCircleShadowXZ::control() {
    if (updateSyncHostVisible(&mIsHostHidden, this, mHost, mIsForceHide)) {
        updatePose();
    }
}

/**
 * Hides or shows with the host.
 */
void SimpleCircleShadowXZ::syncHostVisible() {
    updateSyncHostVisible(&mIsHostHidden, this, mHost, mIsForceHide);
}

/**
 * Starts interpolating the offset.
 * @param rOffset target offset
 */
void SimpleCircleShadowXZ::setOffsetWithInterpole(const sead::Vector3f& rOffset) {
    mEndOffset = rOffset;
    mStartOffset = mOffset;
    mInterpoleStep = 0;
}

/**
 * Starts interpolating the scale.
 * @param rScale target scale
 */
void SimpleCircleShadowXZ::setScaleWithInterpole(const sead::Vector3f& rScale) {
    mEndScale.x = rScale.x;
    mEndScale.y = rScale.y;
    mEndScale.z = rScale.z;
    static_cast<sead::BaseVec3<f32>&>(mStartScale) = getScale(this);
    mInterpoleStep = 0;
}

/**
 * Starts interpolating the rotation.
 * @param rRotate target rotation in degrees
 */
void SimpleCircleShadowXZ::setRotateWithInterpole(const sead::Vector3f& rRotate) {
    mEndRotate.x = rRotate.x;
    mEndRotate.y = rRotate.y;
    mEndRotate.z = rRotate.z;
    static_cast<sead::BaseVec3<f32>&>(mStartRotate) = mRotate;
    mInterpoleStep = 0;
}

/**
 * Sets the interpolation frame count and restarts the interpolation.
 * @param frame frame count
 */
void SimpleCircleShadowXZ::setInterpoleFrame(s32 frame) {
    mInterpoleFrame = frame;
    mInterpoleStep = 0;
}
}  // namespace al
