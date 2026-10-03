#include "Project/Joint/RollingCubePoseKeeper.hpp"

#include <attributes.h>

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/Math/Axis.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Project/Joint/RollingCubePoseKeeperUtil.hpp"
#include "Project/RollingCubePose.hpp"

namespace al {

/**
 * Constructs an empty keeper with no poses.
 */
NOINLINE RollingCubePoseKeeper::RollingCubePoseKeeper() = default;

/**
 * Sets the bounding box of the cube used by every pose.
 * @param rCubeSize Cube bounding box.
 */
NOINLINE void RollingCubePoseKeeper::setCubeSize(const sead::BoundBox3f& rCubeSize) {
    mCubeSize = rCubeSize;
}

/**
 * Checks whether the keys are traversed back and forth.
 * @return True for the Turn and All move types.
 */
bool RollingCubePoseKeeper::isMoveTypeTurn() const {
    return mMoveType == MoveType::Turn || mMoveType == MoveType::All;
}

/**
 * Checks whether the last key links back to the first one.
 * @return True for the Loop and All move types.
 */
bool RollingCubePoseKeeper::isMoveTypeLoop() const {
    return mMoveType == MoveType::Loop || mMoveType == MoveType::All;
}

/**
 * Builds the pose list from the actor placement and its "KeyMoveNext" links.
 * @param rInitInfo Actor init info.
 */
void RollingCubePoseKeeper::init(const ActorInitInfo& rInitInfo) {
    tryGetArg(reinterpret_cast<s32*>(&mMoveType), rInitInfo, "MoveType");
    s32 linkNextNum = calcLinkNestNum(rInitInfo, "KeyMoveNext");

    switch (mMoveType) {
    case MoveType::All:
        if (linkNextNum != 0) {
            mPoseCount = linkNextNum * 2;
        } else {
            mPoseCount = 1;
        }

        break;
    case MoveType::Turn:
        mPoseCount = linkNextNum * 2 + 1;
        break;
    default:
        mPoseCount = linkNextNum + 1;
        break;
    }

    mRollingCubePoses = new RollingCubePose[mPoseCount];

    mRollingCubePoses[0].setCubeSize(mCubeSize);
    mRollingCubePoses[0].init(rInitInfo.getPlacementInfo());

    if (mMoveType == MoveType::Turn) {
        mRollingCubePoses[mPoseCount - 1].setCubeSize(mCubeSize);
        mRollingCubePoses[mPoseCount - 1].init(rInitInfo.getPlacementInfo());
    }

    PlacementInfo currentPlacementInfo = rInitInfo.getPlacementInfo();
    PlacementInfo nextPlacementInfo;

    for (s32 i = 0; i < linkNextNum; i++) {
        getLinksInfo(&nextPlacementInfo, currentPlacementInfo, "KeyMoveNext");
        mRollingCubePoses[i + 1].setCubeSize(mCubeSize);
        mRollingCubePoses[i + 1].init(nextPlacementInfo);

        // Turning move types also visit every intermediate key on the way back. Turn has an
        // extra copy of the first key at the end, which shifts the mirrored keys by one.
        s32 mirrorIndexOffset = mMoveType == MoveType::Turn ? 0 : 1;

        if (i < linkNextNum - 1 && isMoveTypeTurn()) {
            s32 mirrorIndex = mPoseCount - i - 2 + mirrorIndexOffset;
            mRollingCubePoses[mirrorIndex].setCubeSize(mCubeSize);
            mRollingCubePoses[mirrorIndex].init(nextPlacementInfo);
        }

        currentPlacementInfo = nextPlacementInfo;
    }

    for (s32 i = 0; i < mPoseCount - 1; i++) {
        mRollingCubePoses[i].setNextCubePose(&mRollingCubePoses[i + 1]);
    }

    if (isMoveTypeLoop()) {
        mRollingCubePoses[mPoseCount - 1].setNextCubePose(&mRollingCubePoses[0]);
    }
}

/**
 * Advances to the next key, wrapping around for looping move types.
 * @return False if the end was reached on a non-looping move type.
 */
bool RollingCubePoseKeeper::nextKey() {
    mCurrentKeyIndex++;

    if (mCurrentKeyIndex < mPoseCount) {
        return true;
    }

    if (isMoveTypeLoop()) {
        mCurrentKeyIndex = 0;
        return true;
    }

    mCurrentKeyIndex = mPoseCount - 1;
    return false;
}

/**
 * Resets the current key to the first one.
 */
void RollingCubePoseKeeper::setStart() {
    mCurrentKeyIndex = 0;
}

/**
 * Sets the current key index.
 * @param index Key index.
 */
void RollingCubePoseKeeper::setKeyIndex(s32 index) {
    mCurrentKeyIndex = index;
}

/**
 * Gets the pose of the current key.
 * @return Current pose.
 */
const RollingCubePose& RollingCubePoseKeeper::getCurrentPose() const {
    return mRollingCubePoses[mCurrentKeyIndex];
}

/**
 * Gets the pose at the given key index.
 * @param index Key index.
 * @return Pose at that index.
 */
const RollingCubePose& RollingCubePoseKeeper::getPose(s32 index) const {
    return mRollingCubePoses[index];
}

/**
 * Calculates the world center of the cube for a given rotation and translation.
 * @param pCenter Receives the center.
 * @param rQuat Cube rotation.
 * @param rTrans Cube translation.
 */
void RollingCubePoseKeeper::calcBoundingBoxCenter(sead::Vector3f* pCenter, const sead::Quatf& rQuat,
                                                  const sead::Vector3f& rTrans) const {
    pCenter->set(mCubeSize.getCenter());

    sead::Matrix34f mtx;
    mtx.makeQT(rQuat, rTrans);

    pCenter->mul(mtx);
}

/**
 * Constructs a pose with identity rotation and rotate movement.
 */
NOINLINE RollingCubePose::RollingCubePose() = default;

/**
 * Checks whether moving to the next pose rotates over an edge.
 * @return True for rotate movement.
 */
bool RollingCubePose::isMovementRotate() const {
    return mMovementType == MovementType::Rotate;
}

/**
 * Checks whether moving to the next pose slides the cube.
 * @return True for slide movement.
 */
bool RollingCubePose::isMovementSlide() const {
    return mMovementType == MovementType::Slide;
}

/**
 * Sets the cube bounding box.
 * @param rCubeSize Cube bounding box.
 */
NOINLINE void RollingCubePose::setCubeSize(const sead::BoundBox3f& rCubeSize) {
    mCubeSize = rCubeSize;
}

/**
 * Calculates the box-equivalent rotation of this pose closest to a given rotation.
 * @param pNearPose Receives the rotation.
 * @param rQuat Reference rotation.
 */
void RollingCubePose::calcNearPose(sead::Quatf* pNearPose, const sead::Quatf& rQuat) const {
    calcFittingBoxPose(pNearPose, mCubeSize, rQuat, mQuat);
}

/**
 * Snaps a rotation and translation so the cube coincides with this pose.
 * @param pQuat Rotation to snap, in place.
 * @param pTrans Translation to snap, in place.
 */
void RollingCubePose::fittingToBoundingBox(sead::Quatf* pQuat, sead::Vector3f* pTrans) const {
    calcNearPose(pQuat, *pQuat);

    sead::Vector3f currentCenter;
    calcBoundingBoxCenter(&currentCenter);

    sead::Vector3f boxCenter;
    calcBoundingBoxCenter(&boxCenter, *pQuat, *pTrans);

    *pTrans += currentCenter - boxCenter;
}

/**
 * Calculates the world center of the cube in this pose.
 * @param pCenter Receives the center.
 */
void RollingCubePose::calcBoundingBoxCenter(sead::Vector3f* pCenter) const {
    calcBoundingBoxCenter(pCenter, mQuat, mTrans);
}

/**
 * Calculates the world center of the cube for a given rotation and translation.
 * @param pCenter Receives the center.
 * @param rQuat Cube rotation.
 * @param rTrans Cube translation.
 */
void RollingCubePose::calcBoundingBoxCenter(sead::Vector3f* pCenter, const sead::Quatf& rQuat,
                                            const sead::Vector3f& rTrans) const {
    pCenter->set(mCubeSize.getCenter());

    sead::Matrix34f mtx;
    mtx.makeQT(rQuat, rTrans);

    pCenter->mul(mtx);
}

/**
 * Computes how to move from this pose to the next one: rotating over a shared bottom edge if
 * possible, otherwise sliding.
 * @param pNextPose Next pose.
 */
NOINLINE void RollingCubePose::setNextCubePose(const RollingCubePose* pNextPose) {
    mSlideVec = sead::Vector3f::zero;

    sead::Vector3f currentBottomFacePoints[4];
    calcBottomFacePoint(currentBottomFacePoints);

    sead::Vector3f nextBottomFacePoints[4];
    pNextPose->calcBottomFacePoint(nextBottomFacePoints);

    // Find bottom points shared by both poses. Two shared points form the edge to rotate over.
    s32 pointCount = 0;
    sead::Vector3f firstPoint = sead::Vector3f::zero;
    sead::Vector3f secondPoint = sead::Vector3f::zero;

    for (s32 i = 0; i < 4; i++) {
        for (s32 e = 0; e < 4; e++) {
            if ((currentBottomFacePoints[i] - nextBottomFacePoints[e]).length() <= 10.0f) {
                if (pointCount == 0) {
                    pointCount = 1;
                    firstPoint.set(nextBottomFacePoints[e]);
                    continue;
                }

                if (pointCount == 1) {
                    pointCount = 2;
                    secondPoint.set(nextBottomFacePoints[e]);
                    continue;
                }

                pointCount++;
            }
        }
    }

    mMovementType = MovementType::None;

    if (pointCount == 2) {
        mRotateAxis = secondPoint - firstPoint;
        mRotateCenter = firstPoint;

        if (!normalizeOrZero(&mRotateAxis)) {
            sead::Vector3f currentCenter;
            calcBoundingBoxCenter(&currentCenter);

            sead::Vector3f nextCenter;
            pNextPose->calcBoundingBoxCenter(&nextCenter);

            mRotateDegree = calcAngleOnPlaneDegree(currentCenter - mRotateCenter,
                                                   nextCenter - mRotateCenter, mRotateAxis);
            mMovementType = MovementType::Rotate;
        }
    }

    if (mMovementType == MovementType::None) {
        mMovementType = MovementType::Slide;

        sead::Vector3f currentCenter;
        calcBoundingBoxCenter(&currentCenter);

        sead::Vector3f nextCenter;
        pNextPose->calcBoundingBoxCenter(&nextCenter);

        sead::Quatf nearPose;
        pNextPose->calcNearPose(&nearPose, mQuat);
        calcQuatRotateAxisAndDegree(&mRotateAxis, &mRotateDegree, mQuat, nearPose);

        mRotateCenter = currentCenter;
        mSlideVec.setSub(nextCenter, currentCenter);
    }
}

/**
 * Calculates the four world corner points of the face currently on the bottom.
 * @param pFacePoints Receives the four points.
 */
void RollingCubePose::calcBottomFacePoint(sead::Vector3f pFacePoints[4]) const {
    calcBoxFacePoint(pFacePoints, mCubeSize, static_cast<s32>(calcBottomFaceIndex()), mQuat,
                     mTrans);
}

/**
 * Reads the pose from placement info and keeps a copy of it.
 * @param rPlacementInfo Placement info of the key.
 */
NOINLINE void RollingCubePose::init(const PlacementInfo& rPlacementInfo) {
    tryGetQuat(&mQuat, rPlacementInfo);
    tryGetTrans(&mTrans, rPlacementInfo);
    mPlacementInfo = new PlacementInfo(rPlacementInfo);
}

/**
 * Calculates the rotation and translation part way towards the next pose.
 * @param pOutQuat Receives the rotation.
 * @param pOutTrans Receives the translation.
 * @param rQuat Start rotation.
 * @param rTrans Start translation.
 * @param rate Progress in [0, 1].
 */
void RollingCubePose::calcRotateQT(sead::Quatf* pOutQuat, sead::Vector3f* pOutTrans,
                                   const sead::Quatf& rQuat, const sead::Vector3f& rTrans,
                                   f32 rate) const {
    rotateQuatAndTransDegree(pOutQuat, pOutTrans, rQuat, rTrans, mRotateAxis, mRotateCenter,
                             mRotateDegree * rate);
    pOutTrans->setScaleAdd(rate, mSlideVec, *pOutTrans);
}

/**
 * Finds which local axis faces the most downwards.
 * @return Axis of the bottom face.
 */
Axis RollingCubePose::calcBottomFaceIndex() const {
    sead::Vector3f xAxis;
    sead::Vector3f yAxis;
    sead::Vector3f zAxis;
    calcQuatLocalAxisAll(mQuat, &xAxis, &yAxis, &zAxis);
    f32 x = sead::Mathf::abs(xAxis.y);
    f32 y = sead::Mathf::abs(yAxis.y);
    f32 z = sead::Mathf::abs(zAxis.y);

    if (x > y) {
        if (x > z) {
            return 0.0f > xAxis.y ? Axis::X : Axis::InvertX;
        }

        return 0.0f > zAxis.y ? Axis::Z : Axis::InvertZ;
    }

    if (y >= z) {
        return 0.0f > yAxis.y ? Axis::Y : Axis::InvertY;
    }

    return 0.0f > zAxis.y ? Axis::Z : Axis::InvertZ;
}

/**
 * Creates a keeper whose cube size is the model bounding box of an actor.
 * @param pActor Actor providing the model bounding box.
 * @param rInitInfo Actor init info.
 * @return New keeper.
 */
RollingCubePoseKeeper* createRollingCubePoseKeeper(const LiveActor* pActor,
                                                   const ActorInitInfo& rInitInfo) {
    sead::BoundBox3f modelBoundBox;
    calcModelBoundingBox(&modelBoundBox, pActor);
    return createRollingCubePoseKeeper(modelBoundBox, rInitInfo);
}

/**
 * Creates a keeper with the given cube size.
 * @param rCubeSize Cube bounding box.
 * @param rInitInfo Actor init info.
 * @return New keeper.
 */
RollingCubePoseKeeper* createRollingCubePoseKeeper(const sead::BoundBox3f& rCubeSize,
                                                   const ActorInitInfo& rInitInfo) {
    RollingCubePoseKeeper* pKeeper = new RollingCubePoseKeeper();
    pKeeper->setCubeSize(rCubeSize);
    pKeeper->init(rInitInfo);
    return pKeeper;
}

}  // namespace al
