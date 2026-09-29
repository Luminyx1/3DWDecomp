#include "Library/Obj/SimpleCircleShadowXZ.hpp"

#include <math/seadMathCalcCommon.h>
#include <math/seadQuat.h>

#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Obj/PartsFunction.hpp"

namespace {
    /**
     * @brief Linearly interpolates between two vectors.
     * @param pOut The output vector.
     * @param rFrom The start vector.
     * @param rTo The end vector.
     * @param rate The interpolation rate.
     */
    inline void lerpVec(sead::Vector3f* pOut, const sead::Vector3f& rFrom, const sead::Vector3f& rTo, f32 rate) {
        pOut->x = rFrom.x + rate * (rTo.x - rFrom.x);
        pOut->y = rFrom.y + rate * (rTo.y - rFrom.y);
        pOut->z = rFrom.z + rate * (rTo.z - rFrom.z);
    }
}  // namespace

namespace al {
    /**
     * @brief Constructs a circle shadow that follows its host on the XZ plane.
     * @param pName The actor name.
     */
    SimpleCircleShadowXZ::SimpleCircleShadowXZ(const char* pName) : LiveActor(pName) {}

    /**
     * @brief Initializes the shadow model for a host actor.
     * @param pHost The host actor to follow.
     * @param rInfo The actor init info.
     * @param pArchiveName The archive name of the shadow model.
     * @param pSuffix The model suffix.
     */
    void SimpleCircleShadowXZ::initSimpleCircleShadow(LiveActor* pHost, const ActorInitInfo& rInfo,
                                                      const char* pArchiveName, const char* pSuffix) {
        mHostActor = pHost;
        initActorWithArchiveNameNoPlacementInfo(this, rInfo, pArchiveName, pSuffix);
        invalidateClipping(this);
        makeActorAppeared();
    }

    /**
     * @brief Updates the pose and appears.
     */
    void SimpleCircleShadowXZ::makeActorAppeared() {
        updatePose();
        LiveActor::makeActorAppeared();
        mIsHostHidden = false;
    }

    /**
     * @brief Advances the offset/scale/rotate interpolation and places the shadow below the host.
     */
    void SimpleCircleShadowXZ::updatePose() {
        if (mInterpoleStep != mInterpoleFrame) {
            mInterpoleStep++;
            f32 rate = static_cast<f32>(mInterpoleStep) / static_cast<f32>(mInterpoleFrame);
            lerpVec(&mOffset, mOffsetStart, mOffsetEnd, rate);
            lerpVec(tryGetScalePtr(this), mScaleStart, mScaleEnd, rate);
            lerpVec(&mRotate, mRotateStart, mRotateEnd, rate);
        }

        sead::Matrix34f baseMtx = *mHostActor->getBaseMtx();
        baseMtx.m[1][3] = 0.0f;
        sead::Vector3f trans = baseMtx * mOffset;
        setTransX(this, trans.x);
        setTransY(this, trans.y);
        setTransZ(this, trans.z);

        sead::Vector3f front;
        sead::Vector3f up;
        baseMtx.getBase(front, 2);
        baseMtx.getBase(up, 1);
        normalizeOrDirZ(&front);

        sead::Quatf quat;
        makeQuatFrontUp(&quat, front, up);
        sead::Quatf rotateQuat;
        rotateQuat.setRPY(sead::Mathf::deg2rad(mRotate.x), sead::Mathf::deg2rad(mRotate.y),
                          sead::Mathf::deg2rad(mRotate.z));
        quat = quat * rotateQuat;
        updatePoseQuat(this, quat);
    }

    /**
     * @brief Syncs visibility with the host and updates the pose while visible.
     */
    void SimpleCircleShadowXZ::control() {
        if (updateSyncHostVisible(&mIsHostHidden, this, mHostActor, mIsSyncHostHide)) {
            updatePose();
        }
    }

    /**
     * @brief Syncs visibility with the host.
     */
    void SimpleCircleShadowXZ::syncHostVisible() {
        updateSyncHostVisible(&mIsHostHidden, this, mHostActor, mIsSyncHostHide);
    }

    /**
     * @brief Starts interpolating the offset from the host towards a new value.
     * @param rOffset The target offset.
     */
    void SimpleCircleShadowXZ::setOffsetWithInterpole(const sead::Vector3f& rOffset) {
        mOffsetEnd.x = rOffset.x;
        mOffsetEnd.y = rOffset.y;
        mOffsetEnd.z = rOffset.z;
        mOffsetStart = mOffset;
        mInterpoleStep = 0;
    }

    /**
     * @brief Starts interpolating the scale towards a new value.
     * @param rScale The target scale.
     */
    void SimpleCircleShadowXZ::setScaleWithInterpole(const sead::Vector3f& rScale) {
        mScaleEnd.x = rScale.x;
        mScaleEnd.y = rScale.y;
        mScaleEnd.z = rScale.z;
        static_cast<sead::BaseVec3<f32>&>(mScaleStart) = getScale(this);
        mInterpoleStep = 0;
    }

    /**
     * @brief Starts interpolating the rotation towards a new value.
     * @param rRotate The target rotation in degrees.
     */
    void SimpleCircleShadowXZ::setRotateWithInterpole(const sead::Vector3f& rRotate) {
        mRotateEnd.x = rRotate.x;
        mRotateEnd.y = rRotate.y;
        mRotateEnd.z = rRotate.z;
        static_cast<sead::BaseVec3<f32>&>(mRotateStart) = mRotate;
        mInterpoleStep = 0;
    }

    /**
     * @brief Sets the number of frames the interpolations take and restarts them.
     * @param frame The interpolation frame count.
     */
    void SimpleCircleShadowXZ::setInterpoleFrame(s32 frame) {
        mInterpoleFrame = frame;
        mInterpoleStep = 0;
    }
}  // namespace al
